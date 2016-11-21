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

#include "MaEditorUtils.h"
#include "SequenceAreaRenderer.h"

#include <U2Core/GObjectTypes.h>

#include <U2View/MSAEditor.h>
#include <U2View/UndoRedoFramework.h>

#include <U2View/MSAEditorConsensusArea.h>
#include <U2View/MSAEditorNameList.h>
#include <U2View/MSAEditorSequenceArea.h>
#include <U2View/MSAEditorOffsetsView.h>
#include <U2View/MSAEditorOverviewArea.h>
#include <U2View/MSAEditorStatusBar.h>

#include <QGridLayout>

namespace U2 {

/************************************************************************/
/* MaEditorWgt */
/************************************************************************/
MaEditorWgt::MaEditorWgt(MaEditor *editor)
    : editor(editor),
      seqArea(NULL),
      nameList(NULL),
      consArea(NULL),
      overviewArea(NULL),
      offsetsView(NULL),
      statusWidget(NULL),
      nameAreaContainer(NULL),
      collapsibleMode(false),
      delSelectionAction(NULL),
      copySelectionAction(NULL),
      copyFormattedSelectionAction(NULL),
      pasteAction(NULL)
{
    collapseModel = new MSACollapsibleItemModel(this);
    undoFWK = new MsaUndoRedoFramework(this, editor->getMaObject());
}

QWidget* MaEditorWgt::createLabelWidget(const QString& text, Qt::Alignment ali){
    return new MSALabelWidget(this, text, ali);
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

void MaEditorWgt::initWidgets() {
    setContextMenuPolicy(Qt::CustomContextMenu);
    setMinimumSize(300, 200);

    setWindowIcon(GObjectTypes::getTypeInfo(GObjectTypes::MULTIPLE_SEQUENCE_ALIGNMENT).icon);

    GScrollBar* shBar = new GScrollBar(Qt::Horizontal);
    shBar->setObjectName("horizontal_sequence_scroll");
    shBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QScrollBar* nhBar = new QScrollBar(Qt::Horizontal);
    nhBar->setObjectName("horizontal_names_scroll");
    GScrollBar* cvBar = new GScrollBar(Qt::Vertical);
    cvBar->setObjectName("vertical_sequence_scroll");

    initSeqArea(shBar, cvBar);
    seqArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    // SANGER_TODO: everything in under comment until the 'out-of-memory' problem is resolved
    nameList = new MSAEditorNameList(this, nhBar);
    nameList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

//    coreLog.info("NameList success");

    consArea = new MSAEditorConsensusArea(this);
    overviewArea = new MSAEditorOverviewArea(this);
    statusWidget = new MSAEditorStatusWidget(editor->getMaObject(), seqArea);

     // SANGER_TODO: the problem with the row
//    offsetsView = new MSAEditorOffsetsViewController(this, editor, seqArea);
//    offsetsView->getLeftWidget()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
//    offsetsView->getRightWidget()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    QWidget *label;
    label = createLabelWidget(tr("Consensus"));
    label->setMinimumHeight(consArea->height());

    QWidget* label1 = createLabelWidget();
    QWidget* label2 = createLabelWidget();
    label1->setMinimumHeight(consArea->height());
    label2->setMinimumHeight(consArea->height());

    QGridLayout* seqAreaLayout = new QGridLayout();
    seqAreaLayout->setMargin(0);
    seqAreaLayout->setSpacing(0);

    seqAreaLayout->addWidget(label1, 0, 0);
    seqAreaLayout->addWidget(consArea, 0, 1);
    seqAreaLayout->addWidget(label2, 0, 2, 1, 2);

//    seqAreaLayout->addWidget(offsetsView->getLeftWidget(), 1, 0);
    seqAreaLayout->addWidget(seqArea, 1, 1);
//    seqAreaLayout->addWidget(offsetsView->getRightWidget(), 1, 2);
    seqAreaLayout->addWidget(cvBar, 1, 3);

    seqAreaLayout->addWidget(shBar, 2, 0, 1, 3);

    seqAreaLayout->setRowStretch(1, 1);
    seqAreaLayout->setColumnStretch(1, 1);

    QWidget* seqAreaContainer = new QWidget();
    seqAreaContainer->setLayout(seqAreaLayout);

    QVBoxLayout* nameAreaLayout = new QVBoxLayout();
    nameAreaLayout->setMargin(0);
    nameAreaLayout->setSpacing(0);
    nameAreaLayout->addWidget(label);
    nameAreaLayout->addWidget(nameList);
    nameAreaLayout->addWidget(nhBar);

    nameAreaContainer = new QWidget();
    nameAreaContainer->setLayout(nameAreaLayout);

    maSplitter.addWidget(nameAreaContainer, 0, 0.1);
    maSplitter.addWidget(seqAreaContainer, 1, 3);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(maSplitter.getSplitter());
    mainLayout->setStretch(0, 1);
    mainLayout->addWidget(statusWidget);
    mainLayout->addWidget(overviewArea);

    setLayout(mainLayout);

    connect(collapseModel, SIGNAL(toggled()), offsetsView, SLOT(sl_updateOffsets()));
    connect(collapseModel, SIGNAL(toggled()), seqArea,     SLOT(sl_modelChanged()));

    connect(delSelectionAction, SIGNAL(triggered()), seqArea, SLOT(sl_delCurrentSelection()));

    nameList->addAction(delSelectionAction);
}

void MaEditorWgt::initActions() {
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

} // namespace
