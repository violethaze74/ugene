/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2023 UniPro <ugene@unipro.ru>
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

#include <drivers/GTMouseDriver.h>
#include <primitives/GTScrollBar.h>
#include <utils/GTThread.h>

#include <QApplication>
#include <QMainWindow>

#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2View/ADVSingleSequenceWidget.h>
#include <U2View/BaseWidthController.h>
#include <U2View/DrawHelper.h>
#include <U2View/McaEditor.h>
#include <U2View/McaEditorConsensusArea.h>
#include <U2View/McaEditorNameList.h>
#include <U2View/McaEditorReferenceArea.h>
#include <U2View/McaEditorSequenceArea.h>
#include <U2View/RowHeightController.h>
#include <U2View/SequenceObjectContext.h>

#include "GTUtilsMcaEditor.h"
#include "GTUtilsMcaEditorSequenceArea.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProjectTreeView.h"

namespace U2 {
using namespace HI;

#define GT_CLASS_NAME "GTUtilsMcaEditorSequenceArea"

#define GT_METHOD_NAME "getSequenceArea"
McaEditorSequenceArea* GTUtilsMcaEditorSequenceArea::getSequenceArea() {
    QWidget* activeWindow = GTUtilsMcaEditor::getActiveMcaEditorWindow();
    return GTWidget::findExactWidget<McaEditorSequenceArea*>("mca_editor_sequence_area", activeWindow);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getVisibleNames"
QStringList GTUtilsMcaEditorSequenceArea::getVisibleNames() {
    McaEditor* editor = GTUtilsMcaEditor::getEditor();
    McaEditorNameList* nameListArea = GTUtilsMcaEditor::getNameListArea();
    CHECK_SET_ERR_RESULT(nullptr != nameListArea, "Mca Editor name list area is NULL", QStringList());

    const QList<int> visibleRowsIndexes = editor->getUI()->getDrawHelper()->getVisibleMaRowIndexes(
        nameListArea->height());

    QStringList visibleRowNames;
    foreach (const int rowIndex, visibleRowsIndexes) {
        visibleRowNames << editor->getMaObject()->getRow(rowIndex)->getName();
    }
    return visibleRowNames;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getRowHeight"
int GTUtilsMcaEditorSequenceArea::getRowHeight(int rowNumber) {
    McaEditorWgt* ui = GTUtilsMcaEditor::getEditorUi();
    return ui->getRowHeightController()->getRowHeightByViewRowIndex(rowNumber);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickToPosition"
void GTUtilsMcaEditorSequenceArea::clickToPosition(const QPoint& globalMaPosition) {
    auto mcaSeqArea = GTWidget::findExactWidget<McaEditorSequenceArea*>("mca_editor_sequence_area", GTUtilsMcaEditor::getActiveMcaEditorWindow());
    GT_CHECK(mcaSeqArea->isInRange(globalMaPosition),
             QString("Position is out of range: [%1, %2], range: [%3, %4]")
                 .arg(globalMaPosition.x())
                 .arg(globalMaPosition.y())
                 .arg(mcaSeqArea->getEditor()->getAlignmentLen())
                 .arg(mcaSeqArea->getViewRowCount()));

    scrollToPosition(globalMaPosition);
    GTGlobals::sleep();

    BaseWidthController* widthController = mcaSeqArea->getEditor()->getUI()->getBaseWidthController();
    RowHeightController* heightController = mcaSeqArea->getEditor()->getUI()->getRowHeightController();
    QPoint positionCenter(widthController->getBaseScreenCenter(globalMaPosition.x()),
                          heightController->getScreenYRegionByViewRowIndex(globalMaPosition.y()).center());
    GT_CHECK(mcaSeqArea->rect().contains(positionCenter, false), "Position is not visible");

    GTMouseDriver::moveTo(mcaSeqArea->mapToGlobal(positionCenter));
    GTMouseDriver::click();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "scrollToPosition"
void GTUtilsMcaEditorSequenceArea::scrollToPosition(const QPoint& position) {
    auto mcaSeqArea = GTWidget::findExactWidget<McaEditorSequenceArea*>("mca_editor_sequence_area", GTUtilsMcaEditor::getActiveMcaEditorWindow());
    GT_CHECK(mcaSeqArea->isInRange(position),
             QString("Position is out of range: [%1, %2], range: [%3, %4]")
                 .arg(position.x())
                 .arg(position.y())
                 .arg(mcaSeqArea->getEditor()->getAlignmentLen())
                 .arg(mcaSeqArea->getViewRowCount()));

    CHECK(!mcaSeqArea->isVisible(position, false), );

    if (!mcaSeqArea->isRowVisible(position.y(), false)) {
        GTUtilsMcaEditor::scrollToRead(position.y());
    }
    GTThread::waitForMainThread();

    if (!mcaSeqArea->isPositionVisible(position.x(), false)) {
        scrollToBase(position.x());
    }
    GTThread::waitForMainThread();

    CHECK_SET_ERR(mcaSeqArea->isVisible(position, false),
                  QString("The position is still invisible after scrolling: (%1, %2), last visible base: %3")
                      .arg(position.x())
                      .arg(position.y())
                      .arg(mcaSeqArea->getLastVisibleBase(false)));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "scrollToBase"
void GTUtilsMcaEditorSequenceArea::scrollToBase(int position) {
    BaseWidthController* widthController = GTUtilsMcaEditor::getEditorUi()->getBaseWidthController();
    int scrollBarValue = widthController->getBaseGlobalRange(position).center() -
                         GTUtilsMcaEditor::getEditorUi()->getSequenceArea()->width() / 2;
    GTScrollBar::moveSliderWithMouseToValue(GTUtilsMcaEditor::getHorizontalScrollBar(), scrollBarValue);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickCollapseTriangle"
void GTUtilsMcaEditorSequenceArea::clickCollapseTriangle(QString rowName, bool showChromatogram) {
    auto mcaEditArea = GTWidget::findExactWidget<McaEditorSequenceArea*>("mca_editor_sequence_area");

    int viewRowIndex = getVisibleNames().indexOf(rowName);
    GT_CHECK(viewRowIndex != -1, "sequence not found in nameList");
    auto nameList = GTWidget::findWidget("mca_editor_name_list");
    RowHeightController* rowHeightController = mcaEditArea->getEditor()->getMaEditorWgt(0)->getRowHeightController();
    int yPos = rowHeightController->getScreenYRegionByViewRowIndex(viewRowIndex).startPos + rowHeightController->getRowHeightByViewRowIndex(viewRowIndex) / 2;
    if (showChromatogram) {
        yPos -= 65;
    }
    QPoint localCoord = QPoint(15, yPos);
    QPoint globalCoord = nameList->mapToGlobal(localCoord);
    GTMouseDriver::moveTo(globalCoord);
    GTMouseDriver::click();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isChromatogramShown"
bool GTUtilsMcaEditorSequenceArea::isChromatogramShown(QString rowName) {
    GTThread::waitForMainThread();
    auto mcaEditArea = GTWidget::findExactWidget<McaEditorSequenceArea*>("mca_editor_sequence_area");
    int rowNum = GTUtilsMcaEditor::getReadsNames().indexOf(rowName);
    GT_CHECK_RESULT(rowNum != -1, "sequence not found in nameList", false);
    int rowHeight = mcaEditArea->getEditor()->getMaEditorWgt(0)->getRowHeightController()->getRowHeightByViewRowIndex(rowNum);
    bool isChromatogramShown = rowHeight > 100;
    return isChromatogramShown;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getNameList"
QStringList GTUtilsMcaEditorSequenceArea::getNameList() {
    QMainWindow* mw = AppContext::getMainWindow()->getQMainWindow();
    McaEditor* editor = mw->findChild<McaEditor*>();
    CHECK_SET_ERR_RESULT(editor != nullptr, "MsaEditor not found", QStringList());

    QStringList result = editor->getMaObject()->getMultipleAlignment()->getRowNames();

    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "callContextMenu"
void GTUtilsMcaEditorSequenceArea::callContextMenu(const QPoint& innerCoords) {
    if (innerCoords.isNull()) {
        GTWidget::click(getSequenceArea(), Qt::RightButton);
    } else {
        moveTo(innerCoords);
        GTMouseDriver::click(Qt::RightButton);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "moveTo"
void GTUtilsMcaEditorSequenceArea::moveTo(const QPoint& p) {
    QPoint convP = convertCoordinates(p);

    GTMouseDriver::moveTo(convP);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "convertCoordinates"
QPoint GTUtilsMcaEditorSequenceArea::convertCoordinates(const QPoint p) {
    QWidget* activeWindow = GTUtilsMcaEditor::getActiveMcaEditorWindow();
    auto mcaEditArea = GTWidget::findExactWidget<McaEditorSequenceArea*>("mca_editor_sequence_area", activeWindow);

    const int posX = static_cast<int>(mcaEditArea->getEditor()->getUI()->getBaseWidthController()->getBaseGlobalRange(p.x()).center());
    const int posY = static_cast<int>(mcaEditArea->getEditor()->getUI()->getRowHeightController()->getGlobalYRegionByViewRowIndex(p.y()).center());
    return mcaEditArea->mapToGlobal(QPoint(posX, posY));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getReferenceReg"
QString GTUtilsMcaEditorSequenceArea::getReferenceReg(int num, int length) {
    McaEditor* editor = GTUtilsMcaEditor::getEditor();
    MultipleChromatogramAlignmentObject* obj = editor->getMaObject();
    GT_CHECK_RESULT(obj != nullptr, "MultipleChromatogramAlignmentObject not found", QString());

    U2OpStatus2Log status;
    QByteArray seq = obj->getReferenceObj()->getSequenceData(U2Region(num, length), status);
    CHECK_OP(status, QString());

    return seq;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSelectedReferenceReg"
QString GTUtilsMcaEditorSequenceArea::getSelectedReferenceReg() {
    McaEditor* editor = GTUtilsMcaEditor::getEditor();
    MultipleChromatogramAlignmentObject* obj = editor->getMaObject();
    GT_CHECK_RESULT(obj != nullptr, "MultipleChromatogramAlignmentObject not found", QString());

    U2Region sel = GTUtilsMcaEditorSequenceArea::getReferenceSelection();
    int num = sel.startPos;
    int length = sel.length;

    U2OpStatus2Log status;
    QByteArray seq = obj->getReferenceObj()->getSequenceData(U2Region(num, length), status);

    return seq;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "moveTheBorderBetweenAlignmentAndRead"
void GTUtilsMcaEditorSequenceArea::moveTheBorderBetweenAlignmentAndRead(int shift) {
    QStringList visible = getVisibleNames();
    GT_CHECK_RESULT(visible.size() != 0, "No visible reads", );
    QString firstVisible = visible.first();

    const QRect sequenceNameRect = GTUtilsMcaEditor::getReadNameRect(firstVisible);
    GTMouseDriver::moveTo(QPoint(sequenceNameRect.right() + 2, sequenceNameRect.center().y()));
    GTMouseDriver::press(Qt::LeftButton);
    GTGlobals::sleep(1000);
    GTMouseDriver::moveTo(QPoint(sequenceNameRect.right() + 2 + shift, sequenceNameRect.center().y()));
    GTMouseDriver::release(Qt::LeftButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "dragAndDrop"
void GTUtilsMcaEditorSequenceArea::dragAndDrop(const QPoint p) {
    GTMouseDriver::click();
    GTGlobals::sleep(1000);
    GTMouseDriver::press(Qt::LeftButton);
    GTGlobals::sleep(1000);
    GTMouseDriver::moveTo(p);
    GTMouseDriver::release(Qt::LeftButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSelectedRowsNum"
U2Region GTUtilsMcaEditorSequenceArea::getSelectedRowsNum() {
    McaEditor* mcaEditor = GTUtilsMcaEditor::getEditor();
    CHECK_SET_ERR_RESULT(mcaEditor != nullptr, "McaEditor not found", U2Region());

    const MaEditorSelection& selection = mcaEditor->getSelection();
    return U2Region::fromYRange(selection.toRect());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSelectedRowsNames"
QStringList GTUtilsMcaEditorSequenceArea::getSelectedRowsNames() {
    U2Region sel = getSelectedRowsNum();
    QStringList names = getNameList();

    QStringList res;
    for (int i = sel.startPos; i < sel.endPos(); i++) {
        res << names[i];
    }

    return res;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSelectedRect"
QRect GTUtilsMcaEditorSequenceArea::getSelectedRect() {
    auto mcaEditArea = GTWidget::findExactWidget<McaEditorSequenceArea*>("mca_editor_sequence_area");

    return mcaEditArea->getEditor()->getSelection().toRect();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getPositionRect"
QRect GTUtilsMcaEditorSequenceArea::getPositionRect(int rowIndex, int referenceBaseIndex) {
    McaEditorWgt* mcaWidget = getSequenceArea()->getEditor()->getUI();

    U2Region xRegion = mcaWidget->getBaseWidthController()->getBaseScreenRange(referenceBaseIndex);
    U2Region yRegion = mcaWidget->getRowHeightController()->getScreenYRegionByMaRowIndex(rowIndex);

    QPoint topLeftPoint(xRegion.startPos, yRegion.startPos);
    QPoint bottomRight(xRegion.endPos(), yRegion.endPos());

    return QRect(topLeftPoint, bottomRight);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickToReferencePositionCenter"
void GTUtilsMcaEditorSequenceArea::clickToReferencePositionCenter(qint64 position, const QPoint& clickPointAdjustment) {
    moveCursorToReferencePositionCenter(position, clickPointAdjustment);
    GTMouseDriver::click();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "moveCursorToReferencePositionCenter"
void GTUtilsMcaEditorSequenceArea::moveCursorToReferencePositionCenter(qint64 position, const QPoint& movePointAdjustment) {
    QPoint selectedPoint(position, 2);
    auto mcaSeqArea = GTWidget::findExactWidget<McaEditorSequenceArea*>("mca_editor_sequence_area", GTUtilsMcaEditor::getActiveMcaEditorWindow());
    GT_CHECK(mcaSeqArea->isInRange(selectedPoint),
             QString("Position is out of range: [%1, %2], range: [%3, %4]")
                 .arg(selectedPoint.x())
                 .arg(selectedPoint.y())
                 .arg(mcaSeqArea->getEditor()->getAlignmentLen())
                 .arg(mcaSeqArea->getViewRowCount()));

    scrollToPosition(selectedPoint);

    McaEditorWgt* mcaWidget = mcaSeqArea->getEditor()->getUI();
    int centerX = mcaWidget->getBaseWidthController()->getBaseScreenCenter(selectedPoint.x());
    int centerY = mcaWidget->getRowHeightController()->getSingleRowHeight() / 2;
    QPoint cursorPosition(centerX + movePointAdjustment.x(), centerY + movePointAdjustment.y());
    GT_CHECK(mcaSeqArea->rect().contains(cursorPosition, false), "Position is not visible");

    auto refSeqView = GTWidget::findWidget("mca_editor_reference_area");
    GTMouseDriver::moveTo(refSeqView->mapToGlobal(cursorPosition));
    GTThread::waitForMainThread();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getCharacterModificationMode"
short GTUtilsMcaEditorSequenceArea::getCharacterModificationMode() {
    McaEditorSequenceArea* mcaSeqArea = GTUtilsMcaEditorSequenceArea::getSequenceArea();
    return (short)mcaSeqArea->getMode();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSelectedChar"
char GTUtilsMcaEditorSequenceArea::getSelectedReadChar() {
    QRect selection = GTUtilsMcaEditorSequenceArea::getSelectedRect();
    GT_CHECK_RESULT(selection.width() > 0 && selection.height() > 0, "There is no selection", U2Mca::INVALID_CHAR);
    GT_CHECK_RESULT(selection.width() <= 1 && selection.height() <= 1, "The selection is too big", U2Mca::INVALID_CHAR);
    int rowNum = selection.y();
    qint64 pos = selection.x();

    McaEditorSequenceArea* mcaSeqArea = GTUtilsMcaEditorSequenceArea::getSequenceArea();
    GT_CHECK_RESULT(mcaSeqArea != nullptr, "MCA Editor sequence area is not found", U2Mca::INVALID_CHAR);

    McaEditor* mcaEditor = mcaSeqArea->getEditor();
    GT_CHECK_RESULT(mcaSeqArea != nullptr, "MCA Editor is not found", U2Mca::INVALID_CHAR);

    MultipleChromatogramAlignmentObject* mcaObj = mcaEditor->getMaObject();
    GT_CHECK_RESULT(mcaObj != nullptr, "MCA Object is not found", U2Mca::INVALID_CHAR);

    const MultipleChromatogramAlignmentRow mcaRow = mcaObj->getRow(rowNum);

    char selectedChar = mcaRow->charAt(pos);
    return selectedChar;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getReadCharByPos"
char GTUtilsMcaEditorSequenceArea::getReadCharByPos(const QPoint p) {
    int rowNum = p.y();
    qint64 pos = p.x();

    McaEditorSequenceArea* mcaSeqArea = GTUtilsMcaEditorSequenceArea::getSequenceArea();
    GT_CHECK_RESULT(mcaSeqArea != nullptr, "MCA Editor sequence area is not found", U2Mca::INVALID_CHAR);

    McaEditor* mcaEditor = mcaSeqArea->getEditor();
    GT_CHECK_RESULT(mcaSeqArea != nullptr, "MCA Editor is not found", U2Mca::INVALID_CHAR);

    MultipleChromatogramAlignmentObject* mcaObj = mcaEditor->getMaObject();
    GT_CHECK_RESULT(mcaObj != nullptr, "MCA Object is not found", U2Mca::INVALID_CHAR);

    const MultipleChromatogramAlignmentRow mcaRow = mcaObj->getRow(rowNum);

    char selectedChar = mcaRow->charAt(pos);
    return selectedChar;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getRowLength"
qint64 GTUtilsMcaEditorSequenceArea::getRowLength(const int numRow) {
    McaEditorSequenceArea* mcaSeqArea = GTUtilsMcaEditorSequenceArea::getSequenceArea();
    GT_CHECK_RESULT(mcaSeqArea != nullptr, "MCA Editor sequence area is not found", 0);

    McaEditor* mcaEditor = mcaSeqArea->getEditor();
    GT_CHECK_RESULT(mcaSeqArea != nullptr, "MCA Editor is not found", 0);

    MultipleChromatogramAlignmentObject* mcaObj = mcaEditor->getMaObject();
    GT_CHECK_RESULT(mcaObj != nullptr, "MCA Object is not found", 0);

    const MultipleChromatogramAlignmentRow mcaRow = mcaObj->getRow(numRow);

    qint64 rowLength = mcaRow->getCoreLength();
    return rowLength;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getReferenceLength"
qint64 GTUtilsMcaEditorSequenceArea::getReferenceLength() {
    McaEditor* editor = GTUtilsMcaEditor::getEditor();
    MultipleChromatogramAlignmentObject* obj = editor->getMaObject();
    GT_CHECK_RESULT(obj != nullptr, "MultipleChromatogramAlignmentObject not found", 0);

    U2OpStatus2Log status;
    qint64 refLength = obj->getReferenceObj()->getSequenceLength();

    return refLength;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getReferenceLengthWithGaps"
qint64 GTUtilsMcaEditorSequenceArea::getReferenceLengthWithGaps() {
    McaEditor* editor = GTUtilsMcaEditor::getEditor();
    MultipleChromatogramAlignmentObject* obj = editor->getMaObject();
    GT_CHECK_RESULT(obj != nullptr, "MultipleChromatogramAlignmentObject not found", 0);

    int length = obj->getReferenceLengthWithGaps();

    return length;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getReferenceSelection"
U2Region GTUtilsMcaEditorSequenceArea::getReferenceSelection() {
    auto mcaEditArea = GTWidget::findExactWidget<McaEditorReferenceArea*>("mca_editor_reference_area");

    SequenceObjectContext* seqContext = mcaEditArea->getSequenceContext();
    GT_CHECK_RESULT(seqContext != nullptr, "SequenceObjectContext not found", U2Region());

    DNASequenceSelection* dnaSel = seqContext->getSequenceSelection();
    GT_CHECK_RESULT(dnaSel != nullptr, "DNASequenceSelection not found", U2Region());

    QVector<U2Region> region = dnaSel->getSelectedRegions();

    CHECK(region.size() != 0, U2Region());

    GT_CHECK_RESULT(region.size() == 1, "Incorrect selected region", U2Region());

    return region.first();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSelectedConsensusReg"
QString GTUtilsMcaEditorSequenceArea::getSelectedConsensusReg() {
    McaEditorConsensusArea* consArea = GTUtilsMcaEditor::getConsensusArea();
    GT_CHECK_RESULT(consArea != nullptr, "Consensus area not found", QString());

    QSharedPointer<MSAEditorConsensusCache> consCache = consArea->getConsensusCache();

    U2Region sel = GTUtilsMcaEditorSequenceArea::getReferenceSelection();
    int start = sel.startPos;
    int length = sel.length;

    QString res;
    for (int i = 0; i < length; i++) {
        int pos = start + i;
        char ch = consCache->getConsensusChar(pos);
        res.append(ch);
    }
    return res;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getConsensusStringByPos"
QString GTUtilsMcaEditorSequenceArea::getConsensusStringByRegion(const U2Region reg) {
    McaEditorConsensusArea* consArea = GTUtilsMcaEditor::getConsensusArea();
    GT_CHECK_RESULT(consArea != nullptr, "Consensus area not found", QString());

    QSharedPointer<MSAEditorConsensusCache> consCache = consArea->getConsensusCache();

    int start = reg.startPos;
    int length = reg.length;

    QString res;
    for (int i = 0; i < length; i++) {
        int pos = start + i;
        char ch = consCache->getConsensusChar(pos);
        res.append(ch);
    }
    return res;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}  // namespace U2
