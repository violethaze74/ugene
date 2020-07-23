/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2020 UniPro <ugene@unipro.ru>
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

#include "./ParametersDashboardWidget.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>

#include <U2Core/AppContext.h>
#include <U2Core/L10n.h>
#include <U2Core/ProjectModel.h>

#include <U2Gui/HoverQLabel.h>

#include <U2Lang/Dataset.h>
#include <U2Lang/URLContainer.h>
#include <U2Lang/WorkflowUtils.h>

#include "./DomUtils.h"

namespace U2 {

#define URL_MARKER_CLASS "url-marker"
#define DIR_MARKER_CLASS "dir-marker"
#define DATASET_MARKER_CLASS "dataset-marker"

static QList<WorkerInfo> dom2WorkerInfos(const QDomElement &dom) {
    QList<WorkerInfo> workers;
    QDomElement navTabsEl = DomUtils::findChildElementByClass(dom, "params-nav-tabs", 2);
    QDomNodeList workerNameList = navTabsEl.elementsByTagName("li");
    for (int i = 0; i < workerNameList.size(); i++) {
        QDomElement workerNameLi = workerNameList.at(i).toElement();
        QList<WorkerParameterInfo> parameters;
        workers << WorkerInfo(workerNameLi.text().simplified(), parameters);
    }
    QDomElement paramTablesRoot = DomUtils::findChildElementByClass(dom, "params-tab-content", 2);
    QList<QDomElement> paramTableList = DomUtils::findChildElementsByClass(paramTablesRoot, "param-value-column");
    for (int i = 0; i < paramTableList.size() && i < workers.size(); i++) {
        QDomElement table = paramTableList[i];
        QDomElement tr = table.firstChildElement("tr");
        if (tr.isNull()) {
            tr = table.firstChildElement("tbody").firstChildElement("tr");
            if (tr.isNull()) {
                tr = table.firstChildElement("thead").firstChildElement("tr");    // UGENE <= v35 has all <tr>s within <thead>.
            }
        }
        int rowSpan = 0;
        for (; !tr.isNull(); tr = tr.nextSiblingElement("tr"), rowSpan--) {
            QDomElement td1 = tr.firstChildElement("td");
            if (td1.isNull()) {
                continue;
            }
            QString name = rowSpan > 0 ? "" : td1.text().simplified();
            QDomElement td2 = rowSpan > 0 ? td1 : td1.nextSiblingElement("td");
            QString value = td2.text().trimmed();
            bool isUrl = DomUtils::hasClass(td2, URL_MARKER_CLASS);
            bool isDir = DomUtils::hasClass(td2, DIR_MARKER_CLASS);
            bool isDataset = DomUtils::hasClass(td2, DATASET_MARKER_CLASS);
            if (!isUrl) {    // Check if it is old-style URL (UGENE <= v35).
                QDomElement button = td2.firstChildElement("div").firstChildElement("div").firstChildElement("button");
                if (!button.isNull()) {
                    isDir = button.hasAttribute("disabled");
                    int prefixLen = QString("agent.openUrl('").length();
                    int suffixLen = QString("')").length();
                    value = button.attribute("onclick");
                    if (value.length() > prefixLen + suffixLen) {
                        value = value.mid(prefixLen, value.length() - prefixLen - suffixLen);
                    }
                    isUrl = value.length() > 0;
                }
            }
            if (rowSpan > 0) {
                workers[i].parameters.last().isDataset = true;
                workers[i].parameters.last().value += "\n" + value;
            } else {
                workers[i].parameters << WorkerParameterInfo(name, value, isUrl, isDir, isDataset);
            }
            if (td1.hasAttribute("rowspan")) {
                rowSpan = td1.attribute("rowspan").toInt();
            }
        }
    }
    return workers;
}

static QList<WorkerInfo> params2WorkerInfos(const QList<Workflow::Monitor::WorkerParamsInfo> &workerInfoList) {
    QList<WorkerInfo> result;
    for (auto workerInfo : workerInfoList) {
        QList<WorkerParameterInfo> parameters;
        for (auto p : workerInfo.parameters) {
            QString name = p->getDisplayName();
            QString value;
            bool isUrl = false;
            bool isDir = false;
            bool isDataset = false;
            QVariant valueVariant = p->getAttributePureValue();
            if (valueVariant.canConvert<QList<Dataset>>()) {
                QList<Dataset> sets = valueVariant.value<QList<Dataset>>();
                for (const Dataset &set : sets) {
                    if (sets.size() > 1) {
                        name += ": " + set.getName();
                    }
                    QStringList urls;
                    for (auto url : set.getUrls()) {
                        urls << url->getUrl();
                    }
                    value = urls.join("\n");
                    isDataset = true;
                }
            } else {
                value = WorkflowUtils::getStringForParameterDisplayRole(valueVariant);
                UrlAttributeType type = WorkflowUtils::isUrlAttribute(p, workerInfo.actor);
                if (type != NotAnUrl && QString::compare(valueVariant.toString(), "default", Qt::CaseInsensitive) != 0) {
                    isUrl = true;
                    isDir = type == InputDir || type == OutputDir;
                }
            }

            parameters << WorkerParameterInfo(name, value, isUrl, isDir, isDataset);
        }
        result << WorkerInfo(workerInfo.workerName, parameters);
    }
    return result;
}

ParametersDashboardWidget::ParametersDashboardWidget(const QString &dashboardDir, const QDomElement &dom, const WorkflowMonitor *monitor)
    : dashboardDir(dashboardDir) {
    setMinimumWidth(1100);

    layout = new QHBoxLayout();
    setLayout(layout);

    auto nameListWidget = new QWidget();
    nameListWidget->setFixedWidth(300);
    auto nameListWidgetLayout = new QVBoxLayout();
    nameListWidget->setLayout(nameListWidgetLayout);
    nameListWidgetLayout->setSpacing(0);
    layout->addWidget(nameListWidget, 1);

    auto rightWidget = new QWidget();
    auto rightWidgetLayout = new QVBoxLayout();
    rightWidget->setLayout(rightWidgetLayout);
    layout->addWidget(rightWidget);

    parametersLayout = new QGridLayout();
    parametersLayout->setSpacing(0);
    rightWidgetLayout->addLayout(parametersLayout);
    rightWidgetLayout->addStretch(1000);

    workers << dom2WorkerInfos(dom);
    if (monitor != nullptr) {
        workers << params2WorkerInfos(monitor->getWorkersParameters());
    }
    for (int i = 0; i < workers.size(); i++) {
        auto workerNameLabel = new HoverQLabel(workers[i].workerName, "", "");
        workerNameLabel->setWordWrap(true);
        workerNameLabel->setObjectName("worker-" + QString::number(i));
        workerNameLabel->setProperty("worker-index", i);
        nameListWidgetLayout->addWidget(workerNameLabel);
        connect(workerNameLabel, SIGNAL(clicked()), SLOT(sl_workerLabelClicked()));
        workerNameLabels << workerNameLabel;
    }
    nameListWidgetLayout->addStretch();
    showWorkerParameters(0);
}

void ParametersDashboardWidget::sl_workerLabelClicked() {
    auto label = qobject_cast<HoverQLabel *>(sender());
    CHECK(label != nullptr, );
    bool ok = false;
    int index = label->property("worker-index").toInt(&ok);
    CHECK(ok && index >= 0 && index <= workers.size(), );
    showWorkerParameters(index);
}

#define ACTION_CODE_KEY "action-code"

void ParametersDashboardWidget::showWorkerParameters(int workerIndex) {
    CHECK(workerIndex >= 0 && workerIndex <= workers.size(), );
    QLayoutItem *item;
    while ((item = parametersLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    if (workerIndex >= workers.size()) {
        return;
    }
    QString commonWorkerNameLabelStyle("padding: 0.7em;");
    QString activeWorkerNameLabelStyle = commonWorkerNameLabelStyle + "border-top: 1px solid #ddd; border-bottom: 1px solid #ddd; border-left: 1px solid #ddd; border-top-left-radius: 6px; border-bottom-left-radius: 6px;";
    QString notActiveWorkerNameLabelStyle = commonWorkerNameLabelStyle + "border-right: 1px solid #ddd;";
    for (int i = 0; i < workerNameLabels.size(); i++) {
        if (i == workerIndex) {
            workerNameLabels[i]->updateStyles(activeWorkerNameLabelStyle, activeWorkerNameLabelStyle);
        } else {
            workerNameLabels[i]->updateStyles(notActiveWorkerNameLabelStyle + "background: white;", notActiveWorkerNameLabelStyle + "background: #eee;");
        }
    }

    QString commonHeaderStyle = "border: 1px solid #999; background-color: rgb(101, 101, 101);";
    QString commonHeaderLabelStyle = "color: white; padding: 5px 10px;";
    auto parameterNameWidget = new QWidget();
    parameterNameWidget->setObjectName("nameHeaderWidget");
    parameterNameWidget->setStyleSheet("#nameHeaderWidget { " + commonHeaderStyle + "border-top-left-radius: 4px ;border-right: 0px;}");
    auto parameterNameWidgetLayout = new QVBoxLayout();
    parameterNameWidgetLayout->setContentsMargins(0, 0, 0, 0);
    parameterNameWidget->setLayout(parameterNameWidgetLayout);
    auto parameterNameHeaderLabel = new QLabel(tr("Parameter"));
    parameterNameHeaderLabel->setStyleSheet(commonHeaderLabelStyle);
    parameterNameWidgetLayout->addWidget(parameterNameHeaderLabel);
    parametersLayout->addWidget(parameterNameWidget, 0, 0);

    auto parameterValueWidget = new QWidget();
    parameterValueWidget->setObjectName("nameValueWidget");
    parameterValueWidget->setStyleSheet("#nameValueWidget {" + commonHeaderStyle + "border-left: 1px solid white; border-top-right-radius: 4px;}");
    auto parameterValueWidgetLayout = new QVBoxLayout();
    parameterValueWidgetLayout->setContentsMargins(0, 0, 0, 0);
    parameterValueWidget->setLayout(parameterValueWidgetLayout);
    auto parameterValueHeaderLabel = new QLabel(tr("Value"));
    parameterValueHeaderLabel->setStyleSheet(commonHeaderLabelStyle);
    parameterValueWidgetLayout->addWidget(parameterValueHeaderLabel);
    parametersLayout->addWidget(parameterValueWidget, 0, 1);

    QList<WorkerParameterInfo> &parameters = workers[workerIndex].parameters;
    for (int parameterIndex = 0; parameterIndex < parameters.size(); parameterIndex++) {
        const WorkerParameterInfo &parameter = parameters[parameterIndex];
        bool isLast = parameterIndex == parameters.size() - 1;
        auto nameWidget = new QWidget();
        nameWidget->setObjectName("nameWidget");
        nameWidget->setStyleSheet("#nameWidget {border: 1px solid #ddd; border-right: 0px; border-top: 0px;" + QString(isLast ? "border-bottom-left-radius:4px;" : "") + "}");
        auto nameWidgetLayout = new QVBoxLayout();
        nameWidgetLayout->setContentsMargins(0, 0, 0, 0);
        nameWidget->setLayout(nameWidgetLayout);
        auto nameLabel = new QLabel(parameter.name);
        nameLabel->setWordWrap(true);
        nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        nameLabel->setStyleSheet("padding: 7px 10px;");
        nameWidgetLayout->addWidget(nameLabel);
        nameWidgetLayout->addStretch();
        parametersLayout->addWidget(nameWidget, parameterIndex + 1, 0);

        auto valueWidget = new QWidget();
        valueWidget->setObjectName("valueWidget");
        valueWidget->setStyleSheet("#valueWidget {border: 1px solid #ddd; border-top: 0px;" + QString(isLast ? "border-bottom-right-radius:4px;" : "") + "}");
        auto valueWidgetLayout = new QVBoxLayout();
        valueWidgetLayout->setContentsMargins(0, 0, 0, 0);
        valueWidgetLayout->setSpacing(0);
        valueWidget->setLayout(valueWidgetLayout);

        if (!parameter.value.isEmpty() && (parameter.isUrl || parameter.isDir || parameter.isDataset)) {
            QStringList urls = parameter.value.split("\n");
            for (int i = 0; i < urls.length(); i++) {
                QString actionCode = QString::number(workerIndex) + "-" + QString::number(parameterIndex) + "-" + QString::number(i);
                auto button = new QToolButton();
                button->setText(QFileInfo(urls[i]).fileName());
                button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                button->setStyleSheet("QToolButton {"
                                      "  margin: 7px 10px; height: 1.33em; border-radius: 4px;"
                                      "  border: 1px solid #aaa; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 #dadbde);"
                                      "}"
                                      "QToolButton:pressed {"
                                      "  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbde, stop: 1 #f6f7fa);"
                                      "}"
                                      "QToolButton::menu-button {"
                                      "  border: 1px solid #aaa;"
                                      "  border-top-right-radius: 4px; border-bottom-right-radius: 4px;"
                                      "  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 #dadbde);"
                                      "  width: 1.5em;"
                                      "}");
                connect(button, SIGNAL(clicked()), SLOT(sl_openFileClicked()));

                if (!parameter.isDir) {
                    button->setProperty(ACTION_CODE_KEY, "ugene-" + actionCode);

                    auto menu = new PopupMenu(button, this);

                    auto openFolderAction = new QAction(tr("Open folder with the file"), button);
                    openFolderAction->setProperty(ACTION_CODE_KEY, "dir-" + actionCode);
                    connect(openFolderAction, SIGNAL(triggered()), SLOT(sl_openFileClicked()));
                    menu->addAction(openFolderAction);

                    auto openFileAction = new QAction(tr("Open file by OS"), button);
                    openFileAction->setProperty(ACTION_CODE_KEY, "file-" + actionCode);
                    connect(openFileAction, SIGNAL(triggered()), SLOT(sl_openFileClicked()));
                    menu->addAction(openFileAction);

                    button->setMenu(menu);
                    button->setPopupMode(QToolButton::MenuButtonPopup);
                } else {
                    button->setProperty(ACTION_CODE_KEY, "file-" + actionCode);
                }
                valueWidgetLayout->addWidget(button);
            }
        } else {
            auto valueLabel = new QLabel(parameter.value);
            valueLabel->setWordWrap(true);
            valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
            valueLabel->setStyleSheet("padding: 7px 10px;");
            valueWidgetLayout->addWidget(valueLabel);
        }
        valueWidgetLayout->addStretch();
        parametersLayout->addWidget(valueWidget, parameterIndex + 1, 1);
    }
}

void ParametersDashboardWidget::sl_openFileClicked() {
    QString actionCode = sender()->property(ACTION_CODE_KEY).toString();
    QStringList tokens = actionCode.split("-");
    CHECK(tokens.length() == 4, );

    bool ok;
    int workerIndex = tokens[1].toInt(&ok);
    CHECK(ok && workerIndex >= 0 && workerIndex < workers.size(), );

    const WorkerInfo &worker = workers[workerIndex];
    int parameterIndex = tokens[2].toInt(&ok);
    CHECK(ok && parameterIndex >= 0 && parameterIndex < worker.parameters.size(), );

    auto parameter = worker.parameters[parameterIndex];
    QStringList lines = parameter.value.split("\n");
    int lineIndex = tokens[3].toInt(&ok);
    CHECK(ok && lineIndex >= 0 && lineIndex < lines.size(), );
    QString url = lines[lineIndex];

    QFileInfo fileInfo(url);
    if (!fileInfo.isAbsolute()) {
        fileInfo = QFileInfo(QDir(dashboardDir), url);
    }
    if (tokens[0] == "dir") {
        fileInfo = QFileInfo(fileInfo.absolutePath());
    }
    if (!fileInfo.exists()) {
        QMessageBox::critical(QApplication::activeWindow(), L10N::errorTitle(), tr("File is not found: %1").arg(fileInfo.absoluteFilePath()));
        return;
    }
    if (tokens[0] == "ugene") {
        QVariantMap hints;
        hints[ProjectLoaderHint_OpenBySystemIfFormatDetectionFailed] = true;
        Task *task = AppContext::getProjectLoader()->openWithProjectTask(fileInfo.absoluteFilePath(), hints);
        CHECK(task != nullptr, );
        AppContext::getTaskScheduler()->registerTopLevelTask(task);
    } else {
        QString fullFilePath = "file://" + fileInfo.absoluteFilePath();
        QDesktopServices::openUrl(fullFilePath);
    }
}

bool ParametersDashboardWidget::isValidDom(const QDomElement &dom) {
    return !DomUtils::findChildElementByClass(dom, "params-nav-tabs", 2).isNull() &&
           !DomUtils::findChildElementByClass(dom, "params-tab-content", 2).isNull();
}

static QString getParameterTypeClass(const WorkerParameterInfo &parameter) {
    QString result = QString() + ((parameter.isUrl) ? URL_MARKER_CLASS : "") + " " + ((parameter.isDir) ? DIR_MARKER_CLASS : "") + " " + ((parameter.isDataset) ? DATASET_MARKER_CLASS : "");
    return result.isEmpty() ? QString() : " class=\"" + result.trimmed() + "\"";
}

QString ParametersDashboardWidget::toHtml() const {
    QString urlMarkerPart = QString(" class=\"%1\"").arg(URL_MARKER_CLASS);
    QString tabIdPrefix = "params_tab_id_";

    QString html = "<div class=\"tabbable tabs-left\">\n";

    // List with worker names.
    html += "<ul class=\"nav nav-tabs params-nav-tabs\">\n";
    for (int i = 0; i < workers.size(); i++) {
        auto worker = workers[i];
        html += "<li" + QString(i == 0 ? " class=\"active\"" : "") + "><a href=\"#" + tabIdPrefix + QString::number(i) + "\" data-toggle=\"tab\">";
        html += worker.workerName.toHtmlEscaped();
        html += "</a></li>\n";
    }
    html += "</ul>\n";

    // List with parameter tables.
    html += "<div class=\"tab-content params-tab-content\">\n";
    for (int i = 0; i < workers.size(); i++) {
        auto worker = workers[i];
        html += "<div class=\"" + QString(i == 0 ? "tab-pane active" : "tab-pane") + "\" id=\"" + tabIdPrefix + QString::number(i) + "\">\n";
        html += "<table class=\"table table-bordered table-fixed param-value-column\">\n";
        html += "<thead><tr><th>" + tr("Parameter").toHtmlEscaped() + "</th><th>" + tr("Value").toHtmlEscaped() + "</th></tr></thead>\n";
        html += "<tbody>\n";
        for (auto parameter : worker.parameters) {
            html += "<tr>";
            html += "<td>" + parameter.name.toHtmlEscaped() + "</td>";
            html += "<td" + getParameterTypeClass(parameter) + ">" + parameter.value.toHtmlEscaped() + "</td>";
            html += "</tr>\n";
        }
        html += "</tbody>\n";
        html += "</table>\n";
        html += "</div>\n";
    }
    html += "</div>\n";    // param tables.

    html += "</div>\n";
    return html;
}

WorkerInfo::WorkerInfo(const QString &workerName, const QList<WorkerParameterInfo> &parameters)
    : workerName(workerName), parameters(parameters) {
}

WorkerParameterInfo::WorkerParameterInfo(const QString &name, const QString &value, bool isUrl, bool isDir, bool isDataset)
    : name(name), value(value), isUrl(isUrl), isDir(isDir), isDataset(isDataset) {
}

PopupMenu::PopupMenu(QAbstractButton *button, QWidget *parent)
    : QMenu(parent), button(button) {
}

void PopupMenu::showEvent(QShowEvent *event) {
    QPoint position = this->pos();
    QRect rect = button->geometry();
    this->move(position.x() + rect.width() - this->geometry().width(), position.y());
}
}    // namespace U2
