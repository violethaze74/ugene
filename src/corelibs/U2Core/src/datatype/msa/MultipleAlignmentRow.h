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

#include <QSharedPointer>

#include <U2Core/MsaRowUtils.h>
#include <U2Core/U2Msa.h>

namespace U2 {

class MultipleSequenceAlignment;
class MultipleSequenceAlignmentData;
class MultipleAlignmentRowData;
class U2OpStatus;

class U2CORE_EXPORT MultipleSequenceAlignmentRow {
    friend class MultipleSequenceAlignmentData;

protected:
    MultipleSequenceAlignmentRow(MultipleSequenceAlignmentRowData *msaRowData);

public:
    MultipleSequenceAlignmentRow();
    MultipleSequenceAlignmentRow(const MultipleSequenceAlignmentData *msaData);

    /** Creates a row in memory. */
    MultipleSequenceAlignmentRow(const U2MsaRow &rowInDb, const DNASequence &sequence, const U2MsaRowGapModel &gaps, const MultipleSequenceAlignmentData *msa);
    MultipleSequenceAlignmentRow(const U2MsaRow &rowInDb, const QString &rowName, const QByteArray &rawData, const MultipleSequenceAlignmentData *msa);
    MultipleSequenceAlignmentRow(const MultipleSequenceAlignmentRow &row, const MultipleSequenceAlignmentData *msa);

    virtual ~MultipleSequenceAlignmentRow();

    MultipleAlignmentRowData * data() const;
    template <class Derived> inline Derived dynamicCast() const;

    MultipleAlignmentRowData & operator*();
    const MultipleAlignmentRowData & operator*() const;

    MultipleAlignmentRowData * operator->();
    const MultipleAlignmentRowData * operator->() const;

    MultipleSequenceAlignmentRow clone() const;

protected:
    QSharedPointer<MultipleAlignmentRowData> msaRowData;
};

template <class Derived>
Derived MultipleSequenceAlignmentRow::dynamicCast() const {
    return Derived(*this);
}

/**
 * A row in a multiple alignment structure.
 * The row consists of a sequence without gaps
 * and a gap model.
 * A row core is an obsolete concept. Currently,
 * it exactly equals to the row (offset always equals to zero).
 */
class U2CORE_EXPORT MultipleAlignmentRowData {
protected:
    /** Do NOT create a row without an alignment! */
    MultipleSequenceAlignmentRowData();
    MultipleSequenceAlignmentRowData(const MultipleSequenceAlignmentData *msa);

    MultipleAlignmentRowData(const MultipleSequenceAlignmentData *alignment = NULL);
    MultipleAlignmentRowData(const MultipleSequenceAlignmentRow &row, const MultipleSequenceAlignmentData *alignment);
    MultipleAlignmentRowData(const U2MsaRow &rowInDb, const U2MsaRowGapModel &gaps, const MultipleSequenceAlignmentData *alignment);

public:
    virtual ~MultipleAlignmentRowData();

    /** Name of the row, can be empty */
    QString getName() const;
    void setName(const QString &name);

    /** Returns the list of gaps for the row */
    const U2MsaRowGapModel & getGapModel() const;

    /** Careful, the new gap model is not validated! */
    void setGapModel(const U2MsaRowGapModel &newGapModel);

    /** Returns ID of the row in the database. */
    qint64 getRowId() const;

    void setRowId(qint64 rowId);

    void setDataId(const U2DataId &dataId);

    /** Returns ID of the row sequence in the database. */
    U2MsaRow getRowDbInfo() const;

    /** Sets database IDs for row and sequence */
    void setRowDbInfo(const U2MsaRow &dbRow);

    /** Returns length of the sequence + number of gaps including trailing gaps (if any) */
    int getRowLength() const;

    /** Returns length of the sequence + number of gaps. Doesn't include trailing gaps. */
    int getRowLengthWithoutTrailing() const;

    /** Obsolete. Always return the row length (non-inclusive!) */
    int getCoreEnd() const;

    /** Obsolete. Always returns zero. */
    int getCoreStart() const;

    /** Obsolete. The length of the row core */
    int getCoreLength() const;

    int getUngappedLength() const;

    /** Removes all gaps. Returns true if changed. */
    bool simplify();

    /** Adds anotherRow data to this row(ingores trailing gaps), "lengthBefore" must be greater than this row's length. */
    void append(const MultipleSequenceAlignmentRow &anotherRow, int lengthBefore, U2OpStatus &os);

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
    void removeData(int pos, int count, U2OpStatus &os);

    /**
     * If character at 'pos' position is not a gap, returns the char position in sequence.
     * Otherwise returns '-1'.
     */
    int getUngappedPosition(int pos) const;

    /**
     * Returns base count located leftward to the 'before' position in the alignment.
     */
    int getDataSize(int before) const;

    /**
     * Exactly compares the rows. Sequences and gap models must match.
     * However, the rows are considered equal if they differ by trailing gaps only.
     */
    bool isRowContentEqual(const MultipleSequenceAlignmentRow &row) const;
    bool isRowContentEqual(const MultipleAlignmentRowData &rowData) const;

