/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
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

#ifndef _U2_MULTIPLE_ALIGNMENT_H_
#define _U2_MULTIPLE_ALIGNMENT_H_

#include <U2Core/U2Region.h>

#include "MultipleAlignmentRow.h"

namespace U2 {

class DNAAlphabet;

/** Default name for a multiple alignment */
#define MA_OBJECT_NAME QString("Multiple alignment")

/** Gap character */
#define MAlignment_GapChar '-'
#define MAlignment_TailedGapsPattern "\\-+$"

typedef QSharedPointer<MultipleAlignmentData> MultipleAlignment;

/**
 * Multiple alignment
 * The length of the alignment is the maximum length of its rows.
 * There are minimal checks on the alignment's alphabet, but the client of the class
 * is expected to keep the conformance of the data and the alphabet.
 */
class U2CORE_EXPORT MultipleAlignmentData {
protected:
    /**
     * Creates a new alignment.
     * The name must be provided if this is not default alignment.
     */
    MultipleAlignmentData(const QString &name = QString(),
                      const DNAAlphabet *alphabet = NULL,
                      const QList<MultipleAlignmentRow> &rows = QList<MultipleAlignmentRow>());
    MultipleAlignmentData(const MultipleAlignmentData &multipleAlignment);

public:
    virtual ~MultipleAlignmentData();

    const MultipleAlignmentData & operator=(const MultipleAlignmentData &other);

    /**
     * Clears the alignment. Makes alignment length == 0.
     * Doesn't change alphabet or name
     */
    void clear();

    /** Returns  the name of the alignment */
    QString getName() const;

    /** Sets the name of the alignment */
    void setName(const QString &newName);

    /** Returns the alphabet of the alignment */
    const DNAAlphabet * getAlphabet() const;

    /**
     * Sets the alphabet of the alignment, the value can't be NULL.
     * Warning: rows already present in the alignment are not verified to correspond to this alphabet
     */
    void setAlphabet(const DNAAlphabet *alphabet);

    /** Returns the alignment info */
    QVariantMap getInfo() const;

    /** Sets the alignment info */
    void setInfo(const QVariantMap &info);

    /** Returns true if the length of the alignment is 0 */
    bool isEmpty() const;

    /** Returns the length of the alignment */
    int getLength() const;

    /** Sets the length of the alignment. The length must be >= 0. */
    void setLength(int length);

    /** Returns the number of rows in the alignment */
    int getNumRows() const;

    /**
     * Recomputes the length of the alignment and makes it as minimal
     * as possible. All leading gaps columns are removed by default.
     * Returns "true" if the alignment has been modified.
     */
    bool trim(bool removeLeadingGaps = true);

    /**
     * Removes all gaps from all columns in the alignment.
     * Returns "true" if the alignment has been changed.
     */
    bool simplify();

    /** Sorts rows by name */
    void sortRowsByName(bool asc = true);

    /**
     * Sorts rows by similarity making identical rows sequential.
     * Returns 'true' if the rows were resorted, and 'false' otherwise.
     */
    bool sortRowsBySimilarity(QVector<U2Region> &united);

    /** Returns row of the alignment */
    MultipleAlignmentRow & getRow(int row);
    const MultipleAlignmentRow & getRow(int row) const;
    const MultipleAlignmentRow & getRow(const QString &name) const;

    /** Returns all rows in the alignment */
    const QList<MultipleAlignmentRow> & getRows() const;

    /** Returns IDs of the alignment rows in the database */
    QList<qint64> getRowsIds() const;

    MultipleAlignmentRow getRowByRowId(qint64 rowId, U2OpStatus &os) const;

    /** Returns all rows' names in the alignment */
    QStringList getRowNames() const;

    int getRowIndexByRowId(qint64 rowId, U2OpStatus &os) const;

    /**
     * Inserts 'count' gaps into the specified position.
     * Can increase the overall alignment length.
     */
    void insertGaps(int row, int pos, int count, U2OpStatus &os);

    /**
     * Removes up to n characters starting from the specified position.
     * Can decrease the overall alignment length.
     */
    void removeRowData(int row, int pos, int count, U2OpStatus &os);

    /**
     * Removes a region from the alignment.
     * If "removeEmptyRows" is "true", removes all empty rows from the processed region.
     * The alignment is trimmed after removing the region.
     * Can decrease the overall alignment length.
     */
    void removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows);

    /**
     * Renames the row with the specified index.
     * Assumes that the row index is valid and the name is not empty.
     */
    void renameRow(int row, const QString &name);

    /**
     * Modifies the alignment by keeping data from the specified region and rows only.
     * Assumes that the region start is not negative, but it can be greater than a row length.
     */
    bool crop(const U2Region &region, const QSet<QString> &rowNames, U2OpStatus &os);
    bool crop(const U2Region &region, U2OpStatus &os);
    bool crop(int start, int count, U2OpStatus &os);

    U2MaListGapModel getGapModel() const;

    void setRowGapModel(int rowIndex, const U2MaRowGapModel &gapModel);

    /** Updates row ID of the row at 'rowIndex' position */
    void setRowId(int rowIndex, qint64 rowId);

    void setDataId(int rowIndex, const U2DataId &dataId);

    /**
     * Removes a row from alignment.
     * The alignment is changed only (to zero) if the alignment becomes empty.
     */
    void removeRow(int rowIndex, U2OpStatus &os);

    /**
     * Shifts a selection of consequent rows.
     * 'delta' can be positive or negative.
     * It is assumed that indexes of shifted rows are within the bounds of the [0, number of rows).
     */
    void moveRowsBlock(int startRow, int numRows, int delta);

    void appendRow(int rowIdx, const MultipleAlignmentRow &row, bool ignoreTrailingGaps, U2OpStatus &os);
    void appendRow(int rowIdx, int afterPos, const MultipleAlignmentRow &row, U2OpStatus &os);

    /** returns "True" if there are no gaps in the alignment */
    bool hasEmptyGapModel() const;

    /**  returns "True" if all sequences in the alignment have equal lengths */
    bool hasEqualDataLength() const;

    /**
     * Joins two alignments. Alignments must have the same size and alphabet.
     * Increases the alignment length.
     */
    const MultipleAlignmentData & operator+=(const MultipleAlignmentData &ma);

    /**
     * Compares two alignments: lengths, alphabets, rows and infos (that include names).
     */
    bool operator==(const MultipleAlignmentData &ma) const;
    bool operator!=(const MultipleAlignmentData &ma) const;

    /** Checks model consistency */
    void check() const;

    /** Arranges rows in lists order*/
    bool sortRowsByList(const QStringList &order);

    bool isGap(int row, int pos) const;

    static const char GapChar;

    virtual MultipleAlignmentData * clone() const = 0;

protected:
    virtual MultipleAlignmentRow createRow(const MultipleAlignmentRow &row) const = 0;

    /** Helper-method for adding a row to the alignment */
    void addRowPrivate(const MultipleAlignmentRow &row, int rowLenWithTrailingGaps, int rowIndex);

private:
    void copy(const MultipleAlignmentData &other);

    /** Alphabet for all sequences in the alignment */
    const DNAAlphabet *alphabet;

    /** Alignment rows (each row = sequence + gap model) */
    QList<MultipleAlignmentRow> rows;

    /** The length of the longest row in the alignment */
    int length;

    /** Additional alignment info */
    QVariantMap info;
};

}   // namespace U2

#endif // _U2_MULTIPLE_ALIGNMENT_H_
