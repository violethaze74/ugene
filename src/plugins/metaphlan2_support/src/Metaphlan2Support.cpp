/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2019 UniPro <ugene@unipro.ru>
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

#include "Metaphlan2Support.h"

#include <U2Core/U2SafePoints.h>

#include <QFileInfo>

namespace U2 {

const QString Metaphlan2Support::TOOL_NAME = "MetaPhlAn2";
const QString Metaphlan2Support::UTIL_SCRIPT = "utils/read_fastx.py";
//These constants are taken from the 'external_tool_support' project
const QString Metaphlan2Support::ET_PYTHON = "python";
const QString Metaphlan2Support::ET_PYTHON_NUMPY = "numpy";
const QString Metaphlan2Support::ET_BOWTIE_2_ALIGNER = "Bowtie 2 aligner";

Metaphlan2Support::Metaphlan2Support(const QString& name, const QString& path) : ExternalTool(name, path) {
    validationArguments << "--version";

    toolKitName = TOOL_NAME;
    description = tr("<i>MetaPhlAn2 (METAgenomic PHyLogenetic ANalysis)</i> is a tool for profiling the composition of microbial communities (bacteria, archaea, eukaryotes, and viruses) from whole-metagenome shotgun sequencing data.");

    executableFileName = "metaphlan2.py";

    toolRunnerProgramm = ET_PYTHON;
    dependencies << ET_PYTHON << ET_PYTHON_NUMPY << ET_BOWTIE_2_ALIGNER;

    validMessage = "MetaPhlAn version ";
    versionRegExp = QRegExp("MetaPhlAn version (\\d+\\.\\d+(\\.\\d+)?(\\-[a-zA-Z]*)?)");
}

void Metaphlan2Support::performAdditionalChecks(const QString& toolPath) {
    QFileInfo file(toolPath);
    QString utilScriptFullPath = QString("%1/%2").arg(file.absolutePath()).arg(UTIL_SCRIPT);
    if (!QFileInfo::exists(utilScriptFullPath)) {
        additionalErrorMesage = tr("%1 script \"%2\" is not present!").arg(TOOL_NAME).arg(UTIL_SCRIPT);
    }
}

}//namespace
