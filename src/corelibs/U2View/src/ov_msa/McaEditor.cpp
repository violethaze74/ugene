/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
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
#include <QToolBar>

#include <U2Algorithm/BuiltInConsensusAlgorithms.h>
#include <U2Algorithm/MSAConsensusAlgorithm.h>
#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>
#include <U2Algorithm/MsaHighlightingScheme.h>

#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/OptionsPanel.h>
#include <U2Gui/OPWidgetFactoryRegistry.h>

#include "MaConsensusMismatchController.h"
#include "MaEditorConsensusAreaSettings.h"
#include "MaEditorFactory.h"
#include "MaEditorNameList.h"
#include "McaEditor.h"
#include "McaEditorOverviewArea.h"
#include "McaEditorReferenceArea.h"
#include "McaEditorSequenceArea.h"
#include "McaReferenceCharController.h"
#include "MSAEditorConsensusArea.h"
#include "helpers/McaRowHeightController.h"
#include "ov_sequence/SequenceObjectContext.h"
#include "view_rendering/MaEditorWgt.h"
#include "view_rendering/SequenceWithChromatogramAreaRenderer.h"

namespace U2 {

McaEditor::McaEditor(const QString &viewName,
                     MultipleChromatogramAlignmentObject *obj)
    : MaEditor(McaEditorFactory::ID, viewName, obj),
      referenceCtx(NULL)
{

    // SANGER_TODO: set new proper icon
    showChromatogramsAction = new QAction(QIcon(":/core/images/graphs.png"), tr("Show/hide chromatogram(s)"), this);
    showChromatogramsAction->setObjectName("chromatograms");
    showChromatogramsAction->setCheckable(true);
    showChromatogramsAction->setChecked(true);
    connect(showChromatogramsAction, SIGNAL(triggered(bool)), SLOT(sl_showHideChromatograms(bool)));

    U2OpStatusImpl os;
    foreach (const MultipleChromatogramAlignmentRow& row, obj->getMca()->getMcaRows()) {
        chromVisibility.insert(obj->getMca()->getRowIndexByRowId(row->getRowId(), os), true);
    }

    U2SequenceObject* referenceObj = obj->getReferenceObj();
    if (referenceObj) {
        // SANGER_TODO: probably can be big
        referenceCtx = new SequenceObjectContext(referenceObj, this);
    } else {
        FAIL("Trying to open McaEditor without a reference", );
    }
}

void McaEditor::buildStaticToolbar(QToolBar* tb) {
    tb->addAction(zoomInAction);
    tb->addAction(zoomOutAction);
    tb->addAction(zoomToSelectionAction);
    tb->addAction(resetZoomAction);

    tb->addAction(showOverviewAction);
    tb->addAction(showChromatogramsAction);
    tb->addAction(changeFontAction);

    GObjectView::buildStaticToolbar(tb);
}

void McaEditor::buildStaticMenu(QMenu* m) {
    // SANGER_TODO: review the menus and toolbar
//    MaEditor::buildStaticMenu(m);
}

int McaEditor::getRowContentIndent(int rowId) const {
    if (isChromVisible(rowId)) {
        return SequenceWithChromatogramAreaRenderer::INDENT_BETWEEN_ROWS / 2;
    }
    return MaEditor::getRowContentIndent(rowId);
}

bool McaEditor::isChromVisible(qint64 rowId) const {
    return isChromVisible(getMaObject()->getRowPosById(rowId));
}

bool McaEditor::isChromVisible(int rowIndex) const {
    return !ui->getCollapseModel()->isItemCollapsed(rowIndex);
}

QString McaEditor::getReferenceRowName() const {
    return getMaObject()->getReferenceObj()->getSequenceName();
}

char McaEditor::getReferenceCharAt(int pos) const {
    U2OpStatus2Log os;
    SAFE_POINT(getMaObject()->getReferenceObj()->getSequenceLength() > pos, "Invalid position", '\n');
    QByteArray seqData = getMaObject()->getReferenceObj()->getSequenceData(U2Region(pos, 1), os);
    CHECK_OP(os, U2Msa::GAP_CHAR);
    return seqData.isEmpty() ? U2Msa::GAP_CHAR : seqData.at(0);
}

SequenceObjectContext* McaEditor::getReferenceContext() const {
    return referenceCtx;
}

void McaEditor::sl_onContextMenuRequested(const QPoint & pos) {
    Q_UNUSED(pos);

    if (ui->childAt(pos) != NULL) {
        // ignore context menu request if overview area was clicked on
        if (ui->getOverviewArea()->isOverviewWidget(ui->childAt(pos))) {
            return;
        }
    }

    QMenu m;

    addCopyMenu(&m);
    addViewMenu(&m);
    addEditMenu(&m);
    addExportMenu(&m);

    m.addSeparator();

    emit si_buildPopupMenu(this, &m);

    GUIUtils::disableEmptySubmenus(&m);

    m.exec(QCursor::pos());
}

void McaEditor::sl_showHideChromatograms(bool show) {
    ui->getCollapseModel()->collapseAll(!show);
    emit si_completeUpdate();
}

QWidget* McaEditor::createWidget() {
    Q_ASSERT(ui == NULL);
    ui = new McaEditorWgt(this);

    QString objName = "mca_editor_" + maObject->getGObjectName();
    ui->setObjectName(objName);

    connect(ui , SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sl_onContextMenuRequested(const QPoint &)));

