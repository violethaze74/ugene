/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
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

#include <U2Gui/GUIUtils.h>

#include "MSAEditor.h"
#include "MsaEditorNameList.h"

namespace U2 {

MsaEditorNameList::MsaEditorNameList(MaEditorWgt *ui, QScrollBar *nhBar)
    : MaEditorNameList(ui, nhBar)
{

}

void MsaEditorNameList::buildMenu(QMenu *menu) {
    QMenu* editMenu = GUIUtils::findSubMenu(menu, MSAE_MENU_EDIT);
    SAFE_POINT(editMenu != NULL, "editMenu not found", );

    editMenu->insertAction(editMenu->actions().last(), removeSequenceAction);

    CHECK(qobject_cast<MSAEditor*>(editor) != NULL, );
    CHECK(rect().contains(mapFromGlobal(QCursor::pos())), );

    QMenu* copyMenu = GUIUtils::findSubMenu(menu, MSAE_MENU_COPY);
    SAFE_POINT(copyMenu != NULL, "copyMenu not found", );
    copyMenu->addAction(copyCurrentSequenceAction);

    copyCurrentSequenceAction->setDisabled(getSelectedRow() == -1);
    editMenu->insertAction(editMenu->actions().first(), editSequenceNameAction);
}

MSAEditor* MsaEditorNameList::getEditor() const {
    return qobject_cast<MSAEditor*>(editor);
}

}   // namespace U2
