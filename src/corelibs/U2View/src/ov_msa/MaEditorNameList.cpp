/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2019 UniPro <ugene@unipro.ru>
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

#include <QApplication>
#include <QClipboard>
#include <QInputDialog>
#include <QMouseEvent>
#include <QPainter>

#include <U2Core/Counter.h>
#include <U2Core/U2Mod.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/GObjectTypes.h>

#include <U2Gui/GUIUtils.h>

#include "helpers/DrawHelper.h"
#include "helpers/RowHeightController.h"
#include "helpers/ScrollController.h"
#include "MaEditorNameList.h"
#include "McaEditor.h"
#include "McaEditorNameList.h"
#include "McaEditorSequenceArea.h"
#include "MSAEditor.h"

#include "view_rendering/MaEditorSequenceArea.h"
#include "view_rendering/MaEditorWgt.h"
#include "view_rendering/SequenceWithChromatogramAreaRenderer.h"

namespace U2 {

#define CHILDREN_OFFSET 8

MaEditorNameList::MaEditorNameList(MaEditorWgt* _ui, QScrollBar* _nhBar)
        : labels(NULL),
          ui(_ui),
          nhBar(_nhBar),
          editor(_ui->getEditor()) {
    setObjectName("msa_editor_name_list");
    setFocusPolicy(Qt::WheelFocus);
    cachedView = new QPixmap();
    completeRedraw = true;
    dragging = false;
    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    editSequenceNameAction = new QAction(tr("Edit sequence name"), this);
    editSequenceNameAction->setObjectName("edit_sequence_name");
    editSequenceNameAction->setShortcut(QKeySequence(Qt::Key_F2));
    editSequenceNameAction->setShortcutContext(Qt::WidgetShortcut);
    connect(editSequenceNameAction, SIGNAL(triggered()), SLOT(sl_editSequenceName()));
    addAction(editSequenceNameAction);

    copyCurrentSequenceAction = new QAction(tr("Copy current sequence"), this);
    copyCurrentSequenceAction->setObjectName("Copy current sequence");
    connect(copyCurrentSequenceAction, SIGNAL(triggered()), SLOT(sl_copyCurrentSequence()));

    removeSequenceAction = new QAction(tr("Remove sequence(s)"), this);
    removeSequenceAction->setObjectName("Remove sequence");
    removeSequenceAction->setShortcutContext(Qt::WidgetShortcut);
    connect(removeSequenceAction, SIGNAL(triggered()), SLOT(sl_removeSequence()));
    addAction(removeSequenceAction);

    if (editor->getMaObject()) {
        connect(editor->getMaObject(), SIGNAL(si_alignmentChanged(const MultipleAlignment&, const MaModificationInfo&)),
            SLOT(sl_alignmentChanged(const MultipleAlignment&, const MaModificationInfo&)));
        connect(editor->getMaObject(), SIGNAL(si_lockedStateChanged()), SLOT(sl_lockedStateChanged()));
    }

    connect(this,   SIGNAL(si_startMaChanging()),
            ui,     SIGNAL(si_startMaChanging()));
    connect(this,   SIGNAL(si_stopMaChanging(bool)),
            ui,     SIGNAL(si_stopMaChanging(bool)));

    if (ui->getSequenceArea()) {
        connect(ui->getSequenceArea(), SIGNAL(si_selectionChanged(const MaEditorSelection &, const MaEditorSelection &)),
            SLOT(sl_selectionChanged(const MaEditorSelection &, const MaEditorSelection &)));
        connect(ui->getEditor(), SIGNAL(si_fontChanged(const QFont&)), SLOT(sl_completeUpdate()));
    }
    connect(ui->getCollapseModel(), SIGNAL(si_toggled()), SLOT(sl_completeUpdate()));
    connect(editor, SIGNAL(si_referenceSeqChanged(qint64)), SLOT(sl_completeRedraw()));
    connect(editor, SIGNAL(si_cursorPositionChanged(const QPoint&)), SLOT(sl_completeRedraw()));
    connect(editor, SIGNAL(si_completeUpdate()), SLOT(sl_completeUpdate()));
    connect(editor, SIGNAL(si_updateActions()), SLOT(sl_updateActions()));
    connect(ui, SIGNAL(si_completeRedraw()), SLOT(sl_completeRedraw()));
    connect(ui->getScrollController(), SIGNAL(si_visibleAreaChanged()), SLOT(sl_completeRedraw()));
    connect(ui->getScrollController()->getVerticalScrollBar(), SIGNAL(actionTriggered(int)), SLOT(sl_vScrollBarActionPerformed()));

    nhBar->setParent(this);
    nhBar->setVisible(false);
    sl_updateActions();

    QObject *labelsParent = new QObject(this);
    labelsParent->setObjectName("labels_parent");
    labels = new QObject(labelsParent);
}

MaEditorNameList::~MaEditorNameList() {
    delete cachedView;
}

QSize MaEditorNameList::getCanvasSize(const QList<int> &seqIdx) const {
    return QSize(width(), ui->getRowHeightController()->getRowsHeight(seqIdx));
}

void MaEditorNameList::drawNames(QPixmap &pixmap, const QList<int> &seqIdx, bool drawSelection) {
    CHECK(!seqIdx.isEmpty(), );

    SAFE_POINT(NULL != ui, tr("MSA Editor UI is NULL"), );
    MaEditorSequenceArea* seqArea = ui->getSequenceArea();
    SAFE_POINT(NULL != seqArea, tr("MSA Editor sequence area is NULL"), );
    CHECK(!seqArea->isAlignmentEmpty(), );

    const int rowsHeight = ui->getRowHeightController()->getRowsHeight(seqIdx);
    CHECK(rowsHeight < 32768, );

    pixmap = QPixmap(width(), rowsHeight);

    QPainter painter(&pixmap);
    drawNames(painter, seqIdx, drawSelection);
}

void MaEditorNameList::drawNames(QPainter &painter, const QList<int> &seqIdx, bool drawSelection) {
    painter.fillRect(painter.viewport(), Qt::white);

    MultipleAlignmentObject* maObj = editor->getMaObject();
    SAFE_POINT(NULL != maObj, tr("MSA Object is NULL"), );
    const MultipleAlignment ma = maObj->getMultipleAlignment();
    const QPoint& cursorPosition = editor->getCursorPosition();

    const QStringList seqNames = ma->getRowNames();
    for (int number = 0; number < seqIdx.size(); number++) {
        const int index = seqIdx[number];
        SAFE_POINT(index < seqNames.size(), tr("Invalid sequence index"), );
        const bool isSelected = drawSelection && isRowInSelection(index);
        const bool isFocused = cursorPosition.y() == index;
        const U2Region yRange = ui->getRowHeightController()->getRowGlobalRange(index, seqIdx);
        drawSequenceItem(painter, index, yRange, getTextForRow(index), isSelected, isFocused);
    }
}

QAction *MaEditorNameList::getEditSequenceNameAction() const {
    return editSequenceNameAction;
}

QAction *MaEditorNameList::getRemoveSequenceAction() const {
    return removeSequenceAction;
}

U2Region MaEditorNameList::getSelection() const {
    const MaEditorSelection& selection = ui->getSequenceArea()->getSelection();
    return U2Region(selection.y(), selection.height());
}

void MaEditorNameList::setSelection(int startSeq, int count) {
    int width = editor->getAlignmentLen();
    MaEditorSelection selection(0, startSeq, width, count);
    ui->getSequenceArea()->setSelection(selection);
}

bool MaEditorNameList::isRowInSelection(int seqnum) const {
    MaEditorSelection s = ui->getSequenceArea()->getSelection();
    int endPos = s.y() + s.height() - 1;
    return seqnum >= s.y() && seqnum <= endPos;
}

void MaEditorNameList::updateScrollBar() {
    nhBar->disconnect(this);

    QFont f = ui->getEditor()->getFont();
    f.setItalic(true);
    QFontMetrics fm(f,this);
    int maxNameWidth = 0;

    MultipleAlignmentObject* maObj = editor->getMaObject();
    foreach (const MultipleAlignmentRow& row, maObj->getMultipleAlignment()->getRows()) {
        maxNameWidth = qMax(fm.width(row->getName()), maxNameWidth);
    }
    // adjustment for branch primitive in collapsing mode
    if (ui->isCollapsibleMode()) {
        maxNameWidth += 2*CROSS_SIZE + CHILDREN_OFFSET;
    }

    int availableWidth = getAvailableWidth();
    int nSteps = 1;
    int stepSize = fm.width('W');
    if (availableWidth < maxNameWidth) {
        int dw = maxNameWidth - availableWidth;
        nSteps += dw / stepSize + (dw % stepSize != 0 ? 1 : 0);
    }
    nhBar->setMinimum(0);
    nhBar->setMaximum(nSteps - 1);
    nhBar->setValue(0);

    nhBar->setVisible(nSteps > 1);
    connect(nhBar, SIGNAL(valueChanged(int)), SLOT(sl_completeRedraw()));
}

int MaEditorNameList::getSelectedRow() const {
    U2Region sel = getSelection();
    CHECK(!sel.isEmpty(), -1);

    int n = sel.startPos;
    if (ui->isCollapsibleMode()) {
        n = ui->getCollapseModel()->mapToRow(n);
    }
    return n;
}

void MaEditorNameList::sl_copyCurrentSequence() {
    int n = getSelectedRow();
    MultipleAlignmentObject* maObj = editor->getMaObject();
    if (maObj) {
        const MultipleAlignmentRow row = maObj->getRow(n);
        //TODO: trim large sequence?
        U2OpStatus2Log os;
        QApplication::clipboard()->setText(row->toByteArray(os, maObj->getLength()));
    }
}

void MaEditorNameList::sl_alignmentChanged(const MultipleAlignment&, const MaModificationInfo& mi) {
    if (mi.rowListChanged) {
        completeRedraw = true;
        sl_updateActions();
        updateScrollBar();
        update();
    }
}

void MaEditorNameList::sl_removeSequence() {
    GRUNTIME_NAMED_COUNTER(cvat, tvar, "Remove row", editor->getFactoryId());
    U2Region sel = getSelection();
    CHECK(!sel.isEmpty(), );

    MultipleAlignmentObject* maObj = editor->getMaObject();
    CHECK(maObj->getNumRows() > sel.length, );

    U2OpStatusImpl os;
    U2UseCommonUserModStep userModStep(maObj->getEntityRef(), os);
    Q_UNUSED(userModStep);
    SAFE_POINT_OP(os, );

    setSelection(0, 0);

    U2Region mappedSelection = ui->getCollapseModel()->mapSelectionRegionToRows(sel);
    maObj->removeRegion(0, mappedSelection.startPos, maObj->getLength(), mappedSelection.length, true);

    qint64 numRows = editor->getUI()->getCollapseModel()->getDisplayableRowsCount();
    if (sel.startPos < numRows) {
        int count = qMin(sel.length, numRows - sel.startPos);
        setSelection(sel.startPos, count);
    }
}

void MaEditorNameList::sl_selectReferenceSequence() {
    MultipleAlignmentObject* maObj = editor->getMaObject();
    if (maObj) {
        int n = getSelectedRow();
        if (n < 0) {
            return;
        }
        assert(!maObj->isStateLocked());
        editor->setReference(maObj->getRow(n)->getRowId());
    }
}

void MaEditorNameList::sl_lockedStateChanged() {
    sl_updateActions();
}

void MaEditorNameList::resizeEvent(QResizeEvent* e) {
    completeRedraw = true;
    updateScrollBar();
    QWidget::resizeEvent(e);
}

void MaEditorNameList::paintEvent(QPaintEvent*) {
    drawAll();
}

void MaEditorNameList::keyPressEvent(QKeyEvent *e) {
    int key = e->key();
    bool isShiftPressed = e->modifiers().testFlag(Qt::ShiftModifier);
    int cursorRow =  editor->getCursorPosition().y();
    switch(key) {
    case Qt::Key_Up: {
        U2Region sel = getSelection();
        if (sel.isEmpty()) {
            break;
        }
        if (isShiftPressed) {
            bool grow = sel.length == 1 || sel.startPos < cursorRow;
            if (grow) {
                if (sel.startPos > 0) {
                    setSelection(sel.startPos - 1, sel.length + 1);
                }
            } else { // shrink
                setSelection(sel.startPos, sel.length - 1);
            }
            scrollSelectionToView(true);
        } else {
            moveSelection(-1);
        }
        break;
    }
    case Qt::Key_Down: {
        U2Region sel = getSelection();
        if (sel.isEmpty()) {
            break;
        }
        if (isShiftPressed) {
            bool grow = sel.length == 1 || cursorRow < sel.endPos() - 1;
            int numSequences = ui->getSequenceArea()->getNumDisplayableSequences();
            if (grow) {
                if (sel.endPos() < numSequences) {
                    setSelection(sel.startPos, sel.length + 1);
                }
            } else { // shrink
                setSelection(sel.startPos + 1, sel.length - 1);
            }
            scrollSelectionToView(false);
        } else {
            moveSelection(1);
        }
        break;
    }
    case Qt::Key_Left:
        nhBar->triggerAction(QAbstractSlider::SliderSingleStepSub);
        break;
    case Qt::Key_Right:
        nhBar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        break;
    case Qt::Key_Home:
        ui->getScrollController()->scrollToEnd(ScrollController::Up);
        break;
    case Qt::Key_End:
        ui->getScrollController()->scrollToEnd(ScrollController::Down);
        break;
    case Qt::Key_PageUp:
        ui->getScrollController()->scrollPage(ScrollController::Up);
        break;
    case Qt::Key_PageDown:
        ui->getScrollController()->scrollPage(ScrollController::Down);
        break;
    case Qt::Key_Escape:
        ui->getSequenceArea()->sl_cancelSelection();
        break;
    case Qt::Key_Delete:
        if (removeSequenceAction->isEnabled()) {
            sl_removeSequence();
        }
        break;
    }
    QWidget::keyPressEvent(e);
}

void MaEditorNameList::mousePressEvent(QMouseEvent *e) {
    setFocus();
    SAFE_POINT(ui, "MSA Editor UI is NULL", );
    MaEditorSequenceArea* seqArea = ui->getSequenceArea();
    SAFE_POINT(seqArea, "MSA Editor sequence area", );

    if (seqArea->isAlignmentEmpty() || e->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(e);
        return;
    }

    emit si_startMaChanging();
    bool hasShiftModifier = e->modifiers().testFlag(Qt::ShiftModifier);
    mousePressPoint = e->pos();
    int rowIndex = ui->getRowHeightController()->screenYPositionToRowNumber(e->y());
    int cursorX = editor->getCursorPosition().x();
    // Do not update cursor position on clicks with Shift. Clicks with Shift update selection only.
    bool updateCursorPos = !hasShiftModifier;
    if (ui->isCollapsibleMode()) {
        MSACollapsibleItemModel* m = ui->getCollapseModel();
        if (rowIndex >= m->getDisplayableRowsCount()) {
            rowIndex = m->getDisplayableRowsCount() - 1;
        }
        if (updateCursorPos) {
            editor->setCursorPosition(QPoint(cursorX, rowIndex));
        }
        if (m->isTopLevel(rowIndex)) {
            const U2Region yRange = ui->getRowHeightController()->getRowScreenRangeByNumber(rowIndex);
            bool selected = isRowInSelection(rowIndex);
            QRect textRect = calculateTextRect(yRange, selected);
            QRect buttonRect = calculateButtonRect(textRect);
            if (buttonRect.contains(mousePressPoint)) {
                m->toggle(rowIndex);
                sl_completeRedraw();
                QWidget::mousePressEvent(e);
                return;
            }
        }
    }
    if (updateCursorPos) {
        editor->setCursorPosition(QPoint(cursorX, rowIndex));
    }
    U2Region s = getSelection();
    if (s.contains(rowIndex)) {
        dragging = !ui->isCollapsibleMode() || ui->getCollapseModel()->isFakeModel();
    } else {
        rubberBand->setGeometry(QRect(mousePressPoint, QSize()));
        rubberBand->show();
    }

    QWidget::mousePressEvent(e);
}

void MaEditorNameList::mouseMoveEvent(QMouseEvent* e) {
    if (rubberBand->isVisible() || dragging) {
        int mouseRow = ui->getRowHeightController()->screenYPositionToRowNumber(e->y());
        if (ui->getSequenceArea()->isSeqInRange(mouseRow)) {
            if (ui->getSequenceArea()->isRowVisible(mouseRow, false)) {
                ui->getScrollController()->stopSmoothScrolling();
            } else {
                ScrollController::Directions direction = ScrollController::None;
                if (mouseRow < ui->getScrollController()->getFirstVisibleRowNumber(false)) {
                    direction |= ScrollController::Up;
                } else if (mouseRow > ui->getScrollController()->getLastVisibleRowNumber(height(), false)) {
                    direction |= ScrollController::Down;
                }
                ui->getScrollController()->scrollSmoothly(direction);
            }
        }

        if (dragging) {
            assert(!ui->isCollapsibleMode() || ui->getCollapseModel()->isFakeModel());
            moveSelectedRegion(mouseRow - editor->getCursorPosition().y());
        } else {
            rubberBand->setGeometry(QRect(mousePressPoint, e->pos()).normalized());
        }
    }
    QWidget::mouseMoveEvent(e);
}

void MaEditorNameList::mouseReleaseEvent(QMouseEvent *e) {
    bool hasShiftModifier = e->modifiers().testFlag(Qt::ShiftModifier);
    bool hasCtrlModifier = e->modifiers().testFlag(Qt::ControlModifier);
    ScrollController* scrollController = ui->getScrollController();
    if (rubberBand->isVisible() || dragging) {
        RowHeightController* rowsController = ui->getRowHeightController();
        int maxRows = ui->getSequenceArea()->getNumDisplayableSequences();
        int lastVisibleRow = scrollController->getLastVisibleRowNumber(height(), true);
        int lastVisibleRowY = rowsController->getRowScreenRangeByNumber(lastVisibleRow).endPos();

        U2Region selection = getSelection(); // current selection.

        // mousePressRowExt has extended range: -1 (before first) to maxRows (after the last)
        int mousePressRowExt = mousePressPoint.y() >= lastVisibleRowY ? maxRows : rowsController->screenYPositionToRowNumber(mousePressPoint.y());
        int mousePressRow = qBound(0, mousePressRowExt, maxRows - 1);

        // mouseReleaseRowExt has extended range: -1 (before first) to maxRows (after the last)
        int mouseReleaseRowExt = e->y() >= lastVisibleRowY ? maxRows : rowsController->screenYPositionToRowNumber(e->y());
        int mouseReleaseRow = qBound(0, mouseReleaseRowExt, maxRows - 1);

        bool isClick = e->pos() == mousePressPoint;
        if (isClick) {
            // special case: click but don't drag
            dragging = false;
        }

        if (dragging) {
            assert(!ui->isCollapsibleMode() || ui->getCollapseModel()->isFakeModel());
            int shift = 0;
            if (mouseReleaseRow == 0) {
                shift = -selection.startPos;
            } else if (mouseReleaseRow == maxRows - 1) {
                shift = maxRows - (selection.startPos + selection.length);
            } else {
                shift = mouseReleaseRow - editor->getCursorPosition().y();
            }
            moveSelectedRegion(shift);
        } else {
            bool selectionContainsSeqs = mouseReleaseRow <= lastVisibleRow;
            if (selectionContainsSeqs) {
                int newSelectionStart = -1;
                int newSelectionLen = -1;
                QPoint cursorPos = editor->getCursorPosition();
                if (hasShiftModifier && isClick) { // append region between current selection & mouseReleaseRow to the selection.
                    if (mouseReleaseRow < cursorPos.y()) {
                        newSelectionStart = mouseReleaseRow;
                        newSelectionLen = cursorPos.y() - mousePressRow + 1;
                    } else if (mouseReleaseRow > cursorPos.y()) {
                        newSelectionStart = cursorPos.y();
                        newSelectionLen = mousePressRow - cursorPos.y() + 1;
                    }
                } else {
                    // overflowAction is 'true' if selection started & finished outside of any row.
                    bool overflowAction = (mousePressRow < 0 && mouseReleaseRow < 0)
                            || (mousePressRow >= maxRows && mouseReleaseRow >= maxRows);
                    if (overflowAction) {
                        clearSelection();
                    } else {
                        newSelectionStart = qMin(mousePressRow, mouseReleaseRow);
                        newSelectionLen = qAbs(mousePressRow - mouseReleaseRow) + 1;
                        if (selection.length > 0 && hasShiftModifier) { // Add region to the selection when Shift is used and there is an intersection.
                            U2Region selectionExt(newSelectionStart - 1, newSelectionLen + 2); // Region to test intersection. Extended to +1 both sides so we track 'touches' too.
                            if (selectionExt.intersect(selection).length > 0) {
                                if (selection.startPos < newSelectionStart) {
                                    newSelectionLen += newSelectionStart - selection.startPos;
                                    newSelectionStart = selection.startPos;
                                }
                                if (newSelectionStart + newSelectionLen < selection.endPos()) {
                                    newSelectionLen = selection.endPos() - newSelectionStart;
                                }
                            }
                        }
                    }
                }
                if (newSelectionLen > 0) {
                    if (hasCtrlModifier && selection.length > 0) { // with Ctrl we copy X range to the new selection.
                        const MaEditorSelection& maSelection = ui->getSequenceArea()->getSelection();
                        ui->getSequenceArea()->setSelection(MaEditorSelection(maSelection.x(), newSelectionStart, maSelection.width(), newSelectionLen));
                    } else {
                        setSelection(newSelectionStart, newSelectionLen);
                    }
                }
            } else {
                clearSelection();
            }
        }
    }
    rubberBand->hide();
    dragging = false;
    emit si_stopMaChanging(false);
    scrollController->stopSmoothScrolling();

    QWidget::mouseReleaseEvent(e);
}

void MaEditorNameList::wheelEvent(QWheelEvent *we) {
    bool toMin = we->delta() > 0;
    ui->getScrollController()->scrollStep(toMin ? ScrollController::Up : ScrollController::Down);
    QWidget::wheelEvent(we);
}

void MaEditorNameList::clearSelection() {
    ui->getSequenceArea()->setSelection(MaEditorSelection());
}

void MaEditorNameList::sl_selectionChanged(const MaEditorSelection& current, const MaEditorSelection& prev)
{
    if (current.y() == prev.y() && current.height() == prev.height()) {
        return;
    }
    completeRedraw = true;
    update();
    sl_updateActions();
}

void MaEditorNameList::sl_updateActions() {
    SAFE_POINT(NULL != ui, tr("MSA Editor UI is NULL"), );
    MaEditorSequenceArea* seqArea = ui->getSequenceArea();
    SAFE_POINT(NULL != seqArea, tr("MSA Editor sequence area is NULL"), );

    copyCurrentSequenceAction->setEnabled(!seqArea->isAlignmentEmpty());

    MultipleAlignmentObject* maObj = editor->getMaObject();
    if (maObj){
        removeSequenceAction->setEnabled(!maObj->isStateLocked() && getSelectedRow() != -1);
        editSequenceNameAction->setEnabled(!maObj->isStateLocked() && getSelectedRow() != -1);
        addAction(ui->getCopySelectionAction());
        addAction(ui->getPasteAction());
    }
}

//todo: yalgaer ask about use-case for this method
void MaEditorNameList::sl_vScrollBarActionPerformed() {
    CHECK(dragging,);
    assert(!ui->isCollapsibleMode() || ui->getCollapseModel()->isFakeModel());

    GScrollBar *vScrollBar = qobject_cast<GScrollBar *>(sender());
    SAFE_POINT(NULL != vScrollBar, "vScrollBar is NULL", );

    const QAbstractSlider::SliderAction action = vScrollBar->getRepeatAction();
    CHECK(QAbstractSlider::SliderSingleStepAdd == action || QAbstractSlider::SliderSingleStepSub == action, );

    const QPoint localPoint = mapFromGlobal(QCursor::pos());
    const int newSeqNum = ui->getRowHeightController()->screenYPositionToRowNumber(localPoint.y());
    moveSelectedRegion(newSeqNum - editor->getCursorPosition().y());
}

void MaEditorNameList::focusInEvent(QFocusEvent* fe) {
    QWidget::focusInEvent(fe);
    update();
}

void MaEditorNameList::focusOutEvent(QFocusEvent* fe) {
    QWidget::focusOutEvent(fe);
    update();
}

void MaEditorNameList::sl_completeUpdate() {
    completeRedraw = true;
    updateScrollBar();
    update();
}

void MaEditorNameList::sl_completeRedraw() {
    completeRedraw = true;
    update();
}

void MaEditorNameList::sl_onGroupColorsChanged(const GroupColorSchema& colors) {
    groupColors = colors;
    completeRedraw = true;
    update();
}

//////////////////////////////////////////////////////////////////////////
// draw methods
QFont MaEditorNameList::getFont(bool selected) const {
    QFont f = ui->getEditor()->getFont();
    f.setItalic(true);
    if (selected) {
        f.setBold(true);
    }
    return f;
}

QRect MaEditorNameList::calculateTextRect(const U2Region& yRange, bool selected) const {
    int textX = MARGIN_TEXT_LEFT;
    int textW = getAvailableWidth();
    int textY = yRange.startPos + MARGIN_TEXT_TOP;
    int textH = yRange.length - MARGIN_TEXT_TOP - MARGIN_TEXT_BOTTOM;
    QRect textRect(textX, textY, textW, textH);
    if (nhBar->isVisible()) {
        QFontMetrics fm(getFont(selected));
        int stepSize = fm.width('W');
        int dx = stepSize * nhBar->value();
        textRect = textRect.adjusted(-dx, 0, 0, 0);
    }
    return textRect;
}

QRect MaEditorNameList::calculateButtonRect(const QRect& itemRect) const {
    return QRect(itemRect.left() + CROSS_SIZE/2, itemRect.top() + MARGIN_TEXT_TOP, CROSS_SIZE, CROSS_SIZE);
}

int MaEditorNameList::getAvailableWidth() const {
    return width() - MARGIN_TEXT_LEFT;
}

void MaEditorNameList::drawAll() {
    QSize s = size() * devicePixelRatio();
    if (cachedView->size() != s) {
        delete cachedView;
        cachedView = new QPixmap(s);
        cachedView->setDevicePixelRatio(devicePixelRatio());
        completeRedraw = true;
    }
    if (completeRedraw) {
        QPainter pCached(cachedView);
        drawContent(pCached);
        completeRedraw = false;
    }
    QPainter p(this);
    p.drawPixmap(0, 0, *cachedView);
    drawSelection(p);
}

void MaEditorNameList::drawContent(QPainter& painter) {
    painter.fillRect(cachedView->rect(), Qt::white);

    SAFE_POINT(NULL != ui, "MA Editor UI is NULL", );
    MaEditorSequenceArea* seqArea = ui->getSequenceArea();
    SAFE_POINT(NULL != seqArea, "MA Editor sequence area is NULL", );

    CHECK(!seqArea->isAlignmentEmpty(), );

    if (labels) {
        labels->setObjectName("");
    }

    MultipleAlignmentObject* maObj = editor->getMaObject();
    SAFE_POINT(NULL != maObj, "NULL Ma Object in MAEditorNameList::drawContent", );

    const MultipleAlignment ma = maObj->getMultipleAlignment();

    U2OpStatusImpl os;
    const int referenceIndex = editor->getReferenceRowId() == U2MsaRow::INVALID_ROW_ID ? U2MsaRow::INVALID_ROW_ID
                                                                                       : ma->getRowIndexByRowId(editor->getReferenceRowId(), os);
    SAFE_POINT_OP(os, );

    const QPoint& cursorPosition = editor->getCursorPosition();
    if (ui->isCollapsibleMode()) {
        MSACollapsibleItemModel* collapsibleModel = ui->getCollapseModel();
        const QVector<U2Region> groupedRowsToDraw = ui->getDrawHelper()->getGroupedVisibleRowsIndexes(height());
        foreach (const U2Region &group, groupedRowsToDraw) {
            for (int rowIndex = group.startPos; rowIndex < group.endPos(); rowIndex++) {
                const U2Region yRange = ui->getRowHeightController()->getRowScreenRange(rowIndex);
                const int rowNumber = collapsibleModel->rowToMap(rowIndex, true);
                const bool isSelected = isRowInSelection(rowNumber);
                const bool isReference = (rowIndex == referenceIndex);
                const bool isFocused = rowIndex == cursorPosition.y();

                if (!collapsibleModel->isRowInGroup(rowNumber)) {
                    painter.translate(CROSS_SIZE * 2, 0);
                    drawSequenceItem(painter, getTextForRow(rowIndex), yRange, isSelected, isFocused, isReference);
                    painter.translate(-CROSS_SIZE * 2, 0);
                } else {
                    const MSACollapsableItem &item = collapsibleModel->getItemByRowIndex(rowIndex);
                    SAFE_POINT(item.isValid(), QString("Collapsible item was nof found for row number %1").arg(rowIndex), );
                    const QRect rect = calculateTextRect(yRange, isSelected);
                    // SANGER_TODO: check reference
                    if (collapsibleModel->isTopLevel(rowNumber)) {
                        drawCollapsibileSequenceItem(painter, rowIndex, getTextForRow(rowIndex), rect, isSelected, isFocused, item.isCollapsed, isReference);
                    } else {
                        drawChildSequenceItem(painter, getTextForRow(rowIndex), rect, isSelected, isFocused, isReference);
                    }
                }
            }
        }
    } else {
        const QList<int> rowsToDrow = ui->getDrawHelper()->getVisibleRowsIndexes(height());
        foreach (const int rowToDrow, rowsToDrow) {
            const bool isSelected = isRowInSelection(rowToDrow);
            const bool isFocused = rowToDrow == cursorPosition.y() ;
            drawSequenceItem(painter, rowToDrow, ui->getRowHeightController()->getRowScreenRange(rowToDrow), getTextForRow(rowToDrow), isSelected, isFocused);
        }
    }
}

void MaEditorNameList::drawSequenceItem(QPainter& painter, const QString& text, const U2Region& yRange, bool selected, bool focused, bool isReference) {
    QRect rect = calculateTextRect(yRange, selected);

    MultipleAlignmentObject* maObj = editor->getMaObject();
    CHECK(maObj != NULL, );
    drawBackground(painter, text, rect, isReference);
    drawText(painter, text, rect, selected);
    if (focused) {
        drawCursorFrame(painter, rect);
    }
}

void MaEditorNameList::drawSequenceItem(QPainter &painter, int rowIndex, const U2Region &yRange, const QString &text, bool selected, bool focused) {
    // SANGER_TODO: simplify getting the reference status - no reference here!
    MultipleAlignmentObject* maObj = editor->getMaObject();
    CHECK(maObj != NULL, );
    U2OpStatusImpl os;
    bool isReference = (rowIndex == maObj->getMultipleAlignment()->getRowIndexByRowId(editor->getReferenceRowId(), os));
    drawSequenceItem(painter, text, yRange, selected, focused, isReference);
}

void MaEditorNameList::drawCollapsibileSequenceItem(QPainter &painter, int /*rowIndex*/, const QString &name, const QRect &rect,
                                                    bool selected, bool focused, bool collapsed, bool isReference) {
    drawBackground(painter, name, rect, isReference);
    drawCollapsePrimitive(painter, collapsed, rect);
    drawText(painter, name, rect.adjusted(CROSS_SIZE * 2, 0, 0, 0), selected);
    if (focused) {
        drawCursorFrame(painter, rect);
    }
}

void MaEditorNameList::drawChildSequenceItem(QPainter &painter, const QString &name, const QRect &rect,
                                             bool selected, bool focused, bool isReference) {
    drawBackground(painter, name, rect, isReference);
    painter.translate(CROSS_SIZE * 2 + CHILDREN_OFFSET, 0);
    drawText(painter, name, rect, selected);
    painter.translate( - CROSS_SIZE * 2 - CHILDREN_OFFSET, 0);
}

void MaEditorNameList::drawBackground(QPainter& p, const QString& name, const QRect& rect, bool isReference) {
    if (isReference) {
        p.fillRect(rect, QColor("#9999CC")); // SANGER_TODO: create the const, reference  color
        return;
    }

    p.fillRect(rect, Qt::white);
    if (groupColors.contains(name)) {
        if (QColor(Qt::black) != groupColors[name]) {
            p.fillRect(rect, groupColors[name]);
        }
    }
}

void MaEditorNameList::drawText(QPainter& p, const QString& name, const QRect& rect, bool selected) {
    p.setFont(getFont(selected));
    p.drawText(rect, Qt::AlignTop | Qt::AlignLeft, name); // SANGER_TODO: check the alignment
}

void MaEditorNameList::drawCollapsePrimitive(QPainter& p, bool collapsed, const QRect& rect) {
    QStyleOptionViewItemV2 branchOption;
    branchOption.rect = calculateButtonRect(rect);
    if (collapsed) {
        branchOption.state = QStyle::State_Children | QStyle::State_Sibling; // test
    } else {
        branchOption.state = QStyle::State_Open | QStyle::State_Children;
    }
    style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, &p, this);
}

