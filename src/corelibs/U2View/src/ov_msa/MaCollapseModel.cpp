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
#include <U2Core/U2Region.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// MaCollapsibleGroup
//////////////////////////////////////////////////////////////////////////

MaCollapsibleGroup::MaCollapsibleGroup(const QList<int>& maRows, bool isCollapsed)
        : maRows(maRows), isCollapsed(isCollapsed) {
}

bool MaCollapsibleGroup::operator==(const MaCollapsibleGroup& other) const {
    return maRows == other.maRows && isCollapsed == other.isCollapsed;
}

//////////////////////////////////////////////////////////////////////////
/// MaCollapseModel
//////////////////////////////////////////////////////////////////////////

MaCollapseModel::MaCollapseModel(QObject* p, int numSequences)
        : QObject(p), fakeModel(false), hasGroupsWithMultipleItems(false) {
    reset(numSequences);
}

void MaCollapseModel::update(const QVector<MaCollapsibleGroup>& newGroups) {
    if (newGroups == groups) {
        return; // nothing is changed.
    }
    emit si_aboutToBeToggled();
    groups = newGroups;
    updateIndex();
    emit si_toggled( );
}

void MaCollapseModel::updateFromUnitedRows(const QVector<U2Region>& unitedRows, int numSequences) {
    QVector<U2Region> sortedRegions = unitedRows;
    qSort(sortedRegions);
    QVector<MaCollapsibleGroup> newGroups;
    int maIndex = 0;
    foreach (const U2Region region, unitedRows) {
        for (; maIndex < region.startPos; maIndex++) {
            newGroups.append(MaCollapsibleGroup(QList<int>()<<maIndex, true));
        }
        QList<int> maRows;
        for (;maIndex < region.endPos(); maIndex++) {
            maRows.append(maIndex);
        }
        newGroups.append(MaCollapsibleGroup(maRows, true));
    }
    for (; maIndex < numSequences; maIndex++) {
        newGroups.append(MaCollapsibleGroup(QList<int>()<<maIndex, true));
    }
    // Copy collapse info from the current state.
    for (int i = 0, n = qMin(newGroups.size(), groups.size()); i < n; i++) {
        newGroups[i].isCollapsed = groups[i].isCollapsed;
    }
    update(newGroups);
}

void MaCollapseModel::reset(int numSequences) {
    QVector<MaCollapsibleGroup> newGroups;
    for (int maRow = 0; maRow < numSequences; maRow++) {
        newGroups.append(MaCollapsibleGroup(QList<int>() << maRow, true));
    }
    update(newGroups);
}

void MaCollapseModel::collapseAll(bool collapse) {
    emit si_aboutToBeToggled();
    for (int i=0; i < groups.size(); i++) {
        groups[i].isCollapsed = collapse;
    }
    updateIndex();
    emit si_toggled();
}

void MaCollapseModel::toggle(int viewRowIndex) {
    int groupIndex = getCollapsibleGroupIndexByViewRowIndex(viewRowIndex);
    CHECK(groupIndex >= 0 && groupIndex <= groups.size(),)
    MaCollapsibleGroup& group = groups[groupIndex];
    toggleGroup(groupIndex, !group.isCollapsed);
}

void MaCollapseModel::toggle(int viewRowIndex, bool isCollapsed) {
    int groupIndex = getCollapsibleGroupIndexByViewRowIndex(viewRowIndex);
    toggleGroup(groupIndex, isCollapsed);
}

void MaCollapseModel::toggleGroup(int groupIndex, bool isCollapsed) {
    CHECK(groupIndex >= 0 && groupIndex <= groups.size(),)
    MaCollapsibleGroup& group = groups[groupIndex];
    if (group.isCollapsed == isCollapsed) {
        return;
    }
    emit si_aboutToBeToggled();
    group.isCollapsed = isCollapsed;
    updateIndex();
    emit si_toggled();
}

int MaCollapseModel::getMaRowIndexByViewRowIndex(int viewRowIndex) const {
    return maRowByViewRow.value(viewRowIndex, -1);
}

