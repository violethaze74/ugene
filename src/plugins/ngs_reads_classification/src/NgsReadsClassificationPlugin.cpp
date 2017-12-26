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

#include <U2Core/AppContext.h>
#include <U2Core/DataPathRegistry.h>
#include <U2Core/U2SafePoints.h>

#include "NgsReadsClassificationPlugin.h"
#include "ClassificationFilterWorker.h"

namespace U2 {

const QString NgsReadsClassificationPlugin::PLUGIN_NAME = QObject::tr("NGS reads classification");
const QString NgsReadsClassificationPlugin::PLUGIN_DESCRIPRION = QObject::tr("The plugin supports data and utility for the NGS reads classifiers");

const QString NgsReadsClassificationPlugin::TAXONOMY_PATH = "ngs_classification/taxonomy";
const QString NgsReadsClassificationPlugin::TAXONOMY_DATA_ID = "taxonomy_data";
const QString NgsReadsClassificationPlugin::TAXON_PROTEIN_MAP = "prot.accession2taxid.gz";
const QString NgsReadsClassificationPlugin::TAXON_NODES = "nodes.dmp";
const QString NgsReadsClassificationPlugin::TAXON_NAMES = "names.dmp";

const QString NgsReadsClassificationPlugin::MINIKRAKEN_4_GB_PATH = "ngs_classification/kraken";
const QString NgsReadsClassificationPlugin::MINIKRAKEN_4_GB_ID = "minikraken_4gb";

const QString NgsReadsClassificationPlugin::DIAMOND_UNIPROT_50_DATABASE_PATH = "ngs_classification/diamond/uniref50.dmnd";
const QString NgsReadsClassificationPlugin::DIAMOND_UNIPROT_50_DATABASE_ID = "diamond_uniprot_50";

const QString NgsReadsClassificationPlugin::DIAMOND_UNIPROT_90_DATABASE_PATH = "ngs_classification/diamond/uniref90.dmnd";
const QString NgsReadsClassificationPlugin::DIAMOND_UNIPROT_90_DATABASE_ID = "diamond_uniprot_90";

const QString NgsReadsClassificationPlugin::REFSEQ_HUMAN_PATH = "ngs_classification/refseq/human";
const QString NgsReadsClassificationPlugin::REFSEQ_HUMAN_ID = "refseq_human";

const QString NgsReadsClassificationPlugin::REFSEQ_BACTERIA_PATH = "ngs_classification/refseq/bacteria";
const QString NgsReadsClassificationPlugin::REFSEQ_BACTERIA_ID = "refseq_bacteria";

const QString NgsReadsClassificationPlugin::REFSEQ_VIRAL_PATH = "ngs_classification/refseq/viral";
const QString NgsReadsClassificationPlugin::REFSEQ_VIRAL_ID = "refseq_viral";

const QString NgsReadsClassificationPlugin::WORKFLOW_ELEMENTS_GROUP = QObject::tr("NGS: Reads Classification");

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    NgsReadsClassificationPlugin *plugin = new NgsReadsClassificationPlugin();
    return plugin;
}

NgsReadsClassificationPlugin::NgsReadsClassificationPlugin()
    : Plugin(PLUGIN_NAME, PLUGIN_DESCRIPRION)
{
    registerData(TAXONOMY_DATA_ID, TAXONOMY_PATH, tr("Taxonomy data from NCBI"));
    registerData(MINIKRAKEN_4_GB_ID, MINIKRAKEN_4_GB_PATH, tr("Minikraken 4Gb database"), true);
    registerData(DIAMOND_UNIPROT_50_DATABASE_ID, DIAMOND_UNIPROT_50_DATABASE_PATH, tr("DIAMOND database built from UniProt50"));
    registerData(DIAMOND_UNIPROT_90_DATABASE_ID, DIAMOND_UNIPROT_90_DATABASE_PATH, tr("DIAMOND database built from UniProt90"));
    registerData(REFSEQ_HUMAN_ID, REFSEQ_HUMAN_PATH, tr("RefSeq release human data from NCBI"));
    registerData(REFSEQ_BACTERIA_ID, REFSEQ_BACTERIA_PATH, tr("RefSeq release bacteria data from NCBI"));
    registerData(REFSEQ_VIRAL_ID, REFSEQ_VIRAL_PATH, tr("RefSeq release viral data from NCBI"));

    LocalWorkflow::ClassificationFilterWorkerFactory::init();

    // Pre-load taxonomy data
    LocalWorkflow::TaxonomyTree::getInstance();
}

NgsReadsClassificationPlugin::~NgsReadsClassificationPlugin() {
    foreach (const QString &dataId, registeredData) {
        unregisterData(dataId);
    }
}

void NgsReadsClassificationPlugin::registerData(const QString &dataId, const QString &relativePath, const QString &description, bool addAsFolder) {
    U2DataPathRegistry* dataPathRegistry = AppContext::getDataPathRegistry();
    const QString path = QFileInfo(QString(PATH_PREFIX_DATA) + ":" + relativePath).absoluteFilePath();
    U2DataPath *dataPath = new U2DataPath(dataId, path, description, addAsFolder ? U2DataPath::AddOnlyFolders : U2DataPath::None);
    bool ok = dataPathRegistry->registerEntry(dataPath);
    if (!ok) {
        coreLog.error(QString("Failed to register DATA: %1").arg(path));
        delete dataPath;
    } else {
        registeredData << dataId;
    }
}

void NgsReadsClassificationPlugin::unregisterData(const QString &dataId) {
    U2DataPathRegistry* dataPathRegistry = AppContext::getDataPathRegistry();
    CHECK(NULL != dataPathRegistry, );
    dataPathRegistry->unregisterEntry(dataId);
    registeredData.removeAll(dataId);
}

}   // namespace U2
