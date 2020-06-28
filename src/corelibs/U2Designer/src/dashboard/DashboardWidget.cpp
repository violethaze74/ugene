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

#include "DashboardWidget.h"

#include <QHBoxLayout>
#include <QLabel>

namespace U2 {

DashboardWidget::DashboardWidget(const QString &title, QWidget *contentWidget) {
    auto layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    setStyleSheet("QWidget#tabWidgetStyleRoot {"
                  " border-radius: 6px;"
                  " border: 1px solid #ddd;"
                  "}");

    auto styleRootWidget = new QWidget();
    styleRootWidget->setObjectName("tabWidgetStyleRoot");
    layout->addWidget(styleRootWidget);

    auto styleRootWidgetLayout = new QVBoxLayout();
    styleRootWidgetLayout->setMargin(0);
    styleRootWidgetLayout->setSpacing(0);
    styleRootWidget->setLayout(styleRootWidgetLayout);

    auto titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("background: rgb(239, 239, 239);"
                              "color: #222;"
                              "padding: 5px;"
                              "border-top-left-radius: 6px;"
                              "border-top-right-radius: 6px;");
    styleRootWidgetLayout->addWidget(titleLabel);

    auto contentStyleWidget = new QWidget();
    contentStyleWidget->setObjectName("tabWidgetContentStyleRoot");
    contentStyleWidget->setStyleSheet("QWidget#tabWidgetContentStyleRoot {"
                                      " background: white;"
                                      " border-bottom-left-radius: 6px;"
                                      " border-bottom-right-radius: 6px;"
                                      "}");
    styleRootWidgetLayout->addWidget(contentStyleWidget);

    auto contentStyleWidgetLayout = new QVBoxLayout();
    contentStyleWidgetLayout->setMargin(12);
    contentStyleWidgetLayout->setSpacing(0);
    contentStyleWidget->setLayout(contentStyleWidgetLayout);

    contentStyleWidgetLayout->addWidget(contentWidget);
}

}    // namespace U2
