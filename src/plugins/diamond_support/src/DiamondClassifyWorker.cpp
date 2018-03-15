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
#include <U2Core/FailTask.h>
#include <U2Core/FileAndDirectoryUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Lang/BaseSlots.h>
#include <U2Lang/WorkflowMonitor.h>

#include "DiamondClassifyWorker.h"
#include "DiamondClassifyWorkerFactory.h"
#include "DiamondSupport.h"
#include "../../ngs_reads_classification/src/GetReadListWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString DiamondClassifyWorker::DIAMOND_DIR = "diamond";

DiamondClassifyWorker::DiamondClassifyWorker(Actor *actor)
    : BaseWorker(actor, false),
      input(NULL),
      pairedInput(NULL),
      output(NULL),
      pairedReadsInput(false)
{

}

void DiamondClassifyWorker::init() {
    input = ports.value(DiamondClassifyWorkerFactory::INPUT_PORT_ID);
    pairedInput = ports.value(DiamondClassifyWorkerFactory::INPUT_PAIRED_PORT_ID);
    output = ports.value(DiamondClassifyWorkerFactory::OUTPUT_PORT_ID);

    SAFE_POINT(NULL != input, QString("Port with id '%1' is NULL").arg(DiamondClassifyWorkerFactory::INPUT_PORT_ID), );
//    SAFE_POINT(NULL != pairedInput, QString("Port with id '%1' is NULL").arg(DiamondClassifyWorkerFactory::INPUT_PAIRED_PORT_ID), );
    SAFE_POINT(NULL != output, QString("Port with id '%1' is NULL").arg(DiamondClassifyWorkerFactory::OUTPUT_PORT_ID), );

    pairedReadsInput = (getValue<QString>(DiamondClassifyWorkerFactory::INPUT_DATA_ATTR_ID) == DiamondClassifyTaskSettings::PAIRED_END);

    // FIXME: the second port is not taken into account
    output->addComplement(input);
    input->addComplement(output);
}

Task *DiamondClassifyWorker::tick() {
    if (isReadyToRun()) {
        U2OpStatus2Log os;
        DiamondClassifyTaskSettings settings = getSettings(os);
        if (os.hasError()) {
            return new FailTask(os.getError());
        }

        DiamondClassifyTask *task = new DiamondClassifyTask(settings);
        task->addListeners(createLogListeners(pairedReadsInput ? 2 : 1));
        connect(new TaskSignalMapper(task), SIGNAL(si_taskFinished(Task *)), SLOT(sl_taskFinished(Task *)));
        return task;
    }

    if (dataFinished()) {
        setDone();
        output->setEnded();
    }

    if (pairedReadsInput) {
        const QString error = checkPairedReads();
        if (!error.isEmpty()) {
            return new FailTask(error);
        }
    }

    return NULL;
}

void DiamondClassifyWorker::cleanup() {

}

bool DiamondClassifyWorker::isReady() const {
    if (isDone()) {
        return false;
    }

    const int hasMessage1 = input->hasMessage();
    const bool ended1 = input->isEnded();
    if (!pairedReadsInput) {
        return hasMessage1 || ended1;
    }

    const int hasMessage2 = pairedInput->hasMessage();
    const bool ended2 = pairedInput->isEnded();

    if (hasMessage1 && hasMessage2) {
        return true;
    } else if (hasMessage1) {
        return ended2;
    } else if (hasMessage2) {
        return ended1;
    }

    return ended1 && ended2;
}

void DiamondClassifyWorker::sl_taskFinished(Task *task) {
    DiamondClassifyTask *diamondTask = qobject_cast<DiamondClassifyTask *>(task);
    if (!diamondTask->isFinished() || diamondTask->hasError() || diamondTask->isCanceled()) {
        return;
    }

    const QString classificationUrl = diamondTask->getClassificationUrl();
//    const QString pairedClassificationUrl = diamondTask->getPairedClassificationUrl();        // FIXME: diamond can't work with paired reads

    QVariantMap data;
    data[TaxonomySupport::TAXONOMY_CLASSIFICATION_SLOT_ID] = QVariant::fromValue<U2::LocalWorkflow::TaxonomyClassificationResult>(parseReport(classificationUrl));
    output->put(Message(output->getBusType(), data));
    context->getMonitor()->addOutputFile(classificationUrl, getActor()->getId());

    // FIXME: diamond can't work with paired reads
//    if (pairedReadsInput) {
//        // TODO: it is incorrect to put the paired classification to the same slot
//        QVariantMap pairedData;
//        pairedData[DiamondClassifyWorkerFactory::OUTPUT_SLOT_ID] = pairedClassificationUrl;
//        output->put(Message(output->getBusType(), pairedData));     // TODO: fix metadata
//        context->getMonitor()->addOutputFile(pairedClassificationUrl, getActor()->getId());
//    }
}

bool DiamondClassifyWorker::isReadyToRun() const {
    return input->hasMessage() && (!pairedReadsInput || pairedInput->hasMessage());
}

bool DiamondClassifyWorker::dataFinished() const {
    return input->isEnded() || (pairedReadsInput && pairedInput->isEnded());
}

QString DiamondClassifyWorker::checkPairedReads() const {
    CHECK(pairedReadsInput, "");
    if (input->isEnded() && (!pairedInput->isEnded() || pairedInput->hasMessage())) {
        return tr("Not enough downstream reads datasets");
    }
    if (pairedInput->isEnded() && (!input->isEnded() || input->hasMessage())) {
        return tr("Not enough upstream reads datasets");
    }
    return "";
}

