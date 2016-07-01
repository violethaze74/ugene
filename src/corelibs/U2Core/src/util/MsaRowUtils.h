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

#ifndef _U2_MSAROWUTILS_H_
#define _U2_MSAROWUTILS_H_

#include <U2Core/global.h>
#include <U2Core/U2Ma.h>

namespace U2 {

class U2CORE_EXPORT MsaRowUtils {
public:
    static int getRowLength(const QByteArray &seq, const U2MaRowGapModel &gaps);
    static int getGapsLength(const U2MaRowGapModel &gaps);
    static char charAt(const QByteArray &seq, const U2MaRowGapModel &gaps, int pos);
    static qint64 getRowLengthWithoutTrailing(const QByteArray &seq, const U2MaRowGapModel &gaps);
    /**
     * The method maps `pos` in MSA coordinates to a character position in 'seq', i.e. gaps aren't taken into account.
     * If false == 'allowGapInPos' and the gap symbol is located in 'pos' then the method returns -1.
     * Otherwise if true == 'allowGapInPos' and the gap symbol is located in 'pos' then the method returns
     * the position of a non-gap character left-most to the 'pos'.
     */
    static int getUngappedPosition(const QList<U2MaGap> &gaps, int pos, bool allowGapInPos = false);
    static int getCoreStart(const QList<U2MaGap>& gaps);

    /**
     * Add "offset" of gaps to the beginning of the row
     * Warning: it is not verified that the row sequence is not empty.
     */
    static void addOffsetToGapModel(U2MaRowGapModel &gapModel, int offset);
    static void shiftGapModel(U2MaRowGapModel &gapModel, int shiftSize);
    static bool isGap(int sequenceLength, const U2MaRowGapModel &gapModel, int position);
    static bool isGap(const U2MaRowGapModel &gapModel, int position);
    static void chopGapModel(U2MaRowGapModel &gapModel, int maxLength);
};

} // U2

#endif // _U2_MSAROWUTILS_H_
