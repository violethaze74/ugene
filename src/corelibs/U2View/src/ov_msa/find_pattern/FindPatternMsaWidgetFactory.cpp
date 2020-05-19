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

#include "FindPatternMsaWidgetFactory.h"

#include <QPixmap>

#include <U2Core/U2SafePoints.h>

#include <U2Gui/HelpButton.h>

#include "FindPatternMsaWidget.h"

namespace U2 {

const QString FindPatternMsaWidgetFactory::GROUP_ID = "OP_MSA_FIND_PATTERN_WIDGET";
const QString FindPatternMsaWidgetFactory::GROUP_ICON_STR = ":core/images/find_dialog.png";
const QString FindPatternMsaWidgetFactory::GROUP_DOC_PAGE = "39747586";

FindPatternMsaWidgetFactory::FindPatternMsaWidgetFactory() {
    objectViewOfWidget = ObjViewType_AlignmentEditor;
}

#define SEARCH_IN_NAMES_MODE_OPTION_KEY "FindPatternMsaWidgetFactory_findInNames"

QWidget *FindPatternMsaWidgetFactory::createWidget(GObjectView *objView, const QVariantMap &options) {
    SAFE_POINT(objView != nullptr,
               QString("Internal error: unable to create widget for group '%1', object view is NULL.").arg(GROUP_ID),
               nullptr);

    MSAEditor *msaEditor = qobject_cast<MSAEditor *>(objView);
    SAFE_POINT(msaEditor != nullptr,
               QString("Internal error: unable to cast object view to MSAEditor for group '%1'.").arg(GROUP_ID),
               nullptr);

    bool isSearchInNamesMode = options.value(SEARCH_IN_NAMES_MODE_OPTION_KEY).toBool();
    FindPatternMsaWidget *widget = new FindPatternMsaWidget(msaEditor, isSearchInNamesMode);
    widget->setObjectName("FindPatternMsaWidget");
    return widget;
}

OPGroupParameters FindPatternMsaWidgetFactory::getOPGroupParameters() {
    return OPGroupParameters(GROUP_ID, QPixmap(GROUP_ICON_STR), QObject::tr("Search in Alignment"), GROUP_DOC_PAGE);
}

void FindPatternMsaWidgetFactory::applyOptionsToWidget(QWidget *widget, const QVariantMap &options) {
    FindPatternMsaWidget *findPatternMsaWidget = qobject_cast<FindPatternMsaWidget *>(widget);
    CHECK(findPatternMsaWidget != nullptr, )
    bool isSearchInNamesMode = options.value(SEARCH_IN_NAMES_MODE_OPTION_KEY).toBool();
    findPatternMsaWidget->setSearchInNamesMode(isSearchInNamesMode);
}

const QString &FindPatternMsaWidgetFactory::getGroupId() {
    return GROUP_ID;
}

const QVariantMap FindPatternMsaWidgetFactory::getOptionsToActivateSearchInNames() {
    QVariantMap options;
    options[SEARCH_IN_NAMES_MODE_OPTION_KEY] = true;
    return options;
}

}    // namespace U2
