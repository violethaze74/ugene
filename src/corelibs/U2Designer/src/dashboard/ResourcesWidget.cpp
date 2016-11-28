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

#include <U2Lang/WorkflowSettings.h>

#include "ResourcesWidget.h"


namespace U2 {
#if (QT_VERSION < 0x050400) //Qt 5.7
ResourcesWidget::ResourcesWidget(const QWebElement &container, Dashboard *parent)
#else
ResourcesWidget::ResourcesWidget(const QString &container, Dashboard *parent)
#endif
    : DashboardWidget(container, parent)
{
    connect(parent->monitor(), SIGNAL(si_progressChanged(int)), SLOT(sl_progressChanged(int)));
    connect(parent->monitor(), SIGNAL(si_taskStateChanged(Monitor::TaskState)),
        SLOT(sl_taskStateChanged(Monitor::TaskState)));
#if (QT_VERSION < 0x050400) //Qt 5.7
    QString content = QString(
        "<div class=\"well well-small vlayout-item\">"
        "%1: <span id=\"timer\"></span>"
        "</div>"
        "<div class=\"progress-wrapper vlayout-item\">"
        "<div class=\"progress-container\">"
        "<div id=\"progressBar\" class=\"progress small-bar\">"
        "<div class=\"bar\" style=\"width: 0%;\"></div>"
        "</div>"
        "</div>"
        "</div>"
        "<div id=\"status-bar\" class=\"vlayout-item alert\">"
        "<p id=\"status-message\"/>"
        "</div>"
        ).arg(tr("Time"));
    this->container.setInnerXml(content);
#else
    dashboard->page()->runJavaScript("addResWidgetContent(\"" + tr("Time") + "\");");
#endif
    sl_progressChanged(0);
    running();
}

void ResourcesWidget::sl_progressChanged(int progress) {
#if (QT_VERSION < 0x050400) //Qt 5.7
    QWebElement bar = container.findFirst(".bar");
    bar.setStyleProperty("width", QString::number(progress) + "%");
#else
    dashboard->page()->runJavaScript("changeProgress(\"" + QString::number(progress) + "\")");
#endif
}

namespace {
    bool isFinished(TaskState state) {
        switch (state) {
            case Monitor::RUNNING:
            case Monitor::RUNNING_WITH_PROBLEMS:
                return false;
            case Monitor::FINISHED_WITH_PROBLEMS:
            case Monitor::CANCELLED:
            case Monitor::FAILED:
            case Monitor::SUCCESS:
                return true;
            default:
                FAIL("Unknown state", false);
        }
    }
}

void ResourcesWidget::sl_taskStateChanged(TaskState state) {
    if (Monitor::RUNNING == state) {
        running();
    } else if (Monitor::RUNNING_WITH_PROBLEMS == state) {
        runningWithProblems();
    } else if (Monitor::FINISHED_WITH_PROBLEMS == state) {
        finishedWithProblems();
    } else if (Monitor::FAILED == state) {
        failed();
    } else if (Monitor::SUCCESS == state) {
        success();
    } else {
        canceled();
    }

    bool showHint = WorkflowSettings::isShowLoadButtonHint();

    if (isFinished(state)) {
#if (QT_VERSION < 0x050400) //Qt 5.7
        dashboard->getDocument().evaluateJavaScript("showLoadButton(" + QString::number(showHint) + ")");
#else
        dashboard->page()->runJavaScript("showLoadButton(" + QString::number(showHint) + ")");
#endif
    }
}

void ResourcesWidget::running() {
#if (QT_VERSION < 0x050400) //Qt 5.7
    statusBar().addClass("alert-info");
    statusMessage().setPlainText(tr("The workflow task is in progress..."));
#else
    dashboard->page()->runJavaScript("addClass(\"status-bar\", \"alert-info\")");
    dashboard->page()->runJavaScript("addContent(\"status-message\", \"" + tr("The workflow task is in progress...")  + "\")");
#endif
}

void ResourcesWidget::runningWithProblems() {
#if (QT_VERSION < 0x050400) //Qt 5.7
    statusBar().removeClass("alert-info");
    statusMessage().setPlainText(tr("The workflow task is in progress. There are problems..."));
#else
    dashboard->page()->runJavaScript("removeClass(\"status-bar\", \"alert-info\")");
    dashboard->page()->runJavaScript("addContent(\"status-message\", \"" + tr("The workflow task is in progress. There are problems...") + "\")");
#endif
}

void ResourcesWidget::finishedWithProblems() {
#if (QT_VERSION < 0x050400) //Qt 5.7
    statusBar().removeClass("alert-info");
    statusBar().addClass("alert-warning");
    statusMessage().setPlainText(tr("The workflow task has been finished with warnings!"));
#else
    dashboard->page()->runJavaScript("removeClass(\"status-bar\", \"alert-info\")");
    dashboard->page()->runJavaScript("addContent(\"status-message\", \"" + tr("The workflow task has been finished with warnings!") + "\")");
#endif
}

void ResourcesWidget::failed() {
#if (QT_VERSION < 0x050400) //Qt 5.7
    statusBar().removeClass("alert-info");
    statusBar().addClass("alert-error");
    statusMessage().setPlainText(tr("The workflow task has been finished with errors!"));
#else
    dashboard->page()->runJavaScript("removeClass(\"status-bar\", \"alert-info\")");
    dashboard->page()->runJavaScript("addClass(\"status-bar\", \"alert-error\")");
    dashboard->page()->runJavaScript("addContent(\"status-message\", \"" + tr("The workflow task has been finished with errors!") + "\")");
#endif
}

void ResourcesWidget::success() {
#if (QT_VERSION < 0x050400) //Qt 5.7
    statusBar().removeClass("alert-info");
    statusBar().addClass("alert-success");
    statusMessage().setPlainText(tr("The workflow task has been finished successfully!"));
#else
    dashboard->page()->runJavaScript("removeClass(\"status-bar\", \"alert-info\")");
    dashboard->page()->runJavaScript("addClass(\"status-bar\", \"alert-success\")");
    dashboard->page()->runJavaScript("addContent(\"status-message\", \"" + tr("The workflow task has been finished successfully!") + "\")");
#endif
}

void ResourcesWidget::canceled() {
#if (QT_VERSION < 0x050400) //Qt 5.7
    statusBar().removeClass("alert-info");
    statusMessage().setPlainText(tr("The workflow task has been canceled!"));
#else
    dashboard->page()->runJavaScript("removeClass(\"status-bar\", \"alert-info\")");
    dashboard->page()->runJavaScript("addContent(\"status-message\", \"" + tr("The workflow task has been canceled!") + "\")");
#endif
}

#if (QT_VERSION < 0x050400) //Qt 5.7
QWebElement ResourcesWidget::statusBar() {
    return container.findFirst("#status-bar");
}

QWebElement ResourcesWidget::statusMessage() {
    return container.findFirst("#status-message");
}
#endif

} // U2
