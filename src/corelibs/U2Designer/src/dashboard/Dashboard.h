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

#ifndef _U2_DASHBOARD_H_
#define _U2_DASHBOARD_H_

#include <qglobal.h>

#if (QT_VERSION < 0x050400) //Qt 5.7
#include <QWebElement>
#include <QWebView>
#elif (QT_VERSION < 0x050500) //Qt 5.7
#include <QWebSocketServer>
#include <U2Gui/WebSocketClientWrapper.h>
#include <QWebEngineView>
#include <QtWebChannel/QWebChannel>
#else
#include <QWebEngineView>
#include <QWebChannel>
#endif

#include <U2Core/U2SafePoints.h>

#include <U2Lang/WorkflowMonitor.h>

//class QWebSocketServer;
//class WebSocketClientWrapper;


namespace U2 {
using namespace Workflow;

class ExternalToolsWidgetController;
class DashboardPageController;

#if (QT_VERSION < 0x050400) //Qt 5.7
class U2DESIGNER_EXPORT Dashboard : public QWebView {
#else
class U2DESIGNER_EXPORT Dashboard : public QWebEngineView {
#endif
    Q_OBJECT
    Q_DISABLE_COPY(Dashboard)
public:
    Dashboard(const WorkflowMonitor *monitor, const QString &name, QWidget *parent);
    Dashboard(const QString &dirPath, QWidget *parent);
    ~Dashboard();

    void onShow();

    const WorkflowMonitor * monitor();
#if (QT_VERSION < 0x050400) //Qt 5.7
    QWebElement getDocument();
#endif
    
    void setClosed();
    QString directory() const;

    QString getName() const;
    void setName(const QString &value);

    void loadSchema();

    /** Modifies the application settings and emits signal for all dashboards */
    void initiateHideLoadButtonHint();

    bool isWorkflowInProgress();

signals:
    void si_loadSchema(const QString &url);
    void si_hideLoadBtnHint();
    void si_workflowStateChanged(bool isRunning);
#if (QT_VERSION >= 0x050400) //Qt 5.7
    void si_serializeContent(const QString& content);
#endif

public slots:
    /** Hides the hint on the current dashboard instance */
    void sl_hideLoadBtnHint();

private slots:
    void sl_runStateChanged(bool paused);
    void sl_loaded(bool ok);
    void sl_addProblemsWidget();
    void sl_serialize();
    void sl_setDirectory(const QString &dir);
    void sl_workflowStateChanged(Monitor::TaskState state);
#if (QT_VERSION >= 0x050400) //Qt 5.7
    void sl_serializeContent(const QString& content);
#endif
    void sl_taskStateChanged(Monitor::TaskState);
    void sl_newProblem(const Problem &info, int count);
    void sl_workerInfoChanged(const QString &actorId, const U2::Workflow::Monitor::WorkerInfo &info);
    void sl_workerStatsUpdate();
    void sl_newOutputFile(const U2::Workflow::Monitor::FileInfo &info);
    void sl_onLogUpdate();
    void sl_onLogChanged(U2::Workflow::Monitor::LogEntry entry);

private:
    bool loaded;
    QString loadUrl;
    QString name;
    QString dir;
    bool opened;
    const WorkflowMonitor *_monitor;
#if (QT_VERSION < 0x050400) //Qt 5.7
    QWebElement doc;
#endif
    bool initialized;
    bool workflowInProgress;
    ExternalToolsWidgetController* etWidgetController;
    DashboardPageController* dashboardPageController;

    enum DashboardTab {OverviewDashTab, InputDashTab, OutputDashTab, ExternalToolsTab};

    static const QString EXT_TOOLS_TAB_ID;
    static const QString OVERVIEW_TAB_ID;
    static const QString INPUT_TAB_ID;
    //static const QString OUTPUT_TAB_ID;
#if (QT_VERSION >= 0x050400) //Qt 5.7
    static const QString RESOURCE_WIDGET_ID;
    static const QString OUTPUT_WIDGET_ID;
    static const QString STATISTICS_WIDGET_ID;
    static const QString PROBLEMS_WIDGET_ID;
    static const QString PARAMETERS_WIDGET_ID;
#endif
private:
    void loadDocument();
    /** Returns the content area of the widget */
#if (QT_VERSION < 0x050400) //Qt 5.7
    QWebElement addWidget(const QString &title, DashboardTab dashTab, int cntNum = -1);

