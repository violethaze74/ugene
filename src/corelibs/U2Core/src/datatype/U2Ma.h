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

#ifndef _U2_MA_H_
#define _U2_MA_H_

#include <U2Core/U2Alphabet.h>
#include <U2Core/U2Type.h>

namespace U2 {

/**
    Gap model for Multiple Alignment: for every row it keeps gaps map
*/

class U2MaGap;

typedef QList<U2MaGap> U2MaRowGapModel;
typedef QList<U2MaRowGapModel> U2MaListGapModel;
typedef QMap<qint64, U2MaRowGapModel> U2MaMapGapModel;

class U2CORE_EXPORT U2MaGap  {
public:
    U2MaGap();
    U2MaGap(qint64 off, qint64 gap);

    bool isValid() const;

    bool operator==(const U2MaGap &g) const;

    /** Offset of the gap in sequence*/
    qint64 offset;

    /** number of gaps */
    qint64 gap;
};

/**
    Row of multiple alignment: gaps map and sequence id
*/
class U2CORE_EXPORT U2MaRow {
public:
    U2MaRow();

    /** Id of the row in the database */
    qint64          rowId;

    /** Id of the sequence of the row in the database */
    U2DataId        dataObjectId;

    /** Start of the row in the sequence */
    qint64          gstart;

    /** End of the row in the sequence */
    qint64          gend;

    /** A gap model for the row */
    QList<U2MaGap> gaps;

    /** Length of the sequence characters and gaps of the row (without trailing) */
    qint64          length;
};

/**
    Multiple sequence alignment representation
*/
class U2CORE_EXPORT U2Ma : public U2Object {
public:
    U2Ma();
    U2Ma(const U2DataId &id, const QString &dbId, qint64 version);

    U2DataType getType() const;

    /** Alignment alphabet. All sequence in alignment must have alphabet that fits into alignment alphabet */
    U2AlphabetId    alphabet;

    /** Length of the alignment */
    qint64          length;
};

}   // namespace U2

#endif // _U2_MA_H_
