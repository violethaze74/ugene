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

#ifndef _U2_CHROMATOGRAM_UTILS_H_
#define _U2_CHROMATOGRAM_UTILS_H_

#include <U2Core/DNAChromatogramObject.h>
#include <U2Core/U2Type.h>

namespace U2 {

class U2CORE_EXPORT ChromatogramUtils {
public:
    static void append(DNAChromatogram chromatogram, const DNAChromatogram &appendedChromatogram);
    static void remove(U2OpStatus &os, DNAChromatogram &chromatogram, int startPos, int endPos);
    static bool areEqual(const DNAChromatogram &first, const DNAChromatogram &second);
    static void crop(DNAChromatogram &chromatogram, int startPos, int length);
    static U2EntityRef import(U2OpStatus &os, const U2DbiRef &dbiRef, const QString &folder, const DNAChromatogram &chromatogram);
    static DNAChromatogram exportChromatogram(U2OpStatus &os, const U2EntityRef &chromatogramRef);
    static U2Chromatogram getChromatogramDbInfo(U2OpStatus &os, const U2EntityRef &chromatogramRef);
    static qint64 getChromatogramLength(U2OpStatus &os, const U2EntityRef &chromatogramRef);
    static void updateChromtogramData(U2OpStatus &os, const U2EntityRef &chromatogramRef, const DNAChromatogram &chromatogram);
    static U2EntityRef getChromatogramIdByRelatedSequenceId(U2OpStatus &os, const U2EntityRef &sequenceRef);
    static QString getChromatogramName(U2OpStatus &os, const U2EntityRef &chromatogramRef);
};

}   // namespace U2

#endif // _U2_CHROMATOGRAM_UTILS_H_