    /** Compares 2 rows. Rows are equal if their contents and names are equal. */
    bool operator!=(const MultipleAlignmentRowData &rowData) const;
    bool operator==(const MultipleAlignmentRowData &rowData) const;

    /**
     * Crops the row -> keeps only specified region in the row.
     * 'pos' and 'pos + count' can be greater than the row length.
     * Keeps trailing gaps.
     */
    void crop(int pos, int count, U2OpStatus &os);

    void setParentAlignment(const MultipleSequenceAlignment &newAlignment);

    bool isGap(int position) const;

    /** Returns the row sequence (without gaps) */
    const DNASequence & getSequence() const;

    /**
     * Sets a new sequence. Be careful, gap model validity is not verified.
     * The sequence must not contain gaps.
     */
    void setSequence(const DNASequence &newSequence);

    /**
     * The length must be greater or equal to the row length.
     * When the specified length is greater, an appropriate number of
     * trailing gaps are appended to the end of the byte array.
     */
    QByteArray toByteArray(int length, U2OpStatus &os) const;

    /** Packed version: returns the row without leading and trailing gaps */
    QByteArray getCore() const;

    /** Returns the row the way it is -- with leading and trailing gaps */
    QByteArray getData() const;

    /**
     * Sets new sequence and gap model.
     * If the sequence is empty, the offset is ignored (if any).
     */
    void setRowContent(const QByteArray &bytes, int offset);

    /**
     * Returns a character in row at the specified position.
     * If the specified position is outside the row bounds, returns a gap.
     */
    char charAt(int pos) const;

    /** Converts the row sequence to upper case */
    void toUpperCase();

    /**
     * Replaces all occurrences of 'origChar' by 'resultChar'.
     * The 'origChar' must be a non-gap character.
     * The 'resultChar' can be a gap, gaps model is recalculated in this case.
     */
    void replaceChars(char origChar, char resultChar, U2OpStatus &os);

    /**
     * Returns new row of the specified 'count' length, started from 'pos'.
     * 'pos' and 'pos + count' can be greater than the row length.
     * Keeps trailing gaps.
     */
    MultipleSequenceAlignmentRow mid(int pos, int count, U2OpStatus &os) const;

    MultipleSequenceAlignmentRow getExplicitCopy() const;

    static const qint64 INVALID_ROW_ID;

protected:
    /** The row must not contain trailing gaps, this method is used to assure it after the row modification */
    void removeTrailingGaps();

    /** If there are consecutive gaps in the gaps model, merges them into one gap */
    void mergeConsecutiveGaps();

    /** Next methods are supposed to work with data ignoring gap model */
    virtual int getDataLength() const = 0;
    virtual void appendDataCore(const MultipleSequenceAlignmentRow &anotherRow) = 0;
    virtual void removeDataCore(int startPosInData, int endPosInData, U2OpStatus &os) = 0;
    virtual bool isDataEqual(const MultipleAlignmentRowData &rowData) const = 0;

private:
    /** Gets the length of all gaps */
    int getGapsLength() const;

    /**
     * Calculates start and end position in the sequence,
     * depending on the start position in the row and the 'count' character from it
     */
    void getStartAndEndDataPositions(int pos, int count, int &startPosInSeq, int &endPosInSeq);

    /** Removing gaps from the row between position 'pos' and 'pos + count' */
    void removeGapsFromGapModel(int pos, int count);

    const MultipleSequenceAlignmentData *alignment;

    /**
     * Gaps model of the row
     * There should be no trailing gaps!
     * Trailing gaps are 'Virtual': they are stored 'inside' the alignment length
     */
    U2MsaRowGapModel gaps;

    /** The row in the database */
    U2MsaRow initialRowInDb;
};


inline bool	operator!=(const MultipleSequenceAlignmentRow &ptr1, const MultipleSequenceAlignmentRow &ptr2) { return *ptr1 != *ptr2; }
inline bool	operator!=(const MultipleSequenceAlignmentRow &ptr1, const MultipleAlignmentRowData *ptr2) { return *ptr1 != *ptr2; }
inline bool	operator!=(const MultipleAlignmentRowData *ptr1, const MultipleSequenceAlignmentRow &ptr2) { return *ptr1 != *ptr2; }
inline bool	operator==(const MultipleSequenceAlignmentRow &ptr1, const MultipleSequenceAlignmentRow &ptr2) { return *ptr1 == *ptr2; }
inline bool	operator==(const MultipleSequenceAlignmentRow &ptr1, const MultipleAlignmentRowData *ptr2) { return *ptr1 == *ptr2; }
inline bool	operator==(const MultipleAlignmentRowData *ptr1, const MultipleSequenceAlignmentRow &ptr2) { return *ptr1 == *ptr2; }

}   // namespace U2

#endif // _U2_MULTIPLE_ALIGNMENT_ROW_H_
