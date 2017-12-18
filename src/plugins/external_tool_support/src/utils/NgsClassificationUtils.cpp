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

#include <QFileInfo>

#include <U2Core/AppContext.h>
#include <U2Core/DataPathRegistry.h>

#include "NgsClassificationUtils.h"

namespace U2 {

const QString NgsClassificationUtils::TAXONOMY_PATH = "ngs_classification/taxonomy";
const QString NgsClassificationUtils::TAXONOMY_DATA_ID = "taxonomy_data";
const QString NgsClassificationUtils::TAXON_PROTEIN_MAP = "prot.accession2taxid";   // TODO: check, if DIAMOND can work with not zipped map
const QString NgsClassificationUtils::TAXON_NODES = "nodes.dmp";

const QString NgsClassificationUtils::MINIKRAKEN_4_GB_PATH = "ngs_classification/kraken";
const QString NgsClassificationUtils::MINIKRAKEN_4_GB_ID = "minikraken_4gb";

const QString NgsClassificationUtils::DIAMOND_UNIPROT_50_DATABASE_PATH = "ngs_classification/diamond/uniref50.dmnd";
const QString NgsClassificationUtils::DIAMOND_UNIPROT_50_DATABASE_ID = "diamond_uniprot_50";

const QString NgsClassificationUtils::DIAMOND_UNIPROT_90_DATABASE_PATH = "ngs_classification/diamond/uniref90.dmnd";
const QString NgsClassificationUtils::DIAMOND_UNIPROT_90_DATABASE_ID = "diamond_uniprot_90";

const QString NgsClassificationUtils::REFSEQ_HUMAN_PATH = "ngs_classification/refseq/human";
const QString NgsClassificationUtils::REFSEQ_HUMAN_ID = "refseq_human";

const QString NgsClassificationUtils::REFSEQ_BACTERIA_PATH = "ngs_classification/refseq/bacteria";
const QString NgsClassificationUtils::REFSEQ_BACTERIA_ID = "refseq_bacteria";

const QString NgsClassificationUtils::REFSEQ_VIRAL_PATH = "ngs_classification/refseq/viral";
const QString NgsClassificationUtils::REFSEQ_VIRAL_ID = "refseq_viral";

const QString NgsClassificationUtils::ELEMENTS_GROUP = QObject::tr("NGS: Reads Classification");

void NgsClassificationUtils::initData() {
    registerTaxonData();
    registerMinikrakenDatabase();
    registerDiamondUniprot50Database();
    registerDiamondUniprot90Database();
    registerHumanRefSeq();
    registerBacteriaRefSeq();
    registerViralRefSeq();
}

void NgsClassificationUtils::registerTaxonData() {
    registerData(TAXONOMY_DATA_ID, TAXONOMY_PATH, tr("Taxonomy data from NCBI"));
}

void NgsClassificationUtils::registerMinikrakenDatabase() {
    registerData(MINIKRAKEN_4_GB_ID, MINIKRAKEN_4_GB_PATH, tr("Minikraken 4Gb database"), true);
}

void NgsClassificationUtils::registerDiamondUniprot50Database() {
    registerData(DIAMOND_UNIPROT_50_DATABASE_ID, DIAMOND_UNIPROT_50_DATABASE_PATH, tr("DIAMOND database built from UniProt50"));
}

void NgsClassificationUtils::registerDiamondUniprot90Database() {
    registerData(DIAMOND_UNIPROT_90_DATABASE_ID, DIAMOND_UNIPROT_90_DATABASE_PATH, tr("DIAMOND database built from UniProt90"));
}

void NgsClassificationUtils::registerHumanRefSeq() {
    registerData(REFSEQ_HUMAN_ID, REFSEQ_HUMAN_PATH, tr("RefSeq release human data from NCBI"));
}

void NgsClassificationUtils::registerBacteriaRefSeq() {
    registerData(REFSEQ_BACTERIA_ID, REFSEQ_BACTERIA_PATH, tr("RefSeq release bacteria data from NCBI"));
}

void NgsClassificationUtils::registerViralRefSeq() {
    registerData(REFSEQ_VIRAL_ID, REFSEQ_VIRAL_PATH, tr("RefSeq release viral data from NCBI"));
}

void NgsClassificationUtils::registerData(const QString dataId, const QString &relativePath, const QString &description, bool addAsFolder) {
    U2DataPathRegistry* dpr = AppContext::getDataPathRegistry();
    const QString path = QFileInfo(QString(PATH_PREFIX_DATA) + ":" + relativePath).absoluteFilePath();
    U2DataPath *dataPath = new U2DataPath(dataId, path, description, addAsFolder ? U2DataPath::AddOnlyFolders : U2DataPath::None);
    bool ok = dpr->registerEntry(dataPath);
    if (!ok) {
        delete dataPath;
    }
}

}   // namespace U2
