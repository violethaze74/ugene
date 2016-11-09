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

#include <typeinfo>

#include <QSet>

#include <U2Core/MsaRowUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "MultipleChromatogramAlignment.h"
#include "MultipleAlignmentInfo.h"

namespace U2 {

MultipleChromatogramAlignment::MultipleChromatogramAlignment()
    : MultipleAlignment(new MultipleChromatogramAlignmentData())
{

}

MultipleChromatogramAlignment::MultipleChromatogramAlignment(const MultipleAlignment &ma)
    : MultipleAlignment(ma)
{
    SAFE_POINT(NULL != maData.dynamicCast<MultipleChromatogramAlignmentData>(), "Can't cast MultipleAlignment to MultipleChromatogramAlignment", );
}

MultipleChromatogramAlignment::MultipleChromatogramAlignment(MultipleChromatogramAlignmentData *mcaData)
    : MultipleAlignment(mcaData)
{

}

MultipleChromatogramAlignment::MultipleChromatogramAlignment(const QString &name, const DNAAlphabet *alphabet, const QList<MultipleChromatogramAlignmentRow> &rows)
    : MultipleAlignment(new MultipleChromatogramAlignmentData(name, alphabet, rows))
{

}

MultipleChromatogramAlignmentData * MultipleChromatogramAlignment::data() const {
    return getMcaData().data();
}

MultipleChromatogramAlignmentData & MultipleChromatogramAlignment::operator*() {
    return *getMcaData();
}

const MultipleChromatogramAlignmentData & MultipleChromatogramAlignment::operator*() const {
    return *getMcaData();
}

MultipleChromatogramAlignmentData * MultipleChromatogramAlignment::operator->() {
    return getMcaData().data();
}

const MultipleChromatogramAlignmentData * MultipleChromatogramAlignment::operator->() const {
    return getMcaData().data();
}

MultipleChromatogramAlignment MultipleChromatogramAlignment::clone() const {
    return getMcaData()->getCopy();
}

QSharedPointer<MultipleChromatogramAlignmentData> MultipleChromatogramAlignment::getMcaData() const {
    return maData.dynamicCast<MultipleChromatogramAlignmentData>();
}

namespace {

QList<MultipleAlignmentRow> convertToMaRows(const QList<MultipleChromatogramAlignmentRow> &mcaRows) {
    QList<MultipleAlignmentRow> maRows;
    foreach (const MultipleChromatogramAlignmentRow &mcaRow, mcaRows) {
        maRows << mcaRow;
    }
    return maRows;
}

}

MultipleChromatogramAlignmentData::MultipleChromatogramAlignmentData(const QString &name, const DNAAlphabet *alphabet, const QList<MultipleChromatogramAlignmentRow> &rows)
    : MultipleAlignmentData(name, alphabet, convertToMaRows(rows))
{

}

MultipleChromatogramAlignmentData::MultipleChromatogramAlignmentData(const MultipleChromatogramAlignmentData &mcaData)
    : MultipleAlignmentData()
{
    copy(mcaData);
}

MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::getMcaRow(int rowIndex) {
    return getRow(rowIndex).dynamicCast<MultipleChromatogramAlignmentRow>();
}

const MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::getMcaRow(int rowIndex) const {
    return getRow(rowIndex).dynamicCast<MultipleChromatogramAlignmentRow>();
}

const MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::getMcaRow(const QString &name) const {
    return getRow(name).dynamicCast<MultipleChromatogramAlignmentRow>();
}

const QList<MultipleChromatogramAlignmentRow> MultipleChromatogramAlignmentData::getMcaRows() const {
    QList<MultipleChromatogramAlignmentRow> mcaRows;
    foreach (const MultipleAlignmentRow &maRow, getRows()) {
        mcaRows << maRow.dynamicCast<MultipleChromatogramAlignmentRow>();
    }
    return mcaRows;
}

MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::getMcaRowByRowId(qint64 rowId, U2OpStatus &os) const {
    return getRowByRowId(rowId, os).dynamicCast<MultipleChromatogramAlignmentRow>(os);
}

MultipleChromatogramAlignmentData & MultipleChromatogramAlignmentData::operator=(const MultipleChromatogramAlignment &mca) {
    return *this = *mca;
}

MultipleChromatogramAlignmentData & MultipleChromatogramAlignmentData::operator=(const MultipleChromatogramAlignmentData &mcaData) {
    copy(mcaData);
    return *this;
}

bool MultipleChromatogramAlignmentData::isCommonGap(qint64 position, int rowNumber) const {
    SAFE_POINT(0 <= position && position < getLength(), "Position is out of boundaries", true);
    SAFE_POINT(0 <= rowNumber && rowNumber < getNumRows(), "Row number is out of boundaries", true);
    return getMcaRow(rowNumber)->isCommonGap(position);
}

bool MultipleChromatogramAlignmentData::trim(bool removeLeadingGaps) {
    bool result = false;

    if (removeLeadingGaps) {
        // Verify if there are leading columns of gaps
        // by checking the first gap in each row
        qint64 leadingGapColumnsNum = 0;
        foreach (const MultipleChromatogramAlignmentRow &row, getMcaRows()) {
            if (row->getCommonGapModel().count() > 0) {
                const U2MsaGap firstGap = row->getCommonGapModel().first();
                if (firstGap.offset > 0) {
                    leadingGapColumnsNum = 0;
                    break;
                } else {
                    if (leadingGapColumnsNum == 0) {
                        leadingGapColumnsNum = firstGap.gap;
                    } else {
                        leadingGapColumnsNum = qMin(leadingGapColumnsNum, firstGap.gap);
                    }
                }
            } else {
                leadingGapColumnsNum = 0;
                break;
            }
        }

        // If there are leading gap columns, remove them
        U2OpStatus2Log os;
        if (leadingGapColumnsNum > 0) {
            foreach (MultipleChromatogramAlignmentRow mcaRow, getMcaRows()) {
                mcaRow->removeGaps(os, 0, leadingGapColumnsNum);
                CHECK_OP(os, true);
                result = true;
            }
        }
    }

    // Verify right side of the alignment (trailing gaps and rows' lengths)
    qint64 newLength = 0;
    foreach (const MultipleChromatogramAlignmentRow &row, getMcaRows()) {
        if (newLength == 0) {
            newLength = row->getRowLengthWithoutTrailing();
        } else {
            newLength = qMax(row->getRowLengthWithoutTrailing(), newLength);
        }
    }

    if (newLength != getLength()) {
        length = newLength;
        result = true;
    }

    return result;
}

void MultipleChromatogramAlignmentData::insertGaps(int row, qint64 pos, qint64 count, U2OpStatus &os) {
    if (row >= getNumRows() || row < 0 || pos > length || pos < 0 || count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed "
            "to MultipleChromatogramAlignmentData::insertGaps: row index '%1', pos '%2', count '%3'").arg(row).arg(pos).arg(count));
        os.setError("Failed to insert gaps into an alignment");
        return;
    }

    if (pos == length) {
        // add trailing gaps --> just increase alignment len
        length += count;
        return;
    }

    if (pos >= rows[row]->getRowLengthWithoutTrailing()) {
        length += count;
        return;
    }
    getMcaRow(row)->insertGaps(os, pos, count);

    const qint64 rowLength = rows[row]->getRowLengthWithoutTrailing();
    length = qMax(length, rowLength);
}

