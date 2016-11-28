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

#include "SequenceWithChromatogramAreaRenderer.h"
#include "../McaEditorSequenceArea.h"

#include <U2Algorithm/MsaHighlightingScheme.h>
#include <U2Algorithm/MsaColorScheme.h>

#include <U2Core/U2OpStatusUtils.h> // check if is needed

#include <QPainter>

namespace U2 {

SequenceWithChromatogramAreaRenderer::SequenceWithChromatogramAreaRenderer(McaEditorSequenceArea *seqAreaWgt)
    : SequenceAreaRenderer(seqAreaWgt),
      linePen(Qt::gray, 1, Qt::DotLine),
      kLinearTransformTrace(0.0),
      bLinearTransformTrace(0.0) {

    font.setFamily("Courier");
    font.setPointSize(12);
    fontBold = font;
    fontBold.setBold(true);
    QFontMetricsF fm(font);
    charWidth = fm.width('W');
    charHeight = fm.ascent();

    heightBC = seqAreaWgt->getEditor()->getSequenceRowHeight();
    heightPD = seqAreaWgt->getEditor()->getRowHeight() - seqAreaWgt->getEditor()->getSequenceRowHeight() - 15; // 15 - indent between lines
    heightQuality = charHeight;

    maxTraceHeight = heightPD - heightBC;

//    if (chroma.hasQV && p->showQV()) {
//        addUpIfQVL = 0;
//    }
//    else    {
//        addUpIfQVL = heightAreaBC - 2*charHeight;
//        setMinimumHeight(height()-addUpIfQVL);
//        areaHeight = height()-heightAreaBC + addUpIfQVL;
//    }
}

void SequenceWithChromatogramAreaRenderer::setAreaHeight(int h) {
    maxTraceHeight = h;
}

int SequenceWithChromatogramAreaRenderer::getScaleBarValue() const {
    return maxTraceHeight;
}

bool SequenceWithChromatogramAreaRenderer::drawRow(QPainter &p, const MultipleAlignment& msa, qint64 seq, const U2Region& region, qint64 yStart) {
    bool ok = SequenceAreaRenderer::drawRow(p, msa, seq, region, yStart);
    CHECK(ok, false);

    SAFE_POINT(getSeqArea() != NULL, "seqAreaWgt is NULL", false);
    McaEditor* editor = getSeqArea()->getEditor();
    if (editor->getShowChromatogram()) {
        // SANGER_TODO: draw chromotogram below
        p.save();
        p.setFont(font); // SANGER_TODO: remove the font? the font is defined for the whole MSA
        p.translate(0, yStart + editor->getSequenceRowHeight());
        const MultipleChromatogramAlignmentRow& row = editor->getMaObject()->getMcaRow(seq);
        drawChromatogram(p, row, region);
        p.restore();
    }
    return true;
}

void SequenceWithChromatogramAreaRenderer::drawChromatogram(QPainter &p, const MultipleChromatogramAlignmentRow& row, const U2Region& _visible) {
    // SANGER_TODO: move from the method
    static const qreal dividerTraceOrBaseCallsLines = 2;
    static const qreal dividerBoolShowBaseCallsChars = 1.5;

    const DNAChromatogram chroma = row->getChromatogram();

    // SANGER_TODO: should not be here
    chromaMax = 0;
    for (int i = 0; i < chroma.traceLength; i++)
    {
        if (chromaMax < chroma.A[i]) chromaMax = chroma.A[i];
        if (chromaMax < chroma.C[i]) chromaMax = chroma.C[i];
        if (chromaMax < chroma.G[i]) chromaMax = chroma.G[i];
        if (chromaMax < chroma.T[i]) chromaMax = chroma.T[i];
    }

    // SANGER_TODO: not Zero region -- there should be the chrom position on the alignment!
    U2Region visible = U2Region(_visible.startPos, _visible.length + 1).intersect(U2Region(0, chroma.seqLength));
    CHECK(!visible.isEmpty(), );
    int w = visible.length * seqAreaWgt->getEditor()->getColumnWidth();

    QByteArray seq = row->getPredictedSequenceData(); // SANGER_TODO: tmp, get only required region

    // SANGER_TODO:
//    GSLV_UpdateFlags uf = view->getUpdateFlags();
    bool completeRedraw = true; //uf.testFlag(GSLV_UF_NeedCompleteRedraw) || uf.testFlag(GSLV_UF_ViewResized) || uf.testFlag(GSLV_UF_VisibleRangeChanged);
    bool drawQuality = chroma.hasQV && getSeqArea()->getShowQA();

    if (completeRedraw) {
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setFont(font);
        p.setPen(Qt::black);
        if (w / charWidth > visible.length / dividerBoolShowBaseCallsChars) {
            // quality and base calls can be visible
            if (drawQuality) {
                drawQualityValues(chroma, w, heightQuality,
                                  p, visible, seq);
                p.translate(0, heightQuality);
            }
            drawOriginalBaseCalls(chroma, w, charHeight,
                                  p, visible, seq);
        } else {
            drawQuality = false; // to avoid shifting in case the base calls and quality was not visible
        }
        if (w / charWidth > visible.length / dividerTraceOrBaseCallsLines) {
            // draw continious trace
            drawChromatogramTrace(chroma,
                                  0, heightBC, w, heightPD - heightBC - drawQuality * heightQuality,
                                  p, visible);
        } else {
            // draw only "columns" of peaks
            drawChromatogramBaseCallsLines(chroma, w, heightPD,
                                           p, visible, seq);
        }
    }

    // SANGER_TODO: draw selection
//    if (hasSel) {
//        p.setPen(linePen);
//        p.drawRect(selRect);
//        hasSel = false;
//    }

    // SANGER_TODO: edit characters - currentBaseCalls - edited ??
//    if (pd->width() / charWidth > visible.length /dividerBoolShowBaseCallsChars && false/*chromaView->editDNASeq!=NULL*/) {
//        drawOriginalBaseCalls(0, 0, width(), charHeight, p, visible, chromaView->currentBaseCalls, false);
//    }

//    const QVector<U2Region>& sel=seqCtx->getSequenceSelection()->getSelectedRegions();
//    if(!sel.isEmpty()) {
//        //draw current selection
//        //selection base on trace transform coef
//        QPen linePenSelection(Qt::darkGray, 1, Qt::SolidLine);
//        p.setPen(linePenSelection);
//        p.setRenderHint(QPainter::Antialiasing, false);

//        U2Region self=sel.first();
//        int i1=self.startPos,i2=self.endPos()-1;
//        unsigned int startBaseCall = kLinearTransformTrace * chroma.baseCalls[i1];
//        unsigned int endBaseCall = kLinearTransformTrace * chroma.baseCalls[i2];
//        if (i1!=0)  {
//            unsigned int prevBaseCall = kLinearTransformTrace * chroma.baseCalls[i1-1];
//            p.drawLine((startBaseCall + prevBaseCall) / 2 + bLinearTransformTrace, 0,
//                (startBaseCall+ prevBaseCall)/2 + bLinearTransformTrace, pd->height());
//        }else {
//            p.drawLine(startBaseCall + bLinearTransformTrace - charWidth / 2, 0,
//                startBaseCall + bLinearTransformTrace - charWidth / 2, pd->height());
//        }
//        if (i2!=chroma.seqLength-1) {
//            unsigned int nextBaseCall = kLinearTransformTrace * chroma.baseCalls[i2+1];
//            p.drawLine((endBaseCall + nextBaseCall) / 2 + bLinearTransformTrace, 0,
//                (endBaseCall + nextBaseCall) / 2 + bLinearTransformTrace, pd->height());
//        } else {
//            p.drawLine(endBaseCall + bLinearTransformTrace + charWidth / 2, 0,
//                endBaseCall + bLinearTransformTrace + charWidth / 2, pd->height());
//        }
//    }

}

QColor SequenceWithChromatogramAreaRenderer::getBaseColor( char base ) {
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
                                                                 qreal x, qreal y, qreal w, qreal h, QPainter& p,
                                                                 const U2Region& visible)
{
    if (chromaMax == 0) {
        //nothing to draw
        return;
    }
    //founding problems

    //areaHeight how to define startValue?
    //colorForIds to private members
    static const QColor colorForIds[4] = {
        Qt::darkGreen, Qt::blue, Qt::black, Qt::red
    };
    p.setRenderHint(QPainter::Antialiasing, true);

    p.translate(x, h + y);

    int a1 = chroma.baseCalls[visible.startPos];
    int a2 = chroma.baseCalls[visible.endPos() - 1];
    qreal leftMargin, rightMargin;
    leftMargin = rightMargin = charWidth;
    qreal k1 = w - leftMargin  - rightMargin;
    int k2 = a2 - a1;
    kLinearTransformTrace = qreal (k1) / k2;
    bLinearTransformTrace = leftMargin - kLinearTransformTrace*a1;
    int mk1 = qMin(static_cast<int>(leftMargin / kLinearTransformTrace), a1);
    int mk2 = qMin(static_cast<int>(rightMargin / kLinearTransformTrace), chroma.traceLength - a2 - 1);
    int polylineSize = a2-a1+mk1+mk2+1;
    QPolygonF polylineA(polylineSize);
    QPolygonF polylineC(polylineSize);
    QPolygonF polylineG(polylineSize);
    QPolygonF polylineT(polylineSize);
    int areaHeight = (heightPD - heightBC) * this->maxTraceHeight / 100;
    for (int j = a1 - mk1; j <= a2 + mk2; ++j) {
        double x = kLinearTransformTrace*j+bLinearTransformTrace;
        qreal yA = -qMin(static_cast<qreal>(chroma.A[j]) * areaHeight / chromaMax, h);
        qreal yC = -qMin(static_cast<qreal>(chroma.C[j]) * areaHeight / chromaMax, h);
        qreal yG = -qMin(static_cast<qreal>(chroma.G[j]) * areaHeight / chromaMax, h);
        qreal yT = -qMin(static_cast<qreal>(chroma.T[j]) * areaHeight / chromaMax, h);
        polylineA[j-a1+mk1] = QPointF(x, yA);
        polylineC[j-a1+mk1] = QPointF(x, yC);
        polylineG[j-a1+mk1] = QPointF(x, yG);
        polylineT[j-a1+mk1] = QPointF(x, yT);
    }
    if (getSettings().drawTraceA) {
        p.setPen(colorForIds[0]);
        p.drawPolyline(polylineA);
    }
    if (getSettings().drawTraceC) {
        p.setPen(colorForIds[1]);
        p.drawPolyline(polylineC);
    }
    if (getSettings().drawTraceG) {
        p.setPen(colorForIds[2]);
        p.drawPolyline(polylineG);
    }
    if (getSettings().drawTraceT) {
        p.setPen(colorForIds[3]);
        p.drawPolyline(polylineT);
    }
    p.translate(- x, - h - y);
}

void SequenceWithChromatogramAreaRenderer::drawOriginalBaseCalls(const DNAChromatogram& chroma, qreal w, qreal h,
                                                                 QPainter& p, const U2Region& visible, const QByteArray& ba, bool is)
{
    QRectF rect;

    p.setPen(Qt::black);
    p.translate( 0, h);

    int a1 = chroma.baseCalls[visible.startPos];
    int a2 = chroma.baseCalls[visible.endPos() - 1];
    qreal leftMargin, rightMargin;
    leftMargin = rightMargin = charWidth;
    qreal k1 = w - leftMargin  - rightMargin;
    int k2 = a2 - a1;
    qreal kLinearTransformBaseCalls = qreal (k1) / k2;
    qreal bLinearTransformBaseCalls = leftMargin - kLinearTransformBaseCalls*a1;

//    ChromatogramView* cview = qobject_cast<ChromatogramView*>(view);
    for (int i=visible.startPos;i<visible.endPos();i++) {
        QColor color = getBaseColor(ba[i]);
        p.setPen(color);

        // SANGER_TODO: dealing with modified characters (do not forget to remove the upper commented line)
        if (false/*cview->indexOfChangedChars.contains(i)*/ && !is) {
            p.setFont(fontBold);
        } else {
            p.setFont(font);
        }
        int xP = kLinearTransformBaseCalls * chroma.baseCalls[i] + bLinearTransformBaseCalls;
        rect.setRect(xP - charWidth/2 + linePen.width(), -h, charWidth, h);
        p.drawText(rect, Qt::AlignCenter, QString(ba[i]));

        if (is) {
            p.setPen(linePen);
            p.setRenderHint(QPainter::Antialiasing, false);
            p.drawLine(xP, 0, xP, heightPD - h);
        }
    }

    if (is) {
        p.setPen(linePen);
        p.setFont(QFont(QString("Courier New"), 8));
        p.drawText(charWidth*1.3, charHeight/2, QString(tr("original sequence")));
    }
    p.translate( 0, - h);
}

void SequenceWithChromatogramAreaRenderer::drawQualityValues(const DNAChromatogram& chroma, qreal w, qreal h,
                                                             QPainter& p, const U2Region& visible, const QByteArray& ba)
{
    QRectF rectangle;

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


     int a1 = chroma.baseCalls[visible.startPos];
     int a2 = chroma.baseCalls[visible.endPos() - 1];
     qreal leftMargin, rightMargin;
     leftMargin = rightMargin = charWidth;
     qreal k1 = w - leftMargin  - rightMargin;
     int k2 = a2 - a1;
     qreal kLinearTransformQV = qreal (k1) / k2;
     qreal bLinearTransformQV = leftMargin - kLinearTransformQV*a1;

     for (int i = visible.startPos; i < visible.endPos(); i++) {
         int xP = kLinearTransformQV * chroma.baseCalls[i] + bLinearTransformQV - charWidth / 2 + linePen.width();
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


void SequenceWithChromatogramAreaRenderer::drawChromatogramBaseCallsLines(const DNAChromatogram& chroma, qreal w, qreal h, QPainter& p,
                                                                          const U2Region& visible, const QByteArray& ba)
{
    static const QColor colorForIds[4] = {
        Qt::darkGreen, Qt::blue, Qt::black, Qt::red
    };
    p.setRenderHint(QPainter::Antialiasing, false);

    p.translate(0, h);

    int a1 = chroma.baseCalls[visible.startPos];
    int a2 = chroma.baseCalls[visible.endPos()-1];
    qreal leftMargin, rightMargin;
    leftMargin = rightMargin = linePen.width();
    qreal k1 = w - leftMargin  - rightMargin;
    int k2 = a2 - a1;
    kLinearTransformTrace = qreal (k1) / k2;
    bLinearTransformTrace = leftMargin - kLinearTransformTrace*a1;
    double yRes = 0;
    int areaHeight = (heightPD - heightBC) * this->maxTraceHeight / 100;
    for (int j = visible.startPos; j < visible.startPos + visible.length; j++) {
        int temp = chroma.baseCalls[j];
        if (temp >= chroma.traceLength) {
            // damaged data - FIXME improve?
            break;
        }
        double x = kLinearTransformTrace*temp+bLinearTransformTrace;
        bool drawBase = true;
        switch (ba[j])  {
            case 'A':
                yRes = -qMin(static_cast<qreal>(chroma.A[temp])*areaHeight/chromaMax, h);
                p.setPen(colorForIds[0]);
                drawBase = getSettings().drawTraceA;
                break;
            case 'C':
                yRes = -qMin(static_cast<qreal>(chroma.C[temp]) * areaHeight / chromaMax, h);
                p.setPen(colorForIds[1]);
                drawBase = getSettings().drawTraceC;
                break;
            case 'G':
                yRes = -qMin(static_cast<qreal>(chroma.G[temp]) * areaHeight / chromaMax, h);
                p.setPen(colorForIds[2]);
                drawBase = getSettings().drawTraceG;
                break;
            case 'T':
                yRes = -qMin(static_cast<qreal>(chroma.T[temp]) * areaHeight / chromaMax, h);
                p.setPen(colorForIds[3]);
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