void MaEditorNameList::drawRefSequence(QPainter &p, QRect r){
    p.fillRect(r, QColor("#9999CC"));
}

QString MaEditorNameList::getTextForRow(int s) {
    return editor->getMaObject()->getRow(s)->getName();
}

QString MaEditorNameList::getSeqName(int s) {
    return getTextForRow(s);
}

void MaEditorNameList::drawSelection(QPainter &painter) {
    const U2Region selection = getSelection();

    CHECK(selection.length > 0, );

    const U2Region yRange = ui->getRowHeightController()->getRowsScreenRangeByNumbers(selection);
    const QRect selectionRect(0, yRange.startPos, width() - 1, yRange.length - 1);
    CHECK(selectionRect.isValid(), );

    painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    painter.drawRect(selectionRect);
}


/** Used to visualize cursor position. Used for debugging only today. */
#define SHOW_CURSOR_FRAME true

void MaEditorNameList::drawCursorFrame(QPainter& painter, const QRect& rect) {
#ifdef SHOW_FOCUS_FRAME
    QPen penBefore = painter.pen();
    painter.setPen(QPen(Qt::blue, 1, Qt::DotLine));
    painter.drawRect(rect);
    painter.setPen(penBefore);
#endif
}

void MaEditorNameList::sl_editSequenceName() {
    GRUNTIME_NAMED_COUNTER(cvat, tvar, "Rename row", editor->getFactoryId());
    MultipleAlignmentObject* maObj = editor->getMaObject();
    CHECK(!maObj->isStateLocked(), );

    bool ok = false;
    int n = getSelectedRow();
    CHECK(n >= 0, );

    QString curName =  maObj->getMultipleAlignment()->getRow(n)->getName();

    bool isMca = this->editor->getMaObject()->getGObjectType() == GObjectTypes::MULTIPLE_CHROMATOGRAM_ALIGNMENT;
    QString title = isMca ? tr("Rename Read") : tr("Rename Sequence");
    QString newName = QInputDialog::getText(ui, title, tr("New name:"), QLineEdit::Normal, curName, &ok);

    if (ok && !newName.isEmpty() && curName != newName) {
        emit si_sequenceNameChanged(curName, newName);
        maObj->renameRow(n,newName);
    }
}

