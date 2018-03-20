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
#include <U2Core/AppResources.h>
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/Counter.h>
#include <U2Core/DataPathRegistry.h>
#include <U2Core/DocumentImport.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/FailTask.h>
#include <U2Core/FileAndDirectoryUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/ActorValidator.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>

#include "EnsembleClassificationWorker.h"
#include "TaxonomySupport.h"

namespace U2 {
namespace LocalWorkflow {

///////////////////////////////////////////////////////////////
//EnsembleClassification
const QString EnsembleClassificationWorkerFactory::ACTOR_ID("ensemble-classification");

static const QString INPUT_PORT("in");
static const QString INPUT_SLOT1("tax_data1");
static const QString INPUT_SLOT2("tax_data2");
static const QString INPUT_SLOT3("tax_data3");

static const QString OUTPUT_PORT("out");
static const QString OUTPUT_SLOT = BaseSlots::URL_SLOT().getId();

static const QString OUT_FILE("out-file");

QString EnsembleClassificationPrompter::composeRichDoc() {
    const QString outFile = getHyperlink(OUT_FILE, getURL(OUT_FILE));
    return tr("Ensemble classification data from other elements into %1").arg(outFile);
}

class EnsembleSlotValidator : public ActorValidator {
public:
    bool validate(const Actor *actor, ProblemList &problemList, const QMap<QString, QString> &) const;
};

bool EnsembleSlotValidator::validate(const Actor *actor, ProblemList &problemList, const QMap<QString, QString> &) const {
    bool res = true;

    Port* inputPort = actor->getPort(INPUT_PORT);
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(inputPort);
    SAFE_POINT(NULL != input, QString("Port with id '%1' is NULL").arg(INPUT_PORT), false);

    QList<Actor*> producers1 = input->getProducers(INPUT_SLOT1);
    QList<Actor*> producers2 = input->getProducers(INPUT_SLOT2);
    if (producers1.isEmpty() || producers2.isEmpty()) {
        res = false;
        problemList.append(Problem(EnsembleClassificationPrompter::tr("It is required to input taxonomy data for at least the first and the second slot."), actor->getId()));
    }

    return res;
}

/************************************************************************/
/* EnsembleClassificationWorkerFactory */
/************************************************************************/
void EnsembleClassificationWorkerFactory::init() {

    Descriptor desc(ACTOR_ID, EnsembleClassificationWorker::tr("Ensemble Classification Data"),
        EnsembleClassificationWorker::tr("The element ensembles data, produced by classification tools "
                                         "(Kraken, CLARK, DIAMOND), into a single file in CSV format. "
                                         "This file can be used as input for the WEVOTE classifier.") );

    QList<PortDescriptor*> p;
    {
        Descriptor inD(INPUT_PORT, EnsembleClassificationWorker::tr("Input taxonomy data"),
                       EnsembleClassificationWorker::tr("Three input slots are available for taxonomy classification data. "
                                                        "At least first and second slots should be connected to classification data slots."
                ));
        Descriptor outD(OUTPUT_PORT, EnsembleClassificationWorker::tr("Ensembled classification"),
                        EnsembleClassificationWorker::tr("URL to the CSV file with ensembled classification data."));


        Descriptor inSlot1(INPUT_SLOT1, EnsembleClassificationWorker::tr("Input tax data 1"), EnsembleClassificationWorker::tr("Input tax data 1."));
        Descriptor inSlot2(INPUT_SLOT2, EnsembleClassificationWorker::tr("Input tax data 2"), EnsembleClassificationWorker::tr("Input tax data 2."));
        Descriptor inSlot3(INPUT_SLOT3, EnsembleClassificationWorker::tr("Input tax data 3"), EnsembleClassificationWorker::tr("Input tax data 3."));

        QMap<Descriptor, DataTypePtr> inM;
        inM[inSlot1] = TaxonomySupport::TAXONOMY_CLASSIFICATION_TYPE();
        inM[inSlot2] = TaxonomySupport::TAXONOMY_CLASSIFICATION_TYPE();
        inM[inSlot3] = TaxonomySupport::TAXONOMY_CLASSIFICATION_TYPE();

        p << new PortDescriptor(inD, DataTypePtr(new MapDataType("ensemble.input", inM)), true);

        Descriptor outSlot(OUTPUT_SLOT, EnsembleClassificationWorker::tr("Output URL"), EnsembleClassificationWorker::tr("Output URL."));

        QMap<Descriptor, DataTypePtr> outM;
        outM[outSlot] = BaseTypes::STRING_TYPE();
        p << new PortDescriptor(outD, DataTypePtr(new MapDataType("filter.output-url", outM)), false, true);
    }

    QList<Attribute*> a;
    {
        Descriptor outFileDesc(OUT_FILE, EnsembleClassificationWorker::tr("Output file"),
            EnsembleClassificationWorker::tr("Specify the output file. The classification data are stored in CSV format with the following columns:"
                                             "<ol><li> a sequence name"
                                             "<li>taxID from the first tool"
                                             "<li>taxID from the second tool"
                                             "<li>optionally, taxID from the third tool</ol>"
                                           ));

        Attribute *outFileAttribute = new Attribute(outFileDesc, BaseTypes::STRING_TYPE(), true, "ensemble.csv");
        a << outFileAttribute;
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        delegates[OUT_FILE] = new URLDelegate(".csv", "classification/ensemble");
    }

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new EnsembleClassificationPrompter());
    proto->setValidator(new EnsembleSlotValidator());

