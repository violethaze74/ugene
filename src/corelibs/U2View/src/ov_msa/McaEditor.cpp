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

#include <QToolBar>

#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/OptionsPanel.h>
#include <U2Gui/OPWidgetFactoryRegistry.h>

#include "MaConsensusMismatchController.h"
#include "MaEditorFactory.h"
#include "MaEditorNameList.h"
#include "McaEditor.h"
#include "McaEditorConsensusArea.h"
#include "McaEditorSequenceArea.h"
#include "ExportConsensus/MaExportConsensusTabFactory.h"
#include "General/McaGeneralTabFactory.h"
#include "helpers/MaAmbiguousCharactersController.h"
#include "ov_sequence/SequenceObjectContext.h"
#include "view_rendering/SequenceWithChromatogramAreaRenderer.h"

namespace U2 {

McaEditor::McaEditor(const QString &viewName,
                     MultipleChromatogramAlignmentObject *obj)
    : MaEditor(McaEditorFactory::ID, viewName, obj),
      referenceCtx(NULL)
{
    U2OpStatusImpl os;
    foreach (const MultipleChromatogramAlignmentRow& row, obj->getMca()->getMcaRows()) {
        chromVisibility.insert(obj->getMca()->getRowIndexByRowId(row->getRowId(), os), true);
    }

    U2SequenceObject* referenceObj = obj->getReferenceObj();
    SAFE_POINT(NULL != referenceObj, "Trying to open McaEditor without a reference", );
    referenceCtx = new SequenceObjectContext(referenceObj, this);
}

MultipleChromatogramAlignmentObject *McaEditor::getMaObject() const {
    return qobject_cast<MultipleChromatogramAlignmentObject*>(maObject);
}

McaEditorWgt *McaEditor::getUI() const {
    return qobject_cast<McaEditorWgt *>(ui);
}

void McaEditor::buildStaticToolbar(QToolBar* tb) {
    tb->addAction(showChromatogramsAction);
    tb->addAction(showOverviewAction);
    tb->addSeparator();

    tb->addAction(zoomInAction);
    tb->addAction(zoomOutAction);
    tb->addAction(zoomToSelectionAction);
    tb->addAction(resetZoomAction);
    tb->addSeparator();

    GObjectView::buildStaticToolbar(tb);
}

void McaEditor::buildStaticMenu(QMenu* menu) {
    addAlignmentMenu(menu);
    addAppearanceMenu(menu);
    menu->addSeparator();
    menu->addAction(getUI()->getClearSelectionAction());
    menu->addSeparator();
    addNavigationMenu(menu);
    addEditingMenu(menu);
    menu->addSeparator();
    menu->addAction(showConsensusTabAction);

    GObjectView::buildStaticMenu(menu);
    GUIUtils::disableEmptySubmenus(menu);
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

void McaEditor::sl_onContextMenuRequested(const QPoint & /*pos*/) {
    QMenu menu;
    buildStaticMenu(&menu);
    emit si_buildPopupMenu(this, &menu);
    menu.exec(QCursor::pos());
}

void McaEditor::sl_showHideChromatograms(bool show) {
    ui->getCollapseModel()->collapseAll(!show);
    emit si_completeUpdate();
}

void McaEditor::sl_showGeneralTab() {
    OptionsPanel* optionsPanel = getOptionsPanel();
    SAFE_POINT(NULL != optionsPanel, "Internal error: options panel is NULL"
        " when msageneraltab opening was initiated", );
    optionsPanel->openGroupById(McaGeneralTabFactory::getGroupId());
}

void McaEditor::sl_showConsensusTab() {
    OptionsPanel* optionsPanel = getOptionsPanel();
    SAFE_POINT(NULL != optionsPanel, "Internal error: options panel is NULL"
        " when msaconsensustab opening was initiated", );
    optionsPanel->openGroupById(McaExportConsensusTabFactory::getGroupId());
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

void McaEditor::initActions() {
    MaEditor::initActions();

    zoomInAction->setText(tr("Zoom in"));
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    ui->addAction(zoomInAction);

    zoomOutAction->setText(tr("Zoom out"));
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    ui->addAction(zoomOutAction);

    resetZoomAction->setText(tr("Reset zoom"));
    resetZoomAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_0));
    ui->addAction(resetZoomAction);

    showChromatogramsAction = new QAction(QIcon(":/core/images/graphs.png"), tr("Show/hide chromatograms"), this);
    showChromatogramsAction->setObjectName("chromatograms");
    showChromatogramsAction->setCheckable(true);
    showChromatogramsAction->setChecked(true);
    connect(showChromatogramsAction, SIGNAL(triggered(bool)), SLOT(sl_showHideChromatograms(bool)));
    ui->addAction(showChromatogramsAction);

    showGeneralTabAction = new QAction(tr("Open \"General\" tab on options panel"), this);
    connect(showGeneralTabAction, SIGNAL(triggered()), SLOT(sl_showGeneralTab()));
    ui->addAction(showGeneralTabAction);

    showConsensusTabAction = new QAction(tr("Open \"Consensus\" tab on options panel"), this);
    connect(showConsensusTabAction, SIGNAL(triggered()), SLOT(sl_showConsensusTab()));
    ui->addAction(showConsensusTabAction);

    showOverviewAction->setText(tr("Show overview"));
    changeFontAction->setText(tr("Change characters font"));
}

void McaEditor::addAlignmentMenu(QMenu *menu) {
    QMenu *alignmentMenu = menu->addMenu(tr("Alignment"));
    alignmentMenu->menuAction()->setObjectName(MCAE_MENU_ALIGNMENT);

    alignmentMenu->addAction(showGeneralTabAction);
}

void McaEditor::addAppearanceMenu(QMenu *menu) {
    QMenu* appearanceMenu = menu->addMenu(tr("Appearance"));
    appearanceMenu->menuAction()->setObjectName(MCAE_MENU_APPEARANCE);

    appearanceMenu->addAction(showChromatogramsAction);
    appearanceMenu->addMenu(getUI()->getSequenceArea()->getTraceActionsMenu());
    appearanceMenu->addAction(showOverviewAction);
    appearanceMenu->addSeparator();
    appearanceMenu->addAction(zoomInAction);
    appearanceMenu->addAction(zoomOutAction);
    appearanceMenu->addAction(resetZoomAction);
    appearanceMenu->addSeparator();
    appearanceMenu->addAction(getUI()->getSequenceArea()->getIncreasePeaksHeightAction());
    appearanceMenu->addAction(getUI()->getSequenceArea()->getDecreasePeaksHeightAction());
    appearanceMenu->addSeparator();
    appearanceMenu->addAction(changeFontAction);
}

void McaEditor::addNavigationMenu(QMenu *menu) {
    QMenu *navigationMenu = menu->addMenu(tr("Navigation"));
    navigationMenu->menuAction()->setObjectName(MCAE_MENU_NAVIGATION);

    navigationMenu->addAction(getUI()->getSequenceArea()->getAmbiguousCharactersController()->getPreviousAction());
    navigationMenu->addAction(getUI()->getSequenceArea()->getAmbiguousCharactersController()->getNextAction());
    navigationMenu->addSeparator();
    navigationMenu->addAction(getUI()->getConsensusArea()->getMismatchController()->getPrevMismatchAction());
    navigationMenu->addAction(getUI()->getConsensusArea()->getMismatchController()->getNextMismatchAction());
}

void McaEditor::addEditingMenu(QMenu* menu) {
    QMenu* editingMenu = menu->addMenu(tr("Editing"));
    editingMenu->menuAction()->setObjectName(MCAE_MENU_EDITING);

    editingMenu->addAction(getUI()->getSequenceArea()->getInsertAction());
    editingMenu->addAction(getUI()->getSequenceArea()->getReplaceCharacterAction());
    editingMenu->addAction(getUI()->getDelSelectionAction());
    editingMenu->addSeparator();
    editingMenu->addAction(getUI()->getSequenceArea()->getInsertGapAction());
    editingMenu->addAction(getUI()->getSequenceArea()->getRemoveGapBeforeSelectionAction());
    editingMenu->addAction(getUI()->getSequenceArea()->getRemoveColumnsOfGapsAction());
    editingMenu->addAction(getUI()->getSequenceArea()->getTrimLeftEndAction());
    editingMenu->addAction(getUI()->getSequenceArea()->getTrimRightEndAction());
    editingMenu->addSeparator();
    editingMenu->addAction(getUI()->getEditorNameList()->getEditSequenceNameAction());
    editingMenu->addAction(getUI()->getEditorNameList()->getRemoveSequenceAction());
    editingMenu->addSeparator();
    editingMenu->addAction(getUI()->getUndoAction());
    editingMenu->addAction(getUI()->getRedoAction());
}

}   // namespace U2
