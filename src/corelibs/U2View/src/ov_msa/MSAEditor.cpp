/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
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
#include <QFontDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
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
#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/Counter.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentSelection.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectSelection.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/PhyTreeObject.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/QObjectScopedPointer.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/Settings.h>
#include <U2Core/TaskWatchdog.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/ExportDocumentDialogController.h>
#include <U2Gui/ExportImageDialog.h>
#include <U2Gui/ExportObjectUtils.h>
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

#include "Export/MSAImageExportTask.h"
#include "ExportHighlightedDialogController.h"
#include "MSAEditor.h"
#include "MSAEditorConsensusArea.h"
#include "MSAEditorFactory.h"
#include "MSAEditorNameList.h"
#include "MSAEditorOffsetsView.h"
#include "MSAEditorOverviewArea.h"
#include "MSAEditorSequenceArea.h"
#include "MSAEditorState.h"
#include "MSAEditorStatusBar.h"
#include "MSAEditorTasks.h"
#include "MsaEditorSimilarityColumn.h"
#include "AlignSequencesToAlignment/AlignSequencesToAlignmentTask.h"
#include "PhyTrees/MSAEditorMultiTreeViewer.h"
#include "PhyTrees/MSAEditorTreeViewer.h"
#include "ov_msa/TreeOptions//TreeOptionsWidgetFactory.h"
#include "ov_phyltree/TreeViewer.h"
#include "ov_phyltree/TreeViewerTasks.h"
#include "phyltree/CreatePhyTreeDialogController.h"

