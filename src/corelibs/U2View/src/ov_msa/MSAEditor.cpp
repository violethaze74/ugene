/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
 * http://ugene.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <QApplication>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainter>
#include <QResizeEvent>
#include <QSvgGenerator>
#include <QToolBar>
#include <QVBoxLayout>

#include <U2Algorithm/MSADistanceAlgorithm.h>
#include <U2Algorithm/MSADistanceAlgorithmRegistry.h>
#include <U2Algorithm/MsaColorScheme.h>
#include <U2Algorithm/MsaHighlightingScheme.h>
#include <U2Algorithm/PairwiseAlignmentTask.h>
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>

#include <U2Core/AddSequencesToAlignmentTask.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/Counter.h>
#include <U2Core/DNAChromatogram.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentSelection.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectSelection.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/MultipleChromatogramAlignment.h>
#include <U2Core/MultipleChromatogramAlignmentImporter.h>
#include <U2Core/MultipleChromatogramAlignmentObject.h>
#include <U2Core/PhyTreeObject.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/QObjectScopedPointer.h>
#include <U2Core/Settings.h>
#include <U2Core/TaskWatchdog.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/ExportImageDialog.h>
#include <U2Gui/GScrollBar.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/GroupHeaderImageWidget.h>
#include <U2Gui/GroupOptionsWidget.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/OPWidgetFactoryRegistry.h>
#include <U2Gui/OptionsPanel.h>
#include <U2Gui/OptionsPanelWidget.h>
#include <U2Gui/ProjectView.h>
#include <U2Gui/U2FileDialog.h>

#include <U2View/UndoRedoFramework.h>

#include "ExportHighlightedDialogController.h"
#include "MSAEditor.h"
#include "MSAEditorConsensusArea.h"
#include "MSAEditorOffsetsView.h"
#include "MSAEditorOverviewArea.h"
#include "MSAEditorSequenceArea.h"
#include "MSAEditorState.h"
#include "MSAEditorStatusBar.h"
#include "MSAEditorTasks.h"
#include "MaEditorFactory.h"
#include "MaEditorNameList.h"
#include "MsaEditorSimilarityColumn.h"
#include "AlignSequencesToAlignment/AlignSequencesToAlignmentTask.h"
#include "Export/MSAImageExportTask.h"
#include "helpers/MsaRowHeightController.h"
#include "ov_msa/TreeOptions//TreeOptionsWidgetFactory.h"
#include "ov_phyltree/TreeViewer.h"
#include "ov_phyltree/TreeViewerTasks.h"
#include "phyltree/CreatePhyTreeDialogController.h"
#include "PhyTrees/MSAEditorMultiTreeViewer.h"
#include "PhyTrees/MSAEditorTreeViewer.h"


