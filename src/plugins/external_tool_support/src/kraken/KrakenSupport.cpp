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

#include <U2Core/AppContext.h>

#include "KrakenSupport.h"
#include "perl/PerlSupport.h"

namespace U2 {

const QString KrakenSupport::BUILD_TOOL = "kraken-build";
const QString KrakenSupport::CLASSIFY_TOOL = "kraken";

KrakenSupport::KrakenSupport(const QString &name)
    : ExternalTool(name)
{
    if (NULL != AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }

    toolKitName = "Kraken";

    validationArguments << "--version";
    validMessage = "Kraken version ";
    versionRegExp = QRegExp("Kraken version (\\d+\\.\\d+(\\.\\d+)?(\\-[a-zA-Z]*)?)");
    dependencies << ET_PERL;

    if (BUILD_TOOL == name) {
        initBuild();
    }

    if (CLASSIFY_TOOL == name) {
        initClassify();
    }
}

void KrakenSupport::initBuild() {
#ifdef Q_OS_WIN
    executableFileName = "kraken-build.exe";
#elif defined(Q_OS_UNIX)
    executableFileName = "kraken-build";
#endif

    description = tr("The \"<i>kraken-build</i>\" executable is used to build a Kraken database.");
}

void KrakenSupport::initClassify() {
#ifdef Q_OS_WIN
    executableFileName = "kraken.exe";
#elif defined(Q_OS_UNIX)
    executableFileName = "kraken";
#endif

    description = tr("The \"<i>kraken</i>\" executable is used to classify a set of sequences. It does this by examining the k-mers within a read and querying a database with those k-mers.");
}

}   // namespace U2
