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

#include <QScopedPointer>

#include <U2Algorithm/GenomeAssemblyMultiTask.h>
#include <U2Algorithm/GenomeAssemblyRegistry.h>

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/FailTask.h>
#include <U2Core/FileAndDirectoryUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/QVariantUtils.h>
#include <U2Core/GUrlUtils.h>

#include <U2Formats/GenbankLocationParser.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/PairedReadsPortValidator.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>

#include "SpadesPortRelationDescriptor.h"
#include "SpadesSupport.h"
#include "SpadesWorker.h"
#include "SpadesTask.h"
#include "SpadesDelegate.h"

namespace U2 {
namespace LocalWorkflow {

const QString SpadesWorkerFactory::ACTOR_ID = "spades-id";

const QStringList SpadesWorkerFactory::READS_URL_SLOT_ID_LIST = QStringList() <<
                                                                 "readsurl" <<
                                                                 "readsurl-2" <<
                                                                 "readsurl-3" <<
                                                                 "readsurl-4" <<
                                                                 "readsurl-5" <<
                                                                 "readsurl-6" <<
                                                                 "readsurl-7" <<
                                                                 "readsurl-8" <<
                                                                 "readsurl-9" <<
                                                                 "readsurl-10";
const QStringList SpadesWorkerFactory::READS_PAIRED_URL_SLOT_ID_LIST = QStringList() <<
                                                                  "readspairedurl" <<
                                                                  "readspairedurl-2" <<
                                                                  "readspairedurl-3";

const QStringList SpadesWorkerFactory::IN_TYPE_ID_LIST = QStringList() <<
                                                       "spades-paired-data" <<
                                                       "spades-paired-data-2" <<
                                                       "spades-paired-data-3"<<
                                                       "spades-data" <<
                                                       "spades-data-2" <<
                                                       "spades-data-3" <<
                                                       "spades-data-4" <<
                                                       "spades-data-5" <<
                                                       "spades-data-6" <<
                                                       "spades-data-7";

const QString SpadesWorkerFactory::OUT_TYPE_ID = "spades-data-out";

const QString SpadesWorkerFactory::SCAFFOLD_OUT_SLOT_ID = "scaffolds-out";
const QString SpadesWorkerFactory::CONTIGS_URL_OUT_SLOT_ID = "contigs-out";

const QString SpadesWorkerFactory::SEQUENCING_PLATFORM_ID = "platform-id";

const QStringList SpadesWorkerFactory::IN_PORT_ID_LIST = QStringList() <<
                                                       SINGLE_UNPAIRED <<
                                                            SINGLE_CSS <<
                                                            SINGLE_CLR <<
                                                       SINGLE_NANOPORE <<
                                                         SINGLE_SANGER <<
                                                        SINGLE_TRUSTED <<
                                                        SINGLE_UNTRUSTED;

const QStringList SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST = QStringList() <<
                                                                 PAIR_DEFAULT <<
                                                                    PAIR_MATE <<
                                                                   PAIR_HQ_MATE;

const QString SpadesWorkerFactory::MAP_TYPE_ID = "map";

const QString SpadesWorkerFactory::OUT_PORT_DESCR = "out-data";

const QString SpadesWorkerFactory::OUTPUT_DIR = "output-dir";

const QString SpadesWorkerFactory::BASE_SPADES_SUBDIR = "spades";

const QString SpadesWorkerFactory::getPortNameById(const QString& portId) {
    QString res;
    if (portId == IN_PORT_ID_LIST[0]) {
        res = tr("unpaired");
    } else if (portId == IN_PORT_ID_LIST[1]) {
        res = tr("PacBio CCS");
    } else if (portId == IN_PORT_ID_LIST[2]) {
        res = tr("PacBio CLR");
    } else if (portId == IN_PORT_ID_LIST[3]) {
        res = tr("Oxford Nanopore");
    } else if (portId == IN_PORT_ID_LIST[4]) {
        res = tr("Sanger");
    } else if (portId == IN_PORT_ID_LIST[5]) {
        res = tr("trusted");
    } else if (portId == IN_PORT_ID_LIST[6]) {
        res = tr("untrusted");
    } else if (portId == IN_PORT_PAIRED_ID_LIST[0]) {
        res = tr("paired-end");
    } else if (portId == IN_PORT_PAIRED_ID_LIST[1]) {
        res = tr("mate-pairs");
    } else if (portId == IN_PORT_PAIRED_ID_LIST[2]) {
        res = tr("high-quality mate-pairs");
    } else {
        FAIL("Incorrect port id", QString());
    }

    return res;
}

const QString SpadesWorkerFactory::getYamlLibraryNameByPortId(const QString& portId) {
    QString res;
    if (portId == IN_PORT_ID_LIST[0]) {
        res = "single";
    } else if (portId == IN_PORT_ID_LIST[1]) {
        res = "single";
    } else if (portId == IN_PORT_ID_LIST[2]) {
        res = "pacbio";
    } else if (portId == IN_PORT_ID_LIST[3]) {
        res = "nanopore";
    } else if (portId == IN_PORT_ID_LIST[4]) {
        res = "sanger";
    } else if (portId == IN_PORT_ID_LIST[5]) {
        res = "trusted-contigs";
    } else if (portId == IN_PORT_ID_LIST[6]) {
        res = "untrusted-contigs";
    } else if (portId == IN_PORT_PAIRED_ID_LIST[0]) {
        res = "paired-end";
    } else if (portId == IN_PORT_PAIRED_ID_LIST[1]) {
        res = "mate-pairs";
    } else if (portId == IN_PORT_PAIRED_ID_LIST[2]) {
        res = "hq-mate-pairs";
    } else {
        FAIL("Incorrect port id", QString());
    }

    return res;
}


/************************************************************************/
/* Worker */
/************************************************************************/
SpadesWorker::SpadesWorker(Actor *p)
: BaseWorker(p, false)
, output(NULL)
{

}

void SpadesWorker::init() {
    const QStringList portIds = QStringList() <<
        SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST <<
        SpadesWorkerFactory::IN_PORT_ID_LIST;
    foreach(const QString& portId, portIds) {
        IntegralBus* channel = ports.value(portId);
        inChannels << channel;
        readsFetchers << DatasetFetcher(this, channel, context);
    }
    output = ports.value(SpadesWorkerFactory::OUT_PORT_DESCR);
}

Task *SpadesWorker::tick() {
    U2OpStatus2Log os;
    trySetDone(os);
    CHECK(!os.hasError(), new FailTask(os.getError()));

    CHECK(processInputMessagesAndCheckReady(), NULL);

    GenomeAssemblyTaskSettings settings = getSettings(os);
    CHECK(!os.hasError(), new FailTask(os.getError()));

    for (int i = 0; i < readsFetchers.size(); i++) {
        const bool isPortEnabled = readsFetchers[i].hasFullDataset();
        CHECK_CONTINUE(isPortEnabled);

        AssemblyReads read;
        const QString portId = ports.key(inChannels[i]);
        read.libName = SpadesWorkerFactory::getYamlLibraryNameByPortId(portId);

        bool isPaired = false;
        const int index = getReadsUrlSlotIdIndex(portId, isPaired);

        QList<Message> fullDataset = readsFetchers[i].takeFullDataset();
        foreach(const Message& m, fullDataset) {
            QVariantMap data = m.getData().toMap();

            const QString urlSlotId = SpadesWorkerFactory::READS_URL_SLOT_ID_LIST[index];
            const QString readsUrl = data[urlSlotId].toString();
            read.left << readsUrl;

            if (isPaired) {
                const QString urlPairedSlotId = SpadesWorkerFactory::READS_PAIRED_URL_SLOT_ID_LIST[index];
                const QString readsPairedUrl = data[urlPairedSlotId].toString();
                read.right << readsPairedUrl;
            }
        }

        if (isPaired) {
            QVariant inputData = settings.getCustomValue(SpadesTask::OPTION_INPUT_DATA, QVariant());
            SAFE_POINT(inputData != QVariant(), tr("Incorrect input data"), new FailTask(tr("Incorrect input data")));

            QVariantMap mapData = inputData.toMap();
            QStringList values = mapData[portId].toString().split(":");
            SAFE_POINT(values.size() == 2, tr("Incorrect port values"), new FailTask(tr("Incorrect port values")));

            read.orientation = values.first();
            read.readType = values.last();
        }

        settings.reads << read;
    }
    CHECK(!settings.reads.isEmpty(), NULL);

    settings.listeners = createLogListeners();
    GenomeAssemblyMultiTask* t = new GenomeAssemblyMultiTask(settings);
    connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
    return t;
}

void SpadesWorker::cleanup() {
}

bool SpadesWorker::isReady() const {
    if (isDone()) {
        return false;
    }

    bool res = true;
    QList<Port*> inPorts = actor->getInputPorts();
    foreach (Port* port, inPorts) {
        CHECK_CONTINUE(port->isEnabled());

        IntegralBus *inChannel = ports.value(port->getId());
        int hasMsg = inChannel->hasMessage();
        bool ended = inChannel->isEnded();
        res = res && (hasMsg || ended);
    }

    return res;
}

bool SpadesWorker::processInputMessagesAndCheckReady() {
    bool result = true;
    QList<Port*> inPorts = actor->getInputPorts();
    for (int i = 0; i < readsFetchers.size(); i++){
        const QString portId = readsFetchers[i].getPortId();
        Port* port = actor->getPort(portId);
        SAFE_POINT(port != NULL, QString("Port with id %1 not found").arg(portId), false);
        CHECK_CONTINUE(port->isEnabled());

        readsFetchers[i].processInputMessage();
        result = result && readsFetchers[i].hasFullDataset();
        CHECK(result, false);
    }

    return result;
}

int SpadesWorker::getReadsUrlSlotIdIndex(const QString& portId, bool& isPaired) const {
    int index = -1;
    isPaired = SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST.contains(portId);
    if (isPaired) {
        index = SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST.indexOf(portId);
    } else {
        index = SpadesWorkerFactory::IN_PORT_ID_LIST.indexOf(portId) + SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST.size();
    }

    return index;
}

void SpadesWorker::trySetDone(U2OpStatus &os) {
    CHECK(!isDone(), );

    bool isDone = true;
    bool hasReadyFetcher = false;
    bool hasDoneFetcher = false;
    for (int i = 0; i < readsFetchers.size(); i++){
        const QString portId = readsFetchers[i].getPortId();
        Port* port = actor->getPort(portId);
        SAFE_POINT(port != NULL, QString("Port with id %1 not found").arg(portId), );
        CHECK_CONTINUE(port->isEnabled());

        const bool fetcherHasFullDataset = readsFetchers[i].hasFullDataset();
        const bool fetcherIsDone = readsFetchers[i].isDone();
        hasReadyFetcher = hasReadyFetcher || fetcherHasFullDataset;
        hasDoneFetcher = hasDoneFetcher || fetcherIsDone;
        isDone = isDone && fetcherIsDone;
    }

    if (hasReadyFetcher && hasDoneFetcher) {
        os.setError(tr("Some input data elements sent data while some elements already finished their work. Check that all input data elements have the same datasets quantity."));
    }

    if (isDone ) {
        setDone();
        output->setEnded();
    }
}

void SpadesWorker::sl_taskFinished() {
    GenomeAssemblyMultiTask *t = dynamic_cast<GenomeAssemblyMultiTask*>(sender());
    if (!t->isFinished() || t->hasError() || t->isCanceled() || t->getResultUrl().isEmpty()) {
        return;
    }

    QString scaffoldUrl = t->getResultUrl();
     SpadesTask *spadesTask = qobject_cast<SpadesTask*>(t->getAssemblyTask());
     CHECK(spadesTask != NULL, );
     QString contigsUrl = spadesTask->getContigsUrl();

     QVariantMap data;
     data[SpadesWorkerFactory::SCAFFOLD_OUT_SLOT_ID] = qVariantFromValue<QString>(scaffoldUrl);
     data[SpadesWorkerFactory::CONTIGS_URL_OUT_SLOT_ID] = qVariantFromValue<QString>(contigsUrl);
     output->put(Message(output->getBusType(), data));

     context->getMonitor()->addOutputFile(scaffoldUrl, getActor()->getId());
     context->getMonitor()->addOutputFile(contigsUrl, getActor()->getId());

}

GenomeAssemblyTaskSettings SpadesWorker::getSettings( U2OpStatus &os ){
    GenomeAssemblyTaskSettings settings;

    settings.algName = ET_SPADES;
    settings.openView = false;
    QString outDir = getValue<QString>(SpadesWorkerFactory::OUTPUT_DIR);
    if (outDir.isEmpty()) {
        outDir = FileAndDirectoryUtils::createWorkingDir(context->workingDir(), FileAndDirectoryUtils::WORKFLOW_INTERNAL, "", context->workingDir());
    }
    outDir = GUrlUtils::createDirectory(outDir + "/" + SpadesWorkerFactory::BASE_SPADES_SUBDIR, "_", os);
    CHECK_OP(os, settings);
    if (outDir.endsWith("/")){
        outDir.chop(1);
    }

    settings.outDir = outDir;

    QMap<QString, QVariant> customSettings;
    customSettings.insert(SpadesTask::OPTION_THREADS, getValue<int>(SpadesTask::OPTION_THREADS));
    customSettings.insert(SpadesTask::OPTION_MEMLIMIT, getValue<int>(SpadesTask::OPTION_MEMLIMIT));
    customSettings.insert(SpadesTask::OPTION_K_MER, getValue<QString>(SpadesTask::OPTION_K_MER));
    customSettings.insert(SpadesTask::OPTION_INPUT_DATA, getValue<QVariantMap>(SpadesTask::OPTION_INPUT_DATA));
    customSettings.insert(SpadesTask::OPTION_DATASET_TYPE, getValue<QString>(SpadesTask::OPTION_DATASET_TYPE));
    customSettings.insert(SpadesTask::OPTION_RUNNING_MODE, getValue<QString>(SpadesTask::OPTION_RUNNING_MODE));

    settings.setCustomSettings(customSettings);

    return settings;
}

/************************************************************************/
/* Factory */
/************************************************************************/

static const QString INPUT_DATA_DESCRIPTION = QCoreApplication::tr("<html>"
    "Select the type of input for SPAdes. URL(s) to the input files of the selected type(s) should be provided to the corresponding port(s) of the workflow element."
    "<p>At least one library of the following types is required:"
    "<ul>"
    "<li>Illumina paired-end/high-quality mate-pairs/unpaired reads</li>"
    "<li>IonTorrent paired-end/high-quality mate-pairs/unpaired reads</li>"
    "<li>PacBio CCS reads (at least 5 reads coverage is recommended)</li>"
    "</ul></p>"
    "<p>It is strongly suggested to provide multiple paired-end and mate-pair libraries according to their insert size (from smallest to longest).</p>"
    "<p>Additionally, one may input Oxford Nanopore reads, Sanger reads, contigs generated by other assembler(s), etc."
    "Note that Illumina and IonTorrent libraries should not be assembled together. All other types of input data are compatible.</p>"
    "<p>It is also possible to set up reads orientation (forward-reverse (fr), reverse-forward (rf), forward-forward (ff)) and specify whether paired reads are separate or interlaced.</p>"
    "<p>Illumina, IonTorrent or PacBio CCS reads should be provided in FASTQ format.<br>"
    "Illumina or PacBio read may also be provided in FASTA format. Error correction should be skipped in this case (see the \"Running mode\" parameter).<br>"
    "Sanger, Oxford Nanopore and PacBio CLR reads can be provided in both formats since SPAdes does not run error correction for these types of data.</p>"
    "</html>");

void SpadesWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;

