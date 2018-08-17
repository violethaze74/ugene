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

#include <U2Core/U2SafePoints.h>

#include "NotificationsWidget.h"

namespace U2 {

const QString NotificationsWidget::ID = "problems";

NotificationsWidget::NotificationsWidget(const QWebElement &content, Dashboard *parent)
    : TableWidget(content, ID, parent)
{
    createTable();
    foreach (const WorkflowNotification &info, dashboard->monitor()->getNotifications()) {
        sl_newNotification(info);
    }
    connect(dashboard->monitor(), SIGNAL(si_newNotification(const WorkflowNotification &)),
        SLOT(sl_newNotification(const WorkflowNotification &)));
}

void NotificationsWidget::sl_newNotification(const WorkflowNotification &info) {
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, );
    if (rows.contains(id(info))) {
        updateRow(id(info), createMultiRow(info));
    } else {
        addRow(id(info), createRow(info));
    }
}

QString NotificationsWidget::notificationImage(const WorkflowNotification &info) {
    CHECK(!info.type.isEmpty(), "");

    QString image = "qrc:U2Lang/images/";
    QString tooltip;
    if (WorkflowNotification::U2_ERROR == info.type) {
        image += "error.png";
        tooltip = tr("Error");
    } else if (WorkflowNotification::U2_WARNING == info.type) {
        image += "warning.png";
        tooltip = tr("Warning");
    } else if (WorkflowNotification::U2_INFO == info.type) {
        image = "qrc:core/images/info.png";
        tooltip = tr("Information");
    } else {
        FAIL("Unknown type: " + info.type, "");
    }
    return "<img src=\"" + image + "\" title=\"" + tooltip + "\" class=\"problem-icon\"/>";
}

QString NotificationsWidget::createRow(const QStringList &ds) {
    QString row;
    foreach (const QString &d, ds) {
        row += "<td style=\"word-wrap: break-word\">" + d + "</td>";
    }
    return row;
}

QStringList NotificationsWidget::createRow(const WorkflowNotification &info, bool multi) const {
    QStringList result;
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, result);

    QString prefix;
    if (multi) {
        int count = 0;
        foreach (const WorkflowNotification &p, m->getNotifications()) {
            if (p == info) {
                count++;
            }
        }
        prefix = QString("(%1) ").arg(count);
    }

    result << notificationImage(info);
    result << wrapLongText(m->actorName(info.actor));
    result << getTextWithWordBreaks(prefix + info.message);
    return result;
}

QString NotificationsWidget::getTextWithWordBreaks(const QString& text) const {
    QString textWithBreaks = text;
    textWithBreaks = textWithBreaks.replace("\\", "\\<wbr>").replace("/", "/<wbr>");
    return textWithBreaks;
}

QStringList NotificationsWidget::createMultiRow(const WorkflowNotification &info) const {
    return createRow(info, true);
}

QStringList NotificationsWidget::createRow(const WorkflowNotification &info) const {
    return createRow(info, false);
}

QList<int> NotificationsWidget::widths() {
    return QList<int>() << 10 << 30 << 60;
}

QStringList NotificationsWidget::header() {
    return QStringList() << tr("Type") << tr("Element") << tr("Message");
}

QList<QStringList> NotificationsWidget::data() {
    QList<QStringList> result;
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, result);
    foreach (const WorkflowNotification &info, m->getNotifications()) {
        QStringList row;
        row << id(info);
        row << createRow(info);
        result << row;
    }
    return result;
}

QString NotificationsWidget::id(const WorkflowNotification &info) const {
    return info.actor + info.message;
}

} // U2