namespace U2 {

/* TRANSLATOR U2::MSAEditor */

MSAEditor::MSAEditor(const QString& viewName, MultipleSequenceAlignmentObject* obj)
    : MaEditor(MsaEditorFactory::ID, viewName, obj),
      alignSequencesToAlignmentAction(NULL),
      treeManager(this)
{
    buildTreeAction = new QAction(QIcon(":/core/images/phylip.png"), tr("Build Tree"), this);
    buildTreeAction->setObjectName("Build Tree");
    buildTreeAction->setEnabled(!isAlignmentEmpty());
    connect(maObject, SIGNAL(si_alignmentBecomesEmpty(bool)), buildTreeAction, SLOT(setDisabled(bool)));
    connect(maObject, SIGNAL(si_rowsRemoved(const QList<qint64> &)), SLOT(sl_rowsRemoved(const QList<qint64> &)));
    connect(buildTreeAction, SIGNAL(triggered()), SLOT(sl_buildTree()));

    pairwiseAlignmentWidgetsSettings = new PairwiseAlignmentWidgetsSettings;
    if (maObject->getAlphabet() != NULL) {
        pairwiseAlignmentWidgetsSettings->customSettings.insert("alphabet", maObject->getAlphabet()->getId());
    }

    updateActions();
}

void MSAEditor::sl_buildTree() {
    treeManager.buildTreeWithDialog();
}

bool MSAEditor::onObjectRemoved(GObject* obj) {
    bool result = GObjectView::onObjectRemoved(obj);

    obj->disconnect(ui->getSequenceArea());
    obj->disconnect(ui->getConsensusArea());
    obj->disconnect(ui->getEditorNameList());
    return result;
}

void MSAEditor::onObjectRenamed(GObject*, const QString&) {
    // update title
    OpenMaEditorTask::updateTitle(this);
}

bool MSAEditor::onCloseEvent() {
    if (ui->getOverviewArea() != NULL) {
        ui->getOverviewArea()->cancelRendering();
    }
    return true;
}

int MSAEditor::getFirstVisibleBase() const {
    return ui->getSequenceArea()->getFirstVisibleBase();
}

const MultipleSequenceAlignmentRow MSAEditor::getRowByLineNumber(int lineNumber) const {
    if (ui->isCollapsibleMode()) {
        lineNumber = ui->getCollapseModel()->mapToRow(lineNumber);
    }
    return getMaObject()->getMsaRow(lineNumber);
}

MSAEditor::~MSAEditor() {
    delete pairwiseAlignmentWidgetsSettings;
}

void MSAEditor::buildStaticToolbar(QToolBar* tb) {
    MaEditor::buildStaticToolbar(tb);

    tb->addAction(buildTreeAction);
    tb->addAction(alignAction);
    tb->addAction(alignSequencesToAlignmentAction);

    GObjectView::buildStaticToolbar(tb);
}

void MSAEditor::buildStaticMenu(QMenu* m) {
    addLoadMenu(m);

    addCopyMenu(m);
    addEditMenu(m);

    addAlignMenu(m);
    addTreeMenu(m);
    addStatisticsMenu(m);

    addViewMenu(m);
    addExportMenu(m);

    addAdvancedMenu(m);

    GObjectView::buildStaticMenu(m);

    GUIUtils::disableEmptySubmenus(m);
}

void MSAEditor::addTreeMenu(QMenu* m) {
    QMenu* em = m->addMenu(tr("Tree"));
    //em->setIcon(QIcon(":core/images/tree.png"));
    em->menuAction()->setObjectName(MSAE_MENU_TREES);
    em->addAction(buildTreeAction);
}

void MSAEditor::addAdvancedMenu(QMenu* m) {
    QMenu* em = m->addMenu(tr("Advanced"));
    em->menuAction()->setObjectName(MSAE_MENU_ADVANCED);
}

void MSAEditor::addStatisticsMenu(QMenu* m) {
    QMenu* em = m->addMenu(tr("Statistics"));
    em->setIcon(QIcon(":core/images/chart_bar.png"));
    em->menuAction()->setObjectName(MSAE_MENU_STATISTICS);
}

Task* MSAEditor::updateViewTask(const QString& stateName, const QVariantMap& stateData) {
    return new UpdateMSAEditorTask(this, stateName, stateData);
}

QVariantMap MSAEditor::saveState() {
    return MSAEditorState::saveState(this);
}

QWidget* MSAEditor::createWidget() {
    Q_ASSERT(ui == NULL);
    ui = new MSAEditorUI(this);

    QString objName = "msa_editor_" + maObject->getGObjectName();
    ui->setObjectName(objName);

    initActions();

    connect(ui , SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sl_onContextMenuRequested(const QPoint &)));

    alignAction = new QAction(QIcon(":core/images/align.png"), tr("Align"), this);
    alignAction->setObjectName("Align");
    connect(alignAction, SIGNAL(triggered()), this, SLOT(sl_align()));

    alignSequencesToAlignmentAction = new QAction(QIcon(":/core/images/add_to_alignment.png"), tr("Align sequence to this alignment"), this);
    alignSequencesToAlignmentAction->setObjectName("Align sequence to this alignment");
    connect(alignSequencesToAlignmentAction, SIGNAL(triggered()), this, SLOT(sl_addToAlignment()));

    setAsReferenceSequenceAction = new QAction(tr("Set this sequence as reference"), this);
    setAsReferenceSequenceAction->setObjectName("set_seq_as_reference");
    connect(setAsReferenceSequenceAction, SIGNAL(triggered()), SLOT(sl_setSeqAsReference()));

