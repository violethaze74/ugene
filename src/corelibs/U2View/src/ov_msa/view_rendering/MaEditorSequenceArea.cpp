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

#include "MaEditorSequenceArea.h"

#include "MaEditorWgt.h"
#include "../Highlighting/MsaSchemesMenuBuilder.h"
#include "SequenceAreaRenderer.h"
#include "../MaEditor.h"
#include "../MSACollapsibleModel.h"

#include <U2Algorithm/MsaHighlightingScheme.h>
#include <U2Algorithm/MsaColorScheme.h>

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/L10n.h>
#include <U2Core/MultipleAlignmentObject.h>
#include <U2Core/Settings.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2Mod.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/GScrollBar.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/OptionsPanel.h>

#include <U2View/MSAHighlightingTabFactory.h>

#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QRubberBand>

namespace U2 {

MaEditorSequenceArea::MaEditorSequenceArea(MaEditorWgt *ui, GScrollBar *hb, GScrollBar *vb)
    : editor(ui->getEditor()),
      ui(ui),
      colorScheme(NULL),
      highlightingScheme(NULL),
      shBar(hb),
      svBar(vb),
      editModeAnimationTimer(this),
      prevPressedButton(Qt::NoButton),
      msaVersionBeforeShifting(-1),
      useDotsAction(NULL),
      replaceCharacterAction(NULL),
      changeTracker(editor->getMaObject()->getEntityRef())
{
    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    msaMode = ViewMode;

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setMinimumSize(100, 100);
    startPos = 0;
    startSeq = 0;
    highlightSelection = false;
    selecting = false;
    shifting = false;
    editingEnabled = false;

    cachedView = new QPixmap();
    completeRedraw = true;

    replaceCharacterAction = new QAction(tr("Replace selected character"), this);
    replaceCharacterAction->setObjectName("replace_selected_character");
    replaceCharacterAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_R));
    replaceCharacterAction->setShortcutContext(Qt::WidgetShortcut);
    addAction(replaceCharacterAction);
    connect(replaceCharacterAction, SIGNAL(triggered()), SLOT(sl_replaceSelectedCharacter()));

    fillWithGapsinsSymAction = new QAction(tr("Fill selection with gaps"), this);
    fillWithGapsinsSymAction->setObjectName("fill_selection_with_gaps");
    connect(fillWithGapsinsSymAction, SIGNAL(triggered()), SLOT(sl_fillCurrentSelectionWithGaps()));
    addAction(fillWithGapsinsSymAction);

    QAction* undoAction = ui->getUndoAction();
    QAction* redoAction = ui->getRedoAction();
    addAction(undoAction);
    addAction(redoAction);

    connect(editor, SIGNAL(si_completeUpdate()), SLOT(sl_completeUpdate()));
    connect(editor, SIGNAL(si_buildStaticMenu(GObjectView*, QMenu*)), SLOT(sl_buildStaticMenu(GObjectView*, QMenu*)));
    connect(editor, SIGNAL(si_buildStaticToolbar(GObjectView*, QToolBar*)), SLOT(sl_buildStaticToolbar(GObjectView*, QToolBar*)));
    connect(editor, SIGNAL(si_buildPopupMenu(GObjectView* , QMenu*)), SLOT(sl_buildContextMenu(GObjectView*, QMenu*)));
    connect(editor, SIGNAL(si_zoomOperationPerformed(bool)), SLOT(sl_completeUpdate()));
    // SANGER_TODO: why is it commented?
//    connect(editor, SIGNAL(si_fontChanged(QFont)), SLOT(sl_fontChanged(QFont)));

    connect(&editModeAnimationTimer, SIGNAL(timeout()), SLOT(sl_changeSelectionColor()));

    connect(editor->getMaObject(), SIGNAL(si_alignmentChanged(const MultipleAlignment&, const MaModificationInfo&)),
        SLOT(sl_alignmentChanged(const MultipleAlignment&, const MaModificationInfo&)));
}

MaEditorSequenceArea::~MaEditorSequenceArea() {
    exitFromEditCharacterMode();
    delete cachedView;
    deleteOldCustomSchemes();
    delete highlightingScheme;
}

int MaEditorSequenceArea::countWidthForBases(bool countClipped, bool forOffset) const {
    int seqAreaWidth = width();
    int colWidth = editor->getColumnWidth();
    int nVisible = seqAreaWidth / colWidth;

    if(countClipped) {
        bool colIsVisible = ((float)(seqAreaWidth % colWidth) / colWidth < 0.5) ? 0 : 1;
        colIsVisible |= !forOffset;
        nVisible += colIsVisible && (seqAreaWidth % colWidth != 0);
    }
    return nVisible;
}

int MaEditorSequenceArea::getFirstVisibleBase() const {
    return startPos;
}


int MaEditorSequenceArea::getLastVisibleBase(bool countClipped, bool forOffset) const {
    if (isAlignmentEmpty()) {
        return 0;
    }

    int nVisible = countWidthForBases(countClipped, forOffset);
    int alignLen = editor->getAlignmentLen();
    int res = qBound(0, startPos + nVisible - 1, alignLen - 1);
    return res;
}

int MaEditorSequenceArea::getNumVisibleBases(bool countClipped, bool forOffset) const {
    if (isAlignmentEmpty()) {
        return 0;
    }

    int lastVisible = getLastVisibleBase(countClipped, forOffset);
    SAFE_POINT((startPos <= lastVisible || (!countClipped && lastVisible + 1 == startPos /*1 symbol is visible & clipped*/)),
               tr("Last visible base is less than startPos"), 0);
    SAFE_POINT(lastVisible < editor->getAlignmentLen(), tr("Last visible base is out of range"), 0);
    int res = lastVisible - startPos + 1;
    return res;
}

U2Region MaEditorSequenceArea::getBaseXRange(int pos, bool useVirtualCoords) const {
    return getBaseXRange(pos, startPos, useVirtualCoords);
}

U2Region MaEditorSequenceArea::getBaseXRange(int pos, int firstVisiblePos, bool useVirtualCoords) const {
    U2Region res(editor->getColumnWidth() * (pos - firstVisiblePos), editor->getColumnWidth());
    if (!useVirtualCoords) {
        int w = width();
        res = res.intersect(U2Region(0, w));
    }
    return res;
}

int MaEditorSequenceArea::getColumnNumByX(int x, bool selecting) const {
    int colOffs = x / editor->getColumnWidth();
    int pos = startPos + colOffs;
    if (!selecting) {
        if ((pos >= editor->getAlignmentLen()) || (pos < 0)) {
            return -1;
        }
    }
    else {
        if (pos < 0) {
            pos = 0;
        }
        if (pos >= editor->getAlignmentLen()) {
            pos = editor->getAlignmentLen() - 1;
        }
    }
    return pos;

}

int MaEditorSequenceArea::getXByColumnNum(int columnNum) const {
    return (columnNum + 0.5f)*editor->getColumnWidth();
}


void MaEditorSequenceArea::setFirstVisibleBase(int pos) {
    if (pos == startPos) {
        return;
    }

    if (!isAlignmentEmpty()) {
        SAFE_POINT(isPosInRange(pos), tr("Position is out of range: %1").arg(QString::number(pos)), );

        QPoint prev(startPos, startSeq);

        int aliLen = editor->getAlignmentLen();
        int effectiveFirst = qMin(aliLen - countWidthForBases(false), pos);
        startPos = qMax(0, effectiveFirst);

        QPoint p(startPos, startSeq);
        emit si_startChanged(p, prev);
    } else {
        startPos = -1;
    }

    updateHScrollBar();

    completeRedraw = true;
    update();
}

int MaEditorSequenceArea::countHeightForSequences(bool countClipped) const {
    int seqAreaHeight = height();
    int nVisible = seqAreaHeight / editor->getRowHeight() + (countClipped && (seqAreaHeight % editor->getRowHeight() != 0) ? 1 : 0);
    return nVisible;
}

int MaEditorSequenceArea::getFirstVisibleSequence() const {
    return startSeq;
}

int MaEditorSequenceArea::getLastVisibleSequence(bool countClipped) const {
    if (isAlignmentEmpty()) {
        return 0;
    }

    int nVisible = countHeightForSequences(countClipped);
    int numSeqs = getNumDisplayedSequences();
    int res = qBound(0, startSeq + nVisible - 1, numSeqs - 1);
    return res;
}

int MaEditorSequenceArea::getNumVisibleSequences(bool countClipped) const {
    if (isAlignmentEmpty()) {
        return 0;
    }

    int lastVisible =  getLastVisibleSequence(countClipped);
    SAFE_POINT(startSeq <= lastVisible, tr("Last visible sequence is less than startSeq"), 0);
    SAFE_POINT(lastVisible < editor->getNumSequences(), tr("Last visible sequence is out of range"), 0);

    int sequencesNumber = 0;
    if (ui->isCollapsibleMode()) {
        QVector<U2Region> range;
        ui->getCollapseModel()->getVisibleRows(startSeq, lastVisible, range);
        foreach(U2Region region, range) {
            sequencesNumber += region.length;
        }
        return sequencesNumber;
    }
    else {
        sequencesNumber = lastVisible - startSeq + 1;
        return sequencesNumber;
    }
}

