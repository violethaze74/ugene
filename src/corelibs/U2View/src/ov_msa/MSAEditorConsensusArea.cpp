/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
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

#include <U2Gui/GUIUtils.h>

#include "MSAEditor.h"
#include "MSAEditorConsensusArea.h"
#include "view_rendering/MaConsensusAreaRenderer.h"


namespace U2 {

/************************************************************************/
/* MSAEditorConsensusArea */
/************************************************************************/
MSAEditorConsensusArea::MSAEditorConsensusArea(MSAEditorUI *ui)
    : MaEditorConsensusArea(ui) {
    initRenderer();
    setupFontAndHeight();
}

void MSAEditorConsensusArea::initRenderer() {
    renderer = new MaConsensusAreaRenderer(this);
}

void MSAEditorConsensusArea::buildMenu(QMenu* m) {
    QMenu* copyMenu = GUIUtils::findSubMenu(m, MSAE_MENU_COPY);
    SAFE_POINT(copyMenu != NULL, "copyMenu", );
    copyMenu->addAction(copyConsensusAction);
    copyMenu->addAction(copyConsensusWithGapsAction);

    m->addAction(configureConsensusAction);
}

} // namespace U2
