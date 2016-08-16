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

#include "U2AssemblyUtils.h"
#include "dbi/U2AssemblyDbi.h"

#include <U2Core/DbiConnection.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/QRegExp>

namespace U2 {

const int U2AssemblyUtils::MAX_COVERAGE_VECTOR_SIZE = 1000*1000;

U2CigarOp U2AssemblyUtils::char2Cigar(char c, QString& err) {
    char cu = TextUtils::UPPER_CASE_MAP[c];
    switch (cu) {
        case 'D':
            return U2CigarOp_D; // deleted
        case 'I':
            return U2CigarOp_I; // inserted
        case 'H':
            return U2CigarOp_H; // hard-clipped
        case 'M':
            return U2CigarOp_M; // matched
        case 'N':
            return U2CigarOp_N; // skipped
        case 'P':
            return U2CigarOp_P; // padded
        case 'S':
            return U2CigarOp_S; // soft-clipped
        case '=':
            return U2CigarOp_EQ; // sequence match
        case 'X':
            return U2CigarOp_X; // sequence mismatch
    }
    err = tr("Invalid CIGAR op: '%1'!").arg(c);
    return U2CigarOp_Invalid;
}

char U2AssemblyUtils::cigar2Char(U2CigarOp op) {
    char c;
    switch(op) {
            case U2CigarOp_D: c = 'D'; break;
            case U2CigarOp_I: c = 'I'; break;
            case U2CigarOp_H: c = 'H'; break;
            case U2CigarOp_M: c = 'M'; break;
            case U2CigarOp_N: c = 'N'; break;
            case U2CigarOp_P: c = 'P'; break;
            case U2CigarOp_S: c = 'S'; break;
            case U2CigarOp_EQ:c = '='; break;
            case U2CigarOp_X: c = 'X'; break;
            default: assert(0); c = '?';
    }
    return c;
}

QList<U2CigarToken> U2AssemblyUtils::parseCigar(const QByteArray& cigarString, QString& err) {
    QList<U2CigarToken> result;
    int pos = 0, len = cigarString.length();
    const char* cigar = cigarString.constData();
    int count = 0;
    // count numbers
    while (pos < len ) {
        char c = cigar[pos++];
        if (c >= '0' && c <= '9') {
            int n = c - '0';
            count = count * 10 + n;
            continue;
        }
        U2CigarOp op = char2Cigar(c, err);
        if (!err.isEmpty()) {
            break;
        }
        result.append(U2CigarToken(op, count));
        count = 0;
    }
    return result;
}


QByteArray U2AssemblyUtils::cigar2String(const QList<U2CigarToken>& cigar) {
    QByteArray res;
    foreach(const U2CigarToken& t, cigar) {
        if (t.op != U2CigarOp_Invalid) {
            res = res + QByteArray::number(t.count) + cigar2Char(t.op);
        }
    }
    return res;
}

qint64 U2AssemblyUtils::getEffectiveReadLength(const U2AssemblyRead& read) {
    return read->readSequence.length() + getCigarExtraLength(read->cigar);
}

qint64 U2AssemblyUtils::getCigarExtraLength(const QList<U2CigarToken>& cigar) {
    qint64 res = 0;
    foreach(const U2CigarToken& t, cigar) {
        switch(t.op) {
            case U2CigarOp_I:
            case U2CigarOp_S:
                res-=t.count;
                break;
            case U2CigarOp_D:
            case U2CigarOp_N:
                res+=t.count;
                break;
            default:;
        }
    }
    return res;
}

static QByteArray prepareCigarChars() {
    return QByteArray("0123456789DIHMNPS=X");
}

QByteArray U2AssemblyUtils::getCigarAlphabetChars() {
    static QByteArray res = prepareCigarChars();
    return res;
}

QByteArray U2AssemblyUtils::serializeCoverageStat(const U2AssemblyCoverageStat& coverageStat) {
    QByteArray data;
    for(int index = 0;index < coverageStat.coverage->size();index++) {
        for(int i = 0;i < 4;i++) {
            data.append((char)(coverageStat.coverage->at(index).maxValue >> (i*8)));
        }
    }
    return data;
}

void U2AssemblyUtils::deserializeCoverageStat(QByteArray data, U2AssemblyCoverageStat& res, U2OpStatus &os) {
    res.coverage->clear();
    if(!data.isEmpty() && 0 == (data.size() % 4)) {
        for(int index = 0;index < data.size()/4;index++) {
            int value = 0;
            for(int i = 0;i < 4;i++) {
                value |= ((int)data[index*4 + i] & 0xff) << (i*8);
            }
            res.coverage->append(U2Range<int>(value, value));
        }
    } else {
        os.setError("Invalid attribute size");
    }
}

QVector<qint64> U2AssemblyUtils::coverageStatToVector(const U2AssemblyCoverageStat &coverageStat) {
    int size = coverageStat.coverage->size();
    QVector<qint64> res(size);
    for(int i = 0; i < size; ++i) {
        res[i] = coverageStat.coverage->at(i).maxValue;
    }
    return res;
}

U2CigarOp nextCigarOp(const QByteArray &cigarString, int &index, int &insertionsCount, U2OpStatus &os) {
    QString errString;
    U2CigarOp cigarOp = U2CigarOp_Invalid;

    do {
        SAFE_POINT_EXT(index < cigarString.length(), os.setError(U2AssemblyUtils::tr("Cigar string: out of bounds")), U2CigarOp_Invalid);
        cigarOp = U2AssemblyUtils::char2Cigar(cigarString[index], errString);
        SAFE_POINT_EXT(errString.isEmpty(), os.setError(errString), U2CigarOp_Invalid);

        index++;

        if (U2CigarOp_I == cigarOp || U2CigarOp_S == cigarOp) {
            insertionsCount++;
        }
    } while (U2CigarOp_I == cigarOp || U2CigarOp_S == cigarOp || U2CigarOp_P == cigarOp);

    return cigarOp;
}

void U2AssemblyUtils::calculateCoveragePerBase(const U2DbiRef &dbiRef, const U2DataId &assemblyId, const U2Region &region, QVector<CoveragePerBaseInfo> *results, U2OpStatus &os) {
    DbiConnection con(dbiRef, os);
    CHECK_OP(os, );
    U2AssemblyDbi *assemblyDbi = con.dbi->getAssemblyDbi();
    SAFE_POINT_EXT(NULL != assemblyDbi, os.setError(tr("Assembly DBI is NULL")), );

    results->resize(region.length);

    QScopedPointer<U2DbiIterator<U2AssemblyRead> > readsIterator(assemblyDbi->getReads(assemblyId, region, os));
    while (readsIterator->hasNext()) {
        const U2AssemblyRead read = readsIterator->next();
        const qint64 startPos = qMax(read->leftmostPos, region.startPos);
        const qint64 endPos = qMin(read->leftmostPos + read->effectiveLen, region.endPos());
        const U2Region regionToProcess = U2Region(startPos, endPos - startPos);

        // we have used effective length of the read, so insertions/deletions are already taken into account
        // cigarString can be longer than needed
        QByteArray cigarString;
        foreach(const U2CigarToken &cigar, read->cigar) {
            cigarString += QByteArray(cigar.count, U2AssemblyUtils::cigar2Char(cigar.op));
        }

        if (read->leftmostPos < regionToProcess.startPos) {
            cigarString = cigarString.mid(regionToProcess.startPos - read->leftmostPos);
        }

        for (int positionOffset = 0, cigarOffset = 0, deletionsCount = 0, insertionsCount = 0; regionToProcess.startPos + positionOffset < regionToProcess.endPos(); positionOffset++) {
            char currentBase = 'N';
            CoveragePerBaseInfo &info = (*results)[regionToProcess.startPos + positionOffset - region.startPos];
            U2CigarOp cigarOp = nextCigarOp(cigarString, cigarOffset, insertionsCount, os);
            CHECK_OP(os, );
            

            switch (cigarOp) {
            case U2CigarOp_I:
            case U2CigarOp_S:
                // skip the insertion
                continue;
            case U2CigarOp_D:
                // skip the deletion
                deletionsCount++;
                continue;
            case U2CigarOp_N:
                // skip the deletion
                deletionsCount++;
                continue;
            default:
                currentBase = read->readSequence[positionOffset - deletionsCount + insertionsCount];
                break;
            }
            info.basesCount[currentBase] = info.basesCount[currentBase] + 1;
            info.coverage++;
        }
        CHECK_OP(os, );
    }
}

} //namespace