int MaEditorSequenceArea::getNumDisplayedSequences() const {
    if (isAlignmentEmpty()) {
        return 0;
    }

    MSACollapsibleItemModel *model = ui->getCollapseModel();
    SAFE_POINT(NULL != model, tr("Invalid collapsible item model!"), -1);
    return model->displayedRowsCount();
}

U2Region MaEditorSequenceArea::getSequenceYRange(int seqNum, bool useVirtualCoords) const {
    return getSequenceYRange(seqNum, startSeq, useVirtualCoords);
}

U2Region MaEditorSequenceArea::getSequenceYRange(int seq, int firstVisibleRow, bool useVirtualCoords) const {
    U2Region res(editor->getRowHeight()* (seq - firstVisibleRow), editor->getRowHeight());
    if (!useVirtualCoords) {
        int h = height();
        res = res.intersect(U2Region(0, h));
    }
    return res;
}

U2Region MaEditorSequenceArea::getSequenceYRange(int startSeq, int count) const {
    return U2Region(getSequenceYRange(startSeq, true).startPos, count * editor->getRowHeight());
}

int MaEditorSequenceArea::getSequenceNumByY(int y) const {
    int seqOffs = y / editor->getRowHeight();
    int seq = startSeq + seqOffs;
    const int countOfVisibleSeqs = getNumDisplayedSequences();
    if (!selecting) {
        if ((seqOffs >= countOfVisibleSeqs) || (seq < 0)) {
            return -1;
        }
    }
    else {
        if (seq < 0) {
            seq = 0;
        }
        if (seq >= countOfVisibleSeqs) {
            seq = countOfVisibleSeqs - 1;
        }
    }
    return seq;
}

int MaEditorSequenceArea::getYBySequenceNum(int sequenceNum) const {
    return (sequenceNum + 0.5f)*editor->getRowHeight();
}

void MaEditorSequenceArea::setFirstVisibleSequence(int seq) {
    if (seq == startSeq) {
        return;
    }

    if (!isAlignmentEmpty()) {
        SAFE_POINT(isSeqInRange(seq), tr("Sequence is out of range: %1").arg(QString::number(seq)), );

        QPoint prev(startPos, startSeq);

        int nSeq = getNumDisplayedSequences();
        int effectiveFirst = qMin(nSeq - countHeightForSequences(false), seq);
        startSeq = qMax(0, effectiveFirst);

        QPoint p(startPos, startSeq);
        emit si_startChanged(p, prev);
    }

    updateVScrollBar();

    completeRedraw = true;
    update();
}

U2Region MaEditorSequenceArea::getRowsAt(int pos) const {
    if (!ui->isCollapsibleMode()) {
        return U2Region(pos, 1);
    }

    MSACollapsibleItemModel* m = ui->getCollapseModel();
    int itemIdx = m->itemAt(pos);
    if (itemIdx >= 0) {
        const MSACollapsableItem& item = m->getItem(itemIdx);
        return U2Region(item.row, item.numRows);
    }
    return U2Region(m->mapToRow(pos), 1);
}

QPair<QString, int> MaEditorSequenceArea::getGappedColumnInfo() const{
    QPair<QString, int> p;
    CHECK(getEditor() != NULL, p);
    CHECK(qobject_cast<MSAEditor*>(editor) != NULL, p); // SANGER_TODO: no ungappedLen and ungappedPosition for MCA
    if (isAlignmentEmpty()) {
        return QPair<QString, int>(QString::number(0), 0);
    }

    const MultipleSequenceAlignmentRow row = qobject_cast<MSAEditor*>(editor)->getMaObject()->getMsaRow(getSelectedRows().startPos);
    int len = row->getUngappedLength();
    QChar current = row->charAt(selection.topLeft().x());
    if(current == U2Msa::GAP_CHAR){
        return QPair<QString, int>(QString("gap"),len);
    }else{
        int pos = row->getUngappedPosition(selection.topLeft().x());
        return QPair<QString, int>(QString::number(pos + 1),len);
    }
}

bool MaEditorSequenceArea::isAlignmentEmpty() const {
    return editor->isAlignmentEmpty();
}

bool MaEditorSequenceArea::isPosInRange(int p) const {
    return p >= 0 && p < editor->getAlignmentLen();
}

bool MaEditorSequenceArea::isSeqInRange(int s) const {
    return s >= 0 && s < getNumDisplayedSequences();
}

bool MaEditorSequenceArea::isInRange(const QPoint& p) const {
    return isPosInRange(p.x()) && isSeqInRange(p.y());
}

bool MaEditorSequenceArea::isVisible(const QPoint& p, bool countClipped) const {
    return isPosVisible(p.x(), countClipped) && isSeqVisible(p.y(), countClipped);
}

bool MaEditorSequenceArea::isPosVisible(int pos, bool countClipped) const {
    if (pos != 0 && (pos < getFirstVisibleBase() || pos > getLastVisibleBase(countClipped))) {
        return false;
    }
    return true;
}

bool MaEditorSequenceArea::isSeqVisible(int seq, bool countClipped) const {
    if (seq < getFirstVisibleSequence() || seq > getLastVisibleSequence(countClipped)) {
        return false;
    }
    return true;
}

int MaEditorSequenceArea::coordToPos(int x) const {
    int y = getSequenceYRange(getFirstVisibleSequence(), false).startPos;
    return coordToPos(QPoint(x, y)).x();
}

QPoint MaEditorSequenceArea::coordToPos(const QPoint& coord) const {
    QPoint res(-1, -1);
    //Y: row
    int lastSeq = getLastVisibleSequence(true);
    if (ui->isCollapsibleMode()) {
        lastSeq = getNumDisplayedSequences();
    }
    for (int i = getFirstVisibleSequence(); i <= lastSeq; i++) {
        U2Region r = getSequenceYRange(i, false);
        if (r.contains(coord.y())) {
            res.setY(i);
            break;
        }
    }

    //X: position in sequence
    for (int i=getFirstVisibleBase(), n = getLastVisibleBase(true); i<=n; i++) {
        U2Region r = getBaseXRange(i, false);
        if (r.contains(coord.x())) {
            res.setX(i);
            break;
        }
    }
    return res;
}

QPoint MaEditorSequenceArea::coordToAbsolutePos(const QPoint& coord) const {
    int column = getColumnNumByX(coord.x(), selecting);
    int row = getSequenceNumByY(coord.y());

    return QPoint(column, row);
}

QPoint MaEditorSequenceArea::coordToAbsolutePosOutOfRange(const QPoint& coord) const {
    CHECK(editor->getColumnWidth() > 0, QPoint(0, 0));
    CHECK(editor->getRowHeight() > 0, QPoint(0, 0));
    int column = startPos + (coord.x() / editor->getColumnWidth());
    int row = startSeq + (coord.y() / editor->getRowHeight());

    return QPoint(column, row);
}

const MaEditorSelection & MaEditorSequenceArea::getSelection() const {
    SAFE_POINT(checkState(), "Invalid alignment state", selection);
    return selection;
}

void MaEditorSequenceArea::updateSelection(const QPoint& newPos) {
    int width = qAbs(newPos.x() - cursorPos.x()) + 1;
    int height = qAbs(newPos.y() - cursorPos.y()) + 1;
    int left = qMin(newPos.x(), cursorPos.x());
    int top = qMin(newPos.y(), cursorPos.y());

    MaEditorSelection s(left, top, width, height);
    if (newPos.x()!=-1 && newPos.y()!=-1) {
        setSelection(s);
    }
    bool selectionExists = !selection.isNull();
    ui->getCopySelectionAction()->setEnabled(selectionExists);
    ui->getCopyFormattedSelectionAction()->setEnabled(selectionExists);
    emit si_copyFormattedChanging(selectionExists);
}

void MaEditorSequenceArea::updateSelection() {
    CHECK(!baseSelection.isNull(), );

    if (!ui->isCollapsibleMode()) {
        setSelection(baseSelection);
        return;
    }
    MSACollapsibleItemModel* m = ui->getCollapseModel();
    CHECK_EXT(NULL != m, cancelSelection(), );

    int startPos = baseSelection.y();
    int endPos = startPos + baseSelection.height();

    // convert selected rows indexes to indexes of selected collapsible items
    int newStart = m->rowToMap(startPos);
    int newEnd = m->rowToMap(endPos);

    SAFE_POINT_EXT(newStart >= 0 && newEnd >= 0, cancelSelection(), );

    int selectionHeight = newEnd - newStart;
    // accounting of collapsing children items
    int itemIndex = m->itemAt(newEnd);
    if (selectionHeight <= 1 && itemIndex >= 0) {
        const MSACollapsableItem& collapsibleItem = m->getItem(itemIndex);
        if(newEnd == collapsibleItem.row && !collapsibleItem.isCollapsed) {
            newEnd = collapsibleItem.row ;
            selectionHeight = qMax(selectionHeight, endPos - newStart + collapsibleItem.numRows);
        }
    }
    if(selectionHeight > 0 && newStart + selectionHeight <= m->displayedRowsCount()) {
        MaEditorSelection s(selection.topLeft().x(), newStart, selection.width(), selectionHeight);
        setSelection(s);
    } else {
        cancelSelection();
    }
}

