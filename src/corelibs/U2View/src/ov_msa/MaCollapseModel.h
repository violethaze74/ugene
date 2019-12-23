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

#ifndef _U2_MA_COLLAPSE_MODEL_H_
#define _U2_MA_COLLAPSE_MODEL_H_

#include <QObject>
#include <QVector>
#include <QList>
#include <QHash>

#include <U2Core/global.h>

namespace U2 {

class MaCollapsibleGroup {
public:
    /* Creates new collapsible group item that starts with maRowIndex and has numRows inside. */
    MaCollapsibleGroup(const QList<int>& maRows = QList<int>() , bool isCollapsed = false);

    /* Compares this group with another group. Returns true if groups are equal. */
    bool operator== (const MaCollapsibleGroup &other) const;

    /* Ordered set of ma rows in the group. */
    QList<int> maRows;

    /* If group is collapsed or not. */
    bool isCollapsed;
};

class MaEditorWgt;
class MaModificationInfo;
class U2Region;

class U2VIEW_EXPORT MaCollapseModel : public QObject {
    Q_OBJECT
public:
    MaCollapseModel(QObject *p, int numSequences);

    /* Updates model to the given groups. */
    void update(const QVector<MaCollapsibleGroup>& groups);

    /** Updates collapse model using united rows as input. */
    void updateFromUnitedRows(const QVector<U2Region>& unitedRows, int numSequences);

    /* Flattens all collapsible groups: makes every group contain only 1 sequence */
    void reset(int numSequences);

    /* Toggle 'isCollapsed' state for the group at the given row. */
    void toggle(int viewRowIndex);

    /* Updates 'isCollapsed' state for the group at the given row. */
    void toggle(int viewRowIndex, bool isCollapsed);

    /* Toggle 'isCollapsed' state for the group. */
    void toggleGroup(int collapsibleGroupIndex, bool isCollapsed);

    /* Collapse all groups in the view. */
    void collapseAll(bool collapse);

    /* Converts view row index to MA row index. */
    int getMaRowIndexByViewRowIndex(int viewRowIndex) const;

    /* Converts view rows region to MA rows region. */
    U2Region getMaRowIndexRegionByViewRowIndexRegion(const U2Region &viewRowIndexRegion) const;

    /* Returns list of MA row indexes with view row indexes. Invisible (MA rows with no view index) rows are not included to the result. */
    QList<int> getMaRowIndexesByViewRowIndexes(const U2Region& viewRowIndexesRegion);

    /* Returns list of all MA row indexes that have valid view row index (not hidden by collapsing). */
    QList<int> getMaRowsIndexesWithViewRowIndexes() const;

    /*
    * Converts MA row index to the view row index.
    * If MA row has no viewRowIndex (is inside of collapsed group) returns -1 if failIfNotVisible
     * is true , otherwise returns groups view row index.
    */
    int getViewRowIndexByMaRowIndex(int maRowIndex, bool failIfNotVisible = false) const;

    /* Returns 'true' if the MA row is inside of some collapsible group and the group is collapsed. */
    bool isGroupWithMaRowIndexCollapsed(int maRowIndex) const;

    /* Returns the collapsible group index with the row or -1, if the row is not in a collapsible group. */
    int getCollapsibleGroupIndexByViewRowIndex(int viewRowIndex) const;

    /** Returns collapsible group by index or NULL if there is no group for the index. */
    const MaCollapsibleGroup* getCollapsibleGroup(int collapsibleGroupIndex) const;

    /* Returns number of view rows. Every collapsible group has at least one (the first) row counted. */
    int getViewRowCount() const;

    void setFakeCollapsibleModel(bool fakeModel);

    //TODO: rename!
    bool isFakeModel() const {return fakeModel;}

    /* Returns current set of collapsible groups. */
    const QVector<MaCollapsibleGroup>& getGroups() const {return groups;}

    bool hasGroupsWithMultipleRows() const { return hasGroupsWithMultipleItems; }

signals:
    void si_aboutToBeToggled();
    void si_toggled();

private:
    void updateIndex();

    QVector<MaCollapsibleGroup> groups;

    QHash<int, int> viewRowByMaRow;
    QHash<int, int> maRowByViewRow;
    QHash<int, int> groupByMaRow;
    bool hasGroupsWithMultipleItems;

    bool fakeModel;
};

} //namespace

#endif
