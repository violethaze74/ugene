/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
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

#ifndef _U2_MSA_EDITOR_OVERVIEW_H_
#define _U2_MSA_EDITOR_OVERVIEW_H_

#include <U2Core/global.h>

#include <QAction>
#include <QWidget>

namespace U2 {

class MaEditorWgt;
class MaSimpleOverview;
class MaGraphOverview;
class MaOverviewContextMenu;

class U2VIEW_EXPORT MSAEditorOverviewArea : public QWidget {
    Q_OBJECT
public:
    MSAEditorOverviewArea(MaEditorWgt* ui);
    void cancelRendering();
    bool isOverviewWidget(QWidget* wgt) const;

    MaSimpleOverview* getSimpleOverview() const { return simpleOverview; }
    MaGraphOverview* getGraphOverview() const { return graphOverview; }

    static const QString OVERVIEW_AREA_OBJECT_NAME;
public slots:
    void sl_onContextMenuRequested(const QPoint& p);
    void sl_show();
private:
    MaSimpleOverview*  simpleOverview;
    MaGraphOverview*   graphOverview;
    MaOverviewContextMenu* contextMenu;
};

}

#endif // _U2_MSA_EDITOR_OVERVIEW_H_
