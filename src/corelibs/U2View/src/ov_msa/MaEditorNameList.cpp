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

#include <QApplication>
#include <QClipboard>
#include <QInputDialog>
#include <QMouseEvent>
#include <QPainter>

#include <U2Core/U2Mod.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/GUIUtils.h>

#include "MaEditorNameList.h"

#include "MSAEditor.h"
#include "McaEditor.h"
#include "McaEditorSequenceArea.h"

#include "view_rendering/MaEditorWgt.h"
#include "view_rendering/MaEditorSequenceArea.h"
#include "view_rendering/SequenceWithChromatogramAreaRenderer.h"

namespace U2 {

#define CHILDREN_OFFSET 8

MaEditorNameList::MaEditorNameList(MaEditorWgt* _ui, QScrollBar* _nhBar)
    : labels(NULL),
      ui(_ui),
      nhBar(_nhBar),
      singleSelecting(false),
      editor(_ui->getEditor())
{
    setObjectName("msa_editor_name_list");
    setFocusPolicy(Qt::WheelFocus);
    cachedView = new QPixmap();
    completeRedraw = true;
    scribbling = false;
    shifting = false;
    curSeq = 0;
    startSelectingSeq = curSeq;
    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    connect(editor, SIGNAL(si_buildStaticMenu(GObjectView*, QMenu*)), SLOT(sl_buildStaticMenu(GObjectView*, QMenu*)));

    editSequenceNameAction = new QAction(tr("Edit sequence name"), this);
    connect(editSequenceNameAction, SIGNAL(triggered()), SLOT(sl_editSequenceName()));

    copyCurrentSequenceAction = new QAction(tr("Copy current sequence"), this);
    copyCurrentSequenceAction->setObjectName("Copy current sequence");
    connect(copyCurrentSequenceAction, SIGNAL(triggered()), SLOT(sl_copyCurrentSequence()));

    removeSequenceAction = new QAction(tr("Remove sequence(s)"), this);
    removeSequenceAction->setObjectName("Remove sequence");
    connect(removeSequenceAction, SIGNAL(triggered()), SLOT(sl_removeSequence()));
    addAction(removeSequenceAction);

    connect(editor, SIGNAL(si_buildPopupMenu(GObjectView* , QMenu*)), SLOT(sl_buildContextMenu(GObjectView*, QMenu*)));
    if (editor->getMaObject()) {
        connect(editor->getMaObject(), SIGNAL(si_alignmentChanged(const MultipleAlignment&, const MaModificationInfo&)),
            SLOT(sl_alignmentChanged(const MultipleAlignment&, const MaModificationInfo&)));
        connect(editor->getMaObject(), SIGNAL(si_lockedStateChanged()), SLOT(sl_lockedStateChanged()));
    }

    connect(this,   SIGNAL(si_startMsaChanging()),
            ui,     SIGNAL(si_startMsaChanging()));
    connect(this,   SIGNAL(si_stopMsaChanging(bool)),
            ui,     SIGNAL(si_stopMsaChanging(bool)));

    if (ui->getSequenceArea()) {
        connect(ui->getSequenceArea(), SIGNAL(si_startChanged(const QPoint &, const QPoint &)), SLOT(sl_startChanged(const QPoint &, const QPoint &)));
        connect(ui->getSequenceArea(), SIGNAL(si_selectionChanged(const MaEditorSelection &, const MaEditorSelection &)),
            SLOT(sl_selectionChanged(const MaEditorSelection &, const MaEditorSelection &)));
        connect(ui->getEditor(), SIGNAL(si_fontChanged(const QFont&)), SLOT(sl_completeUpdate()));
        connect(ui->getSequenceArea()->getVBar(), SIGNAL(actionTriggered(int)), SLOT(sl_onScrollBarActionTriggered(int)));
    }
    connect(ui->getCollapseModel(), SIGNAL(toggled()), SLOT(sl_completeUpdate()));
    connect(editor, SIGNAL(si_referenceSeqChanged(qint64)), SLOT(sl_referenceSeqChanged(qint64)));
    connect(editor, SIGNAL(si_completeUpdate()), SLOT(sl_completeUpdate()));

    nhBar->setParent(this);
    nhBar->setVisible(false);
    updateActions();

    QObject *labelsParent = new QObject(this);
    labelsParent->setObjectName("labels_parent");
    labels = new QObject(labelsParent);
}

MaEditorNameList::~MaEditorNameList() {
    delete cachedView;
}

void MaEditorNameList::drawNames(QPixmap &p, const QList<qint64> &seqIdx, bool drawSelection) {
    CHECK(!seqIdx.isEmpty(), );

    SAFE_POINT(NULL != ui, tr("MSA Editor UI is NULL"), );
    MaEditorSequenceArea* seqArea = ui->getSequenceArea();
    SAFE_POINT(NULL != seqArea, tr("MSA Editor sequence area is NULL"), );
    CHECK(!seqArea->isAlignmentEmpty(), );

    CHECK(ui->getEditor()->getRowHeight() * seqIdx.size() < 32768, );

    p = QPixmap(width(), ui->getEditor()->getRowHeight() * seqIdx.size());

    QPainter painter(&p);
    drawNames(painter, seqIdx, drawSelection);
}

void MaEditorNameList::drawNames(QPainter &p, const QList<qint64> &seqIdx, bool drawSelection) {
    p.fillRect(QRect(0, 0, width(), ui->getEditor()->getRowHeight() * seqIdx.size()), Qt::white);

    MultipleAlignmentObject* msaObj = editor->getMaObject();
    SAFE_POINT(NULL != msaObj, tr("MSA Object is NULL"), );
    const MultipleAlignment al = msaObj->getMultipleAlignment();

    QStringList seqNames = al->getRowNames();
    for (qint64 i = 0; i < seqIdx.size(); i++) {
        SAFE_POINT(seqIdx[i] < seqNames.size(), tr("Invalid sequence index"), );
        bool isSelected = drawSelection && isRowInSelection(seqIdx[i]);
        drawSequenceItem(p, i, 0, getTextForRow(seqIdx[i]), isSelected);
    }
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

bool MaEditorNameList::isRowInSelection(int seqnum) {
    MaEditorSelection s = ui->getSequenceArea()->getSelection();
    int endPos = s.y() + s.height() - 1;
    return seqnum >= s.y() && seqnum <= endPos;
}

void MaEditorNameList::updateActions() {
    SAFE_POINT(NULL != ui, tr("MSA Editor UI is NULL"), );
    MaEditorSequenceArea* seqArea = ui->getSequenceArea();
    SAFE_POINT(NULL != seqArea, tr("MSA Editor sequence area is NULL"), );

    copyCurrentSequenceAction->setEnabled(!seqArea->isAlignmentEmpty());

    MultipleAlignmentObject* maObj = editor->getMaObject();
    if (maObj){
        removeSequenceAction->setEnabled(!maObj->isStateLocked() && getSelectedRow() != -1);
        editSequenceNameAction->setEnabled(!maObj->isStateLocked());
        addAction(ui->getCopySelectionAction());
        addAction(ui->getPasteAction());
    }
}

#define MARGIN_TEXT_LEFT 5
#define MARGIN_TEXT_TOP 2
#define MARGIN_TEXT_BOTTOM 2

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

    int availableWidth = width() - MARGIN_TEXT_LEFT;
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
    connect(nhBar, SIGNAL(valueChanged(int)), SLOT(sl_nameBarMoved(int)));
}

void MaEditorNameList::sl_buildStaticMenu(GObjectView* v, QMenu* m) {
    Q_UNUSED(v);
    buildMenu(m);
}

void MaEditorNameList::sl_buildContextMenu(GObjectView* v, QMenu* m) {
    Q_UNUSED(v);
    buildMenu(m);
}

void MaEditorNameList::buildMenu(QMenu* m) {
    QMenu* editMenu = GUIUtils::findSubMenu(m, MSAE_MENU_EDIT);
    SAFE_POINT(editMenu != NULL, "editMenu not found", );

    editMenu->insertAction(editMenu->actions().last(), removeSequenceAction);

    CHECK(qobject_cast<MSAEditor*>(editor) != NULL, );
    CHECK(rect().contains(mapFromGlobal(QCursor::pos())), );

    QMenu* copyMenu = GUIUtils::findSubMenu(m, MSAE_MENU_COPY);
    SAFE_POINT(copyMenu != NULL, "copyMenu not found", );
    copyMenu->addAction(copyCurrentSequenceAction);

    copyCurrentSequenceAction->setDisabled(getSelectedRow() == -1);
    editMenu->insertAction(editMenu->actions().first(), editSequenceNameAction);
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
        updateActions();
        updateScrollBar();
        update();
    }
}

