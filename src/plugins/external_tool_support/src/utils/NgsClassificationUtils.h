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

#ifndef _U2_NGS_CLASSIFICATION_UTILS_H_
#define _U2_NGS_CLASSIFICATION_UTILS_H_

#include <QObject>

namespace U2 {

class NgsClassificationUtils : public QObject {
public:
    static void initData();

    static const QString TAXONOMY_PATH;
    static const QString TAXONOMY_DATA_ID;
    static const QString TAXON_PROTEIN_MAP;
    static const QString TAXON_NODES;

    static const QString MINIKRAKEN_4_GB_PATH;
    static const QString MINIKRAKEN_4_GB_ID;

    static const QString DIAMOND_UNIPROT_50_DATABASE_PATH;
    static const QString DIAMOND_UNIPROT_50_DATABASE_ID;

    static const QString DIAMOND_UNIPROT_90_DATABASE_PATH;
    static const QString DIAMOND_UNIPROT_90_DATABASE_ID;

    static const QString REFSEQ_HUMAN_PATH;
    static const QString REFSEQ_HUMAN_ID;

    static const QString REFSEQ_BACTERIA_PATH;
    static const QString REFSEQ_BACTERIA_ID;

    static const QString REFSEQ_VIRAL_PATH;
    static const QString REFSEQ_VIRAL_ID;

    static const QString ELEMENTS_GROUP;

private:
    static void registerTaxonData();
    static void registerMinikrakenDatabase();
    static void registerDiamondUniprot50Database();
    static void registerDiamondUniprot90Database();
    static void registerHumanRefSeq();
    static void registerBacteriaRefSeq();
    static void registerViralRefSeq();

    static void registerData(const QString dataId, const QString &relativePath, const QString &description, bool addAsFolder = false);
};

}   // namespace U2

#endif // _U2_NGS_CLASSIFICATION_UTILS_H_
