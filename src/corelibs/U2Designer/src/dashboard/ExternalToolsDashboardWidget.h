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

class HoverQLabel;

class U2DESIGNER_EXPORT ExternalToolsDashboardWidget : public QWidget {
    Q_OBJECT
public:
    ExternalToolsDashboardWidget(const QDomElement &initialStateDom);
};

class BadgeLabel : public QWidget {
    Q_OBJECT
public:
    BadgeLabel(int kind, const QString &text);
    const int kind;

    HoverQLabel *hoverLabel;
    HoverQLabel *copyButton;
};

class ExternalToolsTreeNode : public QWidget {
    Q_OBJECT
public:
    ExternalToolsTreeNode(int kind, const QString &content, ExternalToolsTreeNode *parent);

    void paintEvent(QPaintEvent *event);

    const int kind;

    const ExternalToolsTreeNode *parent;

    QList<ExternalToolsTreeNode *> children;

    QString content;

public slots:
    void sl_toggle();
    void sl_copyRunCommand();

private:
    void updateExpandCollapseState(bool isParentExpanded, bool isApplyToAllLevelOfChildren = false);

};

}    // namespace U2
#endif
