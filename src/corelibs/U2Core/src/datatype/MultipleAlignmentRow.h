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

#ifndef _U2_MULTIPLE_ALIGNMENT_ROW_H_
#define _U2_MULTIPLE_ALIGNMENT_ROW_H_

#include <U2Core/MsaRowUtils.h>
#include <U2Core/U2Ma.h>

namespace U2 {

class MultipleSequenceAlignment;

/**
 * A row in a multiple alignment structure.
 * The row consists of a sequence without gaps
 * and a gap model.
 * A row core is an obsolete concept. Currently,
 * it exactly equals to the row (offset always equals to zero).
 */
class U2CORE_EXPORT MultipleAlignmentRow {
    friend class MultipleSequenceAlignment;
public:
    /** Name of the row, can be empty */
    virtual QString getName() const = 0;
    virtual void setName(const QString &name) = 0;

    /** Returns the list of gaps for the row */
    inline const U2MaRowGapModel & getGapModel() const;

    /** Careful, the new gap model is not validated! */
    void setGapModel(const U2MaRowGapModel &newGapModel);

    /** Returns ID of the row in the database. */
    qint64 getRowId() const;

    void setRowId(qint64 rowId);

    void setDataId(const U2DataId &dataId);

    /** Returns ID of the row sequence in the database. */
    U2MaRow getRowDbInfo() const;

    /** Sets database IDs for row and sequence */
    void setRowDbInfo(const U2MaRow &dbRow);

    /** Returns length of the sequence + number of gaps including trailing gaps (if any) */
    int getRowLength() const;

    /** Returns length of the sequence + number of gaps. Doesn't include trailing gaps. */
    inline int getRowLengthWithoutTrailing() const;

    /** Packed version: returns the row without leading and trailing gaps */
    QByteArray getCore() const;

    /** Returns the row the way it is -- with leading and trailing gaps */
    QByteArray getData() const;

    /** Obsolete. Always return the row length (non-inclusive!) */
    inline int getCoreEnd() const;

    /** Obsolete. Always returns zero. */
    inline int getCoreStart() const;

    /** Obsolete. The length of the row core */
    int getCoreLength() const;

    /** Removes all gaps. Returns true if changed. */
    inline bool simplify();

    /** Adds anotherRow data to this row(ingores trailing gaps), "lengthBefore" must be greater than this row's length. */
    void append(const MultipleAlignmentRow &anotherRow, int lengthBefore, U2OpStatus &os);

    /**
     * Inserts 'count' gaps into the specified position, if possible.
     * If position is bigger than the row length or negative, does nothing.
     * Returns incorrect status if 'count' is negative.
     */
    void insertGaps(int pos, int count, U2OpStatus &os);

    /**
     * Removes up to 'count' characters starting from the specified position
     * If position is bigger than the row length, does nothing.
     * Returns incorrect status if 'pos' or 'count' is negative.
     */
    void removeChars(int pos, int count, U2OpStatus &os);

    /** Length of the sequence without gaps */
    inline int getUngappedLength() const;

    /**
     * If character at 'pos' position is not a gap, returns the char position in sequence.
     * Otherwise returns '-1'.
     */
    int getUngappedPosition(int pos) const;

    /**
     * Returns base count located leftward to the 'before' position in the alignment.
     */
    int getBaseCount(int before) const;

    /**
     * Exactly compares the rows. Sequences and gap models must match.
     * However, the rows are considered equal if they differ by trailing gaps only.
     */
    bool isRowContentEqual(const MultipleAlignmentRow &row) const;

    /** Compares 2 rows. Rows are equal if their contents and names are equal. */
    inline bool operator!=(const MultipleAlignmentRow &row) const;
    bool operator==(const MultipleAlignmentRow &row) const;

    /**
     * Crops the row -> keeps only specified region in the row.
     * 'pos' and 'pos + count' can be greater than the row length.
     * Keeps trailing gaps.
     */
    void crop(int pos, int count, U2OpStatus &os);

    /**
     * Returns new row of the specified 'count' length, started from 'pos'.
     * 'pos' and 'pos + count' can be greater than the row length.
     * Keeps trailing gaps.
     */
    MultipleAlignmentRow mid(int pos, int count, U2OpStatus &os) const;

    /**
     * Replaces all occurrences of 'origChar' by 'resultChar'.
     * The 'origChar' must be a non-gap character.
     * The 'resultChar' can be a gap, gaps model is recalculated in this case.
     */
    void replaceChars(char origChar, char resultChar, U2OpStatus &os);

    static const qint64 INVALID_ROW_ID;

private:
    /** Do NOT create a row without an alignment! */
    MultipleAlignmentRow(MultipleSequenceAlignment *al = NULL);
    MultipleAlignmentRow(const MultipleAlignmentRow &row, MultipleSequenceAlignment *al);

    /** Splits input to sequence bytes and gaps model */
    static void splitBytesToCharsAndGaps(const QByteArray &input, QByteArray &seqBytes, U2MaRowGapModel &gapModel);     // TODO: move to utils

    /**
     * Add "offset" of gaps to the beginning of the row
     * Warning: it is not verified that the row sequence is not empty.
     */
    static void addOffsetToGapModel(U2MaRowGapModel &gapModel, int offset);     // TODO: move to utils

    /** Gets the length of all gaps */
    inline int getGapsLength() const;

    /** If there are consecutive gaps in the gaps model, merges them into one gap */
    void mergeConsecutiveGaps();

    /** The row must not contain trailing gaps, this method is used to assure it after the row modification */
    void removeTrailingGaps();

    /**
     * Calculates start and end position in the sequence,
     * depending on the start position in the row and the 'count' character from it
     */
    void getStartAndEndSequencePositions(int pos, int count, int &startPosInSeq, int &endPosInSeq);

    /** Removing gaps from the row between position 'pos' and 'pos + count' */
    void removeGapsFromGapModel(int pos, int count);

    void setParentAlignment(MultipleSequenceAlignment *newAl);

    MultipleSequenceAlignment *alignment;

    /**
     * Gaps model of the row
     * There should be no trailing gaps!
     * Trailing gaps are 'Virtual': they are stored 'inside' the alignment length
     */
    U2MaRowGapModel gaps;

    /** The row in the database */
    U2MaRow initialRowInDb;
};

inline int MultipleAlignmentRow::getGapsLength() const {
    return MsaRowUtils::getGapsLength(gaps);
}

inline int MultipleAlignmentRow::getCoreStart() const {
    return MsaRowUtils::getCoreStart(gaps);
}

inline int MultipleAlignmentRow::getCoreEnd() const {
    return getRowLengthWithoutTrailing();
}

inline int MultipleAlignmentRow::getRowLengthWithoutTrailing() const {
    return MsaRowUtils::getRowLength(sequence.seq, gaps);
}

inline int MultipleAlignmentRow::getUngappedLength() const {
    return sequence.length();
}

inline bool MultipleAlignmentRow::simplify() {
    if (gaps.count() > 0) {
        gaps.clear();
        return true;
    }
    return false;
}

inline const U2MaRowGapModel & MultipleAlignmentRow::getGapModel() const {
    return gaps;
}

inline bool MultipleAlignmentRow::operator!=(const MultipleAlignmentRow &row) const {
    return !(*this == row);
}

}   // namespace U2

#endif // _U2_MULTIPLE_ALIGNMENT_ROW_H_
