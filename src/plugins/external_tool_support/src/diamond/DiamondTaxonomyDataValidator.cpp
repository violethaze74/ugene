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

#include "DiamondClassifyPrompter.h"
#include "DiamondSupport.h"
#include "DiamondTaxonomyDataValidator.h"

namespace U2 {
namespace Workflow {

bool DiamondTaxonomyDataValidator::validate(const Actor *actor, ProblemList &problemList, const QMap<QString, QString> &) const {
    U2DataPathRegistry *dataPathRegistry = AppContext::getDataPathRegistry();
    SAFE_POINT_EXT(NULL != dataPathRegistry, problemList << Problem("U2DataPathRegistry is NULL", actor->getId()), false);

    U2DataPath *taxonomyDataPath = dataPathRegistry->getDataPathByName(DiamondSupport::TAXONOMY_DATA);
    CHECK_EXT(NULL != taxonomyDataPath && taxonomyDataPath->isValid(),
              problemList << Problem(LocalWorkflow::DiamondClassifyPrompter::tr("Taxonomy data is not set"), actor->getId()), false);

    CHECK_EXT(!taxonomyDataPath->getPathByName(DiamondSupport::TAXON_PROTEIN_MAP).isEmpty(),
              problemList << Problem(LocalWorkflow::DiamondClassifyPrompter::tr("file '%1' not found").arg("prot.accession2taxid.gz"), actor->getId()), false);

    CHECK_EXT(!taxonomyDataPath->getPathByName(DiamondSupport::TAXON_NODES).isEmpty(),
              problemList << Problem(LocalWorkflow::DiamondClassifyPrompter::tr("file '%1' not found").arg("nodes.dmp"), actor->getId()), false);

    return true;
}

}   // namespace Workflow
}   // namespace U2
