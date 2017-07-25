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

#include <QToolButton>

#include <U2Algorithm/MsaColorScheme.h>
#include <U2Algorithm/MsaHighlightingScheme.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequenceUtils.h>
#include <U2Core/U2Mod.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/GUIUtils.h>

#include "McaEditorSequenceArea.h"
#include "helpers/MaAmbiguousCharactersController.h"
#include "helpers/ScrollController.h"
#include "helpers/RowHeightController.h"
#include "ov_sequence/SequenceObjectContext.h"
#include "view_rendering/SequenceWithChromatogramAreaRenderer.h"

namespace U2 {

McaEditorSequenceArea::McaEditorSequenceArea(McaEditorWgt *ui, GScrollBar *hb, GScrollBar *vb)
    : MaEditorSequenceArea(ui, hb, vb) {
    initRenderer();

    connect(ui, SIGNAL(si_clearSelection()), SLOT(sl_cancelSelection()));

    // TEST - remove the variable after fix
    editingEnabled = true;

    showQVAction = new QAction(tr("Show quality bars"), this);
    showQVAction->setIcon(QIcon(":chroma_view/images/bars.png"));
    showQVAction->setCheckable(true);
    // SANGER_TODO: check quality
//    showQVAction->setChecked(chroma.hasQV);
//    showQVAction->setEnabled(chroma.hasQV);
    connect(showQVAction, SIGNAL(toggled(bool)), SLOT(sl_completeUpdate()));

    showAllTraces = new QAction(tr("Show all"), this);
    connect(showAllTraces, SIGNAL(triggered()), SLOT(sl_showAllTraces()));

    traceActionsMenu = new QMenu(tr("Show/hide trace"), this);
    traceActionsMenu->setObjectName("traceActionsMenu");
    traceActionsMenu->addAction( createToggleTraceAction("A") );
    traceActionsMenu->addAction( createToggleTraceAction("C") );
    traceActionsMenu->addAction( createToggleTraceAction("G") );
    traceActionsMenu->addAction( createToggleTraceAction("T") ) ;
    traceActionsMenu->addSeparator();
    traceActionsMenu->addAction(showAllTraces);

    insertAction = new QAction(tr("Add insertion"), this); // 5491_TODO: the text should be meaningfull
    insertAction->setObjectName("add_insertion");
    insertAction->setShortcut(Qt::SHIFT + Qt::Key_I);
    connect(insertAction, SIGNAL(triggered()), SLOT(sl_addInsertion()));
    addAction(insertAction);

    removeGapBeforeSelectionAction = new QAction(tr("Shift characters left"), this);
    removeGapBeforeSelectionAction->setShortcut(Qt::Key_Backspace);
    connect(removeGapBeforeSelectionAction, SIGNAL(triggered()), SLOT(sl_removeGapBeforeSelection()));
    addAction(removeGapBeforeSelectionAction);

    removeColumnsOfGapsAction = new QAction(tr("Remove columns of gaps"), this);
    removeColumnsOfGapsAction->setObjectName("remove_columns_of_gaps");
    removeColumnsOfGapsAction->setShortcut(Qt::SHIFT + Qt::Key_Delete);
    connect(removeColumnsOfGapsAction, SIGNAL(triggered()), SLOT(sl_removeColumnsOfGaps()));
    addAction(removeColumnsOfGapsAction);

    fillWithGapsinsSymAction->setText(tr("Insert gap"));
    fillWithGapsinsSymAction->setShortcut(Qt::Key_Space);
    fillWithGapsinsSymAction->setShortcutContext(Qt::WidgetShortcut);

    scaleBar = new ScaleBar(Qt::Horizontal);
    scaleBar->setRange(100, 1000);
    scaleBar->setTickInterval(100);
    scaleBar->getPlusAction()->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Up));
    scaleBar->getMinusAction()->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Down));
    addAction(scaleBar->getPlusAction());
    addAction(scaleBar->getMinusAction());
    scaleAction = NULL;

    ambiguousCharactersController = new MaAmbiguousCharactersController(ui);
    addAction(ambiguousCharactersController->getPreviousAction());
    addAction(ambiguousCharactersController->getNextAction());

    SequenceWithChromatogramAreaRenderer* r = qobject_cast<SequenceWithChromatogramAreaRenderer*>(renderer);
    scaleBar->setValue(r->getScaleBarValue());
    connect(scaleBar, SIGNAL(valueChanged(int)), SLOT(sl_setRenderAreaHeight(int)));

    updateColorAndHighlightSchemes();
    updateActions();
}

void McaEditorSequenceArea::adjustReferenceLength(U2OpStatus& os) {
    McaEditor* mcaEditor = getEditor();
    qint64 newLength = mcaEditor->getMaObject()->getLength();
    qint64 currentLength = mcaEditor->getReferenceContext()->getSequenceLength();
    if (newLength > currentLength) {
        U2DataId id = mcaEditor->getMaObject()->getEntityRef().entityId;
        U2Region region(currentLength, 0);
        QByteArray insert(newLength - currentLength, U2Msa::GAP_CHAR);
        DNASequence seq(insert);
        mcaEditor->getReferenceContext()->getSequenceObject()->replaceRegion(id, region, seq, os);
    }
}

