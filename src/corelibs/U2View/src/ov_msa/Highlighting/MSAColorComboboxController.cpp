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

MSAColorComboboxController::MSAColorComboboxController(MSAEditor *msa, QWidget *parent) : QComboBox(parent),
    msa(msa) {
    setItemDelegate(new GroupedComboBoxDelegate(this));
    init();
    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_indexChanged(int)));
}

void MSAColorComboboxController::sl_indexChanged(int index) {
    emit si_schemeChanged(itemData(index).toString());
}

void MSAColorComboboxController::init() {
    bool isAlphabetRaw = msa->getMaObject()->getAlphabet()->getType() == DNAAlphabet_RAW;

    blockSignals(true);

    MsaColorSchemeRegistry *msaColorSchemeRegistry = AppContext::getMsaColorSchemeRegistry();
    QList<MsaColorSchemeFactory*> colorSchemesFactories = msaColorSchemeRegistry->getAllMsaColorSchemes(msa->getMaObject()->getAlphabet()->getType());

    clear();
    if (isAlphabetRaw) {
        fillColorCbWithGrouping(colorSchemesFactories);
    } else {
        foreach(MsaColorSchemeFactory *factory, colorSchemesFactories) {
            addItem(factory->getName(), factory->getId());
        }
    }
    blockSignals(false);
}

void MSAColorComboboxController::fillColorCbWithGrouping(const QList<MsaColorSchemeFactory *> &colorSchemesFactories) {
    QList<MsaColorSchemeFactory *> rawColorSchemesFactories;
    QList<MsaColorSchemeFactory *> aminoColorSchemesFactories;
    QList<MsaColorSchemeFactory *> nucleotideColorSchemesFactories;
    foreach(MsaColorSchemeFactory *factory, colorSchemesFactories) {
        if (factory->isAlphabetFit(DNAAlphabet_RAW)) {
            rawColorSchemesFactories.append(factory);
        } else if (factory->isAlphabetFit(DNAAlphabet_AMINO)) {
            aminoColorSchemesFactories.append(factory);
        } else if (factory->isAlphabetFit(DNAAlphabet_NUCL)) {
            nucleotideColorSchemesFactories.append(factory);
        }
    }

    createAndFillGroup(rawColorSchemesFactories, tr("RAW alphabet"));
    createAndFillGroup(aminoColorSchemesFactories, tr("Amino acid alphabet"));
    createAndFillGroup(nucleotideColorSchemesFactories, tr("Nucleotide alphabet"));
}

void MSAColorComboboxController::createAndFillGroup(QList<MsaColorSchemeFactory *> rawColorSchemesFactories, const QString& groupName) {
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