    //in port
    QList<Descriptor> readDescriptors;
    foreach(const QString& readId, QStringList() << IN_PORT_PAIRED_ID_LIST << IN_PORT_ID_LIST) {
        const QString readName = SpadesWorkerFactory::getPortNameById(readId);
        readDescriptors << Descriptor(readId,
            SpadesWorker::tr("Input %1 reads").arg(readName),
            SpadesWorker::tr("Input %1 reads to be assembled with Spades.").arg(readName));
    }

    QList<Descriptor> inputDescriptors;
    foreach(const QString& id, READS_URL_SLOT_ID_LIST) {
        inputDescriptors << Descriptor(id,
            SpadesWorker::tr("URL of a file with reads"),
            SpadesWorker::tr("Input reads to be assembled."));
    }
    SAFE_POINT(READS_URL_SLOT_ID_LIST.size() == inputDescriptors.size(),
               "Incorrect descriptors quantity", );

    QList<Descriptor> inputPairedDescriptors;
    foreach(const QString& pairedId, READS_PAIRED_URL_SLOT_ID_LIST) {
        inputPairedDescriptors << Descriptor(pairedId,
            SpadesWorker::tr("URL of a file with right pair reads"),
            SpadesWorker::tr("Input right pair reads to be assembled."));
    }
    SAFE_POINT(READS_PAIRED_URL_SLOT_ID_LIST.size() == inputPairedDescriptors.size(),
               "Incorrect paired descriptors quantity", );