    WorkflowEnv::getProtoRegistry()->registerProto(NgsReadsClassificationPlugin::WORKFLOW_ELEMENTS_GROUP, proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new EnsembleClassificationWorkerFactory());
}

void EnsembleClassificationWorkerFactory::cleanup() {
    delete WorkflowEnv::getProtoRegistry()->unregisterProto(ACTOR_ID);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    delete localDomain->unregisterEntry(ACTOR_ID);
}


/************************************************************************/
/* EnsembleClassificationWorker */
/************************************************************************/
EnsembleClassificationWorker::EnsembleClassificationWorker(Actor *a)
:BaseWorker(a, false), input(NULL), /*pairedOutput(NULL),*/ output(NULL), tripleInput(false)
{
}

void EnsembleClassificationWorker::init() {
    input = ports.value(INPUT_PORT);
    output = ports.value(OUTPUT_PORT);

    SAFE_POINT(NULL != input, QString("Port with id '%1' is NULL").arg(INPUT_PORT), );
    SAFE_POINT(NULL != output, QString("Port with id '%1' is NULL").arg(OUTPUT_PORT), );

    output->addComplement(input);
    input->addComplement(output);

    outputFile = getValue<QString>(OUT_FILE);

    Port* inputPort = actor->getPort(INPUT_PORT);
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(inputPort);
    QList<Actor*> producers3 = input->getProducers(INPUT_SLOT3);
    tripleInput = !producers3.isEmpty();
}

Task * EnsembleClassificationWorker::tick() {
    if (input->hasMessage()) {
        const Message message = getMessageAndSetupScriptValues(input);
        QVariantMap data = message.getData().toMap();

        Task* t = new EnsembleClassificationTask(data, tripleInput, outputFile, context->workingDir());
        connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task *)), SLOT(sl_taskFinished(Task *)));
        return t;
    }

    if (input->isEnded()) {
        setDone();
        algoLog.info("Ensemble worker is done as input has ended");
        output->setEnded();
    }

    return NULL;
}