bool MultipleChromatogramAlignmentData::crop(const U2Region &region, const QSet<QString> &rowNames, U2OpStatus &os) {
    // TODO: move to MultipleAlignmentData
    if (!(region.startPos >= 0 && region.length > 0 && region.length < length && region.startPos < length)) {
        os.setError(QString("Incorrect region was passed to MultipleChromatogramAlignmentData::crop, "
                            "startPos '%1', length '%2'").arg(region.startPos).arg(region.length));
        return false;
    }

    int cropLen = region.length;
    if (region.endPos() > length) {
        cropLen -= (region.endPos() - length);
    }

    QList<MultipleChromatogramAlignmentRow> newList;
    for (int i = 0 ; i < getNumRows(); i++) {
        MultipleChromatogramAlignmentRow row = getMcaRow(i).clone();
        const QString rowName = row->getName();
        if (rowNames.contains(rowName)) {
            row->crop(os, region.startPos, cropLen);
            CHECK_OP(os, false);
            newList << row;
        }
    }
    setRows(newList);

    length = cropLen;
    return true;
}

bool MultipleChromatogramAlignmentData::crop(const U2Region &region, U2OpStatus &os) {
    // TODO: move to MultipleAlignmentData
    return crop(region, getRowNames().toSet(), os);
}