void MaEditorSequenceArea::setSelection(const MaEditorSelection& s, bool newHighlightSelection) {
    // TODO: assert(isInRange(s));
    exitFromEditCharacterMode();
    if (highlightSelection != newHighlightSelection) {
        highlightSelection = newHighlightSelection;
        update();
    }
    if (s == selection) {
        return;
    }

    MaEditorSelection prevSelection = selection;
    selection = s;

    int selEndPos = s.x() + s.width() - 1;
    int ofRange = selEndPos - editor->getAlignmentLen();
    if (ofRange >= 0) {
        selection = MaEditorSelection(s.topLeft(), s.width() - ofRange - 1, s.height());
    }

    bool selectionExists = !selection.isNull();
    ui->getCopySelectionAction()->setEnabled(selectionExists);
    ui->getCopyFormattedSelectionAction()->setEnabled(selectionExists);
    emit si_copyFormattedChanging(selectionExists);

    U2Region selectedRowsRegion = getSelectedRows();
    baseSelection = MaEditorSelection(selection.topLeft().x(), getSelectedRows().startPos, selection.width(), selectedRowsRegion.length);

    QStringList selectedRowNames;
    for (int x = selectedRowsRegion.startPos; x < selectedRowsRegion.endPos(); x++) {
        selectedRowNames.append(editor->getMaObject()->getRow(x)->getName());
    }
    emit si_selectionChanged(selectedRowNames);
    emit si_selectionChanged(selection, prevSelection);
    update();
    updateActions();

    const QPoint topLeft = selection.topLeft();
    if (!selection.isNull() && !isVisible(topLeft, false)) {
        if (isVisible(topLeft, true)) {
            if (selection.x() - prevSelection.x() != 0) {
                setFirstVisibleBase(startPos + selection.x() - prevSelection.x());
            }
            if (selection.y() - prevSelection.y() != 0) {
                setFirstVisibleSequence(qMin(getFirstVisibleSequence() + selection.y() - prevSelection.y(), getNumDisplayedSequences() - getNumVisibleSequences(true)));
            }
        } else {
            if (selection.x() - prevSelection.x() != 0) {
                setFirstVisibleBase(topLeft.x());
            }
            if (selection.y() - prevSelection.y() != 0) {
                setFirstVisibleSequence(topLeft.y());
            }
        }
    }
}

void MaEditorSequenceArea::moveSelection(int dx, int dy, bool allowSelectionResize) {
    int leftX = selection.x();
    int topY = selection.y();
    int bottomY = selection.y() + selection.height() - 1;
    int rightX = selection.x() + selection.width() - 1;
    QPoint baseTopLeft(leftX, topY);
    QPoint baseBottomRight(rightX,bottomY);

    QPoint newTopLeft = baseTopLeft + QPoint(dx,dy);
    QPoint newBottomRight = baseBottomRight + QPoint(dx,dy);

    if ((!isInRange(newTopLeft)) || (!isInRange(newBottomRight))) {
        if (!allowSelectionResize) {
            return;
        } else {
            MaEditorSelection newSelection(selection.topLeft(),
                                            qMin(selection.width(), editor->getAlignmentLen() - newTopLeft.x()),
                                            qMin(selection.height(), editor->getNumSequences() - newTopLeft.y()));
            setSelection(newSelection);
        }
    }

    MaEditorSelection newSelection(newTopLeft, selection.width(), selection.height());
    setSelection(newSelection);
}

void MaEditorSequenceArea::cancelSelection() {
    MaEditorSelection emptySelection;
    setSelection(emptySelection);
}

U2Region MaEditorSequenceArea::getSelectedRows() const {
    return ui->getCollapseModel()->mapSelectionRegionToRows(U2Region(selection.y(), selection.height()));
}

int MaEditorSequenceArea::getHeight(){
    return editor->getRowHeight() * (getNumVisibleSequences(true) - 1);
}

QString MaEditorSequenceArea::getCopyFormatedAlgorithmId() const{
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + SETTINGS_COPY_FORMATTED, BaseDocumentFormats::CLUSTAL_ALN).toString();
}

void MaEditorSequenceArea::setCopyFormatedAlgorithmId(const QString& algoId){
    AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_COPY_FORMATTED, algoId);
}


void MaEditorSequenceArea::deleteCurrentSelection() {
    CHECK(getEditor() != NULL, );
    CHECK(!selection.isNull(), );

    assert(isInRange(selection.topLeft()));
    assert(isInRange(QPoint(selection.x() + selection.width() - 1, selection.y() + selection.height() - 1)));
    MultipleAlignmentObject* maObj = getEditor()->getMaObject();
    if (maObj == NULL || maObj->isStateLocked()) {
        return;
    }

    const QRect areaBeforeSelection(0, 0, selection.x(), selection.height());
    const QRect areaAfterSelection(selection.x() + selection.width(), selection.y(),
        maObj->getLength() - selection.x() - selection.width(), selection.height());
    if (maObj->isRegionEmpty(areaBeforeSelection.x(), areaBeforeSelection.y(), areaBeforeSelection.width(), areaBeforeSelection.height())
        && maObj->isRegionEmpty(areaAfterSelection.x(), areaAfterSelection.y(), areaAfterSelection.width(), areaAfterSelection.height())
        && selection.height() == maObj->getNumRows())
    {
        return;
    }

    // if this method was invoked during a region shifting
    // then shifting should be canceled
    cancelShiftTracking();

    U2OpStatusImpl os;
    U2UseCommonUserModStep userModStep(maObj->getEntityRef(), os);
    Q_UNUSED(userModStep);
    SAFE_POINT_OP(os, );

    const U2Region& sel = getSelectedRows();
    maObj->removeRegion(selection.x(), sel.startPos, selection.width(), sel.length, true);

    if (selection.height() == 1 && selection.width() == 1) {
        if (isInRange(selection.topLeft())) {
            return;
        }
    }
    cancelSelection();
}

