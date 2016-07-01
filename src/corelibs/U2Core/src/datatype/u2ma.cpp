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

#include "U2Ma.h"

namespace U2 {

U2MaGap::U2MaGap()
    : offset(0),
      gap(0)
{

}

U2MaGap::U2MaGap(qint64 off, qint64 gap)
    : offset(off),
      gap(gap)
{

}

bool U2MaGap::isValid() const {
    return ((offset >= 0) && (gap > 0));
}

bool U2MaGap::operator==(const U2MaGap &g) const {
    return ((offset == g.offset) && (gap == g.gap));
}

bool U2MaGap::lessThan(const U2MaGap &first, const U2MaGap &second) {
    return first.offset < second.offset;
}

U2MaRow::U2MaRow()
    : rowId(-1),
      gstart(0),
      gend(0),
      length(0)
{

}

U2Ma::U2Ma()
    : length(0)
{

}

U2Ma::U2Ma(const U2DataId &id, const QString &dbId, qint64 version)
    : U2Object(id, dbId, version)
{

}

U2DataType U2Ma::getType() const {
    return U2Type::Msa;
}

}   // namespace U2
