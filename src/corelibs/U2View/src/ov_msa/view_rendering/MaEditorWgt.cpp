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

#include "MaEditorWgt.h"

#include "SequenceAreaRenderer.h"

#include <U2View/MSAEditor.h>
#include <U2View/UndoRedoFramework.h>

namespace U2 {

MaEditorWgt::MaEditorWgt(MSAEditor *editor)
    : editor(editor),
      saRenderer(NULL),
      collapsibleMode(false)
{
    collapseModel = new MSACollapsibleItemModel(this);
    undoFWK = new MsaUndoRedoFramework(this, editor->getMSAObject());

    // SANGER_TODO: check why delAction is not added
    delSelectionAction = new QAction(tr("Remove selection"), this);
    delSelectionAction->setObjectName("Remove selection");
    delSelectionAction->setShortcut(QKeySequence::Delete);
    delSelectionAction->setShortcutContext(Qt::WidgetShortcut);

    copySelectionAction = new QAction(tr("Copy selection"), this);
    copySelectionAction->setObjectName("copy_selection");
    copySelectionAction->setShortcut(QKeySequence::Copy);
    copySelectionAction->setShortcutContext(Qt::WidgetShortcut);
    copySelectionAction->setToolTip(QString("%1 (%2)").arg(copySelectionAction->text())
        .arg(copySelectionAction->shortcut().toString()));

    addAction(copySelectionAction);

    copyFormattedSelectionAction = new QAction(QIcon(":core/images/copy_sequence.png"), tr("Copy formatted"), this);
    copyFormattedSelectionAction->setObjectName("copy_formatted");
    copyFormattedSelectionAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
    copyFormattedSelectionAction->setShortcutContext(Qt::WidgetShortcut);
    copyFormattedSelectionAction->setToolTip(QString("%1 (%2)").arg(copyFormattedSelectionAction->text())
        .arg(copyFormattedSelectionAction->shortcut().toString()));

    addAction(copyFormattedSelectionAction);

    pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setObjectName("paste");
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setShortcutContext(Qt::WidgetShortcut);
    pasteAction->setToolTip(QString("%1 (%2)").arg(pasteAction->text())
        .arg(pasteAction->shortcut().toString()));

    addAction(pasteAction);
}

QAction* MaEditorWgt::getUndoAction() const {
    QAction *a = undoFWK->getUndoAction();
    a->setObjectName("msa_action_undo");
    return a;
}

QAction* MaEditorWgt::getRedoAction() const {
    QAction *a = undoFWK->getRedoAction();
    a->setObjectName("msa_action_redo");
    return a;
}

} // namespace
