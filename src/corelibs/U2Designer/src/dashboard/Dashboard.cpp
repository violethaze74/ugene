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

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QSettings>
#include <QWebChannel>
#include <QWebSocketServer>

#if (QT_VERSION < 0x050500) //Qt 5
#include <U2Gui/WebSocketClientWrapper.h>
#include <U2Gui/WebSocketTransport.h>
#endif // endif

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Task.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Gui/MainWindow.h>

#include <U2Lang/URLAttribute.h>
#include <U2Lang/URLContainer.h>
#include <U2Lang/WorkflowSettings.h>
#include <U2Lang/WorkflowUtils.h>

#include "Dashboard.h"

namespace U2 {

static const QString REPORT_SUB_DIR("report/");
static const QString DB_FILE_NAME("dashboard.html");
static const QString SETTINGS_FILE_NAME("settings.ini");
static const QString OPENED_SETTING("opened");
static const QString NAME_SETTING("name");

const QString STATE_RUNNING = "RUNNING";
const QString STATE_RUNNING_WITH_PROBLEMS = "RUNNING_WITH_PROBLEMS";
const QString STATE_FINISHED_WITH_PROBLEMS = "FINISHED_WITH_PROBLEMS";
const QString STATE_FAILED = "FAILED";
const QString STATE_SUCCESS = "SUCCESS";
const QString STATE_CANCELED = "CANCELED";

DashboardWriter::DashboardWriter(const QString &dir)
    : QObject(NULL),
      dir(dir)
{

}

void DashboardWriter::setDir(const QString &newDir) {
    dir = newDir;
}

void DashboardWriter::write(const QString &content) {
    CHECK(!content.isEmpty(), );

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

/************************************************************************/
/* Dashboard */
/************************************************************************/
Dashboard::Dashboard(const WorkflowMonitor *monitor, const QString &name, QWidget *parent)
    : QWebEngineView(parent),
      loaded(false),
      name(name),
      opened(true),
      monitor(monitor),
      workflowInProgress(true),
      writer(new DashboardWriter(dir))
{
    connect(this, SIGNAL(loadFinished(bool)), SLOT(sl_loaded(bool)));
    connect(this, SIGNAL(loadFinished(bool)), SLOT(sl_serialize()));
    connect(monitor, SIGNAL(si_report()), SLOT(sl_serialize()));
    connect(monitor, SIGNAL(si_dirSet(const QString &)), SLOT(sl_setDirectory(const QString &)));
    connect(monitor, SIGNAL(si_taskStateChanged(Monitor::TaskState)), SLOT(sl_workflowStateChanged(Monitor::TaskState)));

    dashboardPageController = new DashboardPageController(this);
    connect(monitor, SIGNAL(si_progressChanged(int)), dashboardPageController, SLOT(sl_progressChanged(int)));
    connect(monitor, SIGNAL(si_taskStateChanged(U2::Workflow::Monitor::TaskState)), dashboardPageController, SLOT(sl_taskStateChanged(U2::Workflow::Monitor::TaskState)));
    connect(monitor, SIGNAL(si_workerInfoChanged(const QString &, const U2::Workflow::Monitor::WorkerInfo &)),
        dashboardPageController, SLOT(sl_workerInfoChanged(const QString &, const U2::Workflow::Monitor::WorkerInfo &)));
    connect(monitor, SIGNAL(si_updateProducers()), dashboardPageController, SLOT(sl_workerStatsUpdate()));
    connect(monitor, SIGNAL(si_newOutputFile(const U2::Workflow::Monitor::FileInfo &)),
        dashboardPageController, SLOT(sl_newOutputFile(const U2::Workflow::Monitor::FileInfo &)));
    connect(monitor, SIGNAL(si_logChanged(U2::Workflow::Monitor::LogEntry)), dashboardPageController, SLOT(sl_onLogChanged(U2::Workflow::Monitor::LogEntry)));
    connect(dashboardPageController, SIGNAL(si_initialized()), SLOT(sl_serialize()));

    setContextMenuPolicy(Qt::NoContextMenu);
    loadUrl = "qrc:///U2Designer/html/Dashboard_webengine.html";
    loadDocument();
    setObjectName("Dashboard");
}

Dashboard::Dashboard(const QString &dirPath, QWidget *parent)
    : QWebEngineView(parent),
      loaded(false),
      dir(dirPath),
      opened(true),
      monitor(NULL),
      workflowInProgress(false),
      writer(new DashboardWriter(dir))
{
    dashboardPageController = new DashboardPageController(this);
    connect(this, SIGNAL(loadFinished(bool)), SLOT(sl_loaded(bool)));
    setContextMenuPolicy(Qt::NoContextMenu);
    loadUrl = dir + REPORT_SUB_DIR + DB_FILE_NAME;
    loadSettings();
    saveSettings();
}

Dashboard::~Dashboard() {
    writer->deleteLater();
}

void Dashboard::onShow() {
    CHECK(!loaded, );
    loadDocument();
}

void Dashboard::sl_setDirectory(const QString &value) {
    dir = value;
    writer->setDir(dir);
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
#if (QT_VERSION < 0x050500) // Qt 5.7 TODO: recheck local files loadUrl
    server = new QWebSocketServer(QStringLiteral("UGENE Standalone Server"), QWebSocketServer::NonSecureMode, this);
    port = 12346;
    while (!server->listen(QHostAddress::LocalHost, port)) {//TODO: need more useful solution
        port++;
    }

    clientWrapper = new WebSocketClientWrapper(server, this);

    QWebEnginePage *pg = new QWebEnginePage(parentWidget());
    QUrl abs;
    if(!loadUrl.startsWith("qrc")){
        abs = QUrl("file://"+loadUrl);
    }else{
        abs = QUrl(loadUrl);
    }
    pg->load(abs);
    setPage(pg);

    channel = new QWebChannel(this);

    QObject::connect(clientWrapper, &WebSocketClientWrapper::clientConnected,
        channel, &QWebChannel::connectTo);

    channel->registerObject(QString("agent"), dashboardPageController->getAgent());

#else
    QWebEnginePage *pg = new QWebEnginePage(parentWidget());
    QUrl abs;
    if(!loadUrl.startsWith("qrc")){
        abs = QUrl("file://"+loadUrl);
    }else{
        abs = QUrl(loadUrl);
    }
    setPage(pg);
    pg->load(abs);

    channel = new QWebChannel(page());
    page()->setWebChannel(channel);
    channel->registerObject(QString("agent"), dashboardPageController->getAgent());
#endif
}

void Dashboard::sl_loaded(bool ok) {
    CHECK(!dashboardPageController->isInitialized(), );
    SAFE_POINT(ok, "Loaded with errors", );

#if (QT_VERSION < 0x050500)
    page()->runJavaScript("installWebChannel(true," + QString((NULL != getMonitor()) ? "true" : "false") + "," + QString::number(port) + ")");
#else
    page()->runJavaScript("installWebChannel(false," + QString((NULL != getMonitor()) ? "true" : "false") + ")");
#endif

    if (NULL != getMonitor()) {
        connect(getMonitor(), SIGNAL(si_newNotification(WorkflowNotification, int)), dashboardPageController, SLOT(sl_newNotification(WorkflowNotification, int)));
        if (!getMonitor()->getNotifications().isEmpty()) {
            foreach (const WorkflowNotification &notification, getMonitor()->getNotifications()) {
                dashboardPageController->sl_newNotification(notification, 0); // TODO: fix count of problems
            }
        }

        connect(getMonitor(), SIGNAL(si_runStateChanged(bool)), SLOT(sl_runStateChanged(bool)));
        dashboardPageController->setDataReady();
        dashboardPageController->sl_pageLoaded();
    }

    if (!WorkflowSettings::isShowLoadButtonHint()) {
        page()->runJavaScript("hideLoadBtnHint()");
    }
}

void Dashboard::sl_serialize() {
    CHECK(dashboardPageController->isInitialized() && loaded, );
    QCoreApplication::processEvents();
    QString reportDir = dir + REPORT_SUB_DIR;
    QDir d(reportDir);
    if (!d.exists(reportDir)) {
        bool created = d.mkpath(reportDir);
        if (!created) {
            coreLog.error(tr("Can not create a folder: ") + reportDir);
            return;
        }
    }
    serialize();
    saveSettings();
}

void Dashboard::serialize() {
    page()->toHtml([this](const QString& result) mutable {writer->write(result);});
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

const QPointer<const WorkflowMonitor> Dashboard::getMonitor() const {
    return monitor;
}

void Dashboard::sl_runStateChanged(bool paused) {
    QString script = paused ? "pauseTimer()" : "startTimer()";
    page()->runJavaScript(script);
}

void Dashboard::loadSchema() {
    QString url = dir + REPORT_SUB_DIR + WorkflowMonitor::WORKFLOW_FILE_NAME;
    emit si_loadSchema(url);
}

void Dashboard::initiateHideLoadButtonHint() {
    WorkflowSettings::setShowLoadButtonHint(false);
    emit si_hideLoadBtnHint();
}

bool Dashboard::isWorkflowInProgress() {
    return workflowInProgress;
}

void Dashboard::sl_hideLoadBtnHint() {
    page()->runJavaScript("hideLoadBtnHint()");
}

DashboardPageController* Dashboard::getController(){
    return dashboardPageController;
}

/************************************************************************/
/* DashboardPageController */
/************************************************************************/
const int DashboardPageController::LOG_LIMIT = 100;

DashboardPageController::DashboardPageController(Dashboard *parent)
    : QObject(parent),
      progress(0),
      state(STATE_RUNNING),
      isPageLoaded(false),
      isWebChannelInitialized(false),
      isDataReady(false),
      agent(new DashboardJsAgent(parent)),
      monitor(parent->getMonitor()),
      logEntriesQuantity(0),
      isUserWarned(false)
{

}

void DashboardPageController::sl_progressChanged(int newProgress){
    progress = newProgress;
    if(isPageLoaded){
        emit agent->si_progressChanged(progress);
    }
}

namespace {

QString state2String(Monitor::TaskState state) {
    switch (state) {
    case Monitor::RUNNING:
        return STATE_RUNNING;
    case Monitor::RUNNING_WITH_PROBLEMS:
        return STATE_RUNNING_WITH_PROBLEMS;
    case Monitor::FINISHED_WITH_PROBLEMS:
        return STATE_FINISHED_WITH_PROBLEMS;
    case Monitor::FAILED:
        return STATE_FAILED;
    case Monitor::SUCCESS:
        return STATE_SUCCESS;
    default:
        return STATE_CANCELED;
    }
}

}

void DashboardPageController::sl_taskStateChanged(U2::Workflow::Monitor::TaskState newState){
    state = state2String(newState);
    if (isPageLoaded) {
        emit agent->si_taskStateChanged(state);
    }
}

void DashboardPageController::sl_newNotification(const WorkflowNotification &info, int count){
    SAFE_POINT(NULL != monitor, "WorkflowMonitor is NULL", );
    QJsonObject infoJS;
    infoJS["actorId"] = info.actorId;
    infoJS["actorName"] = monitor->actorName(info.actorId);
    infoJS["type"] = info.type;
    infoJS["message"] = info.message;
    infoJS["count"] = count;
    QJsonDocument doc(infoJS);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    if (isPageLoaded) {
        emit agent->si_newProblem(strJson);
    } else {
        problems.append(strJson);
    }
}

void DashboardPageController::sl_workerInfoChanged(const QString &actorId, const U2::Workflow::Monitor::WorkerInfo &info) {
    SAFE_POINT(NULL != monitor, "WorkflowMonitor is NULL", );
    QJsonObject infoJS;
    infoJS["actorId"] = actorId;
    infoJS["actor"] = monitor->actorName(actorId);
    infoJS["timeMks"] = info.timeMks;
    infoJS["countOfProducedData"] = monitor->getDataProduced(actorId);
    QJsonDocument doc(infoJS);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    if (isPageLoaded) {
        emit agent->si_workerStatsInfoChanged(strJson);
    } else {
        infos.append(strJson);
    }
}

void DashboardPageController::sl_workerStatsUpdate() {
    SAFE_POINT(NULL != monitor, "WorkflowMonitor is NULL", );
    QJsonArray workersStatisticsInfo;
    QMap<QString, Monitor::WorkerInfo> infos = monitor->getWorkersInfo();
    foreach (const QString &actorId, infos.keys()) {
        Monitor::WorkerInfo &info = infos[actorId];
        QJsonObject infoJS;
        infoJS["actorId"] = actorId;
        infoJS["actor"] = monitor->actorName(actorId);
        infoJS["timeMks"] = info.timeMks;
        infoJS["countOfProducedData"] = monitor->getDataProduced(actorId);
        workersStatisticsInfo.append(infoJS);
    }

    QJsonDocument doc(workersStatisticsInfo);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    if (isPageLoaded) {
        emit agent->si_workerStatsUpdate(strJson);
    } else {
        workersStatisticsInfos.append(strJson);
    }
}

void DashboardPageController::sl_newOutputFile(const U2::Workflow::Monitor::FileInfo &info) {
    SAFE_POINT(NULL != monitor, "WorkflowMonitor is NULL", );
    QJsonObject infoJS;
    infoJS["actor"] = monitor->actorName(info.actor);
    infoJS["url"] = info.url;
    infoJS["openBySystem"] = info.openBySystem;
    QJsonDocument doc(infoJS);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    if (isPageLoaded) {
        emit agent->si_newOutputFile(strJson);
    } else {
        fileInfos.append(strJson);
    }
}


namespace {

bool logFileContainsMessages(const QString &fileUrl) {
    return QFileInfo(fileUrl).exists() && QFile(fileUrl).size() != 0;
}

}

void DashboardPageController::sl_onLogChanged(U2::Workflow::Monitor::LogEntry entry) {
    SAFE_POINT(NULL != monitor, "WorkflowMonitor is NULL", );
    CHECK(!isUserWarned, );
    if (logEntriesQuantity >= LOG_LIMIT) {
        const QString logDirUrl = monitor->outputDir() + "logs";
        QString logFileUrl = logDirUrl + "/" + WDListener::getStandardErrorLogFileUrl(entry.actorName, entry.runNumber);
        if (!logFileContainsMessages(logFileUrl)) {
            logFileUrl = logDirUrl + "/" + WDListener::getStandardOutputLogFileUrl(entry.actorName, entry.runNumber);
            if (!logFileContainsMessages(logFileUrl)) {
                logFileUrl = "";
            }
        }

        if (!logFileUrl.isEmpty()) {
            entry.lastLine = tr("\n\nThe external tools output is too large and can't be visualized on the dashboard. Find full output in file \"%1\".").arg(logFileUrl);
        } else {
            entry.lastLine = tr("\n\nThe external tools output is too large and can't be visualized on the dashboard.");
        }

        isUserWarned = true;
    }

    QJsonObject entryJS;
    entryJS["toolName"] = entry.toolName;
    entryJS["actorName"] = entry.actorName;
    entryJS["runNumber"] = entry.runNumber;
    entryJS["logType"] = entry.logType;
    entryJS["lastLine"] = entry.lastLine;
    QJsonDocument doc(entryJS);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    if (isPageLoaded) {
        emit agent->si_onLogChanged(strJson);
        logEntriesQuantity++;
    } else {
        logEntries.append(strJson);
    }
}

void DashboardPageController::sl_pageLoaded(){
    if (isWebChannelInitialized && isDataReady) {
        initData();
    }
}

void DashboardPageController::initData(){
    emit agent->si_progressChanged(progress);
    emit agent->si_taskStateChanged(state);

    foreach (const QString &problem, problems) {
        emit agent->si_newProblem(problem);
    }

    foreach (const QString &info, infos) {
        emit agent->si_workerStatsInfoChanged(info);
    }

    foreach (const QString &workersStatisticsInfo, workersStatisticsInfos) {
        emit agent->si_workerStatsUpdate(workersStatisticsInfo);
    }

    foreach (const QString &fileInfo, fileInfos) {
        emit agent->si_newOutputFile(fileInfo);
    }

    foreach (const QString &entry, logEntries) {
        emit agent->si_onLogChanged(entry);
    }

    isPageLoaded = true;
    emit si_initialized();
}

DashboardJsAgent* DashboardPageController::getAgent() {
    return agent;
}

void DashboardPageController::setWebChannelInitialized() {
    isWebChannelInitialized = true;
}

void DashboardPageController::setDataReady() {
    isDataReady = true;
}

bool DashboardPageController::isInitialized() const {
    return isPageLoaded;
}

/************************************************************************/
/* DashboardJsAgent */
/************************************************************************/
DashboardJsAgent::DashboardJsAgent(Dashboard* parent)
    : QObject(parent),
      monitor(parent->getMonitor())
{
    //set language
    lang = AppContext::getAppSettings()->getUserAppsSettings()->getTranslationFile();
    if (lang.isEmpty()) {
        lang = "en";
    } else {
        lang = lang.split("_")[1];
    }
    fillWorkerParamsInfo();
}

void DashboardJsAgent::sl_onJsError(const QString& errorMessage) {
    coreLog.error(errorMessage);
}

void DashboardJsAgent::sl_checkETsLog() {

}

void DashboardJsAgent::sl_webChannelInitialized() {
    Dashboard* dashboard = qobject_cast<Dashboard*>(parent());
    if (dashboard->getMonitor() != NULL) {
        dashboard->getController()->setWebChannelInitialized();
        dashboard->getController()->sl_pageLoaded();
    }
}

void DashboardJsAgent::openUrl(const QString &relative) {
    QString url = absolute(relative);
    QVariantMap hints;
    hints[ProjectLoaderHint_OpenBySystemIfFormatDetectionFailed] = true;
    Task *t = AppContext::getProjectLoader()->openWithProjectTask(url, hints);
    if (t) {
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
    }
}

void DashboardJsAgent::openByOS(const QString &relative) {
    QString url = absolute(relative);
    if (!QFile::exists(url)) {
        QMessageBox::critical((QWidget*)AppContext::getMainWindow()->getQMainWindow(), tr("Error"), tr("The file does not exist"));
        return;
    }
    QDesktopServices::openUrl(QUrl("file:///" + url));
}

QString DashboardJsAgent::absolute(const QString &url) {
    if (QFileInfo(url).isAbsolute()) {
        return url;
    }
    return qobject_cast<Dashboard*>(parent())->directory() + url;
}

void DashboardJsAgent::loadSchema(){
    qobject_cast<Dashboard*>(parent())->loadSchema();
}

void DashboardJsAgent::setClipboardText(const QString &text){
    QApplication::clipboard()->setText(text);
}

void DashboardJsAgent::hideLoadButtonHint() {
    Dashboard* dashboard = qobject_cast<Dashboard*>(parent());
    SAFE_POINT(NULL != dashboard, "NULL dashboard!", );
    dashboard->initiateHideLoadButtonHint();
}

QString DashboardJsAgent::getLang(){
    return lang;
}

QString DashboardJsAgent::getWorkersParamsInfo(){
    return workersParamsInfo;
}

bool DashboardJsAgent::getShowHint(){
    return WorkflowSettings::isShowLoadButtonHint();
}

//Worker parametes initialization
void DashboardJsAgent::fillWorkerParamsInfo(){
    CHECK(monitor,);
    QJsonArray localWorkersParamsInfo;
    QList<Monitor::WorkerParamsInfo> workersParamsList = monitor->getWorkersParameters();
    foreach (Monitor::WorkerParamsInfo workerInfo, workersParamsList) {
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
                    if(type == InputDir || type == OutputDir) {
                        parameterJS["type"] = "Dir";
                    }
                }
                parameters.append(parameterJS);
            }
        }
        workerInfoJS["parameters"] = parameters;
        localWorkersParamsInfo.append(workerInfoJS);
        QJsonDocument doc(localWorkersParamsInfo);
        workersParamsInfo = doc.toJson(QJsonDocument::Compact);
    }
}

/************************************************************************/
/* DashboardWidget */
/************************************************************************/
DashboardWidget::DashboardWidget(const QString &container, Dashboard *parent)
    : QObject(parent),
      dashboard(parent),
      container(container)
{

}

/************************************************************************/
/* LoadDashboardsTask */
/************************************************************************/
ScanDashboardsDirTask::ScanDashboardsDirTask()
: Task(tr("Scan dashboards folder"), TaskFlag_None)
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

DashboardInfo::DashboardInfo(const QString &dirPath, bool opened)
    : path(dirPath),
      opened(opened)
{
    dirName = QDir(path).dirName();
}

bool DashboardInfo::operator==(const DashboardInfo &other) const {
    return path == other.path;
}

}   // namespace U2
