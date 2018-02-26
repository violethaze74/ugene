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
#include <U2Lang/WorkflowEnv.h>

#include "DiamondClassifyPrompter.h"
#include "DiamondClassifyWorker.h"
#include "DiamondClassifyWorkerFactory.h"
#include "DiamondSupport.h"
#include "DiamondTaxonomyDataValidator.h"
#include "../../ngs_reads_classification/src/NgsReadsClassificationPlugin.h"

namespace U2 {
namespace LocalWorkflow {

const QString DiamondClassifyWorkerFactory::ACTOR_ID = "classify-reads-with-diamond";

const QString DiamondClassifyWorkerFactory::INPUT_PORT_ID = "in";
const QString DiamondClassifyWorkerFactory::INPUT_PAIRED_PORT_ID = "in2";
const QString DiamondClassifyWorkerFactory::OUTPUT_PORT_ID = "out";

const QString DiamondClassifyWorkerFactory::INPUT_DATA_ATTR_ID = "input-data";
const QString DiamondClassifyWorkerFactory::DATABASE_ATTR_ID = "database";

const QString DiamondClassifyWorkerFactory::SINGLE_END_TEXT = QObject::tr("SE reads or scaffolds");
const QString DiamondClassifyWorkerFactory::PAIRED_END_TEXT = QObject::tr("PE reads");

DiamondClassifyWorkerFactory::DiamondClassifyWorkerFactory()
    : DomainFactory(ACTOR_ID)
{

}

Worker *DiamondClassifyWorkerFactory::createWorker(Actor *actor) {
    return new DiamondClassifyWorker(actor);
}

void DiamondClassifyWorkerFactory::init() {
    QList<PortDescriptor *> ports;
    {
        const Descriptor inSlotDesc(BaseSlots::URL_SLOT().getId(),
                                    DiamondClassifyPrompter::tr("Input URL"),
                                    DiamondClassifyPrompter::tr("Input URL."));

//        const Descriptor inPairedSlotDesc(BaseSlots::URL_SLOT().getId(),
//                                          DiamondClassifyPrompter::tr("Input URL(s)"),
//                                          DiamondClassifyPrompter::tr("Input URL(s)."));

        QMap<Descriptor, DataTypePtr> inType;
        inType[inSlotDesc] = BaseTypes::STRING_TYPE();

//        QMap<Descriptor, DataTypePtr> inPairedType;
//        inPairedType[inPairedSlotDesc] = BaseTypes::STRING_TYPE();

        QMap<Descriptor, DataTypePtr> outType;
        outType[TaxonomySupport::TAXONOMY_CLASSIFICATION_SLOT()] = TaxonomySupport::TAXONOMY_CLASSIFICATION_TYPE();

        const Descriptor inPortDesc(INPUT_PORT_ID,
                                    DiamondClassifyPrompter::tr("Input sequences"),
                                    DiamondClassifyPrompter::tr("URL(s) to FASTQ or FASTA file(s) should be provided.\n\n"
                                                                "The input files may contain single-end reads, scaffolds, or \"left\" reads in case of the paired-end sequencing (see \"Input data\" parameter of the element)."));

        const Descriptor inPairedPortDesc(INPUT_PAIRED_PORT_ID,
                                          DiamondClassifyPrompter::tr("Input sequences 2"),
                                          DiamondClassifyPrompter::tr("URL(s) to FASTQ or FASTA file(s) should be provided.\n\n"
                                                                      "The port is used, if paired-end sequencing was done. The input files should contain the \"right\" reads (see \"Input data\" parameter of the element)."));

        const Descriptor outPortDesc(OUTPUT_PORT_ID,
                                     DiamondClassifyPrompter::tr("DIAMOND Classification"),
                                     DiamondClassifyPrompter::tr("A list of sequence names with the associated taxonomy IDs, classified by DIAMOND."));

        ports << new PortDescriptor(inPortDesc, DataTypePtr(new MapDataType(ACTOR_ID + "-in", inType)), true /*input*/);
//        ports << new PortDescriptor(inPairedPortDesc, DataTypePtr(new MapDataType(ACTOR_ID + "-paired-in", inPairedType)), true /*input*/);       // FIXME: diamond can't work with paired reads
        ports << new PortDescriptor(outPortDesc, DataTypePtr(new MapDataType(ACTOR_ID + "-out", outType)), false /*input*/, true /*multi*/);
    }

    QList<Attribute *> attributes;
    {
        const Descriptor inputDataDesc(INPUT_DATA_ATTR_ID, DiamondClassifyPrompter::tr("Input data"),
                                             DiamondClassifyPrompter::tr("The input data that should be classified are provided through the input ports of the element.\n\n"
                                                                         "To classify single-end (SE) reads or scaffolds, received by reads de novo assembly, set this parameter to \"SE reads or scaffolds\". The element has one input port in this case. Pass URL(s) to the corresponding files to this port.\n\n"
                                                                         "To classify paired-end (PE) reads, set the value to \"PE reads\". The element has two input ports in this case. Pass URL(s) to the \"left\" and \"right\" reads to the first and the second port correspondingly.\n\n"
                                                                         "The input files should be in FASTA or FASTQ formats."));

        const Descriptor databaseDesc(DATABASE_ATTR_ID, DiamondClassifyPrompter::tr("Database"),
                                      DiamondClassifyPrompter::tr("Input a binary DIAMOND database file."));

//        Attribute *inputDataAttribute = new Attribute(inputDataDesc, BaseTypes::STRING_TYPE(), false, DiamondClassifyTaskSettings::SINGLE_END);       // FIXME: diamond can't work with paired reads
        Attribute *databaseAttribute = new Attribute(databaseDesc, BaseTypes::STRING_TYPE(), true);

//        attributes << inputDataAttribute;       // FIXME: diamond can't work with paired reads
        attributes << databaseAttribute;

//        inputDataAttribute->addPortRelation(PortRelationDescriptor(INPUT_PAIRED_PORT_ID, QVariantList() << DiamondClassifyTaskSettings::PAIRED_END));       // FIXME: diamond can't work with paired reads
    }

    QMap<QString, PropertyDelegate *> delegates;
    {
//        QVariantMap inputDataMap;
//        inputDataMap[SINGLE_END_TEXT] = DiamondClassifyTaskSettings::SINGLE_END;
//        inputDataMap[PAIRED_END_TEXT] = DiamondClassifyTaskSettings::PAIRED_END;
//        delegates[INPUT_DATA_ATTR_ID] = new ComboBoxDelegate(inputDataMap);       // FIXME: diamond can't work with paired reads

        delegates[DATABASE_ATTR_ID] = new URLDelegate("", "diamond/database", false, false, false);
    }

    const Descriptor desc(ACTOR_ID,
                          DiamondClassifyPrompter::tr("Classify Sequences with DIAMOND"),
                          DiamondClassifyPrompter::tr("In general, DIAMOND is a sequence aligner for protein and translated DNA searches similar to "
                                                      "the NCBI BLAST software tools. However, it provides a speedup of BLAST ranging up to x20,000.<br>"
                                                      "Using this workflow element one can use DIAMOND for taxonomic classification of short DNA reads "
                                                      "and longer sequences such as scaffolds."));

    ActorPrototype *proto = new IntegralBusActorPrototype(desc, ports, attributes);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new DiamondClassifyPrompter(NULL));
    proto->addExternalTool(DiamondSupport::TOOL_NAME);
    proto->setValidator(new DiamondTaxonomyDataValidator());
    WorkflowEnv::getProtoRegistry()->registerProto(NgsReadsClassificationPlugin::WORKFLOW_ELEMENTS_GROUP, proto);

    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new DiamondClassifyWorkerFactory());
}

void DiamondClassifyWorkerFactory::cleanup() {
    WorkflowEnv::getProtoRegistry()->unregisterProto(ACTOR_ID);

    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->unregisterEntry(ACTOR_ID);
}

}   // namespace LocalWorkflow
}   // namespace U2
