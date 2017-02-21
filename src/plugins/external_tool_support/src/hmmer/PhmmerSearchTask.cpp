/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QCoreApplication>
#include <QDir>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/Counter.h>
#include <U2Core/CreateAnnotationTask.h>
#include <U2Core/L10n.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include "HmmerParseSearchResultsTask.h"
#include "HmmerSupport.h"
#include "PhmmerSearchTask.h"
#include "utils/ExportTasks.h"

#include "HmmerBuildTask.h"

namespace U2 {

const QString PhmmerSearchTask::INPUT_SEQUENCE_FILENAME = "input_sequence.fa";
const QString PhmmerSearchTask::PER_DOMAIN_HITS_FILENAME = "per_domain_hits.txt";

PhmmerSearchTask::PhmmerSearchTask(const PhmmerSearchSettings &settings)
    : ExternalToolSupportTask(tr("Search with phmmer"), TaskFlags_NR_FOSE_COSC | TaskFlag_ReportingIsEnabled | TaskFlag_ReportingIsSupported),
      settings(settings),
      phmmerTask(NULL),
      parseTask(NULL),
      removeWorkingDir(false)
{
    GCOUNTER(cvar, tvar, "HMMER Search");
    SAFE_POINT_EXT(settings.validate(), setError("Settings are invalid"), );
}

QList<SharedAnnotationData> PhmmerSearchTask::getAnnotations() const {
    CHECK(NULL != parseTask, QList<SharedAnnotationData>());
    return parseTask->getAnnotations();
}

void PhmmerSearchTask::prepare() {
    prepareWorkingDir();

    if (settings.targetSequenceUrl.isEmpty()) {
        SAFE_POINT_EXT(NULL != settings.targetSequence, setError(L10N::nullPointerError("sequence object")), );
        prepareSequenceSaveTask();
        addSubTask(saveSequenceTask);
    } else {
        preparePhmmerTask();
        addSubTask(phmmerTask);
    }
}

QList<Task *> PhmmerSearchTask::onSubTaskFinished(Task *subTask) {
    QList<Task *> result;
    CHECK_OP(stateInfo, result);

    if (subTask == saveSequenceTask) {
        preparePhmmerTask();
        result << phmmerTask;
    } else if (subTask == phmmerTask) {
        parseTask = new HmmerParseSearchResultsTask(settings.workingDir + "/" + PER_DOMAIN_HITS_FILENAME, settings.pattern);
        parseTask->setSubtaskProgressWeight(5);
        result << parseTask;
    } else if (subTask == parseTask) {
        removeTempDir();
        if (settings.annotationTable != NULL) {
            Task *createAnnotationsTask = new CreateAnnotationsTask(settings.annotationTable, parseTask->getAnnotations(), settings.pattern.groupName);
            createAnnotationsTask->setSubtaskProgressWeight(5);
            result << createAnnotationsTask;
        }
    }

    return result;
}

QString PhmmerSearchTask::generateReport() const {
    QString res;
    res += "<table>";
    res += "<tr><td><b>" + tr("Query sequence: ") + "</b></td><td>" + QFileInfo(settings.querySequenceUrl).absoluteFilePath() + "</td></tr>";

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

const QString PHMMER_TEMP_DIR = "phmmer";

QString getTaskTempDirName(const QString &prefix, Task *task) {
    return prefix + QString::number(task->getTaskId()) + "_" +
            QDate::currentDate().toString("dd.MM.yyyy") + "_" +
            QTime::currentTime().toString("hh.mm.ss.zzz") + "_" +
            QString::number(QCoreApplication::applicationPid());
}

}

void PhmmerSearchTask::prepareWorkingDir() {
    if (settings.workingDir.isEmpty()) {
        QString tempDirName = getTaskTempDirName("phmmer_search_", this);
        settings.workingDir = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath(PHMMER_TEMP_DIR) + "/" + tempDirName;
        removeWorkingDir = true;
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

void PhmmerSearchTask::removeTempDir() const {
    CHECK(removeWorkingDir, );
    U2OpStatusImpl os;
    ExternalToolSupportUtils::removeTmpDir(settings.workingDir, os);
}

QStringList PhmmerSearchTask::getArguments() const {
    QStringList arguments;

    if (PhmmerSearchSettings::OPTION_NOT_SET != settings.t) {
        arguments << "-T" << QString::number(settings.t);
    } else {
        arguments << "-E" << QString::number(settings.e);
    }

    if (PhmmerSearchSettings::OPTION_NOT_SET != settings.z) {
        arguments << "-Z" << QString::number(settings.z);
    }

    if (PhmmerSearchSettings::OPTION_NOT_SET != settings.domT) {
        arguments << "--domT" << QString::number(settings.domT);
    } else if (PhmmerSearchSettings::OPTION_NOT_SET != settings.domE) {
        arguments << "--domE" << QString::number(settings.domE);
    }    

    if (PhmmerSearchSettings::OPTION_NOT_SET != settings.domZ) {
        arguments << "--domZ" << QString::number(settings.domZ);
    }

    if (settings.doMax) {
        arguments << "--max";
    } else {
        arguments << "--F1" << QString::number(settings.f1);
        arguments << "--F2" << QString::number(settings.f2);
        arguments << "--F3" << QString::number(settings.f3);
    }

    if (settings.noBiasFilter) {
        arguments << "--nobias";
    }

    if (settings.noNull2) {
        arguments << "--nonull2";
    }

    arguments << "--EmL" << QString::number(settings.eml);
    arguments << "--EmN" << QString::number(settings.emn);
    arguments << "--EvL" << QString::number(settings.evl);
    arguments << "--EvN" << QString::number(settings.evn);
    arguments << "--EfL" << QString::number(settings.efl);
    arguments << "--EfN" << QString::number(settings.efn);
    arguments << "--Eft" << QString::number(settings.eft);

    arguments << "--popen" << QString::number(settings.popen);
    arguments << "--pextend" << QString::number(settings.pextend);

    arguments << "--seed" << QString::number(settings.seed);
    arguments << "--cpu" << QString::number(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());

    arguments << "--noali";
    arguments << "--domtblout" << settings.workingDir + "/" + PER_DOMAIN_HITS_FILENAME;

    arguments << settings.querySequenceUrl;
    arguments << settings.targetSequenceUrl;

    return arguments;
}

void PhmmerSearchTask::prepareSequenceSaveTask() {
    settings.targetSequenceUrl = settings.workingDir + "/" + INPUT_SEQUENCE_FILENAME;
    saveSequenceTask = new SaveSequenceTask(settings.targetSequence, settings.targetSequenceUrl, BaseDocumentFormats::FASTA);
    saveSequenceTask->setSubtaskProgressWeight(5);
}

void PhmmerSearchTask::preparePhmmerTask() {
    phmmerTask = new ExternalToolRunTask(HmmerSupport::PHMMER_TOOL, getArguments(), new Hmmer3LogParser());
    setListenerForTask(phmmerTask);
    phmmerTask->setSubtaskProgressWeight(85);
}

/****************************************
* GTest_UHMM3Phmmer
****************************************/

const QString GTest_UHMM3Phmmer::QUERY_FILENAME_TAG = "query";
const QString GTest_UHMM3Phmmer::DB_FILENAME_TAG = "db";

const QString GTest_UHMM3Phmmer::GAP_OPEN_PROBAB_OPTION_TAG = "popen";
const QString GTest_UHMM3Phmmer::GAP_EXTEND_PROBAB_OPTION_TAG = "pextend";
const QString GTest_UHMM3Phmmer::SUBST_MATR_NAME_OPTION_TAG = "substMatr";

const QString GTest_UHMM3Phmmer::OUTPUT_DIR_TAG = "outputDir";

const double BAD_DOUBLE_OPTION = -1.0;

static void setDoubleOption(double & to, const QString & str, TaskStateInfo & ti) {
    if (str.isEmpty()) {
        return;
    }
    bool ok = false;
    to = str.toDouble(&ok);
    if (!ok) {
        to = BAD_DOUBLE_OPTION;
        ti.setError(QString("cannot_parse_double_from: %1").arg(str));
    }
}

void GTest_UHMM3Phmmer::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    phmmerTask = NULL;
    queryFilename = el.attribute(QUERY_FILENAME_TAG);
    dbFilename = el.attribute(DB_FILENAME_TAG);

    setSearchTaskSettings(searchSettings, el, stateInfo);
    searchSettings.annotationTable = NULL;

    setDoubleOption(searchSettings.popen, el.attribute(GAP_OPEN_PROBAB_OPTION_TAG), stateInfo);
    
    setDoubleOption(searchSettings.pextend, el.attribute(GAP_EXTEND_PROBAB_OPTION_TAG), stateInfo);

    outputDir = el.attribute(OUTPUT_DIR_TAG);

    if (queryFilename.isEmpty()) {
        stateInfo.setError(L10N::badArgument("query sequence filename"));
        return;
    }
    queryFilename = env->getVar("COMMON_DATA_DIR") + "/" + queryFilename;

    searchSettings.querySequenceUrl = queryFilename;
}

void GTest_UHMM3Phmmer::setAndCheckArgs() {
    assert(!stateInfo.hasError());

    if (dbFilename.isEmpty()) {
        stateInfo.setError(L10N::badArgument("db sequence filename"));
        return;
    }
    dbFilename = env->getVar("COMMON_DATA_DIR") + "/" + dbFilename;

    if (outputDir.isEmpty()) {
        stateInfo.setError("output_dir_is_empty");
        return;
    }

    outputDir = env->getVar("TEMP_DATA_DIR") + "/" + outputDir;
}

static void setDoubleOption(double& num, const QDomElement& el, const QString& optionName, TaskStateInfo& si) {
    if (si.hasError()) {
        return;
    }
    QString numStr = el.attribute(optionName);
    if (numStr.isEmpty()) {
    return;
    
    }
    bool ok = false;
    double ret = numStr.toDouble(&ok);
    if (!ok) {
    si.setError(QString("cannot_parse_double_number_from %1. Option: %2").arg(numStr).arg(optionName));
    return;
    }
    num = ret;
}

static void setUseBitCutoffsOption(int& ret, const QDomElement& el, const QString& opName, TaskStateInfo& si) {
    if (si.hasError()) {
        return;
    }
    QString str = el.attribute(opName).toLower();
    if ("ga" == str) {
        ret = HmmerSearchSettings::p7H_GA;
    } else if ("nc" == str) {
        ret = HmmerSearchSettings::p7H_NC;
    } else if ("tc" == str) {
        ret = HmmerSearchSettings::p7H_TC;
    } else if (!str.isEmpty()) {
        si.setError(QString("unrecognized_value_in %1 option").arg(opName));
    }
}

static void setBooleanOption(bool&ret, const QDomElement& el, const QString& opName, TaskStateInfo& si) {
    if (si.hasError()) {
        return;
    }
    QString str = el.attribute(opName).toLower();
    if (!str.isEmpty() && "n" != str && "no" != str) {
        ret = true;    
    } else {
        ret = false;
    }
}

static void setIntegerOption(int& num, const QDomElement& el, const QString& optionName, TaskStateInfo& si) {
    if (si.hasError()) {
        return;
    }
    QString numStr = el.attribute(optionName);
    if (numStr.isEmpty()) {
        return;
    }

    bool ok = false;
    int ret = numStr.toInt(&ok);
    if (!ok) {
        si.setError(QString("cannot_parse_integer_number_from %1. Option: %2").arg(numStr).arg(optionName));
        return;
    }
    num = ret;
}

void GTest_UHMM3Phmmer::setSearchTaskSettings(PhmmerSearchSettings& settings, const QDomElement& el, TaskStateInfo& si) {
    setDoubleOption(settings.e, el, GTest_UHMM3Search::SEQ_E_OPTION_TAG, si);
    setDoubleOption(settings.t, el, GTest_UHMM3Search::SEQ_T_OPTION_TAG, si);
    setDoubleOption(settings.z, el, GTest_UHMM3Search::Z_OPTION_TAG, si);
    setDoubleOption(settings.f1, el, GTest_UHMM3Search::F1_OPTION_TAG, si);
    setDoubleOption(settings.f2, el, GTest_UHMM3Search::F2_OPTION_TAG, si);
    setDoubleOption(settings.f3, el, GTest_UHMM3Search::F3_OPTION_TAG, si);
    setDoubleOption(settings.domE, el, GTest_UHMM3Search::DOM_E_OPTION_TAG, si);
    setDoubleOption(settings.domT, el, GTest_UHMM3Search::DOM_T_OPTION_TAG, si);
    setDoubleOption(settings.domZ, el, GTest_UHMM3Search::DOM_Z_OPTION_TAG, si);

    setBooleanOption(settings.doMax, el, GTest_UHMM3Search::MAX_OPTION_TAG, si);
    setBooleanOption(settings.noBiasFilter, el, GTest_UHMM3Search::NOBIAS_OPTION_TAG, si);
    setBooleanOption(settings.noNull2, el, GTest_UHMM3Search::NONULL2_OPTION_TAG, si);

    setIntegerOption(settings.seed, el, GTest_UHMM3Search::SEED_OPTION_TAG, si);
}

void GTest_UHMM3Phmmer::prepare() {
    assert(!hasError() && NULL == phmmerTask);
    setAndCheckArgs();
    if (hasError()) {
        return;
    }
    searchSettings.workingDir = outputDir;
    searchSettings.targetSequenceUrl = dbFilename;
    searchSettings.querySequenceUrl = queryFilename;
    phmmerTask = new PhmmerSearchTask(searchSettings);
    phmmerTask->addListeners(QList<ExternalToolListener*>() << new OutputCollector());
    addSubTask(phmmerTask);
}

QList<Task*> GTest_UHMM3Phmmer::onSubTaskFinished(Task* subTask) {
    QList< Task* > res;
    if (subTask == phmmerTask) {
        OutputCollector *collector = dynamic_cast<OutputCollector*>(phmmerTask->getListener(0));
        if (collector != NULL) {
            QString hmmSearchLog = collector->getLog();
            //TODO: check non empty log and file existence after writing
            QFile file(searchSettings.workingDir + "/output.txt");
            file.open(QIODevice::WriteOnly);
            file.write(hmmSearchLog.toLatin1());
            file.close();
            delete collector;
        }
    }    
    return res;
}

Task::ReportResult GTest_UHMM3Phmmer::report() {
    return ReportResult_Finished;
}

/****************************************
* GTest_UHMM3PhmmerCompare
****************************************/
const QString GTest_UHMM3PhmmerCompare::ACTUAL_OUT_FILE_TAG = "actualOut";
const QString GTest_UHMM3PhmmerCompare::TRUE_OUT_FILE_TAG = "trueOut";

void GTest_UHMM3PhmmerCompare::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    trueOutFilename = el.attribute(TRUE_OUT_FILE_TAG);
    actualOutFilename = el.attribute(ACTUAL_OUT_FILE_TAG);
}

void GTest_UHMM3PhmmerCompare::setAndCheckArgs() {
    if (trueOutFilename.isEmpty()) {
        stateInfo.setError(L10N::badArgument("true out filename"));
        return;
    }
    trueOutFilename = env->getVar("COMMON_DATA_DIR") + "/" + trueOutFilename;

    if (actualOutFilename.isEmpty()) {
        stateInfo.setError("actual_out_filename_is_empty");
        return;
    }
    actualOutFilename = env->getVar("TEMP_DATA_DIR") + "/" + actualOutFilename;
}

Task::ReportResult GTest_UHMM3PhmmerCompare::report() {
    assert(!hasError());
    setAndCheckArgs();
    if (hasError()) {
        return ReportResult_Finished;
    }

    UHMM3SearchResult trueRes;
    UHMM3SearchResult actualRes;
    try {
        trueRes = GTest_UHMM3SearchCompare::getSearchResultFromOutput(trueOutFilename);
        actualRes = GTest_UHMM3SearchCompare::getSearchResultFromOutput(actualOutFilename);
    } catch (const QString& ex) {
        stateInfo.setError(ex);
    } catch (...) {
        stateInfo.setError("undefined_error_occurred");
    }

    if (hasError()) {
        return ReportResult_Finished;
    }

    GTest_UHMM3SearchCompare::generalCompareResults(actualRes, trueRes, stateInfo);

    return ReportResult_Finished;
}

}   // namespace U2