bool MaEditorSequenceArea::shiftSelectedRegion(int shift) {
    CHECK(shift != 0, true);

    // shifting of selection
    MultipleAlignmentObject *maObj = editor->getMaObject();
    if (!maObj->isStateLocked()) {
        const U2Region rows = getSelectedRows();
        const int x = selection.x();
        const int y = rows.startPos;
        const int width = selection.width();
        const int height = rows.length;
        if (maObj->isRegionEmpty(x, y, width, height)) {
            return true;
        }
        // backup current selection for the case when selection might disappear
        const MaEditorSelection selectionBackup = selection;

        const int resultShift = maObj->shiftRegion(x, y, width, height, shift);
        if (0 != resultShift) {
            int newCursorPosX = (cursorPos.x() + resultShift >= 0) ? cursorPos.x() + resultShift : 0;
            setCursorPos(newCursorPosX);

            const MaEditorSelection newSelection(selectionBackup.x() + resultShift, selectionBackup.y(),
                selectionBackup.width(), selectionBackup.height());
            setSelection(newSelection);
            if ((selectionBackup.getRect().right() == getLastVisibleBase(false) && resultShift > 0)
                || (selectionBackup.x() == getFirstVisibleBase() && 0 > resultShift))
            {
                setFirstVisibleBase(startPos + resultShift);
            }
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void MaEditorSequenceArea::centerPos(const QPoint& pos) {
    assert(isInRange(pos));
    int newStartPos = qMax(0, pos.x() - getNumVisibleBases(false)/2);
    setFirstVisibleBase(newStartPos);

    int newStartSeq = qMax(0, pos.y() - getNumVisibleSequences(false)/2);
    setFirstVisibleSequence(newStartSeq);
}

void MaEditorSequenceArea::centerPos(int pos) {
    centerPos(QPoint(pos, cursorPos.y()));
}


void MaEditorSequenceArea::updateHBarPosition(int base, bool repeatAction) {
    if (isAlignmentEmpty()) {
        shBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
        return;
    }

    if (base <= getFirstVisibleBase()) {
        ( repeatAction ? shBar->setupRepeatAction(QAbstractSlider::SliderSingleStepSub, 50, 10)
                       : shBar->triggerAction(QAbstractSlider::SliderSingleStepSub) );
    } else  if (base >= getLastVisibleBase(true)) {
        ( repeatAction ? shBar->setupRepeatAction(QAbstractSlider::SliderSingleStepAdd, 50, 10)
                       : shBar->triggerAction(QAbstractSlider::SliderSingleStepAdd) );
    } else {
        shBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
    }
}

void MaEditorSequenceArea::updateVBarPosition(int seq, bool repeatAction) {
    if (isAlignmentEmpty()) {
        svBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
        return;
    }

    if (seq <= getFirstVisibleSequence()) {
        ( repeatAction ? svBar->setupRepeatAction(QAbstractSlider::SliderSingleStepSub, 50, 10)
                       : svBar->triggerAction(QAbstractSlider::SliderSingleStepSub) );
    } else if (seq >= getLastVisibleSequence(true)) {
        ( repeatAction ? svBar->setupRepeatAction(QAbstractSlider::SliderSingleStepAdd, 50, 10)
                       : svBar->triggerAction(QAbstractSlider::SliderSingleStepAdd) );
    } else {
        svBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
    }
}

void MaEditorSequenceArea::onVisibleRangeChanged() {
    exitFromEditCharacterMode();
    qint64 firstVisibleSeq = getFirstVisibleSequence();
    qint64 lastVisibleSeq  = getLastVisibleSequence(true);

    QStringList rowNames = editor->getMaObject()->getMultipleAlignment()->getRowNames();
    QStringList visibleSeqs;

    if (!isAlignmentEmpty()) {
        QVector<U2Region> range;
        if (ui->isCollapsibleMode()) {
            ui->getCollapseModel()->getVisibleRows(firstVisibleSeq, lastVisibleSeq, range);
        } else {
            range.append(U2Region(firstVisibleSeq, lastVisibleSeq - firstVisibleSeq + 1));
        }

        foreach(const U2Region& region, range) {
            int start = region.startPos;
            int end = static_cast<int>(qMin(region.endPos(), lastVisibleSeq));
            for (int seq = start; seq <= end; seq++) {
                visibleSeqs.append(rowNames.at(seq));
            }
        }
    }

    emit si_visibleRangeChanged(visibleSeqs, getHeight());
}

bool MaEditorSequenceArea::isAlignmentLocked() {
    MultipleAlignmentObject* obj = editor->getMaObject();
    SAFE_POINT(NULL != obj, tr("Alignment object is not available"), true);
    return obj->isStateLocked();
}

void MaEditorSequenceArea::drawVisibleContent(QPainter& p) {
    drawContent(p, QRect(startPos, getFirstVisibleSequence(), getNumVisibleBases(false), getNumVisibleSequences(true)));
}

bool MaEditorSequenceArea::drawContent(QPainter &p, const QRect &area) {
    QVector<U2Region> range;
    if (ui->isCollapsibleMode()) {
        ui->getCollapseModel()->getVisibleRows(area.y(), area.bottom(), range);
    } else {
        range.append(U2Region(area.y(), area.height()));
    }

    QList <qint64> seqIdx;
    foreach(U2Region region, range) {
        for (qint64 i = region.startPos; i < region.endPos(); i++) {
            seqIdx.append(i);
        }
    }
    bool ok = renderer->drawContent(p, U2Region(area.x(), area.width()), seqIdx);
    emit si_visibleRangeChanged();

    return ok;
}

bool MaEditorSequenceArea::drawContent(QPainter &p, const U2Region &region, const QList<qint64> &seqIdx) {
    // SANGER_TODO: optimize
    return renderer->drawContent(p, region, seqIdx);
}

bool MaEditorSequenceArea::drawContent(QPainter &p) {
    qint64 seqNum = editor->getNumSequences();
    if (ui->isCollapsibleMode()) {
        seqNum = ui->getCollapseModel()->rowToMap(seqNum);
    }
    return drawContent(p, QRect(0, 0, editor->getAlignmentLen(), seqNum));
}

bool MaEditorSequenceArea::drawContent(QPixmap &pixmap) {
    CHECK(editor->getColumnWidth() * editor->getAlignmentLen() < 32768 &&
           editor->getRowHeight() * editor->getNumSequences() < 32768, false);

    qint64 seqNum = editor->getNumSequences();
    if (ui->isCollapsibleMode()) {
        seqNum = ui->getCollapseModel()->rowToMap(seqNum);
    }
    pixmap = QPixmap(editor->getColumnWidth() * editor->getAlignmentLen(),
                      editor->getRowHeight() * seqNum);
    QPainter p(&pixmap);
    return drawContent(p, QRect(0, 0, editor->getAlignmentLen(), seqNum));
}

bool MaEditorSequenceArea::drawContent(QPixmap &pixmap,
                                          const U2Region &region,
                                          const QList<qint64> &seqIdx) {
    CHECK(!region.isEmpty(), false);
    CHECK(!seqIdx.isEmpty(), false);

    CHECK(editor->getColumnWidth() * region.length < 32768 &&
           editor->getRowHeight() * seqIdx.size() < 32768, false);
    pixmap = QPixmap(editor->getColumnWidth() * region.length,
                     editor->getRowHeight() * seqIdx.size());
    QPainter p(&pixmap);
    return drawContent(p, region, seqIdx);
}

void MaEditorSequenceArea::highlightCurrentSelection()  {
    highlightSelection = true;
    update();
}

QString MaEditorSequenceArea::exportHighlighting(int startPos, int endPos, int startingIndex, bool keepGaps, bool dots, bool transpose) {
    CHECK(getEditor() != NULL, QString());
    CHECK(qobject_cast<MSAEditor*>(editor) != NULL, QString());
    SAFE_POINT(editor->getReferenceRowId() != U2MsaRow::INVALID_ROW_ID, "Export highlighting is not supported without a reference", QString());
    QStringList result;

    MultipleAlignmentObject* maObj = editor->getMaObject();
    assert(maObj!=NULL);

    const MultipleAlignment msa = maObj->getMultipleAlignment();

    U2OpStatusImpl os;
    const int refSeq = getEditor()->getMaObject()->getMultipleAlignment()->getRowIndexByRowId(editor->getReferenceRowId(), os);
    SAFE_POINT_OP(os, QString());
    MultipleAlignmentRow row = msa->getRow(refSeq);

    QString header;
    header.append("Position\t");
    QString refSeqName = editor->getReferenceRowName();
    header.append(refSeqName);
    header.append("\t");
    foreach(QString name, maObj->getMultipleAlignment()->getRowNames()){
        if(name != refSeqName){
            header.append(name);
            header.append("\t");
        }
    }
    header.remove(header.length()-1,1);
    result.append(header);

    int posInResult = startingIndex;

    for (int pos = startPos-1; pos < endPos; pos++) {
        QString rowStr;
        rowStr.append(QString("%1").arg(posInResult));
        rowStr.append(QString("\t") + QString(msa->charAt(refSeq, pos)) + QString("\t"));
        bool informative = false;
        for (int seq = 0; seq < msa->getNumRows(); seq++) {  //FIXME possible problems when sequences have moved in view
            if (seq == refSeq) continue;
            char c = msa->charAt(seq, pos);

            const char refChar = row->charAt(pos);
            if (refChar == '-' && !keepGaps) {
                continue;
            }

            QColor unused;
            bool highlight = false;
            highlightingScheme->setUseDots(useDotsAction->isChecked());
            highlightingScheme->process(refChar, c, unused, highlight, pos, seq);

            if (highlight) {
                rowStr.append(c);
                informative = true;
            } else {
                if (dots) {
                    rowStr.append(".");
                } else {
                    rowStr.append(" ");
                }
            }
            rowStr.append("\t");
        }
        if(informative){
            header.remove(rowStr.length() - 1, 1);
            result.append(rowStr);
        }
        posInResult++;
    }

    if (!transpose){
        QStringList transposedRows = TextUtils::transposeCSVRows(result, "\t");
        return transposedRows.join("\n");
    }

    return result.join("\n");
}

MsaColorScheme * MaEditorSequenceArea::getCurrentColorScheme() const {
    return colorScheme;
}

MsaHighlightingScheme * MaEditorSequenceArea::getCurrentHighlightingScheme() const {
    return highlightingScheme;
}

bool MaEditorSequenceArea::getUseDotsCheckedState() const {
    return useDotsAction->isChecked();
}

void MaEditorSequenceArea::sl_changeColorSchemeOutside(const QString &id) {
    QAction* a = GUIUtils::findActionByData(QList<QAction*>() << colorSchemeMenuActions << customColorSchemeMenuActions << highlightingSchemeMenuActions, id);
    if (a != NULL) {
        a->trigger();
    }
}

void MaEditorSequenceArea::sl_changeCopyFormat(const QString& alg){
    setCopyFormatedAlgorithmId(alg);
}

void MaEditorSequenceArea::sl_changeColorScheme() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (NULL == action) {
        action = GUIUtils::getCheckedAction(customColorSchemeMenuActions);
    }
    CHECK(NULL != action, );

    applyColorScheme(action->data().toString());
}

void MaEditorSequenceArea::sl_delCurrentSelection() {
    emit si_startMsaChanging();
    deleteCurrentSelection();
    emit si_stopMsaChanging(true);
}

void MaEditorSequenceArea::sl_fillCurrentSelectionWithGaps() {
    if(!isAlignmentLocked()) {
        emit si_startMsaChanging();
        insertGapsBeforeSelection();
        emit si_stopMsaChanging(true);
    }
}

void MaEditorSequenceArea::sl_buildStaticMenu(GObjectView*, QMenu* m) {
    buildMenu(m);
}

void MaEditorSequenceArea::sl_buildStaticToolbar(GObjectView* , QToolBar* ) {

}

void MaEditorSequenceArea::sl_buildContextMenu(GObjectView*, QMenu* m) {
    buildMenu(m);
}

void MaEditorSequenceArea::sl_alignmentChanged(const MultipleAlignment &, const MaModificationInfo &) {
    exitFromEditCharacterMode();
    int nSeq = editor->getNumSequences();
    int aliLen = editor->getAlignmentLen();
    //! SANGER_TODO
//    if (ui->isCollapsibleMode()) {
//        nSeq = getNumDisplayedSequences();
//        updateCollapsedGroups(modInfo);
//    }

    editor->updateReference();

    //todo: set in one method!
    setFirstVisibleBase(qBound(0, startPos, aliLen-countWidthForBases(false)));
    setFirstVisibleSequence(qBound(0, startSeq, nSeq - countHeightForSequences(false)));

    if ((selection.x() > aliLen - 1) || (selection.y() > nSeq - 1)) {
        cancelSelection();
    } else {
        const QPoint selTopLeft(qMin(selection.x(), aliLen - 1),
            qMin(selection.y(), nSeq - 1));
        const QPoint selBottomRight(qMin(selection.x() + selection.width() - 1, aliLen - 1),
            qMin(selection.y() + selection.height() - 1, nSeq -1));

        MaEditorSelection newSelection(selTopLeft, selBottomRight);
        // we don't emit "selection changed" signal to avoid redrawing
        setSelection(newSelection);
    }

    updateHScrollBar();
    updateVScrollBar();

    completeRedraw = true;
    updateActions();
    update();
}

void MaEditorSequenceArea::buildMenu(QMenu* ) {

}

void MaEditorSequenceArea::sl_onHScrollMoved(int pos) {
    if (isAlignmentEmpty()) {
        setFirstVisibleBase(-1);
    } else {
        SAFE_POINT(0 <= pos && pos <= editor->getAlignmentLen() - getNumVisibleBases(false), tr("Position is out of range: %1").arg(QString::number(pos)), );
        setFirstVisibleBase(pos);
    }
}

void MaEditorSequenceArea::sl_onVScrollMoved(int seq) {
    if (isAlignmentEmpty()) {
        setFirstVisibleSequence(-1);
    } else {
        SAFE_POINT(0 <= seq && seq <= editor->getNumSequences() - getNumVisibleSequences(false), tr("Sequence is out of range: %1").arg(QString::number(seq)), );
        setFirstVisibleSequence(seq);
    }
}

void MaEditorSequenceArea::sl_completeUpdate(){
    completeRedraw = true;
    validateRanges();
    updateActions();
    update();
    onVisibleRangeChanged();
}

void MaEditorSequenceArea::sl_triggerUseDots() {
    useDotsAction->trigger();
}

void MaEditorSequenceArea::sl_useDots(){
    completeRedraw = true;
    update();
    emit si_highlightingChanged();
}

void MaEditorSequenceArea::sl_registerCustomColorSchemes() {
    deleteOldCustomSchemes();

    MsaSchemesMenuBuilder::createAndFillColorSchemeMenuActions(customColorSchemeMenuActions, MsaSchemesMenuBuilder::Custom, getEditor()->getMaObject()->getAlphabet()->getType(), 
        this);
}

void MaEditorSequenceArea::sl_colorSchemeFactoryUpdated() {
    applyColorScheme(colorScheme->getFactory()->getId());
}

void MaEditorSequenceArea::sl_setDefaultColorScheme() {
    MsaColorSchemeFactory *defaultFactory = getDefaultColorSchemeFactory();
    SAFE_POINT(NULL != defaultFactory, L10N::nullPointerError("default color scheme factory"), );
    applyColorScheme(defaultFactory->getId());
}

void MaEditorSequenceArea::sl_changeHighlightScheme(){
    QAction* a = qobject_cast<QAction*>(sender());
    if (NULL == a) {
        a = GUIUtils::getCheckedAction(customColorSchemeMenuActions);
    }
    CHECK(NULL != a, );

    editor->saveHighlightingSettings(highlightingScheme->getFactory()->getId(), highlightingScheme->getSettings());

    QString id = a->data().toString();
    MsaHighlightingSchemeFactory* factory = AppContext::getMsaHighlightingSchemeRegistry()->getSchemeFactoryById(id);
    SAFE_POINT(NULL != factory, L10N::nullPointerError("highlighting scheme"), );
    if (ui->getEditor()->getMaObject() == NULL) {
        return;
    }

    delete highlightingScheme;
    highlightingScheme = factory->create(this, ui->getEditor()->getMaObject());
    highlightingScheme->applySettings(editor->getHighlightingSettings(id));

    const MultipleAlignment ma = ui->getEditor()->getMaObject()->getMultipleAlignment();

    U2OpStatusImpl os;
    const int refSeq = ma->getRowIndexByRowId(editor->getReferenceRowId(), os);

    MSAHighlightingFactory msaHighlightingFactory;
    QString msaHighlightingId = msaHighlightingFactory.getOPGroupParameters().getGroupId();

    CHECK(ui->getEditor(), );
    CHECK(ui->getEditor()->getOptionsPanel(), );

    if(!factory->isRefFree() && refSeq == -1 && ui->getEditor()->getOptionsPanel()->getActiveGroupId() != msaHighlightingId) {
        QMessageBox::warning(ui, tr("No reference sequence selected"),
            tr("Reference sequence for current highlighting scheme is not selected. Use context menu or Highlighting tab on Options panel to select it"));
    }

    foreach(QAction* action, highlightingSchemeMenuActions) {
        action->setChecked(action == a);
    }
    if (factory->isAlphabetSupported(DNAAlphabet_RAW)) {
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_RAW, id);
    }
    if (factory->isAlphabetSupported(DNAAlphabet_NUCL)) {
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_NUCL, id);
    }
    if (factory->isAlphabetSupported(DNAAlphabet_AMINO)) {
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_AMINO, id);
    }
    if (factory->isAlphabetSupported(DNAAlphabet_UNDEFINED)) {
        FAIL(tr("Unknown alphabet"), );
    }

    completeRedraw = true;
    update();
    emit si_highlightingChanged();
}

