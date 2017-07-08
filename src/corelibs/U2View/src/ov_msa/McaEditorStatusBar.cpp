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

#include "McaEditorStatusBar.h"

#include <U2Core/MultipleChromatogramAlignmentObject.h>

#include <QHBoxLayout>

namespace U2 {

McaEditorStatusBar::McaEditorStatusBar(MultipleAlignmentObject* mobj, MaEditorSequenceArea* seqArea)
    : MaEditorStatusBar(mobj, seqArea) {
    setObjectName("mca_editor_status_bar");

    // SANGER_TODO: approve tooltip texts
    colomnLabel->setPatterns(tr("RefPos %1 / %2"),
                             tr("Reference position %1 / %2 (gaps excluded)"));
    positionLabel->setPatterns(tr("ReadPos %1 / %2"),
                               tr("Read position %1 / %2 (gaps excluded)"));
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

} // namespace
