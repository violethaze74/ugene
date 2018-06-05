/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2018 UniPro <ugene@unipro.ru>
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

#include "WevotePrompter.h"
#include "WevoteValidator.h"
#include "../ngs_reads_classification/src/NgsReadsClassificationPlugin.h"

namespace U2 {
namespace Workflow {

bool WevoteValidator::validate(const Actor *actor, ProblemList &problemList, const QMap<QString, QString> &) const {
    return validateTaxonomyData(actor, problemList);
}

bool WevoteValidator::validateTaxonomyData(const Actor *actor, ProblemList &problemList) const {
    bool isValid = true;

    U2DataPathRegistry *dataPathRegistry = AppContext::getDataPathRegistry();
    SAFE_POINT_EXT(NULL != dataPathRegistry, problemList.append(Problem("U2DataPathRegistry is NULL", actor->getId())), false);

    U2DataPath *taxonomyDataPath = dataPathRegistry->getDataPathByName(NgsReadsClassificationPlugin::TAXONOMY_DATA_ID);
    SAFE_POINT_EXT(NULL != taxonomyDataPath, problemList.append(Problem("Taxonomy data path is not registered", actor->getId())), false);
    CHECK_EXT(taxonomyDataPath->isValid(), problemList.append(Problem(LocalWorkflow::WevotePrompter::tr("Taxonomy data are missed"), actor->getId())), false);

    if (taxonomyDataPath->getPathByName(NgsReadsClassificationPlugin::TAXON_NODES_ITEM_ID).isEmpty()) {
        problemList << Problem(LocalWorkflow::WevotePrompter::tr("Taxonomy file '%1' is not found.").arg(NgsReadsClassificationPlugin::TAXON_NODES_ITEM_ID), actor->getId());
        isValid = false;
    }

    if (taxonomyDataPath->getPathByName(NgsReadsClassificationPlugin::TAXON_NAMES_ITEM_ID).isEmpty()) {
        problemList << Problem(LocalWorkflow::WevotePrompter::tr("Taxonomy file '%1' is not found.").arg(NgsReadsClassificationPlugin::TAXON_NAMES_ITEM_ID), actor->getId());
        isValid = false;
    }

    return isValid;
}

}   // namespace Workflow
}   // namespace U2