void MaEditorNameList::sl_nameBarMoved(int) {
    completeRedraw = true;
    update();
}

void MaEditorNameList::sl_removeSequence() {
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
    updateActions();
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
    static int newSeq = 0;
    switch(key) {
    case Qt::Key_Up:
        if (0 != (Qt::ShiftModifier & e->modifiers()) && ui->getSequenceArea()->isSeqInRange(newSeq - 1)) {
            newSeq--;
            updateSelection(newSeq);
            ui->getSequenceArea()->updateVBarPosition(newSeq);
        } else if (0 == (Qt::ShiftModifier & e->modifiers())) {
            ui->getSequenceArea()->moveSelection(0, -1);
            if (0 <= curSeq - 1) {
                curSeq--;
            }
            if (0 <= startSelectingSeq - 1) {
                startSelectingSeq--;
            }
        }
        break;
    case Qt::Key_Down:
        if (0 != (Qt::ShiftModifier & e->modifiers()) && ui->getSequenceArea()->isSeqInRange(newSeq + 1)) {
            newSeq++;
            updateSelection(newSeq);
            ui->getSequenceArea()->updateVBarPosition(newSeq);
        } else if (0 == (Qt::ShiftModifier & e->modifiers())) {
            ui->getSequenceArea()->moveSelection(0, 1);
            if (ui->getSequenceArea()->getNumDisplayedSequences() > curSeq + 1) {
                curSeq++;
            }
            if (ui->getSequenceArea()->getNumDisplayedSequences() > startSelectingSeq + 1) {
                startSelectingSeq++;
            }
        }
        break;
    case Qt::Key_Left:
        nhBar->triggerAction(QAbstractSlider::SliderSingleStepSub);
        break;
    case Qt::Key_Right:
        nhBar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        break;
    case Qt::Key_Home:
        ui->getSequenceArea()->setFirstVisibleSequence(0);
        ui->getSequenceArea()->cancelSelection();
        //TODO: select first sequence?
        break;
    case Qt::Key_End:
        {
            int s = ui->getSequenceArea()->getNumDisplayedSequences() - 1;
            ui->getSequenceArea()->setFirstVisibleSequence(s);
            ui->getSequenceArea()->cancelSelection();
            //TODO: select last sequence?
        }
        break;
    case Qt::Key_PageUp:
        {
            int nVis = ui->getSequenceArea()->getNumVisibleSequences(false);
            int fp = qMax(0, ui->getSequenceArea()->getFirstVisibleSequence() - nVis);
            ui->getSequenceArea()->setFirstVisibleSequence(fp);
            ui->getSequenceArea()->cancelSelection();
        }
        break;
    case Qt::Key_PageDown:
        {
            int nVis = ui->getSequenceArea()->getNumVisibleSequences(false);
            int nSeq = ui->getSequenceArea()->getNumDisplayedSequences();
            int fp = qMin(nSeq-1, ui->getSequenceArea()->getFirstVisibleSequence() + nVis);
            ui->getSequenceArea()->setFirstVisibleSequence(fp);
            ui->getSequenceArea()->cancelSelection();
        }
        break;
    case Qt::Key_Shift:
        curSeq = startSelectingSeq;
        if (startSelectingSeq == ui->getCollapseModel()->rowToMap(ui->getSequenceArea()->getSelectedRows().startPos)) {
            newSeq = ui->getCollapseModel()->rowToMap(ui->getSequenceArea()->getSelectedRows().endPos() - 1);
        } else {
            newSeq = ui->getCollapseModel()->rowToMap(ui->getSequenceArea()->getSelectedRows().startPos);
        }
        break;
    case Qt::Key_Escape:
        ui->getSequenceArea()->cancelSelection();
        curSeq = 0;
        startSelectingSeq = 0;
        break;
    case Qt::Key_Delete:
        sl_removeSequence();
        break;
    }
    QWidget::keyPressEvent(e);
}

