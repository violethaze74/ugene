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

#include "MultilingualHtmlView.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/U2SafePoints.h>

#include <QDesktopServices>
#if (QT_VERSION < 0x050400) //Qt 5.7
#include <QWebElementCollection>
#include <QWebFrame>
#else
#include <QWebChannel>
#endif

namespace U2 {
#if (QT_VERSION < 0x050400) //Qt 5.7
MultilingualHtmlView::MultilingualHtmlView(const QString& htmlPath, QWidget* parent)
    : QWebView(parent),loaded(false)
#else
MultilingualHtmlView::MultilingualHtmlView(const QString& htmlPath, QWidget* parent)
    : QWebEngineView(parent),loaded(false)
#endif
{
    setContextMenuPolicy(Qt::NoContextMenu);
    loadPage(htmlPath);
#if (QT_VERSION < 0x050400) //Qt 5.7
    page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);
#else

#endif
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

#if (QT_VERSION < 0x050400) //Qt 5.7
    QWebFrame* frame = page()->mainFrame();
    SAFE_POINT(frame != NULL, "MainFrame of webView page is NULL", );

    QWebElementCollection otherLangsCollection = frame->findAllElements(QString(":not(:lang(%1))[lang]").arg(lang));
    for (int i = 0; i < otherLangsCollection.count(); i++) {
        otherLangsCollection[i].setStyleProperty("display", "none");
    }
#else
    page()->runJavaScript(QString("showOnlyLang(\"%1\");").arg(lang));
#endif
    emit si_loaded(ok);
}

void MultilingualHtmlView::sl_linkActivated(const QUrl &url) {
    QDesktopServices::openUrl(url);
}

void MultilingualHtmlView::loadPage(const QString& htmlPath) {
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(sl_loaded(bool)));
#if (QT_VERSION < 0x050400) //Qt 5.7
    connect(this, SIGNAL(linkClicked(QUrl)), this, SLOT(sl_linkActivated(QUrl)));
    load(QUrl(htmlPath));
#else
    QWebEnginePage *pg = new MultilingualWebEnginePage(parentWidget());
    pg->load(QUrl(htmlPath));
    setPage(pg);

    channel = new QWebChannel(page());
    page()->setWebChannel(channel);
#endif
}

#if (QT_VERSION >= 0x050400) //Qt 5.7
MultilingualWebEnginePage::MultilingualWebEnginePage(QObject *parent) : QWebEnginePage(parent) {}

bool MultilingualWebEnginePage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool) {
    if (type == NavigationTypeLinkClicked) {
        QDesktopServices::openUrl(url);
        return false;
    }
    return true;
}
#endif

} // namespace
