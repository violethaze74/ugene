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

#include "MsaEditorSortSequencesWidget.h"

#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <U2Core/U2OpStatusUtils.h>

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorSequenceArea.h>

namespace U2 {

MsaEditorSortSequencesWidget::MsaEditorSortSequencesWidget(QWidget *parent, MSAEditor *msaEditor)
    : QWidget(parent), msaEditor(msaEditor) {
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    QLabel *sortByLabel = new QLabel();
    sortByLabel->setText(tr("Sort by"));
    layout->addWidget(sortByLabel);

    sortByCombo = new QComboBox();
    sortByCombo->setObjectName("sortByComboBox");
    sortByCombo->addItem(tr("Name"));
    sortByCombo->addItem(tr("Length"));
    layout->addWidget(sortByCombo);

    QLabel *sortOrderLabel = new QLabel();
    sortOrderLabel->setText(tr("Sort order"));
    layout->addWidget(sortOrderLabel);

    sortOrderCombo = new QComboBox();
    sortOrderCombo->setObjectName("sortOrderComboBox");
    sortOrderCombo->addItem(tr("Ascending"));
    sortOrderCombo->addItem(tr("Descending"));
    layout->addWidget(sortOrderCombo);

    QHBoxLayout *lastRowLayout = new QHBoxLayout();
    layout->addLayout(lastRowLayout);
    lastRowLayout->addStretch(1);

    sortButton = new QPushButton(tr("Sort"));
    sortButton->setObjectName("sortButton");
    sortButton->setToolTip(tr("Click to sort sequence in the alignment"));
    lastRowLayout->addWidget(sortButton);
    connect(sortButton, SIGNAL(clicked()), SLOT(sl_sortClicked()));

    MultipleSequenceAlignmentObject *msaObject = msaEditor->getMaObject();
    sortButton->setEnabled(!msaObject->isStateLocked());
    connect(msaObject, SIGNAL(si_lockedStateChanged()), SLOT(sl_msaObjectStateChanged()));
}

void MsaEditorSortSequencesWidget::sl_sortClicked() {
    MultipleSequenceAlignmentObject *msaObject = msaEditor->getMaObject();
    if (msaObject->isStateLocked()) {
        return;
    }
    MultipleSequenceAlignment msa = msaObject->getMultipleAlignmentCopy();
    MultipleAlignment::Order sortOrder = sortOrderCombo->currentIndex() == 0 ? MultipleAlignment::Ascending : MultipleAlignment::Descending;
    if (sortByCombo->currentIndex() == 0) {
        msa->sortRowsByName(sortOrder);
    } else {
        msa->sortRowsByLength(sortOrder);
    }

    // Drop collapsing mode.
    msaEditor->getUI()->getSequenceArea()->sl_setCollapsingMode(false);

    QStringList rowNames = msa->getRowNames();
    if (rowNames != msaObject->getMultipleAlignment()->getRowNames()) {
        U2OpStatusImpl os;
        msaObject->updateRowsOrder(os, msa->getRowsIds());
        SAFE_POINT_OP(os, );
    }
}

void MsaEditorSortSequencesWidget::sl_msaObjectStateChanged() {
    MultipleSequenceAlignmentObject *msaObject = msaEditor->getMaObject();
    sortButton->setEnabled(!msaObject->isStateLocked());
}

}    // namespace U2
