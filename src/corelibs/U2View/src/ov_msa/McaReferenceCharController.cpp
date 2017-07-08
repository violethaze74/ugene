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

OffsetRegions::OffsetRegions() {

}

void OffsetRegions::append(const U2Region& region, int offset) {
    regions.append(region);
    offsets.append(offset);
}

int OffsetRegions::findIntersectedRegion(const U2Region& region) {
    return region.findIntersectedRegion(regions);
}

U2Region OffsetRegions::getRegion(int i) {
    SAFE_POINT(0 <= i && i < regions.size(), "Index out of range", U2Region());
    return regions[i];
}

int OffsetRegions::getOffset(int i) {
    SAFE_POINT(0 <= i && i < offsets.size(), "Index out of range", 0);
    return offsets[i];
}

int OffsetRegions::getSize() {
    SAFE_POINT(regions.size() == offsets.size(), "Invalid state!", 0);
    return regions.size();
}

void OffsetRegions::clear() {
    regions.clear();
    offsets.clear();
}

McaReferenceCharController::McaReferenceCharController(QObject* p, McaEditor *editor)
    : QObject(p),
      refObject (NULL) {
    SequenceObjectContext* ctx = editor->getReferenceContext();
    SAFE_POINT(ctx != NULL, "SequenceObjectContext is NULL", );
    refObject = ctx->getSequenceObject();
    SAFE_POINT(ctx != NULL, "Reference U2SequenceObject is NULL", );
    initRegions();

    connect(refObject, SIGNAL(si_sequenceChanged()), SLOT(sl_update()));
    connect(editor->getMaObject(), SIGNAL(si_alignmentChanged(MultipleAlignment,MaModificationInfo)),
            SLOT(sl_update(MultipleAlignment,MaModificationInfo)));
}

OffsetRegions McaReferenceCharController::getCharRegions(const U2Region& region) {
    int i = charRegions.findIntersectedRegion(region);
    CHECK(i != -1, OffsetRegions());

    OffsetRegions result;
    do {
        result.append(region.intersect(charRegions.getRegion(i)),
                      charRegions.getOffset(i));
        if (charRegions.getRegion(i).contains(region.endPos())) {
            return result;
        }
        i++;
    } while (i < charRegions.getSize());
    return result;
}

void McaReferenceCharController::sl_update() {
    initRegions();
}

void McaReferenceCharController::sl_update(const MultipleAlignment &, const MaModificationInfo &modInfo) {
    if (modInfo.type == MaModificationType_Undo || modInfo.type == MaModificationType_Redo) {
        initRegions();
    }
}

void McaReferenceCharController::initRegions() {
    charRegions.clear();
    SAFE_POINT(refObject != NULL, "MCA reference object is NULL", );

    U2OpStatusImpl os;
    refObject->forceCachedSequenceUpdate();
    QByteArray data = refObject->getWholeSequenceData(os);

    SAFE_POINT_OP(os, );
    U2Region current;
    int gapCounter = 0;
    for (int i = 0; i < data.size(); i++) {
        if (data.at(i) != U2Msa::GAP_CHAR) {
            if (current.isEmpty()) {
                // start new region
                current = U2Region(i, 1);
            } else {
                // extend the current
                current.length++;
            }
        } else {
            if (!current.isEmpty()) {
                // append the region to resut
                charRegions.append(current, gapCounter);
                current = U2Region();
            }
            gapCounter++;
        }
    }
    if (!current.isEmpty()) {
        charRegions.append(current, gapCounter);
    }
}

} // namepspace
