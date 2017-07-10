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

#include "McaEditor.h"
#include "McaEditorReferenceArea.h"
#include "McaEditorStatusBar.h"
#include "McaReferenceCharController.h"

#include <U2Core/DNASequenceSelection.h>
#include <U2Core/MultipleChromatogramAlignmentObject.h>
#include <U2Core/U2SafePoints.h>

#include <U2View/SequenceObjectContext.h>

#include "view_rendering/MaEditorSelection.h"
#include "view_rendering/MaEditorSequenceArea.h"

#include <QHBoxLayout>

namespace U2 {

McaEditorStatusBar::McaEditorStatusBar(MultipleAlignmentObject* mobj,
                                       MaEditorSequenceArea* seqArea,
                                       McaReferenceCharController* refCharController)
    : MaEditorStatusBar(mobj, seqArea),
      refCharController(refCharController) {
    setObjectName("mca_editor_status_bar");

    colomnLabel->setPatterns(tr("RefPos %1 / %2"),
                             tr("Reference position %1 of %2"));
    positionLabel->setPatterns(tr("ReadPos %1 of %2"),
                               tr("Read position %1 of %2"));
    selectionLabel->hide();

    updateLabels();
    setupLayout();
}

void McaEditorStatusBar::setupLayout() {
    layout->addWidget(lineLabel);
    layout->addWidget(colomnLabel);
    layout->addWidget(positionLabel);
    layout->addWidget(lockLabel);
}

void McaEditorStatusBar::updateLabels() {
    updateLinePositionLabels();

    McaEditor* editor = qobject_cast<McaEditor*>(seqArea->getEditor());
    SAFE_POINT(editor->getReferenceContext() != NULL, "Reference context is NULL", );
    DNASequenceSelection* selection = editor->getReferenceContext()->getSequenceSelection();
    SAFE_POINT(selection != NULL, "Reference selection is NULL", );

    QString ungappedRefLen = QString::number(refCharController->getUngappedLength());
    if (selection->isEmpty()) {
        colomnLabel->update(MaEditorStatusBar::NONE_MARK, ungappedRefLen);
    } else {
        int startSelection = selection->getSelectedRegions().first().startPos;
        int refPos = refCharController->getUngappedPosition(startSelection);
        colomnLabel->update(refPos == -1 ? "gap" : QString::number(refPos + 1), ungappedRefLen);
    }
}

} // namespace