void MaEditorSequenceArea::sl_replaceSelectedCharacter() {
    msaMode = EditCharacterMode;
    editModeAnimationTimer.start(500);
    highlightCurrentSelection();
}

void MaEditorSequenceArea::sl_changeSelectionColor() {
    QColor black(Qt::black);
    selectionColor = (black == selectionColor) ? Qt::darkGray : Qt::black;
    update();
}

void MaEditorSequenceArea::setCursorPos(const QPoint& p) {
    SAFE_POINT(isInRange(p), tr("Cursor position is out of range"), );
    if (p == cursorPos) {
        return;
    }

    cursorPos = p;

    highlightSelection = false;
    updateActions();
}

void MaEditorSequenceArea::setCursorPos(int x, int y) {
    setCursorPos(QPoint(x, y));
}

void MaEditorSequenceArea::setCursorPos(int pos) {
    setCursorPos(QPoint(pos, cursorPos.y()));
}

void MaEditorSequenceArea::resizeEvent(QResizeEvent *e) {
    completeRedraw = true;
    validateRanges();
    QWidget::resizeEvent(e);
}

void MaEditorSequenceArea::paintEvent(QPaintEvent *e) {
    drawAll();
    QWidget::paintEvent(e);
}

void MaEditorSequenceArea::wheelEvent (QWheelEvent * we) {
    bool toMin = we->delta() > 0;
    if (we->modifiers() == 0) {
        shBar->triggerAction(toMin ? QAbstractSlider::SliderSingleStepSub : QAbstractSlider::SliderSingleStepAdd);
    }  else if (we->modifiers() & Qt::SHIFT) {
        svBar->triggerAction(toMin ? QAbstractSlider::SliderSingleStepSub : QAbstractSlider::SliderSingleStepAdd);
    }
    QWidget::wheelEvent(we);
}

void MaEditorSequenceArea::mousePressEvent(QMouseEvent *e) {
    prevPressedButton = e->button();

    if (!hasFocus()) {
        setFocus();
    }

    if ((e->button() == Qt::LeftButton)) {
        if (Qt::ShiftModifier == e->modifiers()) {
            QWidget::mousePressEvent(e);
            return;
        }

        origin = e->pos();
        QPoint p = coordToPos(e->pos());
        if(isInRange(p)) {
            setCursorPos(p);

            const MaEditorSelection &s = getSelection();
            if (s.getRect().contains(cursorPos) && !isAlignmentLocked() && editingEnabled) {
                shifting = true;
                msaVersionBeforeShifting = editor->getMaObject()->getModificationVersion();
                U2OpStatus2Log os;
                changeTracker.startTracking(os);
                CHECK_OP(os, );
                editor->getMaObject()->saveState();
                emit si_startMsaChanging();
            }
        }

        if (!shifting) {
            selecting = true;
            origin = e->pos();
            QPoint q = coordToAbsolutePos(e->pos());
            if (isInRange(q)) {
                setCursorPos(q);
            }
            rubberBand->setGeometry(QRect(origin, QSize()));
            rubberBand->show();
            cancelSelection();
        }
    }

    QWidget::mousePressEvent(e);
}

void MaEditorSequenceArea::mouseReleaseEvent(QMouseEvent *e) {
    rubberBand->hide();
    if (shifting) {
        changeTracker.finishTracking();
        editor->getMaObject()->releaseState();
    }

    QPoint newCurPos = coordToAbsolutePos(e->pos());

    int firstVisibleSeq = getFirstVisibleSequence();
    int visibleRowsNums = getNumDisplayedSequences() - 1;

    int yPosWithValidations = qMax(firstVisibleSeq, newCurPos.y());
    yPosWithValidations = qMin(yPosWithValidations, visibleRowsNums + firstVisibleSeq);

    newCurPos.setY(yPosWithValidations);

    if (shifting) {
        emit si_stopMsaChanging(msaVersionBeforeShifting != editor->getMaObject()->getModificationVersion());
    } else if (Qt::LeftButton == e->button() && Qt::LeftButton == prevPressedButton) {
        updateSelection(newCurPos);
    }
    shifting = false;
    selecting = false;
    msaVersionBeforeShifting = -1;

    shBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
    svBar->setupRepeatAction(QAbstractSlider::SliderNoAction);

    QWidget::mouseReleaseEvent(e);
}

