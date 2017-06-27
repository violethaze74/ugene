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
#include <U2Core/U2SafePoints.h>

#include <U2View/SequenceObjectContext.h>

#include "McaEditor.h"
#include "McaEditorReferenceArea.h"
#include "McaEditorSequenceArea.h"
#include "MSAEditorConsensusArea.h"
#include "helpers/DrawHelper.h"
#include "helpers/ScrollController.h"

namespace U2 {

McaEditorReferenceArea::McaEditorReferenceArea(McaEditorWgt *ui, SequenceObjectContext *ctx)
    : PanView(ui, ctx, McaReferenceAreaRendererFactory(NULL != ui ? ui->getEditor() : NULL)),
      editor(NULL != ui ? ui->getEditor() : NULL),
      ui(ui),
      renderer(dynamic_cast<McaReferenceAreaRenderer *>(getRenderArea()->getRenderer()))
{
    SAFE_POINT(NULL != renderer, "Renderer is NULL", );

    setLocalToolbarVisible(false);
    settings->showMainRuler = false;

    scrollBar->hide();
    rowBar->hide();

    connect(ui->getEditor()->getMaObject(), SIGNAL(si_alignmentChanged(MultipleAlignment,MaModificationInfo)),
            SLOT(sl_update()));

    connect(ui->getScrollController(), SIGNAL(si_visibleAreaChanged()), SLOT(sl_visibleRangeChanged()));
    connect(ui->getSequenceArea(), SIGNAL(si_visibleRangeChanged()), SLOT(sl_visibleRangeChanged()));
    connect(ui->getSequenceArea(), SIGNAL(si_selectionChanged(MaEditorSelection,MaEditorSelection)),
            SLOT(sl_selectionChanged(MaEditorSelection,MaEditorSelection)));

    connect(ui->getSequenceArea(), SIGNAL(si_clearReferenceSelection()),
            SLOT(sl_clearSelection()));

    connect(ctx->getSequenceSelection(),
        SIGNAL(si_selectionChanged(LRegionsSelection*, const QVector<U2Region>&, const QVector<U2Region>&)),
        SLOT(sl_onSelectionChanged()));

    connect(this, SIGNAL(si_selectionChanged()),
            ui->getSequenceArea(), SLOT(sl_backgroundSelectionChanged()));
    connect(editor, SIGNAL(si_fontChanged(const QFont &)), SLOT(sl_fontChanged(const QFont &)));

    connect(ui->getConsensusArea(), SIGNAL(si_mismatchRedrawRequired()), SLOT(completeUpdate()));

    sl_fontChanged(editor->getFont());
}

void McaEditorReferenceArea::sl_selectMismatch(int pos) {
    MaEditorSequenceArea* seqArea = ui->getSequenceArea();
    if (seqArea->getFirstVisibleBase() > pos || seqArea->getLastVisibleBase(false) < pos) {
        seqArea->centerPos(pos);
    }
    setSelection(U2Region(pos, 1));
}

void McaEditorReferenceArea::sl_visibleRangeChanged() {
    const U2Region visibleRange = ui->getDrawHelper()->getVisibleBases(width());
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

void McaEditorReferenceArea::sl_onSelectionChanged() {
    emit si_selectionChanged();
}

} // namespace
