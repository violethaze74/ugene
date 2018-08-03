/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2018 UniPro <ugene@unipro.ru>
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

#include <QJsonArray>

#if (QT_VERSION < 0x050500)
#include <QWebChannel>
#include <QWebEngineView>
#include <QWebSocketServer>

#include <U2Gui/WebSocketClientWrapper.h>
#else
#include <QWebChannel>
#include <QWebEngineView>
#endif

#include <U2Core/U2SafePoints.h>

#include <U2Lang/WorkflowMonitor.h>

namespace U2 {
using namespace Workflow;

class DashboardJsAgent;
class DashboardPageController;
class ExternalToolsWidgetController;

class DashboardWriter : public QObject {
    Q_OBJECT
public:
    DashboardWriter(const QString &dir);

    void setDir(const QString &dir);
    void write(const QString &content);

private:
    QString dir;
};

class U2DESIGNER_EXPORT Dashboard : public QWebEngineView {
    Q_OBJECT
    Q_DISABLE_COPY(Dashboard)
public:
    Dashboard(const WorkflowMonitor *monitor, const QString &name, QWidget *parent);
    Dashboard(const QString &dirPath, QWidget *parent);
    ~Dashboard();

    void onShow();

    const QPointer<const WorkflowMonitor> getMonitor() const;

    void setClosed();
    QString directory() const;

    QString getName() const;
    void setName(const QString &value);

    void loadSchema();

    /** Modifies the application settings and emits signal for all dashboards */
    void initiateHideLoadButtonHint();

    bool isWorkflowInProgress();

    DashboardPageController* getController();

signals:
    void si_loadSchema(const QString &url);
    void si_hideLoadBtnHint();
    void si_workflowStateChanged(bool isRunning);
    void si_serializeContent(const QString& content);

public slots:
    /** Hides the hint on the current dashboard instance */
    void sl_hideLoadBtnHint();

private slots:
    void sl_runStateChanged(bool paused);
    void sl_loaded(bool ok);
    void sl_serialize();
    void sl_setDirectory(const QString &dir);
    void sl_workflowStateChanged(U2::Workflow::Monitor::TaskState state);

private:
    bool loaded;
    QString loadUrl;
    QString name;
    QString dir;
    bool opened;
    const QPointer<const WorkflowMonitor> monitor;
    bool workflowInProgress;
    DashboardPageController* dashboardPageController;

    enum DashboardTab {OverviewDashTab, InputDashTab, OutputDashTab, ExternalToolsTab};

private:
    void loadDocument();
    void serialize();
    void saveSettings();
    void loadSettings();

#if (QT_VERSION < 0x050500)
    QWebSocketServer *server;
    WebSocketClientWrapper *clientWrapper;
    QWebChannel *channel;
    int port;
#else
    QWebChannel *channel;
#endif
    DashboardWriter *writer;
};

////////////////////////////////////////////////////////////////////////////////////////////
class DashboardWidget : public QObject {
    Q_OBJECT
public:
    DashboardWidget(const QString &container, Dashboard *parent);

protected:
    Dashboard *dashboard;
    QString container;
};

////////////////////////////////////////////////////////////////////////////////////////////
class U2DESIGNER_EXPORT DashboardInfo {
public:
    DashboardInfo();
    DashboardInfo(const QString &dirPath, bool opened = true);

    bool operator==(const DashboardInfo &other) const;

    QString path;
    QString dirName;
    QString name;
    bool opened;
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
class DashboardPageController : public QObject {
    Q_OBJECT
public:
    DashboardPageController(Dashboard* parent);

    DashboardJsAgent* getAgent();
    void setWebChannelInitialized();
    void setDataReady();
    bool isInitialized() const;

signals:
    void si_initialized();

public slots:
    void sl_pageLoaded();
    void sl_progressChanged(int progress);
    void sl_taskStateChanged(U2::Workflow::Monitor::TaskState state);
    void sl_newNotification(const WorkflowNotification &info, int count);
    void sl_workerInfoChanged(const QString &actorId, const U2::Workflow::Monitor::WorkerInfo &info);
    void sl_workerStatsUpdate();
    void sl_onLogChanged(U2::Workflow::Monitor::LogEntry entry);
    void sl_newOutputFile(const U2::Workflow::Monitor::FileInfo &info);

private:
    void initData();

    int progress;
    QString state;
    QStringList problems;
    QStringList infos;
    QStringList workersStatisticsInfos;
    QList<QJsonArray> extToolsLogs;
    QStringList logEntries;
    QStringList fileInfos;

    bool isPageLoaded;
    bool isWebChannelInitialized;
    bool isDataReady;
    DashboardJsAgent* agent;
    const QPointer<const WorkflowMonitor> monitor;

    int logEntriesQuantity;
    bool isUserWarned;

    static const int LOG_LIMIT;    // UGENE-6160: dashboard hangs if there are too many messages, limit them.
};

////////////////////////////////////////////////////////////////////////////////////////////
class DashboardJsAgent : public QObject {
    Q_OBJECT
public:
    DashboardJsAgent(Dashboard* parent);

    Q_PROPERTY(QString lang READ getLang CONSTANT)
    Q_PROPERTY(QString workersParamsInfo READ getWorkersParamsInfo CONSTANT)
    Q_PROPERTY(bool showHint READ getShowHint CONSTANT)

public slots:
    void sl_onJsError(const QString& errorMessage);
    void sl_checkETsLog();
    void sl_webChannelInitialized();

    void openUrl(const QString &url);
    void openByOS(const QString &url);
    QString absolute(const QString &url);
    void loadSchema();
    void setClipboardText(const QString &text);
    void hideLoadButtonHint();

signals:
    void si_progressChanged(int progress);
    void si_taskStateChanged(QString state);
    void si_newProblem(QString problem); //workaround for Qt5.4 and Qt5.5, sould be simple QJsonObject. More info see https://bugreports.qt.io/browse/QTBUG-48198
    void si_workerStatsInfoChanged(QString info); //workaround for Qt5.4 and Qt5.5, sould be simple QJsonObject. More info see https://bugreports.qt.io/browse/QTBUG-48198
    void si_workerStatsUpdate(QString workersStatisticsInfo);//workaround for Qt5.4, sould be simple QJsonArray.
    void si_onLogChanged(QString logEntry); //workaround for Qt5.4 and Qt5.5, sould be simple QJsonObject. More info see https://bugreports.qt.io/browse/QTBUG-48198
    void si_newOutputFile(QString fileInfo); //workaround for Qt5.4 and Qt5.5, sould be simple QJsonObject. More info see https://bugreports.qt.io/browse/QTBUG-48198
    void si_createOutputWidget();

private:
    Q_INVOKABLE QString getLang();
    Q_INVOKABLE QString getWorkersParamsInfo();
    Q_INVOKABLE bool getShowHint();
    void fillWorkerParamsInfo();

    QString lang;
    QString workersParamsInfo;
    const QPointer<const WorkflowMonitor> monitor;
};

} // U2

Q_DECLARE_METATYPE(U2::DashboardInfo)

#endif // _U2_DASHBOARD_H_