const MaAmbiguousCharactersController * const McaEditorSequenceArea::getAmbiguousCharactersController() const {
    return ambiguousCharactersController;
}

QMenu *McaEditorSequenceArea::getTraceActionsMenu() const {
    return traceActionsMenu;
}

QAction *McaEditorSequenceArea::getIncreasePeaksHeightAction() const {
    return scaleBar->getPlusAction();
}

QAction *McaEditorSequenceArea::getDecreasePeaksHeightAction() const {
    return scaleBar->getMinusAction();
}

QAction *McaEditorSequenceArea::getInsertAction() const {
    return insertAction;
}

QAction *McaEditorSequenceArea::getInsertGapAction() const {
    return fillWithGapsinsSymAction;
}

QAction *McaEditorSequenceArea::getRemoveGapBeforeSelectionAction() const {
    return removeGapBeforeSelectionAction;
}

QAction *McaEditorSequenceArea::getRemoveColumnsOfGapsAction() const {
    return removeColumnsOfGapsAction;
}

void McaEditorSequenceArea::setSelection(const MaEditorSelection &sel, bool newHighlightSelection) {
    if (sel.height() > 1 || sel.width() > 1) {
        // ignore multi-selection
        return;
    }

    if (getEditor()->getMaObject()->getMca()->isTrailingOrLeadingGap(sel.y(), sel.x())) {
        // clear selection
        emit si_clearReferenceSelection();
        MaEditorSequenceArea::setSelection(MaEditorSelection(), newHighlightSelection);
        return;
    }
    MaEditorSequenceArea::setSelection(sel, newHighlightSelection);
}

void McaEditorSequenceArea::moveSelection(int dx, int dy, bool) {
    CHECK(selection.width() == 1 && selection.height() == 1, );

    const MultipleChromatogramAlignment mca = getEditor()->getMaObject()->getMca();
    if (dy == 0 && mca->isTrailingOrLeadingGap(selection.y(), selection.x() + dx)) {
        return;
    }

    int nextRowToSelect = selection.y() + dy;
    if (dy != 0) {
        bool noRowAvailabe = true;
        for ( ; nextRowToSelect >= 0 && nextRowToSelect < ui->getCollapseModel()->getDisplayableRowsCount(); nextRowToSelect += dy) {
            if (!mca->isTrailingOrLeadingGap(ui->getCollapseModel()->mapToRow(nextRowToSelect), selection.x() + dx)) {
                noRowAvailabe  = false;
                break;
            }
        }
        CHECK(!noRowAvailabe, );
    }

    QPoint newSelectedPoint(selection.x() + dx, nextRowToSelect);
    MaEditorSelection newSelection(newSelectedPoint, selection.width(), selection.height());
    setSelection(newSelection);
    ui->getScrollController()->scrollToMovedSelection(dx, dy);
}

void McaEditorSequenceArea::sl_backgroundSelectionChanged() {
    update();
}

void McaEditorSequenceArea::sl_alignmentChanged(const MultipleAlignment &ma, const MaModificationInfo &modInfo) {
    getEditor()->getReferenceContext()->getSequenceObject()->forceCachedSequenceUpdate();
    MaEditorSequenceArea::sl_alignmentChanged(ma, modInfo);
}

void McaEditorSequenceArea::sl_showHideTrace() {
    QAction* traceAction = qobject_cast<QAction*> (sender());

    if (!traceAction) {
        return;
    }

    if (traceAction->text() == "A") {
        settings.drawTraceA = traceAction->isChecked();
    } else if (traceAction->text() == "C") {
        settings.drawTraceC = traceAction->isChecked();
    } else if(traceAction->text() == "G") {
        settings.drawTraceG = traceAction->isChecked();
    } else if(traceAction->text() == "T") {
        settings.drawTraceT = traceAction->isChecked();
    } else {
        assert(0);
    }

    sl_completeUpdate();
}

void McaEditorSequenceArea::sl_showAllTraces() {
    settings.drawTraceA = true;
    settings.drawTraceC = true;
    settings.drawTraceG = true;
    settings.drawTraceT = true;
    QList<QAction*> actions = traceActionsMenu->actions();
    foreach(QAction* action, actions) {
        action->setChecked(true);
    }
    sl_completeUpdate();
}

void McaEditorSequenceArea::sl_setRenderAreaHeight(int k) {
    //k = chromaMax
    SequenceWithChromatogramAreaRenderer* r = qobject_cast<SequenceWithChromatogramAreaRenderer*>(renderer);
    r->setAreaHeight(k);
    sl_completeUpdate();
}

