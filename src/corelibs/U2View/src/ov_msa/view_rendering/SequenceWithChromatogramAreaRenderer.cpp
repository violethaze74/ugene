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

#include "SequenceWithChromatogramAreaRenderer.h"
#include "ov_msa/McaEditorSequenceArea.h"
#include "ov_msa/MaEditorNameList.h"

#include <U2Algorithm/MsaHighlightingScheme.h>
#include <U2Algorithm/MsaColorScheme.h>

#include <U2View/ADVSequenceObjectContext.h> // SANGER_TODO: don't forget to order the include
#include <U2Core/DNASequenceSelection.h>

#include <QPainter>

namespace U2 {

const int SequenceWithChromatogramAreaRenderer::INDENT_BETWEEN_ROWS = 15;
const int SequenceWithChromatogramAreaRenderer::CHROMATOGRAM_MAX_HEIGHT = 100;
const qreal SequenceWithChromatogramAreaRenderer::TRACE_OR_BC_LINES_DIVIDER = 2;

SequenceWithChromatogramAreaRenderer::SequenceWithChromatogramAreaRenderer(McaEditorSequenceArea *seqAreaWgt)
    : SequenceAreaRenderer(seqAreaWgt),
      linePen(Qt::gray, 1, Qt::DotLine)
{
    drawLeadingAndTrailingGaps = false;

    QFontMetricsF fm(seqAreaWgt->getEditor()->getFont());
    charWidth = fm.width('W');
    charHeight = fm.ascent();

    heightBC = seqAreaWgt->getEditor()->getSequenceRowHeight();
    heightPD = seqAreaWgt->getEditor()->getRowHeight() - seqAreaWgt->getEditor()->getSequenceRowHeight() - INDENT_BETWEEN_ROWS;
    heightQuality = charHeight;

    maxTraceHeight = heightPD - heightBC;
}

void SequenceWithChromatogramAreaRenderer::drawReferenceSelection(QPainter &p) const {
    McaEditor* editor = getSeqArea()->getEditor();
    SAFE_POINT(editor != NULL, "McaEditor is NULL", );
    DNASequenceSelection* selection = editor->getReferenceContext()->getSequenceSelection();
    SAFE_POINT(selection != NULL, "DNASequenceSelection is NULL", );
    SAFE_POINT(selection->regions.size() <= 1, "Unexpected multiselection",);
    CHECK(!selection->regions.isEmpty(), );

    U2Region region = selection->regions.first();
    U2Region xRange = seqAreaWgt->getBaseXRange(region.startPos, true);

    p.save();
    // SANGER_TODO: color can be const -- for consensus and here
    QColor color(Qt::lightGray);
    color = color.lighter(115);
    color.setAlpha(127);
    p.fillRect(xRange.startPos, 0,
               xRange.length * region.length, seqAreaWgt->height(),
               color);
    p.restore();
}

void SequenceWithChromatogramAreaRenderer::drawNameListSelection(QPainter &p) const {
    McaEditor* editor = getSeqArea()->getEditor();
    SAFE_POINT(editor != NULL, "McaEditor is NULL", );
    SAFE_POINT(editor->getUI() != NULL, "McaEditor UI is NULL", );

    MaEditorNameList* nameList = editor->getUI()->getEditorNameList();
    SAFE_POINT(nameList != NULL, "MaEditorNameList is NULL", );
    U2Region selection = nameList->getSelection();
    CHECK(!selection.isEmpty(), );
    U2Region selectionPxl = seqAreaWgt->getSequenceYRange(selection.startPos, (int)selection.length);
    p.save();
    // SANGER_TODO: color can be const -- for consensus and here
    QColor color(Qt::lightGray);
    color = color.lighter(115);
    color.setAlpha(127);
    p.fillRect(0, selectionPxl.startPos, seqAreaWgt->width(), selectionPxl.length, color);
    p.restore();
}

void SequenceWithChromatogramAreaRenderer::setAreaHeight(int h) {
    maxTraceHeight = h;
}

int SequenceWithChromatogramAreaRenderer::getScaleBarValue() const {
    return maxTraceHeight;
}

int SequenceWithChromatogramAreaRenderer::drawRow(QPainter &p, const MultipleAlignment& msa, qint64 seq, const U2Region& region, qint64 yStart) const {
    McaEditor* editor = getSeqArea()->getEditor();
    if (editor->isChromVisible(seq)) {
        p.translate(0, INDENT_BETWEEN_ROWS / 2);
    }
    bool ok = SequenceAreaRenderer::drawRow(p, msa, seq, region, yStart);
    CHECK(ok, -1);

    SAFE_POINT(getSeqArea() != NULL, "seqAreaWgt is NULL", -1);
    int w = getSeqArea()->width();
    int seqRowH = editor->getSequenceRowHeight();
    if (editor->isChromVisible(seq)) {
        p.save();
        p.translate(0, yStart + seqRowH);
        p.setPen(QPen(Qt::gray, 1, Qt::DashLine));
        p.drawLine(0, - INDENT_BETWEEN_ROWS / 2 - seqRowH, w, - INDENT_BETWEEN_ROWS / 2 - seqRowH);

        const MultipleChromatogramAlignmentRow& row = editor->getMaObject()->getMcaRow(seq);
        drawChromatogram(p, row, region);
        p.setPen(QPen(Qt::gray, 1, Qt::DashLine));
        p.restore();
        p.translate(0, - INDENT_BETWEEN_ROWS / 2);
        seqRowH = editor->getRowHeight();
    }
    return seqRowH;
}

void SequenceWithChromatogramAreaRenderer::drawChromatogram(QPainter &p, const MultipleChromatogramAlignmentRow& row, const U2Region& _visible) const {
    const DNAChromatogram chroma = row->getGappedChromatogram();

    // SANGER_TODO: should not be here
    chromaMax = 0;
    for (int i = 0; i < chroma.traceLength; i++)
    {
        if (chromaMax < chroma.A[i]) chromaMax = chroma.A[i];
        if (chromaMax < chroma.C[i]) chromaMax = chroma.C[i];
        if (chromaMax < chroma.G[i]) chromaMax = chroma.G[i];
        if (chromaMax < chroma.T[i]) chromaMax = chroma.T[i];
    }

    U2Region visible = U2Region(_visible.startPos, _visible.length + 1).intersect(row->getCoreRegion());
    CHECK(!visible.isEmpty(), );
    if (visible.startPos > _visible.startPos) {
        MaEditor* editor = seqAreaWgt->getEditor();
        SAFE_POINT(editor != NULL, "MaEditor is NULL", );
        p.translate(editor->getColumnWidth() * (visible.startPos - _visible.startPos), 0);
    }
    visible.startPos -= row->getCoreStart();

    CHECK(!visible.isEmpty(), );
    int w = visible.length * seqAreaWgt->getEditor()->getColumnWidth();

    QByteArray seq = row->getCore(); // SANGER_TODO: tmp, get only required region

    // SANGER_TODO:
//    GSLV_UpdateFlags uf = view->getUpdateFlags();
    bool completeRedraw = true; //uf.testFlag(GSLV_UF_NeedCompleteRedraw) || uf.testFlag(GSLV_UF_ViewResized) || uf.testFlag(GSLV_UF_VisibleRangeChanged);
    bool drawQuality = chroma.hasQV && getSeqArea()->getShowQA();
    bool baseCallsLinesVisible = seqAreaWgt->getEditor()->getResizeMode() == MSAEditor::ResizeMode_FontAndContent;

    if (completeRedraw) {
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(Qt::black);
        if (baseCallsLinesVisible) {
            // quality and base calls can be visible
            if (drawQuality) {
                drawQualityValues(chroma, w, heightQuality,
                                  p, visible, seq);
                p.translate(0, heightQuality);
            }
            drawOriginalBaseCalls(drawQuality * heightQuality, p, visible, seq);
        } else {
            drawQuality = false; // to avoid shifting in case the base calls and quality was not visible
        }
        if (w / charWidth > visible.length / TRACE_OR_BC_LINES_DIVIDER) {
            // draw continious trace
            drawChromatogramTrace(chroma, 0, heightBC, heightPD - heightBC - drawQuality * heightQuality,
                                  p, visible);
        } else {
            // draw only "columns" of peaks
            drawChromatogramBaseCallsLines(chroma, heightPD, p, visible, seq);
        }
    }
}

QColor SequenceWithChromatogramAreaRenderer::getBaseColor( char base ) const {
    switch(base) {
        case 'A':
            return Qt::darkGreen;
        case 'C':
            return Qt::blue;
        case 'G':
            return Qt::black;
        case 'T':
            return Qt::red;
        default:
            return Qt::black;
    }
}

void SequenceWithChromatogramAreaRenderer::drawChromatogramTrace(const DNAChromatogram& chroma,
                                                                 qreal x, qreal y, qreal h, QPainter& p,
                                                                 const U2Region& visible) const
{
    if (chromaMax == 0) {
        //nothing to draw
        return;
    }
    //founding problems

    p.setRenderHint(QPainter::Antialiasing, true);
    p.translate(x, h + y);

    QPolygonF polylineA;
    QPolygonF polylineC;
    QPolygonF polylineG;
    QPolygonF polylineT;
    int areaHeight = (heightPD - heightBC) * this->maxTraceHeight / 100;
    qreal columnWidth = getSeqArea()->getEditor()->getColumnWidth();

    int startPos = visible.startPos;
    int prev = 0;
    if (startPos != 0) {
        int prevStep = chroma.baseCalls[startPos] - chroma.baseCalls[startPos - 1];
        prev = chroma.baseCalls[startPos] - prevStep / 2;
    }
    for (int i = startPos; i < visible.endPos(); i++) {
        SAFE_POINT(i < chroma.baseCalls.length(), "Base calls array is too short: visible range index is out range", );
        int k = chroma.baseCalls[i];
        int pointsCount = k - prev;

        qreal pxPerPoint = columnWidth / pointsCount;
        for (int j = 0; j < pointsCount; j++) {
            double x = columnWidth * (i - startPos) + columnWidth / 2 - (pointsCount - j) * pxPerPoint;
            qreal yA = -qMin(static_cast<qreal>(chroma.A[prev + j]) * areaHeight / chromaMax, h);
            qreal yC = -qMin(static_cast<qreal>(chroma.C[prev + j]) * areaHeight / chromaMax, h);
            qreal yG = -qMin(static_cast<qreal>(chroma.G[prev + j]) * areaHeight / chromaMax, h);
            qreal yT = -qMin(static_cast<qreal>(chroma.T[prev + j]) * areaHeight / chromaMax, h);
            polylineA.append(QPointF(x, yA));
            polylineC.append(QPointF(x, yC));
            polylineG.append(QPointF(x, yG));
            polylineT.append(QPointF(x, yT));
        }
        prev = chroma.baseCalls[i];
    }


    if (getSettings().drawTraceA) {
        p.setPen(getBaseColor('A'));
        p.drawPolyline(polylineA);
    }
    if (getSettings().drawTraceC) {
        p.setPen(getBaseColor('C'));
        p.drawPolyline(polylineC);
    }
    if (getSettings().drawTraceG) {
        p.setPen(getBaseColor('G'));
        p.drawPolyline(polylineG);
    }
    if (getSettings().drawTraceT) {
        p.setPen(getBaseColor('T'));
        p.drawPolyline(polylineT);
    }
    p.translate(- x, - h - y);
}

void SequenceWithChromatogramAreaRenderer::drawOriginalBaseCalls(qreal h, QPainter& p, const U2Region& visible, const QByteArray& ba) const {
    p.setPen(Qt::black);
    p.translate( 0, h);

    int colWidth = getSeqArea()->getEditor()->getColumnWidth();
    for (int i = visible.startPos; i < visible.endPos(); i++) {
        QColor color = getBaseColor(ba[i]);
        p.setPen(color);

        int xP = colWidth * (i - visible.startPos) + colWidth / 2;

        p.setPen(linePen);
        p.setRenderHint(QPainter::Antialiasing, false);
        p.drawLine(xP, 0, xP, heightPD - h);
    }
    p.translate( 0, - h);
}

void SequenceWithChromatogramAreaRenderer::drawQualityValues(const DNAChromatogram& chroma, qreal w, qreal h,
                                                             QPainter& p, const U2Region& visible, const QByteArray& ba) const
{
    p.translate(0, h);

    //draw grid
    p.setPen(linePen);
    p.setRenderHint(QPainter::Antialiasing, false);
    for (int i = 0; i < 5; ++i) {
        p.drawLine(0, -h * i/4, w, -h * i/4);
    }

    QLinearGradient gradient(10, 0, 10, -h);
    gradient.setColorAt(0, Qt::green);
    gradient.setColorAt(0.33, Qt::yellow);
    gradient.setColorAt(0.66, Qt::red);
    QBrush brush(gradient);

    p.setBrush(brush);
    p.setPen(Qt::black);
    p.setRenderHint(QPainter::Antialiasing, true);

    int colWidth = getSeqArea()->getEditor()->getColumnWidth();
    QRectF rectangle;
    for (int i = visible.startPos; i < visible.endPos(); i++) {
        int xP = colWidth * (i - visible.startPos);
        switch (ba[i])  {
        case 'A':
            rectangle.setCoords(xP, 0, xP + charWidth, - h / 100 * chroma.prob_A[i]);
            break;
        case 'C':
            rectangle.setCoords(xP, 0, xP + charWidth, - h / 100 * chroma.prob_C[i]);
            break;
        case 'G':
            rectangle.setCoords(xP, 0, xP + charWidth, - h / 100 * chroma.prob_G[i]);
            break;
        case 'T':
            rectangle.setCoords(xP, 0, xP + charWidth, - h / 100 * chroma.prob_T[i]);
            break;
        }
        if (qAbs( rectangle.height() ) > h / 100) {
            p.drawRoundedRect(rectangle, 1.0, 1.0);
        }
    }

    p.translate( 0, - h);
}


void SequenceWithChromatogramAreaRenderer::drawChromatogramBaseCallsLines(const DNAChromatogram& chroma, qreal h, QPainter& p,
                                                                          const U2Region& visible, const QByteArray& ba) const
{
    p.setRenderHint(QPainter::Antialiasing, false);
    p.translate(0, h);

    double yRes = 0;
    int areaHeight = (heightPD - heightBC) * this->maxTraceHeight / 100;
    int colWidth = getSeqArea()->getEditor()->getColumnWidth();
    for (int i = visible.startPos; i < visible.startPos + visible.length; i++) {
        SAFE_POINT(i < chroma.baseCalls.length(), "Base calls array is too short: visible range index is out range", );
        int temp = chroma.baseCalls[i];
        SAFE_POINT(temp <= chroma.traceLength, "Broken chromatogram data", );

        double x = colWidth * (i - visible.startPos) + colWidth / 2;
        bool drawBase = true;
        p.setPen(getBaseColor(ba[i]));
        switch (ba[i])  {
            case 'A':
                yRes = -qMin(static_cast<qreal>(chroma.A[temp])*areaHeight/chromaMax, h);
                drawBase = getSettings().drawTraceA;
                break;
            case 'C':
                yRes = -qMin(static_cast<qreal>(chroma.C[temp]) * areaHeight / chromaMax, h);
                drawBase = getSettings().drawTraceC;
                break;
            case 'G':
                yRes = -qMin(static_cast<qreal>(chroma.G[temp]) * areaHeight / chromaMax, h);
                drawBase = getSettings().drawTraceG;
                break;
            case 'T':
                yRes = -qMin(static_cast<qreal>(chroma.T[temp]) * areaHeight / chromaMax, h);
                drawBase = getSettings().drawTraceT;
                break;
            case 'N':
                continue;
        };
        if (drawBase) {
            p.drawLine(x, 0, x, yRes);
        }
    }
    p.translate( 0, - h);
}

McaEditorSequenceArea* SequenceWithChromatogramAreaRenderer::getSeqArea() const {
    return qobject_cast<McaEditorSequenceArea*>(seqAreaWgt);
}

const ChromatogramViewSettings& SequenceWithChromatogramAreaRenderer::getSettings() const {
    return getSeqArea()->getSettings();
}

} // namespace
