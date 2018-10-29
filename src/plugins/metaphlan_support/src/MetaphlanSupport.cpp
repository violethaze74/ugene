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

#include "MetaphlanSupport.h"

#include <U2Core/U2SafePoints.h>

#include <QFileInfo>

namespace U2 {

const QString MetaphlanSupport::TOOL_NAME = "MetaPhlAn2";
const QString MetaphlanSupport::UTIL_SCRIPT = "utils/read_fastx.py";
const QString MetaphlanSupport::ET_PYTHON = "python";
const QString MetaphlanSupport::ET_PYTHON_NUMPY = "numpy";
const QString MetaphlanSupport::ET_BOWTIE_2_ALIGNER = "Bowtie 2 aligner";

MetaphlanSupport::MetaphlanSupport(const QString& name, const QString& path) : ExternalTool(name, path) {
    validationArguments << "--version";

    toolKitName = TOOL_NAME;
    description = tr("<i>MetaPhlAn2 (METAgenomic PHyLogenetic ANalysis)</i> is a tool for profiling the composition of microbial communities (bacteria, archaea, eukaryotes, and viruses) from whole-metagenome shotgun sequencing data.");

    executableFileName = "metaphlan2.py";

    toolRunnerProgramm = "java";
    dependencies << ET_PYTHON << ET_PYTHON_NUMPY << ET_BOWTIE_2_ALIGNER;

    validMessage = "MetaPhlAn version ";//TODO
    versionRegExp = QRegExp("((\\-[a-zA-Z]*){,2})MetaPhlAn version (\\d+\\.\\d+\\.\\d+)");//Hmmm

}

void MetaphlanSupport::checkAdditionalScripts(const QString& toolPath, QStringList& unpresentedScripts) const {
    QFileInfo file(toolPath);
    QString utilScriptFullPath = QString("%1/%2").arg(file.absolutePath()).arg(UTIL_SCRIPT);
    if (!QFileInfo::exists(utilScriptFullPath)) {
        unpresentedScripts << UTIL_SCRIPT;
    }
}

}//namespace
