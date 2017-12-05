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

#include <QThread>

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/AppSettings.h>
#include <U2Core/L10n.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "DatabaseSizeRelation.h"
#include "DatabaseValidator.h"
#include "KrakenClassifyWorker.h"
#include "KrakenClassifyWorkerFactory.h"
#include "KrakenClassifyPrompter.h"
#include "KrakenSupport.h"

namespace U2 {
namespace LocalWorkflow {

const QString KrakenClassifyWorkerFactory::ACTOR_ID = "classify-reads-with-kraken";

const QString KrakenClassifyWorkerFactory::INPUT_PORT_ID = "in";
const QString KrakenClassifyWorkerFactory::INPUT_PAIRED_PORT_ID = "in2";
const QString KrakenClassifyWorkerFactory::OUTPUT_PORT_ID = "out";

const QString KrakenClassifyWorkerFactory::INPUT_READS_URL_SLOT_ID = "reads-url";
const QString KrakenClassifyWorkerFactory::INPUT_PAIRED_READS_URL_SLOT_ID = "reads2-url";
const QString KrakenClassifyWorkerFactory::OUTPUT_REPORT_URL_SLOT_ID = "classification-url";

const QString KrakenClassifyWorkerFactory::DATABASE_ATTR_ID = "database";
const QString KrakenClassifyWorkerFactory::SEQUENCING_READS_ATTR_ID = "sequencing-reads";
const QString KrakenClassifyWorkerFactory::QUICK_OPERATION_ATTR_ID = "quick-operation";
const QString KrakenClassifyWorkerFactory::MIN_HITS_NUMBER_ATTR_ID = "min-hits";
const QString KrakenClassifyWorkerFactory::THREADS_NUMBER_ATTR_ID = "threads";
const QString KrakenClassifyWorkerFactory::PRELOAD_DATABASE_ATTR_ID = "preload-database";

const QString KrakenClassifyWorkerFactory::SINGLE_END_TEXT = QObject::tr("Single-end");
const QString KrakenClassifyWorkerFactory::PAIRED_END_TEXT = QObject::tr("Paired-end");

KrakenClassifyWorkerFactory::KrakenClassifyWorkerFactory()
    : DomainFactory(ACTOR_ID)
{

}

Worker *KrakenClassifyWorkerFactory::createWorker(Actor *actor) {
    return new KrakenClassifyWorker(actor);
}

void KrakenClassifyWorkerFactory::init() {
    QList<PortDescriptor *> ports;
    {
        QMap<Descriptor, DataTypePtr> inType;
        inType[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();

        QMap<Descriptor, DataTypePtr> inPairedType;
        inPairedType[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();

        QMap<Descriptor, DataTypePtr> outType;
        outType[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();

        const Descriptor inPortDesc(INPUT_PORT_ID,
                                    KrakenClassifyPrompter::tr("Input sequences 1"),
                                    KrakenClassifyPrompter::tr("URL(s) to FASTQ or FASTA file(s) should be provided.\n\n"
                                                               "The input files may contain single-end reads, scaffolds, or \"left\" reads in case of the paired-end sequencing (see \"Sequencing reads\" parameter of the element)."));
        const Descriptor inPairedPortDesc(INPUT_PAIRED_PORT_ID,
                                          KrakenClassifyPrompter::tr("Input sequences 2"),
                                          KrakenClassifyPrompter::tr("URL(s) to FASTQ or FASTA file(s) should be provided.\n\n"
                                                                     "The port is used, if paired-end sequencing was done. The input files should contain the \"right\" reads (see \"Sequencing reads\" parameter of the element)."));
        const Descriptor outPortDesc(OUTPUT_PORT_ID, KrakenClassifyPrompter::tr("Classified sequences"), KrakenClassifyPrompter::tr("Classification report URL."));

        ports << new PortDescriptor(inPortDesc, DataTypePtr(new MapDataType(ACTOR_ID + "-in", inType)), true /*input*/);
        ports << new PortDescriptor(inPairedPortDesc, DataTypePtr(new MapDataType(ACTOR_ID + "paired-in", inPairedType)), true /*input*/);
        ports << new PortDescriptor(outPortDesc, DataTypePtr(new MapDataType(ACTOR_ID + "-out", outType)), false /*input*/, true /*multi*/);
    }

    QList<Attribute *> attributes;
    {
        const Descriptor databaseDesc(DATABASE_ATTR_ID, KrakenClassifyPrompter::tr("Database"),
                                      KrakenClassifyPrompter::tr("A path to the folder with the Kraken database files."));

        const Descriptor sequencingReadsDesc(SEQUENCING_READS_ATTR_ID, KrakenClassifyPrompter::tr("Sequencing reads"),
                                             KrakenClassifyPrompter::tr("Choose between single-end (SE) and paired-end (PE) sequencing.\n\n"
                                                                        "After that, use the input port(s) of the element to pass URL(s) of the corresponding FASTQ files with the sequencing reads."));

        const Descriptor quickOperationDesc(QUICK_OPERATION_ATTR_ID, KrakenClassifyPrompter::tr("Quick operation"),
                                            KrakenClassifyPrompter::tr("Stop classification of an input read after the certain number of hits.\n\n"
                                                                       "The value can be specified in the “Minimum number of hits” parameter."));

        const Descriptor minHitsDesc(MIN_HITS_NUMBER_ATTR_ID, KrakenClassifyPrompter::tr("Minimum number of hits"),
                                     KrakenClassifyPrompter::tr("The number of hits that are required to declare an input sequence classified.\n\n"
                                                                "This can be especially useful with custom databases when testing to see if sequences either do or do not belong to a particular genome."));

        const Descriptor threadsDesc(THREADS_NUMBER_ATTR_ID, KrakenClassifyPrompter::tr("Number of threads"),
                                     KrakenClassifyPrompter::tr("Use multiple threads (--threads)."));

        const Descriptor preloadDatabaseDesc(PRELOAD_DATABASE_ATTR_ID, KrakenClassifyPrompter::tr("Preload database"),
                                             KrakenClassifyPrompter::tr("Load the Kraken database into RAM (--preload).\n\n"
                                                                        "This can be useful to improve the speed. The database size should be less than the RAM size.\n\n"
                                                                        "The other option to improve the speed is to store the database on ramdisk. Set this parameter to “False” in this case."));

        Attribute *databaseAttribute =  new Attribute(databaseDesc, BaseTypes::STRING_TYPE(), true);
        attributes << databaseAttribute;

        Attribute *sequencingReadsAttribute = new Attribute(sequencingReadsDesc, BaseTypes::STRING_TYPE(), false, KrakenClassifyTaskSettings::SINGLE_END);
        attributes << sequencingReadsAttribute;

        attributes << new Attribute(quickOperationDesc, BaseTypes::BOOL_TYPE(), false, false);

        Attribute *minHitsAttribute = new Attribute(minHitsDesc, BaseTypes::NUM_TYPE(), false, 1);
        attributes << minHitsAttribute;

        attributes << new Attribute(threadsDesc, BaseTypes::NUM_TYPE(), false, AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
        attributes << new Attribute(preloadDatabaseDesc, BaseTypes::BOOL_TYPE(), false, true);

        sequencingReadsAttribute->addPortRelation(PortRelationDescriptor(INPUT_PAIRED_PORT_ID, QVariantList() << KrakenClassifyTaskSettings::PAIRED_END));
        minHitsAttribute->addRelation(new VisibilityRelation(QUICK_OPERATION_ATTR_ID, "true"));
        databaseAttribute->addRelation(new DatabaseSizeRelation(PRELOAD_DATABASE_ATTR_ID));
    }

    QMap<QString, PropertyDelegate *> delegates;
    {
        delegates[DATABASE_ATTR_ID] = new URLDelegate("", "kraken/database", false, true, false);

        QVariantMap sequencingReadsMap;
        sequencingReadsMap[SINGLE_END_TEXT] = KrakenClassifyTaskSettings::SINGLE_END;
        sequencingReadsMap[PAIRED_END_TEXT] = KrakenClassifyTaskSettings::PAIRED_END;
        delegates[SEQUENCING_READS_ATTR_ID] = new ComboBoxDelegate(sequencingReadsMap);

        delegates[QUICK_OPERATION_ATTR_ID] = new ComboBoxWithBoolsDelegate();

        QVariantMap threadsMap;
        threadsMap["minimum"] = 1;
        threadsMap["maximum"] = QThread::idealThreadCount();
        delegates[THREADS_NUMBER_ATTR_ID] = new SpinBoxDelegate(threadsMap);

        delegates[PRELOAD_DATABASE_ATTR_ID] = new ComboBoxWithBoolsDelegate();
    }

    const Descriptor desc(ACTOR_ID, KrakenClassifyPrompter::tr("Classify Sequences with Kraken"),
                          KrakenClassifyPrompter::tr("Kraken is a taxonomic sequence classifier that assigns taxonomic labels to short DNA reads. "
                                                     "It does this by examining the k-mers within a read and querying a database with those."));
    ActorPrototype *proto = new IntegralBusActorPrototype(desc, ports, attributes);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new KrakenClassifyPrompter(NULL));
    proto->addExternalTool(ET_KRAKEN_CLASSIFY);
    proto->addExternalTool(ET_KRAKEN_TRANSLATE);
    proto->setValidator(new DatabaseValidator());
    WorkflowEnv::getProtoRegistry()->registerProto(KrakenClassifyPrompter::tr("NGS: Reads Classification"), proto);     // TODO: replace the category name with a constant after extracting to a separate plugin

    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new KrakenClassifyWorkerFactory());
}

}   // namespace LocalWorkflow
}   // namespace U2
