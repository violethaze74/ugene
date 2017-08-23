/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
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

#include "MultilingualHtmlView.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/U2SafePoints.h>

#include <QDesktopServices>

#include <QWebChannel>
#if (QT_VERSION < 0x050500) //Qt 5.7
#include <QWebSocketServer>
#include <U2Gui/WebSocketClientWrapper.h>
#include <U2Gui/WebSocketTransport.h>
#endif

namespace U2 {

MultilingualHtmlView::MultilingualHtmlView(const QString& htmlPath, QWidget* parent)
    : QWebEngineView(parent),loaded(false)
{
    setContextMenuPolicy(Qt::NoContextMenu);
    loadPage(htmlPath);
}

bool MultilingualHtmlView::isLoaded() const {
    return loaded;
}

void MultilingualHtmlView::sl_loaded(bool ok) {
    disconnect(this, SIGNAL(loadFinished(bool)), this, SLOT(sl_loaded(bool)));
    SAFE_POINT(ok, "Can not load page", );
    loaded = true;

    Settings* s = AppContext::getSettings();
    SAFE_POINT(s != NULL, "AppContext settings is NULL", );
    QString lang = s->getValue("UGENE_CURR_TRANSL", "en").toString();

    page()->runJavaScript(QString("showOnlyLang(\"%1\");").arg(lang));
#if (QT_VERSION < 0x050500)
    page()->runJavaScript("bindLinks();");
#endif
    emit si_loaded(ok);
}

void MultilingualHtmlView::sl_linkActivated(const QUrl &url) {
    QDesktopServices::openUrl(url);
}

void MultilingualHtmlView::loadPage(const QString& htmlPath) {
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(sl_loaded(bool)));
#if (QT_VERSION < 0x050500) //Qt 5.7
    QWebEnginePage *page = new MultilingualWebEnginePage(parentWidget());
    QUrl url(htmlPath);
    page->load(url);
    setPage(page);
#else
    QWebEnginePage *pg = new MultilingualWebEnginePage(parentWidget());
    pg->load(QUrl(htmlPath));
    setPage(pg);

    channel = new QWebChannel(page());
    page()->setWebChannel(channel);
#endif
}

MultilingualWebEnginePage::MultilingualWebEnginePage(QObject *parent) : QWebEnginePage(parent) {

}

#if (QT_VERSION >= 0x050500)
bool MultilingualWebEnginePage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool) {
    if (type == NavigationTypeLinkClicked) {
        QDesktopServices::openUrl(url);
        return false;
    }
    return true;
}
#else
bool MultilingualWebEnginePage::javaScriptConfirm(const QUrl &, const QString &msg){
    QDesktopServices::openUrl(msg);
    return false;
}
#endif


} // namespace
