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
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/GScrollBar.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/OptionsPanel.h>

#include <U2View/MSAHighlightingTabFactory.h>

#include <QMessageBox>
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
      useDotsAction(NULL)
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
    QPair<QString, int> p; // SANGER_TODO: ',' is embarrases CHECK method(?)
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

QString MaEditorSequenceArea::getCopyFormatedAlgorithmId() const{
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + SETTINGS_COPY_FORMATTED, BaseDocumentFormats::CLUSTAL_ALN).toString();
}

void MaEditorSequenceArea::setCopyFormatedAlgorithmId(const QString& algoId){
    AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_COPY_FORMATTED, algoId);
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


void MaEditorSequenceArea::updateHBarPosition(int base) {
    if (isAlignmentEmpty()) {
        shBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
        return;
    }

    if (base <= getFirstVisibleBase()) {
        shBar->setupRepeatAction(QAbstractSlider::SliderSingleStepSub, 50, 10);
    } else  if (base >= getLastVisibleBase(true)) {
        shBar->setupRepeatAction(QAbstractSlider::SliderSingleStepAdd, 50, 10);
    } else {
        shBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
    }
}

void MaEditorSequenceArea::updateVBarPosition(int seq) {
    if (isAlignmentEmpty()) {
        svBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
        return;
    }

    if (seq <= getFirstVisibleSequence()) {
        svBar->setupRepeatAction(QAbstractSlider::SliderSingleStepSub, 50, 10);
    } else if (seq >= getLastVisibleSequence(true)) {
        svBar->setupRepeatAction(QAbstractSlider::SliderSingleStepAdd, 50, 10);
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
    p.fillRect(cachedView->rect(), Qt::white);
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

QString MaEditorSequenceArea::exportHighligtning(int startPos, int endPos, int startingIndex, bool keepGaps, bool dots, bool transpose) {
    CHECK(getEditor() != NULL, QString());
    CHECK(qobject_cast<MSAEditor*>(editor) != NULL, QString());
    QStringList result;

    MultipleAlignmentObject* maObj = editor->getMaObject();
    assert(maObj!=NULL);

    const MultipleAlignment msa = maObj->getMultipleAlignment();

    U2OpStatusImpl os;
    const int refSeq = getEditor()->getMaObject()->getMultipleAlignment()->getRowIndexByRowId(editor->getReferenceRowId(), os);
    // SANGER_TODO: the method should not be launched if not reference is set
    MultipleSequenceAlignmentRow row;
    if (U2MsaRow::INVALID_ROW_ID != refSeq) {
        row = msa->getRow(refSeq);
    }

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

            SAFE_POINT(NULL != row, "MSA row is NULL", "");
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

void MaEditorSequenceArea::sl_changeColorSchemeOutside(const QString &name) {
    QAction* a = GUIUtils::findAction(QList<QAction*>() << colorSchemeMenuActions << customColorSchemeMenuActions << highlightingSchemeMenuActions, name);
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

void MaEditorSequenceArea::sl_useDots(){
    completeRedraw = true;
    update();
    emit si_highlightingChanged();
}

void MaEditorSequenceArea::sl_registerCustomColorSchemes() {
    deleteOldCustomSchemes();

    MsaColorSchemeRegistry *msaColorSchemeRegistry = AppContext::getMsaColorSchemeRegistry();
    QList<MsaColorSchemeFactory *> customFactories = msaColorSchemeRegistry->getMsaCustomColorSchemes(editor->getMaObject()->getAlphabet()->getType());

    foreach (MsaColorSchemeFactory *factory, customFactories) {
        QAction *action = new QAction(factory->getName(), this);
        action->setObjectName(factory->getName());
        action->setCheckable(true);
        action->setData(factory->getId());
        connect(action, SIGNAL(triggered()), SLOT(sl_changeColorScheme()));
        customColorSchemeMenuActions.append(action);
    }
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
    MsaHighlightingSchemeFactory* factory = AppContext::getMsaHighlightingSchemeRegistry()->getMsaHighlightingSchemeFactoryById(id);
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
    switch (factory->getAlphabetType()) {
    case DNAAlphabet_RAW:
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_RAW, id);
        break;
    case DNAAlphabet_NUCL:
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_NUCL, id);
        break;
    case DNAAlphabet_AMINO:
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_AMINO, id);
        break;
    default:
        FAIL(tr("Unknown alphabet"), );
        break;
    }

    completeRedraw = true;
    update();
    emit si_highlightingChanged();
}

void MaEditorSequenceArea::buildMenu(QMenu* m) {

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

    DNAAlphabetType atype = al->getType();
    DNAAlphabetType currentAlphabet = DNAAlphabet_RAW;
    bool colorSchemesActionsIsEmpty = colorSchemeMenuActions.isEmpty();
    MsaColorSchemeRegistry* csr = AppContext::getMsaColorSchemeRegistry();
    MsaHighlightingSchemeRegistry* hsr = AppContext::getMsaHighlightingSchemeRegistry();
    if (!colorSchemesActionsIsEmpty) {
        QString id = colorSchemeMenuActions.first()->data().toString();
        MsaColorSchemeFactory* f = csr->getMsaColorSchemeFactoryById(id);
        currentAlphabet = f->getAlphabetType();
        if (currentAlphabet == atype) {
            return;
        }
    }

    QString csid;
    QString hsid;
    getColorAndHighlightingIds(csid, hsid, atype, colorSchemesActionsIsEmpty);
    MsaColorSchemeFactory* csf = csr->getMsaColorSchemeFactoryById(csid);
    if (csf == NULL) {
        csf = getDefaultColorSchemeFactory();
    }
    SAFE_POINT(csf != NULL, "Color scheme factory is NULL", );
    MsaHighlightingSchemeFactory* hsf = hsr->getMsaHighlightingSchemeFactoryById(hsid);
    initColorSchemes(csf);
    initHighlightSchemes(hsf, atype);
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

    MsaColorSchemeRegistry *msaColorSchemeRegistry = AppContext::getMsaColorSchemeRegistry();
    QList<MsaColorSchemeFactory*> colorFactories = msaColorSchemeRegistry->getMsaColorSchemes(editor->getMaObject()->getAlphabet()->getType());

    foreach (MsaColorSchemeFactory *factory, colorFactories) {
        QAction *action = new QAction(factory->getName(), this);
        action->setObjectName(factory->getName());
        action->setCheckable(true);
        action->setData(factory->getId());
        connect(action, SIGNAL(triggered()), SLOT(sl_changeColorScheme()));
        colorSchemeMenuActions.append(action);
    }
}

void MaEditorSequenceArea::initHighlightSchemes(MsaHighlightingSchemeFactory* hsf, DNAAlphabetType atype) {
    qDeleteAll(highlightingSchemeMenuActions);
    highlightingSchemeMenuActions.clear();
    SAFE_POINT(hsf != NULL, "Highlight scheme factory is NULL", );

    MultipleAlignmentObject* maObj = editor->getMaObject();
    delete highlightingScheme;

    highlightingScheme = hsf->create(this, maObj);

    MsaHighlightingSchemeRegistry* hsr = AppContext::getMsaHighlightingSchemeRegistry();
    QList<MsaHighlightingSchemeFactory*> highFactories = hsr->getMsaHighlightingSchemes(atype);
    foreach (MsaHighlightingSchemeFactory* factory, highFactories) {
        QAction* action = new QAction(factory->getName(), this);
        action->setObjectName(factory->getName());
        action->setCheckable(true);
        action->setChecked(factory == hsf);
        action->setData(factory->getId());
        connect(action, SIGNAL(triggered()), SLOT(sl_changeHighlightScheme()));
        highlightingSchemeMenuActions.append(action);
    }
}

MsaColorSchemeFactory * MaEditorSequenceArea::getDefaultColorSchemeFactory() {
    MsaColorSchemeRegistry *msaColorSchemeRegistry = AppContext::getMsaColorSchemeRegistry();

    switch (editor->getMaObject()->getAlphabet()->getType()) {
    case DNAAlphabet_RAW:
        return msaColorSchemeRegistry->getMsaColorSchemeFactoryById(MsaColorScheme::EMPTY_RAW);
    case DNAAlphabet_NUCL:
        return msaColorSchemeRegistry->getMsaColorSchemeFactoryById(MsaColorScheme::UGENE_NUCL);
    case DNAAlphabet_AMINO:
        return msaColorSchemeRegistry->getMsaColorSchemeFactoryById(MsaColorScheme::UGENE_AMINO);
    default:
        FAIL(tr("Unknown alphabet"), NULL);
    }
    return NULL;
}

void MaEditorSequenceArea::getColorAndHighlightingIds(QString &csid, QString &hsid, DNAAlphabetType atype, bool isFirstInitialization) {
    Settings* s = AppContext::getSettings();
    switch (atype) {
    case DNAAlphabet_RAW:
        if (isFirstInitialization) {
            csid = s->getValue(SETTINGS_ROOT + SETTINGS_COLOR_RAW, MsaColorScheme::EMPTY_RAW).toString();
            hsid = s->getValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_NUCL, MsaHighlightingScheme::EMPTY_RAW).toString();
        } else {
            csid = MsaColorScheme::EMPTY_RAW;
            hsid = MsaHighlightingScheme::EMPTY_RAW;
        }
        break;
    case DNAAlphabet_NUCL:
        if (isFirstInitialization) {
            csid = s->getValue(SETTINGS_ROOT + SETTINGS_COLOR_NUCL, MsaColorScheme::UGENE_NUCL).toString();
            hsid = s->getValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_NUCL, MsaHighlightingScheme::EMPTY_NUCL).toString();
        } else {
            csid = MsaColorScheme::UGENE_NUCL;
            hsid = MsaHighlightingScheme::EMPTY_NUCL;
        }
        break;
    case DNAAlphabet_AMINO:
        if (isFirstInitialization) {
            csid = s->getValue(SETTINGS_ROOT + SETTINGS_COLOR_AMINO, MsaColorScheme::UGENE_AMINO).toString();
            hsid = s->getValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_AMINO, MsaHighlightingScheme::EMPTY_AMINO).toString();
        } else {
            csid = MsaColorScheme::UGENE_AMINO;
            hsid = MsaHighlightingScheme::EMPTY_AMINO;
        }
        break;
    default:
        csid = "";
        hsid = "";
        break;
    }
}

void MaEditorSequenceArea::applyColorScheme(const QString &id) {
    CHECK(NULL != ui->getEditor()->getMaObject(), );

    MsaColorSchemeFactory *factory = AppContext::getMsaColorSchemeRegistry()->getMsaColorSchemeFactoryById(id);
    delete colorScheme;
    colorScheme = factory->create(this, ui->getEditor()->getMaObject());

    connect(factory, SIGNAL(si_factoryChanged()), SLOT(sl_colorSchemeFactoryUpdated()), Qt::UniqueConnection);
    connect(factory, SIGNAL(destroyed(QObject *)), SLOT(sl_setDefaultColorScheme()), Qt::UniqueConnection);

    QList<QAction *> tmpActions = QList<QAction *>() << colorSchemeMenuActions << customColorSchemeMenuActions;
    foreach (QAction *action, tmpActions) {
        action->setChecked(action->data() == id);
    }

    switch (factory->getAlphabetType()) {
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