void MaEditorNameList::mousePressEvent(QMouseEvent *e) {
    SAFE_POINT(ui, "MSA Editor UI is NULL", );
    MaEditorSequenceArea* seqArea = ui->getSequenceArea();
    SAFE_POINT(seqArea, "MSA Editor sequence area", );

    if (seqArea->isAlignmentEmpty()) {
        QWidget::mousePressEvent(e);
        return;
    }

    if ((e->button() == Qt::LeftButton)) {
        emit si_startMsaChanging();

        if(Qt::ShiftModifier == e->modifiers()) {
            QWidget::mousePressEvent(e);
            scribbling = true;
            return;
        }
        origin = e->pos();
        curSeq = seqArea->getSequenceNumByY(e->y());
        if (ui->isCollapsibleMode()) {
            MSACollapsibleItemModel* m = ui->getCollapseModel();
            if(curSeq >= m->displayedRowsCount()){
                curSeq = m->displayedRowsCount() - 1;
            }
            if (m->isTopLevel(curSeq)) {
                const U2Region& yRange = seqArea->getSequenceYRange(curSeq, true);
                bool selected = isRowInSelection(curSeq);
                QRect textRect = calculateTextRect(yRange, selected);
                QRect buttonRect = calculateButtonRect(textRect);
                if (buttonRect.contains(origin)) {
                    m->toggle(curSeq);
                    QWidget::mousePressEvent(e);
                    return;
                }
            }
        }

        // SANGER_TODO: there should not be such stuff
        McaEditor* mcaEditor = qobject_cast<McaEditor*>(editor);
        if (mcaEditor != NULL) {
            const U2Region& yRange = seqArea->getSequenceYRange(curSeq, true);
            bool selected = isRowInSelection(curSeq);
            QRect textRect = calculateTextRect(yRange, selected);
            QRect buttonRect = calculateButtonRect(textRect);
            if (buttonRect.contains(origin)) {
                mcaEditor->toggleChromVisibility(curSeq);
                QWidget::mousePressEvent(e);
                return;
            }
        }

        startSelectingSeq = curSeq;
        U2Region s = getSelection();
        if (s.contains(curSeq)) {
            if (!ui->isCollapsibleMode()) {
                shifting = true;
            }
        } else {
            if (!seqArea->isSeqInRange(startSelectingSeq)) {
                if (e->y() < origin.y()) {
                    startSelectingSeq = 0;
                } else {
                    startSelectingSeq = ui->getEditor()->getNumSequences() - 1;
                }
            }
            rubberBand->setGeometry(QRect(origin, QSize()));
            rubberBand->show();
            seqArea->cancelSelection();
            scribbling = true;
        }
        if (seqArea->isSeqInRange(curSeq)) {
            singleSelecting = true;
            scribbling = true;
        }
    }

    QWidget::mousePressEvent(e);
}

