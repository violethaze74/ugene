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

#include "MultipleChromatogramAlignmentRow.h"

namespace U2 {

class MultipleChromatogramAlignmentData;

class U2CORE_EXPORT MultipleChromatogramAlignment {
public:
    MultipleChromatogramAlignment();
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
    template <class Derived> inline Derived dynamicCast() const;

private:
    QSharedPointer<MultipleChromatogramAlignmentData> mcaData;
};

template <class Derived>
Derived MultipleChromatogramAlignment::dynamicCast() const {
    return Derived(*this);
}

class U2CORE_EXPORT MultipleChromatogramAlignmentData {
    friend class MultipleChromatogramAlignment;

private:
    MultipleChromatogramAlignmentData();
    MultipleChromatogramAlignmentData(const QString &name,
                                      const DNAAlphabet *alphabet = NULL,
                                      const QList<MultipleChromatogramAlignmentRow> &rows = QList<MultipleChromatogramAlignmentRow>());
    MultipleChromatogramAlignmentData(const MultipleChromatogramAlignmentData &mcaData);

public:
    enum Order {
        Ascending,
        Descending
    };

    MultipleChromatogramAlignmentRow getRow(int rowIndex);
    const MultipleChromatogramAlignmentRow getRow(int rowIndex) const;
    const MultipleChromatogramAlignmentRow getRow(const QString &name) const;
    const QList<MultipleChromatogramAlignmentRow> & getRows() const;

    MultipleChromatogramAlignmentData & operator=(const MultipleChromatogramAlignment &msa);
    MultipleChromatogramAlignmentData & operator=(const MultipleChromatogramAlignmentData &msaData);

    void clear();
    bool isEmpty() const;
    bool isCommonGap(qint64 position, int rowNumber) const;

    QString getName() const;
    void setName(const QString &name);

    const DNAAlphabet * getAlphabet() const;
    void setAlphabet(const DNAAlphabet *alphabet);

    QVariantMap getInfo() const;
    void setInfo(const QVariantMap &info);

    qint64 getLength() const;
    void setLength(int length);

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

    void sortRowsByName(Order order = Ascending);

    /**
     * Sorts rows by similarity making identical rows sequential.
     * Returns 'true' if the rows were resorted, and 'false' otherwise.
     */
    bool sortRowsBySimilarity();

    QStringList getRowNames() const;

    /**
     * Inserts 'count' gaps into the specified position.
     * Can increase the overall alignment length.
     */
    void insertGaps(int row, qint64 pos, qint64 count, U2OpStatus &os);

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

    /**
     * Creates a new alignment from the sub-alignment. Do not trims the result.
     * Assumes that 'start' >= 0, and 'start + len' is less or equal than the alignment length.
     */
    MultipleChromatogramAlignment mid(qint64 start, qint64 len) const;

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

    bool operator==(const MultipleChromatogramAlignmentData &mcaData) const;
    bool operator!=(const MultipleChromatogramAlignmentData &mcaData) const;

    bool sortRowsByList(const QStringList &namesOrder);

    U2MsaListGapModel getPredictedSequencesGapModel() const;
    U2MsaListGapModel getEditedSequencesGapModel() const;
    void setRowGapModel(int rowNumber, const U2MsaRowGapModel &gapModel);
    void setGapModel(const U2MsaListGapModel &gapModel);

    void addRow(const DNAChromatogram &chromatogram,
                const DNASequence &predictedSequence,
                const U2MsaRowGapModel &gapModel);
    void addRow(const DNAChromatogram &chromatogram,
                const DNASequence &predictedSequence,
                const U2MsaRowGapModel &predictedSequenceGapModel,
                const DNASequence &editedSequence,
                const U2MsaRowGapModel &editedSequenceGapModel,
                const U2Region &workingArea = U2_REGION_MAX);
    void addRow(const QString &rowName,
                const DNAChromatogram &chromatogram,
                const QByteArray &predictedSequenceRawData);

    MultipleChromatogramAlignment getCopy() const;

private:
    void copy(const MultipleChromatogramAlignmentData &mcaData);
    void addRowPrivate(const MultipleChromatogramAlignmentRow &row);

    const DNAAlphabet *alphabet;
    QList<MultipleChromatogramAlignmentRow> rows;
    qint64 length;
    QVariantMap info;
};

inline bool	operator!=(const MultipleChromatogramAlignment &ptr1, const MultipleChromatogramAlignment &ptr2) { return *ptr1 != *ptr2; }
inline bool	operator!=(const MultipleChromatogramAlignment &ptr1, const MultipleChromatogramAlignmentData *ptr2) { return *ptr1 != *ptr2; }
inline bool	operator!=(const MultipleChromatogramAlignmentData *ptr1, const MultipleChromatogramAlignment &ptr2) { return *ptr1 != *ptr2; }
inline bool	operator==(const MultipleChromatogramAlignment &ptr1, const MultipleChromatogramAlignment &ptr2) { return *ptr1 == *ptr2; }
inline bool	operator==(const MultipleChromatogramAlignment &ptr1, const MultipleChromatogramAlignmentData *ptr2) { return *ptr1 == *ptr2; }
inline bool	operator==(const MultipleChromatogramAlignmentData *ptr1, const MultipleChromatogramAlignment &ptr2) { return *ptr1 == *ptr2; }

}   // namespace U2

#endif // _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_H_
