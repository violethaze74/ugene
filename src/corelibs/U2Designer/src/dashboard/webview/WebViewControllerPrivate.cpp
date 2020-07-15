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

#include "WebViewControllerPrivate.h"

#include <QDesktopServices>

#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

WebViewControllerPrivate::WebViewControllerPrivate(U2WebView *_webView)
    : webView(_webView) {
}

void WebViewControllerPrivate::saveContent(const QString &url, const QString &data) {
    CHECK(!data.isEmpty(), );
    if (!IOAdapterUtils::writeTextFile(url, data)) {
        ioLog.error(tr("Can not open a file for writing: ") + url);
    }
}

void WebViewControllerPrivate::sl_linkClicked(const QUrl &url) {
    QDesktopServices::openUrl(url);
}

}    // namespace U2
