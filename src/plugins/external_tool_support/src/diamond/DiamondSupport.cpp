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

#include "DiamondSupport.h"

namespace U2 {

const QString DiamondSupport::TOOL_NAME = "diamond";

DiamondSupport::DiamondSupport(const QString &name)
    : ExternalTool(name)
{
    if (NULL != AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }

    validationArguments << "--version";
    validMessage = "diamond version ";
    versionRegExp = QRegExp("diamond version (\\d+\\.\\d+\\.\\d+)");
    executableFileName = "diamond";
    description = tr("\"<i>Diamond</i>\" is accelerated BLAST compatible local sequence aligner.");
}

}   // namesapce U2
