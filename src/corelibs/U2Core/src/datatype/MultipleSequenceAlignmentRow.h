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

#ifndef _U2_MULTIPLE_SEQUENCE_ALIGNMENT_ROW_H_
#define _U2_MULTIPLE_SEQUENCE_ALIGNMENT_ROW_H_

#include <U2Core/DNASequence.h>
#include <U2Core/MsaRowUtils.h>
#include <U2Core/U2Ma.h>

#include "MultipleAlignmentRow.h"

namespace U2 {

class MultipleSequenceAlignmentData;
class MultipleSequenceAlignmentRowData;

typedef  QSharedPointer<MultipleSequenceAlignmentData> MultipleSequenceAlignment;
typedef  QSharedPointer<MultipleSequenceAlignmentRowData> MultipleSequenceAlignmentRow;

/**
 * A row in a multiple alignment structure.
 * The row consists of a sequence without gaps
 * and a gap model.
 * A row core is an obsolete concept. Currently,
 * it exactly equals to the row (offset always equals to zero).
 */
class U2CORE_EXPORT MultipleSequenceAlignmentRowData : public MultipleAlignmentRowData {
    friend class MultipleSequenceAlignmentData;

    /** Do NOT create a row without an alignment! */
    MultipleSequenceAlignmentRowData();
    MultipleSequenceAlignmentRowData(const MultipleSequenceAlignmentData *msa);

    /** Creates a row in memory. */
    MultipleSequenceAlignmentRowData(const U2MaRow &rowInDb, const DNASequence &sequence, const U2MaRowGapModel &gaps, const MultipleSequenceAlignmentData *msa);
    MultipleSequenceAlignmentRowData(const U2MaRow &rowInDb, const QString &rowName, const QByteArray &rawData, const MultipleSequenceAlignmentData *msa);
    MultipleSequenceAlignmentRowData(const MultipleSequenceAlignmentRow &row, const MultipleSequenceAlignmentData *msa);

public:
    /** Name of the row (equals to the sequence name), can be empty */
    QString getName() const;
    void setName(const QString &name);

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

    MultipleAlignmentRowData * clone() const;
    MultipleSequenceAlignmentRowData * explicitClone() const;

private:
    int getDataLength() const;
    void appendDataCore(const MultipleAlignmentRow &anotherRow);
    void removeDataCore(int startPosInData, int endPosInData, U2OpStatus &os);
    bool isDataEqual(const MultipleAlignmentRowData &rowData) const;

    /**
     * Joins sequence chars and gaps into one byte array.
     * "keepOffset" specifies to take into account gaps at the beginning of the row.
     */
    QByteArray joinCharsAndGaps(bool keepOffset, bool keepTrailingGaps) const;

    /** The sequence of the row without gaps (cached) */
    DNASequence         sequence;
};

}   // namespace U2

#endif // _U2_MULTIPLE_SEQUENCE_ALIGNMENT_ROW_H_