void MaEditorNameList::mouseDoubleClickEvent(QMouseEvent *e) {
    Q_UNUSED(e);
    if (e->button() == Qt::LeftButton) {
        sl_editSequenceName();
    }
}

void MaEditorNameList::moveSelectedRegion(int shift) {
    CHECK(shift != 0, );
    MultipleAlignmentObject* maObj = editor->getMaObject();
    CHECK(!maObj->isStateLocked(),);

    U2Region selection = getSelection();
    int numRowsInSelection = selection.length;
    int firstRowInSelection = selection.startPos;
    int lastRowInSelection = selection.endPos() - 1;

    // "out-of-range" checks
    if ((shift > 0 && lastRowInSelection + shift >= editor->getNumSequences())
        || (shift < 0 && firstRowInSelection + shift < 0)
        || (shift < 0 && firstRowInSelection + qAbs(shift) > editor->getNumSequences()))
    {
        return;
    }
    maObj->moveRowsBlock(firstRowInSelection, numRowsInSelection, shift);
    const QPoint& cursorPosition = editor->getCursorPosition();
    editor->setCursorPosition(QPoint(cursorPosition.x(), cursorPosition.y() + shift));
    setSelection(firstRowInSelection + shift, numRowsInSelection);
}

qint64 MaEditorNameList::sequenceIdAtPos(const QPoint &p) {
    int rowIndex = ui->getRowHeightController()->screenYPositionToRowNumber(p.y());
    CHECK(ui->getSequenceArea()->isSeqInRange(rowIndex), U2MsaRow::INVALID_ROW_ID);
    CHECK(rowIndex >= 0, U2MsaRow::INVALID_ROW_ID);
    MultipleAlignmentObject* maObj = editor->getMaObject();
    return maObj->getMultipleAlignment()->getRow(ui->getCollapseModel()->mapToRow(rowIndex))->getRowId();
}

void MaEditorNameList::clearGroupsSelections() {
    groupColors.clear();
}

void MaEditorNameList::moveSelection(int offset) {
    const QPoint& cursorPosition = editor->getCursorPosition();
    int rowCount = ui->getSequenceArea()->getNumDisplayableSequences();
    if (offset != 0) {
        QPoint newCursorPosition = QPoint(cursorPosition.x(), qBound(0, cursorPosition.y() + offset, rowCount - 1));
        editor->setCursorPosition(newCursorPosition);
    }
    U2Region oldSelection = getSelection();
    int maxSelectionStart = rowCount - (int) oldSelection.length;
    int newSelectionStart = qBound(0, (int) oldSelection.startPos + offset, maxSelectionStart);
    ui->getSequenceArea()->moveSelection(newSelectionStart, oldSelection.length);
    setSelection(newSelectionStart, oldSelection.length);

    scrollSelectionToView(offset >= 0);
}

void MaEditorNameList::scrollSelectionToView(bool fromStart) {
    U2Region selection = getSelection();
    int height = ui->getSequenceArea()->height();
    ui->getScrollController()->scrollToRowByNumber(fromStart ? selection.startPos : selection.endPos() - 1, height);
}

} // namespace U2
