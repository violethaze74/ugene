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

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/URLAttribute.h>
#include <U2Lang/WorkflowEnv.h>

#include "DiamondBuildPrompter.h"
#include "DiamondBuildWorker.h"
#include "DiamondBuildWorkerFactory.h"
#include "DiamondSupport.h"
#include "kraken/GenomicLibraryDelegate.h"

namespace U2 {
namespace LocalWorkflow {

const QString DiamondBuildWorkerFactory::ACTOR_ID = "build-diamond-database";

const QString DiamondBuildWorkerFactory::OUTPUT_PORT_ID = "out";

const QString DiamondBuildWorkerFactory::DATABASE_ATTR_ID = "database";
const QString DiamondBuildWorkerFactory::GENOMIC_LIBRARY_ATTR_ID = "genomic-library";

DiamondBuildWorkerFactory::DiamondBuildWorkerFactory()
    : DomainFactory(ACTOR_ID)
{

}

Worker *DiamondBuildWorkerFactory::createWorker(Actor *actor) {
    return new DiamondBuildWorker(actor);
}

void DiamondBuildWorkerFactory::init() {
    QList<PortDescriptor *> ports;
    {
        const Descriptor outSlotDesc(BaseSlots::URL_SLOT().getId(), DiamondBuildPrompter::tr("Output URL"), DiamondBuildPrompter::tr("Output URL."));

        QMap<Descriptor, DataTypePtr> outType;
        outType[outSlotDesc] = BaseTypes::STRING_TYPE();

        const Descriptor outPortDesc(OUTPUT_PORT_ID, DiamondBuildPrompter::tr("Output DIAMOND database"), DiamondBuildPrompter::tr("URL to the DIAMOND database file."));
        ports << new PortDescriptor(outPortDesc, DataTypePtr(new MapDataType(ACTOR_ID + "-out", outType)), false /*input*/, true /*multi*/);
    }

    QList<Attribute *> attributes;
    {
        const Descriptor databaseDesc(DATABASE_ATTR_ID, DiamondBuildPrompter::tr("Database"),
                                               DiamondBuildPrompter::tr("A name of the binary DIAMOND database file that should be created."));

        const Descriptor genomicLibraryDesc(GENOMIC_LIBRARY_ATTR_ID, DiamondBuildPrompter::tr("Genomic library"),
                                            DiamondBuildPrompter::tr("Genomes that should be used to build the database."));

        Attribute *databaseAttribute = new Attribute(databaseDesc, BaseTypes::STRING_TYPE(), true, "diamond_db.dmnd");
        Attribute *genomicLibraryAttribute = new URLAttribute(genomicLibraryDesc, BaseTypes::URL_DATASETS_TYPE(), true);

        attributes << databaseAttribute;
        attributes << genomicLibraryAttribute;
    }

    QMap<QString, PropertyDelegate *> delegates;
    {
        delegates[DATABASE_ATTR_ID] = new URLDelegate("", "diamond/database", false, true, true);
        delegates[GENOMIC_LIBRARY_ATTR_ID] = new GenomicLibraryDelegate();
    }

    const Descriptor desc(ACTOR_ID, DiamondBuildPrompter::tr("Build DIAMOND Database"),
                          DiamondBuildPrompter::tr("Build a DIAMOND formatted database from a FASTA input file."));

    ActorPrototype *proto = new IntegralBusActorPrototype(desc, ports, attributes);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new DiamondBuildPrompter(NULL));
    proto->addExternalTool(DiamondSupport::TOOL_NAME);
    WorkflowEnv::getProtoRegistry()->registerProto(DiamondBuildPrompter::tr("NGS: Reads Classification"), proto);     // TODO: replace the category name with a constant after extracting to a separate plugin

    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new DiamondBuildWorkerFactory());
}

}   // namespace LocalWorkflow
}   // namespace U2
