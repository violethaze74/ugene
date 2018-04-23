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

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <QtWebChannel/QWebChannel>

#include <QDesktopServices>
#if (QT_VERSION < 0x050500) //Qt 5.7
#include <QtWebSockets/QWebSocketServer>
#include <U2Gui/WebSocketClientWrapper.h>
#include <U2Gui/WebSocketTransport.h>
#endif

#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Core/Settings.h>
#include <U2Core/U2SafePoints.h>

#include "WelcomePageController.h"
#include "main_window/MainWindowImpl.h"

#include "WelcomePageWidget.h"

namespace U2 {

namespace {
    const int MAX_RECENT = 7;
}

WelcomePageWidget::WelcomePageWidget(QWidget *parent, WelcomePageController *controller)
#if (QT_VERSION < 0x50500)
    : MultilingualHtmlView("qrc:///ugene/html/welcome_page_webengine_qt542.html", parent),
#else
    : MultilingualHtmlView("qrc:///ugene/html/welcome_page_webengine.html", parent),
#endif
      controller(controller)
{
    installEventFilter(this);
    setObjectName("webView");
#if (QT_VERSION < 0x050500) //Qt 5.7
    server = new QWebSocketServer(QStringLiteral("UGENE Standalone Server"), QWebSocketServer::NonSecureMode, this);
    if (!server->listen(QHostAddress::LocalHost, 12345)) {
        return;
    }

    clientWrapper = new WebSocketClientWrapper(server);//TODO delete in dtor

    channel = new QWebChannel(this);

    QObject::connect(clientWrapper, &WebSocketClientWrapper::clientConnected,
        channel, &QWebChannel::connectTo);
    channel->registerObject(QString("ugene"), controller);
#else
    channel->registerObject(QString("ugene"), controller);
#endif
}

void WelcomePageWidget::sl_loaded(bool ok) {
    MultilingualHtmlView::sl_loaded(ok);
    addController();
}

void WelcomePageWidget::updateRecent(const QStringList &recentProjects, const QStringList &recentFiles) {
    updateRecentFilesContainer("recent_projects", recentProjects, tr("No opened projects yet"));
    updateRecentFilesContainer("recent_files", recentFiles, tr("No opened files yet"));
    page()->runJavaScript("updateLinksVisibility()");
}

void addRecentItem(const QString &id, const QString & file, QWebEnginePage *page) {
    if (id.contains("recent_files")) {
            page->runJavaScript(QString("addRecentItem(\"recent_files\", \"%1\", \"%2\")").arg(file).arg(QFileInfo(file).fileName()));
    } else if (id.contains("recent_projects")) {
            page->runJavaScript(QString("addRecentItem(\"recent_projects\", \"%1\", \"%2\")").arg(file).arg(QFileInfo(file).fileName()));
    } else {
        SAFE_POINT(false, "Unknown containerId", );
    }
}

void addNoItems(const QString &id, const QString & message, QWebEnginePage *page) {
    if (id.contains("recent_files")) {
        page->runJavaScript(QString("addRecentItem(\"recent_files\", \"%1\", \"\")").arg(message));
    } else if (id.contains("recent_projects")) {
        page->runJavaScript(QString("addRecentItem(\"recent_projects\", \"%1\", \"\")").arg(message));
    } else {
        SAFE_POINT(false, "Unknown containerId", );
    }
}

void WelcomePageWidget::updateRecentFilesContainer(const QString &id, const QStringList &files, const QString &message) {
    page()->runJavaScript(QString("clearRecent(\"%1\")").arg(id));
    bool emptyList = true;
    foreach(const QString &file, files.mid(0, MAX_RECENT)) {
        if (file.isEmpty()) {
            continue;
        }
        emptyList = false;
        addRecentItem(id, file, page());
    }

    if (emptyList) {
        addNoItems(id, message, page());
    }
}

void WelcomePageWidget::addController() {
    controller->onPageLoaded();
}

void WelcomePageWidget::dragEnterEvent(QDragEnterEvent *event) {
    MainWindowDragNDrop::dragEnterEvent(event);
}

void WelcomePageWidget::dropEvent(QDropEvent *event) {
    MainWindowDragNDrop::dropEvent(event);
}

void WelcomePageWidget::dragMoveEvent(QDragMoveEvent *event) {
    MainWindowDragNDrop::dragMoveEvent(event);
}

bool WelcomePageWidget::eventFilter(QObject *watched, QEvent *event) {
    CHECK(this == watched, false);
    switch (event->type()) {
        case QEvent::DragEnter:
            dragEnterEvent(dynamic_cast<QDragEnterEvent*>(event));
            return true;
        case QEvent::DragMove:
            dragMoveEvent(dynamic_cast<QDragMoveEvent*>(event));
            return true;
        case QEvent::Drop:
            dropEvent(dynamic_cast<QDropEvent*>(event));
            return true;
        case QEvent::FocusIn:
            setFocus();
            return true;
        default:
            return false;
    }
}

} // U2
