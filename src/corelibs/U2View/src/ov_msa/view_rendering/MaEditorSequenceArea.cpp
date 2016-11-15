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

#include "MaEditorSequenceArea.h"

#include "MaEditorWgt.h"
#include "../MaEditor.h"
#include "../MSACollapsibleModel.h"

#include <U2Core/U2SafePoints.h>
#include <U2Core/MultipleAlignmentObject.h>

#include <U2Gui/GScrollBar.h>

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
      msaVersionBeforeShifting(-1)
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

    cachedView = new QPixmap();
    completeRedraw = true;

    initRenderer();
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

// SANGER_TODO: check the method is used correctly
U2Region MaEditorSequenceArea::getSequenceYRange(int seq, int firstVisibleRow, bool useVirtualCoords) const {
    U2Region res(editor->getRowHeight()* (seq - firstVisibleRow), editor->getSequenceRowHeight());
    if (!useVirtualCoords) {
        int h = height();
        res = res.intersect(U2Region(0, h));
    }
    return res;
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
    for (int i=getFirstVisibleSequence(); i<=lastSeq; i++) {
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

    selectedRowNames.clear();
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

//void MaEditorSequenceArea::initRenderer() {
//}

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
