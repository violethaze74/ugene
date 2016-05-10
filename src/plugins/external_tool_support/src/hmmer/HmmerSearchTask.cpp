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
#include <U2Core/AppResources.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Counter.h>
#include <U2Core/CreateAnnotationTask.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include "HmmerParseSearchResultsTask.h"
#include "HmmerSearchTask.h"
#include "HmmerSupport.h"

namespace U2 {

AnnotationCreationPattern::AnnotationCreationPattern()
    : type(U2FeatureTypes::MiscFeature)
{

}

const double HmmerSearchSettings::OPTION_NOT_SET = -1.0;

HmmerSearchSettings::HmmerSearchSettings()
    : e(10.0),
      t(OPTION_NOT_SET),
      z(OPTION_NOT_SET),
      domE(10.0),
      domT(OPTION_NOT_SET),
      domZ(OPTION_NOT_SET),
      useBitCutoffs(None),
      f1(0.02),
      f2(1e-3),
      f3(1e-5),
      doMax(false),
      noBiasFilter(false),
      noNull2(false),
      seed(42),
      annotationTable(NULL)
{

}

bool HmmerSearchSettings::validate() const {
    CHECK(0 < e, false);
    CHECK(0 < t || OPTION_NOT_SET == t, false);
    CHECK(0 < z || OPTION_NOT_SET == z, false);
    CHECK(0 < domE, false);
    CHECK(0 < domT || OPTION_NOT_SET == domT, false);
    CHECK(0 < domZ || OPTION_NOT_SET == domZ, false);
    CHECK(0 <= seed, false);
    CHECK(!hmmProfileUrl.isEmpty(), false);
    CHECK(!sequenceUrl.isEmpty(), false);

    return true;
}

const QString HmmerSearchTask::PER_SEQUENCE_HITS_FILENAME = "per_sequence_hits.txt";
const QString HmmerSearchTask::PER_DOMAIN_HITS_FILENAME = "per_domain_hits.txt";

HmmerSearchTask::HmmerSearchTask(const HmmerSearchSettings &settings)
    : ExternalToolSupportTask(tr("HMMER search"), TaskFlags_NR_FOSE_COSC | TaskFlag_ReportingIsEnabled | TaskFlag_ReportingIsSupported),
      settings(settings),
      hmmerTask(NULL),
      parseTask(NULL)
{
    GCOUNTER(cvar, tvar, "HMMER Search");
    SAFE_POINT_EXT(settings.validate(), setError("Settings are invalid"), );
}

QList<SharedAnnotationData> HmmerSearchTask::getAnnotations() const {
    CHECK(NULL != parseTask, QList<SharedAnnotationData>());
    return parseTask->getAnnotations();
}

void HmmerSearchTask::prepare() {
    prepareWorkingDir();
    hmmerTask = new ExternalToolRunTask(HmmerSupport::SEARCH_TOOL, getArguments(), new ExternalToolLogParser);
    hmmerTask->setSubtaskProgressWeight(90);
    addSubTask(hmmerTask);
}

QList<Task *> HmmerSearchTask::onSubTaskFinished(Task *subTask) {
    QList<Task *> result;
    CHECK_OP(stateInfo, result);

    if (subTask == hmmerTask) {
        parseTask = new HmmerParseSearchResultsTask(settings.workingDir + "/" + PER_DOMAIN_HITS_FILENAME, settings.pattern);
        parseTask->setSubtaskProgressWeight(5);
        result << parseTask;
    } else if (subTask == parseTask) {
        Task *createAnnotationsTask = new CreateAnnotationsTask(settings.annotationTable, parseTask->getAnnotations(), settings.pattern.groupName);
        createAnnotationsTask->setSubtaskProgressWeight(5);
        result << createAnnotationsTask;
    }

    return result;
}

QString HmmerSearchTask::generateReport() const {
    QString res;
    res += "<table>";
    res += "<tr><td><b>" + tr("HMM profile used: ") + "</b></td><td>" + QFileInfo(settings.hmmProfileUrl).absoluteFilePath() + "</td></tr>";

    if (hasError() || isCanceled()) {
        res += "<tr><td><b>" + tr("Task was not finished") + "</b></td><td></td></tr>";
        res += "</table>";
        return res;
    }

    if (NULL != settings.annotationTable && NULL != settings.annotationTable->getDocument()) {
        res += "<tr><td><b>" + tr("Result annotation table: ") + "</b></td><td>" + settings.annotationTable->getDocument()->getName() + "</td></tr>";
    }
    res += "<tr><td><b>" + tr("Result annotation group: ") + "</b></td><td>" + settings.pattern.groupName + "</td></tr>";
    res += "<tr><td><b>" + tr("Result annotation name: ") + "</b></td><td>" + settings.pattern.annotationName + "</td></tr>";

    res += "<tr><td><b>" + tr("Results count: ") + "</b></td><td>" + QString::number(getAnnotations().size()) + "</td></tr>";
    res += "</table>";
    return res;
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

void HmmerSearchTask::prepareWorkingDir() {
    if (settings.workingDir.isEmpty()) {
        QString tempDirName = getTaskTempDirName("hmmer_search_", this);
        settings.workingDir = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath(HMMER_TEMP_DIR) + "/" + tempDirName;
    }

    QDir tempDir(settings.workingDir);
    if (tempDir.exists()) {
        ExternalToolSupportUtils::removeTmpDir(settings.workingDir, stateInfo);
        CHECK_OP(stateInfo, );
    }

    if (!tempDir.mkpath(settings.workingDir)) {
        setError(tr("Cannot create a directory for temporary files."));
        return;
    }
}

QStringList HmmerSearchTask::getArguments() const {
    QStringList arguments;

    arguments << "-E" << QString::number(settings.e);
    if (HmmerSearchSettings::OPTION_NOT_SET != settings.t) {
        arguments << "-T" << QString::number(settings.t);
    }

    if (HmmerSearchSettings::OPTION_NOT_SET != settings.z) {
        arguments << "-Z" << QString::number(settings.z);
    }

    arguments << "--domE" << QString::number(settings.domE);
    if (HmmerSearchSettings::OPTION_NOT_SET != settings.domT) {
        arguments << "--domT" << QString::number(settings.domT);
    }

    if (HmmerSearchSettings::OPTION_NOT_SET != settings.domZ) {
        arguments << "--domZ" << QString::number(settings.domZ);
    }

    switch (settings.useBitCutoffs) {
    case HmmerSearchSettings::None:
        break;
    case HmmerSearchSettings::p7H_GA:
        arguments << "--cut_ga";
        break;
    case HmmerSearchSettings::p7H_TC:
        arguments << "--cut_nc";
        break;
    case HmmerSearchSettings::p7H_NC:
        arguments << "--cut_tc";
        break;
    default:
        FAIL(tr("Unknown option controlling model-specific thresholding"), arguments);
    }

    arguments << "--F1" << QString::number(settings.f1);
    arguments << "--F2" << QString::number(settings.f2);
    arguments << "--F3" << QString::number(settings.f3);

    if (settings.doMax) {
        arguments << "--max";
    }

    if (settings.noBiasFilter) {
        arguments << "--nobias";
    }

    if (settings.noNull2) {
        arguments << "--nonull2";
    }

    arguments << "--seed" << QString::number(settings.seed);
    arguments << "--cpu" << QString::number(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());

    arguments << "--noali";
    arguments << "--tblout" << settings.workingDir + "/" + PER_SEQUENCE_HITS_FILENAME;
    arguments << "--domtblout" << settings.workingDir + "/" + PER_DOMAIN_HITS_FILENAME;

    arguments << settings.hmmProfileUrl;
    arguments << settings.sequenceUrl;

    return arguments;
}

}   // namespace U2