bool MultipleChromatogramAlignmentData::crop(qint64 start, qint64 count, U2OpStatus &os) {
    // TODO: move to MultipleAlignmentData
    return crop(U2Region(start, count), os);
}

MultipleChromatogramAlignment MultipleChromatogramAlignmentData::mid(qint64 start, qint64 len) const {
    SAFE_POINT(start >= 0 && start + len <= length,
               QString("Incorrect parameters were passed to MultipleChromatogramAlignmentData::mid: "
                       "start '%1', len '%2', the alignment length is '%3'").arg(start).arg(len).arg(length),
               MultipleChromatogramAlignment());

    MultipleChromatogramAlignment res(getName(), alphabet);

    U2OpStatus2Log os;
    foreach (const MultipleChromatogramAlignmentRow &row, getMcaRows()) {
        MultipleChromatogramAlignmentRow midRow = row->mid(os, start, len);
        midRow->setParentAlignment(res);
        res->rows << midRow;
    }
    res->length = len;
    return res;
}

bool MultipleChromatogramAlignmentData::operator==(const MultipleChromatogramAlignmentData &mcaData) const {
    const bool lengthsAreEqual = (length == mcaData.length);
    const bool alphabetsAreEqual = (alphabet == mcaData.alphabet);
    const bool rowsAreEqual = (rows == mcaData.rows);
    return lengthsAreEqual && alphabetsAreEqual && rowsAreEqual;
}

bool MultipleChromatogramAlignmentData::operator!=(const MultipleChromatogramAlignmentData &mcaData) const {
    return !operator==(mcaData);
}

bool MultipleChromatogramAlignmentData::sortRowsByList(const QStringList &newNamesOrder) {
    const QStringList namesOrder = getRowNames();
    foreach (const QString &rowName, namesOrder) {
        CHECK(newNamesOrder.contains(rowName), false);
    }

    QList<MultipleChromatogramAlignmentRow> sortedRows;
    foreach (const QString &rowName, newNamesOrder) {
        int rowIndex = namesOrder.indexOf(rowName);
        if (rowIndex >= 0) {
            sortedRows << rows[rowIndex];
        }
    }

    setRows(sortedRows);
    return true;
}

U2MsaListGapModel MultipleChromatogramAlignmentData::getPredictedSequencesGapModel() const {
    U2MsaListGapModel gapModel;
    foreach (const MultipleChromatogramAlignmentRow &row, rows) {
        gapModel << row->getPredictedSequenceGapModel();
    }
    return gapModel;
}

U2MsaListGapModel MultipleChromatogramAlignmentData::getEditedSequencesGapModel() const {
    U2MsaListGapModel gapModel;
    foreach (const MultipleChromatogramAlignmentRow &row, rows) {
        gapModel << row->getEditedSequenceGapModel();
    }
    return gapModel;
}

void MultipleChromatogramAlignmentData::setRowContent(U2OpStatus &os, int rowNumber, const McaRowMemoryData &mcaRowMemoryData) {
    SAFE_POINT_EXT(0 <= rowNumber && rowNumber < getNumRows(), os.setError("Row number is out of boundaries"), );
    getMcaRow(rowNumber)->setRowContent(os, mcaRowMemoryData);
}

void MultipleChromatogramAlignmentData::setRowGapModel(int rowNumber, const U2MsaRowGapModel &gapModel) {
    getMcaRow(rowNumber)->setGapModel(gapModel);
}

void MultipleChromatogramAlignmentData::setGapModel(const U2MsaListGapModel &gapModel) {
    const U2MsaListGapModel guaranteedGaps = getGuaranteedGaps();
    for (int i = 0; i < getNumRows(); i++) {
        const U2MsaRowGapModel rowGapModel = MsaRowUtils::insertGapModel(gapModel[i], guaranteedGaps[i]);
        setRowGapModel(i, rowGapModel);
    }
}

