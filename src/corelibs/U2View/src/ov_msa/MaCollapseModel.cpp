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

#include "MaCollapseModel.h"
#include "MSAEditor.h"

#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/U2Region.h>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// MSACollapsableItem
//////////////////////////////////////////////////////////////////////////

MaCollapsibleGroup::MaCollapsibleGroup()
    : maRowIndex(-1), numRows(-1), isCollapsed(false)
{

}

MaCollapsibleGroup::MaCollapsibleGroup(int maRowIndex, int numRows)
    : maRowIndex(maRowIndex), numRows(numRows), isCollapsed(false)
{

}

bool MaCollapsibleGroup::isValid() const {
    return maRowIndex != -1 && numRows != -1;
}

//////////////////////////////////////////////////////////////////////////
/// MSACollapsibleItemModel
//////////////////////////////////////////////////////////////////////////

MaCollapseModel::MaCollapseModel(MaEditorWgt *p)
    : QObject(p),
      ui(p),
      fakeModel(false)
{

}

void MaCollapseModel::update(const QVector<U2Region>& collapsibleGroupRegions) {
    groups.clear();
    positions.clear();
    foreach(const U2Region& r, collapsibleGroupRegions) {
        if (r.length < 1) {
            continue;
        }
        groups.append(MaCollapsibleGroup(r.startPos, r.length));
        positions.append(r.startPos);
    }
    collapseAll(true);
}

void MaCollapseModel::clear() {
    const bool modelExists = (!groups.isEmpty( ) || !positions.isEmpty( ) );
    if ( modelExists ) {
        emit si_aboutToBeToggled();
        groups.clear( );
        positions.clear( );
        emit si_toggled( );
    }
}

void MaCollapseModel::collapseAll(bool collapse) {
    emit si_aboutToBeToggled();
    int delta = 0;
    for (int i=0; i < groups.size(); i++) {
        MaCollapsibleGroup& item = groups[i];
        positions[i] = item.maRowIndex - delta;
        item.isCollapsed = collapse;
        if (collapse) {
            delta += item.numRows - 1;
        }
    }
    emit si_toggled();
}

void MaCollapseModel::toggle(int viewRowIndex) {
    emit si_aboutToBeToggled();
    QVector<int>::ConstIterator i = qBinaryFind(positions, viewRowIndex);
    assert(i != positions.constEnd());
    int index = i - positions.constBegin();
    triggerItem(index);
    emit si_toggled();
}

void MaCollapseModel::triggerItem(int collapsibleGroupIndex) {
    MaCollapsibleGroup& item = groups[collapsibleGroupIndex];
    item.isCollapsed = !item.isCollapsed;
    int delta = item.numRows - 1;
    CHECK(delta != 0, );
    assert(delta > 0);
    if (item.isCollapsed) {
        delta *= -1;
    }
    for (int j = collapsibleGroupIndex + 1; j < groups.size(); j++ ) {
        positions[j] += delta;
    }
}

int MaCollapseModel::mapToMaRow(int collapsibleGroupIndex, int viewRowIndex) const {
    const MaCollapsibleGroup& item = groups.at(collapsibleGroupIndex);
    int row = item.maRowIndex + viewRowIndex - positions.at(collapsibleGroupIndex);
    if (item.isCollapsed) {
        row += item.numRows - 1;
    }
    return row;
}

int MaCollapseModel::viewRowToMaRow(int viewRowIndex) const {
    QVector<int>::ConstIterator i = qLowerBound(positions, viewRowIndex);
    int collapsibleGroupIndex = i - positions.constBegin() - 1;
    if (collapsibleGroupIndex < 0) {
        return viewRowIndex;
    } else {
        return mapToMaRow(collapsibleGroupIndex, viewRowIndex);
    }
}

U2Region MaCollapseModel::viewRowsToMaRows(const U2Region& viewRowsRegion) const {
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
        const MaCollapsibleGroup& startItem = getCollapsibleGroup(startItemIdx);
        startSeq = startItem.maRowIndex;
    } else {
        startSeq = viewRowToMaRow(startPos);
    }

    int endItemIdx = viewRowToGroupIndex(endPos);

    if (endItemIdx >= 0) {
        const MaCollapsibleGroup& endItem = getCollapsibleGroup(endItemIdx);
        endSeq = endItem.maRowIndex + endItem.numRows;
    } else {
        endSeq = viewRowToMaRow(endPos) + 1;
    }

    return U2Region(startSeq, endSeq - startSeq);
}

QList<int> MaCollapseModel::visibleViewRowsToMaRows(const U2Region &viewRowsRegion) {
    QList<int> msaRowIndexes;
    for (int i = viewRowsRegion.startPos; i < viewRowsRegion.endPos(); i++) {
        msaRowIndexes << viewRowToMaRow(i);
    }
    return msaRowIndexes;
}

QList<int> MaCollapseModel::getVisibleMaRows() const {
    QList<int> visibleMsaRowIndexes;
    for (int rowNumber = 0; rowNumber < getVisibleRowCount(); rowNumber++) {
        visibleMsaRowIndexes << viewRowToMaRow(rowNumber);
    }
    return visibleMsaRowIndexes;
}