void McaEditorSequenceArea::sl_buildStaticToolbar(GObjectView *, QToolBar *t) {
    QToolButton* button = new QToolButton();
    button->setMenu(traceActionsMenu);
    button->setIcon(QIcon(":chroma_view/images/traces.png"));
    button->setPopupMode(QToolButton::InstantPopup);
    t->addWidget(button);
    t->addSeparator();

    if (scaleAction != NULL) {
        t->addAction(scaleAction);
    } else {
        scaleAction = t->addWidget(scaleBar);
    }
    t->addSeparator();

    t->addAction(ui->getUndoAction());
    t->addAction(ui->getRedoAction());

    t->addSeparator();

    t->addAction(ambiguousCharactersController->getPreviousAction());
    t->addAction(ambiguousCharactersController->getNextAction());
}

void McaEditorSequenceArea::sl_addInsertion() {
    maMode = InsertCharMode;

    editModeAnimationTimer.start(500);
    highlightCurrentSelection();
}

void McaEditorSequenceArea::sl_removeGapBeforeSelection() {
    emit si_startMaChanging();
    removeGapsPrecedingSelection(1);
    emit si_stopMaChanging(true);
}

void McaEditorSequenceArea::sl_removeColumnsOfGaps() {
    U2OpStatus2Log os;
    U2UseCommonUserModStep userModStep(editor->getMaObject()->getEntityRef(), os);
    Q_UNUSED(userModStep);
    SAFE_POINT_OP(os, );
    editor->getMaObject()->deleteColumnsWithGaps(os);
}

void McaEditorSequenceArea::initRenderer() {
    renderer = new SequenceWithChromatogramAreaRenderer(ui, this);
}

void McaEditorSequenceArea::updateActions() {
    MultipleAlignmentObject* maObj = editor->getMaObject();
    SAFE_POINT(NULL != maObj, "MaObj is NULL", );

    const bool readOnly = maObj->isStateLocked();
    const bool canEditAlignment = !readOnly && !isAlignmentEmpty();
    const bool canEditSelectedArea = canEditAlignment && !selection.isNull();
    const bool isSingleSymbolSelected = (selection.getRect().size() == QSize(1, 1));
    const bool hasGapBeforeSelection = canEditAlignment && (maObj->getMultipleAlignment()->isGap(selection.y(), selection.x() - 1));

    ui->getDelSelectionAction()->setEnabled(canEditSelectedArea);
    insertAction->setEnabled(canEditSelectedArea && isSingleSymbolSelected);
    replaceCharacterAction->setEnabled(canEditSelectedArea && isSingleSymbolSelected);
    fillWithGapsinsSymAction->setEnabled(canEditSelectedArea && isSingleSymbolSelected);
    removeGapBeforeSelectionAction->setEnabled(hasGapBeforeSelection);
}

void McaEditorSequenceArea::drawBackground(QPainter &painter) {
    SequenceWithChromatogramAreaRenderer* r = qobject_cast<SequenceWithChromatogramAreaRenderer*>(renderer);
    SAFE_POINT(r != NULL, "Wrong renderer: fail to cast renderer to SequenceWithChromatogramAreaRenderer", );
    r->drawReferenceSelection(painter);
    r->drawNameListSelection(painter);
}

void McaEditorSequenceArea::getColorAndHighlightingIds(QString &csid, QString &hsid) {
    csid = MsaColorScheme::UGENE_SANGER_NUCL;
    hsid = MsaHighlightingScheme::DISAGREEMENTS;
}

QAction* McaEditorSequenceArea::createToggleTraceAction(const QString& actionName) {
    QAction* showTraceAction = new QAction(actionName, this);
    showTraceAction->setCheckable(true);
    showTraceAction->setChecked(true);
    showTraceAction->setEnabled(true);
    connect(showTraceAction, SIGNAL(triggered(bool)), SLOT(sl_showHideTrace()));

    return showTraceAction;
}

void McaEditorSequenceArea::insertChar(char newCharacter) {
    CHECK(maMode == InsertCharMode, );
    CHECK(getEditor() != NULL, );
    CHECK(!selection.isNull(), );

    assert(isInRange(selection.topLeft()));
    assert(isInRange(QPoint(selection.x() + selection.width() - 1, selection.y() + selection.height() - 1)));

    MultipleChromatogramAlignmentObject* maObj = getEditor()->getMaObject();
    CHECK(maObj != NULL && !maObj->isStateLocked(), );

    // if this method was invoked during a region shifting
    // then shifting should be canceled
    cancelShiftTracking();

    U2OpStatusImpl os;
    U2UseCommonUserModStep userModStep(maObj->getEntityRef(), os);
    Q_UNUSED(userModStep);
    SAFE_POINT_OP(os, );

    maObj->enlargeLength(os, maObj->getLength() + 1);
    maObj->insertCharacter(selection.y(), selection.x(), newCharacter);

    // insert char into the reference
    U2SequenceObject* ref = getEditor()->getMaObject()->getReferenceObj();
    U2Region region = U2Region(selection.x(), 0);
    ref->replaceRegion(maObj->getEntityRef().entityId, region, DNASequence(QByteArray(1, U2Msa::GAP_CHAR)), os);
    SAFE_POINT_OP(os, );

    exitFromEditCharacterMode();
}

McaEditorWgt *McaEditorSequenceArea::getMcaEditorWgt() const {
    return qobject_cast<McaEditorWgt *>(ui);
}

} // namespace