void MultipleChromatogramAlignmentData::addRow(const DNAChromatogram &chromatogram, const DNASequence &predictedSequence, const U2MsaRowGapModel &gapModel) {
    const U2McaRow rowInDb;
    MultipleChromatogramAlignmentRow row(this, rowInDb, chromatogram, predictedSequence, gapModel);
    addRowPrivate(row);
}

void MultipleChromatogramAlignmentData::addRow(const U2McaRow &rowInDb,
                                               const DNAChromatogram &chromatogram,
                                               const DNASequence &predictedSequence,
                                               const U2MsaRowGapModel &predictedSequenceGapModel,
                                               const DNASequence &editedSequence,
                                               const U2MsaRowGapModel &editedSequenceGapModel,
                                               const U2Region &workingArea) {
    MultipleChromatogramAlignmentRow row(this, rowInDb, chromatogram, predictedSequence, predictedSequenceGapModel, editedSequence, editedSequenceGapModel, workingArea);
    addRowPrivate(row);
}

void MultipleChromatogramAlignmentData::addRow(const QString &rowName, const DNAChromatogram &chromatogram, const QByteArray &predictedSequenceRawData) {
    const U2McaRow rowInDb;
    MultipleChromatogramAlignmentRow row(this, rowInDb, rowName, chromatogram, predictedSequenceRawData);
    addRowPrivate(row);
}

MultipleAlignment MultipleChromatogramAlignmentData::getCopy() const {
    return getExplicitCopy();
}

MultipleChromatogramAlignment MultipleChromatogramAlignmentData::getExplicitCopy() const {
    return MultipleChromatogramAlignment(new MultipleChromatogramAlignmentData(*this));
}

MultipleAlignmentRow MultipleChromatogramAlignmentData::getEmptyRow() const {
    return MultipleChromatogramAlignmentRow();
}

void MultipleChromatogramAlignmentData::copy(const MultipleAlignmentData &maData) {
    try {
        copy(dynamic_cast<const MultipleChromatogramAlignmentData &>(maData));
    } catch (std::bad_cast) {
        FAIL("Can't cast MultipleAlignmentData to MultipleChromatogramAlignmentData", );
    }
}

void MultipleChromatogramAlignmentData::copy(const MultipleChromatogramAlignmentData &mcaData) {
    clear();

    alphabet = mcaData.alphabet;
    length = mcaData.length;
    info = mcaData.info;

    for (int i = 0; i < mcaData.rows.size(); i++) {
        const MultipleChromatogramAlignmentRow row = mcaData.getMcaRow(i)->getExplicitCopy(this);
        addRowPrivate(row);
    }
}

void MultipleChromatogramAlignmentData::addRowPrivate(const MultipleChromatogramAlignmentRow &row) {
    length = qMax(length, row->getRowLengthWithoutTrailing());
    rows << row;
}

void MultipleChromatogramAlignmentData::setRows(const QList<MultipleChromatogramAlignmentRow> &mcaRows) {
    rows = convertToMaRows(mcaRows);
}

U2MsaListGapModel MultipleChromatogramAlignmentData::getGuaranteedGaps() const {
    U2MsaListGapModel editedSequencesDifferenceGapModels;
    foreach (const MultipleChromatogramAlignmentRow &row, getMcaRows()) {
        editedSequencesDifferenceGapModels << row->getEditedSequenceDifferenceGapModel();
    }

    const U2MsaRowGapModel mergedDifferencesGapModel = MsaRowUtils::mergeGapModels(editedSequencesDifferenceGapModels);

    U2MsaListGapModel guaranteedGapModels;
    foreach (const MultipleChromatogramAlignmentRow &row, getMcaRows()) {
        guaranteedGapModels << MsaRowUtils::subtitudeGapModel(mergedDifferencesGapModel, row->getEditedSequenceDifferenceGapModel());
    }

    return guaranteedGapModels;
}

}   // namespace U2
