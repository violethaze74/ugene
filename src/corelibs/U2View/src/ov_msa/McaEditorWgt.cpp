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

#include <QApplication>

#include <U2Algorithm/BuiltInConsensusAlgorithms.h>
#include <U2Algorithm/MSAConsensusAlgorithm.h>
#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>

#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>

#include "MaConsensusMismatchController.h"
#include "McaEditor.h"
#include "McaEditorConsensusArea.h"
#include "McaEditorNameList.h"
#include "McaEditorOverviewArea.h"
#include "McaEditorReferenceArea.h"
#include "McaEditorSequenceArea.h"
#include "McaEditorStatusBar.h"
#include "McaEditorWgt.h"
#include "McaReferenceCharController.h"
#include "MSACollapsibleModel.h"
#include "helpers/McaRowHeightController.h"
#include "ov_sequence/SequenceObjectContext.h"

namespace U2 {

#define TOP_INDENT 10

McaEditorWgt::McaEditorWgt(McaEditor *editor)
    : MaEditorWgt(editor)
{
    rowHeightController = new McaRowHeightController(this);
    refCharController = new McaReferenceCharController(this, editor);

    initActions();
    initWidgets();

    refArea = new McaEditorReferenceArea(this, getEditor()->getReferenceContext());
    connect(refArea, SIGNAL(si_selectionChanged()), statusBar, SLOT(sl_update()));
    seqAreaHeaderLayout->insertWidget(0, refArea);

    MaEditorConsensusAreaSettings consSettings;
    consSettings.visibleElements = MSAEditorConsElement_CONSENSUS_TEXT | MSAEditorConsElement_RULER;
    consSettings.highlightMismatches = true;
    consArea->setDrawSettings(consSettings);

    MSAConsensusAlgorithmFactory* algoFactory = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(BuiltInConsensusAlgorithms::SIMPLE_EXTENDED_ALGO);
    consArea->setConsensusAlgorithm(algoFactory);

    QString name = getEditor()->getReferenceContext()->getSequenceObject()->getSequenceName();
    QWidget *refName = createHeaderLabelWidget(tr("Reference %1:").arg(name),
                                               Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter), refArea);
    refName->setObjectName("reference label container widget");

    nameAreaLayout->insertWidget(0, refName);

    QVector<U2Region> itemRegions;
    for (int i = 0; i < editor->getNumSequences(); i++) {
        itemRegions << U2Region(i, 1);
    }

    collapseModel->setTrivialGroupsPolicy(MSACollapsibleItemModel::Allow);
    collapseModel->reset(itemRegions);
    collapseModel->collapseAll(false);
    collapseModel->setFakeCollapsibleModel(true);
    collapsibleMode = true;

    McaEditorConsensusArea* mcaConsArea = qobject_cast<McaEditorConsensusArea*>(consArea);
    SAFE_POINT(mcaConsArea != NULL, "Failed to cast consensus area to MCA consensus area", );
    seqAreaHeaderLayout->setContentsMargins(0, TOP_INDENT, 0, 0);
    seqAreaHeader->setStyleSheet("background-color: white;");
    connect(mcaConsArea->getMismatchController(), SIGNAL(si_selectMismatch(int)), refArea, SLOT(sl_selectMismatch(int)));
}

McaEditor *McaEditorWgt::getEditor() const {
    return qobject_cast<McaEditor *>(editor);
}

McaEditorConsensusArea *McaEditorWgt::getConsensusArea() const {
    return qobject_cast<McaEditorConsensusArea *>(consArea);
}

McaEditorSequenceArea* McaEditorWgt::getSequenceArea() const {
    return qobject_cast<McaEditorSequenceArea*>(seqArea);
}

McaReferenceCharController* McaEditorWgt::getRefCharController() const {
    return refCharController;
}

QAction *McaEditorWgt::getClearSelectionAction() const {
    return clearSelectionAction;
}

void McaEditorWgt::initActions() {
    MaEditorWgt::initActions();

    clearSelectionAction = new QAction(tr("Clear selection"), this);
    clearSelectionAction->setShortcut(Qt::Key_Escape);
    connect(clearSelectionAction, SIGNAL(triggered()), SIGNAL(si_clearSelection()));
    addAction(clearSelectionAction);

    delSelectionAction->setText(tr("Remove character/gap"));
}

void McaEditorWgt::initSeqArea(GScrollBar* shBar, GScrollBar* cvBar) {
    seqArea = new McaEditorSequenceArea(this, shBar, cvBar);
}

void McaEditorWgt::initOverviewArea() {
    overviewArea = new McaEditorOverviewArea(this);
}

void McaEditorWgt::initNameList(QScrollBar* nhBar) {
    nameList = new McaEditorNameList(this, nhBar);
}

void McaEditorWgt::initConsensusArea() {
    consArea = new McaEditorConsensusArea(this);
}

void McaEditorWgt::initStatusBar() {
    statusBar = new McaEditorStatusBar(editor->getMaObject(), seqArea, refCharController);
}

}   // namespace U2