void MaEditorNameList::mouseMoveEvent(QMouseEvent* e) {
    if ((e->buttons() & Qt::LeftButton) && scribbling) {
        int newSeqNum = ui->getSequenceArea()->getSequenceNumByY(e->pos().y());
        if (ui->getSequenceArea()->isSeqInRange(newSeqNum)) {
            ui->getSequenceArea()->updateVBarPosition(newSeqNum);
            if (singleSelecting) {
                singleSelecting = false;
            }
        }
        if (shifting) {
            assert(!ui->isCollapsibleMode());
            moveSelectedRegion(newSeqNum - curSeq);
        } else {
            rubberBand->setGeometry(QRect(origin, e->pos()).normalized());
        }
    }
    QWidget::mouseMoveEvent(e);
}

void MaEditorNameList::mouseReleaseEvent(QMouseEvent *e) {
    rubberBand->hide();
    if (scribbling) {
        int newSeq = ui->getSequenceArea()->getSequenceNumByY(qMax(e->y(), 0));
        if (!ui->getSequenceArea()->isSeqInRange(newSeq)) {
            if (e->y() < origin.y()) {
                newSeq = 0;
            } else {
                newSeq = ui->getSequenceArea()->getNumDisplayedSequences() - 1;
            }
        }
        if (e->pos() == origin) {
            // special case: click but don't drag
            shifting = false;
        }
        if (shifting) {
            assert(!ui->isCollapsibleMode());
            int shift = 0;
            int numSeq = ui->getSequenceArea()->getNumDisplayedSequences();
            int selectionStart = getSelection().startPos;
            int selectionSize = getSelection().length;
            if (newSeq == 0) {
                shift = -selectionStart;
            } else if (newSeq == numSeq - 1) {
                shift = numSeq - (selectionStart + selectionSize);
            } else {
                shift = newSeq - curSeq;
            }
            moveSelectedRegion(shift);
            shifting = false;

            emit si_stopMsaChanging(true);
        } else {
            ui->getSequenceArea()->setSelection(MaEditorSelection());

            int firstVisibleRow = ui->getSequenceArea()->getFirstVisibleSequence();
            int lastVisibleRow = ui->getSequenceArea()->getNumVisibleSequences(true) + firstVisibleRow - 1;
            bool selectionContainsSeqs = (startSelectingSeq <= lastVisibleRow || newSeq <= lastVisibleRow);

            if (selectionContainsSeqs) {
                if (singleSelecting) {
                    curSeq = newSeq;
                    singleSelecting = false;
                } else {
                    curSeq = (startSelectingSeq < firstVisibleRow) ? firstVisibleRow : startSelectingSeq;
                    curSeq = (startSelectingSeq > lastVisibleRow) ? lastVisibleRow : startSelectingSeq;
                    if (newSeq > lastVisibleRow || newSeq < firstVisibleRow) {
                        newSeq = newSeq > 0 ? lastVisibleRow : 0;
                    }
                }
                updateSelection(newSeq);
            }
            emit si_stopMsaChanging(false);
        }
        scribbling = false;
    } else {
        emit si_stopMsaChanging(false);
    }
    ui->getSequenceArea()->getVBar()->setupRepeatAction(QAbstractSlider::SliderNoAction);

    QWidget::mouseReleaseEvent(e);
}

