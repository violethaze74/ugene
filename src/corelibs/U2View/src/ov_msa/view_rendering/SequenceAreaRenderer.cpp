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

#include "SequenceAreaRenderer.h"

#include <U2Algorithm/MsaHighlightingScheme.h>
#include <U2Algorithm/MsaColorScheme.h>

#include <U2Core/U2OpStatusUtils.h>

#include <QPainter>

namespace U2 {

SequenceAreaRenderer::SequenceAreaRenderer(MaEditorSequenceArea *seqAreaWgt)
    : QObject(seqAreaWgt),
      seqAreaWgt(seqAreaWgt) {
}

bool SequenceAreaRenderer::drawContent(QPainter &p, const U2Region &region, const QList<qint64> &seqIdx) {
    CHECK(!region.isEmpty(), false);
    CHECK(!seqIdx.isEmpty(), false);

    MsaHighlightingScheme* highlightingScheme = seqAreaWgt->getCurrentHighlightingScheme();
    MaEditor* editor = seqAreaWgt->getEditor();

    p.fillRect(QRect(0, 0,
                     editor->getColumnWidth() * region.length,
                     editor->getRowHeight() * seqIdx.size()),
               Qt::white);
    p.setPen(Qt::black);
    p.setFont(editor->getFont());

    MultipleAlignmentObject* maObj = editor->getMaObject();
    SAFE_POINT(maObj != NULL, tr("Alignment object is NULL"), false);
    const MultipleAlignment msa = maObj->getMultipleAlignment();

    //Use dots to draw regions, which are similar to reference sequence
    highlightingScheme->setUseDots(seqAreaWgt->getUseDotsCheckedState());

    U2Region baseYRange = U2Region(0, editor->getSequenceRowHeight());

    bool ok = true;
    for (qint64 iSeq = 0; iSeq < seqIdx.size(); iSeq++) {
        qint64 seq = seqIdx[iSeq];

        ok = drawRow(p, msa, seq, region, baseYRange.startPos);
        CHECK(ok, false);

        baseYRange.startPos += editor->getRowHeight();
    }

    return true;
}

bool SequenceAreaRenderer::drawRow(QPainter &p, const MultipleAlignment& msa, qint64 seq, const U2Region& region, qint64 yStart) {
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
    for (int pos = region.startPos; pos <= regionEnd; pos++) {
        U2Region baseXRange = U2Region(columnWidth * (pos - region.startPos), columnWidth);
        QRect cr(baseXRange.startPos, yStart, baseXRange.length + 1, editor->getSequenceRowHeight());
        char c = msa->charAt(seq, pos);

        bool highlight = false;
        QColor color = seqAreaWgt->getCurrentColorScheme()->getColor(seq, pos, c); //! SANGER_TODO: add NULL checks or do smt with the infrastructure
        if (isGapsScheme || highlightingScheme->getFactory()->isRefFree()) { //schemes which applied without reference
            const char refChar = '\n';
            highlightingScheme->process(refChar, c, color, highlight, pos, seq);
        } else if (seq == refSeq || refSeqName.isEmpty()) {
            highlight = true;
        } else {
            const char refChar = msa->charAt(refSeq, pos);
            highlightingScheme->process(refChar, c, color, highlight, pos, seq);
        }

        if (color.isValid() && highlight) {
            p.fillRect(cr, color);
        }
        if (isResizeMode) {
            p.drawText(cr, Qt::AlignCenter, QString(c));
        }
    }
    return true;
}

} // namespace
