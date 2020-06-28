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

#ifndef _U2_EXTERNAL_TOOLS_DASHBOARD_WIDGET_H_
#define _U2_EXTERNAL_TOOLS_DASHBOARD_WIDGET_H_

#include <QDomElement>
#include <QWidget>

#include <U2Lang/WorkflowMonitor.h>

namespace U2 {

class U2DESIGNER_EXPORT ExternalToolsDashboardWidget : public QWidget {
    Q_OBJECT
public:
    ExternalToolsDashboardWidget(const QDomElement &initialStateDom);
};

class BadgeLabel : public QWidget {
    Q_OBJECT
public:
    BadgeLabel(const QString &text, const QString &style, bool isCopyButtonVisible = false);
};

class ExternalToolsTreeNode : public QWidget {
    Q_OBJECT
public:
    ExternalToolsTreeNode(int level, QWidget *contentWidget, bool isLast = true);

    void paintEvent(QPaintEvent *event);

private:
    int level;
    bool isLast;
};

}    // namespace U2
#endif
