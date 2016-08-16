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

//#ifndef _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_H_
//#define _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_H_

//#include "MultipleAlignment.h"
//#include "MultipleChromatogramAlignmentRow.h"

//namespace U2 {

//class MultipleChromatogramAlignmentData;
//typedef QSharedPointer<MultipleChromatogramAlignmentData> MultipleChromatogramAlignment;

//class U2CORE_EXPORT MultipleChromatogramAlignmentData : public MultipleAlignmentData {
//private:
//    /**
//     * Creates a new alignment.
//     * The name must be provided if this is not default alignment.
//     */
//    MultipleChromatogramAlignmentData(const QString &name = QString(),
//                                  const DNAAlphabet *alphabet = NULL,
//                                  const QList<MultipleChromatogramAlignmentRow> &rows = QList<MultipleChromatogramAlignmentRow>());
//    MultipleChromatogramAlignmentData(const MultipleChromatogramAlignmentData &mca);

//public:
//    static MultipleChromatogramAlignment createMca(const QString &name = QString(),
//                                               const DNAAlphabet *alphabet = NULL,
//                                               const QList<MultipleChromatogramAlignmentRow> &rows = QList<MultipleChromatogramAlignmentRow>());

//    /** Returns a character (a gap or a non-gap) in the specified row and position */
//    char charAt(int rowIndex, int pos) const;

//    /**
//     * Sets the new content for the row with the specified index.
//     * Assumes that the row index is valid.
//     * Can modify the overall alignment length (increase or decrease).
//     */
//    void setRowContent(int row, const QByteArray& rawData, int offset = 0);

//    /** Converts all rows' sequences to upper case */
//    void toUpperCase();

//    /**
//     * Creates a new alignment from the sub-alignment. Do not trims the result.
//     * Assumes that 'start' >= 0, and 'start + len' is less or equal than the alignment length.
//     */
//    MultipleChromatogramAlignment mid(int start, int len) const;

//    /**
//     * Adds a new row to the alignment.
//     * If rowIndex == -1 -> appends the row to the alignment.
//     * Otherwise, if rowIndex is incorrect, the closer bound is used (the first or the last row).
//     * Does not trim the original alignment.
//     * Can increase the overall alignment length.
//     */
//    void addRow(const QString &name, const QByteArray &bytes);
//    void addRow(const QString &name, const QByteArray &bytes, int rowIndex);
//    void addRow(const U2MaRow &rowInDb, const DNASequence &sequence, U2OpStatus &os);
//    void addRow(const QString &name, const DNASequence &sequence, const U2MaRowGapModel &gaps, U2OpStatus &os);

//    /**
//     * Replaces all occurrences of 'origChar' by 'resultChar' in the row with the specified index.
//     * The 'origChar' must be a non-gap character.
//     * The 'resultChar' can be a gap, gaps model is recalculated in this case.
//     * The index must be valid as well.
//     */
//    void replaceChars(int row, char origChar, char resultChar);

//    /**
//     * Appends chars to the row with the specified index.
//     * The chars are appended to the alignment end, not to the row end
//     * (i.e. the alignment length is taken into account).
//     * Does NOT recalculate the alignment length!
//     * The index must be valid.
//     */
//    void appendChars(int row, const char* str, int len);
//    void appendChars(int row, int afterPos, const char *str, int len);

//    MultipleChromatogramAlignmentRow getMcaRow(int i);
//    const MultipleChromatogramAlignmentRow getMcaRow(int i) const;
//    const MultipleChromatogramAlignmentRow getMcaRow(const QString &name) const;
//    QList<MultipleChromatogramAlignmentRow> getMcaRows() const;
//    const MultipleChromatogramAlignmentRow getMcaRowByRowId(qint64 rowId, U2OpStatus &os) const;

//    MultipleAlignment getCopy() const;
//    MultipleChromatogramAlignment getExplicitCopy() const;

//    static MultipleChromatogramAlignment getEmptyMca();
//    static MultipleChromatogramAlignmentRow getEmptyRow();

//private:
//    static const MultipleChromatogramAlignment EMPTY_MCA;
//    static const MultipleChromatogramAlignmentRow EMPTY_ROW;

//    /** Create a new row (sequence + gap model) from the bytes */
//    MultipleChromatogramAlignmentRow createSequenceRow(const QString &name, const QByteArray &rawData) const;

//    /**
//     * Sequence must not contain gaps.
//     * All gaps in the gaps model (in 'rowInDb') must be valid and have an offset within the bound of the sequence.
//     */
//    MultipleChromatogramAlignmentRow createSequenceRow(const U2MaRow &rowInDb, const DNASequence &sequence, const U2MaRowGapModel &gaps, U2OpStatus &os);
//    MultipleAlignmentRow createRow(const MultipleAlignmentRow &row) const;

//    DNASequence referenceSequence;
//};

//}   // namespace U2

//#endif // _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_H_
