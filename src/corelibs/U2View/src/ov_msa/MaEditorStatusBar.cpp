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

#include <QHBoxLayout>

#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/U2SafePoints.h>

#include "MSAEditorSequenceArea.h"
#include "MaEditorStatusBar.h"

namespace U2 {

MaEditorStatusBar::TwoArgPatternLabel::TwoArgPatternLabel(QString textPattern, QString tooltipPattern,
                                                          QString objectName, QWidget* parent)
    : QLabel(textPattern, parent),
      textPattern(textPattern),
      tooltipPattern(tooltipPattern),
      fm(QFontMetrics(font(),this)) {
    setObjectName(objectName);
    setAlignment(Qt::AlignCenter);
}

MaEditorStatusBar::TwoArgPatternLabel::TwoArgPatternLabel(QString objectName, QWidget* parent)
    : QLabel(parent),
      fm(QFontMetrics(font(),this)) {
    setObjectName(objectName);
    setAlignment(Qt::AlignCenter);
}

void MaEditorStatusBar::TwoArgPatternLabel::setPatterns(QString textPattern, QString tooltipPattern) {
    this->textPattern = textPattern;
    this->tooltipPattern = tooltipPattern;
}

void MaEditorStatusBar::TwoArgPatternLabel::update(QString firstArg, QString secondArg) {
    setText(textPattern.arg(firstArg, secondArg));
    setToolTip(tooltipPattern.arg(firstArg, secondArg));
    setMinimumWidth(10 + fm.width(textPattern.arg(secondArg).arg(secondArg)));
}

MaEditorStatusBar::MaEditorStatusBar(MultipleAlignmentObject* mobj, MaEditorSequenceArea* sa)
    : aliObj(mobj),
      seqArea(sa),
      lockedIcon(":core/images/lock.png"),
      unlockedIcon(":core/images/lock_open.png")
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    selectionPattern = tr("Sel %1");
    lineLabel = new TwoArgPatternLabel(tr("Ln %1 / %2"), tr("Line %1 of %2"), "Line", this);
    colomnLabel = new TwoArgPatternLabel(tr("Col %1 / %2"), tr("Column %1 of %2"), "Column", this);
    positionLabel = new TwoArgPatternLabel(tr("Pos %1 / %2"), tr("Position %1 of %2"), "Position", this);
    selectionLabel = new TwoArgPatternLabel(selectionPattern, tr("Selection width and height are %1"), "Selection", this);

    lockLabel = new QLabel();

    layout = new QHBoxLayout();
    layout->setMargin(2);
    layout->addStretch(1);
    setLayout(layout);

    connect(seqArea, SIGNAL(si_selectionChanged(const MaEditorSelection& , const MaEditorSelection& )),
            SLOT(sl_selectionChanged(const MaEditorSelection& , const MaEditorSelection&)));
    connect(mobj, SIGNAL(si_alignmentChanged(const MultipleAlignment&, const MaModificationInfo&)),
            SLOT(sl_alignmentChanged()));
    connect(mobj, SIGNAL(si_lockedStateChanged()), SLOT(sl_lockStateChanged()));

    updateLock();
}

void MaEditorStatusBar::sl_alignmentChanged() {
    updateLabels();
}

void MaEditorStatusBar::sl_lockStateChanged() {
    updateLock();
}

void MaEditorStatusBar::sl_selectionChanged(const MaEditorSelection& , const MaEditorSelection& ) {
    updateLabels();
}

const char NONE_MARK = '-';
const QString NONE_SELECTION = MaEditorStatusBar::tr("none");

void MaEditorStatusBar::updateLabels() {
    MaEditorSelection selection = seqArea->getSelection();
    bool emptySelection = selection.isEmpty();
    const QPoint& pos = seqArea->getSelection().topLeft();

    int aliLen = aliObj->getLength();
    int nSeq = aliObj->getNumRows();

    lineLabel->update(QString::number(emptySelection ? NONE_MARK : pos.y() + 1),
                      QString::number(nSeq));
    colomnLabel->update(QString::number(emptySelection ? NONE_MARK : pos.x() + 1),
                        QString::number(aliLen));
    QPair<QString, int> pp = getGappedColumnInfo(pos); // refactor
    positionLabel->update(pp.first, QString(pp.second));


    // selection label
    QString selSize;
    if (emptySelection) {
        selSize = NONE_SELECTION;
    } else {
        selSize = QString::number(selection.width()) + "x" + QString::number(selection.height());
    }
    selectionLabel->update(selSize, QString());// second should provide max size

    QFontMetrics fm(lineLabel->font(),this);
    int maxSelLength = fm.width(selectionPattern.arg(QString::number(aliLen) + "x" + QString::number(nSeq)));
    int nonSelLength = fm.width(selectionPattern.arg(NONE_SELECTION));
    selectionLabel->setMinimumWidth(10 + qMax(maxSelLength, nonSelLength));
}

void MaEditorStatusBar::updateLock() {
    bool locked = aliObj->isStateLocked();
    lockLabel->setPixmap(locked ? lockedIcon : unlockedIcon);
    lockLabel->setToolTip(locked ? tr("Alignment object is locked") : tr("Alignment object is not locked"));
}

// rename and recfactror
QPair<QString, int> MaEditorStatusBar::getGappedColumnInfo(const QPoint& pos) const{
    if (pos.isNull()) {
        // for an empty alignment?
        return QPair<QString, int>(QString::number(0), 0);
    }

    QPair<QString, int> p;
    MSAEditor* editor = qobject_cast<MSAEditor*>(seqArea->getEditor());
    CHECK(editor != NULL, p); // SANGER_TODO: no ungappedLen and ungappedPosition for MCA


    // collapsing mode should be taken into account
    const MultipleSequenceAlignmentRow row = qobject_cast<MSAEditor*>(editor)->getMaObject()->getMsaRow(pos.y()/*getSelectedRows().startPos*/);
    int len = row->getUngappedLength();
    QChar current = row->charAt(pos.x());
    if(current == U2Msa::GAP_CHAR){
        return QPair<QString, int>(QString("gap"),len);
    } else{
        return QPair<QString, int>(QString::number(row->getUngappedPosition(pos.x()) + 1),len);
    }
}


}//namespace