void MaEditorSequenceArea::mouseMoveEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton) {
        QPoint newCurPos = coordToAbsolutePosOutOfRange(e->pos());
        if (isInRange(newCurPos)) {
            updateHBarPosition(newCurPos.x(), true);
            updateVBarPosition(newCurPos.y(), true);
        }

        if (shifting && editingEnabled) {
            shiftSelectedRegion(newCurPos.x() - cursorPos.x());
        } else if (selecting) {
            rubberBand->setGeometry(QRect(origin, e->pos()).normalized());
        }
    }

    QWidget::mouseMoveEvent(e);
}


void MaEditorSequenceArea::keyPressEvent(QKeyEvent *e) {
    if (!hasFocus()) {
        return;
    }

    int key = e->key();
    if (msaMode == EditCharacterMode) {
        processCharacterInEditMode(e);
        return;
    }

    bool enlargeSelection = qobject_cast<MSAEditor*>(getEditor()) != NULL;

    bool shift = e->modifiers().testFlag(Qt::ShiftModifier);
    const bool ctrl = e->modifiers().testFlag(Qt::ControlModifier);
#ifdef Q_OS_MAC
    // In one case it is better to use a Command key as modifier,
    // in another - a Control key. genuineCtrl - Control key on Mac OS X.
    const bool genuineCtrl = e->modifiers().testFlag(Qt::MetaModifier);
#else
    const bool genuineCtrl = ctrl;
#endif
    static QPoint selectionStart(0, 0);
    static QPoint selectionEnd(0, 0);

    if (ctrl && (key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up || key == Qt::Key_Down)) {
        //remap to page_up/page_down
        shift = key == Qt::Key_Up || key == Qt::Key_Down;
        key =  (key == Qt::Key_Up || key == Qt::Key_Left) ? Qt::Key_PageUp : Qt::Key_PageDown;
    }
    //part of these keys are assigned to actions -> so them never passed to keyPressEvent (action handling has higher priority)
    int endX, endY;
    switch(key) {
        case Qt::Key_Escape:
             cancelSelection();
             break;
        case Qt::Key_Left:
            if(!shift || !enlargeSelection) {
                moveSelection(-1,0);
                break;
            }
            if (selectionEnd.x() < 1) {
                break;
            }
            selectionEnd.setX(selectionEnd.x() - 1);
            endX = selectionEnd.x();
            if (isPosInRange(endX)) {
                if (endX != -1) {
                    int firstColumn = qMin(selectionStart.x(),endX);
                    int width = qAbs(endX - selectionStart.x()) + 1;
                    int startSeq = selection.y();
                    int height = selection.height();
                    if (selection.isNull()) {
                        startSeq = cursorPos.y();
                        height = 1;
                    }
                    MaEditorSelection _selection(firstColumn, startSeq, width, height);
                    setSelection(_selection);
                    updateHBarPosition(endX);
                }
            }
            break;
        case Qt::Key_Right:
            if(!shift || !enlargeSelection) {
                moveSelection(1,0);
                break;
            }
            if (selectionEnd.x() >= (editor->getAlignmentLen() - 1)) {
                break;
            }

            selectionEnd.setX(selectionEnd.x() +  1);
            endX = selectionEnd.x();
            if (isPosInRange(endX)) {
                if (endX != -1) {
                    int firstColumn = qMin(selectionStart.x(),endX);
                    int width = qAbs(endX - selectionStart.x()) + 1;
                    int startSeq = selection.y();
                    int height = selection.height();
                    if (selection.isNull()) {
                        startSeq = cursorPos.y();
                        height = 1;
                    }
                    MaEditorSelection _selection(firstColumn, startSeq, width, height);
                    setSelection(_selection);
                    updateHBarPosition(endX);
                }
            }
            break;
        case Qt::Key_Up:
            if(!shift || !enlargeSelection) {
                moveSelection(0,-1);
                break;
            }
            if(selectionEnd.y() < 1) {
                break;
            }
            selectionEnd.setY(selectionEnd.y() - 1);
            endY = selectionEnd.y();
            if (isSeqInRange(endY)) {
                if (endY != -1) {
                    int startSeq = qMin(selectionStart.y(),endY);
                    int height = qAbs(endY - selectionStart.y()) + 1;
                    int firstColumn = selection.x();
                    int width = selection.width();
                    if (selection.isNull()) {
                        firstColumn = cursorPos.x();
                        width = 1;
                    }
                    MaEditorSelection _selection(firstColumn, startSeq, width, height);
                    setSelection(_selection);
                    updateVBarPosition(endY);
                }
            }
            break;
        case Qt::Key_Down:
            if(!shift || !enlargeSelection) {
                moveSelection(0,1);
                break;
            }
            if (selectionEnd.y() >= (ui->getCollapseModel()->displayedRowsCount() - 1)) {
                break;
            }
            selectionEnd.setY(selectionEnd.y() + 1);
            endY = selectionEnd.y();
            if (isSeqInRange(endY)) {
                if (endY != -1) {
                    int startSeq = qMin(selectionStart.y(),endY);
                    int height = qAbs(endY - selectionStart.y()) + 1;
                    int firstColumn = selection.x();
                    int width = selection.width();
                    if (selection.isNull()) {
                        firstColumn = cursorPos.x();
                        width = 1;
                    }
                    MaEditorSelection _selection(firstColumn, startSeq, width, height);
                    setSelection(_selection);
                    updateVBarPosition(endY);
                }
            }
            break;
        case Qt::Key_Delete:
            if (!isAlignmentLocked() && !shift) {
                emit si_startMsaChanging();
                deleteCurrentSelection();
            }
            break;
        case Qt::Key_Home:
            cancelSelection();
            if (shift) { //scroll namelist
                setFirstVisibleSequence(0);
                setCursorPos(QPoint(cursorPos.x(), 0));
            } else { //scroll sequence
                cancelSelection();
                setFirstVisibleBase(0);
                setCursorPos(QPoint(0, cursorPos.y()));
            }
            break;
        case Qt::Key_End:
            cancelSelection();
            if (shift) { //scroll namelist
                int n = getNumDisplayedSequences() - 1;
                setFirstVisibleSequence(n);
                setCursorPos(QPoint(cursorPos.x(), n));
            } else { //scroll sequence
                int n = editor->getAlignmentLen() - 1;
                setFirstVisibleBase(n);
                setCursorPos(QPoint(n, cursorPos.y()));
            }
            break;
        case Qt::Key_PageUp:
            cancelSelection();
            if (shift) { //scroll namelist
                int nVis = getNumVisibleSequences(false);
                int fp = qMax(0, getFirstVisibleSequence() - nVis);
                int cp = qMax(0, cursorPos.y() - nVis);
                setFirstVisibleSequence(fp);
                setCursorPos(QPoint(cursorPos.x(), cp));
            } else { //scroll sequence
                int nVis = getNumVisibleBases(false);
                int fp = qMax(0, getFirstVisibleBase() - nVis);
                int cp = qMax(0, cursorPos.x() - nVis);
                setFirstVisibleBase(fp);
                setCursorPos(QPoint(cp, cursorPos.y()));
            }
            break;
        case Qt::Key_PageDown:
            cancelSelection();
            if (shift) { //scroll namelist
                int nVis = getNumVisibleSequences(false);
                int nSeq = getNumDisplayedSequences();
                int fp = qMin(nSeq-1, getFirstVisibleSequence() + nVis);
                int cp = qMin(nSeq-1, cursorPos.y() + nVis);
                setFirstVisibleSequence(fp);
                setCursorPos(QPoint(cursorPos.x(), cp));
            } else { //scroll sequence
                int nVis = getNumVisibleBases(false);
                int len = editor->getAlignmentLen();
                int fp  = qMin(len-1, getFirstVisibleBase() + nVis);
                int cp  = qMin(len-1, cursorPos.x() + nVis);
                setFirstVisibleBase(fp);
                setCursorPos(QPoint(cp, cursorPos.y()));
            }
            break;
        case Qt::Key_Backspace:
            removeGapsPrecedingSelection(genuineCtrl ? 1 : -1);
            break;
        case Qt::Key_Insert:
        case Qt::Key_Space:
            // We can't use Command+Space on Mac OS X - it is reserved
            if(!isAlignmentLocked()) {
                emit si_startMsaChanging();
                insertGapsBeforeSelection(genuineCtrl ? 1 : -1);
            }
            break;
        case Qt::Key_Shift:
            if (!selection.isNull()) {
                selectionStart = selection.topLeft();
                selectionEnd = selection.getRect().bottomRight();
            } else {
                selectionStart = cursorPos;
                selectionEnd = cursorPos;
            }
            break;
    }
    QWidget::keyPressEvent(e);
}

void MaEditorSequenceArea::keyReleaseEvent(QKeyEvent *ke) {
    if ((ke->key() == Qt::Key_Space || ke->key() == Qt::Key_Delete) && !isAlignmentLocked() && !ke->isAutoRepeat()) {
        emit si_stopMsaChanging(true);
    }

    QWidget::keyReleaseEvent(ke);
}

