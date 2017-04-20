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
 
#ifndef _U2_MSA_HIGHLIGHTING_COMBO_BOX_CONTROLLER_H_
#define _U2_MSA_HIGHLIGHTING_COMBO_BOX_CONTROLLER_H_

#include <QComboBox>

class QStandardItemModel;

namespace U2 {

class MSAEditor;
class MsaHighlightingSchemeFactory;
class GroupedComboBoxDelegate;

class MSAHighlightingComboboxController : public QComboBox {
    Q_OBJECT
public:
    MSAHighlightingComboboxController(MSAEditor *msa, QWidget *parent = NULL);
    void init();
signals:
    void si_schemeChanged(const QString &newScheme);
private slots:
    void sl_indexChanged(int index);
private:
    void fillHighlightingCbWithGrouping(const QList<MsaHighlightingSchemeFactory *> &colorSchemesFactories);
    void createAndFillGroup(QList<MsaHighlightingSchemeFactory *> rawColorSchemesFactories, const QString& groupName);

    MSAEditor *msa;
};

}

#endif