DiamondClassifyTaskSettings DiamondClassifyWorker::getSettings(U2OpStatus &os) {
    DiamondClassifyTaskSettings settings;
    settings.databaseUrl = getValue<QString>(DiamondClassifyWorkerFactory::DATABASE_ATTR_ID);

    const Message message = getMessageAndSetupScriptValues(input);
    settings.readsUrl = message.getData().toMap()[GetReadsListWorkerFactory::SE_SLOT().getId()].toString();

    QString tmpDir = FileAndDirectoryUtils::createWorkingDir(context->workingDir(), FileAndDirectoryUtils::WORKFLOW_INTERNAL, "", context->workingDir());
    tmpDir = GUrlUtils::createDirectory(tmpDir + DIAMOND_DIR , "_", os);

    settings.classificationUrl = getValue<QString>(DiamondClassifyWorkerFactory::OUTPUT_URL_ATTR_ID);
    if (settings.classificationUrl.isEmpty()) {
        settings.classificationUrl = tmpDir + "/" + getClassificationFileName(message);
    }
    settings.classificationUrl = GUrlUtils::rollFileName(settings.classificationUrl, "_");

    settings.sensitive = getValue<QString>(DiamondClassifyWorkerFactory::SENSITIVE_ATTR_ID);
    settings.matrix = getValue<QString>(DiamondClassifyWorkerFactory::MATRIX_ATTR_ID);
    settings.max_evalue = getValue<double>(DiamondClassifyWorkerFactory::EVALUE_ATTR_ID);
    settings.block_size = getValue<double>(DiamondClassifyWorkerFactory::BSIZE_ATTR_ID);
    settings.gencode = getValue<unsigned>(DiamondClassifyWorkerFactory::GENCODE_ATTR_ID);
    settings.frame_shift = getValue<unsigned>(DiamondClassifyWorkerFactory::FSHIFT_ATTR_ID);
    settings.gap_open = getValue<int>(DiamondClassifyWorkerFactory::GO_PEN_ATTR_ID);
    settings.gap_extend = getValue<int>(DiamondClassifyWorkerFactory::GE_PEN_ATTR_ID);
    settings.index_chunks = getValue<int>(DiamondClassifyWorkerFactory::CHUNKS_ATTR_ID);
    settings.num_threads = getValue<int>(DiamondClassifyWorkerFactory::THREADS_ATTR_ID);


//    if (pairedReadsInput) {
//        settings.pairedReads = true;
//        const Message pairedMessage = getMessageAndSetupScriptValues(pairedInput);
//        settings.pairedReadsUrl = pairedMessage.getData().toMap()[GetReadsListWorkerFactory::PE_SLOT().getId()].toString();
//        settings.pairedClassificationUrl = tmpDir + "/" + getClassificationFileName(pairedMessage);
//        if (settings.classificationUrl == settings.pairedClassificationUrl) {
//            settings.pairedClassificationUrl = GUrlUtils::rollFileName(settings.pairedClassificationUrl, QSet<QString>() << settings.classificationUrl);
//        }
//    }

    U2DataPathRegistry *dataPathRegistry = AppContext::getDataPathRegistry();
    SAFE_POINT_EXT(NULL != dataPathRegistry, os.setError("U2DataPathRegistry is NULL"), settings);

    U2DataPath *taxonomyDataPath = dataPathRegistry->getDataPathByName(NgsReadsClassificationPlugin::TAXONOMY_DATA_ID);
    SAFE_POINT_EXT(NULL != taxonomyDataPath, os.setError("Taxonomy data path is not registered"), settings);
    CHECK_EXT(taxonomyDataPath->isValid(), os.setError(tr("Taxonomy data is missed")), settings);
    settings.taxonMapUrl = taxonomyDataPath->getPathByName(NgsReadsClassificationPlugin::TAXON_PROTEIN_MAP);
    settings.taxonNodesUrl = taxonomyDataPath->getPathByName(NgsReadsClassificationPlugin::TAXON_NODES);

    return settings;
}

QString DiamondClassifyWorker::getClassificationFileName(const Message &message) const {
    const MessageMetadata metadata = context->getMetadataStorage().get(message.getMetadataId());
    return QFileInfo(metadata.getFileUrl()).baseName() + "_DIAMOND_classification.txt";
}

TaxonomyClassificationResult DiamondClassifyWorker::parseReport(const QString &url)
{
    TaxonomyClassificationResult result;
    QFile reportFile(url);
    if (!reportFile.open(QIODevice::ReadOnly)) {
        reportError(tr("Cannot open classification report: %1").arg(url));
    } else {
        QByteArray line;

        while ((line = reportFile.readLine()).size() != 0) {

            QList<QByteArray> row = line.split('\t');
            if (row.size() == 3) {
                QString objID = row[0];
                QByteArray &assStr = row[1];
                algoLog.trace(QString("Found Diamond classification: %1=%2").arg(objID).arg(QString(assStr)));

                bool ok = true;
                TaxID assID = assStr.toUInt(&ok);
                if (ok) {
                    if (result.contains(objID)) {
                        QString msg = tr("Duplicate sequence name '%1' have been detected in the classification output.").arg(objID);
                        monitor()->addInfo(msg, getActorId(), Problem::U2_WARNING);
                        algoLog.info(msg);
                    } else {
                        result[objID] = assID;
                    }
                    continue;
                }
            }
            reportError(tr("Broken Diamond report : %1").arg(url));
            break;
        }
        reportFile.close();
    }
    return result;
}

}   // namespace LocalWorkflow
}   // namespace U2