void MaEditorSequenceArea::insertGapsBeforeSelection(int countOfGaps) {
    CHECK(getEditor() != NULL, );
    if (selection.isNull() || 0 == countOfGaps || -1 > countOfGaps) {
        return;
    }
    SAFE_POINT(isInRange(selection.topLeft()), tr("Top left corner of the selection has incorrect coords"), );
    SAFE_POINT(isInRange(QPoint(selection.x() + selection.width() - 1, selection.y() + selection.height() - 1)),
        tr("Bottom right corner of the selection has incorrect coords"), );

    // if this method was invoked during a region shifting
    // then shifting should be canceled
    cancelShiftTracking();

    MultipleAlignmentObject *maObj = editor->getMaObject();
    if (NULL == maObj || maObj->isStateLocked()) {
        return;
    }
    U2OpStatus2Log os;
    U2UseCommonUserModStep userModStep(maObj->getEntityRef(), os);
    Q_UNUSED(userModStep);
    SAFE_POINT_OP(os,);

    const MultipleAlignment ma = maObj->getMultipleAlignment();
    if (selection.width() == ma->getLength() && selection.height() == ma->getNumRows()) {
        return;
    }

    const int removedRegionWidth = (-1 == countOfGaps) ? selection.width() : countOfGaps;
    const U2Region& sequences = getSelectedRows();
    maObj->insertGap(sequences,  selection.x() , removedRegionWidth);
    moveSelection(removedRegionWidth, 0, true);
}

void MaEditorSequenceArea::removeGapsPrecedingSelection(int countOfGaps) {
    const MaEditorSelection selectionBackup = selection;
    // check if selection exists
    if (selectionBackup.isNull()) {
        return;
    }

    const QPoint selectionTopLeftCorner(selectionBackup.topLeft());
    // don't perform the deletion if the selection is at the alignment start
    if (0 == selectionTopLeftCorner.x() || -1 > countOfGaps || 0 == countOfGaps) {
        return;
    }

    int removedRegionWidth = (-1 == countOfGaps) ? selectionBackup.width() : countOfGaps;
    QPoint topLeftCornerOfRemovedRegion(selectionTopLeftCorner.x() - removedRegionWidth,
        selectionTopLeftCorner.y());
    if (0 > topLeftCornerOfRemovedRegion.x()) {
        removedRegionWidth -= qAbs(topLeftCornerOfRemovedRegion.x());
        topLeftCornerOfRemovedRegion.setX(0);
    }

    MultipleAlignmentObject *maObj = editor->getMaObject();
    if (NULL == maObj || maObj->isStateLocked()) {
        return;
    }

    // if this method was invoked during a region shifting
    // then shifting should be canceled
    cancelShiftTracking();

    const U2Region rowsContainingRemovedGaps(getSelectedRows());
    U2OpStatus2Log os;
    U2UseCommonUserModStep userModStep(maObj->getEntityRef(), os);
    Q_UNUSED(userModStep);

    const int countOfDeletedSymbols = maObj->deleteGap(os, rowsContainingRemovedGaps,
        topLeftCornerOfRemovedRegion.x(), removedRegionWidth);

    // if some symbols were actually removed and the selection is not located
    // at the alignment end, then it's needed to move the selection
    // to the place of the removed symbols
    if (0 < countOfDeletedSymbols) {
        const MaEditorSelection newSelection(selectionBackup.x() - countOfDeletedSymbols,
            topLeftCornerOfRemovedRegion.y(), selectionBackup.width(),
            selectionBackup.height());
        setSelection(newSelection);
    }
}

void MaEditorSequenceArea::cancelShiftTracking() {
    shifting = false;
    selecting = false;
    changeTracker.finishTracking();
    editor->getMaObject()->releaseState();
}

void MaEditorSequenceArea::drawAll() {
    QSize s = size() * devicePixelRatio();
    if (cachedView->size() != s) {
        delete cachedView;
        cachedView = new QPixmap(s);
        cachedView->setDevicePixelRatio(devicePixelRatio());
        completeRedraw = true;
    }
    if (completeRedraw) {
        cachedView->fill(Qt::transparent);
        QPainter pCached(cachedView);
        drawVisibleContent(pCached);
        completeRedraw = false;
    }

    QPainter p(this);
    p.fillRect(QRect(QPoint(0, 0), s), Qt::white);
    drawBackground(p);

    p.drawPixmap(0, 0, *cachedView);
    renderer->drawSelection(p);
    renderer->drawFocus(p);
}

void MaEditorSequenceArea::validateRanges() {
    //check x dimension
    int aliLen = editor->getAlignmentLen();
    int visibleBaseCount = countWidthForBases(false);

    if (isAlignmentEmpty()) {
        setFirstVisibleBase(-1);
    } else if (visibleBaseCount > aliLen) {
        setFirstVisibleBase(0);
    } else if (startPos + visibleBaseCount > aliLen) {
        setFirstVisibleBase(aliLen - visibleBaseCount);
    }

    SAFE_POINT(0 <= startPos || isAlignmentEmpty(), tr("Negative startPos with non-empty alignment"), );
    SAFE_POINT(startPos + visibleBaseCount <= aliLen || aliLen < visibleBaseCount, tr("startPos is too big"), );

    updateHScrollBar();

    //check y dimension
//    if (ui->isCollapsibleMode()) {
//        sl_modelChanged();
//        return;
//    }

    int nSeqs = editor->getNumSequences();
    int visibleSequenceCount = countHeightForSequences(false);

    if (isAlignmentEmpty()) {
        setFirstVisibleSequence(-1);
    } else if (visibleSequenceCount > nSeqs) {
        setFirstVisibleSequence(0);
    } else if (startSeq + visibleSequenceCount > nSeqs) {
        setFirstVisibleSequence(nSeqs - visibleSequenceCount);
    }

    SAFE_POINT(0 <= startSeq || isAlignmentEmpty(), tr("Negative startSeq with non-empty alignment"), );
    SAFE_POINT(startSeq + visibleSequenceCount <= nSeqs || nSeqs < visibleSequenceCount, tr("startSeq is too big"), );

    updateVScrollBar();
}

void MaEditorSequenceArea::updateColorAndHighlightSchemes() {
    Settings* s = AppContext::getSettings();
    if (!s || !editor){
        return;
    }
    MultipleAlignmentObject* maObj = editor->getMaObject();
    if (!maObj){
        return;
    }

    const DNAAlphabet* al = maObj->getAlphabet();
    if (!al){
        return;
    }

    MsaColorSchemeRegistry* csr = AppContext::getMsaColorSchemeRegistry();
    MsaHighlightingSchemeRegistry* hsr = AppContext::getMsaHighlightingSchemeRegistry();

    QString csid;
    QString hsid;
    getColorAndHighlightingIds(csid, hsid);
    MsaColorSchemeFactory* csf = csr->getSchemeFactoryById(csid);
    MsaHighlightingSchemeFactory* hsf = hsr->getSchemeFactoryById(hsid);
    initColorSchemes(csf);
    initHighlightSchemes(hsf);
}

void MaEditorSequenceArea::initColorSchemes(MsaColorSchemeFactory *defaultColorSchemeFactory) {
    MsaColorSchemeRegistry *msaColorSchemeRegistry = AppContext::getMsaColorSchemeRegistry();
    connect(msaColorSchemeRegistry, SIGNAL(si_customSettingsChanged()), SLOT(sl_registerCustomColorSchemes()));

    registerCommonColorSchemes();
    sl_registerCustomColorSchemes();

    useDotsAction = new QAction(QString(tr("Use dots")), this);
    useDotsAction->setCheckable(true);
    useDotsAction->setChecked(false);
    connect(useDotsAction, SIGNAL(triggered()), SLOT(sl_useDots()));

    applyColorScheme(defaultColorSchemeFactory->getId());
}

void MaEditorSequenceArea::registerCommonColorSchemes() {
    qDeleteAll(colorSchemeMenuActions);
    colorSchemeMenuActions.clear();

    MsaSchemesMenuBuilder::createAndFillColorSchemeMenuActions(colorSchemeMenuActions, MsaSchemesMenuBuilder::Common, getEditor()->getMaObject()->getAlphabet()->getType(), this);
}

void MaEditorSequenceArea::initHighlightSchemes(MsaHighlightingSchemeFactory* hsf) {
    qDeleteAll(highlightingSchemeMenuActions);
    highlightingSchemeMenuActions.clear();
    SAFE_POINT(hsf != NULL, "Highlight scheme factory is NULL", );

    MultipleAlignmentObject* maObj = editor->getMaObject();
    delete highlightingScheme;

    highlightingScheme = hsf->create(this, maObj);

    MsaSchemesMenuBuilder::createAndFillHighlightingMenuActions(highlightingSchemeMenuActions, getEditor()->getMaObject()->getAlphabet()->getType(), this);
    QList<QAction *> tmpActions = QList<QAction *>() << highlightingSchemeMenuActions;
    foreach(QAction *action, tmpActions) {
        action->setChecked(action->data() == hsf->getId());
    }
}

MsaColorSchemeFactory * MaEditorSequenceArea::getDefaultColorSchemeFactory() const {
    MsaColorSchemeRegistry *msaColorSchemeRegistry = AppContext::getMsaColorSchemeRegistry();

    switch (editor->getMaObject()->getAlphabet()->getType()) {
    case DNAAlphabet_RAW:
        return msaColorSchemeRegistry->getSchemeFactoryById(MsaColorScheme::EMPTY);
    case DNAAlphabet_NUCL:
        return msaColorSchemeRegistry->getSchemeFactoryById(MsaColorScheme::UGENE_NUCL);
    case DNAAlphabet_AMINO:
        return msaColorSchemeRegistry->getSchemeFactoryById(MsaColorScheme::UGENE_AMINO);
    default:
        FAIL(tr("Unknown alphabet"), NULL);
    }
    return NULL;
}