    unsetReferenceSequenceAction = new QAction(tr("Unset reference sequence"), this);
    unsetReferenceSequenceAction->setObjectName("unset_reference");
    connect(unsetReferenceSequenceAction, SIGNAL(triggered()), SLOT(sl_unsetReferenceSeq()));

    optionsPanel = new OptionsPanel(this);
    OPWidgetFactoryRegistry *opWidgetFactoryRegistry = AppContext::getOPWidgetFactoryRegistry();

    QList<OPFactoryFilterVisitorInterface*> filters;
    filters.append(new OPFactoryFilterVisitor(ObjViewType_AlignmentEditor));

    QList<OPWidgetFactory*> opWidgetFactories = opWidgetFactoryRegistry->getRegisteredFactories(filters);
    foreach (OPWidgetFactory *factory, opWidgetFactories) {
        optionsPanel->addGroup(factory);
    }

    qDeleteAll(filters);

    connect(ui, SIGNAL(si_showTreeOP()), SLOT(sl_showTreeOP()));
    connect(ui, SIGNAL(si_hideTreeOP()), SLOT(sl_hideTreeOP()));
    sl_hideTreeOP();

    treeManager.loadRelatedTrees();

    initDragAndDropSupport();
    return ui;
}

void MSAEditor::sl_onContextMenuRequested(const QPoint & pos) {
    Q_UNUSED(pos);

    if (ui->childAt(pos) != NULL) {
        // ignore context menu request if overview area was clicked on
        if (ui->getOverviewArea()->isOverviewWidget(ui->childAt(pos))) {
            return;
        }
    }

    QMenu m;

    addLoadMenu(&m);
    addCopyMenu(&m);
    addEditMenu(&m);
    addAlignMenu(&m);
    addTreeMenu(&m);
    addStatisticsMenu(&m);
    addViewMenu(&m);
    addExportMenu(&m);
    addAdvancedMenu(&m);

    m.addSeparator();
    snp.clickPoint = QCursor::pos( );
    const QPoint nameMapped = ui->getEditorNameList()->mapFromGlobal( snp.clickPoint );
    const qint64 hoverRowId = ( 0 <= nameMapped.y( ) )
        ? ui->getEditorNameList()->sequenceIdAtPos( nameMapped ) : U2MsaRow::INVALID_ROW_ID;
    if ( ( hoverRowId != getReferenceRowId( )
        || U2MsaRow::INVALID_ROW_ID == getReferenceRowId( ) )
        && hoverRowId != U2MsaRow::INVALID_ROW_ID )
    {
        m.addAction( setAsReferenceSequenceAction );
    }
    if ( U2MsaRow::INVALID_ROW_ID != getReferenceRowId( ) ) {
        m.addAction( unsetReferenceSequenceAction );
    }
    m.addSeparator();

    emit si_buildPopupMenu(this, &m);

    GUIUtils::disableEmptySubmenus(&m);

    m.exec(QCursor::pos());
}

void MSAEditor::updateActions() {
    MaEditor::updateActions();
    if(alignSequencesToAlignmentAction != NULL) {
        alignSequencesToAlignmentAction->setEnabled(!maObject->isStateLocked());
    }
}

void MSAEditor::copyRowFromSequence(U2SequenceObject *seqObj, U2OpStatus &os) {
    MSAUtils::copyRowFromSequence(getMaObject(), seqObj, os);
    maObject->updateCachedMultipleAlignment();
}

void MSAEditor::sl_onSeqOrderChanged(const QStringList& order ){
    if(!maObject->isStateLocked()) {
        maObject->sortRowsByList(order);
    }
}

void MSAEditor::sl_showTreeOP() {
    OptionsPanelWidget* opWidget = dynamic_cast<OptionsPanelWidget*>(optionsPanel->getMainWidget());
    if(NULL == opWidget) {
        return;
    }

    QWidget* addTreeGroupWidget = opWidget->findOptionsWidgetByGroupId("OP_MSA_ADD_TREE_WIDGET");
    if(NULL != addTreeGroupWidget) {
        addTreeGroupWidget->hide();
        opWidget->closeOptionsPanel();
    }
    QWidget* addTreeHeader = opWidget->findHeaderWidgetByGroupId("OP_MSA_ADD_TREE_WIDGET");
    if(NULL != addTreeHeader) {
        addTreeHeader->hide();
    }

    GroupHeaderImageWidget* header = opWidget->findHeaderWidgetByGroupId("OP_MSA_TREES_WIDGET");
    if(NULL != header) {
        header->show();
        header->changeState();
    }
}

