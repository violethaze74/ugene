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

bool MaCollapsibleGroup::operator==(const MaCollapsibleGroup& other) const {
    return maRowIndex == other.maRowIndex && numRows == other.numRows && isCollapsed == other.isCollapsed;
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
    QVector<MaCollapsibleGroup> newGroups;
    QVector<int> newPositions;
    for (int i = 0; i < collapsibleGroupRegions.length(); i++ ) {
        const U2Region& r = collapsibleGroupRegions[i];
        if (r.length < 1) {
            continue;
        }
        MaCollapsibleGroup newGroup(r.startPos, r.length);
        if (i < groups.length()) {
            newGroup.isCollapsed = groups[i].isCollapsed;
        }
        newGroups.append(newGroup);
        newPositions.append(r.startPos);
    }
    if (newGroups == groups && newPositions == positions) {
        return; // nothing is changed.
    }
    groups = newGroups;
    positions = newPositions;
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

int MaCollapseModel::getMaRowIndexByViewRowIndex(int viewRowIndex) const {
    QVector<int>::ConstIterator i = qLowerBound(positions, viewRowIndex);
    int collapsibleGroupIndex = i - positions.constBegin() - 1;
    if (collapsibleGroupIndex < 0) {
        return viewRowIndex;
    } else {
        return mapToMaRow(collapsibleGroupIndex, viewRowIndex);
    }
}

U2Region MaCollapseModel::getMaRowIndexRegionByViewRowIndexRegion(const U2Region& viewRowIndexRegion) const {
    if (viewRowIndexRegion.isEmpty()) {
        return U2Region();
    }

    if (!ui->isCollapsibleMode()) {
        return viewRowIndexRegion;
    }

    int viewStartIndex = viewRowIndexRegion.startPos;
    int viewEndIndex = viewStartIndex + (int) viewRowIndexRegion.length - 1;

    int msaStartIndex = getMaRowIndexByViewRowIndex(viewStartIndex);
    int msaEndIndex = getMaRowIndexByViewRowIndex(viewEndIndex) + 1;

    // If the end position is a header of the collapsing group then use the whole group.
    int endGroupIndex = getCollapsibleGroupIndexByViewRowIndex(viewEndIndex);
    if (endGroupIndex >= 0) {
        const MaCollapsibleGroup& group = getCollapsibleGroup(endGroupIndex);
        msaEndIndex = group.maRowIndex + group.numRows;
    }
    return U2Region(msaStartIndex, msaEndIndex - msaStartIndex);
}

QList<int> MaCollapseModel::getMaRowIndexesByViewRowIndexes(const U2Region &viewRowIndexesRegion) {
    QList<int> msaRowIndexes;
    for (int i = viewRowIndexesRegion.startPos; i < viewRowIndexesRegion.endPos(); i++) {
        msaRowIndexes << getMaRowIndexByViewRowIndex(i);
    }
    return msaRowIndexes;
}

QList<int> MaCollapseModel::getMaRowsIndexesWithViewRowIndexes() const {
    QList<int> visibleMsaRowIndexes;
    for (int viewRowIndex = 0; viewRowIndex < getViewRowCount(); viewRowIndex++) {
        visibleMsaRowIndexes << getMaRowIndexByViewRowIndex(viewRowIndex);
    }
    return visibleMsaRowIndexes;
}

int MaCollapseModel::getViewRowIndexByMaRowIndex(int maRowIndex, bool failIfNotVisible) const {
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

QList<U2Region> MaCollapseModel::getGroupedMaRowIndexesWithViewRowIndexes(int startViewRowIndex, int endViewRowIndex) const {
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
    return getCollapsibleGroupIndexByViewRowIndex(viewRowIndex) >= 0;
}

bool MaCollapseModel::isGroupWithMaRowIndexCollapsed(int maRowIndex) const {
    const MaCollapsibleGroup item = ui->getCollapseModel()->findCollapsibleGroupByMaRowIndex(maRowIndex);
    return item.isValid() && item.isCollapsed;
}

bool MaCollapseModel::isMaRowHasViewRowIndex(int maRowIndex) const {
    int viewRowIndex = getViewRowIndexByMaRowIndex(maRowIndex, true);
    return isFirstRowOfCollapsibleGroup(viewRowIndex) || !isGroupWithMaRowIndexCollapsed(maRowIndex);
}

int MaCollapseModel::getCollapsibleGroupIndexByViewRowIndex(int viewRowIndex) const {
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

MaCollapsibleGroup MaCollapseModel::findCollapsibleGroupByMaRowIndex(int maRowIndex) const {
    int viewRowIndex = getViewRowIndexByMaRowIndex(maRowIndex);
    const int collapsibleItemIndex = getCollapsibleGroupIndexByViewRowIndex(viewRowIndex);
    CHECK(0 <= collapsibleItemIndex && collapsibleItemIndex < groups.size(), MaCollapsibleGroup());
    return groups[collapsibleItemIndex];
}

int MaCollapseModel::getViewRowCount() const {
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
