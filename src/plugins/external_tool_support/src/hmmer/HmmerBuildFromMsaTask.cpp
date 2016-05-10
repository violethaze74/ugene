/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
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

#include <QCoreApplication>
#include <QDir>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/UserApplicationsSettings.h>

#include "HmmerBuildFromMsaTask.h"
#include "utils/ExportTasks.h"

namespace U2 {

HmmerBuildFromMsaTask::HmmerBuildFromMsaTask(const HmmerBuildSettings &settings, MAlignment &msa)
    : ExternalToolSupportTask(tr("Build HMMER profile from msa"), TaskFlags_NR_FOSE_COSC | TaskFlag_ReportingIsEnabled | TaskFlag_ReportingIsSupported),
      settings(settings),
      msa(msa)
{
    SAFE_POINT_EXT(!settings.profileUrl.isEmpty(), setError(tr("HMM profile URL is empty")), );
}

const QString & HmmerBuildFromMsaTask::getHmmUrl() const {
    return settings.profileUrl;
}

namespace {

const QString HMMER_TEMP_DIR = "hmmer";

QString getTaskTempDirName(const QString &prefix, Task *task) {
    return prefix + QString::number(task->getTaskId()) + "_" +
            QDate::currentDate().toString("dd.MM.yyyy") + "_" +
            QTime::currentTime().toString("hh.mm.ss.zzz") + "_" +
            QString::number(QCoreApplication::applicationPid());
}

}

void HmmerBuildFromMsaTask::prepare() {
    QString tempDirName = getTaskTempDirName("hmmer_build_", this);
    QString tempDirPath = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath(HMMER_TEMP_DIR) + "/" + tempDirName;
    QString msaUrl = tempDirPath + "/" + GUrlUtils::fixFileName(msa.getName()) + ".sto";

    QDir tempDir(tempDirPath);
    if (tempDir.exists()){
        ExternalToolSupportUtils::removeTmpDir(tempDirPath, stateInfo);
        CHECK_OP(stateInfo, );
    }
    if (!tempDir.mkpath(tempDirPath)){
        setError(tr("Cannot create a directory for temporary files."));
        return;
    }

    saveTask = new SaveAlignmentTask(msa, msaUrl, BaseDocumentFormats::STOCKHOLM);
    saveTask->setSubtaskProgressWeight(5);
    addSubTask(saveTask);
}

QList<Task *> HmmerBuildFromMsaTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> result;
    CHECK_OP(stateInfo, result);
    if (saveTask == subTask) {
        hmmerTask = new HmmerBuildTask(settings, saveTask->getUrl());
        hmmerTask->setSubtaskProgressWeight(95);
        result << hmmerTask;
    }
    return result;
}

QString HmmerBuildFromMsaTask::generateReport() const {
    return HmmerBuildTask::getReport(this, settings, "");
}

}   // namespace U2
