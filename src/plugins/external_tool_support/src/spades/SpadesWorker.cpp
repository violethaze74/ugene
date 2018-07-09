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

#include <U2Core/FailTask.h>
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

const QString SpadesWorkerFactory::READS_URL_SLOT_ID = "readsurl";
const QString SpadesWorkerFactory::READS_PAIRED_URL_SLOT_ID = "readspairedurl";

const QString SpadesWorkerFactory::IN_TYPE_ID = "spades-data";
const QString SpadesWorkerFactory::IN_PAIRED_TYPE_ID = "spades-paired-data";

const QString SpadesWorkerFactory::OUT_TYPE_ID = "spades-data-out";

const QString SpadesWorkerFactory::SCAFFOLD_OUT_SLOT_ID = "scaffolds-out";

const QString SpadesWorkerFactory::REQUIRED_SEQUENCING_PLATFORM_ID = "required-platform-id";
const QString SpadesWorkerFactory::ADDITIONAL_SEQUENCING_PLATFORM_ID = "additional-platform-id";

const QStringList SpadesWorkerFactory::IN_PORT_ID_LIST = QStringList() <<
                                                   "in-unpaired-reads" <<
                                                "in-pac-bio-ccs-reads" <<
                                                "in-pac-bio-clr-reads" <<
                                            "in-oxford-nanopore-reads" <<
                                                     "in-sanger-reads" <<
                                                  "in-trusted-contigs" <<
                                                  "in-untrusted-contigs";

const QStringList SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST = QStringList() <<
                                                                    "in-data" <<
                                                              "in-mate-pairs" <<
                                                   "in-high-quality-mate-pairs";

const QString SpadesWorkerFactory::MAP_TYPE_ID = "map";

const QString SpadesWorkerFactory::IN_PORT_DESCR = "in-data";

const QString SpadesWorkerFactory::OUT_PORT_DESCR = "out-data";

const QString SpadesWorkerFactory::OUTPUT_DIR = "output-dir";

const QString SpadesWorkerFactory::BASE_SPADES_SUBDIR = "spades";


/************************************************************************/
/* Worker */
/************************************************************************/
SpadesWorker::SpadesWorker(Actor *p)
: BaseWorker(p, false)
, inChannel(NULL)
, output(NULL)
{

}

void SpadesWorker::init() {
    inChannel = ports.value(SpadesWorkerFactory::IN_PORT_DESCR);
    output = ports.value(SpadesWorkerFactory::OUT_PORT_DESCR);
}

