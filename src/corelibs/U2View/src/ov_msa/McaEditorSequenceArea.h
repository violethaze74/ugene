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

#ifndef _U2_MCA_EDITOR_SEQUENCE_AREA_
#define _U2_MCA_EDITOR_SEQUENCE_AREA_

#include "view_rendering/MaEditorSequenceArea.h"
#include "McaEditor.h"
#include <U2Gui/ScaleBar.h>

namespace U2 {

class McaEditor;

struct ChromatogramViewSettings {
    ChromatogramViewSettings()  {
        drawTraceA = true;
        drawTraceC = true;
        drawTraceG = true;
        drawTraceT = true;
    }
    bool drawTraceA;
    bool drawTraceC;
    bool drawTraceG;
    bool drawTraceT;
};

class McaEditorSequenceArea : public MaEditorSequenceArea {
    Q_OBJECT
public:
    McaEditorSequenceArea(MaEditorWgt* ui, GScrollBar* hb, GScrollBar* vb);

    McaEditor* getEditor() const { return qobject_cast<McaEditor*>(editor); }

    const ChromatogramViewSettings&  getSettings() const { return settings; }
    bool getShowQA() const {return showQVAction->isChecked(); }

    void deleteCurrentSelection() {}

private slots:
    void sl_showHideTrace();
    void sl_showAllTraces();
    void sl_setRenderAreaHeight(int k);

    void sl_buildStaticToolbar(GObjectView* v, QToolBar* t);

private:
    void initRenderer();
    void updateActions() {}

    void buildMenu(QMenu* m);

    QAction* createToggleTraceAction(const QString& actionName);

private:
    ChromatogramViewSettings    settings;

    QAction*    showQVAction;
    QAction*    showAllTraces;
    QMenu*      traceActionMenu;
    ScaleBar*   scaleBar;
};


} // namespace

#endif // _U2_MCA_EDITOR_SEQUENCE_AREA_

