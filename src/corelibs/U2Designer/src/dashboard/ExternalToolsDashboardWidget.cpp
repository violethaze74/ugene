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

#include "ExternalToolsDashboardWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>

#include <U2Gui/HoverQLabel.h>

namespace U2 {

const QString COMMON_BADGE_STYLE = "border-radius: 6px;"
                                   "padding: 2px 4px;"
                                   "color: white;";

const QString L1_ACTOR_BADGE_STYLE = COMMON_BADGE_STYLE + "background-color: #92939E;";
const QString L2_TOOL_BADGE_STYLE = COMMON_BADGE_STYLE + "background-color: #bdb0a0;";
const QString L3_SUCCESSFUL_RUN_BADGE_STYLE = COMMON_BADGE_STYLE + "background-color: #50A976;";
const QString L3_FAILED_RUN_BADGE_STYLE = COMMON_BADGE_STYLE + "background-color: #CC6666;";
const QString L4_CMDLINE_BADGE_STYLE = COMMON_BADGE_STYLE + "background-color: #79ACAC;";
const QString L4_OUTPUT_BADGE_STYLE = COMMON_BADGE_STYLE + "background-color: #9999CC;";

ExternalToolsDashboardWidget::ExternalToolsDashboardWidget(const QDomElement &initialStateDom) {
    setMinimumWidth(900);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    layout->addWidget(new ExternalToolsTreeNode(0, new BadgeLabel("Actor", L1_ACTOR_BADGE_STYLE)));
    layout->addWidget(new ExternalToolsTreeNode(1, new BadgeLabel("Tool", L2_TOOL_BADGE_STYLE, true)));
    layout->addWidget(new ExternalToolsTreeNode(2, new BadgeLabel("Run", L3_SUCCESSFUL_RUN_BADGE_STYLE)));
    layout->addWidget(new ExternalToolsTreeNode(3, new BadgeLabel("Command line", L4_CMDLINE_BADGE_STYLE), false));
    layout->addWidget(new ExternalToolsTreeNode(3, new BadgeLabel("Output", L4_OUTPUT_BADGE_STYLE)));
}

BadgeLabel::BadgeLabel(const QString &text, const QString &style, bool isCopyButtonVisible) {
    auto layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    QString finalStyle = isCopyButtonVisible ? style + ";border-top-right-radius: 0; border-bottom-right-radius: 0;" : style;
    auto label = new HoverQLabel(text, "QLabel {" + finalStyle + "}", "QLabel {" + finalStyle + "; color: black;}");
    layout->addWidget(label);
    if (isCopyButtonVisible) {
        QString copyButtonStyle = style + ";border-top-left-radius: 0; border-bottom-left-radius: 0; border-left: 1px solid #eee;";
        auto copyButton = new HoverQLabel("", "QLabel {" + copyButtonStyle + "}", "QLabel {" + copyButtonStyle + "; color: black; background: #777;}");
        copyButton->setPixmap(QPixmap(":U2Designer/images/copy.png"));
        copyButton->setToolTip(tr("Copy output"));
        layout->addWidget(copyButton);
    }
    layout->addStretch(1);
};

#define TREE_NODE_X_OFFSET 50

ExternalToolsTreeNode::ExternalToolsTreeNode(int level, QWidget *contentWidget, bool isLast)
    : level(level), isLast(isLast) {
    setContentsMargins(0, 5, 0, 5);
    auto layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    layout->addSpacing(level * TREE_NODE_X_OFFSET);
    layout->addWidget(contentWidget);
}

void ExternalToolsTreeNode::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    if (level == 0 || width() == 0 || height() == 0) {
        return;
    }

    QPoint p1((level - 1) * TREE_NODE_X_OFFSET + 15, 0);
    QPoint p2(p1.x(), height() / 2);
    QPoint p3(level * TREE_NODE_X_OFFSET - 5, p2.y());
    QPoint p4(p1.x(), height());

    QPainter painter(this);
    painter.setPen(QPen(QBrush(QColor("#999999")), 1));
    painter.drawLine(p1, p2);
    painter.drawLine(p2, p3);
    if (!isLast) {
        painter.drawLine(p2, p4);
    }
}

}    // namespace U2
