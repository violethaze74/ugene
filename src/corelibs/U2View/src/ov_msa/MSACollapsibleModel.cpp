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

#include "MSACollapsibleModel.h"
#include "MSAEditor.h"

#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/U2Region.h>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// MSACollapsableItem
//////////////////////////////////////////////////////////////////////////

MSACollapsibleItem::MSACollapsibleItem()
    : msaRowIndex(-1), numRows(-1), isCollapsed(false)
{

}

MSACollapsibleItem::MSACollapsibleItem(int msaRowIndex, int numRows)
    : msaRowIndex(msaRowIndex), numRows(numRows), isCollapsed(false)
{

}

bool MSACollapsibleItem::isValid() const {
    return msaRowIndex != -1 && numRows != -1;
}

//////////////////////////////////////////////////////////////////////////
/// MSACollapsibleItemModel
//////////////////////////////////////////////////////////////////////////

MSACollapsibleItemModel::MSACollapsibleItemModel(MaEditorWgt *p)
    : QObject(p),
      ui(p),
      fakeModel(false)
{

}

void MSACollapsibleItemModel::reset(const QVector<U2Region>& collapsibleItemRegions) {
    items.clear();
    positions.clear();
    foreach(const U2Region& r, collapsibleItemRegions) {
        if (r.length < 1) {
            continue;
        }
        items.append(MSACollapsibleItem(r.startPos, r.length));
        positions.append(r.startPos);
    }
    collapseAll(true);
}

void MSACollapsibleItemModel::reset() {
    const bool modelExists = ( !items.isEmpty( ) || !positions.isEmpty( ) );
    if ( modelExists ) {
        emit si_aboutToBeToggled();
        items.clear( );
        positions.clear( );
        emit si_toggled( );
    }
}

void MSACollapsibleItemModel::collapseAll(bool collapse) {
    emit si_aboutToBeToggled();
    int delta = 0;
    for (int i=0; i < items.size(); i++) {
        MSACollapsibleItem& item = items[i];
        positions[i] = item.msaRowIndex - delta;
        item.isCollapsed = collapse;
        if (collapse) {
            delta += item.numRows - 1;
        }
    }
    emit si_toggled();
}

void MSACollapsibleItemModel::toggle(int viewRowIndex) {
    emit si_aboutToBeToggled();
    QVector<int>::ConstIterator i = qBinaryFind(positions, viewRowIndex);
    assert(i != positions.constEnd());
    int index = i - positions.constBegin();
    triggerItem(index);
    emit si_toggled();
}

void MSACollapsibleItemModel::triggerItem(int collapsibleGroupIndex) {
    MSACollapsibleItem& item = items[collapsibleGroupIndex];
    item.isCollapsed = !item.isCollapsed;
    int delta = item.numRows - 1;
    CHECK(delta != 0, );
    assert(delta > 0);
    if (item.isCollapsed) {
        delta *= -1;
    }
    for (int j = collapsibleGroupIndex + 1; j < items.size(); j++ ) {
        positions[j] += delta;
    }
}

int MSACollapsibleItemModel::mapToMsaRow(int collapsibleGroupIndex, int viewRowIndex) const {
    const MSACollapsibleItem& item = items.at(collapsibleGroupIndex);
    int row = item.msaRowIndex + viewRowIndex - positions.at(collapsibleGroupIndex);
    if (item.isCollapsed) {
        row += item.numRows - 1;
    }
    return row;
}

int MSACollapsibleItemModel::viewRowToMsaRow(int viewRowIndex) const {
    QVector<int>::ConstIterator i = qLowerBound(positions, viewRowIndex);
    int collapsibleGroupIndex = i - positions.constBegin() - 1;
    if (collapsibleGroupIndex < 0) {
        return viewRowIndex;
    } else {
        return mapToMsaRow(collapsibleGroupIndex, viewRowIndex);
    }
}

U2Region MSACollapsibleItemModel::viewRowsToMsaRows(const U2Region& viewRowsRegion) const {
    if (viewRowsRegion.isEmpty()) {
        return U2Region();
    }

    if (!ui->isCollapsibleMode()) {
        return viewRowsRegion;
    }

    int startPos = viewRowsRegion.startPos;
    int endPos = startPos + viewRowsRegion.length - 1;

    int startSeq = 0;
    int endSeq = 0;

    int startItemIdx = viewRowToGroupIndex(startPos);

    if (startItemIdx >= 0) {
        const MSACollapsibleItem& startItem = getItem(startItemIdx);
        startSeq = startItem.msaRowIndex;
    } else {
        startSeq = viewRowToMsaRow(startPos);
    }

    int endItemIdx = viewRowToGroupIndex(endPos);

    if (endItemIdx >= 0) {
        const MSACollapsibleItem& endItem = getItem(endItemIdx);
        endSeq = endItem.msaRowIndex + endItem.numRows;
    } else {
        endSeq = viewRowToMsaRow(endPos) + 1;
    }

    return U2Region(startSeq, endSeq - startSeq);
}

QList<int> MSACollapsibleItemModel::visibleViewRowsToMsaRows(const U2Region &viewRowsRegion) {
    QList<int> rowsIndexes;
    for (int i = viewRowsRegion.startPos; i < viewRowsRegion.endPos(); i++) {
        rowsIndexes << viewRowToMsaRow(i);
    }
    return rowsIndexes;
}

QList<int> MSACollapsibleItemModel::getVisibleMsaRows() const {
    QList<int> visibleMsaRowIndexes;
    for (int rowNumber = 0; rowNumber < getVisibleRowCount(); rowNumber++) {
        visibleMsaRowIndexes << viewRowToMsaRow(rowNumber);
    }
    return visibleMsaRowIndexes;
}

