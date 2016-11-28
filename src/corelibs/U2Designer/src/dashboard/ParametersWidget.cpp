/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include "ParametersWidget.h"

#include <U2Core/U2SafePoints.h>

#include <U2Lang/Dataset.h>
#include <U2Lang/URLAttribute.h>
#include <U2Lang/URLContainer.h>
#include <U2Lang/WorkflowUtils.h>


namespace U2 {

#if (QT_VERSION < 0x050400) //Qt 5.7
ParametersWidget::ParametersWidget(const QWebElement &container, Dashboard *parent)
#else
ParametersWidget::ParametersWidget(const QString &container, Dashboard *parent)
#endif
    : DashboardWidget(container, parent)
{
    const WorkflowMonitor *workflowMonitor = dashboard->monitor();
    SAFE_POINT(NULL != workflowMonitor, "NULL workflow monitor!", );

    const QList<WorkerParamsInfo> &workersParamsInfo = workflowMonitor->getWorkersParameters();
    createWidget(workersParamsInfo);
}

void ParametersWidget::createWidget(const QList<WorkerParamsInfo> &workersParamsInfo) {
    int i = 0;
    foreach (const WorkerParamsInfo &info, workersParamsInfo) {
        // Create the tab
        QString tabId = "params_tab_id_" + QString::number(i);
        QString createTabFunc;
#if (QT_VERSION < 0x050400) //Qt 5.7
        if (0 == i) {
            createTabFunc = "pwInitAndActiveTab";
        }
        else {
            createTabFunc = "pwAddTab";
        }
        createTabFunc += "(this, '" + info.workerName + "', '" + tabId + "')";

        container.evaluateJavaScript(createTabFunc);
#else
        if (0 == i) {
            createTabFunc = "pwInitAndActiveTab_webengine";
        } else {
            createTabFunc = "pwAddTab_webengine";
        }
        createTabFunc += "('" + container + "', '" + info.workerName + "', '" + tabId + "')";
        dashboard->page()->runJavaScript(createTabFunc);
#endif
        // Add the parameters
        foreach (Attribute* param, info.parameters) {
            SAFE_POINT(NULL != param, "NULL attribute!", );

            QVariant paramValueVariant = param->getAttributePureValue();
            if (paramValueVariant.canConvert< QList<Dataset> >()) {
                QList<Dataset> sets = paramValueVariant.value< QList<Dataset > >();

                foreach (const Dataset &set, sets) {
                    QString paramName = param->getDisplayName();
                    if (sets.size() > 1) {
                        paramName += ": <i>" + set.getName() + "</i>";
                    }
                    QStringList urls;
                    foreach (URLContainer *c, set.getUrls()) {
                        urls << c->getUrl();
                    }
                    QString paramValue = urls.join(";");

                    QString createParamFunc = "pwAddFilesParameter";
                    createParamFunc += "('" + tabId + "', ";
                    createParamFunc += "'" + paramName + "', ";
                    createParamFunc += "'" + paramValue + "', false)";
#if (QT_VERSION < 0x050400) //Qt 5.7
                    container.evaluateJavaScript(createParamFunc);
#else
                    dashboard->page()->runJavaScript(createParamFunc);
#endif
                }
            }
            else {
                QString paramName = param->getDisplayName();
                QString paramValue = WorkflowUtils::getStringForParameterDisplayRole(paramValueVariant);

                QString createParamFunc;

                UrlAttributeType type = WorkflowUtils::isUrlAttribute(param, info.actor);
                if (type == NotAnUrl || QString::compare(paramValueVariant.toString(), "default", Qt::CaseInsensitive) == 0) {
                    createParamFunc = "pwAddCommonParameter";
                }
                else {
                    createParamFunc = "pwAddFilesParameter";
                }

                createParamFunc += "('" + tabId + "', ";
                createParamFunc += "'" + paramName + "', ";
                createParamFunc += "'" + paramValue + "'";

                if (type != NotAnUrl && QString::compare(paramValueVariant.toString(), "default", Qt::CaseInsensitive) != 0) {
                    bool disableOpeningByUgene = false;
                    if (type == InputDir || type == OutputDir) {
                        disableOpeningByUgene = true;
                    }
                    createParamFunc += ", " + QString::number(disableOpeningByUgene);
                }

                createParamFunc += ")";
#if (QT_VERSION < 0x050400) //Qt 5.7
                container.evaluateJavaScript(createParamFunc);
#else
                dashboard->page()->runJavaScript(createParamFunc);
#endif
            }
        }

        // Increase the iterator
        i++;
    }
}


} // namespace U2