namespace U2 {

/* TRANSLATOR U2::MSAEditor */

MSAEditor::MSAEditor(const QString& viewName, GObject* obj)
    : MaEditor(MSAEditorFactory::ID, viewName, obj),
      alignSequencesToAlignmentAction(NULL),
      treeManager(this) {

    showChromatograms = true; // SANGER_TODO: check if there are chromatograms

    saveAlignmentAction = new QAction(QIcon(":core/images/msa_save.png"), tr("Save alignment"), this);
    saveAlignmentAction->setObjectName("Save alignment");
    connect(saveAlignmentAction, SIGNAL(triggered()), SLOT(sl_saveAlignment()));

    saveAlignmentAsAction = new QAction(QIcon(":core/images/msa_save_as.png"), tr("Save alignment as"), this);
    saveAlignmentAsAction->setObjectName("Save alignment as");
    connect(saveAlignmentAsAction, SIGNAL(triggered()), SLOT(sl_saveAlignmentAs()));

    zoomInAction = new QAction(QIcon(":core/images/zoom_in.png"), tr("Zoom In"), this);
    zoomInAction->setObjectName("Zoom In");
    connect(zoomInAction, SIGNAL(triggered()), SLOT(sl_zoomIn()));

    zoomOutAction = new QAction(QIcon(":core/images/zoom_out.png"), tr("Zoom Out"), this);
    zoomOutAction->setObjectName("Zoom Out");
    connect(zoomOutAction, SIGNAL(triggered()), SLOT(sl_zoomOut()));

    zoomToSelectionAction = new QAction(QIcon(":core/images/zoom_reg.png"), tr("Zoom To Selection"), this);
    zoomToSelectionAction->setObjectName("Zoom To Selection");
    connect(zoomToSelectionAction, SIGNAL(triggered()), SLOT(sl_zoomToSelection()));

    resetFontAction = new QAction(QIcon(":core/images/zoom_whole.png"), tr("Reset Zoom"), this);
    resetFontAction->setObjectName("Reset Zoom");
    connect(resetFontAction, SIGNAL(triggered()), SLOT(sl_resetZoom()));

    changeFontAction = new QAction(QIcon(":core/images/font.png"), tr("Change Font"), this);
    changeFontAction->setObjectName("Change Font");
    connect(changeFontAction, SIGNAL(triggered()), SLOT(sl_changeFont()));

    buildTreeAction = new QAction(QIcon(":/core/images/phylip.png"), tr("Build Tree"), this);
    buildTreeAction->setObjectName("Build Tree");
    buildTreeAction->setEnabled(!isAlignmentEmpty());
    connect(msaObject, SIGNAL(si_alignmentBecomesEmpty(bool)), buildTreeAction, SLOT(setDisabled(bool)));
    connect(msaObject, SIGNAL(si_rowsRemoved(const QList<qint64> &)), SLOT(sl_rowsRemoved(const QList<qint64> &)));
    connect(buildTreeAction, SIGNAL(triggered()), SLOT(sl_buildTree()));

    connect(msaObject, SIGNAL(si_lockedStateChanged()), SLOT(sl_lockedStateChanged()));

    Settings* s = AppContext::getSettings();
    zoomFactor = MOBJECT_DEFAULT_ZOOM_FACTOR;
    font.setFamily(s->getValue(MOBJECT_SETTINGS_ROOT + MOBJECT_SETTINGS_FONT_FAMILY, MOBJECT_DEFAULT_FONT_FAMILY).toString());
    font.setPointSize(s->getValue(MOBJECT_SETTINGS_ROOT + MOBJECT_SETTINGS_FONT_SIZE, MOBJECT_DEFAULT_FONT_SIZE).toInt());
    font.setItalic(s->getValue(MOBJECT_SETTINGS_ROOT + MOBJECT_SETTINGS_FONT_ITALIC, false).toBool());
    font.setBold(s->getValue(MOBJECT_SETTINGS_ROOT + MOBJECT_SETTINGS_FONT_BOLD, false).toBool());
    calcFontPixelToPointSizeCoef();

    if ( (font.pointSize() == MOBJECT_MIN_FONT_SIZE) && (zoomFactor < 1.0f) ) {
        resizeMode = ResizeMode_OnlyContent;
    } else {
        resizeMode = ResizeMode_FontAndContent;
    }

    pairwiseAlignmentWidgetsSettings = new PairwiseAlignmentWidgetsSettings;
    if (msaObject->getAlphabet() != NULL) {
        pairwiseAlignmentWidgetsSettings->customSettings.insert("alphabet", msaObject->getAlphabet()->getId());
    }

    exportHighlightedAction = new QAction(tr("Export highlighted"), this);
    exportHighlightedAction->setObjectName("Export highlighted");
    connect(exportHighlightedAction, SIGNAL(triggered()), this, SLOT(sl_exportHighlighted()));
    exportHighlightedAction->setDisabled(true);

    updateActions();
}

void MSAEditor::sl_saveAlignment(){
    AppContext::getTaskScheduler()->registerTopLevelTask(new SaveDocumentTask(msaObject->getDocument()));
}

void MSAEditor::sl_saveAlignmentAs(){

    Document* srcDoc = msaObject->getDocument();
    if (srcDoc == NULL) {
        return;
    }
    if (!srcDoc->isLoaded()) {
        return;
    }

    QObjectScopedPointer<ExportDocumentDialogController> dialog = new ExportDocumentDialogController(srcDoc, ui);
    dialog->setAddToProjectFlag(true);
    dialog->setWindowTitle(tr("Save alignment"));
    ExportObjectUtils::export2Document(dialog);
}

void MSAEditor::sl_zoomIn() {

    int pSize = font.pointSize();

    if (resizeMode == ResizeMode_OnlyContent) {
        zoomFactor *= zoomMult;
    } else if ( (pSize < MOBJECT_MAX_FONT_SIZE) && (resizeMode == ResizeMode_FontAndContent) ) {
        font.setPointSize(pSize+1);
        setFont(font);
    }

    bool resizeModeChanged = false;

    if (zoomFactor >= 1) {
        ResizeMode oldMode = resizeMode;
        resizeMode = ResizeMode_FontAndContent;
        resizeModeChanged = resizeMode != oldMode;
        zoomFactor = 1;
    }
    updateActions();

    emit si_zoomOperationPerformed(resizeModeChanged);
}

void MSAEditor::sl_zoomOut() {
    int pSize = font.pointSize();

    bool resizeModeChanged = false;

    if (pSize > MOBJECT_MIN_FONT_SIZE) {
        font.setPointSize(pSize-1);
        setFont(font);
    } else {
        SAFE_POINT(zoomMult > 0, QString("Incorrect value of MSAEditor::zoomMult"),);
        zoomFactor /= zoomMult;
        ResizeMode oldMode = resizeMode;
        resizeMode = ResizeMode_OnlyContent;
        resizeModeChanged = resizeMode != oldMode;
    }
    updateActions();

    emit si_zoomOperationPerformed(resizeModeChanged);
}

void MSAEditor::sl_zoomToSelection()
{
    ResizeMode oldMode = resizeMode;
    int seqAreaWidth =  ui->getSequenceArea()->width();
    MSAEditorSelection selection = ui->getSequenceArea()->getSelection();
    if (selection.isNull()) {
        return;
    }
    int selectionWidth = selection.width();
    float pixelsPerBase = (seqAreaWidth / float(selectionWidth)) * zoomMult;
    int fontPointSize = int(pixelsPerBase / fontPixelToPointSize);
    if (fontPointSize >= MOBJECT_MIN_FONT_SIZE) {
        if (fontPointSize > MOBJECT_MAX_FONT_SIZE) {
            fontPointSize = MOBJECT_MAX_FONT_SIZE;
        }
        font.setPointSize(fontPointSize);
        setFont(font);
        resizeMode = ResizeMode_FontAndContent;
        zoomFactor = 1;
    } else {
        if (font.pointSize() != MOBJECT_MIN_FONT_SIZE) {
            font.setPointSize(MOBJECT_MIN_FONT_SIZE);
            setFont(font);
        }
        zoomFactor = pixelsPerBase / (MOBJECT_MIN_FONT_SIZE * fontPixelToPointSize);
        resizeMode = ResizeMode_OnlyContent;
    }
    ui->getSequenceArea()->setFirstVisibleBase(selection.x());
    ui->getSequenceArea()->setFirstVisibleSequence(selection.y());

    updateActions();

    emit si_zoomOperationPerformed(resizeMode != oldMode);
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
    OpenMSAEditorTask::updateTitle(this);
}

bool MSAEditor::onCloseEvent() {
    if (ui->getOverviewArea() != NULL) {
        ui->getOverviewArea()->cancelRendering();
    }
    return true;
}

static void saveFont(const QFont& f) {
    Settings* s = AppContext::getSettings();
    s->setValue(MOBJECT_SETTINGS_ROOT + MOBJECT_SETTINGS_FONT_FAMILY, f.family());
    s->setValue(MOBJECT_SETTINGS_ROOT + MOBJECT_SETTINGS_FONT_SIZE, f.pointSize());
    s->setValue(MOBJECT_SETTINGS_ROOT + MOBJECT_SETTINGS_FONT_ITALIC, f.italic());
    s->setValue(MOBJECT_SETTINGS_ROOT + MOBJECT_SETTINGS_FONT_BOLD, f.bold());
}

void MSAEditor::setFont(const QFont& f) {
    int pSize = f.pointSize();
    font = f;
    calcFontPixelToPointSizeCoef();
    font.setPointSize(qBound(MOBJECT_MIN_FONT_SIZE, pSize, MOBJECT_MAX_FONT_SIZE));
    emit si_fontChanged(f);
    saveFont(font);
}

void MSAEditor::setFirstVisibleBase(int firstPos) {

    if (ui->getSequenceArea()->isPosInRange(firstPos)) {
        ui->getSequenceArea()->setFirstVisibleBase(firstPos);
    }
}

int MSAEditor::getFirstVisibleBase() const {

    return ui->getSequenceArea()->getFirstVisibleBase();
}

const MultipleSequenceAlignmentRow MSAEditor::getRowByLineNumber(int lineNumber) const {
    if (ui->isCollapsibleMode()) {
        lineNumber = ui->getCollapseModel()->mapToRow(lineNumber);
    }
    return getMSAObject()->getMsaRow(lineNumber);
}

void MSAEditor::sl_changeFont() {
    bool ok = false;
    QFont f = QFontDialog::getFont(&ok, font, widget, tr("Select font for alignment"));
    if (!ok) {
        return;
    }
    setFont(f);
    updateActions();
}

void MSAEditor::sl_resetZoom() {
    QFont f = getFont();
    f.setPointSize(MOBJECT_DEFAULT_FONT_SIZE);
    setFont(f);
    zoomFactor = MOBJECT_DEFAULT_ZOOM_FACTOR;
    ResizeMode oldMode = resizeMode;
    resizeMode = ResizeMode_FontAndContent;
    emit si_zoomOperationPerformed(resizeMode != oldMode);

    updateActions();
}

MSAEditor::~MSAEditor() {
    delete pairwiseAlignmentWidgetsSettings;
}

void MSAEditor::buildStaticToolbar(QToolBar* tb) {
    tb->addAction(ui->getCopyFormattedSelectionAction());
    tb->addAction(saveAlignmentAction);
    tb->addAction(saveAlignmentAsAction);
    tb->addAction(zoomInAction);
    tb->addAction(zoomOutAction);
    tb->addAction(zoomToSelectionAction);
    tb->addAction(resetFontAction);
    tb->addAction(showOverviewAction);
    tb->addAction(showChromatogramsAction);
    tb->addAction(changeFontAction);
    tb->addAction(buildTreeAction);
    tb->addAction(saveScreenshotAction);
    tb->addAction(alignAction);
    tb->addAction(alignSequencesToAlignmentAction);

    toolbar = tb;

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
    //addSNPMenu(m);

    GObjectView::buildStaticMenu(m);

    GUIUtils::disableEmptySubmenus(m);
}


void MSAEditor::addCopyMenu(QMenu* m) {
    QMenu* cm = m->addMenu(tr("Copy/Paste"));
    cm->menuAction()->setObjectName(MSAE_MENU_COPY);
}

void MSAEditor::addEditMenu(QMenu* m) {
    QMenu* em = m->addMenu(tr("Edit"));
    em->menuAction()->setObjectName(MSAE_MENU_EDIT);
}

void MSAEditor::addExportMenu(QMenu* m) {
    QMenu* em = m->addMenu(tr("Export"));
    em->menuAction()->setObjectName(MSAE_MENU_EXPORT);
    em->addAction(saveScreenshotAction);
    em->addAction(exportHighlightedAction);
    if(!ui->getSequenceArea()->getCurrentHighlightingScheme()->getFactory()->isRefFree() &&
                getReferenceRowId() != U2MsaRow::INVALID_ROW_ID){
        exportHighlightedAction->setEnabled(true);
    }else{
        exportHighlightedAction->setDisabled(true);
    }
}

void MSAEditor::addViewMenu(QMenu* m) {
    QMenu* em = m->addMenu(tr("View"));
    em->menuAction()->setObjectName(MSAE_MENU_VIEW);
    if (ui->getOffsetsViewController() != NULL) {
        em->addAction(ui->getOffsetsViewController()->getToggleColumnsViewAction());
    }
}

void MSAEditor::addAlignMenu(QMenu* m) {
    QMenu* em = m->addMenu(tr("Align"));
    em->setIcon(QIcon(":core/images/align.png"));
    em->menuAction()->setObjectName(MSAE_MENU_ALIGN);
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

void MSAEditor::addLoadMenu( QMenu* m ) {
    QMenu* lsm = m->addMenu(tr("Add"));
    lsm->menuAction()->setObjectName(MSAE_MENU_LOAD);
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

    QString objName = "msa_editor_" + msaObject->getGObjectName();
    ui->setObjectName(objName);

    connect(ui , SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sl_onContextMenuRequested(const QPoint &)));
    saveScreenshotAction = new QAction(QIcon(":/core/images/cam2.png"), tr("Export as image"), this);
    saveScreenshotAction->setObjectName("Export as image");
    connect(saveScreenshotAction, SIGNAL(triggered()), ui, SLOT(sl_saveScreenshot()));

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

    showOverviewAction = new QAction(QIcon(":/core/images/msa_show_overview.png"), tr("Overview"), this);
    showOverviewAction->setObjectName("Show overview");
    showOverviewAction->setCheckable(true);
    showOverviewAction->setChecked(true);
    connect(showOverviewAction, SIGNAL(triggered()), ui->getOverviewArea(), SLOT(sl_show()));

    // SANGER_TODO: set new proper icon
    showChromatogramsAction = new QAction(QIcon(":/core/images/graphs.png"), tr("Show/hide chromatogram(s)"), this);
    showChromatogramsAction->setObjectName("chromatograms");
    showChromatogramsAction->setCheckable(true);
    showChromatogramsAction->setChecked(showChromatograms);
    connect(showChromatogramsAction, SIGNAL(triggered(bool)), SLOT(sl_showHideChromatograms(bool)));

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
    zoomInAction->setEnabled(font.pointSize() < MOBJECT_MAX_FONT_SIZE);
    zoomOutAction->setEnabled( getColumnWidth() > MOBJECT_MIN_COLUMN_WIDTH );
    zoomToSelectionAction->setEnabled( font.pointSize() < MOBJECT_MAX_FONT_SIZE);
    changeFontAction->setEnabled( resizeMode == ResizeMode_FontAndContent);
    if(alignSequencesToAlignmentAction != NULL) {
        alignSequencesToAlignmentAction->setEnabled(!msaObject->isStateLocked());
    }
}

void MSAEditor::calcFontPixelToPointSizeCoef() {
    QFontInfo info(font);
    fontPixelToPointSize = (float) info.pixelSize() / (float) info.pointSize();

}

void MSAEditor::copyRowFromSequence(U2SequenceObject *seqObj, U2OpStatus &os) {
    MSAUtils::copyRowFromSequence(msaObject, seqObj, os);
    msaObject->updateCachedMultipleAlignment();
}

void MSAEditor::sl_onSeqOrderChanged(const QStringList& order ){
    if(!msaObject->isStateLocked()) {
        msaObject->sortRowsByList(order);
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
            if (msaObject->isStateLocked()) {
                return false;
            }
            U2SequenceObject* dnaObj = qobject_cast<U2SequenceObject*> (gomd->objPtr.data());
            if (dnaObj != NULL ) {
                if (U2AlphabetUtils::deriveCommonAlphabet(dnaObj->getAlphabet(), msaObject->getAlphabet()) == NULL) {
                    return false;
                }
                if (e->type() == QEvent::DragEnter) {
                    de->acceptProposedAction();
                } else {
                    U2OpStatusImpl os;
                    DNASequence seq = dnaObj->getWholeSequence(os);
                    seq.alphabet = dnaObj->getAlphabet();
                    Task *task = new AddSequenceObjectsToAlignmentTask(msaObject, QList<DNASequence>() << seq);
                    TaskWatchdog::trackResourceExistence(msaObject, task, tr("A problem occurred during adding sequences. The multiple alignment is no more available."));
                    AppContext::getTaskScheduler()->registerTopLevelTask(task);
                }
            }
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
    MultipleSequenceAlignmentObject* msaObject = getMSAObject();
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
        if(object == getMSAObject() || (object->getGObjectType() != GObjectTypes::MULTIPLE_SEQUENCE_ALIGNMENT && object->getGObjectType() != GObjectTypes::SEQUENCE)) {
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
    extractor.setAlphabet(msaObject->getAlphabet());
    extractor.extractSequencesFromObjects(objects);

    if(!extractor.getSequenceRefs().isEmpty()) {
        AlignSequencesToAlignmentTask* task = new AlignSequencesToAlignmentTask(msaObject, extractor);
        TaskWatchdog::trackResourceExistence(msaObject, task, tr("A problem occurred during adding sequences. The multiple alignment is no more available."));
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
        LoadSequencesAndAlignToAlignmentTask * task = new LoadSequencesAndAlignToAlignmentTask(msaObject, urls);
        TaskWatchdog::trackResourceExistence(msaObject, task, tr("A problem occurred during adding sequences. The multiple alignment is no more available."));
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

void MSAEditor::sl_exportHighlighted(){
    QObjectScopedPointer<ExportHighligtingDialogController> d = new ExportHighligtingDialogController(ui, (QWidget*)AppContext::getMainWindow()->getQMainWindow());
    d->exec();
    CHECK(!d.isNull(), );

    if (d->result() == QDialog::Accepted){
        AppContext::getTaskScheduler()->registerTopLevelTask(new ExportHighligtningTask(d.data(), ui->getSequenceArea()));
    }
}

void MSAEditor::sl_lockedStateChanged() {
    updateActions();
}

void MSAEditor::sl_showHideChromatograms(bool show) {
    showChromatograms = show;
    emit si_completeUpdate();
}

//////////////////////////////////////////////////////////////////////////
MSAEditorUI::MSAEditorUI(MSAEditor* editor)
    : MaEditorWgt(editor),
      multiTreeViewer(NULL),
      similarityStatistics(NULL) {
}

void MSAEditorUI::sl_saveScreenshot(){
    MSAImageExportController controller(this);
    QWidget *p = (QWidget*)AppContext::getMainWindow()->getQMainWindow();
    QObjectScopedPointer<ExportImageDialog> dlg = new ExportImageDialog(&controller, ExportImageDialog::MSA, ExportImageDialog::NoScaling, p);
    dlg->exec();
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
    if(NULL == multiTreeViewer) {
        // SANGER_TODO: avoid qobject_cast???
        multiTreeViewer = new MSAEditorMultiTreeViewer(tr("Tree view"), qobject_cast<MSAEditor*>(editor));
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
        settings.ma = editor->getMSAObject();
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
    }
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
