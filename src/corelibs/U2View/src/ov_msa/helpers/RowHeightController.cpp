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

#include "RowHeightController.h"
#include "ScrollController.h"
#include "ov_msa/MaEditor.h"
#include "ov_msa/MaCollapseModel.h"
#include "ov_msa/view_rendering/MaEditorWgt.h"

namespace U2 {

RowHeightController::RowHeightController(MaEditorWgt *maEditorWgt)
    : QObject(maEditorWgt),
      ui(maEditorWgt)
{

}

int RowHeightController::getGlobalYPositionByMaRowIndex(int maRowIndex) const {
    int offset = 0;
    for (int i = 0; i < maRowIndex; i++) {
        offset += getRowHeightByMaIndex(i);
    }
    return offset;
}

int RowHeightController::getGlobalYPositionByMaRowIndex(int maRowIndex, const QList<int> &maRowIndexes) const {
    int offset = 0;
    foreach (int currentIndex, maRowIndexes) {
        if (currentIndex == maRowIndex) {
            return offset;
        }
        offset += getRowHeightByMaIndex(currentIndex);
    }
    FAIL(false, 0);
}

int RowHeightController::getGlobalYPositionOfTheFirstVisibleRow(bool countClipped) const {
    return getGlobalYPositionByMaRowIndex(ui->getScrollController()->getFirstVisibleRowIndex(countClipped));
}

int RowHeightController::getScreenYPositionOfTheFirstVisibleRow(bool countClipped) const {
    const int globalYPositionOfTheFirstVisibleRow = getGlobalYPositionOfTheFirstVisibleRow(countClipped);
    return globalYPositionOfTheFirstVisibleRow - ui->getScrollController()->getScreenPosition().y();
}

int RowHeightController::getRowHeightByViewRowIndex(int viewRowIndex) const {
    int maRowIndex = ui->getCollapseModel()->getMaRowIndexByViewRowIndex(viewRowIndex);
    return getRowHeightByMaIndex(maRowIndex);
}

int RowHeightController::getSumOfRowHeightsByMaIndexes(const QList<int> &maRowIndexes) const {
    int sumHeight = 0;
    foreach (int maRowIndex, maRowIndexes) {
        sumHeight += getRowHeightByMaIndex(maRowIndex);
    }
    return sumHeight;
}

int RowHeightController::getTotalAlignmentHeight() const {
    U2Region globalYRegion = getGlobalYRegionByViewRowIndexRegion(0, ui->getCollapseModel()->getViewRowCount());
    return static_cast<int>(globalYRegion.length);
}

int RowHeightController::getSingleRowHeight() const {
    const int fontHeight = QFontMetrics(ui->getEditor()->getFont(), ui).height();
    const float zoomMult = ui->getEditor()->zoomMult;
    return qRound(fontHeight * zoomMult);
}

int RowHeightController::getMaRowIndexByGlobalYPosition(int y) const {
    int viewRowIndex = getViewRowIndexByGlobalYPosition(y);
    return ui->getCollapseModel()->getMaRowIndexByViewRowIndex(viewRowIndex);
}

int RowHeightController::getViewRowIndexByGlobalYPosition(int y) const {
    const int viewRowCount = ui->getCollapseModel()->getViewRowCount();
    int accumulatedHeight = 0;
    for (int viewRowIndex = 0; viewRowIndex < viewRowCount; viewRowIndex++) {
        const int rowHeight = getRowHeightByViewRowIndex(viewRowIndex);
        if (accumulatedHeight + rowHeight <= y) {
            accumulatedHeight += rowHeight;
        } else {
            return viewRowIndex;
        }
    }
    return -1;
}

int RowHeightController::getMaRowIndexByScreenYPosition(int y) const {
    return getMaRowIndexByGlobalYPosition(y + ui->getScrollController()->getScreenPosition().y());
}

int RowHeightController::getViewRowIndexByScreenYPosition(int y) const {
    return getViewRowIndexByGlobalYPosition(y + ui->getScrollController()->getScreenPosition().y());
}

U2Region RowHeightController::getGlobalYRegionByMaRowIndex(int maRowIndex) const {
    int globalYPosition = getGlobalYPositionByMaRowIndex(maRowIndex);
    int rowHeight = getRowHeightByMaIndex(maRowIndex);
    return U2Region(globalYPosition, rowHeight);
}

U2Region RowHeightController::getGlobalYRegionByMaRowIndex(int maRowIndex, const QList<int> &maRowIndexes) const {
    int globalYPosition = getGlobalYPositionByMaRowIndex(maRowIndex, maRowIndexes);
    int rowHeight = getRowHeightByMaIndex(maRowIndex);
    return U2Region(globalYPosition, rowHeight);
}

U2Region RowHeightController::getGlobalYRegionByViewRowIndex(int viewRowIndex) const {
    int maRowIndex = ui->getCollapseModel()->getMaRowIndexByViewRowIndex(viewRowIndex);
    return getGlobalYRegionByMaRowIndex(maRowIndex);
}

U2Region RowHeightController::getGlobalYRegionByViewRowIndexRegion(int startViewRowIndex, int count) const {
    QList<int> rowIndexes;
    for (int i = startViewRowIndex; i < startViewRowIndex + count; i++) {
        rowIndexes << ui->getCollapseModel()->getMaRowIndexByViewRowIndex(i);
    }
    return getGlobalYRegionByMaRowIndexes(rowIndexes);
}

U2Region RowHeightController::getGlobalYRegionByMaRowIndexes(const QList<int> &maRowIndexes) const {
    CHECK(!maRowIndexes.isEmpty(), U2Region());
    int height = 0;
    foreach (const int rowIndex, maRowIndexes) {
        height += getRowHeightByMaIndex(rowIndex);
    }
    U2Region globalYRegion = getGlobalYRegionByMaRowIndex(maRowIndexes.first());
    return U2Region(globalYRegion.startPos, height);
}

U2Region RowHeightController::getScreenYRegionByMaRowIndex(int maRowIndex) const {
    return getScreenYRegionByMaRowIndex(maRowIndex, ui->getScrollController()->getScreenPosition().y());
}

U2Region RowHeightController::getScreenYRegionByMaRowIndex(int maRowIndex, const QList<int> &maRowIndexes, int screenYOrigin) const {
    const U2Region rowGlobalRange = getGlobalYRegionByMaRowIndex(maRowIndex, maRowIndexes);
    return U2Region(rowGlobalRange.startPos - screenYOrigin, rowGlobalRange.length);
}

U2Region RowHeightController::getScreenYRegionByMaRowIndex(int maRowIndex, int screenYOrigin) const {
    const U2Region rowRange = getGlobalYRegionByMaRowIndex(maRowIndex);
    return U2Region(rowRange.startPos - screenYOrigin, rowRange.length);
}

U2Region RowHeightController::getScreenYRegionByViewRowIndex(int viewRowIndex) const {
    const int screenYOrigin = ui->getScrollController()->getScreenPosition().y();
    int maRowIndex = ui->getCollapseModel()->getMaRowIndexByViewRowIndex(viewRowIndex);
    return getScreenYRegionByMaRowIndex(maRowIndex, screenYOrigin);
}

U2Region RowHeightController::getScreenYRegionByViewRowIndex(int viewRowIndex, int screenYOrigin) const {
    int maRowIndex = ui->getCollapseModel()->getMaRowIndexByViewRowIndex(viewRowIndex);
    return getScreenYRegionByMaRowIndex(maRowIndex, screenYOrigin);
}

U2Region RowHeightController::getScreenYRegionByViewRowIndexes(const U2Region &viewRowIndexes) const {
    const QList<int> rowsIndexes = ui->getCollapseModel()->getMaRowIndexesByViewRowIndexes(viewRowIndexes);
    const U2Region rowsGlobalRange = getGlobalYRegionByMaRowIndexes(rowsIndexes);
    return U2Region(rowsGlobalRange.startPos - ui->getScrollController()->getScreenPosition().y(), rowsGlobalRange.length);
}

}   // namespace U2
