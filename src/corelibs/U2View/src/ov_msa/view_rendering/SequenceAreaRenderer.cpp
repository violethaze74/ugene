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

#include "SequenceAreaRenderer.h"

#include <U2Algorithm/MsaHighlightingScheme.h>
#include <U2Algorithm/MsaColorScheme.h>

#include <U2Core/U2OpStatusUtils.h>

#include <QPainter>

namespace U2 {

SequenceAreaRenderer::SequenceAreaRenderer(MaEditorSequenceArea *seqAreaWgt)
    : QObject(seqAreaWgt),
      seqAreaWgt(seqAreaWgt),
      drawLeadingAndTrailingGaps(true) {
}

bool SequenceAreaRenderer::drawContent(QPainter &p, const U2Region &region, const QList<qint64> &seqIdx)  const {
    CHECK(!region.isEmpty(), false);
    CHECK(!seqIdx.isEmpty(), false);

    MsaHighlightingScheme* highlightingScheme = seqAreaWgt->getCurrentHighlightingScheme();
    MaEditor* editor = seqAreaWgt->getEditor();

    p.setPen(Qt::black);
    p.setFont(editor->getFont());

    MultipleAlignmentObject* maObj = editor->getMaObject();
    SAFE_POINT(maObj != NULL, tr("Alignment object is NULL"), false);
    const MultipleAlignment msa = maObj->getMultipleAlignment();

    //Use dots to draw regions, which are similar to reference sequence
    highlightingScheme->setUseDots(seqAreaWgt->getUseDotsCheckedState());

    U2Region baseYRange = U2Region(0, editor->getSequenceRowHeight());

    int rowHeight = 0;
    for (qint64 iSeq = 0; iSeq < seqIdx.size(); iSeq++) {
        qint64 seq = seqIdx[iSeq];

        rowHeight = drawRow(p, msa, seq, region, baseYRange.startPos);
        CHECK(rowHeight, false);

        baseYRange.startPos += rowHeight;
    }

    return true;
}

void SequenceAreaRenderer::drawSelection(QPainter &p) const {
    MaEditorSelection selection = seqAreaWgt->getSelection();;

    int x = selection.x();
    int y = selection.y();

    U2Region xRange = seqAreaWgt->getBaseXRange(x, true);
    U2Region yRange = seqAreaWgt->getSequenceYRange(y, selection.height());

    QPen pen(seqAreaWgt->highlightSelection || seqAreaWgt->hasFocus()
             ? seqAreaWgt->selectionColor
             : Qt::gray);
    if (seqAreaWgt->msaMode == MaEditorSequenceArea::ViewMode) {
        pen.setStyle(Qt::DashLine);
    }
    pen.setWidth(seqAreaWgt->highlightSelection ? 2 : 1);
    p.setPen(pen);
    if (yRange.startPos > 0) {
        switch (seqAreaWgt->msaMode) {
        case MaEditorSequenceArea::ViewMode:
        case MaEditorSequenceArea::ReplaceCharMode:
            p.drawRect(xRange.startPos, yRange.startPos, xRange.length*selection.width(), yRange.length);
            break;
        case MaEditorSequenceArea::InsertCharMode:
            p.drawLine(xRange.startPos, yRange.startPos, xRange.startPos, yRange.startPos + yRange.length);
        }
    }
    else {
        qint64 regionHeight = yRange.length + yRange.startPos + 1;
        if (regionHeight <= 0) {
            return;
        }
        p.drawRect(xRange.startPos, -1, xRange.length*selection.width(), regionHeight);
    }

}

void SequenceAreaRenderer::drawFocus(QPainter &p) const {
    if (seqAreaWgt->hasFocus()) {
        p.setPen(QPen(Qt::black, 1, Qt::DotLine));
        p.drawRect(0, 0, seqAreaWgt->width() - 1, seqAreaWgt->height() - 1);
    }
}

int SequenceAreaRenderer::drawRow(QPainter &p, const MultipleAlignment& msa, qint64 seq, const U2Region& region, qint64 yStart) const {
    // SANGER_TODO: deal with frequent handlign of editor or h/color schemes through the editor etc.
    // move to class parameter
    MsaHighlightingScheme* highlightingScheme = seqAreaWgt->getCurrentHighlightingScheme();
    highlightingScheme->setUseDots(seqAreaWgt->getUseDotsCheckedState());

    MaEditor* editor = seqAreaWgt->getEditor();
    int columnWidth = editor->getColumnWidth();
    QString schemeName = highlightingScheme->metaObject()->className();
    bool isGapsScheme = schemeName == "U2::MSAHighlightingSchemeGaps";
    bool isResizeMode = editor->getResizeMode() == MSAEditor::ResizeMode_FontAndContent;

    U2OpStatusImpl os;
    const int refSeq = msa->getRowIndexByRowId(editor->getReferenceRowId(), os);
    QString refSeqName = editor->getReferenceRowName();

    qint64 regionEnd = region.endPos() - (int)(region.endPos() == editor->getAlignmentLen());
    MultipleAlignmentRow row = msa->getRow(seq);
    int rowHeight = editor->getSequenceRowHeight();
    for (int pos = region.startPos; pos <= regionEnd; pos++) {
        if (!drawLeadingAndTrailingGaps
                && (pos < row->getCoreStart() || pos > row->getCoreStart() + row->getCoreLength() - 1)) {
            continue;
        }

        U2Region baseXRange = U2Region(columnWidth * (pos - region.startPos), columnWidth);
        QRect cr(baseXRange.startPos, yStart, baseXRange.length + 1, rowHeight);
        char c = msa->charAt(seq, pos);

        bool highlight = false;
        QColor color = seqAreaWgt->getCurrentColorScheme()->getColor(seq, pos, c); //! SANGER_TODO: add NULL checks or do smt with the infrastructure
        if (isGapsScheme || highlightingScheme->getFactory()->isRefFree()) { //schemes which applied without reference
            const char refChar = '\n';
            highlightingScheme->process(refChar, c, color, highlight, pos, seq);
        } else if (seq == refSeq || refSeqName.isEmpty()) {
            highlight = true;
        } else {
            const char refChar = editor->getReferenceCharAt(pos);
            highlightingScheme->process(refChar, c, color, highlight, pos, seq);
        }

        if (color.isValid() && highlight) {
            p.fillRect(cr, color);
        }
        if (isResizeMode) {
            p.drawText(cr, Qt::AlignCenter, QString(c));
        }
    }
    return rowHeight;
}

} // namespace
