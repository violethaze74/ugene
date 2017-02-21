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

#ifndef _U2_SEQUENCE_AREA_RENDERER_H_
#define _U2_SEQUENCE_AREA_RENDERER_H_

#include <U2Core/DNAChromatogram.h>

#include <U2View/MSAEditorSequenceArea.h>

#include <QPen>

namespace U2 {

class SequenceAreaRenderer : public QObject {
    Q_OBJECT
public:
    SequenceAreaRenderer(MaEditorSequenceArea* seqAreaWgt);

    bool drawContent(QPainter &p, const U2Region& region, const QList<qint64> &seqIdx) const;

    void drawSelection(QPainter &p) const;
    void drawFocus(QPainter& p) const;

protected:
    // returns the height of the drawn row
    virtual int drawRow(QPainter &p, const MultipleAlignment& msa, qint64 seq, const U2Region& region, qint64 yStart) const;

    MaEditorSequenceArea*  seqAreaWgt;

    bool drawLeadingAndTrailingGaps;
};

} // namespace

#endif // _U2_SEQUENCE_AREA_RENDERER_H_

