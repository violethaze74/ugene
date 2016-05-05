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

#ifndef _U2_CMDLINE_IO_OUT_TASK_RUNNER_H_
#define _U2_CMDLINE_IO_OUT_TASK_RUNNER_H_

#include <QTemporaryFile>
#include <U2Core/DocumentProviderTask.h>

namespace U2 {

class CmdlineTaskRunner;
class GObject;
class LoadDocumentTask;
class SaveDocumentTask;

class U2CORE_EXPORT CmdlineInOutTaskConfig {
public:
    CmdlineInOutTaskConfig();

    QList<GObject*>     inputObjects;
    DocumentFormatId    inputFormat;
    QVariantMap         inputDocHints;

    DocumentFormatId    outputFormat;
    QVariantMap         outputDocHints;
    bool                emptyOutputPossible;

    QString             command;
    QStringList         arguments;
    bool                withPluginList;
    QStringList         pluginList;
};

class U2CORE_EXPORT CmdlineInOutTaskRunner : public DocumentProviderTask {
    Q_OBJECT
public:
    CmdlineInOutTaskRunner(const CmdlineInOutTaskConfig &config);
    void prepare();
    QList<Task*> onSubTaskFinished(Task *subTask);

private:
    void prepareTmpFile(const QString &format, QTemporaryFile &tmpFile);

    CmdlineInOutTaskConfig  config;
    Document*               inputDoc;
    QTemporaryFile          inputTmpFile;
    QTemporaryFile          outputTmpFile;
    SaveDocumentTask*       saveTask;
    CmdlineTaskRunner*      cmdlineTask;
    LoadDocumentTask*       loadTask;
};

} // U2

#endif // _U2_CMDLINE_IO_OUT_TASK_RUNNER_H_