void MSAEditor::sl_hideTreeOP() {
    OptionsPanelWidget* opWidget = dynamic_cast<OptionsPanelWidget*>(optionsPanel->getMainWidget());
    if(NULL == opWidget) {
        return;
    }
    GroupHeaderImageWidget* header = opWidget->findHeaderWidgetByGroupId("OP_MSA_TREES_WIDGET");
    QWidget* groupWidget = opWidget->findOptionsWidgetByGroupId("OP_MSA_TREES_WIDGET");
    bool openAddTreeGroup = (NULL != groupWidget);
    if(openAddTreeGroup) {
        //header->changeState();
    }

    header->hide();

    GroupHeaderImageWidget* addTreeHeader = opWidget->findHeaderWidgetByGroupId("OP_MSA_ADD_TREE_WIDGET");
    if(NULL != addTreeHeader) {
        addTreeHeader->show();
        if(openAddTreeGroup) {
            addTreeHeader->changeState();
        }
    }
}


bool MSAEditor::eventFilter(QObject*, QEvent* e) {
    if (e->type() == QEvent::DragEnter || e->type() == QEvent::Drop) {
        QDropEvent* de = (QDropEvent*)e;
        const QMimeData* md = de->mimeData();
        const GObjectMimeData* gomd = qobject_cast<const GObjectMimeData*>(md);
        if (gomd != NULL) {
            if (maObject->isStateLocked()) {
                return false;
            }
            U2SequenceObject* dnaObj = qobject_cast<U2SequenceObject*> (gomd->objPtr.data());
            if (dnaObj != NULL ) {
                if (U2AlphabetUtils::deriveCommonAlphabet(dnaObj->getAlphabet(), maObject->getAlphabet()) == NULL) {
                    return false;
                }
                if (e->type() == QEvent::DragEnter) {
                    de->acceptProposedAction();
                } else {
                    U2OpStatusImpl os;
                    DNASequence seq = dnaObj->getWholeSequence(os);
                    seq.alphabet = dnaObj->getAlphabet();
                    Task *task = new AddSequenceObjectsToAlignmentTask(getMaObject(), QList<DNASequence>() << seq);
                    TaskWatchdog::trackResourceExistence(maObject, task, tr("A problem occurred during adding sequences. The multiple alignment is no more available."));
                    AppContext::getTaskScheduler()->registerTopLevelTask(task);
                }
            }
        }
    }
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(e);
        if (keyEvent->matches(QKeySequence::Paste)) {
            ui->getPasteAction()->trigger();
            return true;
        }
    }

    return false;
}

void MSAEditor::initDragAndDropSupport()
{
    SAFE_POINT(ui!= NULL, QString("MSAEditor::ui is not initialized in MSAEditor::initDragAndDropSupport"),);
    ui->setAcceptDrops(true);
    ui->installEventFilter(this);
}

void MSAEditor::sl_align(){
    QMenu m, *mm;

    addLoadMenu(&m);
    addCopyMenu(&m);
    addEditMenu(&m);
    addAlignMenu(&m);
    addTreeMenu(&m);
    addStatisticsMenu(&m);
    addViewMenu(&m);
    addExportMenu(&m);
    addAdvancedMenu(&m);

    emit si_buildPopupMenu(this, &m);

    GUIUtils::disableEmptySubmenus(&m);

    mm = GUIUtils::findSubMenu(&m, MSAE_MENU_ALIGN);
    SAFE_POINT(mm != NULL, "mm", );

    mm->exec(QCursor::pos());
}

