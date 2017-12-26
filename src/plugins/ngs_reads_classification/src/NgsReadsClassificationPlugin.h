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

#ifndef _U2_NGS_READS_CLASSIFICATION_PLUGIN_H_
#define _U2_NGS_READS_CLASSIFICATION_PLUGIN_H_

#include <U2Core/PluginModel.h>

namespace U2 {

#ifdef BUILDING_NGS_READS_CLASSIFICATION_DLL
#   define U2NGS_READS_CLASSIFICATION_EXPORT Q_DECL_EXPORT
#else
#   define U2NGS_READS_CLASSIFICATION_EXPORT Q_DECL_IMPORT
#endif

class U2NGS_READS_CLASSIFICATION_EXPORT NgsReadsClassificationPlugin : public Plugin {
    Q_OBJECT
public:
    NgsReadsClassificationPlugin();
    ~NgsReadsClassificationPlugin();

    static const QString PLUGIN_NAME;
    static const QString PLUGIN_DESCRIPRION;

    static const QString TAXONOMY_PATH;
    static const QString TAXONOMY_DATA_ID;
    static const QString TAXON_PROTEIN_MAP;
    static const QString TAXON_NODES;
    static const QString TAXON_NAMES;

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

    static const QString WORKFLOW_ELEMENTS_GROUP;

private:
    void registerData(const QString &dataId, const QString &relativePath, const QString &description, bool addAsFolder = false);
    void unregisterData(const QString &dataId);

    QStringList registeredData;
};

}   // namespace U2

#endif // _U2_NGS_READS_CLASSIFICATION_PLUGIN_H_
