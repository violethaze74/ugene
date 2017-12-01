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
#include <U2Core/Counter.h>
#include <U2Core/U2SafePoints.h>

#include "KrakenSupport.h"
#include "KrakenClassifyLogParser.h"
#include "KrakenClassifyTask.h"
#include "KrakenTranslateLogParser.h"

namespace U2 {

KrakenClassifyTask::KrakenClassifyTask(const KrakenClassifyTaskSettings &settings)
    : ExternalToolSupportTask(tr("Classify reads with Kraken"), TaskFlags_NR_FOSE_COSC),
      settings(settings),
      classifyTask(NULL)
{
    GCOUNTER(cvar, tvar, "KrakenClassifyTask");

    SAFE_POINT_EXT(!settings.readsUrl.isEmpty(), setError("Reads URL is empty"), );
    SAFE_POINT_EXT(!settings.pairedReads || !settings.readsUrl.isEmpty(), setError("Paired reads URL is empty, but the 'paired reads' option is set"), );
    SAFE_POINT_EXT(!settings.databaseUrl.isEmpty(), setError("Kraken database URL is empty"), );
    SAFE_POINT_EXT(!settings.rawClassificationUrl.isEmpty(), setError("Kraken classification URL is empty"), );
    SAFE_POINT_EXT(!settings.translatedClassificationUrl.isEmpty(), setError("URL to write translated Kraken classification is empty"), );
}

const QString &KrakenClassifyTask::getRawClassificationUrl() const {
    return settings.rawClassificationUrl;
}

const QString &KrakenClassifyTask::getTranslatedClassificationUrl() const {
    return settings.translatedClassificationUrl;
}

void KrakenClassifyTask::prepare() {
    classifyTask = new ExternalToolRunTask(KrakenSupport::CLASSIFY_TOOL, getClassifyArguments(), new KrakenClassifyLogParser());
    setListenerForTask(classifyTask);
    addSubTask(classifyTask);
}

QList<Task *> KrakenClassifyTask::onSubTaskFinished(Task *subTask) {
    QList<Task *> newSubTasks;
    CHECK_OP(stateInfo, newSubTasks);

    if (classifyTask == subTask) {
        ExternalToolRunTask *translateTask = new ExternalToolRunTask(KrakenSupport::TRANSLATE_TOOL, getTranslateArguments(), new KrakenTranslateLogParser());
        translateTask->setStandartOutputFile(settings.translatedClassificationUrl);
        setListenerForTask(translateTask, 1);
        newSubTasks << translateTask;
    }

    return newSubTasks;
}

QStringList KrakenClassifyTask::getClassifyArguments() {
    QStringList arguments;
    arguments << "--db" << settings.databaseUrl;
    arguments << "--threads" << QString::number(settings.numberOfThreads);

    if (settings.quickOperation) {
        arguments << "--quick";
        arguments << "--min-hits" << QString::number(settings.minNumberOfHits);
    }

    arguments << "--output" << settings.rawClassificationUrl;
    if (settings.preloadDatabase) {
        arguments << "--preload";
    }

    if (settings.pairedReads) {
        arguments << "--paired";
        arguments << "--check-names";
    }

    arguments << settings.readsUrl;
    if (settings.pairedReads) {
        arguments << settings.pairedReadsUrl;
    }

    return arguments;
}

QStringList KrakenClassifyTask::getTranslateArguments() {
    QStringList arguments;
    arguments << "--db" << settings.databaseUrl;
    arguments << settings.rawClassificationUrl;
    return arguments;
}

}   // namespace U2
