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

#include "McaEditor.h"
#include "McaReferenceCharController.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2View/SequenceObjectContext.h>

namespace U2 {

McaReferenceCharController::McaReferenceCharController(QObject* p, McaEditor *editor)
    : QObject(p) {
    SequenceObjectContext* ctx = editor->getReferenceContext();
    SAFE_POINT(ctx != NULL, "SequenceObjectContext is NULL", );
    U2SequenceObject* refObject = ctx->getSequenceObject();
    SAFE_POINT(ctx != NULL, "Reference U2SequenceObject is NULL", );
    initRegions(refObject);
}

QVector<U2Region> McaReferenceCharController::getCharRegions(const U2Region& region) {
    int i = region.findIntersectedRegion(charRegions);
    CHECK(i != -1, QVector<U2Region>());

    QVector<U2Region> result;
    do {
        result << charRegions[i];
        i++;
    } while (!charRegions[i].contains(region.endPos()));
    return result;
}

void McaReferenceCharController::initRegions(U2SequenceObject *reference) {
    U2OpStatusImpl os;
    QByteArray data = reference->getWholeSequenceData(os);
    SAFE_POINT_OP(os, );
    U2Region current;
    for (int i = 0; i < data.size(); i++) {
        if (data.at(i) != U2Msa::GAP_CHAR) {
            if (current.isEmpty()) {
                // start new region
                current = U2Region(i, 1);
            } else {
                // extend the current
                current.length++;
            }
        } else if (!current.isEmpty()) {
            // append the region to resut
            charRegions << current;
            current = U2Region();
        }
    }
}

} // namepspace