U2Region MaCollapseModel::getMaRowIndexRegionByViewRowIndexRegion(const U2Region& viewRowIndexRegion) const {
    if (viewRowIndexRegion.isEmpty()) {
        return U2Region();
    }
    if (!hasGroupsWithMultipleItems) {
        return viewRowIndexRegion;
    }
    int minMaRowIndex = INT_MAX;
    int maxMaRowIndex = 0;
    for (int viewRowIndex = viewRowIndexRegion.startPos; viewRowIndex < viewRowIndexRegion.endPos(); viewRowIndex++) {
        int maRowIndex = maRowByViewRow.value(viewRowIndex, -1);
        if (maRowIndex == -1) {
            continue;
        }
        minMaRowIndex = qMin(minMaRowIndex, maRowIndex);
        maxMaRowIndex = qMax(maxMaRowIndex, maRowIndex);
    }
    return U2Region(minMaRowIndex, maxMaRowIndex - minMaRowIndex + 1);
}

QList<int> MaCollapseModel::getMaRowIndexesByViewRowIndexes(const U2Region &viewRowIndexesRegion) {
    QList<int> maRows;
    for (int viewRow = viewRowIndexesRegion.startPos, n = viewRowIndexesRegion.endPos(); viewRow < n; viewRow++) {
        int maRow = getMaRowIndexByViewRowIndex(viewRow);
        if (maRow >= 0) {
            maRows << maRow;
        }
    }
    return maRows;
}

QList<int> MaCollapseModel::getMaRowsIndexesWithViewRowIndexes() const {
    QList<int> maRows;
    for (int viewRow = 0, n = getViewRowCount(); viewRow < n ; viewRow++) {
        int maRow = getMaRowIndexByViewRowIndex(viewRow);
        if (maRow >= 0) {
            maRows << maRow;
        }
    }
    return maRows;
}

int MaCollapseModel::getViewRowIndexByMaRowIndex(int maRowIndex, bool failIfNotVisible) const {
    int viewRowIndex = viewRowByMaRow.value(maRowIndex, -1);
    if (viewRowIndex >= 0) {
        return viewRowIndex;
    }
    if (failIfNotVisible) {
        return -1;
    }
    int groupIndex = groupByMaRow.value(maRowIndex, -1);
    if (groupIndex == -1) {
        return -1;
    }
    const MaCollapsibleGroup& group = groups[groupIndex];
    int firstMaInGroup = group.maRows[0];
    return viewRowByMaRow.value(firstMaInGroup, -1);
}

bool MaCollapseModel::isGroupWithMaRowIndexCollapsed(int maRowIndex) const {
    int viewRowIndex = getViewRowIndexByMaRowIndex(maRowIndex);
    int groupIndex = getCollapsibleGroupIndexByViewRowIndex(viewRowIndex);
    const MaCollapsibleGroup* group = getCollapsibleGroup(groupIndex);
    return group != NULL && group->isCollapsed;
}

int MaCollapseModel::getCollapsibleGroupIndexByViewRowIndex(int viewRowIndex) const {
    int maIndex = maRowByViewRow.value(viewRowIndex, -1);
    return groupByMaRow.value(maIndex, -1);
}

const MaCollapsibleGroup* MaCollapseModel::getCollapsibleGroup(int collapsibleGroupIndex) const {
    if (collapsibleGroupIndex < 0 || collapsibleGroupIndex >= groups.length()) {
        return NULL;
    }
    return &groups.constData()[collapsibleGroupIndex];
}

int MaCollapseModel::getViewRowCount() const {
    return viewRowByMaRow.size();
}

void MaCollapseModel::setFakeCollapsibleModel(bool fakeModelStatus) {
    fakeModel = fakeModelStatus;
}

void MaCollapseModel::updateIndex() {
    viewRowByMaRow.clear();
    maRowByViewRow.clear();
    groupByMaRow.clear();
    hasGroupsWithMultipleItems = false;
    int viewRow = 0;
    for (int groupIndex = 0; groupIndex < groups.size(); groupIndex++) {
        const MaCollapsibleGroup& group = groups[groupIndex];
        hasGroupsWithMultipleItems = hasGroupsWithMultipleItems || group.maRows.size() > 1;
        for (int i = 0; i < group.maRows.size(); i++) {
            int maRow = group.maRows[i];
            if (i == 0 || !group.isCollapsed) {
                viewRowByMaRow.insert(maRow, viewRow);
                maRowByViewRow.insert(viewRow, maRow);
                viewRow++;
            }
            groupByMaRow.insert(maRow, groupIndex);
        }
    }    
}

} // namespace U2
