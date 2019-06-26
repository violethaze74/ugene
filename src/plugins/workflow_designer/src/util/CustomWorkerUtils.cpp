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
#include "U2Core/ExternalToolRegistry.h"
#include <U2Core/U2SafePoints.h>

#include "CustomWorkerUtils.h"

namespace U2 {
namespace Workflow {

////////////////////////////////////////
// Special external tools
QMap<QString, QString> CustomWorkerUtils::specialTools {{"java", "UGENE_JAVA"},
                                                        {"python", "UGENE_PYTHON"},
                                                        {"perl", "UGENE_PERL"},
                                                        {"Rscript", "UGENE_RSCRIPT"}};


bool CustomWorkerUtils::commandContainsSpecialTool(const QString &cmd, const QString &toolKey) {
    QString value = specialTools.value(toolKey);
    if (!value.isNull() && !value.isEmpty()) {
        QRegularExpression regex1 = QRegularExpression("([^\\\\]|^)%" + value + "%");
        return cmd.indexOf(regex1) >= 0;
    }
    return false;
}

bool CustomWorkerUtils::commandReplaceSpecialByUgenePath(QString &cmd, const QString &toolKey) {
    bool result = false;
    QString value = specialTools.value(toolKey, nullptr);
    if (!value.isNull() && !value.isEmpty()) {
        QRegularExpression regex1 = QRegularExpression("([^\\\\]|[^\\\\](\\\\\\\\)+|^)%" + value + "%");
        while (cmd.indexOf(regex1) >= 0) {
            ExternalTool* tool = AppContext::getExternalToolRegistry()->getByName(toolKey);
            CHECK(tool, false);
            cmd.replace(regex1, "\\1\"" + tool->getPath() + "\"");
            result |= true;
        }
    }
    return result;
}

void CustomWorkerUtils::commandReplaceAllSpecialByUgenePath(QString &cmd) {
    for (auto key : specialTools.keys()) {
        commandReplaceSpecialByUgenePath(cmd, key);
    }
}


} // Workflow
} // U2
