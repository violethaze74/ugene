///**
// * UGENE - Integrated Bioinformatics Tools.
// * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
// * http://ugene.unipro.ru
// *
// * This program is free software; you can redistribute it and/or
// * modify it under the terms of the GNU General Public License
// * as published by the Free Software Foundation; either version 2
// * of the License, or (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program; if not, write to the Free Software
// * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// * MA 02110-1301, USA.
// */

//#ifndef _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_ROW_H_
//#define _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_ROW_H_

//#include <U2Core/DNAChromatogram.h>
//#include <U2Core/DNASequence.h>

//#include "MultipleAlignmentRow.h"

//namespace U2 {

//class MultipleChromatogramAlignmentData;
//class MultipleChromatogramAlignmentRowData;

//typedef QSharedPointer<MultipleChromatogramAlignmentData> MultipleChromatogramAlignment;
//typedef QSharedPointer<MultipleChromatogramAlignmentRowData> MultipleChromatogramAlignmentRow;

//class MultipleChromatogramAlignmentRowData : public MultipleAlignmentRowData {
//    friend class MultipleChromatogramAlignmentData;

//    /** Do NOT create a row without an alignment! */
//    MultipleChromatogramAlignmentRowData();
//    MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentData *mca);

//    /** Creates a row in memory. */
//    MultipleChromatogramAlignmentRowData(const U2MaRow &rowInDb, const DNAChromatogram &chromatogram, const U2MaRowGapModel &gaps, const MultipleChromatogramAlignmentData *mca);
//    MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentRow &row, const MultipleChromatogramAlignmentData *mca);

//public:
//    enum Trace {
//        Trace_A,
//        Trace_C,
//        Trace_G,
//        Trace_T,
//    };

//    enum SequenceType {
//        Predicted,
//        Edited
//    };

//    /** Name of the row (equals to the sequence name), can be empty */
//    QString getName() const;
//    void setName(const QString &name);

//    /** Returns the row sequence (without gaps) */
//    const DNAChromatogram & getChromatogram() const;
//    const DNASequence & getSequence(SequenceType sequenceType) const;

//    /**
//     * Sets a new sequence. Be careful, gap model validity is not verified.
//     * The sequence must not contain gaps.
//     */
//    void setChromatogram(const DNAChromatogram &newChromatogram);
//    void setSequence(SequenceType sequenceType, const U2::DNASequence &newSequence);

//    /**
//     * The length must be greater or equal to the row length.
//     * When the specified length is greater, an appropriate number of
//     * trailing gaps are appended to the end of the byte array.
//     */
//    QByteArray toByteArray(SequenceType sequenceType, int length, U2OpStatus &os) const;

//    /** Packed version: returns the row without leading and trailing gaps */
//    QByteArray getCore(SequenceType sequenceType) const;

//    /** Returns the row the way it is -- with leading and trailing gaps */
//    QByteArray getData(SequenceType sequenceType) const;

//    /**
//     * Returns a character in row at the specified position.
//     * If the specified position is outside the row bounds, returns a gap.
//     */
//    char charAt(SequenceType sequenceType, int pos) const;
//    ushort traceValueAt(Trace track, int pos) const;

//    /** Converts the row sequence to upper case */
//    void toUpperCase();

//    /**
//     * Replaces all occurrences of 'origChar' by 'resultChar'.
//     * The 'origChar' must be a non-gap character.
//     * The 'resultChar' can be a gap, gaps model is recalculated in this case.
//     */
//    void replaceChars(char origChar, char resultChar, U2OpStatus &os);
//    void replaceChars(SequenceType sequenceType, char origChar, char resultChar, U2OpStatus &os);

//    /**
//     * Returns new row of the specified 'count' length, started from 'pos'.
//     * 'pos' and 'pos + count' can be greater than the row length.
//     * Keeps trailing gaps.
//     */
//    MultipleChromatogramAlignmentRow mid(int pos, int count, U2OpStatus &os) const;

//    MultipleChromatogramAlignmentRow getCopy() const;

//private:
//    int getDataLength() const;
//    void appendDataCore(const MultipleAlignmentRow &anotherRow);
//    void removeDataCore(int startPosInData, int endPosInData, U2OpStatus &os);
//    bool isDataEqual(const MultipleAlignmentRowData &rowData) const;

//    /**
//     * Joins sequence chars and gaps into one byte array.
//     * "keepOffset" specifies to take into account gaps at the beginning of the row.
//     */
//    QByteArray joinCharsAndGaps(SequenceType sequenceType, bool keepLeadingGaps, bool keepTrailingGaps) const;

//    DNASequence * selectSequence(SequenceType sequenceType);
//    const DNASequence * selectSequence(SequenceType sequenceType) const;

//    /** The sequence of the row without gaps (cached) */
//    QString name;
//    DNAChromatogram chromatogram;
//    DNASequence predictedSequence;
//    DNASequence editedSequence;
//};

//}   // namespace U2

//#endif // _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_ROW_H_
