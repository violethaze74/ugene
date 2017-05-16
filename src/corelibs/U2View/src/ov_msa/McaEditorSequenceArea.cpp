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

#include "McaEditorSequenceArea.h"
#include "ov_sequence/SequenceObjectContext.h"

#include "view_rendering/SequenceWithChromatogramAreaRenderer.h"

#include <U2Algorithm/MsaColorScheme.h>
#include <U2Algorithm/MsaHighlightingScheme.h>

#include <U2Core/DNASequenceUtils.h>
#include <U2Core/U2Mod.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/GUIUtils.h>

#include <QToolButton>

namespace U2 {

McaEditorSequenceArea::McaEditorSequenceArea(MaEditorWgt *ui, GScrollBar *hb, GScrollBar *vb)
    : MaEditorSequenceArea(ui, hb, vb) {
    initRenderer();

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

    traceActionMenu = new QMenu(tr("Show/hide trace"), this);
    traceActionMenu->addAction( createToggleTraceAction("A") );
    traceActionMenu->addAction( createToggleTraceAction("C") );
    traceActionMenu->addAction( createToggleTraceAction("G") );
    traceActionMenu->addAction( createToggleTraceAction("T") ) ;
    traceActionMenu->addSeparator();
    traceActionMenu->addAction(showAllTraces);

    insertAction = new QAction(tr("Add insertion"), this); // 5491_TODO: the text should be meaningfull
    insertAction->setObjectName("add_insertion");
    // 5491_TODO: add shortcut
    connect(insertAction, SIGNAL(triggered()), SLOT(sl_addInsertion()));
    addAction(insertAction);

    scaleBar = new ScaleBar(Qt::Horizontal);
    scaleBar->slider()->setRange(100, 1000);
    scaleBar->slider()->setTickInterval(100);
    scaleAction = NULL;

    SequenceWithChromatogramAreaRenderer* r = qobject_cast<SequenceWithChromatogramAreaRenderer*>(renderer);
    scaleBar->setValue(r->getScaleBarValue());
    connect(scaleBar, SIGNAL(valueChanged(int)), SLOT(sl_setRenderAreaHeight(int)));

    updateColorAndHighlightSchemes();
    updateActions();
}

U2Region McaEditorSequenceArea::getSequenceYRange(int seq, int firstVisibleRow, bool useVirtualCoords) const {
    int start = 0;

    // temporarry fix -- should be rewritten according to the solution of UGENE-5479 (smooth scrolling)
    bool positiveDirection = seq > firstVisibleRow;
    int i = firstVisibleRow;
    while (i != seq) {
        if (getEditor()->isChromVisible(i)) {
            start += editor->getRowHeight();
        } else {
            start += editor->getSequenceRowHeight();
        }

        if (positiveDirection ) {
            i++;
        } else {
            i--;
        }
    }
    if (!positiveDirection ) {
        start *= -1;
    }

    U2Region res(start, getEditor()->isChromVisible(seq) ? editor->getRowHeight() : editor->getSequenceRowHeight());
    if (!useVirtualCoords) {
        int h = height();
        res = res.intersect(U2Region(0, h));
    }
    return res;
}

int McaEditorSequenceArea::getSequenceNumByY(int y) const {
    int seqNum = startSeq;
    U2Region r;
    do {
        r = MaEditorSequenceArea::getSequenceYRange(seqNum, true);
        seqNum++;
    } while (!r.contains(y));

    return seqNum - 1;
}

U2Region McaEditorSequenceArea::getSequenceYRange(int startSeq, int count) const {
    int len = 0;
    for (int i = startSeq; i < startSeq + count; i++) {
        if (getEditor()->isChromVisible(i)) {
            len += editor->getRowHeight();
        } else {
            len += editor->getSequenceRowHeight();
        }
    }
    U2Region res(MaEditorSequenceArea::getSequenceYRange(startSeq, true).startPos, len);
    return res;
}

int McaEditorSequenceArea::countHeightForSequences(bool countClipped) const {
    int seqAreaHeight = height();
    int nVisible = 0;
    int  i = startSeq;
    while (seqAreaHeight > 0) {
        seqAreaHeight -= getEditor()->isChromVisible(i) ? editor->getRowHeight()
                                                        : editor->getSequenceRowHeight();
        nVisible++;
        i++;
    }
    return nVisible;
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

    const MultipleChromatogramAlignment& mca = getEditor()->getMaObject()->getMca();
    if (dy == 0 && mca->isTrailingOrLeadingGap(selection.y(), selection.x() + dx)) {
        return;
    }

    int nextRowToSelect = selection.y() + dy;
    if (dy != 0) {
        bool noRowAvailabe = true;
        for ( ; nextRowToSelect >= 0 && nextRowToSelect < editor->getNumSequences(); nextRowToSelect += dy) {
            if (!mca->isTrailingOrLeadingGap(nextRowToSelect, selection.x() + dx)) {
                noRowAvailabe  = false;
                break;
            }
        }
        CHECK(!noRowAvailabe, );
    }

    QPoint newSelectedPoint(selection.x() + dx, nextRowToSelect);
    MaEditorSelection newSelection(newSelectedPoint, selection.width(), selection.height());
    setSelection(newSelection);
}

void McaEditorSequenceArea::sl_backgroundSelectionChanged() {
    update();
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
    QList<QAction*> actions = traceActionMenu->actions();
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
    t->addAction(showQVAction);

    QToolButton* button = new QToolButton();
    button->setMenu(traceActionMenu);
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
}

void McaEditorSequenceArea::sl_addInsertion() {
    msaMode = InsertCharMode;

    editModeAnimationTimer.start(500);
    highlightCurrentSelection();
}

void McaEditorSequenceArea::initRenderer() {
    renderer = new SequenceWithChromatogramAreaRenderer(this);
}

void McaEditorSequenceArea::updateActions() {
    // 5491_TODO: checkall acions and disable the right ones
    /// add separate methods smt like 'updateEditActions'
    MultipleAlignmentObject* maObj = editor->getMaObject();
    assert(maObj != NULL);
    bool readOnly = maObj->isStateLocked();
    bool canEditAlignment = !readOnly && !isAlignmentEmpty();
    bool canEditSelectedArea = canEditAlignment && !selection.isNull();
    ui->getDelSelectionAction()->setEnabled(canEditSelectedArea);
}

void McaEditorSequenceArea::drawBackground(QPainter& p) {
    SequenceWithChromatogramAreaRenderer* r = qobject_cast<SequenceWithChromatogramAreaRenderer*>(renderer);
    SAFE_POINT(r != NULL, "Wrong renderer: fail to cast renderer to SequenceWithChromatogramAreaRenderer", );
    r->drawReferenceSelection(p);
    r->drawNameListSelection(p);
}

void McaEditorSequenceArea::buildMenu(QMenu *m) {
    QMenu* viewMenu = GUIUtils::findSubMenu(m, MSAE_MENU_VIEW);
    SAFE_POINT(viewMenu != NULL, "viewMenu", );
    viewMenu->addAction(showQVAction);
    viewMenu->addMenu(traceActionMenu);

    // SANGER_TODO
    QMenu* editMenu = GUIUtils::findSubMenu(m, MSAE_MENU_EDIT);
    SAFE_POINT(editMenu != NULL, "editMenu", );
    QList<QAction*> actions;
    actions << fillWithGapsinsSymAction << replaceCharacterAction << insertAction;
    editMenu->insertActions(editMenu->isEmpty() ? NULL : editMenu->actions().first(), actions);
    editMenu->insertAction(editMenu->actions().first(), ui->getDelSelectionAction());
}

void McaEditorSequenceArea::getColorAndHighlightingIds(QString &csid, QString &hsid,
                                                       DNAAlphabetType atype) {
    csid = MsaColorScheme::UGENE_NUCL;
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
        CHECK(msaMode == InsertCharMode, );
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

        maObj->insertCharacter(selection.y(), selection.x(), newCharacter);

        // insert char into the reference
        U2SequenceObject* ref = getEditor()->referenceObj;
        U2Region region = U2Region(selection.x(), 0);
        ref->replaceRegion(maObj->getEntityRef().entityId, region, DNASequence(QByteArray(1, U2Msa::GAP_CHAR)), os);
        SAFE_POINT_OP(os, );

        exitFromEditCharacterMode();
}

} // namespace
