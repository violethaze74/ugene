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

#include <QDomDocument>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

#include <U2Core/CustomExternalTool.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>

#include "CustomToolConfigParser.h"

namespace U2 {

const QString CustomToolConfigParser::ELEMENT_CONFIG = "ugeneExternalToolConfig";
const QString CustomToolConfigParser::ATTRIBUTE_VERSION = "version";
const QString CustomToolConfigParser::HARDCODED_EXPECTED_VERSION = "1.0";

const QString CustomToolConfigParser::ID = "id";
const QString CustomToolConfigParser::NAME = "name";
const QString CustomToolConfigParser::PATH = "executableFullPath";
const QString CustomToolConfigParser::DESCRIPTION = "description";
const QString CustomToolConfigParser::TOOLKIT_NAME = "toolkitName";
const QString CustomToolConfigParser::TOOL_VERSION = "version";
const QString CustomToolConfigParser::LAUNCHER_ID = "launcherId";
const QString CustomToolConfigParser::DEPENDENCIES = "dependencies";
const QString CustomToolConfigParser::BINARY_NAME = "executableName";

CustomExternalTool *CustomToolConfigParser::parse(U2OpStatus &os, const QString &url) {
    QFile file(url);
    CHECK_EXT(file.open(QIODevice::ReadOnly), os.setError(tr("Invalid config file format: file %1 cann not be opened").arg(url)), nullptr);

    QDomDocument doc;
    doc.setContent(&file);
    file.close();

    QScopedPointer<CustomExternalTool> tool(new CustomExternalTool());

    const QDomNodeList nodesList = doc.elementsByTagName(ELEMENT_CONFIG);
    CHECK_EXT(!nodesList.isEmpty(), os.setError(tr("Invalid config file format: custom tool description not found")), nullptr);
    CHECK_EXT(1 == nodesList.count(), os.setError(tr("Invalid config file format: there are too many entities in the file")), nullptr);

    QDomElement configElement = nodesList.item(0).toElement();
    CHECK_EXT(!configElement.isNull(), os.setError(tr("Can't parse the config file")), nullptr);

    const QString &version = configElement.attribute(ATTRIBUTE_VERSION);
    CHECK_EXT(HARDCODED_EXPECTED_VERSION == version, os.setError(tr("Can't parse config with version %1").arg(version)), nullptr);

    const QDomNodeList toolConfigElements = configElement.childNodes();
    QFileInfo urlFi(url);
    for (int i = 0, n = toolConfigElements.count(); i < n; ++i) {
        const QDomElement element = toolConfigElements.item(i).toElement();
        CHECK_CONTINUE(!element.isNull());
        const QString tagName = element.tagName();

        if (ID == tagName) {
            tool->setId(element.text());
        } else if (NAME == tagName) {
            tool->setName(element.text());
        } else if (PATH == tagName) {
            if (!element.text().isEmpty()) {
                QString text = element.text();
                QFileInfo pathFi(element.text());
                QString absPath;
                if (pathFi.isRelative()) {
                    QString newPath = urlFi.absoluteDir().absolutePath() + "/" + element.text();
                    pathFi = QFileInfo(newPath);
                }
                absPath = pathFi.absoluteFilePath();
                tool->setPath(absPath);
            }
        } else if (DESCRIPTION == tagName) {
            tool->setDescription(element.text());
        } else if (TOOLKIT_NAME == tagName) {
            tool->setToolkitName(element.text());
        } else if (TOOL_VERSION == tagName) {
            tool->setPredefinedVersion(element.text());
        } else if (LAUNCHER_ID == tagName) {
            tool->setLauncher(element.text());
        } else if (DEPENDENCIES == tagName) {
            QStringList dependencies;
            foreach (const QString &dependency, element.text().split(",", QString::SkipEmptyParts)) {
                dependencies << dependency.trimmed();
            }
            tool->setDependencies(dependencies);
        } else if (BINARY_NAME == tagName) {
            tool->setBinaryName(element.text());
        } else {
            os.addWarning(tr("Unknown element: '%1', skipping").arg(tagName));
        }
    }

    if (tool->getPath().isEmpty()) {
        QString expectedExecutableUrl = urlFi.absoluteDir().absolutePath() + "/" + tool->getExecutableFileName();
        QFile expectedExecutable(expectedExecutableUrl);
        if (expectedExecutable.exists()) {
            tool->setPath(expectedExecutableUrl);
        }
    }

    if (tool->getToolKitName().isEmpty()) {
        tool->setToolkitName(tool->getName());
    }

    const bool valid = validate(os, tool.data());
    CHECK(valid, nullptr);

    return tool.take();
}

QDomDocument CustomToolConfigParser::serialize(CustomExternalTool *tool) {
    QDomDocument doc;
    QDomProcessingInstruction xmlHeader = doc.createProcessingInstruction("xml", "version = \"1.0\" encoding = \"UTF-8\"");
    doc.appendChild(xmlHeader);

    QDomElement configElement = doc.createElement(ELEMENT_CONFIG);
    configElement.setAttribute(ATTRIBUTE_VERSION, HARDCODED_EXPECTED_VERSION);
    configElement.appendChild(addChildElement(doc, ID, tool->getId()));
    configElement.appendChild(addChildElement(doc, NAME, tool->getName()));
    configElement.appendChild(addChildElement(doc, PATH, tool->getPath()));
    configElement.appendChild(addChildElement(doc, DESCRIPTION, tool->getDescription()));
    configElement.appendChild(addChildElement(doc, TOOLKIT_NAME, tool->getToolKitName()));
    configElement.appendChild(addChildElement(doc, TOOL_VERSION, tool->getPredefinedVersion()));
    configElement.appendChild(addChildElement(doc, LAUNCHER_ID, tool->getToolRunnerProgramId()));
    configElement.appendChild(addChildElement(doc, DEPENDENCIES, tool->getDependencies().join(",")));
    configElement.appendChild(addChildElement(doc, BINARY_NAME, tool->getExecutableFileName()));
    doc.appendChild(configElement);
    return doc;
}

bool CustomToolConfigParser::validate(U2OpStatus &os, CustomExternalTool *tool) {
    CHECK(nullptr != tool, false);
    CHECK_EXT(!tool->getId().isEmpty(), os.setError(tr("The tool id is not specified in the config file")), false);
    CHECK_EXT(!tool->getId().contains(QRegularExpression("[^A-Za-z0-9_\\-]")), os.setError(tr("The tool id contains unexpected characters, the only letters, numbers, underlines and dashes are allowed")), false);
    CHECK_EXT(!tool->getName().isEmpty(), os.setError(tr("The tool name is not specified in the config file")), false);
    CHECK_EXT(!tool->getExecutableFileName().isEmpty(), os.setError(tr("The tool's executable file name is not specified in the config file")), false);
    return true;
}

QDomElement CustomToolConfigParser::addChildElement(QDomDocument &doc, const QString &elementName, const QString &elementData) {
    QDomElement element = doc.createElement(elementName);
    QDomText elementDataNode = doc.createTextNode(elementData);
    element.appendChild(elementDataNode);
    return element;
}

}   // namespace U2
