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

#include "MSAColorComboboxController.h"

#include <QStandardItemModel>

#include <U2Algorithm/MsaColorScheme.h>

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>

#include <U2Gui/GroupedComboboxDelegate.h>

#include <U2View/MSAEditor.h>

namespace U2 {

MsaColorComboboxController::MsaColorComboboxController(MSAEditor *msa, QWidget *parent) : QComboBox(parent),
    msa(msa) {
    setItemDelegate(new GroupedComboBoxDelegate(this));
    init();
    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_indexChanged(int)));
}

void MsaColorComboboxController::sl_indexChanged(int index) {
    emit si_schemeChanged(itemData(index).toString());
}

void MsaColorComboboxController::init() {
    bool isAlphabetRaw = msa->getMaObject()->getAlphabet()->getType() == DNAAlphabet_RAW;

    blockSignals(true);

    MsaColorSchemeRegistry *msaColorSchemeRegistry = AppContext::getMsaColorSchemeRegistry();
    QList<MsaColorSchemeFactory*> colorSchemesFactories = msaColorSchemeRegistry->getAllMsaColorSchemes(msa->getMaObject()->getAlphabet()->getType());

    clear();
    if (isAlphabetRaw) {
        fillColorCbWithGrouping();
    } else {
        foreach(MsaColorSchemeFactory *factory, colorSchemesFactories) {
            addItem(factory->getName(), factory->getId());
        }
    }
    blockSignals(false);
}

void MsaColorComboboxController::fillColorCbWithGrouping() {
    MsaColorSchemeRegistry *msaColorSchemeRegistry = AppContext::getMsaColorSchemeRegistry();
    QMap<AlphabetFlags, QList<MsaColorSchemeFactory*> > colorSchemesFactories = msaColorSchemeRegistry->getAllMsaColorSchemesGrouped();

    QList<MsaColorSchemeFactory *> rawColorSchemesFactories = colorSchemesFactories[DNAAlphabet_RAW | DNAAlphabet_AMINO | DNAAlphabet_NUCL];
    QList<MsaColorSchemeFactory *> aminoColorSchemesFactories = colorSchemesFactories[DNAAlphabet_RAW | DNAAlphabet_AMINO];
    QList<MsaColorSchemeFactory *> nucleotideColorSchemesFactories = colorSchemesFactories[DNAAlphabet_RAW | DNAAlphabet_NUCL];

    createAndFillGroup(rawColorSchemesFactories, tr("RAW alphabet"));
    createAndFillGroup(aminoColorSchemesFactories, tr("Amino acid alphabet"));
    createAndFillGroup(nucleotideColorSchemesFactories, tr("Nucleotide alphabet"));
}

void MsaColorComboboxController::createAndFillGroup(QList<MsaColorSchemeFactory *> rawColorSchemesFactories, const QString& groupName) {
    GroupedComboBoxDelegate *colorSchemeDelegate = qobject_cast<GroupedComboBoxDelegate*>(itemDelegate());
    QStandardItemModel *colorSchemeModel = qobject_cast<QStandardItemModel*>(model());
    CHECK(colorSchemeDelegate != NULL, );
    CHECK(colorSchemeModel != NULL, );
    colorSchemeDelegate->addParentItem(colorSchemeModel, groupName);
    foreach(MsaColorSchemeFactory *factory, rawColorSchemesFactories) {
        colorSchemeDelegate->addChildItem(colorSchemeModel, factory->getName(), factory->getId());
    }
}

};