MsaHighlightingSchemeFactory* MaEditorSequenceArea::getDefaultHighlightingSchemeFactory() const {
    MsaHighlightingSchemeRegistry *hsr = AppContext::getMsaHighlightingSchemeRegistry();
    MsaHighlightingSchemeFactory *hsf = hsr->getSchemeFactoryById(MsaHighlightingScheme::EMPTY);
    return hsf;
}

void MaEditorSequenceArea::getColorAndHighlightingIds(QString &csid, QString &hsid) {
    DNAAlphabetType atype = getEditor()->getMaObject()->getAlphabet()->getType();
    Settings* s = AppContext::getSettings();
    switch (atype) {
    case DNAAlphabet_RAW:
        csid = s->getValue(SETTINGS_ROOT + SETTINGS_COLOR_RAW, MsaColorScheme::EMPTY).toString();
        hsid = s->getValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_RAW, MsaHighlightingScheme::EMPTY).toString();
        break;
    case DNAAlphabet_NUCL:
        csid = s->getValue(SETTINGS_ROOT + SETTINGS_COLOR_NUCL, MsaColorScheme::UGENE_NUCL).toString();
        hsid = s->getValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_NUCL, MsaHighlightingScheme::EMPTY).toString();
        break;
    case DNAAlphabet_AMINO:
        csid = s->getValue(SETTINGS_ROOT + SETTINGS_COLOR_AMINO, MsaColorScheme::UGENE_AMINO).toString();
        hsid = s->getValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_AMINO, MsaHighlightingScheme::EMPTY).toString();
        break;
    default:
        csid = "";
        hsid = "";
        break;
    }

    MsaColorSchemeRegistry* csr = AppContext::getMsaColorSchemeRegistry();
    MsaHighlightingSchemeRegistry* hsr = AppContext::getMsaHighlightingSchemeRegistry();

    MsaColorSchemeFactory* csf = csr->getSchemeFactoryById(csid);
    if (csf == NULL) {
        csid = getDefaultColorSchemeFactory()->getId();
    }
    MsaHighlightingSchemeFactory* hsf = hsr->getSchemeFactoryById(hsid);
    if (hsf == NULL) {
        hsid = getDefaultHighlightingSchemeFactory()->getId();
    }
    

    if (colorScheme != NULL && colorScheme->getFactory()->isAlphabetSupported(atype)) {
        csid = colorScheme->getFactory()->getId();
    }
    if (highlightingScheme != NULL && highlightingScheme->getFactory()->isAlphabetSupported(atype)) {
        hsid = highlightingScheme->getFactory()->getId();
    }
}

void MaEditorSequenceArea::applyColorScheme(const QString &id) {
    CHECK(NULL != ui->getEditor()->getMaObject(), );

    MsaColorSchemeFactory *factory = AppContext::getMsaColorSchemeRegistry()->getSchemeFactoryById(id);
    delete colorScheme;
    colorScheme = factory->create(this, ui->getEditor()->getMaObject());

    connect(factory, SIGNAL(si_factoryChanged()), SLOT(sl_colorSchemeFactoryUpdated()), Qt::UniqueConnection);
    connect(factory, SIGNAL(destroyed(QObject *)), SLOT(sl_setDefaultColorScheme()), Qt::UniqueConnection);

    QList<QAction *> tmpActions = QList<QAction *>() << colorSchemeMenuActions << customColorSchemeMenuActions;
    foreach (QAction *action, tmpActions) {
        action->setChecked(action->data() == id);
    }

    switch (ui->getEditor()->getMaObject()->getAlphabet()->getType()) {
    case DNAAlphabet_RAW:
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_COLOR_RAW, id);
        break;
    case DNAAlphabet_NUCL:
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_COLOR_NUCL, id);
        break;
    case DNAAlphabet_AMINO:
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_COLOR_AMINO, id);
        break;
    default:
        FAIL(tr("Unknown alphabet"), );
        break;
    }

    completeRedraw = true;
    update();
    emit si_highlightingChanged();
}

void MaEditorSequenceArea::updateHScrollBar() {
    shBar->disconnect(this);

    if (isAlignmentEmpty()) {
        shBar->setVisible(false);
    } else {
        int numVisibleBases = getNumVisibleBases(false);
        int alignmentLen = editor->getAlignmentLen();

        SAFE_POINT(numVisibleBases <= alignmentLen, tr("Horizontal scrollbar appears unexpectedly: numVisibleBases is too small"), );

        shBar->setMinimum(0);
        shBar->setMaximum(alignmentLen - numVisibleBases);
        shBar->setSliderPosition(getFirstVisibleBase());

        shBar->setSingleStep(1);
        shBar->setPageStep(numVisibleBases);

        shBar->setVisible(numVisibleBases != alignmentLen);
    }

    connect(shBar, SIGNAL(valueChanged(int)), SLOT(sl_onHScrollMoved(int)));
}

void MaEditorSequenceArea::updateVScrollBar() {
    svBar->disconnect(this);

    if (isAlignmentEmpty()) {
        svBar->setVisible(false);
    } else {
        int start = getFirstVisibleSequence();
        int numVisibleSequences = getNumVisibleSequences(false);
        int nSeqs = getNumDisplayedSequences();

        SAFE_POINT(numVisibleSequences <= nSeqs, tr("Vertical scrollbar appears unexpectedly: numVisibleSequences is too small"), );

        svBar->setMinimum(0);
        svBar->setMaximum(nSeqs - numVisibleSequences);
        svBar->setSliderPosition(start);

        svBar->setSingleStep(1);
        svBar->setPageStep(numVisibleSequences);

        svBar->setVisible(numVisibleSequences != nSeqs);
    }

    connect(svBar, SIGNAL(valueChanged(int)), SLOT(sl_onVScrollMoved(int)));

    onVisibleRangeChanged();
}

bool MaEditorSequenceArea::checkState() const {
#ifdef _DEBUG
    MultipleAlignmentObject* maObj = editor->getMaObject();
    int aliLen = maObj->getLength();
    int nSeqs = maObj->getNumRows();

    assert((startPos >=0 && startSeq >=0) || isAlignmentEmpty());
    int lastPos = getLastVisibleBase(true);
    int lastSeq = getLastVisibleSequence(true);
    assert((lastPos < aliLen && lastSeq < nSeqs) || isAlignmentEmpty());

    // TODO: check selection is valid
    //int cx = cursorPos.x();
    //int cy = cursorPos.y();
    //assert(cx >= 0 && cy >= 0);
    //assert(cx < aliLen && cy < nSeqs);
#endif
    return true;
}

void MaEditorSequenceArea::processCharacterInEditMode(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape) {
        exitFromEditCharacterMode();
        return;
    }

    QString text = e->text().toUpper();
    if (1 == text.length()) {
        QChar emDash(0x2015);
        QRegExp latinCharacterOrGap(QString("([A-Z]| |-|%1)").arg(emDash));
        if (latinCharacterOrGap.exactMatch(text)) {
            QChar newChar = text.at(0);
            newChar = (newChar == '-' || newChar == emDash || newChar == ' ') ? U2Msa::GAP_CHAR : newChar;
            processCharacterInEditMode(newChar.toLatin1());
        }
        else {
            MainWindow *mainWindow = AppContext::getMainWindow();
            const QString message = tr("It is not possible to insert the character into the alignment."
                                       "Please use a character from set A-Z (upper-case or lower-case) or the gap character ('Space', '-' or '%1').").arg(emDash);
            mainWindow->addNotification(message, Error_Not);
            exitFromEditCharacterMode();
        }
    }
}

void MaEditorSequenceArea::processCharacterInEditMode(char newCharacter) {
    CHECK(getEditor() != NULL, );
    if (selection.isNull()) {
        return;
    }
    SAFE_POINT(isInRange(selection.topLeft()), "Incorrect selection is detected!", );
    MultipleAlignmentObject* maObj = editor->getMaObject();
    if (maObj == NULL || maObj->isStateLocked()) {
        return;
    }

    U2OpStatusImpl os;
    U2UseCommonUserModStep userModStep(maObj->getEntityRef(), os);
    Q_UNUSED(userModStep);
    SAFE_POINT_OP(os, );

    // replacement is valid only for one symbol
    const U2Region& sel = getSelectedRows();
    for (qint64 rowIndex = sel.startPos; rowIndex < sel.endPos(); rowIndex++) {
        maObj->replaceCharacter(selection.x(), rowIndex, newCharacter);
    }

    exitFromEditCharacterMode();
}


void MaEditorSequenceArea::exitFromEditCharacterMode() {
    if (msaMode == EditCharacterMode) {
        editModeAnimationTimer.stop();
        highlightSelection = false;
        selectionColor = Qt::black;
        msaMode = ViewMode;
        update();
    }
}

void MaEditorSequenceArea::deleteOldCustomSchemes() {
    qDeleteAll(customColorSchemeMenuActions);
    customColorSchemeMenuActions.clear();
}

} // namespace