    QList<QMap<Descriptor, DataTypePtr> > inTypeMapList;
    for (int i = 0; i < inputDescriptors.size(); i++) {
        const Descriptor& desc = inputDescriptors[i];

        QMap<Descriptor, DataTypePtr> inTypeMap;
        inTypeMap[desc] = BaseTypes::STRING_TYPE();
        if (i < inputPairedDescriptors.size()) {
            const Descriptor& pairedDesc = inputPairedDescriptors[i];
            inTypeMap[pairedDesc] = BaseTypes::STRING_TYPE();
        }

        DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID_LIST[i], inTypeMap));
        portDescs << new PortDescriptor(readDescriptors[i], inTypeSet, true);
    }

    //out port
    QMap<Descriptor, DataTypePtr> outTypeMap;
    Descriptor scaffoldOutDesc(SCAFFOLD_OUT_SLOT_ID,
        SpadesWorker::tr("Scaffolds URL"),
        SpadesWorker::tr("Output scaffolds URL."));

    Descriptor contigsOutDesc(CONTIGS_URL_OUT_SLOT_ID,
        SpadesWorker::tr("Contigs URL"),
        SpadesWorker::tr("Output contigs URL."));

    Descriptor outPortDesc(OUT_PORT_DESCR,
        SpadesWorker::tr("Output File"),
        SpadesWorker::tr("Output assembly files."));

    outTypeMap[scaffoldOutDesc] = BaseTypes::STRING_TYPE();
    outTypeMap[contigsOutDesc] = BaseTypes::STRING_TYPE();

    DataTypePtr outTypeSet(new MapDataType(OUT_TYPE_ID, outTypeMap));
    portDescs << new PortDescriptor(outPortDesc, outTypeSet, false, true);


     QList<Attribute*> attrs;
     {
         Descriptor outDir(OUTPUT_DIR,
             SpadesWorker::tr("Output folder"),
             SpadesWorker::tr("Folder to save Spades output files."));

         Descriptor threads(SpadesTask::OPTION_THREADS,
             SpadesWorker::tr("Number of threads"),
             SpadesWorker::tr("Number of threads (-t)."));

         Descriptor memLim(SpadesTask::OPTION_MEMLIMIT,
             SpadesWorker::tr("Memory limit (GB)"),
             SpadesWorker::tr("Memory limit (-m)."));

         Descriptor datasetType(SpadesTask::OPTION_DATASET_TYPE,
             SpadesWorker::tr("Dataset type"),
             SpadesWorker::tr("Input dataset type."));

         Descriptor rMode(SpadesTask::OPTION_RUNNING_MODE,
             SpadesWorker::tr("Running mode"),
             SpadesWorker::tr("Running mode."));

         Descriptor kMer(SpadesTask::OPTION_K_MER,
             SpadesWorker::tr("K-mers"),
             SpadesWorker::tr("k-mer sizes (-k)."));

         Descriptor inputData(SpadesTask::OPTION_INPUT_DATA,
             SpadesWorker::tr("Input data"),
             INPUT_DATA_DESCRIPTION);
        attrs << new Attribute(datasetType, BaseTypes::STRING_TYPE(), true, QVariant("Multi Cell"));
        attrs << new Attribute(rMode, BaseTypes::STRING_TYPE(), true, QVariant("Error Correction and Assembly"));
        attrs << new Attribute(kMer, BaseTypes::STRING_TYPE(), true, QVariant("auto"));

        QVariantMap defaultValue;
        defaultValue.insert(IN_PORT_PAIRED_ID_LIST[0], QString("%1:%2").arg(ORIENTATION_FR).arg(TYPE_SINGLE));
        defaultValue.insert(SEQUENCING_PLATFORM_ID, "Illumina");
        Attribute* inputAttr = new Attribute(inputData, BaseTypes::MAP_TYPE(), false, QVariant::fromValue<QVariantMap>(defaultValue));
        foreach (const QString& read, IN_PORT_ID_LIST) {
            inputAttr->addPortRelation(new SpadesPortRelationDescriptor(read, QVariantList() << read));
        }
        foreach (const QString& pairedRead, IN_PORT_PAIRED_ID_LIST) {
            inputAttr->addPortRelation(new SpadesPortRelationDescriptor(pairedRead, QVariantList() << pairedRead));
        }
        attrs << inputAttr;

        attrs << new Attribute(threads, BaseTypes::NUM_TYPE(), false, QVariant(16));
        attrs << new Attribute(memLim, BaseTypes::NUM_TYPE(), false, QVariant(250));
        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), Attribute::CanBeEmpty | Attribute::Required);
     }

     QMap<QString, PropertyDelegate*> delegates;
     {
         DelegateTags outputUrlTags;
         outputUrlTags.set(DelegateTags::PLACEHOLDER_TEXT, "Auto");
         delegates[OUTPUT_DIR] = new URLDelegate(outputUrlTags, "spades/output", false, true);

         QVariantMap spinMap; spinMap["minimum"] = QVariant(1); spinMap["maximum"] = QVariant(INT_MAX);
         delegates[SpadesTask::OPTION_THREADS]  = new SpinBoxDelegate(spinMap);
         delegates[SpadesTask::OPTION_MEMLIMIT]  = new SpinBoxDelegate(spinMap);

         QVariantMap contentMap;
         contentMap["Multi Cell"] = "Multi Cell";
         contentMap["Single Cell"] = "Single Cell";
         delegates[SpadesTask::OPTION_DATASET_TYPE] = new ComboBoxDelegate(contentMap);

         QVariantMap contentMap2;
         contentMap2["Error Correction and Assembly"] = "Error Correction and Assembly";
         contentMap2["Assembly only"] = "Assembly only";
         contentMap2["Error correction only"] = "Error correction only";
         delegates[SpadesTask::OPTION_RUNNING_MODE] = new ComboBoxDelegate(contentMap2);

         delegates[SpadesTask::OPTION_INPUT_DATA] = new SpadesDelegate();
    }

    Descriptor protoDesc(SpadesWorkerFactory::ACTOR_ID,
        SpadesWorker::tr("Assemble Reads with SPAdes"),
        SpadesWorker::tr("Performes assembly of input short reads."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setPrompter(new SpadesPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    for (int i = 0; i < IN_PORT_PAIRED_ID_LIST.size(); i++) {
        proto->setPortValidator(IN_PORT_PAIRED_ID_LIST[i], new PairedReadsPortValidator(READS_URL_SLOT_ID_LIST[i], READS_PAIRED_URL_SLOT_ID_LIST[i]));
    }
    for (int i = 0; i < IN_PORT_ID_LIST.size(); i++) {
        const int index = READS_PAIRED_URL_SLOT_ID_LIST.size() + i;
        proto->setPortValidator(IN_PORT_ID_LIST[i], new PairedReadsPortValidator(READS_URL_SLOT_ID_LIST[index], QString()));
    }
    proto->addExternalTool(ET_SPADES);
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_NGS_MAP_ASSEMBLE_READS(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new SpadesWorkerFactory());
}

Worker *SpadesWorkerFactory::createWorker(Actor *a) {
    return new SpadesWorker(a);
}

QString SpadesPrompter::composeRichDoc() {
    QString res = "";

    Actor* readsProducer =
        qobject_cast<IntegralBusPort*>(target->getPort(SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST.first()))
                                                ->getProducer(SpadesWorkerFactory::READS_URL_SLOT_ID_LIST.first());

    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString readsUrl = readsProducer ? readsProducer->getLabel() : unsetStr;

    res.append(tr("Assemble reads from <u>%1</u>.").arg(readsUrl));

    return res;
}

} // LocalWorkflow
} // U2

