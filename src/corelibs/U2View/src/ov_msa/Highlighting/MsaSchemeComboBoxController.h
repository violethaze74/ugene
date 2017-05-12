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

#ifndef _U2_MSA_COMBO_BOX_CONTROLLER_H_
#define _U2_MSA_COMBO_BOX_CONTROLLER_H_

#include <QComboBox>

class QStandardItemModel;

namespace U2 {

class MSAEditor;
class GroupedComboBoxDelegate;

template <class Factory, class Registry>
class MsaSchemeComboBoxController {
public:
    MsaSchemeComboBoxController(MSAEditor *msa, Registry *registry, QWidget *parent = NULL);
    void init();
signals:
    void si_schemeChanged(const QString &newScheme);
private slots:
    void sl_indexChanged(int index);
private:
    void fillCbWithGrouping();
    void createAndFillGroup(QList<Factory *> rawSchemesFactories, const QString& groupName);

    MSAEditor *msa;
    Registry *registry;
    QComboBox *comboBox;
};

template <class Factory, class Registry>
MsaSchemeComboBoxController<Factory, Registry>::MsaSchemeComboBoxController(MSAEditor *msa, Registry *registry, QWidget *parent /*= NULL*/) 
    : msa(msa), registry(registry) {
    comboBox(parent);
    CHECK(registry != NULL, );
    setItemDelegate(new GroupedComboBoxDelegate(this));
    init();
    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_indexChanged(int)));
}

template <class Factory, class Registry>
void MsaSchemeComboBoxController<Factory, Registry>::init() {
    bool isAlphabetRaw = msa->getMaObject()->getAlphabet()->getType() == DNAAlphabet_RAW;

    comboBox->blockSignals(true);
    clear();
    if (isAlphabetRaw) {
        fillCbWithGrouping();
    } else {
        QList<Factory *> schemesFactories = registry->getAllSchemes(msa->getMaObject()->getAlphabet()->getType());
        Factory* emptySchemeFactory = registry->getEmptySchemeFactory();
        schemesFactories.removeAll(emptySchemeFactory);
        schemesFactories.prepend(emptySchemeFactory);
        foreach(Factory *factory, schemesFactories) {
            addItem(factory->getName(), factory->getId());
        }
    }
    comboBox->blockSignals(false);
}

template <class Factory, class Registry>
void MsaSchemeComboBoxController<Factory, Registry>::sl_indexChanged(int index) {
    emit si_schemeChanged(itemData(index).toString());
}

template <class Factory, class Registry>
void MsaSchemeComboBoxController<Factory, Registry>::fillCbWithGrouping() {
    QMap<AlphabetFlags, QList<Factory*> > schemesFactories = registry->getAllSchemesGrouped();
    MsaHighlightingSchemeFactory* emptySchemeFactory = registry->getSchemeFactoryById(MsaHighlightingScheme::EMPTY);

    QList<Factory *> commonSchemesFactories = schemesFactories[DNAAlphabet_RAW | DNAAlphabet_AMINO | DNAAlphabet_NUCL];
    QList<Factory *> aminoSchemesFactories = schemesFactories[DNAAlphabet_RAW | DNAAlphabet_AMINO];
    QList<Factory *> nucleotideSchemesFactories = schemesFactories[DNAAlphabet_RAW | DNAAlphabet_NUCL];

    commonSchemesFactories.removeAll(emptySchemeFactory);
    commonSchemesFactories.prepend(emptySchemeFactory);

    createAndFillGroup(commonSchemesFactories, tr("All alphabets"));
    createAndFillGroup(aminoSchemesFactories, tr("Amino acid alphabet"));
    createAndFillGroup(nucleotideSchemesFactories, tr("Nucleotide alphabet"));
}

template <class Factory, class Registry>
void MsaSchemeComboBoxController<Factory, Registry>::createAndFillGroup(QList<Factory *> rawSchemesFactories, const QString& groupName) {
    if (rawSchemesFactories.isEmpty()) {
        return;
    }
    GroupedComboBoxDelegate *schemeDelegate = qobject_cast<GroupedComboBoxDelegate*>(itemDelegate());
    QStandardItemModel *schemeModel = qobject_cast<QStandardItemModel*>(model());
    CHECK(schemeDelegate != NULL, );
    CHECK(schemeModel != NULL, );
    schemeDelegate->addParentItem(schemeModel, groupName);
    foreach(MsaHighlightingSchemeFactory *factory, rawSchemesFactories) {
        schemeDelegate->addChildItem(schemeModel, factory->getName(), factory->getId());
    }
}

}

#endif
