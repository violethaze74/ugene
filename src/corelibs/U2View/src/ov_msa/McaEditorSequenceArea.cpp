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

#include "McaEditorSequenceArea.h"

#include "view_rendering/SequenceWithChromatogramAreaRenderer.h"

#include <U2Algorithm/MsaColorScheme.h>
#include <U2Algorithm/MsaHighlightingScheme.h>

#include <U2Gui/GUIUtils.h>

#include <QToolButton>

namespace U2 {

McaEditorSequenceArea::McaEditorSequenceArea(MaEditorWgt *ui, GScrollBar *hb, GScrollBar *vb)
    : MaEditorSequenceArea(ui, hb, vb) {
    initRenderer();

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
    for (int i = firstVisibleRow; i < seq; i++) {
        if (getEditor()->isChromVisible(i)) {
            start += editor->getRowHeight();
        } else {
            start += editor->getSequenceRowHeight();
        }
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
    U2Region res(MaEditorSequenceArea::getSequenceYRange(startSeq, false).startPos, len);
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

void McaEditorSequenceArea::sl_referenceSelectionChanged() {
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
}

void McaEditorSequenceArea::initRenderer() {
    renderer = new SequenceWithChromatogramAreaRenderer(this);
}

void McaEditorSequenceArea::drawBackground(QPainter& p) {
    SequenceWithChromatogramAreaRenderer* r = qobject_cast<SequenceWithChromatogramAreaRenderer*>(renderer);
    SAFE_POINT(r != NULL, "Wrong renderer: fail to cast renderer to SequenceWithChromatogramAreaRenderer", );
    r->drawReferenceSelection(p);
}

void McaEditorSequenceArea::buildMenu(QMenu *m) {
    QMenu* viewMenu = GUIUtils::findSubMenu(m, MSAE_MENU_VIEW);
    SAFE_POINT(viewMenu != NULL, "viewMenu", );
    viewMenu->addAction(showQVAction);
    viewMenu->addMenu(traceActionMenu);
}

void McaEditorSequenceArea::getColorAndHighlightingIds(QString &csid, QString &hsid,
                                                       DNAAlphabetType, bool) {
    // SANGER_TODO: basically sanger cannot be not nucleotide
    csid = MsaColorScheme::UGENE_NUCL;
    hsid = MsaHighlightingScheme::DISAGREEMENTS_NUCL;
}

QAction* McaEditorSequenceArea::createToggleTraceAction(const QString& actionName) {
    QAction* showTraceAction = new QAction(actionName, this);
    showTraceAction->setCheckable(true);
    showTraceAction->setChecked(true);
    showTraceAction->setEnabled(true);
    connect(showTraceAction, SIGNAL(triggered(bool)), SLOT(sl_showHideTrace()));

    return showTraceAction;
}

} // namespace