void MaEditorNameList::updateSelection(int newSeq) {
    CHECK(ui->getSequenceArea()->isSeqInRange(newSeq) || ui->getSequenceArea()->isSeqInRange(curSeq), );

    setSelection(qMin(curSeq, newSeq), qAbs(newSeq - curSeq) + 1);
}

void MaEditorNameList::wheelEvent(QWheelEvent *we) {
    bool toMin = we->delta() > 0;
    ui->getSequenceArea()->getVBar()->triggerAction(toMin ? QAbstractSlider::SliderSingleStepSub : QAbstractSlider::SliderSingleStepAdd);
    QWidget::wheelEvent(we);
}

void MaEditorNameList::sl_startChanged(const QPoint& p, const QPoint& prev) {
    if (p.y() == prev.y()) {
        return;
    }
    completeRedraw = true;
    update();
}

void MaEditorNameList::sl_referenceSeqChanged(qint64) {
    completeRedraw = true;
    update();
}

void MaEditorNameList::updateContent() {
    completeRedraw = true;
    update();
}

void MaEditorNameList::sl_selectionChanged(const MaEditorSelection& current, const MaEditorSelection& prev)
{
    Q_UNUSED(current);
    Q_UNUSED(prev);

    if (current.y() == prev.y() && current.height() == prev.height()) {
        return;
    }
    completeRedraw = true;
    update();
    updateActions();
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
    int w = width();
    int textX = MARGIN_TEXT_LEFT;
    int textW = w - MARGIN_TEXT_LEFT;
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

void MaEditorNameList::drawContent(QPainter& p) {
    p.fillRect(cachedView->rect(), Qt::white);

    SAFE_POINT(NULL != ui, "MA Editor UI is NULL", );
    MaEditorSequenceArea* seqArea = ui->getSequenceArea();
    SAFE_POINT(NULL != seqArea, "MA Editor sequence area is NULL", );

    if (seqArea->isAlignmentEmpty()) {
        return;
    }

    int startSeq = seqArea->getFirstVisibleSequence();
    int lastSeq = qBound(startSeq, seqArea->getLastVisibleSequence(true) + 1, seqArea->getNumDisplayedSequences() - 1);

    if (labels) {
        labels->setObjectName("");
    }

    MultipleAlignmentObject* msaObj = editor->getMaObject();
    SAFE_POINT(NULL != msaObj, "NULL Msa Object in MSAEditorNameList::drawContent!",);

    const MultipleAlignment al = msaObj->getMultipleAlignment();


    U2OpStatusImpl os;
    int referenceId = editor->getReferenceRowId() == U2MsaRow::INVALID_ROW_ID ? U2MsaRow::INVALID_ROW_ID
                                                                              : msaObj->getMultipleAlignment()->getRowIndexByRowId(editor->getReferenceRowId(), os);
    SAFE_POINT_OP(os, );

    if (ui->isCollapsibleMode()) {
        MSACollapsibleItemModel* m = ui->getCollapseModel();
        QVector<U2Region> range;
        m->getVisibleRows(startSeq, lastSeq, range);
        U2Region yRange = seqArea->getSequenceYRange(startSeq, true);
        int numRows = al->getNumRows();

        int pos = startSeq;
        foreach(const U2Region& r, range) {
            int end = qMin(numRows, static_cast<int>(r.endPos()));
            for (int s = r.startPos; s < end; s++) {
                bool isSelected = isRowInSelection(pos);
                if (m->itemAt(pos) < 0) {
                    p.translate(CROSS_SIZE * 2, 0);
                    drawSequenceItem(p, getTextForRow(s), yRange, isSelected, s == referenceId);
                    p.translate(- CROSS_SIZE * 2, 0);
                } else {
                    const MSACollapsableItem& item = m->getItem(m->itemAt(pos));
                    QRect rect = calculateTextRect(yRange, isSelected);
                    // SANGER_TODO: check reference
                    if (pos == m->getItemPos(m->itemAt(pos))) {
                        drawCollapsibileSequenceItem(p, getTextForRow(s), rect, isSelected, item.isCollapsed, s == referenceId);
                    } else {
                        drawChildSequenceItem(p, getTextForRow(s), rect, isSelected, s == referenceId);
                    }
                }
                yRange.startPos += ui->getEditor()->getRowHeight();
                pos++;
            }
        }
    } else {
        for (int s = startSeq; s <= lastSeq; s++) {
            bool isSelected = isRowInSelection(s);
            drawSequenceItem(p, s, startSeq, getTextForRow(s), isSelected);
        }
    }
}

void MaEditorNameList::drawSequenceItem(QPainter& p, const QString& text, const U2Region& yRange, bool selected, bool isReference) {
    QRect rect = calculateTextRect(yRange, selected);

    MultipleAlignmentObject* maObj = editor->getMaObject();
    CHECK(maObj != NULL, );
    drawBackground(p, text, rect, isReference);
    drawText(p, text, rect, selected);
}

void MaEditorNameList::drawSequenceItem(QPainter &p, int row, int firstVisibleRow, const QString &text, bool selected) {
    U2Region yRange = ui->getSequenceArea()->getSequenceYRange(row, firstVisibleRow, true);
    // SANGER_TODO: simplify getting the reference status - no reference here!
    MultipleAlignmentObject* maObj = editor->getMaObject();
    CHECK(maObj != NULL, );
    U2OpStatusImpl os;
    bool isReference = row == maObj->getMultipleAlignment()->getRowIndexByRowId(editor->getReferenceRowId(), os);
    drawSequenceItem(p, text, yRange, selected, isReference);
}

void MaEditorNameList::drawCollapsibileSequenceItem(QPainter &p, const QString &name, const QRect& rect,
                                                    bool selected, bool collapsed, bool isReference) {
    drawBackground(p, name, rect, isReference);
    drawCollapsePrimitive(p, collapsed, rect);
    p.translate(CROSS_SIZE * 2, 0);
    drawText(p, name, rect, selected);
    p.translate( - CROSS_SIZE * 2, 0);
}

void MaEditorNameList::drawChildSequenceItem(QPainter &p, const QString &name, const QRect& rect,
                                             bool selected, bool isReference) {
    drawBackground(p, name, rect, isReference);
    p.translate(CROSS_SIZE * 2 + CHILDREN_OFFSET, 0);
    drawText(p, name, rect, selected);
    p.translate( - CROSS_SIZE * 2 - CHILDREN_OFFSET, 0);
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

void MaEditorNameList::drawSelection(QPainter& p) {
    U2Region sel = getSelection();
    CHECK(!sel.isEmpty(), );

    int w = width();
    U2Region yRange = ui->getSequenceArea()->getSequenceYRange(sel.startPos, true);
    QRect itemsRect(0, yRange.startPos, w - 1, sel.length*yRange.length -1);

    p.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    p.drawRect(itemsRect);
}

void MaEditorNameList::sl_onScrollBarActionTriggered(int scrollAction)
{
    if (scrollAction ==  QAbstractSlider::SliderSingleStepAdd || scrollAction == QAbstractSlider::SliderSingleStepSub) {
        if (scribbling) {
            // TODO: sync origin point
            /*int y = origin.y();
            int step = ui->getSequenceArea()->getVBar()->singleStep();
            if (scrollAction == QAbstractSlider::SliderSingleStepAdd) {
                origin.setY(y - step);
            } else {
                origin.setY(y + step);
            }*/
        }
    }
}

void MaEditorNameList::sl_editSequenceName() {
    MultipleAlignmentObject* maObj = editor->getMaObject();
    CHECK(!maObj->isStateLocked(), );

    bool ok = false;
    int n = getSelectedRow();
    CHECK(n >= 0, );

    QString curName =  maObj->getMultipleAlignment()->getRow(n)->getName();
    QString newName = QInputDialog::getText(this, tr("Rename"),
            tr("New sequence name:"), QLineEdit::Normal, curName, &ok);
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

    MultipleAlignmentObject* maObj = editor->getMaObject();
    if (!maObj->isStateLocked()) {
        maObj->moveRowsBlock(firstRowInSelection, numRowsInSelection, shift);
        curSeq += shift;
        startSelectingSeq = curSeq;
        setSelection(firstRowInSelection + shift, numRowsInSelection);
    }
}

qint64 MaEditorNameList::sequenceIdAtPos(const QPoint &p) {
    qint64 result = U2MsaRow::INVALID_ROW_ID;
    curSeq = ui->getSequenceArea()->getSequenceNumByY(p.y());
    if (!ui->getSequenceArea()->isSeqInRange(curSeq)) {
        return result;
    }
    if (ui->isCollapsibleMode()) {
        curSeq = ui->getCollapseModel()->mapToRow(curSeq);
    }
    if (curSeq != -1) {
        MultipleAlignmentObject* maObj = editor->getMaObject();
        result = maObj->getMultipleAlignment()->getRow(curSeq)->getRowId();
    }
    return result;
}

void MaEditorNameList::clearGroupsSelections() {
    groupColors.clear();
}

McaEditorNameList::McaEditorNameList(McaEditorWgt *ui, QScrollBar *nhBar)
    : MaEditorNameList(ui, nhBar) {
    connect(this, SIGNAL(si_selectionChanged()),
            ui->getSequenceArea(), SLOT(sl_backgroundSelectionChanged()));
}

void McaEditorNameList::sl_selectionChanged(const MaEditorSelection& current, const MaEditorSelection& )
{
    setSelection(current.y(), current.height());
    updateActions();
}

void McaEditorNameList::drawSequenceItem(QPainter& p, int row, int firstVisibleRow, const QString& text, bool selected) {
    U2Region yRange = ui->getSequenceArea()->getSequenceYRange(row, firstVisibleRow, true);
    QRect textRect = calculateTextRect(yRange, selected);

    McaEditor* mcaEditor = getEditor();
    SAFE_POINT(mcaEditor != NULL, "McaEditor is NULL", );

    drawCollapsibileSequenceItem(p, text, textRect, selected, !mcaEditor->isChromVisible(row), false);
}

U2Region McaEditorNameList::getSelection() const {
    return localSelection;
}

void McaEditorNameList::setSelection(int startSeq, int count) {
    localSelection = U2Region(startSeq, count);
    completeRedraw = true;
    update();
    updateActions();
    emit si_selectionChanged();
}

bool McaEditorNameList::isRowInSelection(int row) {
    return localSelection.contains(row);
}

McaEditor* McaEditorNameList::getEditor() const {
    return qobject_cast<McaEditor*>(editor);
}

MsaEditorNameList::MsaEditorNameList(MaEditorWgt *ui, QScrollBar *nhBar)
    : MaEditorNameList(ui, nhBar) {

}

MSAEditor* MsaEditorNameList::getEditor() const {
    return qobject_cast<MSAEditor*>(editor);
}

} // namespace U2
