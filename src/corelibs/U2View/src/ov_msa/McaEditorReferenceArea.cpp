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

#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/SignalBlocker.h>
#include <U2Core/U2Region.h>
#include <U2Core/U2SafePoints.h>

#include <U2View/SequenceObjectContext.h>

#include "McaEditor.h"
#include "McaEditorConsensusArea.h"
#include "McaEditorReferenceArea.h"
#include "McaEditorSequenceArea.h"
#include "MSAEditorConsensusArea.h"
#include "helpers/DrawHelper.h"
#include "helpers/ScrollController.h"
#include "ov_msa/helpers/BaseWidthController.h"

namespace U2 {

McaEditorReferenceArea::McaEditorReferenceArea(McaEditorWgt *ui, SequenceObjectContext *ctx)
    : PanView(ui, ctx, McaEditorReferenceRenderAreaFactory(ui, NULL != ui ? ui->getEditor() : NULL)),
      editor(NULL != ui ? ui->getEditor() : NULL),
      ui(ui),
      renderer(dynamic_cast<McaReferenceAreaRenderer *>(getRenderArea()->getRenderer()))
{
    SAFE_POINT(NULL != renderer, "Renderer is NULL", );

    singleBaseSelection = true;
    setLocalToolbarVisible(false);
    settings->showMainRuler = false;

    scrollBar->hide();
    rowBar->hide();

    connect(ui->getEditor()->getMaObject(), SIGNAL(si_alignmentChanged(MultipleAlignment,MaModificationInfo)),
            SLOT(sl_update()));

    connect(ui->getScrollController(), SIGNAL(si_visibleAreaChanged()), SLOT(sl_visibleRangeChanged()));
    connect(ui->getSequenceArea(), SIGNAL(si_selectionChanged(MaEditorSelection,MaEditorSelection)),
            SLOT(sl_selectionChanged(MaEditorSelection,MaEditorSelection)));

    connect(ui, SIGNAL(si_clearSelection()), SLOT(sl_clearSelection()));
    connect(ui->getSequenceArea(), SIGNAL(si_clearReferenceSelection()),
            SLOT(sl_clearSelection()));

    connect(ctx->getSequenceSelection(),
        SIGNAL(si_selectionChanged(LRegionsSelection*, const QVector<U2Region>&, const QVector<U2Region>&)),
        SLOT(sl_onSelectionChanged(LRegionsSelection*, const QVector<U2Region>&, const QVector<U2Region>&)));

    connect(this, SIGNAL(si_selectionChanged()),
            ui->getSequenceArea(), SLOT(sl_backgroundSelectionChanged()));
    connect(editor, SIGNAL(si_fontChanged(const QFont &)), SLOT(sl_fontChanged(const QFont &)));

    connect(ui->getConsensusArea(), SIGNAL(si_mismatchRedrawRequired()), SLOT(completeUpdate()));
    connect(scrollBar, SIGNAL(valueChanged(int)), ui->getScrollController()->getHorizontalScrollBar(), SLOT(setValue(int)));

    sl_fontChanged(editor->getFont());
}

void McaEditorReferenceArea::sl_selectMismatch(int pos) {
    MaEditorSequenceArea* seqArea = ui->getSequenceArea();
    if (seqArea->getFirstVisibleBase() > pos || seqArea->getLastVisibleBase(false) < pos) {
        seqArea->centerPos(pos);
    }
    seqArea->sl_cancelSelection();
    setSelection(U2Region(pos, 1));
}

void McaEditorReferenceArea::sl_visibleRangeChanged() {
    const U2Region visibleRange = ui->getDrawHelper()->getVisibleBases(ui->getSequenceArea()->width());
    setVisibleRange(visibleRange);
}

void McaEditorReferenceArea::sl_selectionChanged(const MaEditorSelection &current, const MaEditorSelection &) {
    U2Region selection(current.x(), current.width());
    setSelection(selection);
}

void McaEditorReferenceArea::sl_clearSelection() {
    ctx->getSequenceSelection()->clear();
}

void McaEditorReferenceArea::sl_fontChanged(const QFont &newFont) {
    renderer->setFont(newFont);
    setFixedHeight(renderer->getMinimumHeight());
}

void McaEditorReferenceArea::sl_update() {
    getSequenceObject()->forceCachedSequenceUpdate();
    completeUpdate();
}

void McaEditorReferenceArea::keyPressEvent(QKeyEvent *event) {
    const int key = event->key();
    bool accepted = false;
    DNASequenceSelection * const selection = ctx->getSequenceSelection();
    U2Region selectedRegion = (NULL != selection && !selection->isEmpty() ? selection->getSelectedRegions().first() : U2Region());

    switch(key) {
    case Qt::Key_Left:
        if (!selectedRegion.isEmpty() && selectedRegion.startPos > 0) {
            selectedRegion.startPos--;
            ctx->getSequenceSelection()->setSelectedRegions(QVector<U2Region>() << selectedRegion);
            ui->getScrollController()->scrollToBase(selectedRegion.startPos, width());
        }
    case Qt::Key_Up:
        accepted = true;
        break;
    case Qt::Key_Right:
        if (!selectedRegion.isEmpty() && selectedRegion.endPos() < ctx->getSequenceLength()) {
            selectedRegion.startPos++;
            ctx->getSequenceSelection()->setSelectedRegions(QVector<U2Region>() << selectedRegion);
            ui->getScrollController()->scrollToBase(selectedRegion.endPos() - 1, width());
        }
    case Qt::Key_Down:
        accepted = true;
        break;
    case Qt::Key_Home:
        ui->getScrollController()->scrollToEnd(ScrollController::Left);
        accepted = true;
        break;
    case Qt::Key_End:
        ui->getScrollController()->scrollToEnd(ScrollController::Right);
        accepted = true;
        break;
    case Qt::Key_PageUp:
        ui->getScrollController()->scrollPage(ScrollController::Left);
        accepted = true;
        break;
    case Qt::Key_PageDown:
        ui->getScrollController()->scrollPage(ScrollController::Right);
        accepted = true;
        break;
    }

    if (accepted) {
        event->accept();
    } else {
        PanView::keyPressEvent(event);
    }
}

void McaEditorReferenceArea::updateScrollBar() {
    SignalBlocker signalBlocker(scrollBar);
    Q_UNUSED(signalBlocker);

    const QScrollBar * const hScrollbar = ui->getScrollController()->getHorizontalScrollBar();

    scrollBar->setMinimum(hScrollbar->minimum());
    scrollBar->setMaximum(hScrollbar->maximum());
    scrollBar->setSliderPosition(hScrollbar->value());
    scrollBar->setSingleStep(hScrollbar->singleStep());
    scrollBar->setPageStep(hScrollbar->pageStep());
}

void McaEditorReferenceArea::sl_onSelectionChanged(LRegionsSelection * /*selection*/, const QVector<U2Region> &addedRegions, const QVector<U2Region> &removedRegions) {
    if (addedRegions.size() == 1) {
        const U2Region addedRegion = addedRegions.first();
        qint64 baseToScrollTo = -1;
        if (removedRegions.size() == 1) {
            const U2Region removedRegion = removedRegions.first();
            if (addedRegion.startPos < removedRegion.startPos && addedRegion.endPos() == removedRegion.endPos()) {
                baseToScrollTo = addedRegion.startPos;
            } else if (addedRegion.startPos == removedRegion.startPos && addedRegion.endPos() > removedRegion.endPos()) {
                baseToScrollTo = addedRegion.endPos() - 1;
            } else {
                baseToScrollTo = addedRegion.startPos;
            }
        } else {
            baseToScrollTo = addedRegion.startPos;
        }
        ui->getScrollController()->scrollToBase(static_cast<int>(baseToScrollTo), width());
    }
    emit si_selectionChanged();
}

McaEditorReferenceRenderArea::McaEditorReferenceRenderArea(McaEditorWgt *_ui, PanView *d, PanViewRenderer *renderer)
    : PanViewRenderArea(d, renderer),
    ui(_ui) {
}

qint64 McaEditorReferenceRenderArea::coordToPos(int x) const {
    qint64 res = 0;
    if (ui != NULL) {
        res = qBound(0, ui->getBaseWidthController()->screenXPositionToColumn(x), ui->getEditor()->getAlignmentLen());
    }
    return res;
}

McaEditorReferenceRenderAreaFactory::McaEditorReferenceRenderAreaFactory(McaEditorWgt *_ui, McaEditor *_editor)
    : PanViewRenderAreaFactory(),
    ui(_ui),
    maEditor(_editor) {

}

PanViewRenderArea * McaEditorReferenceRenderAreaFactory::createRenderArea(PanView *panView) const {
    return new McaEditorReferenceRenderArea(ui, panView, new McaReferenceAreaRenderer(panView, panView->getSequenceContext(), maEditor));
}

}   // namespace U2
