/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2020 UniPro <ugene@unipro.ru>
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

#include "MsaColorSchemeWeakSimilarities.h"

namespace U2 {

const QList<QPair<QColor, QColor>> MsaColorSchemeWeakSimilarities::colorPairsByFrequence = {QPair<QColor, QColor>(QColor("#00FFFF"), QColor("#0000FF")), 
                                                                                            QPair<QColor, QColor>(QColor("#FF00FF"), QColor("#FFFFFF")), 
                                                                                            QPair<QColor, QColor>(QColor("#000000"), QColor("#FFFFFF")), 
                                                                                            QPair<QColor, QColor>(QColor("#000000"), QColor("#C0C0C0")), 
                                                                                            QPair<QColor, QColor>(QColor("#FF6600"), QColor("#FFFFFF"))};


 MsaColorSchemeWeakSimilarities::MsaColorSchemeWeakSimilarities(QObject *parent, const MsaColorSchemeFactory *factory, MultipleAlignmentObject *maObj)
    : MsaColorScheme(parent, factory, maObj) {
 }

QColor MsaColorSchemeWeakSimilarities::getBackgroundColor(int seq, int pos, char c) const {
    return QColor();
 }

QColor MsaColorSchemeWeakSimilarities::getFontColor(int seq, int pos, char c) const {
    return QColor();
 }

void MsaColorSchemeWeakSimilarities::sl_alignmentChanged() {
 }

 MsaColorSchemeWeakSimilaritiesFactory::MsaColorSchemeWeakSimilaritiesFactory(QObject *parent, const QString &id, const QString &name, const AlphabetFlags &supportedAlphabets)
     : MsaColorSchemeFactory(parent, id, name, supportedAlphabets) {
 }

MsaColorScheme *MsaColorSchemeWeakSimilaritiesFactory::create(QObject *parent, MultipleAlignmentObject *maObj) const {
    return new MsaColorSchemeWeakSimilarities(parent, this, maObj);
 }

 }    // namespace U2