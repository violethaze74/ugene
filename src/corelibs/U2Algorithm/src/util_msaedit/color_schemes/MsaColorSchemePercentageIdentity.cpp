/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
 * http://ugene.net
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

#include <QColor>

#include <U2Algorithm/MSAConsensusUtils.h>

#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentObject.h>

#include "MsaColorSchemePercentageIdentity.h"

namespace U2 {

MsaColorSchemePercentageIdentity::MsaColorSchemePercentageIdentity(QObject *parent, const MsaColorSchemeFactory *factory, MAlignmentObject *maObj)
    : MsaColorScheme(parent, factory, maObj),
      cacheVersion(0),
      objVersion(1)
{
    mask4[0]=81;
    mask4[1]=61;
    mask4[2]=41;
    mask4[3]=25;

    colorsByRange[0] = QColor("#6464FF");
    colorsByRange[1] = QColor("#9999FF");
    colorsByRange[2] = QColor("#CCCCFF");
    colorsByRange[3] = QColor();

    connect(maObj, SIGNAL(si_alignmentChanged(const MAlignment &, const MAlignmentModInfo &)), SLOT(sl_alignmentChanged()));
}

QColor MsaColorSchemePercentageIdentity::getColor(int /*seq*/, int pos, char c) const {
    updateCache();
    if (c == MAlignment_GapChar) {
        return QColor();
    }
    quint32 packedVal = indentCache[pos];
    MSAConsensusUtils::unpackConsensusCharsFromInt(packedVal, tmpChars, tmpRanges);
    for (int i=0; i < 4; i++) {
        if (c == tmpChars[i]) {
            int range = tmpRanges[i];
            return colorsByRange[range];
        }
    }
    return QColor();
}

void MsaColorSchemePercentageIdentity::sl_alignmentChanged() {
    objVersion++;
}

void MsaColorSchemePercentageIdentity::updateCache() const {
    if (cacheVersion == objVersion) {
        return;
    }
    const MAlignment& ma = maObj->getMAlignment();
    int aliLen = ma.getLength();
    indentCache.resize(aliLen);
    for (int i = 0; i < aliLen; i++) {
        indentCache[i] = MSAConsensusUtils::packConsensusCharsToInt(ma, i, mask4, true);
    }
    cacheVersion = objVersion;
}

MsaColorSchemePercentageIdentityFactory::MsaColorSchemePercentageIdentityFactory(QObject *parent, const QString &id, const QString &name, const DNAAlphabetTypes &alphabetTypes)
    : MsaColorSchemeFactory(parent, id, name, alphabetTypes)
{

}

MsaColorScheme * MsaColorSchemePercentageIdentityFactory::create(QObject *parent, MAlignmentObject *maObj) const {
    return new MsaColorSchemePercentageIdentity(parent, this, maObj);
}

}   // namespace U2