    initActions();

    optionsPanel = new OptionsPanel(this);
    OPWidgetFactoryRegistry *opWidgetFactoryRegistry = AppContext::getOPWidgetFactoryRegistry();

    QList<OPFactoryFilterVisitorInterface*> filters;
    filters.append(new OPFactoryFilterVisitor(ObjViewType_ChromAlignmentEditor));

    QList<OPWidgetFactory*> opWidgetFactories = opWidgetFactoryRegistry->getRegisteredFactories(filters);
    foreach (OPWidgetFactory *factory, opWidgetFactories) {
        optionsPanel->addGroup(factory);
    }

    qDeleteAll(filters);

    return ui;
}

McaEditorWgt::McaEditorWgt(McaEditor *editor)
    : MaEditorWgt(editor)
{
    rowHeightController = new McaRowHeightController(this);
    refCharController = new McaReferenceCharController(this, editor);

    initActions();
    initWidgets();

    refArea = new McaEditorReferenceArea(this, getEditor()->getReferenceContext());
    refArea->installEventFilter(this);
    seqAreaHeaderLayout->insertWidget(0, refArea);

    MaEditorConsensusAreaSettings consSettings;
    consSettings.visibleElements = MSAEditorConsElement_CONSENSUS_TEXT | MSAEditorConsElement_RULER;
    consSettings.highlightMismatches = true;
    consArea->setDrawSettings(consSettings);

    MSAConsensusAlgorithmFactory* algoFactory = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(BuiltInConsensusAlgorithms::LEVITSKY_ALGO);
    consArea->setConsensusAlgorithm(algoFactory);

    QString name = getEditor()->getReferenceContext()->getSequenceObject()->getSequenceName();
    QWidget *refName = createHeaderLabelWidget(name, Qt::AlignCenter, refArea);

    nameAreaLayout->insertWidget(0, refName);

    QVector<U2Region> itemRegions;
    for (int i = 0; i < editor->getNumSequences(); i++) {
        itemRegions << U2Region(i, 1);
    }

    collapseModel->setTrivialGroupsPolicy(MSACollapsibleItemModel::Allow);
    collapseModel->reset(itemRegions);
    collapseModel->collapseAll(false);
    collapsibleMode = true;

    McaEditorConsensusArea* mcaConsArea = qobject_cast<McaEditorConsensusArea*>(consArea);
    SAFE_POINT(mcaConsArea != NULL, "Failed to cast consensus area to MCA consensus area", );
    connect(mcaConsArea->getMismatchController(), SIGNAL(si_selectMismatch(int)), refArea, SLOT(sl_selectMismatch(int)));
}

McaEditorSequenceArea* McaEditorWgt::getSequenceArea() const {
    return qobject_cast<McaEditorSequenceArea*>(seqArea);
}

bool McaEditorWgt::eventFilter(QObject *watched, QEvent *event) {
    if (watched == refArea) {
        QApplication::sendEvent(seqArea, event);
        if (event->type() == QEvent::KeyPress) {
            return true;
        }
        return false;
    }
    return MaEditorWgt::eventFilter(watched, event);
}

void McaEditorWgt::initSeqArea(GScrollBar* shBar, GScrollBar* cvBar) {
    seqArea = new McaEditorSequenceArea(this, shBar, cvBar);
}

void McaEditorWgt::initOverviewArea() {
    overviewArea = new McaEditorOverviewArea(this);
}

void McaEditorWgt::initNameList(QScrollBar* nhBar) {
    nameList = new McaEditorNameList(this, nhBar);
}

void McaEditorWgt::initConsensusArea() {
    consArea = new McaEditorConsensusArea(this);
}

} // namespace
