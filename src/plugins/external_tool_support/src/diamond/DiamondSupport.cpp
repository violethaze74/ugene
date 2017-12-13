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

#include "DiamondSupport.h"

namespace U2 {

const QString DiamondSupport::TOOL_NAME = "diamond";

const QString DiamondSupport::TAXONOMY_DATA = "taxonomy_data";
const QString DiamondSupport::TAXON_PROTEIN_MAP = "prot.accession2taxid.gz";
const QString DiamondSupport::TAXON_NODES = "nodes.dmp";

DiamondSupport::DiamondSupport(const QString &name)
    : ExternalTool(name)
{
    if (NULL != AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }

    validationArguments << "--version";
    validMessage = "diamond version ";
    versionRegExp = QRegExp("diamond version (\\d+\\.\\d+\\.\\d+)");
    executableFileName = "diamond";
    description = tr("\"<i>DIAMOND</i>\" is accelerated BLAST compatible local sequence aligner.");

    registerTaxonData();
}

void DiamondSupport::registerTaxonData() {
    U2DataPathRegistry* dpr = AppContext::getDataPathRegistry();
    const QString taxonomyPath = QFileInfo(QString(PATH_PREFIX_DATA) + ":ngs_classification/taxonomy").absoluteFilePath();
    U2DataPath *dataPath = new U2DataPath(TAXONOMY_DATA, taxonomyPath, tr("Taxonomy data from NCBI"));
    bool ok = dpr->registerEntry(dataPath);
    if (!ok) {
        delete dataPath;
    }
}

}   // namesapce U2
