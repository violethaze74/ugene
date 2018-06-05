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

#include <QFileInfo>

#include <U2Core/AppContext.h>

#include "TrimmomaticSupport.h"
#include "java/JavaSupport.h"

namespace U2 {

TrimmomaticSupport::TrimmomaticSupport(const QString &name, const QString &path)
    : ExternalTool(name, path)
{
    toolKitName = "Trimmomatic";
    description = tr("<i>Trimmomatic</i> is a flexible read trimming tool for Illumina NGS data.");

    executableFileName = "trimmomatic.jar";
    validationArguments << "-h";
    validMessage = "PE \\[-version\\] \\[-threads <threads>\\] \\[-phred33|-phred64\\] \\[-trimlog <trimLogFile>\\]";

    toolRunnerProgramm = ET_JAVA;
    dependencies << ET_JAVA;
}

} // namespace U2
