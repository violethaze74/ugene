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

#include "DashboardTabPage.h"

#include "DashboardWidget.h"
#include "FlowLayout.h"

namespace U2 {

DashboardTabPage::DashboardTabPage(const QString &tabObjectName, bool useFlowLayout) {
    setObjectName(tabObjectName);
    setStyleSheet("QWidget#tabPageStyleRoot {"
                  " background: url(':U2Designer/images/background.png') repeat scroll 0 0 transparent; "
                  " padding: 10px;"
                  "}");

    QWidget *mainWidget = new QWidget();

    auto layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    mainWidget->setLayout(layout);

    auto styleRootWidget = new QWidget();
    styleRootWidget->setObjectName("tabPageStyleRoot");
    layout->addWidget(styleRootWidget);

    auto styleRootWidgetLayout = new QHBoxLayout();
    styleRootWidgetLayout->setMargin(0);
    styleRootWidgetLayout->setSpacing(0);
    styleRootWidget->setLayout(styleRootWidgetLayout);

    styleRootWidgetLayout->addStretch(1);
    int widgetSpacing = 15;
    if (useFlowLayout) {
        widgetsLayout = new FlowLayout(widgetSpacing, widgetSpacing, widgetSpacing);
    } else {
        auto vBoxLayout = new QVBoxLayout();
        vBoxLayout->setContentsMargins(widgetSpacing, widgetSpacing, widgetSpacing, widgetSpacing);
        widgetsLayout = vBoxLayout;
    }
    styleRootWidgetLayout->addLayout(widgetsLayout);
    styleRootWidgetLayout->addStretch(1);

    setWidget(mainWidget);
    setWidgetResizable(true);    // make the widget to fill whole available space
}

void DashboardTabPage::addDashboardWidget(const QString &title, QWidget *contentWidget) {
    widgetsLayout->addWidget(new DashboardWidget(title, contentWidget));
}

}    // namespace U2