int MSACollapsibleItemModel::msaRowToViewRow(int msaRowIndex, bool failIfNotVisible) const {
    int invisibleRows = 0;
    for (QVector<MSACollapsibleItem>::ConstIterator it = items.constBegin(); it < items.constEnd() && it->msaRowIndex < msaRowIndex; it++) {
        if (it->isCollapsed) {
            if (it->msaRowIndex + it->numRows > msaRowIndex && failIfNotVisible) {
                return -1;
            }
            invisibleRows += (it->msaRowIndex + it->numRows <= msaRowIndex) ? it->numRows - 1 : msaRowIndex - it->msaRowIndex;
        }
    }
    return msaRowIndex - invisibleRows;
}

void MSACollapsibleItemModel::getVisibleMsaRows(int startViewRowIndex, int endViewRowIndex, QVector<U2Region>& visibleMsaRows) const {
    if (items.isEmpty()) {
        CHECK(0 <= startViewRowIndex && 0 <= endViewRowIndex && startViewRowIndex <= endViewRowIndex, );
        visibleMsaRows.append(U2Region(startViewRowIndex, endViewRowIndex - startViewRowIndex + 1));
        return;
    }
    QVector<int>::ConstIterator i = qLowerBound(positions, startViewRowIndex);
    int idx = i - positions.constBegin() - 1;
    int start = 0;
    if (idx < 0) {
        start = startViewRowIndex;
    } else {
        start = mapToMsaRow(idx, startViewRowIndex);
    }

    int j = i - positions.constBegin();
    for (; j < items.size(); j++) {
        const MSACollapsibleItem& item = items.at(j);
        if (positions[j] > endViewRowIndex)
            break;
        if (item.isCollapsed) {
            visibleMsaRows.append(U2Region(start, item.msaRowIndex - start + 1));
            start = item.msaRowIndex + item.numRows;
        }
    }

    int lastRow = 0;
    if (j - 1 < 0) {
        lastRow = endViewRowIndex;
    } else {
        lastRow = mapToMsaRow(j - 1, endViewRowIndex);
    }

    MaEditor* ed = ui->getEditor();
    MultipleAlignmentObject* obj = ed->getMaObject();
    int alnNumRows = obj->getNumRows();
    lastRow = qMin(lastRow, alnNumRows - 1);
    int len = lastRow - start + 1;
    if (len>0) {
        visibleMsaRows.append(U2Region(start, len));
    }
}


bool MSACollapsibleItemModel::isFirstRowOfCollapsibleGroup(int viewRowIndex) const {
    QVector<int>::ConstIterator i = qBinaryFind(positions, viewRowIndex);
    return i != positions.constEnd();
}

bool MSACollapsibleItemModel::isInCollapsibleGroup(int viewRowIndex) const {
    return viewRowToGroupIndex(viewRowIndex) >= 0;
}

bool MSACollapsibleItemModel::isGroupWithMsaRowCollapsed(int msaRowIndex) const {
    const MSACollapsibleItem item = ui->getCollapseModel()->getItemByMsaRowIndex(msaRowIndex);
    return item.isValid() && item.isCollapsed;
}

bool MSACollapsibleItemModel::isMsaRowVisible(int msaRowIndex) const {
    int viewRowIndex = msaRowToViewRow(msaRowIndex, true);
    return isFirstRowOfCollapsibleGroup(viewRowIndex) || !isGroupWithMsaRowCollapsed(msaRowIndex);
}

int MSACollapsibleItemModel::viewRowToGroupIndex(int viewRowIndex) const {
    QVector<int>::ConstIterator i = qLowerBound(positions, viewRowIndex);

    if (i < positions.constEnd() && *i == viewRowIndex) {
        return i - positions.constBegin();
    }

    int closestItem = i - positions.constBegin() - 1;
    if (closestItem < 0) {
        return -1;
    }

    const MSACollapsibleItem& item = items.at(closestItem);
    if (item.isCollapsed) {
        return -1;
    } else {
        int itBottom = positions.at(closestItem) + item.numRows - 1;
        if (viewRowIndex <= itBottom) {
            return closestItem;
        }
        return -1;
    }
}

MSACollapsibleItem MSACollapsibleItemModel::getItem(int collapsibleItemIndex) const {
    return items.at(collapsibleItemIndex);
}

MSACollapsibleItem MSACollapsibleItemModel::getItemByMsaRowIndex(int msaRowIndex) const {
    int viewRowIndex = msaRowToViewRow(msaRowIndex);
    const int collapsibleItemIndex = viewRowToGroupIndex(viewRowIndex);
    CHECK(0 <= collapsibleItemIndex && collapsibleItemIndex < items.size(), MSACollapsibleItem());
    return items[collapsibleItemIndex];
}

int MSACollapsibleItemModel::getVisibleRowCount() const {
    MaEditor *ed = ui->getEditor();
    MultipleAlignmentObject *o = ed->getMaObject();
    int size = o->getNumRows();
    foreach (const MSACollapsibleItem &item, items) {
        if (item.isCollapsed) {
            size -= item.numRows - 1;
        }
    }
    return size;
}

bool MSACollapsibleItemModel::isEmpty() const {
    return items.isEmpty();
}

void MSACollapsibleItemModel::setFakeCollapsibleModel(bool fakeModelStatus) {
    fakeModel = fakeModelStatus;
}

bool MSACollapsibleItemModel::isFakeModel() const {
    return fakeModel;
}

int MSACollapsibleItemModel::getCollapsibleItemCount() const {
    return items.size();
}

} // namespace U2
