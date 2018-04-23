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

#ifndef _U2_MULTILINGUAL_HTML_VIEW_H_
#define _U2_MULTILINGUAL_HTML_VIEW_H_

#include <qglobal.h>
#include <QWebEngineView>
#include <QWebChannel>
#if (QT_VERSION < 0x050500) //Qt 5.7
#include <QWebSocketServer>
#include <U2Gui/WebSocketClientWrapper.h>
#endif

namespace U2 {

class MultilingualWebEnginePage : public QWebEnginePage {
    Q_OBJECT
public:
    MultilingualWebEnginePage(QObject* parent = 0);
protected:
#if (QT_VERSION >= 0x050500)
    virtual bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);
#else
    virtual bool javaScriptConfirm(const QUrl & securityOrigin, const QString & msg); // hack for Qt5.4 only
#endif

};

class MultilingualHtmlView : public QWebEngineView {
    Q_OBJECT
public:
    MultilingualHtmlView(const QString& htmlPath, QWidget* parent = NULL);
    bool isLoaded() const;

public slots:
    void performAction(const QString&) {}
protected slots:
    virtual void sl_loaded(bool ok);
    virtual void sl_linkActivated(const QUrl& url);

signals:
    void si_loaded(bool ok);
protected:
#if (QT_VERSION < 0x050500) //Qt 5.7
    QWebSocketServer *server;
    WebSocketClientWrapper *clientWrapper;
    QWebChannel *channel;
#else
    QWebChannel *channel;
#endif
private:
    void loadPage(const QString& htmlPath);
    bool loaded;
};

} // namespace

#endif // _U2_MULTILINGUAL_HTML_VIEW_H_
