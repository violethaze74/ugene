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

#ifndef _U2_MSA_COLLAPSIBLE_MODEL_H_
#define _U2_MSA_COLLAPSIBLE_MODEL_H_

#include <QObject>
#include <QVector>

#include <U2Core/MultipleSequenceAlignment.h>

namespace U2 {

class MSACollapsibleItem {
public:
    /* Creates new collapsible group item in invalid state. */
    MSACollapsibleItem();

    /* Creates new collapsible group item that starts with msaRowIndex and has numRows inside. */
    MSACollapsibleItem(int msaRowIndex, int numRows);

    /* Returns true if the collapsible item values (rowIndex & numRows) are valid. */
    bool isValid() const;

    /* First collapsible row index in MSA model. */
    int msaRowIndex;

    /* Number of collapsible rows in the group. */
    int numRows;

    /* If group is collapsed or not. */
    bool isCollapsed;
};

class MaEditorWgt;
class MaModificationInfo;
class U2Region;

class U2VIEW_EXPORT MSACollapsibleItemModel : public QObject {
    Q_OBJECT
public:
    MSACollapsibleItemModel(MaEditorWgt *p);

    // Updates model to the given regions with every item collapsed.
    // 'collapsibleItemRegions' must be a sorted list of non-intersecting regions.
    void reset(const QVector<U2Region> &collapsibleItemRegions);

    /* Removes all collapsible items from the model. */
    void reset();

    /* Toggle 'isCollapsed' state for the item at the given row. */
    void toggle(int viewRowIndex);

    /* Collapse all groups in the view. */
    void collapseAll(bool collapse);

    /* Converts view row index to msa row index. */
    int viewRowToMsaRow(int viewRowIndex) const;

    /* Converts view rows region to msa rows region. */
    U2Region viewRowsToMsaRows(const U2Region &viewRowsRegion) const;

    /* Returns list of visible msa row indexes. Invisible rows are not included to the result. */
    QList<int> visibleViewRowsToMsaRows(const U2Region &viewRowsRegion);

    /* Returns list of all visible MSA row indexes. */
    QList<int> getVisibleMsaRows() const;

    /*
    * Converts msa row index into its view position (i.e. the row position that takes into account collapsed items).
    * Returns -1 if the row is inside of collapsed item and @failIfNotVisible is true.
    */
    int msaRowToViewRow(int msaRowIndex, bool failIfNotVisible = false) const;

    /* Adds all visible MSA regions between start & end (inclusive) view rows to the 'visibleRowsResult'. */
    void getVisibleMsaRows(int startViewRowIndex, int endViewRowIndex, QVector<U2Region> &visibleMsaRows) const;

    /* Returns 'true' if the view row index is the first row in some collapsible group. */
    bool isFirstRowOfCollapsibleGroup(int viewRowIndex) const;

    /* Returns 'true' if the view row index is inside of some collapsible group. */
    bool isInCollapsibleGroup(int viewRowIndex) const;

    /* Returns 'true' if the MSA row is inside of some collapsible group and the group is collapsed. */
    bool isGroupWithMsaRowCollapsed(int msaRowIndex) const;

    /** Returns true if MSA row is visible (not inside of collapsed block). */
    bool isMsaRowVisible(int msaRowIndex) const;

    /* Returns the collapsible group index with the row or -1, if the row is not in a collapsible group. */
    int viewRowToGroupIndex(int viewRowIndex) const;

    MSACollapsibleItem getItem(int collapsibleItemIndex) const;

    /* Returns collapsible group item by msa row index. */
    MSACollapsibleItem getItemByMsaRowIndex(int msaRowIndex) const;

    /* Returns number of visible rows. Every collapsible group has at least one (the first) row visible. */
    int getVisibleRowCount() const;

    /* Returns true if there are no collapsible groups in the model. */
    bool isEmpty() const;

    void setFakeCollapsibleModel(bool fakeModel);

    bool isFakeModel() const;

    /* Returns count of collapsible groups in the view. */
    int getCollapsibleItemCount() const;

signals:
    void si_aboutToBeToggled();
    void si_toggled();

private:
    void triggerItem(int collapsibleGroupIndex);
    int mapToMsaRow(int collapsibleGroupIndex, int viewRowIndex) const;

private:
    MaEditorWgt* ui;
    QVector<MSACollapsibleItem> items;
    QVector<int> positions;
    bool fakeModel;
};

} //namespace

#endif
