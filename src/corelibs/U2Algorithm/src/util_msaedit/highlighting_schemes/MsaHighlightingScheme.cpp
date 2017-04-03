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

const QString MsaHighlightingScheme::EMPTY_NUCL             = "HIGHLIGHT_SCHEME_EMPTY_NUCL";
const QString MsaHighlightingScheme::EMPTY_AMINO            = "HIGHLIGHT_SCHEME_EMPTY_AMINO";
const QString MsaHighlightingScheme::EMPTY_RAW              = "HIGHLIGHT_SCHEME_EMPTY_RAW";
const QString MsaHighlightingScheme::AGREEMENTS_NUCL        = "HIGHLIGHT_SCHEME_AGREEMENTS_NUCL";
const QString MsaHighlightingScheme::AGREEMENTS_AMINO       = "HIGHLIGHT_SCHEME_AGREEMENTS_AMINO";
const QString MsaHighlightingScheme::DISAGREEMENTS_NUCL     = "HIGHLIGHT_SCHEME_DISAGREEMENTS_NUCL";
const QString MsaHighlightingScheme::DISAGREEMENTS_AMINO    = "HIGHLIGHT_SCHEME_DISAGREEMENTS_AMINO";
const QString MsaHighlightingScheme::TRANSITIONS_NUCL       = "HIGHLIGHT_SCHEME_TRANSITIONS_AMINO";
const QString MsaHighlightingScheme::TRANSVERSIONS_NUCL     = "HIGHLIGHT_SCHEME_TRANSVERSIONS_AMINO";
const QString MsaHighlightingScheme::GAPS_AMINO             = "HIGHLIGHT_SCHEME_GAPS_AMINO";
const QString MsaHighlightingScheme::GAPS_NUCL              = "HIGHLIGHT_SCHEME_GAPS_NUCL";
const QString MsaHighlightingScheme::GAPS_RAW               = "HIGHLIGHT_SCHEME_GAPS_RAW";
const QString MsaHighlightingScheme::CONSERVATION_AMINO     = "CONSERVATION_SCHEME_GAPS_AMINO";
const QString MsaHighlightingScheme::CONSERVATION_NUCL      = "CONSERVATION_SCHEME_GAPS_NUCL";

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

MsaHighlightingSchemeFactory::MsaHighlightingSchemeFactory(QObject *parent, const QString &id, const QString &name, DNAAlphabetType alphabetType, bool refFree, bool needThreshold)
    : QObject(parent),
      id(id),
      name(name),
      alphabetType(alphabetType),
      refFree(refFree),
      needThreshold(needThreshold)
{

}

const QString & MsaHighlightingSchemeFactory::getId() const {
    return id;
}

const QString MsaHighlightingSchemeFactory::getName(bool nameWithAlphabet) const {
    if (nameWithAlphabet) {
        if (alphabetType == DNAAlphabet_NUCL) {
            return tr("Nucleotide") + " " + name;
        } else if (alphabetType == DNAAlphabet_AMINO) {
            return  tr("Amino") + " " + name;
        }
    }
    return name;
}

DNAAlphabetType MsaHighlightingSchemeFactory::getAlphabetType() const {
    return alphabetType;
}

bool MsaHighlightingSchemeFactory::isRefFree() const {
    return refFree;
}

bool MsaHighlightingSchemeFactory::isNeedThreshold() const {
    return needThreshold;
}

MsaHighlightingSchemeRegistry::MsaHighlightingSchemeRegistry(){
    schemes.append(new MsaHighlightingSchemeNoColorsFactory(this, MsaHighlightingScheme::EMPTY_NUCL, tr("No highlighting"), DNAAlphabet_NUCL));
    schemes.append(new MsaHighlightingSchemeNoColorsFactory(this, MsaHighlightingScheme::EMPTY_AMINO, tr("No highlighting"), DNAAlphabet_AMINO));
    schemes.append(new MsaHighlightingSchemeNoColorsFactory(this, MsaHighlightingScheme::EMPTY_RAW, tr("No highlighting"), DNAAlphabet_RAW));

    schemes.append(new MsaHighlightingSchemeAgreementsFactory(this, MsaHighlightingScheme::AGREEMENTS_NUCL, tr("Agreements"), DNAAlphabet_NUCL));
    schemes.append(new MsaHighlightingSchemeAgreementsFactory(this, MsaHighlightingScheme::AGREEMENTS_AMINO, tr("Agreements"), DNAAlphabet_AMINO));

    schemes.append(new MsaHighlightingSchemeDisagreementsFactory(this, MsaHighlightingScheme::DISAGREEMENTS_NUCL, tr("Disagreements"), DNAAlphabet_NUCL));
    schemes.append(new MsaHighlightingSchemeDisagreementsFactory(this, MsaHighlightingScheme::DISAGREEMENTS_AMINO, tr("Disagreements"), DNAAlphabet_AMINO));

    schemes.append(new MsaHighlightingSchemeGapsFactory(this, MsaHighlightingScheme::GAPS_NUCL, tr("Gaps"), DNAAlphabet_NUCL));
    schemes.append(new MsaHighlightingSchemeGapsFactory(this, MsaHighlightingScheme::GAPS_AMINO, tr("Gaps"), DNAAlphabet_AMINO));
    schemes.append(new MsaHighlightingSchemeGapsFactory(this, MsaHighlightingScheme::GAPS_RAW, tr("Gaps"), DNAAlphabet_RAW));

    schemes.append(new MsaHighlightingSchemeConservationFactory(this, MsaHighlightingScheme::CONSERVATION_NUCL, tr("Conservation level"), DNAAlphabet_NUCL));
    schemes.append(new MsaHighlightingSchemeConservationFactory(this, MsaHighlightingScheme::CONSERVATION_AMINO, tr("Conservation level"), DNAAlphabet_AMINO));

    schemes.append(new MsaHighlightingSchemeTransitionsFactory(this, MsaHighlightingScheme::TRANSITIONS_NUCL, tr("Transitions"), DNAAlphabet_NUCL));
    schemes.append(new MsaHighlightingSchemeTransversionsFactory(this, MsaHighlightingScheme::TRANSVERSIONS_NUCL, tr("Transversions"), DNAAlphabet_NUCL));
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

QStringList MsaHighlightingSchemeRegistry::getExcludedIdsFromRawAlphabetSchemes() {
    static QStringList res;
    if (res.isEmpty()) {
        res << MsaHighlightingScheme::EMPTY_AMINO;
        res << MsaHighlightingScheme::EMPTY_NUCL;
        res << MsaHighlightingScheme::GAPS_AMINO;
        res << MsaHighlightingScheme::GAPS_NUCL;
    }
    return res;
}

QList<MsaHighlightingSchemeFactory *> MsaHighlightingSchemeRegistry::getMsaHighlightingSchemes(DNAAlphabetType alphabetType) const {
    QList<MsaHighlightingSchemeFactory *> res;
    if (alphabetType == DNAAlphabet_RAW) {
        foreach(MsaHighlightingSchemeFactory *factory, schemes) {
            QString fId = factory->getId();
            if (getExcludedIdsFromRawAlphabetSchemes().contains(fId)) {
                continue;
            }
            res.append(factory);
        }
        return res;
    }
    foreach (MsaHighlightingSchemeFactory *factory, schemes) {
        if (factory->getAlphabetType() == alphabetType) {
            res.append(factory);
        }
    }
    return res;
}

}   // namespace U2
