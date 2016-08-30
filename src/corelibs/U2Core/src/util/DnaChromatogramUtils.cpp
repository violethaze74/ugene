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

#include <U2Core/DNAChromatogram.h>

#include "DnaChromatogramUtils.h"

namespace U2 {

void DnaChromatogramUtils::append(DNAChromatogram &chromatogram, const DNAChromatogram &appendedChromatogram) {
    chromatogram.traceLength += appendedChromatogram.traceLength;
    chromatogram.seqLength += appendedChromatogram.seqLength;
    chromatogram.baseCalls += appendedChromatogram.baseCalls;
    chromatogram.A += appendedChromatogram.A;
    chromatogram.C += appendedChromatogram.C;
    chromatogram.G += appendedChromatogram.G;
    chromatogram.T += appendedChromatogram.T;
    chromatogram.prob_A += appendedChromatogram.prob_A;
    chromatogram.prob_C += appendedChromatogram.prob_C;
    chromatogram.prob_G += appendedChromatogram.prob_G;
    chromatogram.hasQV &= appendedChromatogram.hasQV;
}

}   // namespace U2
