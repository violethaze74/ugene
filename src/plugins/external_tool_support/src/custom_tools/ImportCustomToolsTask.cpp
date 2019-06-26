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

#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QXmlInputSource>
#include <QXmlSimpleReader>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Counter.h>
#include <U2Core/CustomExternalTool.h>
#include <U2Core/GUrl.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include "ImportCustomToolsTask.h"
#include "CustomToolConfigParser.h"

namespace U2 {

const QString ImportCustomToolsTask::SETTINGS_PATH = "external_tools/custom_tool_configs";

ImportCustomToolsTask::ImportCustomToolsTask(const QString &_url)
    : Task(tr("Import custom external tools configuration"), TaskFlag_None),
      url(_url)
{

}

void ImportCustomToolsTask::run() {
    GCOUNTER(cvar, tvar, "ImportCustomToolsTask");

    parseConfigFile();
    CHECK_OP(stateInfo, );

    const bool registered = registerTool(tool.data());
    if (registered) {
        tool.take();
    }
}

void ImportCustomToolsTask::parseConfigFile() {
    QFile file(url);
    file.open(QIODevice::ReadOnly);

    QDomDocument doc;
    doc.setContent(&file);

    tool.reset(CustomToolConfigParser::parse(stateInfo, doc));
    CHECK_OP(stateInfo, );
    SAFE_POINT_EXT(nullptr != tool, setError("The imported tool is nullptr"), );
    if (!QFileInfo(tool->getPath()).isAbsolute()) {
        tool->setPath(GUrl(QFileInfo(url).absoluteDir().path() + "/" + tool->getPath()).getURLString());
    }
}

bool ImportCustomToolsTask::registerTool(CustomExternalTool *tool) {
    const bool registered = AppContext::getExternalToolRegistry()->registerEntry(tool);
    CHECK_EXT(registered, setError(tr("Can't register the tool '%1': there is another tool with the same name").arg(tool->getName())), false);

    QDomDocument doc = CustomToolConfigParser::serialize(tool);

    const QString storagePath = AppContext::getAppSettings()->getUserAppsSettings()->getCustomToolsConfigsDirPath();
    QDir().mkpath(storagePath);

    QFile configFile(GUrlUtils::rollFileName(storagePath + "/" + GUrlUtils::fixFileName(tool->getId()) + ".xml", "_"));
    configFile.open(QIODevice::WriteOnly);
    QTextStream stream(&configFile);
    stream << doc.toString(4);
    configFile.close();

    return true;
}

}   // namespace U2
