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

#include <U2Core/DatatypeSerializeUtils.h>
#include <U2Core/DbiConnection.h>
#include <U2Core/DNAChromatogramObject.h>
#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/RawDataUdrSchema.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2ObjectRelationsDbi.h>

#include "ChromatogramUtils.h"

namespace U2 {

void ChromatogramUtils::append(DNAChromatogram chromatogram, const DNAChromatogram &appendedChromatogram) {
    chromatogram.traceLength += appendedChromatogram.traceLength;
    chromatogram.seqLength += appendedChromatogram.seqLength;
    chromatogram.baseCalls += appendedChromatogram.baseCalls;   // TODO: recalculte appended positions
    chromatogram.A += appendedChromatogram.A;
    chromatogram.C += appendedChromatogram.C;
    chromatogram.G += appendedChromatogram.G;
    chromatogram.T += appendedChromatogram.T;
    chromatogram.prob_A += appendedChromatogram.prob_A;
    chromatogram.prob_C += appendedChromatogram.prob_C;
    chromatogram.prob_G += appendedChromatogram.prob_G;
    chromatogram.prob_T += appendedChromatogram.prob_T;
    chromatogram.hasQV &= appendedChromatogram.hasQV;
}

void ChromatogramUtils::remove(U2OpStatus &os, DNAChromatogram &chromatogram, int startPos, int endPos) {
    if ((endPos <= startPos) || (startPos < 0) || (endPos > chromatogram.traceLength)) {
        coreLog.trace(L10N::internalError("incorrect parameters was passed to ChromatogramUtils::remove, "
            "startPos '%1', endPos '%2', chromatogram length '%3'").arg(startPos).arg(endPos).arg(chromatogram.traceLength));
        os.setError("Can't remove chars from a chromatogram");
        return;
    }

    const int regionLength = endPos - startPos;
    chromatogram.traceLength -= regionLength;
    chromatogram.seqLength -= regionLength;
    chromatogram.baseCalls.remove(startPos, regionLength);
    chromatogram.A.remove(startPos, regionLength);
    chromatogram.C.remove(startPos, regionLength);
    chromatogram.G.remove(startPos, regionLength);
    chromatogram.T.remove(startPos, regionLength);
    chromatogram.prob_A.remove(startPos, regionLength);
    chromatogram.prob_C.remove(startPos, regionLength);
    chromatogram.prob_G.remove(startPos, regionLength);
    chromatogram.prob_T.remove(startPos, regionLength);
}

bool ChromatogramUtils::areEqual(const DNAChromatogram &first, const DNAChromatogram &second) {
    return first.traceLength == second.traceLength &&
            first.seqLength == second.seqLength &&
            first.baseCalls == second.baseCalls &&
            first.A == second.A &&
            first.C == second.C &&
            first.G == second.G &&
            first.T == second.T &&
            first.prob_A == second.prob_A &&
            first.prob_C == second.prob_C &&
            first.prob_G == second.prob_G &&
            first.prob_T == second.prob_T &&
            first.hasQV == second.hasQV;
}

namespace {

template <class T>
void zeroEndingCrop(QVector<T> &data, int startPos, int length) {
    data = data.mid(startPos, length);
    if (data.size() == startPos + length + 1) {
        data << 0;
    }
}

}

void ChromatogramUtils::crop(DNAChromatogram &chromatogram, int startPos, int length) {
    const U2Region traceRegion = sequenceRegion2TraceRegion(chromatogram, U2Region(startPos, length));
    const ushort baseCallOffset = traceRegion.startPos == 0 ? 0 : chromatogram.baseCalls[startPos - 1];
    if (traceRegion.startPos > 0) {
        for (int i = startPos, n = qMin(startPos + length, chromatogram.baseCalls.size()); i < n; i++) {
            chromatogram.baseCalls[i] -= baseCallOffset;
        }
    }
    zeroEndingCrop(chromatogram.baseCalls, startPos, length);
    chromatogram.traceLength = qMin(chromatogram.traceLength - traceRegion.startPos, traceRegion.length);
    chromatogram.seqLength = qMin(chromatogram.seqLength - startPos, length);

    zeroEndingCrop(chromatogram.A, traceRegion.startPos, traceRegion.length);
    zeroEndingCrop(chromatogram.C, traceRegion.startPos, traceRegion.length);
    zeroEndingCrop(chromatogram.G, traceRegion.startPos, traceRegion.length);
    zeroEndingCrop(chromatogram.T, traceRegion.startPos, traceRegion.length);
    zeroEndingCrop(chromatogram.prob_A, startPos, length);
    zeroEndingCrop(chromatogram.prob_C, startPos, length);
    zeroEndingCrop(chromatogram.prob_G, startPos, length);
    zeroEndingCrop(chromatogram.prob_T, startPos, length);
}

U2EntityRef ChromatogramUtils::import(U2OpStatus &os, const U2DbiRef &dbiRef, const QString &folder, const DNAChromatogram &chromatogram) {
    U2Chromatogram dbChromatogram(dbiRef);
    dbChromatogram.visualName = chromatogram.name;
    dbChromatogram.serializer = DNAChromatogramSerializer::ID;

    RawDataUdrSchema::createObject(dbiRef, folder, dbChromatogram, os);
    CHECK_OP(os, U2EntityRef());

    const U2EntityRef entityRef(dbiRef, dbChromatogram.id);
    updateChromtogramData(os, entityRef, chromatogram);
    CHECK_OP(os, U2EntityRef());

    return entityRef;
}

DNAChromatogram ChromatogramUtils::exportChromatogram(U2OpStatus &os, const U2EntityRef &chromatogramRef) {
    const QString serializer = RawDataUdrSchema::getObject(chromatogramRef, os).serializer;
    CHECK_OP(os, DNAChromatogram());
    SAFE_POINT_EXT(DNAChromatogramSerializer::ID == serializer, os.setError(QString("Unknown serializer id: %1").arg(serializer)), DNAChromatogram());
    const QByteArray data = RawDataUdrSchema::readAllContent(chromatogramRef, os);
    CHECK_OP(os, DNAChromatogram());
    return DNAChromatogramSerializer::deserialize(data, os);
}

U2Chromatogram ChromatogramUtils::getChromatogramDbInfo(U2OpStatus &os, const U2EntityRef &chromatogramRef) {
    return RawDataUdrSchema::getObject(chromatogramRef, os);
}

qint64 ChromatogramUtils::getChromatogramLength(U2OpStatus &os, const U2EntityRef &chromatogramRef) {
    return exportChromatogram(os, chromatogramRef).traceLength;
}

void ChromatogramUtils::updateChromtogramData(U2OpStatus &os, const U2EntityRef &chromatogramRef, const DNAChromatogram &chromatogram) {
    const QByteArray data = DNAChromatogramSerializer::serialize(chromatogram);
    RawDataUdrSchema::writeContent(data, chromatogramRef, os);
    CHECK_OP(os, );
}

U2EntityRef ChromatogramUtils::getChromatogramIdByRelatedSequenceId(U2OpStatus &os, const U2EntityRef &sequenceRef) {
    DbiConnection connection(sequenceRef.dbiRef, os);
    CHECK_OP(os, U2EntityRef());

    const QList<U2DataId> relatedObjects = connection.dbi->getObjectRelationsDbi()->getReferenceRelatedObjects(sequenceRef.entityId, ObjectRole_Sequence, os);
    CHECK_OP(os, U2EntityRef());

    QList<U2DataId> chromatogramsIds;
    foreach (const U2DataId &dataId, relatedObjects) {
        if (U2Type::Chromatogram != U2DbiUtils::toType(dataId)) {
            continue;
        }
        chromatogramsIds << dataId;
    }
    assert(chromatogramsIds.size() <= 1);

    CHECK(chromatogramsIds.size() == 1, U2EntityRef());
    return U2EntityRef(sequenceRef.dbiRef, chromatogramsIds.first());
}

QString ChromatogramUtils::getChromatogramName(U2OpStatus &os, const U2EntityRef &chromatogramRef) {
    DbiConnection connection(chromatogramRef.dbiRef, os);
    CHECK_OP(os, QString());

    U2Object object;
    connection.dbi->getObjectDbi()->getObject(object, chromatogramRef.entityId, os);
    CHECK_OP(os, QString());
    return object.visualName;
}

DNAChromatogram ChromatogramUtils::reverse(const DNAChromatogram &chromatogram) {
    DNAChromatogram reversedChromatogram = chromatogram;

    reversedChromatogram.baseCalls.clear();
    bool zeroEnding = false;
    for (int i = 1, n = chromatogram.baseCalls.size(); i < n; i++) {
        if (i >= chromatogram.seqLength && chromatogram.baseCalls[i] == 0) {
            zeroEnding = true;
            continue;
        }
        reversedChromatogram.baseCalls << chromatogram.traceLength - chromatogram.baseCalls[i - 1];
    }
    reversedChromatogram.baseCalls << chromatogram.traceLength - chromatogram.baseCalls[chromatogram.baseCalls.size() - 1];
    std::reverse(reversedChromatogram.baseCalls.begin(), reversedChromatogram.baseCalls.end());
    if (zeroEnding) {
        reversedChromatogram.baseCalls << 0;        // zero-ending vector
    }

    std::reverse(reversedChromatogram.A.begin(), reversedChromatogram.A.end());
    std::reverse(reversedChromatogram.C.begin(), reversedChromatogram.C.end());
    std::reverse(reversedChromatogram.G.begin(), reversedChromatogram.G.end());
    std::reverse(reversedChromatogram.T.begin(), reversedChromatogram.T.end());

    if (reversedChromatogram.seqLength > 0) {
        std::reverse(reversedChromatogram.prob_A.begin(), (chromatogram.prob_A.last() == 0 ? reversedChromatogram.prob_A.end() - 1 : reversedChromatogram.prob_A.end()));
        std::reverse(reversedChromatogram.prob_C.begin(), (chromatogram.prob_C.last() == 0 ? reversedChromatogram.prob_C.end() - 1 : reversedChromatogram.prob_C.end()));
        std::reverse(reversedChromatogram.prob_G.begin(), (chromatogram.prob_G.last() == 0 ? reversedChromatogram.prob_G.end() - 1 : reversedChromatogram.prob_G.end()));
        std::reverse(reversedChromatogram.prob_T.begin(), (chromatogram.prob_T.last() == 0 ? reversedChromatogram.prob_T.end() - 1 : reversedChromatogram.prob_T.end()));
    }

    return reversedChromatogram;
}

DNAChromatogram ChromatogramUtils::complement(const DNAChromatogram &chromatogram) {
    DNAChromatogram complementedChromatogram = chromatogram;
    complementedChromatogram.A = chromatogram.T;
    complementedChromatogram.C = chromatogram.G;
    complementedChromatogram.G = chromatogram.C;
    complementedChromatogram.T = chromatogram.A;
    complementedChromatogram.prob_A = chromatogram.prob_T;
    complementedChromatogram.prob_C = chromatogram.prob_G;
    complementedChromatogram.prob_G = chromatogram.prob_C;
    complementedChromatogram.prob_T = chromatogram.prob_A;
    return complementedChromatogram;
}

DNAChromatogram ChromatogramUtils::reverseComplement(const DNAChromatogram &chromatogram) {
    return reverse(complement(chromatogram));
}

U2Region ChromatogramUtils::sequenceRegion2TraceRegion(const DNAChromatogram &chromatogram, const U2Region &sequenceRegion) {
    SAFE_POINT(sequenceRegion.startPos <= chromatogram.baseCalls.length()
               && sequenceRegion.endPos() <= chromatogram.baseCalls.length(),
               "Sequence region is out of base calls array boundaries", U2Region());
    const int traceStartPos = sequenceRegion.startPos == 0 ? 0 : chromatogram.baseCalls[sequenceRegion.startPos - 1];
    const int traceLength = chromatogram.baseCalls[sequenceRegion.endPos()] - traceStartPos - 1;
    return U2Region(traceStartPos, traceLength);
}

void ChromatogramUtils::insertBase(DNAChromatogram &chromatogram, int pos) {
    if (pos != 0 && pos != chromatogram.seqLength) {
        ushort newTracePos = chromatogram.baseCalls[pos] + (chromatogram.baseCalls[pos + 1] - chromatogram.baseCalls[pos])/2;
        chromatogram.baseCalls.insert(pos + 1, newTracePos);

        chromatogram.prob_A.insert(pos + 1, 0); // SANGER_TODO: or 100?
        chromatogram.prob_C.insert(pos + 1, 0);
        chromatogram.prob_G.insert(pos + 1, 0);
        chromatogram.prob_T.insert(pos + 1, 0);

        chromatogram.seqLength += 1;
    }
}

}   // namespace U2
