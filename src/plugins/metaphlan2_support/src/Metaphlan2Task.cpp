/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2018 UniPro <ugene@unipro.ru>
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

#include <QDir>
#include <QFileInfo>

#include <U2Core/Counter.h>
#include <U2Core/GUrlUtils.h>

#include "Metaphlan2Support.h"
#include "Metaphlan2Task.h"

namespace U2 {


Metaphlan2TaskSettings::Metaphlan2TaskSettings() : isPairedEnd(false),
                                                   numberOfThreads(1) {}

Metaphlan2ClassifyTask::Metaphlan2ClassifyTask(const Metaphlan2TaskSettings& _settings) :
                                    ExternalToolSupportTask(tr("Classify reads with Metaphlan2"),
                                    TaskFlags_NR_FOSE_COSC | TaskFlag_MinimizeSubtaskErrorText),
                                    settings(_settings),
                                    classifyTask(nullptr){
    GCOUNTER(cvar, tvar, "Metaphlan2ClassifyTask");

    SAFE_POINT_EXT(!settings.databaseUrl.isEmpty(), setError(tr("Metaphlan2 database URL is empty")), );
    SAFE_POINT_EXT(!settings.bowtie2OutputFile.isEmpty(), setError(tr("Bowtie2 output file URL is empty")), );
    SAFE_POINT_EXT(!settings.outputFile.isEmpty(), setError(tr("Metaphlan2 output file URL is empty")), );

    SAFE_POINT_EXT(!settings.tmpDir.isEmpty(), setError("Temporary directory URL is empty"), );
    SAFE_POINT_EXT(!settings.readsUrl.isEmpty(), setError(tr("Reads URL is empty")), );
    SAFE_POINT_EXT(!settings.isPairedEnd ||
                   !settings.readsUrl.isEmpty(),
                   setError(tr("Paired reads URL is empty, but the 'paired reads' option is set")), );
}

const QString& Metaphlan2ClassifyTask::getBowtie2OutputUrl() const {
    return settings.bowtie2OutputFile;
}

const QString& Metaphlan2ClassifyTask::getOutputUrl() const {
    return settings.outputFile;
}

void Metaphlan2ClassifyTask::prepare() {
    classifyTask = new ExternalToolRunTask(Metaphlan2Support::TOOL_NAME,
                                           getArguments(),
                                           new ExternalToolLogParser(),
                                           QString(),
                                           QStringList() << settings.bowtie2ExternalToolPath);
    setListenerForTask(classifyTask);
    addSubTask(classifyTask);
}

QStringList Metaphlan2ClassifyTask::getArguments() {
    QStringList arguments;
    if (!settings.isPairedEnd) {
        arguments << QDir::toNativeSeparators(settings.readsUrl);
    } else {
        arguments << QString("%1,%2").arg(QDir::toNativeSeparators(settings.readsUrl)).arg(QDir::toNativeSeparators(settings.pairedReadsUrl));
    }

    arguments << "--nproc" << QString::number(settings.numberOfThreads);
    arguments << "--tmp_dir" << QDir::toNativeSeparators(settings.tmpDir);
    arguments << "--input_type" << settings.inputType;
    arguments << "--bowtie2out" << QDir::toNativeSeparators(settings.bowtie2OutputFile);

    QDir databaseDir(QDir::toNativeSeparators(settings.databaseUrl));
    QStringList filters = QStringList() << "*.pkl";
    QStringList pklFiles = databaseDir.entryList(filters);
    SAFE_POINT_EXT(!pklFiles.isEmpty(), stateInfo.setError(tr(".pkl file is absent in the database folder")), QStringList());
    SAFE_POINT_EXT(pklFiles.size() == 1, stateInfo.setError(tr("There is 1 .pkl file in the database folder expected")), QStringList());

    arguments << "--mpa_pkl" << QDir::toNativeSeparators(QString("%1/%2").arg(settings.databaseUrl).arg(pklFiles.first()));
    arguments << "--bowtie2db" << QDir::toNativeSeparators(settings.databaseUrl);
    arguments << "-o" << QDir::toNativeSeparators(settings.outputFile);

    return arguments;
}


} // namespace U2

