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

#include <QDir>
#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/CmdlineTaskRunner.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/U2SafePoints.h>

#include "CmdlineInOutTaskRunner.h"

namespace U2 {

CmdlineInOutTaskConfig::CmdlineInOutTaskConfig()
: emptyOutputPossible(true), withPluginList(false)
{
}

CmdlineInOutTaskRunner::CmdlineInOutTaskRunner(const CmdlineInOutTaskConfig &config)
: DocumentProviderTask(tr("Run input/output UGENE command line: %1").arg(config.command), TaskFlags_NR_FOSE_COSC),
config(config), inputDoc(NULL), saveTask(NULL), cmdlineTask(NULL), loadTask(NULL)
{
    inputDoc = new Document(BaseDocumentFormats::get(config.inputFormat), IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE),
        GUrl("unused"), U2DbiRef(), config.inputObjects, config.inputDocHints);
    inputDoc->setParent(this);
}

void CmdlineInOutTaskRunner::prepare() {
    prepareTmpFile(config.inputFormat, inputTmpFile);
    CHECK_OP(stateInfo, );
    prepareTmpFile(config.outputFormat, outputTmpFile);
    CHECK_OP(stateInfo, );

    saveTask = new SaveDocumentTask(inputDoc, IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), inputTmpFile.fileName());
    saveTask->setSubtaskProgressWeight(0.1f);
    addSubTask(saveTask);
}

QList<Task*> CmdlineInOutTaskRunner::onSubTaskFinished(Task *subTask) {
    QList<Task*> result;

    if (subTask == saveTask) {
        config.arguments << "--in=" + inputTmpFile.fileName();
        config.arguments << "--out=" + outputTmpFile.fileName();

        CmdlineTaskConfig cmdlineConfig;
        cmdlineConfig.command = config.command;
        cmdlineConfig.arguments = config.arguments;
        cmdlineConfig.withPluginList = config.withPluginList;
        cmdlineConfig.pluginList = config.pluginList;
        cmdlineTask = new CmdlineTaskRunner(cmdlineConfig);
        cmdlineTask->setSubtaskProgressWeight(0.85f);
        result << cmdlineTask;
    } else if (subTask == cmdlineTask) {
        if (0 == QFileInfo(outputTmpFile.fileName()).size()) {
            if (!config.emptyOutputPossible) {
                setError(tr("An error occurred during the task. See the log for details."));
            }
            return result;
        }
        IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        ioLog.details(tr("Loading result file '%1'").arg(outputTmpFile.fileName()));
        loadTask = new LoadDocumentTask(config.outputFormat, outputTmpFile.fileName(), iof, config.outputDocHints);
        loadTask->setSubtaskProgressWeight(0.05f);
        result << loadTask;
    } else if (subTask == loadTask) {
        resultDocument = loadTask->takeDocument();
    }

    return result;
}

void CmdlineInOutTaskRunner::prepareTmpFile(const QString &format, QTemporaryFile &tmpFile) {
    tmpFile.setFileTemplate(QString("%1/XXXXXX.%2").arg(QDir::tempPath()).arg(format));
    if (!tmpFile.open()) {
        setError(tr("Cannot create temporary file for writing."));
        return;
    }
#ifdef _DEBUG
    tmpFile.setAutoRemove(false);
#endif
    tmpFile.close();
}

} // U2
