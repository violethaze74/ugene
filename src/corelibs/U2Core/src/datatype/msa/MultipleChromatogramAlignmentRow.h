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
#include <U2Core/McaRowInnerData.h>
#include <U2Core/U2Mca.h>
#include <U2Core/U2Region.h>

#include "MultipleAlignmentRow.h"

namespace U2 {

class MultipleChromatogramAlignment;
class MultipleChromatogramAlignmentData;
class MultipleChromatogramAlignmentRowData;

class U2CORE_EXPORT MultipleChromatogramAlignmentRow : public MultipleAlignmentRow {
public:
    MultipleChromatogramAlignmentRow();
    MultipleChromatogramAlignmentRow(const MultipleAlignmentRow &maRow);
    MultipleChromatogramAlignmentRow(const MultipleChromatogramAlignmentData *mcaData);
    MultipleChromatogramAlignmentRow(MultipleChromatogramAlignmentRowData *mcaRowData);

    /** Creates a row in memory. */
    MultipleChromatogramAlignmentRow(const MultipleChromatogramAlignmentData *mcaData,
                                     const U2McaRow &rowInDb,
                                     const DNAChromatogram &chromatogram,
                                     const DNASequence &predictedSequence,
                                     const U2MsaRowGapModel &gapModel);
    MultipleChromatogramAlignmentRow(const MultipleChromatogramAlignmentData *mcaData,
                                     const U2McaRow &rowInDb,
                                     const DNAChromatogram &chromatogram,
                                     const DNASequence &predictedSequence,
                                     const U2MsaRowGapModel &predictedSequenceGapModel,
                                     const DNASequence &editedSequence,
                                     const U2MsaRowGapModel &editedSequenceGapModel,
                                     const U2Region &workingArea = U2_REGION_MAX);
    MultipleChromatogramAlignmentRow(const MultipleChromatogramAlignmentData *mcaData,
                                     const U2McaRow &rowInDb,
                                     const QString &rowName,
                                     const DNAChromatogram &chromatogram,
                                     const QByteArray &predictedSequenceRawData);
    MultipleChromatogramAlignmentRow(const MultipleChromatogramAlignmentData *mcaData,
                                     const MultipleChromatogramAlignmentRowData &mcaRowData);

    MultipleChromatogramAlignmentRowData * data() const;

    MultipleChromatogramAlignmentRowData & operator*();
    const MultipleChromatogramAlignmentRowData & operator*() const;

    MultipleChromatogramAlignmentRowData * operator->();
    const MultipleChromatogramAlignmentRowData * operator->() const;

    MultipleChromatogramAlignmentRow clone() const;

private:
    QSharedPointer<MultipleChromatogramAlignmentRowData> getMcaRowData() const;
};

// TODO: write a correct class description
// Working area - a part of the core which is shown. It can be expanded to the core size or reduced to some little value (0 or 1 symbol?), it starts from the core start
// Core - a sequence + gaps whithin it (without leading and trailing gaps)
// Guaranteed gaps


/**
 * The MultipleChromatogramAlignmentRowData class
 * The row name is defined by the edited sequence name
 * There are the next parts of the row:
 *   commonGapModel - a gap model that contains gaps which present both in predicted and edited sequence
 *   predictedSequenceGapModelDifference - a gap model that contains gaps which present in the predicted sequence only.
 *                                         These gaps coordinates don't take into account common gaps.
 *   editedSequenceGapModelDifference - a gap model that contains gaps which present in the edited sequence only.
 *                                         These gaps coordinates don't take into account common gaps.
 *   chromatogram - a chromatogram of the row. It shares a gap model as the predicted sequence.
 *   predictedSequence - a sequence built from chromatogram. It can't be edited.
 *   editedSequence - a sequence copied from predictedSequence. It can be edited by user.
 *
 * There are the next regions in the row:
 *   Core - a region without leading and trailing common gaps.
 *   Data - the row part without common gaps. It includes sequence bytes and difference gap model.
 *   Sequence data - the row part without any gaps. It includes sequence bytes only.
 *   Working area - a region within the core region. It is supposed, that the data outside the working area is not shown.
 *
 * Examples (the end position is non-including):
 * -------AAA-----AAAA-----AAA---
 * -----AAAAAA-----AAAA---AA-----
 *      ^                     ^
 *      |  core region        |
 * commonGapModel: (0, 5), (11, 4), (20, 3), (27, 3)
 * predictedSequenceGapModelDifference: (0, 2), (5, 1), (10, 2)
 * predictedSequenceGapModelDifference: (6, 1), (13, 2)
 */
class MultipleChromatogramAlignmentRowData : public MultipleAlignmentRowData {
    friend class MultipleChromatogramAlignmentData;
    friend class MultipleChromatogramAlignmentRow;

private:
    MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentData *mcaData = NULL);
    MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentData *mcaData,
                                         const U2McaRow &rowInDb,
                                         const DNAChromatogram &chromatogram,
                                         const DNASequence &predictedSequence,
                                         const U2MsaRowGapModel &gapModel);
    MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentData *mcaData,
                                         const U2McaRow &rowInDb,
                                         const DNAChromatogram &chromatogram,
                                         const DNASequence &predictedSequence,
                                         const U2MsaRowGapModel &predictedSequenceGapModel,
                                         const DNASequence &editedSequence,
                                         const U2MsaRowGapModel &editedSequenceGapModel,
                                         const U2Region &workingArea = U2_REGION_MAX);
    MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentData *mcaData,
                                         const U2McaRow &rowInDb,
                                         const QString &rowName,
                                         const DNAChromatogram &chromatogram,
                                         const QByteArray &predictedSequenceRawData);
    MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentData *mcaData,
                                         const MultipleChromatogramAlignmentRowData &mcaRowData);