    /** Returns size of the QWebElement "name", it is searched inside "insideElt" only*/
    int containerSize(const QWebElement &insideElt, const QString &name);
#else
    void addWidget(const QString &title, DashboardTab dashTab, int cntNum = -1, const QString &widgetId = "");
#endif
    void serialize(U2OpStatus &os);
    void saveSettings();
    void loadSettings();

    void createExternalToolTab();
#if (QT_VERSION >= 0x050500) //Qt 5.7
    QWebChannel *channel;
#elif (QT_VERSION >= 0x050400) //Qt 5.7
    QWebSocketServer *server;
    WebSocketClientWrapper *clientWrapper;
    QWebChannel *channel;
#endif
};
////////////////////////////////////////////////////////////////////////////////////////////
class DashboardWidget : public QObject {
    Q_OBJECT
public:
#if (QT_VERSION < 0x050400) //Qt 5.7
    DashboardWidget(const QWebElement &container, Dashboard *parent);
#else
    DashboardWidget(const QString &container, Dashboard *parent);
#endif
protected:
    Dashboard *dashboard;
#if (QT_VERSION < 0x050400) //Qt 5.7
    QWebElement container;
#else
    QString container;
#endif
};
////////////////////////////////////////////////////////////////////////////////////////////
class JavascriptAgent : public QObject {
    Q_OBJECT
public:
    JavascriptAgent(Dashboard *dashboard);

public slots:
    void loadSchema();
    void hideLoadButtonHint();
    void setClipboardText(const QString &text);

private:
    Dashboard *dashboard;
};
////////////////////////////////////////////////////////////////////////////////////////////
class U2DESIGNER_EXPORT DashboardInfo {
public:
    QString path;
    QString dirName;
    QString name;
    bool opened;

public:
    DashboardInfo();
    DashboardInfo(const QString &dirPath, bool opened = true);
    bool operator==(const DashboardInfo &other) const;
};
////////////////////////////////////////////////////////////////////////////////////////////
class U2DESIGNER_EXPORT ScanDashboardsDirTask : public Task {
    Q_OBJECT
public:
    ScanDashboardsDirTask();
    void run();

    QStringList getOpenedDashboards() const;
    QList<DashboardInfo> getResult() const;

private:
    bool isDashboardDir(const QString &dirPath, DashboardInfo &info);

private:
    QStringList openedDashboards;
    QList<DashboardInfo> result;
};
////////////////////////////////////////////////////////////////////////////////////////////
class U2DESIGNER_EXPORT RemoveDashboardsTask : public Task {
    Q_OBJECT
public:
    RemoveDashboardsTask(const QList<DashboardInfo> &dashboards);
    void run();

private:
    QList<DashboardInfo> dashboards;
};
////////////////////////////////////////////////////////////////////////////////////////////
using namespace Workflow::Monitor;
class DashboardPageController : public QObject {
    Q_OBJECT
public:
    DashboardPageController(Dashboard* parent);

    Q_PROPERTY(QString lang READ getLang)
    Q_PROPERTY(QJsonArray workersParamsInfo READ getWorkersParamsInfo)

public slots:
    void sl_onJsError(const QString& errorMessage);
    void sl_checkETsLog();
    void openUrl(const QString &url);
    void openByOS(const QString &url);
    QString absolute(const QString &url);
signals:
    void si_progressChanged(int progress);
    void si_taskStateChanged(QString state);
    void si_newProblem(QJsonObject problem, int count);
    void si_workerStatsInfoChanged(QJsonObject info);
    void si_workerStatsUpdate(QJsonArray workersStatisticsInfo);
    void si_onLogUpdate(QJsonArray extToolsLog);
    void si_onLogChanged(QJsonObject logEntry);
    void si_newOutputFile(QJsonObject fileInfo);
private:
    Q_INVOKABLE QString getLang();
    Q_INVOKABLE QJsonArray getWorkersParamsInfo();
    void fillWorkerParamsInfo();

    QString lang;
    QJsonArray workersParamsInfo;
    const WorkflowMonitor* monitor;
};

} // U2
Q_DECLARE_METATYPE(U2::DashboardInfo)

#endif // _U2_DASHBOARD_H_
