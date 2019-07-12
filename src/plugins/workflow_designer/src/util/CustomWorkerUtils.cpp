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

#include <QRegularExpression>

#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>

#include "CustomWorkerUtils.h"

namespace U2 {
namespace Workflow {

QString CustomWorkerUtils::getVarName(const ExternalTool *tool) {
    SAFE_POINT(!tool->isModule(),
               "Bad external tool id",
               "__UGENE_BAD_EXTERNAL_TOOL_TYPE__");
    QString id = tool->getId();

// TODO: workaround while id(s) are not yet committed
#ifndef __ichebyki__
    id = id.toUpper().replace(QRegularExpression("[^A-Z0-9_]"), "_");
#endif

    SAFE_POINT((id.indexOf(QRegularExpression("[^A-Z0-9_]")) < 0),
               "Bad external tool id",
               "__UGENE_BAD_EXTERNAL_TOOL_ID__");

    return "UGENE_" + id;
}

bool CustomWorkerUtils::commandContainsSpecialTool(const QString &cmd, const QString toolName) {
    ExternalTool *tool = AppContext::getExternalToolRegistry()->getByName(toolName);
    if (tool) {
        QString varName = getVarName(tool);
        QRegularExpression regex1 = QRegularExpression(CMDTOOL_SPECIAL_REGEX + ("%" + varName + "%"));
        return cmd.indexOf(regex1) >= 0;
    }
    return false;
}

bool CustomWorkerUtils::commandContainsSpecialTool(const QString &cmd, const ExternalTool *tool) {
    QString varName = getVarName(tool);
    QRegularExpression regex1 = QRegularExpression(CMDTOOL_SPECIAL_REGEX + ("%" + varName + "%"));
    return cmd.indexOf(regex1) >= 0;
}

bool CustomWorkerUtils::commandReplaceSpecialByUgenePath(QString &cmd, const QString varName, const QString path) {
    bool result = false;
    QRegularExpression regex1 = QRegularExpression(CMDTOOL_SPECIAL_REGEX + ("%" + varName + "%"));
    while (cmd.indexOf(regex1) >= 0) {
        cmd.replace(regex1, "\\1\"" + path + "\"");
        result |= true;
    }

    return result;
}

bool CustomWorkerUtils::commandReplaceSpecialByUgenePath(QString &cmd, const ExternalTool *tool) {
    bool result = false;
    QString varName = getVarName(tool);
    QRegularExpression regex1 = QRegularExpression(CMDTOOL_SPECIAL_REGEX + ("%" + varName + "%"));
    while (cmd.indexOf(regex1) >= 0) {
        cmd.replace(regex1, "\\1\"" + tool->getPath() + "\"");
        result |= true;
    }

    return result;
}

void CustomWorkerUtils::commandReplaceAllSpecialByUgenePath(QString &cmd, ExternalProcessConfig *cfg) {
    commandReplaceSpecialByUgenePath(cmd, "TOOL_PATH", cfg->customToolPath);

    QList<ExternalTool *> all = AppContext::getExternalToolRegistry()->getAllEntries();
    for (auto tool : all) {
        if (!tool->isModule()) {
            commandReplaceSpecialByUgenePath(cmd, tool);
        }
    }
}


} // Workflow
} // U2
