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

#include "FormatDBSupportTask.h"
#include "FormatDBSupport.h"

#include <QCoreApplication>
#include <QDir>

#include <U2Core/AddDocumentTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Counter.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Formats/ConvertFileTask.h>


namespace U2 {

void FormatDBSupportTaskSettings::reset() {
    inputFilesPath=QList<QString>();
    outputPath="";
    databaseTitle="";
    isInputAmino=true;
}

FormatDBSupportTask::FormatDBSupportTask(const QString& name, const FormatDBSupportTaskSettings& _settings) :
        Task(tr("Run NCBI FormatDB task"), TaskFlags_NR_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled), toolName(name),
        settings(_settings),
        convertSubTaskCounter(0)
{
    GCOUNTER( cvar, tvar, "FormatDBSupportTask" );
    formatDBTask = NULL;
}

void FormatDBSupportTask::prepare(){
    prepareInputFastaFiles();
    if (convertSubTaskCounter == 0) {
        createFormatDbTask();
        addSubTask(formatDBTask);
    }
}

QList<Task*> FormatDBSupportTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> result;
    CHECK(subTask != NULL, result);
    CHECK(!subTask->isCanceled() && !subTask->hasError(), result);

    DefaultConvertFileTask* convertTask = qobject_cast<DefaultConvertFileTask*>(subTask);
    if (convertTask != NULL) {
        convertSubTaskCounter--;
        inputFastaFiles << convertTask->getResult();
        fastaTmpFiles << convertTask->getResult();

        if (convertSubTaskCounter == 0) {
            createFormatDbTask();
            result << formatDBTask;
        }
    }

    return result;
}

Task::ReportResult FormatDBSupportTask::report(){
    // remove tmp files
    if (!fastaTmpFiles.isEmpty()) {
        QDir dir(QFileInfo(fastaTmpFiles.first()).absoluteDir());
        if (!dir.removeRecursively()) {
            stateInfo.addWarning(tr("Can not remove folder for temporary files."));
            emit si_stateChanged();
        }
    }
    return ReportResult_Finished;
}

QString FormatDBSupportTask::generateReport() const {
    QString res;
    if (isCanceled()) {
        res += QString(tr("Blast database creation has been cancelled")) + "<br>";
        if (QFile::exists(externalToolLog)) {
            res += prepareLink(externalToolLog);
        }
        return res;
    }
    if (!hasError()) {
        res += QString(tr("Blast database has been successfully created") + "<br><br>");
        res += QString(tr("Source sequences: "));
        foreach(const QString &filePath, settings.inputFilesPath){
            res += prepareLink(filePath);
            if(filePath.size() > 1){
                res += "<br>    ";
            }
        }
        res += "<br>";
        res += QString(tr("Database file path: %1")).arg(QDir::toNativeSeparators(settings.outputPath)) + "<br>";
        QString type = settings.isInputAmino ? "protein" : "nucleotide";
        res += QString(tr("Type: %1")).arg(type) + "<br>";
        if (QFile::exists(externalToolLog)) {
            res += QString(tr("Formatdb log file path: "));
            res += prepareLink(externalToolLog);
        }
    }else{
        res += QString(tr("Blast database creation has been failed")) + "<br><br>";
        if (QFile::exists(externalToolLog)) {
            res += QString(tr("Formatdb log file path: "));
            res += prepareLink(externalToolLog);
        }
    }
    return res;
}

void FormatDBSupportTask::prepareInputFastaFiles() {
    QString tmpDirName = "FormatDB_"+QString::number(this->getTaskId())+"_"+
                         QDate::currentDate().toString("dd.MM.yyyy")+"_"+
                         QTime::currentTime().toString("hh.mm.ss.zzz")+"_"+
                         QString::number(QCoreApplication::applicationPid())+"/";
    QString tmpDir = GUrlUtils::prepareDirLocation(AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath(FORMATDB_TMP_DIR) + "/"+ tmpDirName,
                                                   stateInfo);
    CHECK_OP(stateInfo, );
    CHECK_EXT(!tmpDir.isEmpty(), setError(tr("Cannot create temp folder")), );

    for(int i = 0; i < settings.inputFilesPath.length(); i++){
        GUrl url(settings.inputFilesPath[i]);

        QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(url);
        if (formats.isEmpty()) {
            stateInfo.addWarning(tr("File '%1' was skipped. Cannot detect file format.").arg(url.getURLString()));
            continue;
        }
        QString firstFormat = formats.first().format->getFormatId();
        if (firstFormat != BaseDocumentFormats::FASTA) {
            DefaultConvertFileTask* convertTask = new DefaultConvertFileTask(url, firstFormat, BaseDocumentFormats::FASTA, tmpDir);
            addSubTask(convertTask);
            convertSubTaskCounter++;
        } else {
            inputFastaFiles << url.getURLString();
        }
    }
}

QString FormatDBSupportTask::prepareLink( const QString &path ) const {
    QString preparedPath = path;
    if(preparedPath.startsWith("'") || preparedPath.startsWith("\"")) {
        preparedPath.remove(0,1);
    }
    if (preparedPath.endsWith("'") || preparedPath.endsWith("\"")) {
        preparedPath.chop(1);
    }
    return "<a href=\"file:///" + QDir::toNativeSeparators(preparedPath) + "\">" +
        QDir::toNativeSeparators(preparedPath) + "</a><br>";
}

void FormatDBSupportTask::createFormatDbTask() {
    SAFE_POINT_EXT(formatDBTask == NULL, setError(tr("Trying to initialize Format DB task second time")), );

    QStringList arguments;
    assert((toolName == ET_FORMATDB)||(toolName == ET_MAKEBLASTDB));
    if(toolName == ET_FORMATDB){
        for (int i = 0; i < inputFastaFiles.length(); i++){
            if (inputFastaFiles[i].contains(" ")) {
                stateInfo.setError(tr("Input files paths contain space characters."));
                return;
            }
        }
        arguments <<"-i"<< inputFastaFiles.join(" ");
        arguments <<"-l"<< settings.outputPath + "formatDB.log";
        arguments <<"-n"<< settings.outputPath;
        arguments <<"-p"<< (settings.isInputAmino ? "T" : "F");
        externalToolLog = settings.outputPath + "formatDB.log";
    }else if (toolName == ET_MAKEBLASTDB){
        for (int i = 0; i < inputFastaFiles.length(); i++){
            inputFastaFiles[i]="\""+inputFastaFiles[i]+"\"";
        }
        arguments <<"-in"<< inputFastaFiles.join(" ");
        arguments <<"-logfile"<< settings.outputPath + "MakeBLASTDB.log";
        externalToolLog = settings.outputPath + "MakeBLASTDB.log";
        if(settings.outputPath.contains(" ")){
            stateInfo.setError(tr("Output database path contain space characters."));
            return;
        }
        arguments <<"-out"<< settings.outputPath;
        arguments <<"-dbtype"<< (settings.isInputAmino ? "prot" : "nucl");
    }

    formatDBTask = new ExternalToolRunTask(toolName, arguments, new ExternalToolLogParser());
    formatDBTask->setSubtaskProgressWeight(95);
}

}//namespace
