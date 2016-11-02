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

#ifndef _U2_SEQUENCE_AREA_RENDERER_
#define _U2_SEQUENCE_AREA_RENDERER_

#include <U2Core/DNAChromatogram.h>

#include <U2View/MSAEditorSequenceArea.h>

#include <QPen>

namespace U2 {

struct ChromatogramViewSettings {
    bool drawTraceA, drawTraceC, drawTraceG, drawTraceT;
    ChromatogramViewSettings()  {
        drawTraceA = true;
        drawTraceC = true;
        drawTraceG = true;
        drawTraceT = true;
    }
};

class SequenceAreaRenderer : public QObject {
    Q_OBJECT
public:
    SequenceAreaRenderer(MSAEditorSequenceArea*  seqAreaWgt);

    bool drawContent(QPainter &p, const U2Region& region, const QList<qint64> &seqIdx);

private:
    void drawChromatogram(QPainter &p, DNAChromatogram &chroma, U2Region& visibleRange);

private:
    MSAEditorSequenceArea*  seqAreaWgt;
    // SANGER_TODO: move to area-wgt - it should be controlled from there
    ChromatogramViewSettings    settings;


    qreal   charWidth;
    qreal   charHeight;
    qreal   addUpIfQVL;

    int             chromaMax;
    QPen            linePen;
    QFont           font;
    QFont           fontBold;
    int             heightPD;
    int             heightAreaBC;
    int             areaHeight;
    qreal           kLinearTransformTrace;
    qreal           bLinearTransformTrace;
    qreal           kLinearTransformBaseCallsOfEdited;
    qreal           bLinearTransformBaseCallsOfEdited;
    qreal           xBaseCallsOfEdited;
    qreal           yBaseCallsOfEdited;
    qreal           wBaseCallsOfEdited;
    qreal           hBaseCallsOfEdited;

    QColor getBaseColor(char base);

    void drawChromatogramTrace(const DNAChromatogram& chroma,
                               qreal x, qreal y, qreal w, qreal h, QPainter& p,
                               const U2Region& visible /*, const ChromatogramViewSettings& settings*/);
    void drawOriginalBaseCalls(const DNAChromatogram& chroma,
                               qreal x, qreal y, qreal w, qreal h,
                               QPainter& p, const U2Region& visible, const QByteArray& ba, bool is = true);
    void drawQualityValues(const DNAChromatogram& chroma,
                           qreal x, qreal y, qreal w, qreal h,
                           QPainter& p, const U2Region& visible, const QByteArray& ba);
    void drawChromatogramBaseCallsLines(const DNAChromatogram& chroma,
                                        qreal x, qreal y, qreal w, qreal h,
                                        QPainter& p, const U2Region& visible, const QByteArray& ba/*, const ChromatogramViewSettings& settings*/);
};

} // namespace

#endif // _U2_SEQUENCE_AREA_RENDERER_

