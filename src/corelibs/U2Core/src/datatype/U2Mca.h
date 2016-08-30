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

#ifndef _U2_MCA_H_
#define _U2_MCA_H_

#include "U2Msa.h"

namespace U2 {

/**
    Row of multiple chromatogram alignment: gaps map and sequence id
*/
class U2CORE_EXPORT U2McaRow : public U2MsaRow {
public:
    U2McaRow(const U2MsaRow &msaRow)
        : U2MsaRow(msaRow)
    {

    }

    U2DataId predictedSequenceId;
    U2DataId chromatogramId;
};

}   // namespace U2

#endif // _U2_MCA_H_
