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

#include <QApplication>
#include <QDesktopServices>
#include <QFile>
#include <QMessageBox>

#include <QtWebSockets/QWebSocketServer>
#include <QtWebChannel/QWebChannel>

#include <QDesktopServices>

#if (QT_VERSION < 0x050500) //Qt 5
#include <U2Gui/WebSocketClientWrapper.h>
#include <U2Gui/WebSocketTransport.h>
#endif // endif

#include <QClipboard>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Task.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/MainWindow.h>

#include <U2Lang/WorkflowSettings.h>
#include <U2Lang/URLAttribute.h>
#include <U2Lang/URLContainer.h>
#include <U2Lang/WorkflowUtils.h>

#include "Dashboard.h"

namespace U2 {

static const QString REPORT_SUB_DIR("report/");
static const QString DB_FILE_NAME("dashboard.html");
static const QString SETTINGS_FILE_NAME("settings.ini");
static const QString OPENED_SETTING("opened");
static const QString NAME_SETTING("name");

/************************************************************************/
/* Dashboard */
/************************************************************************/
const QString Dashboard::STATUS_WIDGET_ID = "statusWidget";
const QString Dashboard::OUTPUT_WIDGET_ID = "outputWidget";
const QString Dashboard::STATISTICS_WIDGET_ID = "statisticsWidget";
const QString Dashboard::PROBLEMS_WIDGET_ID = "problemsWidget";
const QString Dashboard::PARAMETERS_WIDGET_ID = "parametersWidget";
const QString Dashboard::ETOOLS_WIDGET_ID = "externalToolsWidget";

Dashboard::Dashboard(const WorkflowMonitor *monitor, const QString &_name, QWidget *parent)
    : QWebEngineView(parent), loaded(false), name(_name), opened(true), _monitor(monitor), initialized(false), workflowInProgress(true)
{
    connect(this, SIGNAL(loadFinished(bool)), SLOT(sl_loaded(bool)));
    connect(_monitor, SIGNAL(si_report()), SLOT(sl_serialize()));
    connect(_monitor, SIGNAL(si_dirSet(const QString &)), SLOT(sl_setDirectory(const QString &)));
    connect(_monitor, SIGNAL(si_taskStateChanged(Monitor::TaskState)), SLOT(sl_workflowStateChanged(Monitor::TaskState)));

    dashboardPageController = new DashboardPageController(this);

    setContextMenuPolicy(Qt::NoContextMenu);
#if (QT_VERSION < 0x050400) //Qt 5.7 TODO need recheck urls in resources
    loadUrl = ":U2Designer/html/Dashboard.html";
#else
    loadUrl = "qrc:///U2Designer/html/Dashboard_webengine.html";
#endif
    loadDocument();
    setObjectName("Dashboard");

    connect(_monitor, SIGNAL(si_progressChanged(int)), dashboardPageController, SIGNAL(si_progressChanged(int)));
    connect(_monitor, SIGNAL(si_taskStateChanged(Monitor::TaskState)), SLOT(sl_taskStateChanged(Monitor::TaskState)));
    connect(_monitor, SIGNAL(si_newProblem(Problem,int)), SLOT(sl_newProblem(Problem,int)));
    connect(_monitor, SIGNAL(si_workerInfoChanged(const QString &, const U2::Workflow::Monitor::WorkerInfo &)),
        SLOT(sl_workerInfoChanged(const QString &, const U2::Workflow::Monitor::WorkerInfo &)));
    connect(_monitor, SIGNAL(si_updateProducers()), SLOT(sl_workerStatsUpdate()));
    connect(_monitor, SIGNAL(si_newOutputFile(const U2::Workflow::Monitor::FileInfo &)),
        SLOT(sl_newOutputFile(const U2::Workflow::Monitor::FileInfo &)));
    connect(_monitor, SIGNAL(si_logChanged(U2::Workflow::Monitor::LogEntry)), SLOT(sl_onLogChanged(U2::Workflow::Monitor::LogEntry)));
}

Dashboard::Dashboard(const QString &dirPath, QWidget *parent)
    : QWebEngineView(parent), loaded(false), dir(dirPath), opened(true), _monitor(NULL), initialized(false), workflowInProgress(false)
{
    dashboardPageController = new DashboardPageController(this);
    connect(this, SIGNAL(loadFinished(bool)), SLOT(sl_loaded(bool)));
    setContextMenuPolicy(Qt::NoContextMenu);
    loadUrl = dir + REPORT_SUB_DIR + DB_FILE_NAME;
    loadSettings();
    saveSettings();
}

Dashboard::~Dashboard() {
}

void Dashboard::onShow() {
    CHECK(!loaded, );
    loadDocument();
}

void Dashboard::sl_setDirectory(const QString &value) {
    dir = value;
    U2OpStatus2Log os;
    saveSettings();
}

void Dashboard::sl_workflowStateChanged(Monitor::TaskState state) {
    workflowInProgress = (state == Monitor::RUNNING) || (state == Monitor::RUNNING_WITH_PROBLEMS);
    if (!workflowInProgress) {
        emit si_workflowStateChanged(workflowInProgress);
    }
}

void Dashboard::setClosed() {
    opened = false;
    U2OpStatus2Log os;
    saveSettings();
}

QString Dashboard::directory() const {
    return dir;
}

QString Dashboard::getName() const {
    return name;
}

void Dashboard::setName(const QString &value) {
    name = value;
    saveSettings();
}

void Dashboard::loadDocument() {
    loaded = true;
    QFile file(loadUrl);
#if (QT_VERSION < 0x050500) //Qt 5.7 TODO: recheck local files loadUrl
    server = new QWebSocketServer(QStringLiteral("UGENE Standalone Server"), QWebSocketServer::NonSecureMode, this);
    if (!server->listen(QHostAddress::LocalHost, 12346)) {
        return;
    }

    clientWrapper = new WebSocketClientWrapper(server, this);

    channel = new QWebChannel(this);

    QObject::connect(clientWrapper, &WebSocketClientWrapper::clientConnected,
        channel, &QWebChannel::connectTo);

    channel->registerObject(QString("agent"), dashboardPageController);
#else
    QWebEnginePage *pg = new QWebEnginePage(parentWidget());
    QUrl abs;
    if(!loadUrl.startsWith("qrc")){
        abs = QUrl("file://"+loadUrl);
    }else{
        abs = QUrl(loadUrl);
    }
    pg->load(abs);
    setPage(pg);

    channel = new QWebChannel(page());
    page()->setWebChannel(channel);
    channel->registerObject(QString("agent"), dashboardPageController);
#endif
}

void Dashboard::sl_loaded(bool ok) {
    CHECK(!initialized, );
    SAFE_POINT(ok, "Loaded with errors", );
    initialized = true;
    if (NULL != monitor()) {
        page()->runJavaScript("parametersWidget = new ParametersWidget(\""+PARAMETERS_WIDGET_ID+"\")");
        page()->runJavaScript("outputWidget = new OutputFilesWidget(\""+OUTPUT_WIDGET_ID+"\");");
        page()->runJavaScript("statusWidget = new StatusWidget(\""+STATUS_WIDGET_ID+"\");");
        page()->runJavaScript("statisticsWidget = new StatisticsWidget(\""+STATISTICS_WIDGET_ID+"\");");
        sl_workerStatsUpdate();

        sl_runStateChanged(false);
        if (!monitor()->getProblems().isEmpty()) {//TODO fix adding problems widget on starting execution
            sl_addProblemsWidget();
        }

        createExternalToolTab();

        connect(monitor(), SIGNAL(si_runStateChanged(bool)), SLOT(sl_runStateChanged(bool)));
        connect(monitor(), SIGNAL(si_firstProblem()), SLOT(sl_addProblemsWidget()));
    }
    if (!WorkflowSettings::isShowLoadButtonHint()) {
        page()->runJavaScript("document.getElementById('wrapper').hideLoadBtnHint()");
    }
}

void Dashboard::sl_addProblemsWidget() {
    page()->runJavaScript("problemsWidget = new ProblemsWidget(\""+PROBLEMS_WIDGET_ID+"\");");
}

void Dashboard::sl_serialize() {
    QCoreApplication::processEvents();
    QString reportDir = dir + REPORT_SUB_DIR;
    QDir d(reportDir);
    if (!d.exists(reportDir)) {
        bool created = d.mkpath(reportDir);
        if (!created) {
            coreLog.error(tr("Can not create a directory: ") + reportDir);
            return;
        }
    }
    serialize();
    saveSettings();
}

void Dashboard::sl_taskStateChanged(TaskState state){
    QString stateStr;
    if (Monitor::RUNNING == state) {
        stateStr = "RUNNING";
    } else if (Monitor::RUNNING_WITH_PROBLEMS == state) {
        stateStr = "RUNNING_WITH_PROBLEMS";
    } else if (Monitor::FINISHED_WITH_PROBLEMS == state) {
        stateStr = "FINISHED_WITH_PROBLEMS";
    } else if (Monitor::FAILED == state) {
        stateStr = "FAILED";
    } else if (Monitor::SUCCESS == state) {
        stateStr = "SUCCESS";
    } else {
        stateStr = "CANCELED";
    }
    emit dashboardPageController->si_taskStateChanged(stateStr);
}

void Dashboard::sl_newProblem(const Problem &info, int count){
    QJsonObject infoJS;
    infoJS["actor"] = monitor()->actorName(info.actor);
    infoJS["type"] = info.type;
    infoJS["message"] = info.message;
    emit dashboardPageController->si_newProblem(infoJS, count);
}

void Dashboard::sl_workerInfoChanged(const QString &actorId, const WorkerInfo &info){
    QJsonObject infoJS;
    infoJS["actor"] = monitor()->actorName(actorId);
    infoJS["timeMks"] = info.timeMks;
    infoJS["countOfProducedData"] = monitor()->getDataProduced(actorId);
    emit dashboardPageController->si_workerStatsInfoChanged(infoJS);
}

void Dashboard::sl_workerStatsUpdate(){
    QJsonArray workersStatisticsInfo;
    QMap<QString, WorkerInfo> infos = monitor()->getWorkersInfo();
    foreach (const QString &actorId, infos.keys()) {
        WorkerInfo &info = infos[actorId];
        QJsonObject infoJS;
        infoJS["actor"] = monitor()->actorName(actorId);
        infoJS["timeMks"] = info.timeMks;
        infoJS["countOfProducedData"] = monitor()->getDataProduced(actorId);
        workersStatisticsInfo.append(infoJS);
    }
    emit dashboardPageController->si_workerStatsUpdate(workersStatisticsInfo);
}

void Dashboard::sl_newOutputFile(const U2::Workflow::Monitor::FileInfo &info){
    QJsonObject infoJS;
    infoJS["actor"] = monitor()->actorName(info.actor);
    infoJS["url"] = info.url;
    infoJS["openBySystem"] = info.openBySystem;
    emit dashboardPageController->si_newOutputFile(infoJS);
}
void Dashboard::sl_onLogChanged(U2::Workflow::Monitor::LogEntry entry){
    QJsonObject entryJS;
    entryJS["toolName"] = entry.toolName;
    entryJS["actorName"] = entry.actorName;
    entryJS["runNumber"] = entry.runNumber;
    entryJS["logType"] = entry.logType;
    entryJS["lastLine"] = entry.lastLine;
    emit dashboardPageController->si_onLogChanged(entryJS);
}
void Dashboard::serialize() {
    page()->toHtml([this](const QString &result) { return result; });
    connect(this, SIGNAL(si_serializeContent(const QString&)), this, SLOT(sl_serializeContent(const QString&)));
    page()->toHtml([this](const QString& result) mutable {emit si_serializeContent(result);});
}

void Dashboard::saveSettings() {
    QSettings s(dir + REPORT_SUB_DIR + SETTINGS_FILE_NAME, QSettings::IniFormat);
    s.setValue(OPENED_SETTING, opened);
    s.setValue(NAME_SETTING, name);
    s.sync();
}

void Dashboard::loadSettings() {
    QSettings s(dir + REPORT_SUB_DIR + SETTINGS_FILE_NAME, QSettings::IniFormat);
    opened = true;
    name = s.value(NAME_SETTING).toString();
}

void Dashboard::createExternalToolTab() {
    const WorkflowMonitor* mon = monitor();
    SAFE_POINT(mon, "Monitor is NULL", );

    foreach (const WorkerParamsInfo& info, mon->getWorkersParameters()) {
        SAFE_POINT(info.actor, "Actor is NULL", );
        const ActorPrototype* proto = info.actor->getProto();
        SAFE_POINT(proto, "Actor prototype is NULL", );

        if (!proto->getExternalTools().isEmpty()) {
            page()->runJavaScript("externalToolsWidget = externalToolsWidget || new ExternalToolsWidget(\""+ETOOLS_WIDGET_ID+"\");");
            break;
        }
    }
}

const WorkflowMonitor * Dashboard::monitor() {
    return _monitor;
}

void Dashboard::sl_runStateChanged(bool paused) {
    QString script = paused ? "pauseTimer()" : "startTimer()";
    page()->runJavaScript(script);
}

//void Dashboard::initiateHideLoadButtonHint() {
//    WorkflowSettings::setShowLoadButtonHint(false);
//    emit si_hideLoadBtnHint();
//}

bool Dashboard::isWorkflowInProgress() {
    return workflowInProgress;
}

void Dashboard::sl_hideLoadBtnHint() {
    page()->runJavaScript("hideLoadBtnHint()");
}

void Dashboard::sl_serializeContent(const QString& content) {
    
    QString fileName = dir + REPORT_SUB_DIR + DB_FILE_NAME;
    QFile file(fileName);
    bool opened = file.open(QIODevice::WriteOnly);
    if (!opened) {
        coreLog.error(tr("Can not open a file for writing: ") + fileName);
        return;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << content;
    stream.flush();
    file.close();
}

DashboardPageController::DashboardPageController(Dashboard* parent) : QObject(parent) {
    monitor = parent->monitor();
    //set language
    lang = AppContext::getAppSettings()->getUserAppsSettings()->getTranslationFile();
    if(lang.isEmpty()){
        lang = "en";
    }else {
        lang = lang.split("_")[1];
    }
    fillWorkerParamsInfo();
}

void DashboardPageController::sl_onJsError(const QString& errorMessage) {
    coreLog.error(errorMessage);
}
void DashboardPageController::sl_checkETsLog(){

}
void DashboardPageController::openUrl(const QString &relative) {
    QString url = absolute(relative);
    QVariantMap hints;
    hints[ProjectLoaderHint_OpenBySystemIfFormatDetectionFailed] = true;
    Task *t = AppContext::getProjectLoader()->openWithProjectTask(url, hints);
    if (t) {
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
    }
}

void DashboardPageController::openByOS(const QString &relative) {
    QString url = absolute(relative);
    if (!QFile::exists(url)) {
        QMessageBox::critical((QWidget*)AppContext::getMainWindow()->getQMainWindow(), tr("Error"), tr("The file does not exist"));
        return;
    }
    QDesktopServices::openUrl(QUrl("file:///" + url));
}

QString DashboardPageController::absolute(const QString &url) {
    if (QFileInfo(url).isAbsolute()) {
        return url;
    }
    return qobject_cast<Dashboard*>(parent())->directory() + url;
}


QString DashboardPageController::getLang(){
    return lang;
}
QJsonArray DashboardPageController::getWorkersParamsInfo(){
    return workersParamsInfo;
}
//Worker parametes initialization
void DashboardPageController::fillWorkerParamsInfo(){
    CHECK(monitor,)
    QList<WorkerParamsInfo> workersParamsList = monitor->getWorkersParameters();
    foreach (WorkerParamsInfo workerInfo, workersParamsList) {
        QJsonObject workerInfoJS;
        workerInfoJS["workerName"] = workerInfo.workerName;
        workerInfoJS["actor"] = workerInfo.actor->getLabel();
        QJsonArray parameters;
        foreach (Attribute* parameter, workerInfo.parameters) {
            QJsonObject parameterJS;
            parameterJS["name"] = parameter->getDisplayName();
            QVariant paramValueVariant = parameter->getAttributePureValue();
            if (paramValueVariant.canConvert< QList<Dataset> >()) {
                QList<Dataset> sets = paramValueVariant.value< QList<Dataset > >();
                foreach (const Dataset &set, sets) {
                    QString paramName = parameter->getDisplayName();
                    if (sets.size() > 1) {
                        paramName += ": <i>" + set.getName() + "</i>";
                    }
                    parameterJS["name"] = paramName;
                    QStringList urls;
                    foreach (URLContainer *c, set.getUrls()) {
                        urls << c->getUrl();
                    }
                    parameterJS["value"] = urls.join(";");
                    parameterJS["isDataset"] = true;
                    parameters.append(parameterJS);
                }
            }else{
                parameterJS["value"] = WorkflowUtils::getStringForParameterDisplayRole(paramValueVariant);
                UrlAttributeType type = WorkflowUtils::isUrlAttribute(parameter, workerInfo.actor);
                if (type == NotAnUrl || QString::compare(paramValueVariant.toString(), "default", Qt::CaseInsensitive) == 0) {
                    parameterJS["isUrl"] = false;
                }else{
                    parameterJS["isUrl"] = true;
                }
                parameters.append(parameterJS);
            }
        }
        workerInfoJS["parameters"] = parameters;
        workersParamsInfo.append(workerInfoJS);
    }
}
/************************************************************************/
/* DashboardWidget */
/************************************************************************/
DashboardWidget::DashboardWidget(const QString &_container, Dashboard *parent)
: QObject(parent), dashboard(parent), container(_container)
{

}

/************************************************************************/
/* LoadDashboardsTask */
/************************************************************************/
ScanDashboardsDirTask::ScanDashboardsDirTask()
: Task(tr("Scan dashboards directory"), TaskFlag_None)
{

}

void ScanDashboardsDirTask::run() {
    QDir outDir(WorkflowSettings::getWorkflowOutputDirectory());
    CHECK(outDir.exists(), );

    QFileInfoList dirs = outDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    foreach (const QFileInfo &info, dirs) {
        QString dirPath = info.absoluteFilePath() + "/";
        DashboardInfo dbi(dirPath);
        if (isDashboardDir(dirPath, dbi)) {
            result << dbi;
            if (dbi.opened) {
                openedDashboards << dirPath;
            }
        }
    }
}

bool ScanDashboardsDirTask::isDashboardDir(const QString &dirPath, DashboardInfo &info) {
    QDir dir(dirPath + REPORT_SUB_DIR);
    CHECK(dir.exists(), false);
    CHECK(dir.exists(DB_FILE_NAME), false);
    CHECK(dir.exists(SETTINGS_FILE_NAME), false);

    QSettings s(dirPath + REPORT_SUB_DIR + SETTINGS_FILE_NAME, QSettings::IniFormat);
    info.opened = s.value(OPENED_SETTING).toBool();
    info.name = s.value(NAME_SETTING).toString();
    return true;
}

QStringList ScanDashboardsDirTask::getOpenedDashboards() const {
    return openedDashboards;
}

QList<DashboardInfo> ScanDashboardsDirTask::getResult() const {
    return result;
}

/************************************************************************/
/* RemoveDashboardsTask */
/************************************************************************/
RemoveDashboardsTask::RemoveDashboardsTask(const QList<DashboardInfo> &_dashboards)
: Task(tr("Remove dashboards"), TaskFlag_None), dashboards(_dashboards)
{

}
void RemoveDashboardsTask::run() {
    foreach (const DashboardInfo &info, dashboards) {
        U2OpStatus2Log os;
        GUrlUtils::removeDir(info.path, os);
    }
}

/************************************************************************/
/* DashboardInfo */
/************************************************************************/
DashboardInfo::DashboardInfo() {

}

DashboardInfo::DashboardInfo(const QString &dirPath, bool _opened)
: path(dirPath), opened(_opened)
{
    dirName = QDir(path).dirName();
}

bool DashboardInfo::operator==(const DashboardInfo &other) const {
    return path == other.path;
}

} // ns

