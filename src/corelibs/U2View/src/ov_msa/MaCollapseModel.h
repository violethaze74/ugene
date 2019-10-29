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

#include <U2Core/MultipleSequenceAlignment.h>

namespace U2 {

class MaCollapsibleGroup {
public:
    /* Creates new collapsible group item in invalid state. */
    MaCollapsibleGroup();

    /* Creates new collapsible group item that starts with maRowIndex and has numRows inside. */
    MaCollapsibleGroup(int maRowIndex, int numRows);

    /* Returns true if the collapsible item values (rowIndex & numRows) are valid. */
    bool isValid() const;

    /* First collapsible row index in MA model. */
    int maRowIndex;

    /* Number of collapsible rows in the group. */
    int numRows;

    /* If group is collapsed or not. */
    bool isCollapsed;
};

class MaEditorWgt;
class MaModificationInfo;
class U2Region;

class U2VIEW_EXPORT MaCollapseModel : public QObject {
    Q_OBJECT
public:
    MaCollapseModel(MaEditorWgt *p);

    /*
    * Updates model to the given regions with every group collapsed.
    * 'collapsibleGroupRegions' must be a sorted list of non-intersecting regions.
    */
    void update(const QVector<U2Region> &collapsibleGroupRegions);

    /* Removes all collapsible groups from the model. */
    void clear();

    /* Toggle 'isCollapsed' state for the group at the given row. */
    void toggle(int viewRowIndex);

    /* Collapse all groups in the view. */
    void collapseAll(bool collapse);

    /* Converts view row index to MA row index. */
    int viewRowToMaRow(int viewRowIndex) const;

    /* Converts view rows region to MA rows region. */
    U2Region viewRowsToMaRows(const U2Region &viewRowsRegion) const;

    /* Returns list of visible (not hidden by collapsing) MA row indexes. Invisible rows are not included to the result. */
    QList<int> visibleViewRowsToMaRows(const U2Region &viewRowsRegion);

    /* Returns list of all visible (not hidden by collapsing) MA row indexes. */
    QList<int> getVisibleMaRows() const;

    /* Returns all visible (not hidden by collapsing) MA regions between start & end (inclusive) view rows. */
    QList<U2Region> getVisibleMaRows(int startViewRowIndex, int endViewRowIndex) const;

    /*
    * Converts MA row index into its view position (i.e. the row position that takes into account collapsed groups).
    * Returns -1 if the row is inside of collapsed group and @failIfNotVisible is true.
    */
    int maRowToViewRow(int maRowIndex, bool failIfNotVisible = false) const;

    /* Returns 'true' if the view row index is the first row in some collapsible group. */
    bool isFirstRowOfCollapsibleGroup(int viewRowIndex) const;

    /* Returns 'true' if the view row index is inside of some collapsible group. */
    bool isInCollapsibleGroup(int viewRowIndex) const;

    /* Returns 'true' if the MA row is inside of some collapsible group and the group is collapsed. */
    bool isGroupWithMaRowCollapsed(int maRowIndex) const;

    /* Returns true if MA row is visible (not inside of collapsed block). */
    bool isMaRowVisible(int maRowIndex) const;

    /* Returns the collapsible group index with the row or -1, if the row is not in a collapsible group. */
    int viewRowToGroupIndex(int viewRowIndex) const;

    MaCollapsibleGroup getCollapsibleGroup(int collapsibleGroupIndex) const;

    /* Returns collapsible group item by MA row index. */
    MaCollapsibleGroup getItemByMaRowIndex(int maRowIndex) const;

    /* Returns number of visible rows. Every collapsible group has at least one (the first) row visible. */
    int getVisibleRowCount() const;

    /* Returns true if there are no collapsible groups in the model. */
    bool isEmpty() const;

    void setFakeCollapsibleModel(bool fakeModel);

    bool isFakeModel() const;

    /* Returns count of collapsible groups in the view. */
    int getCollapsibleGroupCount() const;

signals:
    void si_aboutToBeToggled();
    void si_toggled();

private:
    void triggerItem(int collapsibleGroupIndex);
    int mapToMaRow(int collapsibleGroupIndex, int viewRowIndex) const;

private:
    MaEditorWgt* ui;
    QVector<MaCollapsibleGroup> groups;
    QVector<int> positions;
    bool fakeModel;
};

} //namespace

#endif