int MaCollapseModel::maRowToViewRow(int maRowIndex, bool failIfNotVisible) const {
    int invisibleRows = 0;
    for (QVector<MaCollapsibleGroup>::ConstIterator it = groups.constBegin(); it < groups.constEnd() && it->maRowIndex < maRowIndex; it++) {
        if (it->isCollapsed) {
            if (it->maRowIndex + it->numRows > maRowIndex && failIfNotVisible) {
                return -1;
            }
            invisibleRows += (it->maRowIndex + it->numRows <= maRowIndex) ? it->numRows - 1 : maRowIndex - it->maRowIndex;
        }
    }
    return maRowIndex - invisibleRows;
}

QList<U2Region> MaCollapseModel::getVisibleMaRows(int startViewRowIndex, int endViewRowIndex) const {
    QList<U2Region> visibleMsaRows;
    if (groups.isEmpty()) {
        CHECK(0 <= startViewRowIndex && 0 <= endViewRowIndex && startViewRowIndex <= endViewRowIndex, visibleMsaRows);
        visibleMsaRows.append(U2Region(startViewRowIndex, endViewRowIndex - startViewRowIndex + 1));
        return visibleMsaRows;
    }
    QVector<int>::ConstIterator i = qLowerBound(positions, startViewRowIndex);
    int idx = i - positions.constBegin() - 1;
    int start = 0;
    if (idx < 0) {
        start = startViewRowIndex;
    } else {
        start = mapToMaRow(idx, startViewRowIndex);
    }

    int j = i - positions.constBegin();
    for (; j < groups.size(); j++) {
        const MaCollapsibleGroup& item = groups.at(j);
        if (positions[j] > endViewRowIndex)
            break;
        if (item.isCollapsed) {
            visibleMsaRows.append(U2Region(start, item.maRowIndex - start + 1));
            start = item.maRowIndex + item.numRows;
        }
    }

    int lastRow = 0;
    if (j - 1 < 0) {
        lastRow = endViewRowIndex;
    } else {
        lastRow = mapToMaRow(j - 1, endViewRowIndex);
    }

    MaEditor* ed = ui->getEditor();
    MultipleAlignmentObject* obj = ed->getMaObject();
    int alnNumRows = obj->getNumRows();
    lastRow = qMin(lastRow, alnNumRows - 1);
    int len = lastRow - start + 1;
    if (len>0) {
        visibleMsaRows.append(U2Region(start, len));
    }
    return visibleMsaRows;
}


bool MaCollapseModel::isFirstRowOfCollapsibleGroup(int viewRowIndex) const {
    QVector<int>::ConstIterator i = qBinaryFind(positions, viewRowIndex);
    return i != positions.constEnd();
}

bool MaCollapseModel::isInCollapsibleGroup(int viewRowIndex) const {
    return viewRowToGroupIndex(viewRowIndex) >= 0;
}

bool MaCollapseModel::isGroupWithMaRowCollapsed(int maRowIndex) const {
    const MaCollapsibleGroup item = ui->getCollapseModel()->getItemByMaRowIndex(maRowIndex);
    return item.isValid() && item.isCollapsed;
}

bool MaCollapseModel::isMaRowVisible(int maRowIndex) const {
    int viewRowIndex = maRowToViewRow(maRowIndex, true);
    return isFirstRowOfCollapsibleGroup(viewRowIndex) || !isGroupWithMaRowCollapsed(maRowIndex);
}

int MaCollapseModel::viewRowToGroupIndex(int viewRowIndex) const {
    QVector<int>::ConstIterator i = qLowerBound(positions, viewRowIndex);

    if (i < positions.constEnd() && *i == viewRowIndex) {
        return i - positions.constBegin();
    }

    int closestItem = i - positions.constBegin() - 1;
    if (closestItem < 0) {
        return -1;
    }

    const MaCollapsibleGroup& item = groups.at(closestItem);
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

MaCollapsibleGroup MaCollapseModel::getCollapsibleGroup(int collapsibleGroupIndex) const {
    return groups.at(collapsibleGroupIndex);
}

MaCollapsibleGroup MaCollapseModel::getItemByMaRowIndex(int maRowIndex) const {
    int viewRowIndex = maRowToViewRow(maRowIndex);
    const int collapsibleItemIndex = viewRowToGroupIndex(viewRowIndex);
    CHECK(0 <= collapsibleItemIndex && collapsibleItemIndex < groups.size(), MaCollapsibleGroup());
    return groups[collapsibleItemIndex];
}

int MaCollapseModel::getVisibleRowCount() const {
    MaEditor *ed = ui->getEditor();
    MultipleAlignmentObject *o = ed->getMaObject();
    int size = o->getNumRows();
    foreach (const MaCollapsibleGroup &item, groups) {
        if (item.isCollapsed) {
            size -= item.numRows - 1;
        }
    }
    return size;
}

bool MaCollapseModel::isEmpty() const {
    return groups.isEmpty();
}

void MaCollapseModel::setFakeCollapsibleModel(bool fakeModelStatus) {
    fakeModel = fakeModelStatus;
}

bool MaCollapseModel::isFakeModel() const {
    return fakeModel;
}

int MaCollapseModel::getCollapsibleGroupCount() const {
    return groups.size();
}

} // namespace U2
