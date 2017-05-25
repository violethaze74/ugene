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
#include <QKeyEvent>

#include <U2Core/DNAAlphabet.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/U2SafePoints.h>

#include "MSAEditorSequenceArea.h"
#include "MSAEditorStatusBar.h"

namespace U2 {

MSAEditorStatusWidget::MSAEditorStatusWidget(MultipleAlignmentObject* mobj, MaEditorSequenceArea* sa)
    : aliObj(mobj),
      seqArea(sa),
      lockedIcon(":core/images/lock.png"),
      unlockedIcon(":core/images/lock_open.png")
{
    setObjectName("msa_editor_status_bar");
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    QLabel* findLabel = new QLabel();
    findLabel->setText(tr("Find:"));

    prevButton = new QPushButton();
    prevButton->setObjectName("Find backward");
    prevButton->setToolTip(tr("Find backward <b>(SHIFT + Enter)</b>"));
    prevButton->setIcon(QIcon(":core/images/msa_find_prev.png"));
    prevButton->setFlat(true);
    nextButton = new QPushButton();
    nextButton->setObjectName("Find forward");
    nextButton->setToolTip(tr("Find forward <b>(Enter)</b>"));
    nextButton->setIcon(QIcon(":core/images/msa_find_next.png"));
    nextButton->setFlat(true);

    searchEdit = new QLineEdit();
    searchEdit->setObjectName("searchEdit");
    //searchEdit->setMinimumWidth(200);
    searchEdit->installEventFilter(this);
    searchEdit->setMaxLength(1000);
    validator = new MSASearchValidator(mobj->getAlphabet(), this);
    searchEdit->setValidator(validator);
    findLabel->setBuddy(searchEdit);

    linesLabel = new QLabel();
    linesLabel->setObjectName("Line");
    linesLabel->setAlignment(Qt::AlignCenter);
    colsLabel = new QLabel();
    colsLabel->setObjectName("Column");
    colsLabel->setAlignment(Qt::AlignCenter);
    posLabel = new QLabel();
    posLabel->setObjectName("Position");
    posLabel->setAlignment(Qt::AlignCenter);

    selectionLabel = new QLabel();
    selectionLabel->setObjectName("Selection");
    selectionLabel->setAlignment(Qt::AlignCenter);

    lockLabel = new QLabel();

    QHBoxLayout* l = new QHBoxLayout();
    l->setMargin(2);
    l->addStretch(1);
    l->addWidget(findLabel);
    l->addWidget(prevButton);
    l->addWidget(searchEdit);
    l->addWidget(nextButton);
//    l->addStretch(1);
    l->addWidget(linesLabel);
    l->addWidget(colsLabel);
    l->addWidget(posLabel);
    if (qobject_cast<MSAEditor*>(seqArea->getEditor()) != NULL) {
        l->addWidget(selectionLabel);
    }
    l->addWidget(lockLabel);
    setLayout(l);

    connect(seqArea, SIGNAL(si_selectionChanged(const MaEditorSelection& , const MaEditorSelection& )),
        SLOT(sl_selectionChanged(const MaEditorSelection& , const MaEditorSelection&)));
    connect(mobj, SIGNAL(si_alignmentChanged(const MultipleAlignment&, const MaModificationInfo&)),
        SLOT(sl_alignmentChanged()));
    connect(mobj, SIGNAL(si_lockedStateChanged()), SLOT(sl_lockStateChanged()));
    connect(mobj, SIGNAL(si_alphabetChanged(const MaModificationInfo&, const DNAAlphabet *)), SLOT(sl_alphabetChanged()));

    connect(prevButton, SIGNAL(clicked()), SLOT(sl_findPrev()));
    connect(nextButton, SIGNAL(clicked()), SLOT(sl_findNext()));

    findAction = new QAction(tr("Find in alignment"), this);//this action is used only to enable shortcut to change focus today
    findAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
    findAction->setShortcutContext(Qt::WindowShortcut);
    connect(findAction, SIGNAL(triggered()), SLOT(sl_findFocus()));
    addAction(findAction);

    updateCoords();
    updateLock();
}

void MSAEditorStatusWidget::updateLock() {
    bool locked = aliObj->isStateLocked();
    lockLabel->setPixmap(locked ? lockedIcon : unlockedIcon);
    lockLabel->setToolTip(locked ? tr("Alignment object is locked") : tr("Alignment object is not locked"));
}

void MSAEditorStatusWidget::updateCoords() {
    const QPoint& pos = seqArea->getSelection().topLeft();
    int aliLen = aliObj->getLength();
    int nSeq = aliObj->getNumRows();
    QFontMetrics fm(linesLabel->font(),this);

    QString lpattern = QString(tr("Ln %1 / %2"));
    qint64 shownLine = seqArea->isAlignmentEmpty() ? pos.y() : pos.y() + 1;
    QString ltext = lpattern.arg(shownLine).arg(nSeq);
    linesLabel->setText(ltext);
    linesLabel->setToolTip(tr("Line %1 of %2").arg(pos.y() + 1).arg(nSeq));
    linesLabel->setMinimumWidth(10 + fm.width(lpattern.arg(nSeq).arg(nSeq)));

    QString cpattern = QString(tr("Col %1 / %2"));
    qint64 shownCol = seqArea->isAlignmentEmpty() ? pos.x() : pos.x() + 1;
    QString ctext = cpattern.arg(shownCol).arg(aliLen);
    colsLabel->setText(ctext);
    colsLabel->setToolTip(tr("Column %1 of %2").arg(pos.x() + 1).arg(aliLen));
    colsLabel->setMinimumWidth(10 + fm.width(cpattern.arg(aliLen).arg(aliLen)));

    QPair<QString, int> pp = seqArea->getGappedColumnInfo(); // SANGER_TODO: the method is used only here, move to this class
    QString ppattern = QString(tr("Pos %1 / %2"));
    QString ptext = ppattern.arg(pp.first).arg(pp.second);
    posLabel->setText(ptext);
    posLabel->setToolTip(tr("Position %1 of %2").arg(pp.first).arg(pp.second));
    posLabel->setMinimumWidth(10 + fm.width(ppattern.arg(pp.second).arg(pp.second)));

    QString selectionPattern = QString(tr("Sel %1"));
    QString selectionToolTipPattern = tr("Selection width and height are %1");
    MaEditorSelection selection = seqArea->getSelection();
    QString selSize;
    QString noneSelection = tr("none");
    if (selection.isNull()) {
        selSize = noneSelection;
    } else {
        selSize = QString::number(selection.width()) + "x" + QString::number(selection.height());
    }

    selectionLabel->setText(selectionPattern.arg(selSize));
    selectionLabel->setToolTip(selectionToolTipPattern.arg(selSize));
    MaEditor *editor = seqArea->getEditor();
    CHECK(editor != NULL, );
    int maxSelLength = fm.width(selectionPattern.arg(QString::number(editor->getAlignmentLen()) + "x" +
        QString::number(editor->getNumSequences())));
    int nonSelLength = fm.width(selectionPattern.arg(noneSelection));
    selectionLabel->setMinimumWidth(10 + qMax(maxSelLength, nonSelLength));
}

bool MSAEditorStatusWidget::eventFilter(QObject*, QEvent* ev) {
    if (ev->type() == QEvent::KeyPress) {
        QKeyEvent* kev = (QKeyEvent*)ev;
        if (kev->key() == Qt::Key_Enter || kev->key() == Qt::Key_Return) {
            if (kev->modifiers() == Qt::SHIFT) {
                prevButton->click();
            } else {
                nextButton->click();
            }
        } else if (kev->key() == Qt::Key_Escape) {
            seqArea->setFocus();
        }
    }
    return false;
}

void MSAEditorStatusWidget::sl_findNext( ) {
    if (seqArea->isAlignmentEmpty()) {
        return;
    }

    QByteArray pat = searchEdit->text( ).toLocal8Bit( );
    if ( pat.isEmpty( ) ) {
        return;
    }
    const MultipleAlignment msa = aliObj->getMultipleAlignment();
    if ( !msa->getAlphabet( )->isCaseSensitive( ) ) {
        pat = pat.toUpper( );
    }
    const int aliLen = msa->getLength( );
    const int nSeq = seqArea->getNumDisplayedSequences( );
    QPoint selectionTopLeft = seqArea->getSelection( ).topLeft( );

    if ( selectionTopLeft == lastSearchPos ) {
        selectionTopLeft.setX( selectionTopLeft.x( ) + 1 );
    }
    for (int s = selectionTopLeft.y(); s < nSeq; s++) {
        const U2Region rowsAtPosition = seqArea->getRowsAt( s );
        SAFE_POINT( 0 <= rowsAtPosition.startPos, "Invalid row number!", );
        const MultipleAlignmentRow row = msa->getRow( rowsAtPosition.startPos );
        // if s == pos.y -> search from the current base, otherwise search from the seq start
        int p = ( s == selectionTopLeft.y( ) ) ? selectionTopLeft.x( ) : 0;
        for ( ; p < ( aliLen - pat.length( ) + 1 ); p++ ) {
            char c = row->charAt( p );
            int selLength = 0;
            if ( U2Msa::GAP_CHAR != c && MSAUtils::equalsIgnoreGaps(row, p, pat, selLength) ) {
                // select the result now
                MaEditorSelection sel( p, s, selLength, 1 );
                seqArea->setSelection( sel, true );
                seqArea->centerPos( sel.topLeft( ) );
                lastSearchPos = seqArea->getSelection( ).topLeft( );
                return;
            }
        }
    }
}

void MSAEditorStatusWidget::sl_findPrev( ) {
    if (seqArea->isAlignmentEmpty()) {
        return;
    }

    QByteArray pat = searchEdit->text( ).toLocal8Bit( );
    if ( pat.isEmpty( ) ) {
        return;
    }
    const MultipleAlignment msa = aliObj->getMultipleAlignment();
    if ( !msa->getAlphabet( )->isCaseSensitive( ) ) {
        pat = pat.toUpper( );
    }
    int aliLen = msa->getLength( );
    QPoint pos = seqArea->getSelection( ).topLeft( );
    if ( pos == lastSearchPos ) {
        pos.setX( pos.x( ) - 1 );
    }
    for ( int s = pos.y( ); 0 <= s; s-- ) {
        const U2Region rowsAtPosition = seqArea->getRowsAt( s );
        SAFE_POINT( 0 <= rowsAtPosition.startPos, "Invalid row number!", );
        const MultipleAlignmentRow row = msa->getRow( rowsAtPosition.startPos );
        //if s == pos.y -> search from the current base, otherwise search from the seq end
        int p = ( s == pos.y( ) ? pos.x( ) : ( aliLen - pat.length( ) + 1) );
        while ( 0 <= p ) {
            int selectionLength = 0;
            if ( U2Msa::GAP_CHAR != row->charAt( p )
                && MSAUtils::equalsIgnoreGaps( row, p, pat, selectionLength ) )
            {
                // select the result now
                MaEditorSelection sel( p, s, selectionLength, 1 );
                seqArea->setSelection( sel, true );
                seqArea->centerPos( sel.topLeft( ) );
                lastSearchPos = seqArea->getSelection( ).topLeft( );
                return;
            }
            p--;
        }
    }
}

void MSAEditorStatusWidget::sl_findFocus() {
    searchEdit->setFocus();
}

void MSAEditorStatusWidget::sl_alphabetChanged(){
    if (!aliObj->getAlphabet()->isRaw()){
        QByteArray alphabetChars = aliObj->getAlphabet()->getAlphabetChars(true);
        //remove special characters
        alphabetChars.remove(alphabetChars.indexOf('*'), 1);
        alphabetChars.remove(alphabetChars.indexOf('-'), 1);
        validator->setRegExp(QRegExp(QString("[%1]+").arg(alphabetChars.constData())));
    }else{
        validator->setRegExp(QRegExp(".*"));
    }

    //check is pattern clean required
    QString currentPattern = QString(searchEdit->text());
    int pos = 0;
    if(validator->validate(currentPattern, pos) != QValidator::Acceptable){
        searchEdit->clear();
    }
}

MSASearchValidator::MSASearchValidator(const DNAAlphabet* alphabet, QObject *parent)
: QRegExpValidator(parent)
{
    if (!alphabet->isRaw()){
        QByteArray alphabetChars = alphabet->getAlphabetChars(true);
        //remove special characters
        alphabetChars.remove(alphabetChars.indexOf('*'), 1);
        alphabetChars.remove(alphabetChars.indexOf('-'), 1);
        setRegExp(QRegExp(QString("[%1]+").arg(alphabetChars.constData())));
    }
}

QValidator::State MSASearchValidator::validate(QString &input, int &pos) const {
    input = input.simplified();
    input = input.toUpper();
    input.remove(" ");
    input.remove("-"); // Gaps are not used in search model
    return QRegExpValidator::validate(input, pos);
}

}//namespace