void MSAEditor::sl_addToAlignment() {
    MultipleSequenceAlignmentObject* msaObject = getMaObject();
    if (msaObject->isStateLocked()) {
        return;
    }

    ProjectView* pv = AppContext::getProjectView();
    SAFE_POINT(pv != NULL, "Project view is null",);

    const GObjectSelection* selection = pv->getGObjectSelection();
    SAFE_POINT(selection  != NULL, "GObjectSelection is null",);

    QList<GObject*> objects = selection->getSelectedObjects();
    bool selectFromProject = !objects.isEmpty();

    foreach(GObject* object, objects) {
        if(object == getMaObject() || (object->getGObjectType() != GObjectTypes::MULTIPLE_SEQUENCE_ALIGNMENT && object->getGObjectType() != GObjectTypes::SEQUENCE)) {
            selectFromProject = false;
            break;
        }
    }
    if(selectFromProject ) {
        alignSequencesFromObjectsToAlignment(objects);
    } else {
        alignSequencesFromFilesToAlignment();
    }
}

void MSAEditor::alignSequencesFromObjectsToAlignment(const QList<GObject*>& objects) {
    SequenceObjectsExtractor extractor;
    extractor.setAlphabet(maObject->getAlphabet());
    extractor.extractSequencesFromObjects(objects);

    if(!extractor.getSequenceRefs().isEmpty()) {
        AlignSequencesToAlignmentTask* task = new AlignSequencesToAlignmentTask(getMaObject(), extractor);
        TaskWatchdog::trackResourceExistence(maObject, task, tr("A problem occurred during adding sequences. The multiple alignment is no more available."));
        AppContext::getTaskScheduler()->registerTopLevelTask(task);
    }
}

void MSAEditor::alignSequencesFromFilesToAlignment() {
    QString filter = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::SEQUENCE, true);

    LastUsedDirHelper lod;
    QStringList urls;
#ifdef Q_OS_MAC
    if (qgetenv(ENV_GUI_TEST).toInt() == 1 && qgetenv(ENV_USE_NATIVE_DIALOGS).toInt() == 0) {
        urls = U2FileDialog::getOpenFileNames(ui, tr("Open file with sequences"), lod.dir, filter, 0, QFileDialog::DontUseNativeDialog );
    } else
#endif
        urls = U2FileDialog::getOpenFileNames(ui, tr("Open file with sequences"), lod.dir, filter);

    if (!urls.isEmpty()) {
        lod.url = urls.first();
        LoadSequencesAndAlignToAlignmentTask * task = new LoadSequencesAndAlignToAlignmentTask(getMaObject(), urls);
        TaskWatchdog::trackResourceExistence(maObject, task, tr("A problem occurred during adding sequences. The multiple alignment is no more available."));
        AppContext::getTaskScheduler()->registerTopLevelTask(task);
    }
}

void MSAEditor::sl_setSeqAsReference(){
    QPoint menuCallPos = snp.clickPoint;
    QPoint nameMapped = ui->getEditorNameList()->mapFromGlobal(menuCallPos);
    if ( nameMapped.y() >= 0 ) {
        qint64 newRowId = ui->getEditorNameList()->sequenceIdAtPos(nameMapped);
        if (U2MsaRow::INVALID_ROW_ID != newRowId && newRowId != snp.seqId) {
            setReference(newRowId);
        }
    }
}

void MSAEditor::sl_unsetReferenceSeq( ) {
    if ( U2MsaRow::INVALID_ROW_ID != getReferenceRowId( ) ) {
        setReference( U2MsaRow::INVALID_ROW_ID );
    }
}

void MSAEditor::sl_rowsRemoved(const QList<qint64> &rowIds) {
    foreach (qint64 rowId, rowIds) {
        if (getReferenceRowId() == rowId) {
            sl_unsetReferenceSeq();
            break;
        }
    }
}

void MSAEditor::buildTree() {
    sl_buildTree();
}

QString MSAEditor::getReferenceRowName() const {
    const MultipleAlignment alignment = getMaObject()->getMultipleAlignment();
    U2OpStatusImpl os;
    const int refSeq = alignment->getRowIndexByRowId(getReferenceRowId(), os);
    return (U2MsaRow::INVALID_ROW_ID != refSeq) ? alignment->getRowNames().at(refSeq)
                                                : QString();
}

