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

#ifndef _U2_NOTIFICATIONS_WIDGET_H_
#define _U2_NOTIFICATIONS_WIDGET_H_

#include "TableWidget.h"

namespace U2 {

using namespace Workflow::Monitor;

class NotificationsWidget : public TableWidget {
    Q_OBJECT
public:
    NotificationsWidget(const QWebElement &content, Dashboard *parent);

    virtual QList<int> widths();
    virtual QStringList header();
    virtual QList<QStringList> data();

    static QString notificationImage(const WorkflowNotification &info);

    static const QString ID;

private slots:
    void sl_newNotification(const WorkflowNotification &info);

protected:
    QString createRow(const QStringList &d);

private:
    QStringList createRow(const WorkflowNotification &info, bool multi) const;
    QStringList createRow(const WorkflowNotification &info) const;
    QStringList createMultiRow(const WorkflowNotification &info) const;
    QString id(const WorkflowNotification &info) const;
    QString getTextWithWordBreaks(const QString &text) const;
};

} // U2

#endif // _U2_NOTIFICATIONS_WIDGET_H_