public:
    QString getName() const;     // name is defined by the edited sequence
    void setName(const QString &name);

    qint64 getRowId() const;
    void setRowId(qint64 rowId);

    U2McaRow getRowDbInfo() const;
    void setRowDbInfo(const U2McaRow &dbRow);

    const DNASequence & getPredictedSequence() const;
    const DNASequence & getEditedSequence() const;
    const DNAChromatogram & getChromatogram() const;
    McaRowMemoryData getRowMemoryData() const;

    const U2MsaRowGapModel & getPredictedSequenceGapModel() const;
    const U2MsaRowGapModel & getEditedSequenceGapModel() const;
    const U2MsaRowGapModel & getCommonGapModel() const;
    void setGapModel(const U2MsaRowGapModel &newGapModel);      // be careful, this method doesn't consider guaranteed gaps from other rows

    const U2MsaRowGapModel & getEditedSequenceDifferenceGapModel() const;

    QByteArray getPredictedSequenceData() const;
    QByteArray getPredictedSequenceCore() const;
    QByteArray getPredictedSequenceWorkingArea() const;

    QByteArray getEditedSequenceData() const;
    QByteArray getEditedSequenceCore() const;
    QByteArray getEditedSequenceWorkingArea() const;

    void setRowContent(U2OpStatus &os, const McaRowMemoryData &mcaRowMemoryData);
    void setParentAlignment(const MultipleChromatogramAlignment &mca);
    void setParentAlignment(MultipleChromatogramAlignmentData *mcaData);

    qint64 getPredictedSequenceLength() const;
    qint64 getEditedSequenceLength() const;
    qint64 getChromatogramLength() const;

    qint64 getCoreStart() const;    // a position of the first meaningful symbol (a position where leading gaps end)
    qint64 getCoreLength() const;    // from the first symbol to the last symbol, included gaps whithin, includes hided symbols (if there are gaps guaranteed by difference at the end, then it is included to the core)
    qint64 getWorkingAreaLength() const;    // from the first nonhided symbol to the last nonhided symbol, included gaps whithin
    qint64 getRowLength() const;    // returns an MCA length
    qint64 getRowLengthWithoutTrailing() const; // leading gaps + core length - a real length of the row without trailing gaps
    qint64 getBaseCount(qint64 before) const;

    U2Region getCoreRegion() const;
    U2Region getWorkingAreaRegion() const;

    QByteArray toByteArray(U2OpStatus &os, qint64 length) const;
    char charAt(qint64 position) const;

    char getPredictedSequenceWorkingAreaChar(qint64 position) const;
    char getPredictedSequenceChar(qint64 position) const;

    char getEditedSequenceWorkingAreaChar(qint64 position) const;
    char getEditedSequenceChar(qint64 position) const;

    ushort getChromatogramWorkingAreaValue(DNAChromatogram::Trace trace, qint64 workingAreaPosition) const;
    ushort getChromatogramValue(DNAChromatogram::Trace trace, qint64 position) const;

    void insertGap(U2OpStatus &os, qint64 position);
    void insertGaps(U2OpStatus &os, qint64 position, qint64 count);

    void removeGap(U2OpStatus &os, qint64 position);
    void removeGaps(U2OpStatus &os, qint64 position, qint64 count);

    void replaceCharInEditedSequence(U2OpStatus &os, qint64 position, char newChar);

    MultipleChromatogramAlignmentRow getExplicitCopy(const MultipleChromatogramAlignmentData *mcaData = NULL) const;

    bool operator ==(const MultipleChromatogramAlignmentRowData &mcaRowData) const;
    bool operator ==(const MultipleAlignmentRowData &maRowData) const;
    bool operator !=(const MultipleChromatogramAlignmentRowData &mcaRowData) const;
    bool operator !=(const MultipleAlignmentRowData &maRowData) const;

    void crop(U2OpStatus &os, qint64 position, qint64 count);
    MultipleChromatogramAlignmentRow mid(U2OpStatus &os, qint64 mcaVisiblePosition, qint64 count) const;

    bool isCommonGap(qint64 position) const;

