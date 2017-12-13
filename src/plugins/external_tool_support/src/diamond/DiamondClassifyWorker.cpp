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
    data[DiamondClassifyWorkerFactory::OUTPUT_SLOT_ID] = classificationUrl;
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
    settings.readsUrl = message.getData().toMap()[BaseSlots::URL_SLOT().getId()].toString();

    QString tmpDir = FileAndDirectoryUtils::createWorkingDir(context->workingDir(), FileAndDirectoryUtils::WORKFLOW_INTERNAL, "", context->workingDir());
    tmpDir = GUrlUtils::createDirectory(tmpDir + DIAMOND_DIR , "_", os);

    settings.classificationUrl = tmpDir + "/" + getClassificationFileName(message);

    if (pairedReadsInput) {
        settings.pairedReads = true;
        const Message pairedMessage = getMessageAndSetupScriptValues(pairedInput);
        settings.pairedReadsUrl = pairedMessage.getData().toMap()[BaseSlots::URL_SLOT().getId()].toString();
        settings.pairedClassificationUrl = tmpDir + "/" + getClassificationFileName(pairedMessage);
        if (settings.classificationUrl == settings.pairedClassificationUrl) {
            settings.pairedClassificationUrl = GUrlUtils::rollFileName(settings.pairedClassificationUrl, QSet<QString>() << settings.classificationUrl);
        }
    }

    U2DataPathRegistry *dataPathRegistry = AppContext::getDataPathRegistry();
    SAFE_POINT_EXT(NULL != dataPathRegistry, os.setError("U2DataPathRegistry is NULL"), settings);

    U2DataPath *taxonomyDataPath = dataPathRegistry->getDataPathByName(DiamondSupport::TAXONOMY_DATA);
    SAFE_POINT_EXT(NULL != taxonomyDataPath, os.setError("Taxonomy data path is not registered"), settings);
    CHECK_EXT(taxonomyDataPath->isValid(), os.setError(tr("Taxonomy data is missed")), settings);
    settings.taxonMapUrl = taxonomyDataPath->getPathByName(DiamondSupport::TAXON_PROTEIN_MAP);
    settings.taxonNodesUrl = taxonomyDataPath->getPathByName(DiamondSupport::TAXON_NODES);

    return settings;
}

QString DiamondClassifyWorker::getClassificationFileName(const Message &message) const {
    const MessageMetadata metadata = context->getMetadataStorage().get(message.getMetadataId());
    return QFileInfo(metadata.getFileUrl()).baseName() + "_classification.txt";
}

}   // namespace LocalWorkflow
}   // namespace U2