char MSAEditor::getReferenceCharAt(int pos) const {
    CHECK(getReferenceRowId() != U2MsaRow::INVALID_ROW_ID, '\n');

    U2OpStatusImpl os;
    const int refSeq = maObject->getMultipleAlignment()->getRowIndexByRowId(getReferenceRowId(), os);
    SAFE_POINT_OP(os, '\n');

    return maObject->getMultipleAlignment()->charAt(refSeq, pos);
}

//////////////////////////////////////////////////////////////////////////
MSAEditorUI::MSAEditorUI(MSAEditor* editor)
    : MaEditorWgt(editor),
      multiTreeViewer(NULL),
      similarityStatistics(NULL) {
    rowHeightController = new MsaRowHeightController(this);
    initActions();
    initWidgets();
}

MSAEditorSequenceArea* MSAEditorUI::getSequenceArea() const {
    return qobject_cast<MSAEditorSequenceArea* >(seqArea);
}

void MSAEditorUI::sl_onTabsCountChanged(int curTabsNumber) {
    if(curTabsNumber < 1) {
        maSplitter.removeWidget(multiTreeViewer);
        delete multiTreeViewer;
        multiTreeViewer = NULL;
        emit si_hideTreeOP();
        nameList->clearGroupsSelections();
    }
}

void MSAEditorUI::createDistanceColumn(MSADistanceMatrix* matrix)
{
    dataList->setMatrix(matrix);
    dataList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    MsaEditorAlignmentDependentWidget* statisticsWidget = new MsaEditorAlignmentDependentWidget(dataList);

    maSplitter.addWidget(nameAreaContainer, statisticsWidget, 0.04, 1);
}

void MSAEditorUI::addTreeView(GObjectViewWindow* treeView) {
    if (NULL == multiTreeViewer) {
        multiTreeViewer = new MSAEditorMultiTreeViewer(tr("Tree view"), getEditor());
        maSplitter.addWidget(nameAreaContainer, multiTreeViewer, 0.35);
        multiTreeViewer->addTreeView(treeView);
        emit si_showTreeOP();
        connect(multiTreeViewer, SIGNAL(si_tabsCountChanged(int)), SLOT(sl_onTabsCountChanged(int)));
    }
    else {
        multiTreeViewer->addTreeView(treeView);
    }
}

void MSAEditorUI::setSimilaritySettings( const SimilarityStatisticsSettings* settings ) {
    similarityStatistics->setSettings(settings);
}

void MSAEditorUI::refreshSimilarityColumn() {
    dataList->updateWidget();
}

void MSAEditorUI::showSimilarity() {
    if(NULL == similarityStatistics) {
        SimilarityStatisticsSettings settings;
        settings.ma = getEditor()->getMaObject();
        settings.algoName = AppContext::getMSADistanceAlgorithmRegistry()->getAlgorithmIds().at(0);
        settings.ui = this;

        dataList = new MsaEditorSimilarityColumn(this, new QScrollBar(Qt::Horizontal), &settings);
        dataList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        similarityStatistics = new MsaEditorAlignmentDependentWidget(dataList);

        maSplitter.addWidget(nameAreaContainer, similarityStatistics, 0.04, 1);
    }
    else {
        similarityStatistics->show();
    }

}

void MSAEditorUI::hideSimilarity() {
    if(NULL != similarityStatistics) {
        similarityStatistics->hide();
        similarityStatistics->cancelPendingTasks();
    }
}

void MSAEditorUI::initSeqArea(GScrollBar* shBar, GScrollBar* cvBar) {
    seqArea = new MSAEditorSequenceArea(this, shBar, cvBar);
}

void MSAEditorUI::initOverviewArea() {
    overviewArea = new MSAEditorOverviewArea(this);
}

void MSAEditorUI::initNameList(QScrollBar *nhBar) {
    nameList = new MsaEditorNameList(this, nhBar);
}

MSAEditorTreeViewer* MSAEditorUI::getCurrentTree() const
{
    if(NULL == multiTreeViewer) {
        return NULL;
    }
    GObjectViewWindow* page = qobject_cast<GObjectViewWindow*>(multiTreeViewer->getCurrentWidget());
    if(NULL == page) {
        return NULL;
    }
    return qobject_cast<MSAEditorTreeViewer*>(page->getObjectView());
}

}//namespace