Task *SpadesWorker::tick() {
    if (inChannel->hasMessage()) {
        U2OpStatus2Log os;

        Message m = getMessageAndSetupScriptValues(inChannel);
        QVariantMap data = m.getData().toMap();

        GenomeAssemblyTaskSettings settings = getSettings(os);
        if (os.hasError()) {
            return new FailTask(os.getError());
        }

        QVariant inputData = settings.getCustomValue(SpadesTask::OPTION_INPUT_DATA, QVariant());
        SAFE_POINT(inputData != QVariant(), "Incorrect input data", NULL);

        QVariantMap mapData = inputData.toMap();

        QString readsUrl = data[SpadesWorkerFactory::READS_URL_SLOT_ID].toString();
        QString readsPairedUrl = data[SpadesWorkerFactory::READS_PAIRED_URL_SLOT_ID].toString();

        QStringList dataKeys = mapData.keys();
        foreach (const QString& read, SpadesWorkerFactory::IN_PORT_ID_LIST) {
            CHECK_CONTINUE(dataKeys.contains(read));

            AssemblyReads assemblyRead;
            assemblyRead.left = readsUrl;
            assemblyRead.libNumber = "1";
            assemblyRead.libName = read;

            settings.reads << assemblyRead;
        }
        foreach (const QString& pairedRead, SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST) {
            CHECK_CONTINUE(dataKeys.contains(pairedRead));

            AssemblyReads assemblyRead;
            assemblyRead.left = readsUrl;
            assemblyRead.right = readsPairedUrl;
            assemblyRead.libNumber = "1";
            assemblyRead.libName = pairedRead;
            QStringList values = mapData[pairedRead].toString().split(":");
            assemblyRead.orientation = values.first();
            assemblyRead.readType = values.last();

            settings.reads << assemblyRead;
        }

        AssemblyReads read;
        read.left = readsUrl;
        read.libNumber = "1";
        read.orientation = ORIENTATION_FR;
        read.libType = PAIR_TYPE_DEFAULT;

        if (data.contains(SpadesWorkerFactory::READS_PAIRED_URL_SLOT_ID)){
            QString readsPairedUrl = data[SpadesWorkerFactory::READS_PAIRED_URL_SLOT_ID].toString();
            read.libType = PAIR_TYPE_DEFAULT;
            read.libName = LIBRARY_PAIRED;
            read.right = readsPairedUrl;
        }else {
            read.libName = LIBRARY_SINGLE;
        }

        //settings.reads /*= QList<AssemblyReads>()*/ << read;

        settings.listeners = createLogListeners();
        GenomeAssemblyMultiTask* t = new GenomeAssemblyMultiTask(settings);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }else if (inChannel->isEnded()) {
        setDone();
        output->setEnded();
    }
    return NULL;
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

void SpadesWorker::sl_taskFinished() {
    GenomeAssemblyMultiTask *t = dynamic_cast<GenomeAssemblyMultiTask*>(sender());
    if (!t->isFinished() || t->hasError() || t->isCanceled() || t->getResultUrl().isEmpty()) {
        return;
    }


     QString url = t->getResultUrl();

     QVariantMap data;
     data[SpadesWorkerFactory::SCAFFOLD_OUT_SLOT_ID] = qVariantFromValue<QString>(url);
     output->put(Message(output->getBusType(), data));

    context->getMonitor()->addOutputFile(url, getActor()->getId());

    if (inChannel->isEnded() && !inChannel->hasMessage()) {
        setDone();
        output->setEnded();
    }
}

GenomeAssemblyTaskSettings SpadesWorker::getSettings( U2OpStatus &os ){
    GenomeAssemblyTaskSettings settings;

    settings.algName = ET_SPADES;
    settings.openView = false;

    QString outDir = GUrlUtils::createDirectory(
        getValue<QString>(SpadesWorkerFactory::OUTPUT_DIR) + "/" + SpadesWorkerFactory::BASE_SPADES_SUBDIR,
        "_", os);
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

static const QString INPUT_DATA_DESCRIPTION = QObject::tr("<html><head></head><body>"
    "<p>Select the type of input for SPAdes. URL(s) to the input files of the selected type(s) should be provided to the corresponding port(s) of the workflow element.</p>"
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
    "</body></html>");

class SpadesInputSlotsValidator : public PortValidator {
    public:

    bool validate(const IntegralBusPort *port, ProblemList &problemList) const {
        QVariant busMap = port->getParameter(Workflow::IntegralBusPort::BUS_MAP_ATTR_ID)->getAttributePureValue();
        bool data = isBinded(busMap.value<StrStrMap>(), SpadesWorkerFactory::READS_URL_SLOT_ID);
        if (!data){
            QString dataName = slotName(port, SpadesWorkerFactory::READS_URL_SLOT_ID);
            problemList.append(Problem(IntegralBusPort::tr("The slot must be not empty: '%1'").arg(dataName)));
            return false;
        }


        QString slot1Val = busMap.value<StrStrMap>().value(SpadesWorkerFactory::READS_URL_SLOT_ID);
        QString slot2Val = busMap.value<StrStrMap>().value(SpadesWorkerFactory::READS_PAIRED_URL_SLOT_ID);
        U2OpStatusImpl os;
        const QList<IntegralBusSlot>& slots1 = IntegralBusSlot::listFromString(slot1Val, os);
        const QList<IntegralBusSlot>& slots2 = IntegralBusSlot::listFromString(slot2Val, os);

        bool hasCommonElements = false;

        foreach(const IntegralBusSlot& ibsl1, slots1){
            if (hasCommonElements){
                break;
            }
            foreach(const IntegralBusSlot& ibsl2, slots2){
                if (ibsl1 == ibsl2){
                    hasCommonElements = true;
                    break;
                }
            }
        }

        if (hasCommonElements){
            problemList.append(Problem(SpadesWorker::tr("SPAdes cannot recognize read pairs from the same file. Please, perform demultiplexing first.")));
            return false;
        }

        return true;
    }
    };

void SpadesWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;

    //in port
    Descriptor readsDesc(READS_URL_SLOT_ID,
        SpadesWorker::tr("URL of a file with reads"),
        SpadesWorker::tr("Input reads to be assembled."));
    Descriptor readsPairedDesc(READS_PAIRED_URL_SLOT_ID,
        SpadesWorker::tr("URL of a file with right pair reads"),
        SpadesWorker::tr("Input right pair reads to be assembled."));

    QMap<Descriptor, DataTypePtr> inTypeMap;
    inTypeMap[readsDesc] = BaseTypes::STRING_TYPE();

    QMap<Descriptor, DataTypePtr> inTypeMapPaired;
    inTypeMapPaired[readsDesc] = BaseTypes::STRING_TYPE();
    inTypeMapPaired[readsPairedDesc] = BaseTypes::STRING_TYPE();

    QList<Descriptor> readDescriptors;
    foreach (const QString& readId, IN_PORT_ID_LIST) {
        readDescriptors << Descriptor(readId,
            SpadesWorker::tr("Input %1 reads").arg(readId),
            SpadesWorker::tr("Input %1 reads to be assembled with Spades.").arg(readId));
    }

    QList<Descriptor> readPairedDescriptors;
    foreach (const QString& readId, IN_PORT_PAIRED_ID_LIST) {
        readPairedDescriptors << Descriptor(readId,
            SpadesWorker::tr("Input %1 reads").arg(readId),
            SpadesWorker::tr("Input %1 reads to be assembled with Spades.").arg(readId));
    }

    DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID, inTypeMap));
    DataTypePtr inTypeSetPaired(new MapDataType(IN_PAIRED_TYPE_ID, inTypeMapPaired));

    foreach (const Descriptor& readDesc, readDescriptors) {
        portDescs << new PortDescriptor(readDesc, inTypeSet, true);
    }
    foreach (const Descriptor& readPairedDesc, readPairedDescriptors) {
        portDescs << new PortDescriptor(readPairedDesc, inTypeSetPaired, true);
    }

    //out port
    QMap<Descriptor, DataTypePtr> outTypeMap;
    Descriptor scaffoldOutDesc(SCAFFOLD_OUT_SLOT_ID,
        SpadesWorker::tr("Scaffolds URL"),
        SpadesWorker::tr("Output scaffolds URL."));

    Descriptor outPortDesc(OUT_PORT_DESCR,
        SpadesWorker::tr("Output File"),
        SpadesWorker::tr("Output assembly files."));

    outTypeMap[scaffoldOutDesc] = BaseTypes::STRING_TYPE();

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
             SpadesWorker::tr("Input Data"),
             SpadesWorker::tr(INPUT_DATA_DESCRIPTION.toLatin1()));

        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(datasetType, BaseTypes::STRING_TYPE(), true, QVariant("Multi Cell"));
        attrs << new Attribute(rMode, BaseTypes::STRING_TYPE(), true, QVariant("Error Correction and Assembly"));
        attrs << new Attribute(kMer, BaseTypes::STRING_TYPE(), true, QVariant("auto"));

        QMap<Descriptor, DataTypePtr> inputAttrMap;
        foreach (const Descriptor& desc, readDescriptors) {
            inputAttrMap[desc] = BaseTypes::STRING_TYPE();
        }
        foreach (const Descriptor& pairedDesc, readPairedDescriptors) {
            inputAttrMap[pairedDesc] = BaseTypes::STRING_TYPE();
        }
        DataTypeRegistry* registry = WorkflowEnv::getDataTypeRegistry();
        SAFE_POINT(registry != NULL, "Registry is not found", );

        DataTypePtr mapDataType(new MapDataType(MAP_TYPE_ID, inputAttrMap));
        registry->registerEntry(mapDataType);

        QMap<QString, QVariant> defaultValue;
        defaultValue.insert(IN_PORT_PAIRED_ID_LIST[0], QString("%1:%2").arg(ORIENTATION_FR).arg(TYPE_SINGLE));
        defaultValue.insert(REQUIRED_SEQUENCING_PLATFORM_ID, "Illumina");
        Attribute* inputAttr = new Attribute(inputData, BaseTypes::MAP_TYPE(), false, QVariant::fromValue<QMap<QString, QVariant>>(defaultValue));
        foreach (const QString& read, IN_PORT_ID_LIST) {
            inputAttr->addPortRelation(new SpadesPortRelationDescriptor(read, QVariantList() << read));
        }
        foreach (const QString& pairedRead, IN_PORT_PAIRED_ID_LIST) {
            inputAttr->addPortRelation(new SpadesPortRelationDescriptor(pairedRead, QVariantList() << pairedRead));
        }
        attrs << inputAttr;

        attrs << new Attribute(threads, BaseTypes::NUM_TYPE(), false, QVariant(16));
        attrs << new Attribute(memLim, BaseTypes::NUM_TYPE(), false, QVariant(250));
     }

     QMap<QString, PropertyDelegate*> delegates;
     {
         delegates[OUTPUT_DIR] = new URLDelegate("", "", false, true);

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
    proto->setPortValidator(IN_PORT_DESCR, new SpadesInputSlotsValidator());
    proto->addExternalTool(ET_SPADES);
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_NGS_MAP_ASSEMBLE_READS(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new SpadesWorkerFactory());
}

Worker *SpadesWorkerFactory::createWorker(Actor *a) {
    return new SpadesWorker(a);
}

QString SpadesPrompter::composeRichDoc() {
    QString res = "";

    Actor* readsProducer = qobject_cast<IntegralBusPort*>(target->getPort(SpadesWorkerFactory::IN_PORT_DESCR))->getProducer(SpadesWorkerFactory::READS_URL_SLOT_ID);

    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString readsUrl = readsProducer ? readsProducer->getLabel() : unsetStr;

    res.append(tr("Assemble reads from <u>%1</u>.").arg(readsUrl));

    return res;
}

} // LocalWorkflow
} // U2

