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
 
#include "MsaHighlightingComboBoxController.h"

#include <QStandardItemModel>

#include <U2Algorithm/MsaHighlightingScheme.h>

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>

#include <U2Gui/GroupedComboBoxDelegate.h>

#include <U2View/MSAEditor.h>

namespace U2 {

MsaHighlightingComboBoxController::MsaHighlightingComboBoxController(MSAEditor *msa, QWidget *parent) : QComboBox(parent),
    msa(msa) {
    setItemDelegate(new GroupedComboBoxDelegate(this));
    init();
    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_indexChanged(int)));
}

void MsaHighlightingComboBoxController::sl_indexChanged(int index) {
    emit si_schemeChanged(itemData(index).toString());
}

void MsaHighlightingComboBoxController::init() {
    bool isAlphabetRaw = msa->getMaObject()->getAlphabet()->getType() == DNAAlphabet_RAW;

    blockSignals(true);
    clear();
    if (isAlphabetRaw) {
        fillHighlightingCbWithGrouping();
    } else {
        MsaHighlightingSchemeRegistry *msaHighlightingSchemeRegistry = AppContext::getMsaHighlightingSchemeRegistry();
        CHECK(msaHighlightingSchemeRegistry != NULL, );
        QList<MsaHighlightingSchemeFactory*> HighlightingSchemesFactories = msaHighlightingSchemeRegistry->getSchemes(msa->getMaObject()->getAlphabet()->getType());
        MsaHighlightingSchemeFactory* nohighlightingFactory = msaHighlightingSchemeRegistry->getSchemeFactoryById(MsaHighlightingScheme::EMPTY);
        HighlightingSchemesFactories.removeAll(nohighlightingFactory);
        HighlightingSchemesFactories.prepend(nohighlightingFactory);
        foreach(MsaHighlightingSchemeFactory *factory, HighlightingSchemesFactories) {
            addItem(factory->getName(), factory->getId());
        }
    }
    blockSignals(false);
}

void MsaHighlightingComboBoxController::fillHighlightingCbWithGrouping() {

    MsaHighlightingSchemeRegistry *msaHighlightingSchemeRegistry = AppContext::getMsaHighlightingSchemeRegistry();
    QMap<AlphabetFlags, QList<MsaHighlightingSchemeFactory*> > highlightingSchemesFactories = msaHighlightingSchemeRegistry->getAllSchemesGrouped();
    MsaHighlightingSchemeFactory* nohighlightingFactory = msaHighlightingSchemeRegistry->getSchemeFactoryById(MsaHighlightingScheme::EMPTY);

    QList<MsaHighlightingSchemeFactory *> commonHighlightSchemesFactories = highlightingSchemesFactories[DNAAlphabet_RAW | DNAAlphabet_AMINO | DNAAlphabet_NUCL];
    QList<MsaHighlightingSchemeFactory *> aminoHighlightSchemesFactories = highlightingSchemesFactories[DNAAlphabet_RAW | DNAAlphabet_AMINO];
    QList<MsaHighlightingSchemeFactory *> nucleotideHighlightSchemesFactories = highlightingSchemesFactories[DNAAlphabet_RAW | DNAAlphabet_NUCL];

    commonHighlightSchemesFactories.removeAll(nohighlightingFactory);
    commonHighlightSchemesFactories.prepend(nohighlightingFactory);

    createAndFillGroup(commonHighlightSchemesFactories, tr("All alphabets"));
    createAndFillGroup(aminoHighlightSchemesFactories, tr("Amino acid alphabet"));
    createAndFillGroup(nucleotideHighlightSchemesFactories, tr("Nucleotide alphabet"));
}

void MsaHighlightingComboBoxController::createAndFillGroup(QList<MsaHighlightingSchemeFactory *> rawHighlightingSchemesFactories, const QString& groupName) {
    if (rawHighlightingSchemesFactories.isEmpty()) {
        return;
    }
    GroupedComboBoxDelegate *highlightingSchemeDelegate = qobject_cast<GroupedComboBoxDelegate*>(itemDelegate());
    QStandardItemModel *highlightingSchemeModel = qobject_cast<QStandardItemModel*>(model());
    CHECK(highlightingSchemeDelegate != NULL, );
    CHECK(highlightingSchemeModel != NULL, );
    highlightingSchemeDelegate->addParentItem(highlightingSchemeModel, groupName);
    foreach(MsaHighlightingSchemeFactory *factory, rawHighlightingSchemesFactories) {
        highlightingSchemeDelegate->addChildItem(highlightingSchemeModel, factory->getName(), factory->getId());
    }
}

};