private:
    void extractCommonGapModel(const U2MsaRowGapModel &predictedSequenceGapModel, const U2MsaRowGapModel &editedSequenceGapModel);

    void removeTrailingGaps();
    void updateCachedGapModels() const;
    qint64 getPredictedSequenceGuaranteedGapsLength() const;
    qint64 getEditedSequenceGuaranteedGapsLength() const;

    U2MsaRowGapModel getPredictedSequenceCoreGapModel() const;
    U2MsaRowGapModel getPredictedSequenceWorkingAreaGapModel() const;
    U2MsaRowGapModel getEditedSequenceCoreGapModel() const;
    U2MsaRowGapModel getEditedSequenceWorkingAreaGapModel() const;
    U2MsaRowGapModel getCommonCoreGapModel() const;

    char getPredictedSequenceCoreChar(qint64 corePosition) const;
    char getEditedSequenceCoreChar(qint64 corePosition) const;

    qint64 getPredictedSequenceDataLength() const;
    qint64 getEditedSequenceDataLength() const;

    void replaceGapToCharInEditedSequence(U2OpStatus &os, qint64 position, char newChar);
    void replaceCharToGapInEditedSequence(U2OpStatus &os, qint64 position);
    void replaceCharToCharInEditedSequence(U2OpStatus &os, qint64 position, char newChar);

    void copy(const MultipleChromatogramAlignmentRowData &mcaRowData);

    DNAChromatogram chromatogram;
    DNASequence predictedSequence;
    DNASequence editedSequence;
    U2MsaRowGapModel commonGapModel;
    mutable U2MsaRowGapModel predictedSequenceCachedGapModel;
    mutable U2MsaRowGapModel editedSequenceCachedGapModel;
    U2MsaRowGapModel predictedSequenceGapModelDifference;       // difference is a gap model inside a core, positions of gaps are in core-based coordinates
    U2MsaRowGapModel editedSequenceGapModelDifference;
    U2Region workingArea;
    U2McaRow initialRowInDb;
    const MultipleChromatogramAlignmentData *mcaData;
};

inline bool	operator!=(const MultipleChromatogramAlignmentRow &ptr1, const MultipleChromatogramAlignmentRow &ptr2) { return *ptr1 != *ptr2; }
inline bool	operator!=(const MultipleChromatogramAlignmentRow &ptr1, const MultipleChromatogramAlignmentRowData *ptr2) { return *ptr1 != *ptr2; }
inline bool	operator!=(const MultipleChromatogramAlignmentRowData *ptr1, const MultipleChromatogramAlignmentRow &ptr2) { return *ptr1 != *ptr2; }
inline bool	operator==(const MultipleChromatogramAlignmentRow &ptr1, const MultipleChromatogramAlignmentRow &ptr2) { return *ptr1 == *ptr2; }
inline bool	operator==(const MultipleChromatogramAlignmentRow &ptr1, const MultipleChromatogramAlignmentRowData *ptr2) { return *ptr1 == *ptr2; }
inline bool	operator==(const MultipleChromatogramAlignmentRowData *ptr1, const MultipleChromatogramAlignmentRow &ptr2) { return *ptr1 == *ptr2; }

}   // namespace U2

#endif // _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_ROW_H_
