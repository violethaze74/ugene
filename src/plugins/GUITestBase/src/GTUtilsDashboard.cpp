/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2019 UniPro <ugene@unipro.ru>
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

#include <QRegularExpression>
#include <QTabWidget>
#include <QWebElement>
#include <QWebFrame>
#include <QWebView>

#include <primitives/GTWebView.h>
#include <primitives/GTWidget.h>

#include "GTUtilsDashboard.h"

namespace U2 {
using namespace HI;

#define GT_CLASS_NAME "GTUtilsDashboard"
QMap<QString, GTUtilsDashboard::Tabs> GTUtilsDashboard::initTabMap(){
    QMap<QString, GTUtilsDashboard::Tabs> result;
    result.insert("Overview", GTUtilsDashboard::Overview);
    result.insert("Input", GTUtilsDashboard::Input);
    result.insert("External Tools", GTUtilsDashboard::ExternalTools);
    return result;
}

QString GTUtilsDashboard::getNodeSpanId(const QString &nodeId) {
    // It is defined in ExternalToolsWidget.js.
    return nodeId + "_span";
}

HIWebElement GTUtilsDashboard::getCopyButton(GUITestOpStatus &os, const QString &toolRunNodeId) {
    const QString selector = QString("SPAN#%1 > BUTTON").arg(getNodeSpanId(toolRunNodeId));

    GTGlobals::FindOptions options;
    options.searchInHidden = true;

    return GTWebView::findElementBySelector(os, getDashboard(os), selector, options);
}

HIWebElement GTUtilsDashboard::getNodeSpan(GUITestOpStatus &os, const QString &nodeId) {
    const QString selector = QString("SPAN#%1").arg(getNodeSpanId(nodeId));

    GTGlobals::FindOptions options;
    options.searchInHidden = true;

    return GTWebView::findElementBySelector(os, getDashboard(os), selector, options);
}

HIWebElement GTUtilsDashboard::getNodeUl(GUITestOpStatus &os, const QString &nodeId) {
    const QString selector = QString("UL#%1").arg(nodeId);

    GTGlobals::FindOptions options;
    options.searchInHidden = true;

    return GTWebView::findElementBySelector(os, getDashboard(os), selector, options);
}

const QMap<QString, GTUtilsDashboard::Tabs> GTUtilsDashboard::tabMap = initTabMap();
const QString GTUtilsDashboard::TREE_ROOT_ID = "treeRoot";
const QString GTUtilsDashboard::PARENT_LI = "parent_li";

const QString GTUtilsDashboard::WIDTH = "width";
const QString GTUtilsDashboard::HEIGHT = "height";
const QString GTUtilsDashboard::TITLE = "title";
const QString GTUtilsDashboard::COLLAPSED_NODE_TITLE = "Expand this branch";
const QString GTUtilsDashboard::ON_CLICK = "onclick";

QWebView* GTUtilsDashboard::getDashboard(HI::GUITestOpStatus &os){
    return GTWidget::findExactWidget<QWebView*>(os, "Dashboard");
}

QTabWidget* GTUtilsDashboard::getTabWidget(HI::GUITestOpStatus &os){
    return GTWidget::findExactWidget<QTabWidget*>(os, "WorkflowTabView");
}

QStringList GTUtilsDashboard::getOutputFiles(HI::GUITestOpStatus &os) {
    const QString selector = "button.btn.full-width.long-text";
    const QList<HIWebElement> outputFilesButtons = GTWebView::findElementsBySelector(os, getDashboard(os), selector);
    QStringList outputFilesNames;
    foreach (const HIWebElement &outputFilesButton, outputFilesButtons) {
        const QString outputFileName = outputFilesButton.toPlainText();
        if (!outputFileName.isEmpty()) {
            outputFilesNames << outputFileName;
        }
    }
    return outputFilesNames;
}

#define GT_METHOD_NAME "clickOutputFile"
void GTUtilsDashboard::clickOutputFile(GUITestOpStatus &os, const QString &outputFileName) {
    const QString selector = "button.btn.full-width.long-text";
    const QList<HIWebElement> outputFilesButtons = GTWebView::findElementsBySelector(os, getDashboard(os), selector);
    foreach (const HIWebElement &outputFilesButton, outputFilesButtons) {
        QString buttonText = outputFilesButton.toPlainText();
        if (buttonText == outputFileName) {
            click(os, outputFilesButton);
            return;
        }

        if (buttonText.endsWith("...")) {
            buttonText.chop(QString("...").length());
            if (!buttonText.isEmpty() && outputFileName.startsWith(buttonText)) {
                click(os, outputFilesButton);
                return;
            }
        }
    }

    GT_CHECK(false, QString("The output file with name '%1' not found").arg(outputFileName));
}
#undef GT_METHOD_NAME

HIWebElement GTUtilsDashboard::findElement(HI::GUITestOpStatus &os, QString text, QString tag, bool exactMatch){
    return GTWebView::findElement(os, getDashboard(os), text, tag, exactMatch);
}

HIWebElement GTUtilsDashboard::findTreeElement(HI::GUITestOpStatus &os, QString text){
    return GTWebView::findTreeElement(os, getDashboard(os), text);
}

HIWebElement GTUtilsDashboard::findContextMenuElement(HI::GUITestOpStatus &os, QString text){
    return GTWebView::findContextMenuElement(os, getDashboard(os), text);
}

void GTUtilsDashboard::click(HI::GUITestOpStatus &os, HIWebElement el, Qt::MouseButton button){
    GTWebView::click(os, getDashboard(os), el, button);
}

bool GTUtilsDashboard::areThereNotifications(HI::GUITestOpStatus &os) {
    openTab(os, Overview);
    return GTWebView::doesElementExist(os, getDashboard(os), "Notifications", "DIV", true);
}

#define GT_METHOD_NAME "openTab"
void GTUtilsDashboard::openTab(HI::GUITestOpStatus &os, Tabs tab){
    HIWebElement el = GTWebView::findElement(os, getDashboard(os), tabMap.key(tab), "A");
    GTWebView::click(os, getDashboard(os), el);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "doesTabExist"
bool GTUtilsDashboard::doesTabExist(HI::GUITestOpStatus &os, Tabs tab) {
    return GTWebView::doesElementExist(os, getDashboard(os), tabMap.key(tab), "A");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getRootNode"
QString GTUtilsDashboard::getRootNode(GUITestOpStatus &os) {
    return GTWebView::findElementById(os, getDashboard(os), TREE_ROOT_ID).id();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getNodeText"
QString GTUtilsDashboard::getNodeText(GUITestOpStatus &os, const QString &nodeId) {
    return getNodeSpan(os, nodeId).toPlainText();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getChildrenNodesCount"
int GTUtilsDashboard::getChildrenNodesCount(GUITestOpStatus &os, const QString &nodeId) {
    const QString selector = QString("UL#%1 > LI.%2 > UL").arg(nodeId).arg(PARENT_LI);

    GTGlobals::FindOptions options;
    options.failIfNotFound = false;
    options.searchInHidden = true;

    return GTWebView::findElementsBySelector(os, getDashboard(os), selector, options).size();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getChildNodeId"
QString GTUtilsDashboard::getChildNodeId(GUITestOpStatus &os, const QString &nodeId, int childNum) {
    return getDescendantNodeId(os, nodeId, { childNum });
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getDescendantNodeId"
QString GTUtilsDashboard::getDescendantNodeId(GUITestOpStatus &os, const QString &nodeId, const QList<int> &childNums) {
    QString selector = QString("UL#%1").arg(nodeId);
    foreach (const int childNum, childNums) {
        selector += QString(" > LI:nth-of-type(%1) > UL").arg(childNum + 1);
    }

    GTGlobals::FindOptions options;
    options.searchInHidden = true;

    return GTWebView::findElementBySelector(os, getDashboard(os), selector, options).id();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getCopyButtonSize"
QSize GTUtilsDashboard::getCopyButtonSize(GUITestOpStatus &os, const QString &toolRunNodeId) {
    return getCopyButton(os, toolRunNodeId).geometry().size();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickCopyButton"
void GTUtilsDashboard::clickCopyButton(GUITestOpStatus &os, const QString &toolRunNodeId) {
    click(os, getCopyButton(os, toolRunNodeId));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isNodeVisible"
bool GTUtilsDashboard::isNodeVisible(GUITestOpStatus &os, const QString &nodeId) {
    const HIWebElement nodeSpanElement = getNodeSpan(os, nodeId);
    return nodeSpanElement.geometry().isValid();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isNodeCollapsed"
bool GTUtilsDashboard::isNodeCollapsed(GUITestOpStatus &os, const QString &nodeId) {
    const HIWebElement nodeSpanElement = getNodeSpan(os, nodeId);
    return nodeSpanElement.attribute(TITLE, "") == COLLAPSED_NODE_TITLE;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "collapseNode"
void GTUtilsDashboard::collapseNode(GUITestOpStatus &os, const QString &nodeId) {
    GT_CHECK(isNodeVisible(os, nodeId),
             QString("SPAN of the node with ID '%1' is not visible. Some of the parent nodes are collapsed?").arg(nodeId));

    GT_CHECK(!isNodeCollapsed(os, nodeId),
             QString("UL of the node with ID '%1' is not visible. It is already collapsed.").arg(nodeId));

    click(os, getNodeSpan(os, nodeId));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "expandNode"
void GTUtilsDashboard::expandNode(GUITestOpStatus &os, const QString &nodeId) {
    GT_CHECK(isNodeVisible(os, nodeId),
             QString("SPAN of the node with ID '%1' is not visible. Some of the parent nodes are collapsed?").arg(nodeId));

    GT_CHECK(isNodeCollapsed(os, nodeId),
             QString("UL of the node with ID '%1' is visible. It is already expanded.").arg(nodeId));

    click(os, getNodeSpan(os, nodeId));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getLogFileUrlFromOutputNode"
QString GTUtilsDashboard::getLogFileUrlFromOutputNode(GUITestOpStatus &os, const QString &outputNodeId) {
    const QString logFileLinkSelector = QString("SPAN#%1 A").arg(getNodeSpanId(outputNodeId));
    const QString onclickFunction = GTWebView::findElementBySelector(os, getDashboard(os), logFileLinkSelector).attribute(ON_CLICK);
    QRegularExpression urlFetcher("openLog\\(\\\'(.*)\\\'\\)");
    const QRegularExpressionMatch match = urlFetcher.match(onclickFunction);
    GT_CHECK_RESULT(match.hasMatch(),
                    QString("Can't get URL with a regexp from a string: regexp is '%1', string is '%2'")
                    .arg(urlFetcher.pattern()).arg(logFileLinkSelector), QString());
    return match.captured(1);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
