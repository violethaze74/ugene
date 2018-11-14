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

#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFile>
#include <QScrollBar>

#include <U2Core/Version.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/SimpleWebViewBasedWidgetController.h>

#include "StatisticalReportController.h"

namespace U2 {

StatisticalReportController::StatisticalReportController(const QString &newHtmlFilepath, QWidget *parent) : QDialog(parent) {
    setupUi(this);
    lblStat->setText(tr("<b>Optional:</b> Help make UGENE better by automatically sending anonymous usage statistics."));

    Version v = Version::appVersion();
    setWindowTitle(tr("Welcome to UGENE %1.%2").arg(v.major).arg(v.minor));

    htmlView = new U2WebView(this);
    htmlView->setMinimumSize(400, 10);
    frameLayout->addWidget(htmlView);

    htmlViewController = new SimpleWebViewBasedWidgetController(htmlView);
    connect(htmlViewController, SIGNAL(si_pageReady()), SLOT(sl_pageReady()));
    htmlViewController->loadPage(newHtmlFilepath);

    connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
}

bool StatisticalReportController::isInfoSharingAccepted() const {
    return chkStat->isChecked();
}

void StatisticalReportController::sl_pageReady() {
#if (QT_VERSION < 0x050500 && defined(UGENE_QT_WEB_ENGINE)) 
    htmlViewController->runJavaScript("bindLinks();");
#endif

    // Update the widget size
    htmlViewController->runJavaScript("getBodyHeight();", [&](const QVariant &var) {
        int pageHeight = var.toInt();
        htmlView->setMinimumHeight(pageHeight);
#ifndef Q_OS_MAC //TODO recheck this code on OS X
        // UGENE crashes on the update event processing on mac
        // It has some connection with htmlView loading method
        // There was no crash before f3a45ef1cd53fe28faf90a763d195e964bc6c752 commit
        // Find a solution and fix it, if you have some free time
        QWidget *mainWindowWidget = qApp->activeModalWidget()->parentWidget();
        move((mainWindowWidget->x() + mainWindowWidget->width() / 2) - width() / 2,
            (mainWindowWidget->y() + mainWindowWidget->height() / 2) - pageHeight / 2);
#endif
    });
}

void StatisticalReportController::accept() {
    QDialog::close();
}

}
