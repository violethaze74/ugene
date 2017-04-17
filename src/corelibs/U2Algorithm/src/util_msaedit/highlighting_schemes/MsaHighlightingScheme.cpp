/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
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

#include "MsaHighlightingScheme.h"
#include "MsaHighlightingSchemeAgreements.h"
#include "MsaHighlightingSchemeConservation.h"
#include "MsaHighlightingSchemeDisagreements.h"
#include "MsaHighlightingSchemeGaps.h"
#include "MsaHighlightingSchemeNoColors.h"
#include "MsaHighlightingSchemeTransitions.h"
#include "MsaHighlightingSchemeTransversions.h"

namespace U2 {

const QString MsaHighlightingScheme::EMPTY          = "HIGHLIGHT_SCHEME_EMPTY";
const QString MsaHighlightingScheme::AGREEMENTS     = "HIGHLIGHT_SCHEME_AGREEMENTS";
const QString MsaHighlightingScheme::DISAGREEMENTS  = "HIGHLIGHT_SCHEME_DISAGREEMENTS";
const QString MsaHighlightingScheme::TRANSITIONS    = "HIGHLIGHT_SCHEME_TRANSITIONS";
const QString MsaHighlightingScheme::TRANSVERSIONS  = "HIGHLIGHT_SCHEME_TRANSVERSIONS";
const QString MsaHighlightingScheme::GAPS           = "HIGHLIGHT_SCHEME_GAPS";
const QString MsaHighlightingScheme::CONSERVATION   = "CONSERVATION_SCHEME_GAPS";

const QString MsaHighlightingScheme::THRESHOLD_PARAMETER_NAME           = "threshold";
const QString MsaHighlightingScheme::LESS_THAN_THRESHOLD_PARAMETER_NAME = "less_than_threshold";

MsaHighlightingScheme::MsaHighlightingScheme(QObject *parent, const MsaHighlightingSchemeFactory *factory, MultipleAlignmentObject *maObj)
    : QObject(parent),
      factory(factory),
      maObj(maObj),
      useDots(false)
{

}

void MsaHighlightingScheme::process(const char /*refChar*/, char &seqChar, QColor &/*color*/, bool &highlight, int /*refCharColumn*/, int /*refCharRow*/) const {
    if (useDots && !highlight){
        seqChar = '.';
    }
}

const MsaHighlightingSchemeFactory * MsaHighlightingScheme::getFactory() const {
    return factory;
}

void MsaHighlightingScheme::setUseDots(bool use) {
    useDots = use;
}

bool MsaHighlightingScheme::getUseDots() const {
    return useDots;
}

void MsaHighlightingScheme::applySettings(const QVariantMap &) {

}

QVariantMap MsaHighlightingScheme::getSettings() const {
    return QVariantMap();
}

MsaHighlightingSchemeFactory::MsaHighlightingSchemeFactory(QObject *parent, const QString &id, const QString &name, QList<DNAAlphabetType> &supportedAlphabetsList, bool refFree, bool needThreshold)
    : QObject(parent),
      id(id),
      name(name),
      refFree(refFree),
      needThreshold(needThreshold) {
    foreach(DNAAlphabetType alType, supportedAlphabetsList) {
        supportedAlphabets |= alType;
    }
}

const QString & MsaHighlightingSchemeFactory::getId() const {
    return id;
}

const QString MsaHighlightingSchemeFactory::getName() const {
    return name;
}

bool MsaHighlightingSchemeFactory::isRefFree() const {
    return refFree;
}

bool MsaHighlightingSchemeFactory::isNeedThreshold() const {
    return needThreshold;
}

const QFlags<DNAAlphabetType>& MsaHighlightingSchemeFactory::getSupportedAlphabets() const {
    return supportedAlphabets;
}

MsaHighlightingSchemeRegistry::MsaHighlightingSchemeRegistry() {
    schemes.append(new MsaHighlightingSchemeNoColorsFactory(this, MsaHighlightingScheme::EMPTY, tr("No highlighting"), QList<DNAAlphabetType>() << DNAAlphabet_NUCL << DNAAlphabet_AMINO << DNAAlphabet_RAW));
    schemes.append(new MsaHighlightingSchemeAgreementsFactory(this, MsaHighlightingScheme::AGREEMENTS, tr("Agreements"), QList<DNAAlphabetType>() << DNAAlphabet_NUCL << DNAAlphabet_AMINO));
    schemes.append(new MsaHighlightingSchemeDisagreementsFactory(this, MsaHighlightingScheme::DISAGREEMENTS, tr("Disagreements"), QList<DNAAlphabetType>() << DNAAlphabet_NUCL << DNAAlphabet_AMINO));
    schemes.append(new MsaHighlightingSchemeGapsFactory(this, MsaHighlightingScheme::GAPS, tr("Gaps"), QList<DNAAlphabetType>() << DNAAlphabet_NUCL << DNAAlphabet_AMINO << DNAAlphabet_RAW));
    schemes.append(new MsaHighlightingSchemeConservationFactory(this, MsaHighlightingScheme::CONSERVATION, tr("Conservation level"), QList<DNAAlphabetType>() << DNAAlphabet_NUCL << DNAAlphabet_AMINO));
    schemes.append(new MsaHighlightingSchemeTransitionsFactory(this, MsaHighlightingScheme::TRANSITIONS, tr("Transitions"), QList<DNAAlphabetType>() << DNAAlphabet_NUCL));
    schemes.append(new MsaHighlightingSchemeTransversionsFactory(this, MsaHighlightingScheme::TRANSVERSIONS, tr("Transversions"), QList<DNAAlphabetType>() << DNAAlphabet_NUCL));
}

MsaHighlightingSchemeRegistry::~MsaHighlightingSchemeRegistry() {
    qDeleteAll(schemes);
}

MsaHighlightingSchemeFactory * MsaHighlightingSchemeRegistry::getMsaHighlightingSchemeFactoryById(const QString& id) const {
    foreach (MsaHighlightingSchemeFactory *factory, schemes) {
        if (factory->getId() == id) {
            return factory;
        }
    }
    return NULL;
}

QList<MsaHighlightingSchemeFactory *> MsaHighlightingSchemeRegistry::getMsaHighlightingSchemes(DNAAlphabetType alphabetType) const {
    QList<MsaHighlightingSchemeFactory *> res;
    foreach (MsaHighlightingSchemeFactory *factory, schemes) {
        if (factory->getSupportedAlphabets().testFlag(alphabetType)) {
            res.append(factory);
        }
    }
    return res;
}

}   // namespace U2
