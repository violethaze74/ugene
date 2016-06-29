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

#ifndef _U2_MULTIPLE_SEQUENCE_ALIGNMENT_H_
#define _U2_MULTIPLE_SEQUENCE_ALIGNMENT_H_

#include <U2Core/DNASequence.h>
#include <U2Core/MsaRowUtils.h>
#include <U2Core/U2Ma.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2Region.h>
#include <U2Core/U2SafePoints.h>

#include "MultipleAlignmentInfo.h"
#include "MultipleSequenceAlignmentRow.h"

namespace U2 {

/** Default name for a multiple alignment */
#define MA_OBJECT_NAME QString("Multiple alignment")

/** Gap character */
#define MAlignment_GapChar '-'
#define MAlignment_TailedGapsPattern "\\-+$"

class DNAAlphabet;

/**
 * Multiple sequence alignment
 * The length of the alignment is the maximum length of its rows.
 * There are minimal checks on the alignment's alphabet, but the client of the class
 * is expected to keep the conformance of the data and the alphabet.
 */
class U2CORE_EXPORT MultipleSequenceAlignment {
public:
    /**
     * Creates a new alignment.
     * The name must be provided if this is not default alignment.
     */
    MultipleSequenceAlignment(const QString& name = QString(),
               const DNAAlphabet* alphabet = NULL,
               const QList<MultipleSequenceAlignmentRow>& rows = QList<MultipleSequenceAlignmentRow>());
    MultipleSequenceAlignment(const MultipleSequenceAlignment& m);

    MultipleSequenceAlignment & operator=(const MultipleSequenceAlignment &other);

    /**
     * Clears the alignment. Makes alignment length == 0.
     * Doesn't change alphabet or name
     */
    void clear();

    /** Returns  the name of the alignment */
    QString getName() const { return MultipleAlignmentInfo::getName(info); }

    /** Sets the name of the alignment */
    void setName(const QString& newName) { MultipleAlignmentInfo::setName(info, newName); }

    /** Returns the alphabet of the alignment */
    const DNAAlphabet* getAlphabet() const { return alphabet; }

    /**
     * Sets the alphabet of the alignment, the value can't be NULL.
     * Warning: rows already present in the alignment are not verified to correspond to this alphabet
     */
    void setAlphabet(const DNAAlphabet* al);

    /** Returns the alignment info */
    QVariantMap getInfo() const { return info; }

    /** Sets the alignment info */
    void setInfo(const QVariantMap& _info) { info = _info; }

    /** Returns true if the length of the alignment is 0 */
    bool isEmpty() const { return getLength() == 0; }

    /** Returns the length of the alignment */
    int getLength() const { return length; }

    /** Sets the length of the alignment. The length must be >= 0. */
    void setLength(int len);

    /** Returns the number of rows in the alignment */
    int getNumRows() const { return rows.size(); }

    /**
     * Recomputes the length of the alignment and makes it as minimal
     * as possible. All leading gaps columns are removed by default.
     * Returns "true" if the alignment has been modified.
     */
    bool trim( bool removeLeadingGaps = true );

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
    bool sortRowsBySimilarity(QVector<U2Region>& united);

    /** Returns row of the alignment */
    inline MultipleSequenceAlignmentRow& getRow(int row);
    inline const MultipleSequenceAlignmentRow& getRow(int row) const;

    const MultipleSequenceAlignmentRow& getRow(QString name) const;

    /** Returns all rows in the alignment */
    const QList<MultipleSequenceAlignmentRow>& getRows() const { return rows; }

    /** Returns IDs of the alignment rows in the database */
    QList<qint64> getRowsIds() const;

    MultipleSequenceAlignmentRow getRowByRowId(qint64 rowId, U2OpStatus& os) const;

    /** Returns all rows' names in the alignment */
    QStringList getRowNames() const;

    int getRowIndexByRowId(qint64 rowId, U2OpStatus &os) const;

    /** Returns a character (a gap or a non-gap) in the specified row and position */
    char charAt(int rowIndex, int pos) const;

    /**
     * Inserts 'count' gaps into the specified position.
     * Can increase the overall alignment length.
     */
    void insertGaps(int row, int pos, int count, U2OpStatus& os);

    /**
     * Removes up to n characters starting from the specified position.
     * Can decrease the overall alignment length.
     */
    void removeChars(int row, int pos, int count, U2OpStatus& os);

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
    void renameRow(int row, const QString& name);

    /**
     * Sets the new content for the row with the specified index.
     * Assumes that the row index is valid.
     * Can modify the overall alignment length (increase or decrease).
     */
    void setRowContent(int row, const QByteArray& sequence, int offset = 0);

    /** Converts all rows' sequences to upper case */
    void toUpperCase();

    /**
     * Modifies the alignment by keeping data from the specified region and rows only.
     * Assumes that the region start is not negative, but it can be greater than a row length.
     */
    bool crop(const U2Region& region, const QSet<QString>& rowNames, U2OpStatus& os);
    bool crop(const U2Region &region, U2OpStatus& os);
    bool crop(int start, int count, U2OpStatus& os);

    /**
     * Creates a new alignment from the sub-alignment. Do not trims the result.
     * Assumes that 'start' >= 0, and 'start + len' is less or equal than the alignment length.
     */
    MultipleSequenceAlignment mid(int start, int len) const;

    U2MaListGapModel getGapModel() const;

    void setRowGapModel(int rowIndex, const QList<U2MaGap>& gapModel);

