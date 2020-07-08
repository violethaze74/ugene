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

#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollArea>

#include <U2Gui/HoverQLabel.h>

#include "DomUtils.h"

namespace U2 {

#define NODE_KIND_ACTOR 1
#define NODE_KIND_TOOL 2
#define NODE_KIND_RUN 3
#define NODE_KIND_COMMAND 4
#define NODE_KIND_COMMAND_CONTENT 5
#define NODE_KIND_OUTPUT 6
#define NODE_KIND_OUTPUT_CONTENT 7

ExternalToolsDashboardWidget::ExternalToolsDashboardWidget(const QDomElement &initialStateDom) {
    setMinimumWidth(900);
    //    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    auto layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    QList<QDomElement> actorElementList;
    DomUtils::findChildElementsByClass(initialStateDom, "actor-node", actorElementList);
    for (auto actorSpan : actorElementList) {
        auto actorNode = new ExternalToolsTreeNode(NODE_KIND_ACTOR, actorSpan.text(), nullptr);
        layout->addWidget(actorNode);

        QList<QDomElement> toolElementList;
        DomUtils::findChildElementsByClass(actorSpan.parentNode().toElement(), "actor-tick-node", toolElementList);
        for (auto toolSpan : toolElementList) {
            auto toolNode = new ExternalToolsTreeNode(NODE_KIND_TOOL, toolSpan.text(), actorNode);
            layout->addWidget(toolNode);

            QList<QDomElement> runElementList;
            DomUtils::findChildElementsByClass(toolSpan.parentNode().toElement(), "tool-run-node", runElementList);
            for (auto runSpan : runElementList) {
                auto runNode = new ExternalToolsTreeNode(NODE_KIND_RUN, runSpan.text(), toolNode);
                layout->addWidget(runNode);

                QList<QDomElement> commandElementList;
                DomUtils::findChildElementsByClass(runSpan.parentNode().toElement(), "command", commandElementList);
                for (auto commandSpan : commandElementList) {
                    auto commandNode = new ExternalToolsTreeNode(NODE_KIND_COMMAND, commandSpan.text(), runNode);
                    layout->addWidget(commandNode);

                    QDomNode commandParentEl = commandSpan.parentNode();

                    QDomElement commandContentSpan = DomUtils::findChildElementByClass(commandParentEl.toElement(), "content");
                    if (!commandContentSpan.isNull()) {
                        auto commandContentNode = new ExternalToolsTreeNode(NODE_KIND_COMMAND_CONTENT, commandContentSpan.text(), commandNode);
                        layout->addWidget(commandContentNode);
                    }

                    QDomElement outputLi = commandParentEl.nextSiblingElement("li");
                    QDomElement outputSpan = DomUtils::findChildElementByClass(outputLi, "badge");
                    if (!outputSpan.isNull()) {
                        auto outputNode = new ExternalToolsTreeNode(NODE_KIND_OUTPUT, outputSpan.text(), runNode);
                        layout->addWidget(outputNode);

                        QDomElement outputContentSpan = DomUtils::findChildElementByClass(outputLi.toElement(), "content");
                        if (!outputContentSpan.isNull()) {
                            auto outputContentNode = new ExternalToolsTreeNode(NODE_KIND_OUTPUT_CONTENT, DomUtils::toString(outputContentSpan), outputNode);
                            layout->addWidget(outputContentNode);
                        }
                    }
                }
            }
        }
    }
}

const QString COMMON_BADGE_STYLE = "border-radius: 6px;"
                                   "padding: 2px 4px;"
                                   "color: white;";

//const QString L3_FAILED_RUN_BADGE_STYLE = COMMON_BADGE_STYLE + "background-color: #CC6666;";

BadgeLabel::BadgeLabel(int kind, const QString &text)
    : kind(kind), hoverLabel(nullptr), copyButton(nullptr) {
    QString style = COMMON_BADGE_STYLE;
    bool isCopyButtonVisible = false;
    bool isLogView = false;
    switch (kind) {
    case NODE_KIND_ACTOR:
        style += "background-color: #92939E;";
        break;
    case NODE_KIND_TOOL:
        style += "background-color: #bdb0a0;";
        break;
    case NODE_KIND_RUN:
        style += "background-color: #50A976;";
        isCopyButtonVisible = true;
        break;
    case NODE_KIND_COMMAND:
        style += "background-color: #79ACAC;";
        break;
    case NODE_KIND_OUTPUT:
        style += "background-color: #9999CC;";
        break;
    case NODE_KIND_COMMAND_CONTENT:
    case NODE_KIND_OUTPUT_CONTENT:
        style += "background-color: #F0F0F0; color: black;";    // selection-color: yellow; selection-background-color: blue;
        isLogView = true;
        break;
    }

    auto layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    QString finalStyle = isCopyButtonVisible ? style + ";border-top-right-radius: 0; border-bottom-right-radius: 0;" : style;
    //TODO: process onclick="openLog()"
    if (isLogView) {
        auto label = new QLabel("<code>" + text + "</code>");
        label->setStyleSheet(finalStyle);
        label->setTextInteractionFlags(Qt::TextBrowserInteraction);
        label->setContextMenuPolicy(Qt::DefaultContextMenu);
        label->setOpenExternalLinks(true);

        auto scrollArea = new QScrollArea();
        scrollArea->setStyleSheet("QScrollArea {" + finalStyle + "}");
        scrollArea->setWidget(label);
        layout->addWidget(scrollArea);
    } else {
        hoverLabel = new HoverQLabel(text, "QLabel {" + finalStyle + "}", "QLabel {" + finalStyle + "; color: black;}");
        layout->addWidget(hoverLabel);
    }

    if (isCopyButtonVisible) {
        QString copyButtonStyle = style + ";border-top-left-radius: 0; border-bottom-left-radius: 0; border-left: 1px solid #eee;";
        copyButton = new HoverQLabel("", "QLabel {" + copyButtonStyle + "}", "QLabel {" + copyButtonStyle + "; color: black; background: #777;}");
        copyButton->setPixmap(QPixmap(":U2Designer/images/copy.png"));
        copyButton->setToolTip(tr("Copy output"));
        layout->addWidget(copyButton);
    }
    if (!isLogView) {
        layout->addStretch(1);
    }
};

#define TREE_NODE_X_OFFSET 50

static int getLevelByNodeKind(int kind) {
    switch (kind) {
    case NODE_KIND_ACTOR:
        return 0;
    case NODE_KIND_TOOL:
        return 1;
    case NODE_KIND_RUN:
        return 2;
    case NODE_KIND_COMMAND:
    case NODE_KIND_OUTPUT:
        return 3;
    case NODE_KIND_COMMAND_CONTENT:
    case NODE_KIND_OUTPUT_CONTENT:
        return 4;
    }
    SAFE_POINT(false, "Unknown kind: " + QString::number(kind), 0);
}

ExternalToolsTreeNode::ExternalToolsTreeNode(int kind, const QString &content, ExternalToolsTreeNode *parent)
    : kind(kind), parent(parent), content(content) {
    if (parent != nullptr) {
        parent->children << this;
    }
    setContentsMargins(0, 5, 0, 5);
    auto layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    int level = getLevelByNodeKind(kind);
    layout->addSpacing(level * TREE_NODE_X_OFFSET);
    auto label = new BadgeLabel(kind, content);
    layout->addWidget(label);

    if (label->hoverLabel) {
        connect(label->hoverLabel, SIGNAL(clicked()), SLOT(sl_toggle()));
    }

    if (label->copyButton && kind == NODE_KIND_RUN) {
        connect(label->copyButton, SIGNAL(clicked()), SLOT(sl_copyRunCommand()));
    }

    setVisible(level <= getLevelByNodeKind(NODE_KIND_RUN));
}

void ExternalToolsTreeNode::sl_toggle() {
    bool isExpandedBefore = children.size() > 0 && children.first()->isVisible();
    bool isExpandedAfter = !isExpandedBefore;

    // Auto-expand command & output nodes when RUN node is clicked.
    bool expandAllChildren = isExpandedAfter && getLevelByNodeKind(kind) >= getLevelByNodeKind(NODE_KIND_RUN);

    for (auto child : children) {
        child->updateExpandCollapseState(isExpandedAfter, expandAllChildren);
    }
}

void ExternalToolsTreeNode::updateExpandCollapseState(bool isParentExpanded, bool isApplyToAllLevelOfChildren) {
    this->setVisible(isParentExpanded);
    if (!isParentExpanded) {    // make all children invisible (we use flat VBOX layout model for the tree, so parent must hide children manually).
        for (auto child : children) {
            child->updateExpandCollapseState(false);
        }
    } else if (isApplyToAllLevelOfChildren) {    // make children on all levels visible.
        for (auto child : children) {
            child->updateExpandCollapseState(true, true);
        }
    }
}

void ExternalToolsTreeNode::sl_copyRunCommand() {
    if (kind == NODE_KIND_RUN && !children.isEmpty() && !children[0]->children.isEmpty()) {
        QApplication::clipboard()->setText(children[0]->children[0]->content);
    }
}

static bool isLastChild(const ExternalToolsTreeNode *node) {
    return node != nullptr && node->parent != nullptr && node->parent->children.last() == node;
}

#define BRANCH_X_PADDING 15

void ExternalToolsTreeNode::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    if (kind == NODE_KIND_ACTOR || width() == 0 || height() == 0) {
        return;
    }

    QPainter painter(this);
    painter.setPen(QPen(QBrush(QColor("#999999")), 1));

    for (const ExternalToolsTreeNode *node = this; node != nullptr && node->kind != NODE_KIND_ACTOR; node = node->parent) {
        int level = getLevelByNodeKind(node->kind);
        int x = (level - 1) * TREE_NODE_X_OFFSET + BRANCH_X_PADDING;
        if (node == this) {
            painter.drawLine(x, 0, x, isLastChild(node) ? height() / 2 : height());
            int horizontalLineY = height() / 2;
            painter.drawLine(x, horizontalLineY, x + TREE_NODE_X_OFFSET - 5, horizontalLineY);
        } else if (!isLastChild(node)) {
            painter.drawLine(x, 0, x, height());
        }
    }
}

}    // namespace U2
