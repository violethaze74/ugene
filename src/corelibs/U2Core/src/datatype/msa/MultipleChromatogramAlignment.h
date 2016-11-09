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

#ifndef _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_H_
#define _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_H_

#include "MultipleAlignment.h"
#include "MultipleChromatogramAlignmentRow.h"

namespace U2 {

class McaRowMemoryData;
class MultipleChromatogramAlignmentData;

class U2CORE_EXPORT MultipleChromatogramAlignment : public MultipleAlignment {
public:
    MultipleChromatogramAlignment();
    MultipleChromatogramAlignment(const MultipleAlignment &ma);
    MultipleChromatogramAlignment(MultipleChromatogramAlignmentData *mcaData);
    MultipleChromatogramAlignment(const QString &name,
                                  const DNAAlphabet *alphabet = NULL,
                                  const QList<MultipleChromatogramAlignmentRow> &rows = QList<MultipleChromatogramAlignmentRow>());

    MultipleChromatogramAlignmentData * data() const;

    MultipleChromatogramAlignmentData & operator*();
    const MultipleChromatogramAlignmentData & operator*() const;

    MultipleChromatogramAlignmentData * operator->();
    const MultipleChromatogramAlignmentData * operator->() const;

    MultipleChromatogramAlignment clone() const;

private:
    QSharedPointer<MultipleChromatogramAlignmentData> getMcaData() const;
};

class U2CORE_EXPORT MultipleChromatogramAlignmentData : public MultipleAlignmentData {
    friend class MultipleChromatogramAlignment;

private:
    MultipleChromatogramAlignmentData(const QString &name = QString(),
                                      const DNAAlphabet *alphabet = NULL,
                                      const QList<MultipleChromatogramAlignmentRow> &rows = QList<MultipleChromatogramAlignmentRow>());
    MultipleChromatogramAlignmentData(const MultipleChromatogramAlignmentData &mcaData);

public:
    MultipleChromatogramAlignmentRow getMcaRow(int rowIndex);
    const MultipleChromatogramAlignmentRow getMcaRow(int rowIndex) const;
    const MultipleChromatogramAlignmentRow getMcaRow(const QString &name) const;
    const QList<MultipleChromatogramAlignmentRow> getMcaRows() const;
    MultipleChromatogramAlignmentRow getMcaRowByRowId(qint64 rowId, U2OpStatus &os) const;

    MultipleChromatogramAlignmentData & operator=(const MultipleChromatogramAlignment &mca);
    MultipleChromatogramAlignmentData & operator=(const MultipleChromatogramAlignmentData &mcaData);

    bool isCommonGap(qint64 position, int rowNumber) const;

    /**
     * Recomputes the length of the alignment and makes it as minimal
     * as possible. All leading gaps columns are removed by default.
     * Returns "true" if the alignment has been modified.
     */
    bool trim(bool removeLeadingGaps = true);

    /**
     * Inserts 'count' gaps into the specified position.
     * Can increase the overall alignment length.
     */
    void insertGaps(int row, qint64 pos, qint64 count, U2OpStatus &os);

    /**
     * Modifies the alignment by keeping data from the specified region and rows only.
     * Assumes that the region start is not negative, but it can be greater than a row length.
     */
    bool crop(const U2Region &region, const QSet<QString> &rowNames, U2OpStatus &os);
    bool crop(const U2Region &region, U2OpStatus &os);
    bool crop(qint64 start, qint64 count, U2OpStatus &os);

    /**
     * Creates a new alignment from the sub-alignment. Do not trims the result.
     * Assumes that 'start' >= 0, and 'start + len' is less or equal than the alignment length.
     */
    MultipleChromatogramAlignment mid(qint64 start, qint64 len) const;

    bool operator==(const MultipleChromatogramAlignmentData &mcaData) const;
    bool operator!=(const MultipleChromatogramAlignmentData &mcaData) const;

    bool sortRowsByList(const QStringList &namesOrder);

    U2MsaListGapModel getPredictedSequencesGapModel() const;
    U2MsaListGapModel getEditedSequencesGapModel() const;

    void setRowContent(U2OpStatus &os, int rowNumber, const McaRowMemoryData &mcaRowMemoryData);
    void setRowGapModel(int rowNumber, const U2MsaRowGapModel &gapModel);
    void setGapModel(const U2MsaListGapModel &gapModel);

    void addRow(const DNAChromatogram &chromatogram,
                const DNASequence &predictedSequence,
                const U2MsaRowGapModel &gapModel);
    void addRow(const U2McaRow &rowInDb,
                const DNAChromatogram &chromatogram,
                const DNASequence &predictedSequence,
                const U2MsaRowGapModel &predictedSequenceGapModel,
                const DNASequence &editedSequence,
                const U2MsaRowGapModel &editedSequenceGapModel,
                const U2Region &workingArea = U2_REGION_MAX);
    void addRow(const QString &rowName,
                const DNAChromatogram &chromatogram,
                const QByteArray &predictedSequenceRawData);

    MultipleAlignment getCopy() const;
    MultipleChromatogramAlignment getExplicitCopy() const;

private:
    MultipleAlignmentRow getEmptyRow() const;

    void copy(const MultipleAlignmentData &maData);
    void copy(const MultipleChromatogramAlignmentData &mcaData);

    void addRowPrivate(const MultipleChromatogramAlignmentRow &row);

    void setRows(const QList<MultipleChromatogramAlignmentRow> &mcaRows);

    U2MsaListGapModel getGuaranteedGaps() const;
};

inline bool	operator!=(const MultipleChromatogramAlignment &ptr1, const MultipleChromatogramAlignment &ptr2) { return *ptr1 != *ptr2; }
inline bool	operator!=(const MultipleChromatogramAlignment &ptr1, const MultipleChromatogramAlignmentData *ptr2) { return *ptr1 != *ptr2; }
inline bool	operator!=(const MultipleChromatogramAlignmentData *ptr1, const MultipleChromatogramAlignment &ptr2) { return *ptr1 != *ptr2; }
inline bool	operator==(const MultipleChromatogramAlignment &ptr1, const MultipleChromatogramAlignment &ptr2) { return *ptr1 == *ptr2; }
inline bool	operator==(const MultipleChromatogramAlignment &ptr1, const MultipleChromatogramAlignmentData *ptr2) { return *ptr1 == *ptr2; }
inline bool	operator==(const MultipleChromatogramAlignmentData *ptr1, const MultipleChromatogramAlignment &ptr2) { return *ptr1 == *ptr2; }

}   // namespace U2

#endif // _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_H_
