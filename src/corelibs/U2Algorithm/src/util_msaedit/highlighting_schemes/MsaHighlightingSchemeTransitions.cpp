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

#include "MsaHighlightingSchemeTransitions.h"

namespace U2 {

MsaHighlightingSchemeTransitions::MsaHighlightingSchemeTransitions(QObject *parent, const MsaHighlightingSchemeFactory *factory, MAlignmentObject *maObj)
    : MsaHighlightingScheme(parent, factory, maObj)
{

}

void MsaHighlightingSchemeTransitions::process(const char refChar, char &seqChar, bool &color, int refCharColumn, int refCharRow) const {
    switch (refChar) {
    case 'N':
        color = true;
        break;
    case 'A':
        color = (seqChar == 'G');
        break;
    case 'C':
        color = (seqChar == 'T');
        break;
    case 'G':
        color = (seqChar == 'A');
        break;
    case 'T':
        color = (seqChar == 'C');
        break;
    default:
        color = false;
        break;
    }
    MsaHighlightingScheme::process(refChar, seqChar, color, refCharColumn, refCharRow);
}

MsaHighlightingSchemeTransitionsFactory::MsaHighlightingSchemeTransitionsFactory(QObject *parent, const QString &id, const QString &name, DNAAlphabetType alphabetType)
    : MsaHighlightingSchemeFactory(parent, id, name, alphabetType)
{

}

MsaHighlightingScheme * MsaHighlightingSchemeTransitionsFactory::create(QObject *parent, MAlignmentObject *maObj ) const {
    return new MsaHighlightingSchemeTransitions(parent, this, maObj);
}

}   // namespace U2
