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

#ifndef _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_ROW_H_
#define _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_ROW_H_

#include <U2Core/DNAChromatogram.h>
#include <U2Core/DNASequence.h>
#include <U2Core/U2Mca.h>

#include "MultipleSequenceAlignmentRow.h"

namespace U2 {

class MultipleChromatogramAlignmentData;
class MultipleChromatogramAlignmentRowData;

class U2CORE_EXPORT MultipleChromatogramAlignmentRow : public MultipleSequenceAlignmentRow {
public:
    MultipleChromatogramAlignmentRow();
    MultipleChromatogramAlignmentRow(MultipleChromatogramAlignmentData *mcaData);
    MultipleChromatogramAlignmentRow(MultipleChromatogramAlignmentRowData *mcaRowData);

    /** Creates a row in memory. */
    MultipleChromatogramAlignmentRow(const U2MsaRow &rowInDb,
                                     const DNAChromatogram chromatogram,
                                     const DNASequence &predictedSequence,
                                     const DNASequence &editableSequence,
                                     const U2MsaRowGapModel &gaps,
                                     MultipleChromatogramAlignmentData *mcaData);
    MultipleChromatogramAlignmentRow(const U2MsaRow &rowInDb,
                                     const QString &rowName,
                                     const DNAChromatogram chromatogram,
                                     const DNASequence &predictedSequence,
                                     const QByteArray &rawData,
                                     MultipleChromatogramAlignmentData *mcaData);
    MultipleChromatogramAlignmentRow(const MultipleChromatogramAlignmentRow &row, MultipleChromatogramAlignmentData *mcaData);

    MultipleChromatogramAlignmentRowData * data() const;

    MultipleChromatogramAlignmentRowData & operator*();
    const MultipleChromatogramAlignmentRowData & operator*() const;

    MultipleChromatogramAlignmentRowData * operator->();
    const MultipleChromatogramAlignmentRowData * operator->() const;

    MultipleChromatogramAlignmentRow clone() const;

private:
    QSharedPointer<MultipleChromatogramAlignmentRowData> getMcaRowData() const;
};

class MultipleChromatogramAlignmentRowData : public MultipleSequenceAlignmentRowData {
    friend class MultipleChromatogramAlignmentData;
    friend class MultipleChromatogramAlignmentRow;

    /** Do NOT create a row without an alignment! */
    MultipleChromatogramAlignmentRowData();
    MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentData *mcaData);

    /** Creates a row in memory. */
    MultipleChromatogramAlignmentRowData(const U2MsaRow &rowInDb,
                                         const DNAChromatogram &chromatogram,
                                         const DNASequence &predictedSequence,
                                         const DNASequence &editableSequence,
                                         const U2MsaRowGapModel &gaps,
                                         const MultipleChromatogramAlignmentData *mcaData);
    MultipleChromatogramAlignmentRowData(const U2MsaRow &rowInDb,
                                         const QString &rowName,
                                         const DNAChromatogram &chromatogram,
                                         const DNASequence &predictedSequence,
                                         const QByteArray &rawData,
                                         const MultipleChromatogramAlignmentData *mcaData);
    MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentRow &row, const MultipleChromatogramAlignmentData *mcaData);

public:
    enum SequenceType {
        Predicted,
        Editable
    };

    /** Returns the row sequence (without gaps) */
    const DNAChromatogram & getChromatogram() const;
    const DNASequence & getSequence(SequenceType sequenceType) const;

    /**
     * Sets a new sequence. Be careful, gap model validity is not verified.
     * The sequence must not contain gaps.
     */
    void setChromatogram(const DNAChromatogram &chromatogram);
    void setSequence(const DNASequence &sequence);
    void setSequence(SequenceType sequenceType, const DNASequence &sequence);

    /**
     * The length must be greater or equal to the row length.
     * When the specified length is greater, an appropriate number of
     * trailing gaps are appended to the end of the byte array.
     */
    QByteArray toByteArray(SequenceType sequenceType, int length, U2OpStatus &os) const;

    /** Packed version: returns the row without leading and trailing gaps */
    QByteArray getCore(SequenceType sequenceType) const;

    /** Returns the row the way it is -- with leading and trailing gaps */
    QByteArray getData(SequenceType sequenceType) const;

    /** Adds anotherRow data to this row(ingores trailing gaps), "lengthBefore" must be greater than this row's length. */
    void append(const MultipleSequenceAlignmentRow &anotherRow, int lengthBefore, U2OpStatus &os);
    void append(const MultipleSequenceAlignmentRowData &anotherRow, int lengthBefore, U2OpStatus &os);
    void append(const MultipleChromatogramAlignmentRow &anotherRow, int lengthBefore, U2OpStatus &os);
    void append(const MultipleChromatogramAlignmentRowData &anotherRow, int lengthBefore, U2OpStatus &os);

    void setRowContent(const QByteArray &bytes, int offset, U2OpStatus &os);

    void removeChars(int pos, int count, U2OpStatus& os);

    /**
     * Returns a character in row at the specified position.
     * If the specified position is outside the row bounds, returns a gap.
     */
    char charAt(int pos) const;
    char charAt(SequenceType sequenceType, int pos) const;
    ushort traceValueAt(DNAChromatogram::Trace trace, int pos) const;

    bool isRowContentEqual(const MultipleSequenceAlignmentRow &row) const;
    bool isRowContentEqual(const MultipleSequenceAlignmentRowData &rowData) const;
    bool isRowContentEqual(const MultipleChromatogramAlignmentRow &row) const;
    bool isRowContentEqual(const MultipleChromatogramAlignmentRowData &rowData) const;

    inline bool operator!=(const MultipleSequenceAlignmentRowData &rowData) const;
    inline bool operator!=(const MultipleSequenceAlignmentRowData &rowData) const;
    bool operator==(const MultipleChromatogramAlignmentRowData &rowData) const;
    bool operator==(const MultipleChromatogramAlignmentRowData &rowData) const;

    void crop(int pos, int count, U2OpStatus &os);

    /**
     * Returns new row of the specified 'count' length, started from 'pos'.
     * 'pos' and 'pos + count' can be greater than the row length.
     * Keeps trailing gaps.
     */
    MultipleChromatogramAlignmentRow mid(int pos, int count, U2OpStatus &os) const;

    /** Converts the row sequence to upper case */
    void toUpperCase();

    /**
     * Replaces all occurrences of 'origChar' by 'resultChar'.
     * The 'origChar' must be a non-gap character.
     * The 'resultChar' can be a gap, gaps model is recalculated in this case.
     */
    void replaceChars(char origChar, char resultChar, U2OpStatus &os);
    void replaceChars(SequenceType sequenceType, char origChar, char resultChar, U2OpStatus &os);

    MultipleChromatogramAlignmentRow getCopy() const;

private:
    QByteArray predictedSequenceToByteArray(int length, U2OpStatus &os) const;

    DNAChromatogram chromatogram;
    DNASequence predictedSequence;
};

}   // namespace U2

#endif // _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_ROW_H_
