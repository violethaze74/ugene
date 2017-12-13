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

#include <U2Core/FailTask.h>
#include <U2Core/FileAndDirectoryUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/MultiTask.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Lang/BaseSlots.h>
#include <U2Lang/WorkflowMonitor.h>

#include "KrakenClassifyTask.h"
#include "KrakenClassifyWorker.h"
#include "KrakenClassifyWorkerFactory.h"

namespace U2 {
namespace LocalWorkflow {

const QString KrakenClassifyWorker::KRAKEN_DIR = "kraken";

KrakenClassifyWorker::KrakenClassifyWorker(Actor *actor)
    : BaseWorker(actor, false),
      input(NULL),
      pairedInput(NULL),
      output(NULL),
      pairedReadsInput(false)
{

}

void KrakenClassifyWorker::init() {
    input = ports.value(KrakenClassifyWorkerFactory::INPUT_PORT_ID);
    pairedInput = ports.value(KrakenClassifyWorkerFactory::INPUT_PAIRED_PORT_ID);
    output = ports.value(KrakenClassifyWorkerFactory::OUTPUT_PORT_ID);

    SAFE_POINT(NULL != input, QString("Port with id '%1' is NULL").arg(KrakenClassifyWorkerFactory::INPUT_PORT_ID), );
    SAFE_POINT(NULL != pairedInput, QString("Port with id '%1' is NULL").arg(KrakenClassifyWorkerFactory::INPUT_PAIRED_PORT_ID), );
    SAFE_POINT(NULL != output, QString("Port with id '%1' is NULL").arg(KrakenClassifyWorkerFactory::OUTPUT_PORT_ID), );

    pairedReadsInput = (getValue<QString>(KrakenClassifyWorkerFactory::INPUT_DATA_ATTR_ID) == KrakenClassifyTaskSettings::PAIRED_END);

    // FIXME: the second port is not taken into account
    output->addComplement(input);
    input->addComplement(output);
}

Task *KrakenClassifyWorker::tick() {
    if (isReadyToRun()) {
        U2OpStatus2Log os;
        KrakenClassifyTaskSettings settings = getSettings(os);
        if (os.hasError()) {
            return new FailTask(os.getError());
        }

        KrakenClassifyTask *task = new KrakenClassifyTask(settings);
        task->addListeners(createLogListeners(2));
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

void KrakenClassifyWorker::cleanup() {

}

bool KrakenClassifyWorker::isReady() const {
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

void KrakenClassifyWorker::sl_taskFinished(Task *task) {
    KrakenClassifyTask *krakenTask = qobject_cast<KrakenClassifyTask *>(task);
    if (!krakenTask->isFinished() || krakenTask->hasError() || krakenTask->isCanceled()) {
        return;
    }

    const QString rawClassificationUrl = krakenTask->getRawClassificationUrl();
    const QString translatedClassificationUrl = krakenTask->getTranslatedClassificationUrl();

    QVariantMap data;
    data[BaseSlots::URL_SLOT().getId()] = rawClassificationUrl;
    output->put(Message(output->getBusType(), data));
    context->getMonitor()->addOutputFile(translatedClassificationUrl, getActor()->getId());
}

bool KrakenClassifyWorker::isReadyToRun() const {
    return input->hasMessage() && (!pairedReadsInput || pairedInput->hasMessage());
}

bool KrakenClassifyWorker::dataFinished() const {
    return input->isEnded() || (pairedReadsInput && pairedInput->isEnded());
}

QString KrakenClassifyWorker::checkPairedReads() const {
    CHECK(pairedReadsInput, "");
    if (input->isEnded() && (!pairedInput->isEnded() || pairedInput->hasMessage())) {
        return tr("Not enough downstream reads datasets");
    }
    if (pairedInput->isEnded() && (!input->isEnded() || input->hasMessage())) {
        return tr("Not enough upstream reads datasets");
    }
    return "";
}

KrakenClassifyTaskSettings KrakenClassifyWorker::getSettings(U2OpStatus &os) {
    KrakenClassifyTaskSettings settings;
    settings.databaseUrl = getValue<QString>(KrakenClassifyWorkerFactory::DATABASE_ATTR_ID);
    settings.quickOperation = getValue<bool>(KrakenClassifyWorkerFactory::QUICK_OPERATION_ATTR_ID);
    settings.minNumberOfHits = getValue<int>(KrakenClassifyWorkerFactory::MIN_HITS_NUMBER_ATTR_ID);
    settings.numberOfThreads = getValue<int>(KrakenClassifyWorkerFactory::THREADS_NUMBER_ATTR_ID);
    settings.preloadDatabase = getValue<bool>(KrakenClassifyWorkerFactory::PRELOAD_DATABASE_ATTR_ID);

    const Message message = getMessageAndSetupScriptValues(input);
    settings.readsUrl = message.getData().toMap()[BaseSlots::URL_SLOT().getId()].toString();

    if (pairedReadsInput) {
        settings.pairedReads = true;
        const Message pairedMessage = getMessageAndSetupScriptValues(pairedInput);
        settings.pairedReadsUrl = pairedMessage.getData().toMap()[BaseSlots::URL_SLOT().getId()].toString();
    }

    QString tmpDir = FileAndDirectoryUtils::createWorkingDir(context->workingDir(), FileAndDirectoryUtils::WORKFLOW_INTERNAL, "", context->workingDir());
    tmpDir = GUrlUtils::createDirectory(tmpDir + KRAKEN_DIR , "_", os);

    settings.rawClassificationUrl = tmpDir + "/raw_classification.txt";
    settings.translatedClassificationUrl = tmpDir + "/translated_classification.txt";

    return settings;
}

}   // namespace LocalWorkflow
}   // namespace U2