void EnsembleClassificationWorker::sl_taskFinished(Task *t)
{
    EnsembleClassificationTask *task = qobject_cast<EnsembleClassificationTask *>(t);
    SAFE_POINT(NULL != task, "Invalid task is encountered", );
    if (!task->isFinished() || task->hasError() || task->isCanceled()) {
        return;
    }
    QString reportUrl = task->getOutputFile();
    QVariantMap m;
    m[OUTPUT_SLOT] = reportUrl;
    output->put(Message(output->getBusType(), m/*, metadata.getId()*/));
    monitor()->addOutputFile(reportUrl, getActor()->getId());
    if (task->foundMismatches()) {
        QString msg = tr("Different taxonomy data do not match. Some sequence names were skipped.");
        algoLog.info(msg);
        monitor()->addInfo(msg, getActorId(), Problem::U2_WARNING);
    }
}

void EnsembleClassificationTask::run() {
    TaxonomyClassificationResult tax1 = data[INPUT_SLOT1].value<U2::LocalWorkflow::TaxonomyClassificationResult>();
    TaxonomyClassificationResult tax2 = data[INPUT_SLOT2].value<U2::LocalWorkflow::TaxonomyClassificationResult>();
    TaxonomyClassificationResult tax3 = data[INPUT_SLOT3].value<U2::LocalWorkflow::TaxonomyClassificationResult>();

    QStringList seqs = tax1.keys();
    seqs << tax2.keys();
    if (tripleInput) {
        seqs << tax3.keys();
    }
    CHECK_OP(stateInfo, );
    seqs.removeDuplicates();
    CHECK_OP(stateInfo, );
    seqs.sort();
    QString csv;csv.reserve(seqs.size() * 64);
    int counter = 0;
    foreach (QString seq, seqs) {
        CHECK_OP(stateInfo, );
        stateInfo.setProgress(++counter * 100 /seqs.size());

        TaxID id1 = tax1.value(seq, TaxonomyTree::UNDEFINED_ID);
        TaxID id2 = tax2.value(seq, TaxonomyTree::UNDEFINED_ID);
        TaxID id3 = tax3.value(seq, TaxonomyTree::UNDEFINED_ID);
        if (id1 == TaxonomyTree::UNDEFINED_ID) {
            QString msg = tr("Taxonomy classification for '%1' is missing from %2 slot").arg(seq).arg(INPUT_SLOT1);
            algoLog.trace(msg);
            hasMissing = true;
            continue;
        }
        if (id2 == TaxonomyTree::UNDEFINED_ID) {
            QString msg = tr("Taxonomy classification for '%1' is missing from %2 slot").arg(seq).arg(INPUT_SLOT2);
            algoLog.trace(msg);
            hasMissing = true;
            continue;
        }
        if (tripleInput && id3 == TaxonomyTree::UNDEFINED_ID) {
            QString msg = tr("Taxonomy classification for '%1' is missing from %2 slot").arg(seq).arg(INPUT_SLOT3);
            algoLog.trace(msg);
            hasMissing = true;
            continue;
        }
        csv.append(seq).append(',').append(QString::number(id1)).append(',').append(QString::number(id2));
        if (tripleInput) {
            csv.append(',').append(QString::number(id3));
        }
        csv.append('\n');
    }

    if (!QFileInfo(outputFile).isAbsolute()) {
        QString tmpDir = FileAndDirectoryUtils::createWorkingDir(workingDir, FileAndDirectoryUtils::WORKFLOW_INTERNAL, "", workingDir);
        tmpDir = GUrlUtils::createDirectory(tmpDir, "_", stateInfo);
        CHECK_OP(stateInfo, );
        outputFile = tmpDir + '/' + outputFile;
    }

    QFile csvFile(outputFile);
    csvFile.open(QIODevice::Append);
    csvFile.write(csv.toLocal8Bit());
    csvFile.close();
}

EnsembleClassificationTask::EnsembleClassificationTask(const QVariantMap &data, const bool tripleInput, const QString &outputFile, const QString &workingDir)
    : Task(tr("Ensemble different classifications"), TaskFlag_None), data(data), tripleInput(tripleInput), workingDir(workingDir), outputFile(outputFile), hasMissing(false)
{

}

} //LocalWorkflow
} //U2