    /** Updates row ID of the row at 'rowIndex' position */
    void setRowId(int rowIndex, qint64 rowId);

    void setSequenceId(int rowIndex, U2DataId sequenceId);

    /**
     * Adds a new row to the alignment.
     * If rowIndex == -1 -> appends the row to the alignment.
     * Otherwise, if rowIndex is incorrect, the closer bound is used (the first or the last row).
     * Does not trim the original alignment.
     * Can increase the overall alignment length.
     */
    void addRow(const QString& name, const QByteArray& bytes, U2OpStatus& os);
    void addRow(const QString& name, const QByteArray& bytes, int rowIndex, U2OpStatus& os);
    void addRow(const U2MaRow& rowInDb, const DNASequence& sequence, U2OpStatus& os);
    void addRow(const QString& name, const DNASequence& sequence, const QList<U2MaGap>& gaps, U2OpStatus& os);

    /**
     * Removes a row from alignment.
     * The alignment is changed only (to zero) if the alignment becomes empty.
     */
    void removeRow(int rowIndex, U2OpStatus& os);

    /**
     * Shifts a selection of consequent rows.
     * 'delta' can be positive or negative.
     * It is assumed that indexes of shifted rows are within the bounds of the [0, number of rows).
     */
    void moveRowsBlock(int startRow, int numRows, int delta);

    /**
     * Replaces all occurrences of 'origChar' by 'resultChar' in the row with the specified index.
     * The 'origChar' must be a non-gap character.
     * The 'resultChar' can be a gap, gaps model is recalculated in this case.
     * The index must be valid as well.
     */
    void replaceChars(int row, char origChar, char resultChar);

    /**
     * Appends chars to the row with the specified index.
     * The chars are appended to the alignment end, not to the row end
     * (i.e. the alignment length is taken into account).
     * Does NOT recalculate the alignment length!
     * The index must be valid.
     */
    void appendChars(int row, const char* str, int len);

    void appendChars(int row, int afterPos, const char *str, int len);

    void appendRow(int rowIdx, const MultipleSequenceAlignmentRow& r, bool ignoreTrailingGaps, U2OpStatus& os);

    void appendRow(int rowIdx, int afterPos, const MultipleSequenceAlignmentRow &r, U2OpStatus& os);

    /** returns "True" if there are no gaps in the alignment */
    bool hasEmptyGapModel() const;

    /**  returns "True" if all sequences in the alignment have equal lengths */
    bool hasEqualLength() const;

    /**
     * Joins two alignments. Alignments must have the same size and alphabet.
     * Increases the alignment length.
     */
    MultipleSequenceAlignment& operator+=(const MultipleSequenceAlignment& ma);

    /**
     * Compares two alignments: lengths, alphabets, rows and infos (that include names).
     */
    bool operator==(const MultipleSequenceAlignment& ma) const;
    bool operator!=(const MultipleSequenceAlignment& ma) const;


    /** Estimates memory size consumed by alignment structure in bytes */
    int estimateMemorySize() const;

    /** Checks model consistency */
    void check() const;

    /** Arranges rows in lists order*/
    bool sortRowsByList(const QStringList& order);

    static const char GapChar;

private:
    /** Create a new row (sequence + gap model) from the bytes */
    MultipleSequenceAlignmentRow createRow(const QString& name, const QByteArray& bytes, U2OpStatus& os);

    /**
     * Sequence must not contain gaps.
     * All gaps in the gaps model (in 'rowInDb') must be valid and have an offset within the bound of the sequence.
     */
    MultipleSequenceAlignmentRow createRow(const U2MaRow& rowInDb, const DNASequence& sequence, const QList<U2MaGap>& gaps, U2OpStatus& os);

    MultipleSequenceAlignmentRow createRow(const MultipleSequenceAlignmentRow& r, U2OpStatus& os);

    /** Helper-method for adding a row to the alignment */
    void addRow(const MultipleSequenceAlignmentRow& row, int rowLenWithTrailingGaps, int rowIndex, U2OpStatus& os);

    /** Alphabet for all sequences in the alignment */
    const DNAAlphabet*            alphabet;

    /** Alignment rows (each row = sequence + gap model) */
    QList<MultipleSequenceAlignmentRow>    rows;

    /** The length of the longest row in the alignment */
    int                     length;

    /** Additional alignment info */
    QVariantMap             info;

private:
    static bool registerMeta;
};

inline MultipleSequenceAlignmentRow& MultipleSequenceAlignment::getRow(int rowIndex) {
    static MultipleSequenceAlignmentRow emptyRow;
    int rowsCount = rows.count();
    SAFE_POINT(0 != rowsCount, "No rows!", emptyRow);
    SAFE_POINT(rowIndex >= 0 && (rowIndex < rowsCount), "Internal error: unexpected row index was passed to MAlignmnet::getRow!", emptyRow);
    return rows[rowIndex];
}

inline const MultipleSequenceAlignmentRow& MultipleSequenceAlignment::getRow(int rowIndex) const {
    static MultipleSequenceAlignmentRow emptyRow;
    int rowsCount = rows.count();
    SAFE_POINT(0 != rowsCount, "No rows!", emptyRow);
    SAFE_POINT(rowIndex >= 0 && (rowIndex < rowsCount), "Internal error: unexpected row index was passed to MAlignmnet::getRow!", emptyRow);
    return rows.at(rowIndex);
}

}//namespace

Q_DECLARE_METATYPE(U2::MultipleSequenceAlignment)

#endif
