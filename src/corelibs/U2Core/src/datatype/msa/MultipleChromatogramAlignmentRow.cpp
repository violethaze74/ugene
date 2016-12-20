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

#include <U2Core/ChromatogramUtils.h>
#include <U2Core/DNASequenceUtils.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/MsaRowUtils.h>

#include "MultipleChromatogramAlignment.h"
#include "MultipleChromatogramAlignmentRow.h"

namespace U2 {

MultipleChromatogramAlignmentRow::MultipleChromatogramAlignmentRow()
    : MultipleAlignmentRow(new MultipleChromatogramAlignmentRowData)
{

}

MultipleChromatogramAlignmentRow::MultipleChromatogramAlignmentRow(const MultipleAlignmentRow &maRow)
    : MultipleAlignmentRow(maRow)
{
    SAFE_POINT(NULL != maRowData.dynamicCast<MultipleChromatogramAlignmentRowData>(), "Can't cast MultipleAlignmentRow to MultipleChromatogramAlignmentRow", );
}

MultipleChromatogramAlignmentRow::MultipleChromatogramAlignmentRow(const MultipleChromatogramAlignmentData *mcaData)
    : MultipleAlignmentRow(new MultipleChromatogramAlignmentRowData(mcaData))
{

}

MultipleChromatogramAlignmentRow::MultipleChromatogramAlignmentRow(MultipleChromatogramAlignmentRowData *mcaRowData)
    : MultipleAlignmentRow(mcaRowData)
{

}

MultipleChromatogramAlignmentRow::MultipleChromatogramAlignmentRow(const MultipleChromatogramAlignmentData *mcaData,
                                                                   const U2McaRow &rowInDb,
                                                                   const DNAChromatogram &chromatogram,
                                                                   const DNASequence &predictedSequence,
                                                                   const U2MsaRowGapModel &gapModel)
    : MultipleAlignmentRow(new MultipleChromatogramAlignmentRowData(mcaData, rowInDb, chromatogram, predictedSequence, gapModel))
{

}

MultipleChromatogramAlignmentRow::MultipleChromatogramAlignmentRow(const MultipleChromatogramAlignmentData *mcaData,
                                                                   const U2McaRow &rowInDb,
                                                                   const DNAChromatogram &chromatogram,
                                                                   const DNASequence &predictedSequence,
                                                                   const U2MsaRowGapModel &predictedSequenceGapModel,
                                                                   const DNASequence &editedSequence,
                                                                   const U2MsaRowGapModel &editedSequenceGapModel,
                                                                   const U2Region &workingArea)
    : MultipleAlignmentRow(new MultipleChromatogramAlignmentRowData(mcaData, rowInDb, chromatogram, predictedSequence, predictedSequenceGapModel, editedSequence, editedSequenceGapModel, workingArea))
{

}

MultipleChromatogramAlignmentRow::MultipleChromatogramAlignmentRow(const MultipleChromatogramAlignmentData *mcaData,
                                                                   const U2McaRow &rowInDb,
                                                                   const QString &rowName,
                                                                   const DNAChromatogram &chromatogram,
                                                                   const QByteArray &predictedSequenceRawData)
    : MultipleAlignmentRow(new MultipleChromatogramAlignmentRowData(mcaData, rowInDb, rowName, chromatogram, predictedSequenceRawData))
{

}

MultipleChromatogramAlignmentRow::MultipleChromatogramAlignmentRow(const MultipleChromatogramAlignmentData *mcaData, const MultipleChromatogramAlignmentRowData &mcaRowData)
    : MultipleAlignmentRow(new MultipleChromatogramAlignmentRowData(mcaData, mcaRowData))
{

}

MultipleChromatogramAlignmentRowData * MultipleChromatogramAlignmentRow::data() const {
    return getMcaRowData().data();
}

MultipleChromatogramAlignmentRowData & MultipleChromatogramAlignmentRow::operator*() {
    return *getMcaRowData();
}

const MultipleChromatogramAlignmentRowData & MultipleChromatogramAlignmentRow::operator*() const {
    return *getMcaRowData();
}

MultipleChromatogramAlignmentRowData * MultipleChromatogramAlignmentRow::operator->() {
    return getMcaRowData().data();
}

const MultipleChromatogramAlignmentRowData * MultipleChromatogramAlignmentRow::operator->() const {
    return getMcaRowData().data();
}

MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentRow::clone() const {
    return getMcaRowData()->getExplicitCopy();
}

QSharedPointer<MultipleChromatogramAlignmentRowData> MultipleChromatogramAlignmentRow::getMcaRowData() const {
    return maRowData.dynamicCast<MultipleChromatogramAlignmentRowData>();
}

MultipleChromatogramAlignmentRowData::MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentData *mcaData)
    : mcaData(mcaData)
{

}

MultipleChromatogramAlignmentRowData::MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentData *mcaData,
                                                                           const U2McaRow &rowInDb,
                                                                           const DNAChromatogram &chromatogram,
                                                                           const DNASequence &predictedSequence,
                                                                           const U2MsaRowGapModel &gapModel)
    : chromatogram(chromatogram),
      predictedSequence(predictedSequence),
      editedSequence(predictedSequence),
      commonGapModel(gapModel),
      initialRowInDb(rowInDb),
      mcaData(mcaData)
{
    removeTrailingGaps();
    workingArea = U2Region(0, MsaRowUtils::getRowLength(predictedSequence.seq, getCommonGapModel()));
}

MultipleChromatogramAlignmentRowData::MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentData *mcaData,
                                                                           const U2McaRow &rowInDb,
                                                                           const DNAChromatogram &chromatogram,
                                                                           const DNASequence &predictedSequence,
                                                                           const U2MsaRowGapModel &predictedSequenceGapModel,
                                                                           const DNASequence &editedSequence,
                                                                           const U2MsaRowGapModel &editedSequenceGapModel,
                                                                           const U2Region &workingArea)
    : chromatogram(chromatogram),
      predictedSequence(predictedSequence),
      editedSequence(editedSequence),
      predictedSequenceCachedGapModel(predictedSequenceGapModel),
      editedSequenceCachedGapModel(editedSequenceGapModel),
      initialRowInDb(rowInDb),
      mcaData(mcaData)
{
    extractCommonGapModel(predictedSequenceGapModel, editedSequenceGapModel);
    removeTrailingGaps();
    this->workingArea = U2Region(0, getCoreLength()).intersect(workingArea);
}

MultipleChromatogramAlignmentRowData::MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentData *mcaData,
                                                                           const U2McaRow &rowInDb,
                                                                           const QString &rowName,
                                                                           const DNAChromatogram &chromatogram,
                                                                           const QByteArray &predictedSequenceRawData)
    : chromatogram(chromatogram),
      initialRowInDb(rowInDb),
      mcaData(mcaData)
{
    QByteArray sequenceData;
    U2MsaRowGapModel gapModel;
    MsaDbiUtils::splitBytesToCharsAndGaps(predictedSequenceRawData, sequenceData, gapModel);

    predictedSequence = DNASequence(rowName, sequenceData);
    editedSequence = DNASequence(rowName, sequenceData);
    commonGapModel = gapModel;

    removeTrailingGaps();

    workingArea = U2Region(0, getCoreLength());
}

MultipleChromatogramAlignmentRowData::MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentData *newMcaData, const MultipleChromatogramAlignmentRowData &mcaRowData)
{
    copy(mcaRowData);
    mcaData = newMcaData;
}

QString MultipleChromatogramAlignmentRowData::getName() const {
    return editedSequence.getName();
}

void MultipleChromatogramAlignmentRowData::setName(const QString &name) {
    editedSequence.setName(name);
}

qint64 MultipleChromatogramAlignmentRowData::getRowId() const {
    return initialRowInDb.rowId;
}

void MultipleChromatogramAlignmentRowData::setRowId(qint64 rowId) {
    initialRowInDb.rowId = rowId;
}

U2McaRow MultipleChromatogramAlignmentRowData::getRowDbInfo() const {
    U2McaRow dbRow;
    dbRow.rowId = initialRowInDb.rowId;
    dbRow.chromatogramId = initialRowInDb.chromatogramId;
    dbRow.predictedSequenceId = initialRowInDb.predictedSequenceId;
    dbRow.sequenceId = initialRowInDb.sequenceId;
    dbRow.gstart = workingArea.startPos;
    dbRow.gend = workingArea.endPos();
    dbRow.predictedSequenceGaps = predictedSequenceCachedGapModel;
    dbRow.gaps = editedSequenceCachedGapModel;
    dbRow.length = getRowLengthWithoutTrailing();
    return dbRow;
}

void MultipleChromatogramAlignmentRowData::setRowDbInfo(const U2McaRow &dbRow) {
    initialRowInDb = dbRow;
}

const DNASequence & MultipleChromatogramAlignmentRowData::getPredictedSequence() const {
    return predictedSequence;
}

const DNASequence & MultipleChromatogramAlignmentRowData::getEditedSequence() const {
    return editedSequence;
}

const DNAChromatogram & MultipleChromatogramAlignmentRowData::getChromatogram() const {
    return chromatogram;
}

McaRowMemoryData MultipleChromatogramAlignmentRowData::getRowMemoryData() const {
    McaRowMemoryData memoryData;
    memoryData.chromatogram = chromatogram;
    memoryData.predictedSequence = predictedSequence;
    memoryData.editedSequence = editedSequence;
    memoryData.predictedSequenceGapModel = predictedSequenceCachedGapModel;
    memoryData.editedSequenceGapModel = editedSequenceCachedGapModel;
    memoryData.workingArea = workingArea;
    memoryData.rowLength = getRowLengthWithoutTrailing();
    return memoryData;
}

const U2MsaRowGapModel & MultipleChromatogramAlignmentRowData::getPredictedSequenceGapModel() const {
    return predictedSequenceCachedGapModel;
}

const U2MsaRowGapModel & MultipleChromatogramAlignmentRowData::getEditedSequenceGapModel() const {
    return editedSequenceCachedGapModel;
}

const U2MsaRowGapModel & MultipleChromatogramAlignmentRowData::getCommonGapModel() const {
    return commonGapModel;
}

void MultipleChromatogramAlignmentRowData::setGapModel(const U2MsaRowGapModel &newGapModel) {
    commonGapModel = newGapModel;
    updateCachedGapModels();
}

const U2MsaRowGapModel & MultipleChromatogramAlignmentRowData::getEditedSequenceDifferenceGapModel() const {
    return editedSequenceGapModelDifference;
}

QByteArray MultipleChromatogramAlignmentRowData::getPredictedSequenceData() const {
    return predictedSequence.seq;
}

QByteArray MultipleChromatogramAlignmentRowData::getPredictedSequenceCore() const {
    return MsaRowUtils::joinCharsAndGaps(predictedSequence.seq, getPredictedSequenceCoreGapModel(), getCoreLength(), true, true);
}

QByteArray MultipleChromatogramAlignmentRowData::getPredictedSequenceWorkingArea() const {
    return MsaRowUtils::joinCharsAndGaps(predictedSequence.seq, getPredictedSequenceWorkingAreaGapModel(), getWorkingAreaLength(), true, true);
}

QByteArray MultipleChromatogramAlignmentRowData::getEditedSequenceData() const {
    return editedSequence.seq;
}

QByteArray MultipleChromatogramAlignmentRowData::getEditedSequenceCore() const {
    return MsaRowUtils::joinCharsAndGaps(editedSequence.seq, getEditedSequenceCoreGapModel(), getCoreLength(), true, true);
}

QByteArray MultipleChromatogramAlignmentRowData::getEditedSequenceWorkingArea() const {
    return MsaRowUtils::joinCharsAndGaps(editedSequence.seq, getEditedSequenceWorkingAreaGapModel(), getWorkingAreaLength(), true, true);
}

void MultipleChromatogramAlignmentRowData::setRowContent(U2OpStatus &os, const McaRowMemoryData &mcaRowMemoryData) {
    SAFE_POINT_EXT(mcaRowMemoryData.chromatogram.traceLength == mcaRowMemoryData.predictedSequence.length(), os.setError("Inconsistent row data"), );
    chromatogram = mcaRowMemoryData.chromatogram;
    predictedSequence = mcaRowMemoryData.predictedSequence;
    editedSequence = mcaRowMemoryData.editedSequence;

    extractCommonGapModel(mcaRowMemoryData.predictedSequenceGapModel, mcaRowMemoryData.editedSequenceGapModel);
    removeTrailingGaps();

    SAFE_POINT_EXT(predictedSequence.length() + getPredictedSequenceGuaranteedGapsLength() == editedSequence.length() + getEditedSequenceGuaranteedGapsLength(),
                   os.setError("Inconsistent row data"), );

    this->workingArea = U2Region(0, getCoreLength()).intersect(mcaRowMemoryData.workingArea);
}

void MultipleChromatogramAlignmentRowData::setParentAlignment(const MultipleChromatogramAlignment &mca) {
    setParentAlignment(mca.data());
}

void MultipleChromatogramAlignmentRowData::setParentAlignment(MultipleChromatogramAlignmentData *newMcaData) {
    SAFE_POINT(NULL != newMcaData, "A NULL MCA is set as parent alignment. A row without a parent is inconsistent", );
    mcaData = newMcaData;
}

qint64 MultipleChromatogramAlignmentRowData::getPredictedSequenceLength() const {
    return predictedSequence.length();
}

qint64 MultipleChromatogramAlignmentRowData::getEditedSequenceLength() const {
    return editedSequence.length();
}

qint64 MultipleChromatogramAlignmentRowData::getChromatogramLength() const {
    return chromatogram.traceLength;
}

qint64 MultipleChromatogramAlignmentRowData::getCoreStart() const {
    return MsaRowUtils::getCoreStart(commonGapModel);
}

qint64 MultipleChromatogramAlignmentRowData::getCoreLength() const {
    return getRowLengthWithoutTrailing() - getCoreStart();
}

qint64 MultipleChromatogramAlignmentRowData::getWorkingAreaLength() const {
    return workingArea.length;
}

qint64 MultipleChromatogramAlignmentRowData::getRowLength() const {
    return mcaData->getLength();
}

qint64 MultipleChromatogramAlignmentRowData::getRowLengthWithoutTrailing() const {
    const qint64 gappedPredictedDataLength = MsaRowUtils::getRowLengthWithoutTrailing(predictedSequence.length(), predictedSequenceGapModelDifference);
    const qint64 gappedEditedDataLength = MsaRowUtils::getRowLengthWithoutTrailing(editedSequence.length(), editedSequenceGapModelDifference);
    return MsaRowUtils::getRowLengthWithoutTrailing(qMax(gappedPredictedDataLength, gappedEditedDataLength), commonGapModel);
}

qint64 MultipleChromatogramAlignmentRowData::getBaseCount(qint64 before) const {
    const qint64 rowLength = MsaRowUtils::getRowLength(editedSequence.seq, editedSequenceCachedGapModel);
    return MsaRowUtils::getUngappedPosition(editedSequenceCachedGapModel, editedSequence.length(), qMin(rowLength, before), true);
}

U2Region MultipleChromatogramAlignmentRowData::getCoreRegion() const {
    return U2Region(getCoreStart(), getCoreLength());
}

U2Region MultipleChromatogramAlignmentRowData::getWorkingAreaRegion() const {
    return workingArea;
}

QByteArray MultipleChromatogramAlignmentRowData::toByteArray(U2OpStatus &os, qint64 length) const {
    // SANGER_TODO: limit to the provider length!
    return getEditedSequenceData();
}

char MultipleChromatogramAlignmentRowData::charAt(qint64 position) const {
    return getEditedSequenceChar(position);
}

char MultipleChromatogramAlignmentRowData::getPredictedSequenceWorkingAreaChar(qint64 position) const {
    CHECK(workingArea.contains(position), U2Msa::GAP_CHAR);
    return getPredictedSequenceChar(position);
}

char MultipleChromatogramAlignmentRowData::getPredictedSequenceChar(qint64 position) const {
    CHECK(0 <= position && position <= getRowLength(), U2Msa::GAP_CHAR);
    return MsaRowUtils::charAt(predictedSequence.seq, predictedSequenceCachedGapModel, position);
}

char MultipleChromatogramAlignmentRowData::getEditedSequenceWorkingAreaChar(qint64 position) const {
    CHECK(workingArea.contains(position), U2Msa::GAP_CHAR);
    return getEditedSequenceChar(position);
}

char MultipleChromatogramAlignmentRowData::getEditedSequenceChar(qint64 position) const {
    CHECK(0 <= position && position <= getRowLength(), U2Msa::GAP_CHAR);
    return MsaRowUtils::charAt(editedSequence.seq, editedSequenceCachedGapModel, position);
}

ushort MultipleChromatogramAlignmentRowData::getChromatogramWorkingAreaValue(DNAChromatogram::Trace trace, qint64 position) const {
    CHECK(workingArea.contains(position), DNAChromatogram::INVALID_VALUE);
    return getChromatogramValue(trace, position);
}

ushort MultipleChromatogramAlignmentRowData::getChromatogramValue(DNAChromatogram::Trace trace, qint64 position) const {
    CHECK(0 <= position && position <= getRowLength(), DNAChromatogram::INVALID_VALUE);
    const qint64 dataPosition = MsaRowUtils::getUngappedPosition(predictedSequenceCachedGapModel, chromatogram.traceLength, position);
    CHECK(-1 != dataPosition, DNAChromatogram::INVALID_VALUE);
    return chromatogram.getValue(trace, dataPosition);
}

void MultipleChromatogramAlignmentRowData::insertGap(U2OpStatus &os, qint64 position) {
    insertGaps(os, position, 1);
}

void MultipleChromatogramAlignmentRowData::insertGaps(U2OpStatus &os, qint64 position, qint64 count) {
    MsaRowUtils::insertGaps(os, commonGapModel, getRowLengthWithoutTrailing(), position, count);
    updateCachedGapModels();
}

void MultipleChromatogramAlignmentRowData::removeGap(U2OpStatus &os, qint64 position) {
    removeGaps(os, position, 1);
}

void MultipleChromatogramAlignmentRowData::removeGaps(U2OpStatus &os, qint64 position, qint64 count) {
    MsaRowUtils::removeGaps(os, commonGapModel, getRowLengthWithoutTrailing(), position, count);
}

void MultipleChromatogramAlignmentRowData::replaceCharInEditedSequence(U2OpStatus &os, qint64 position, char newChar) {
    SAFE_POINT(getCoreRegion().contains(position), "An attempt to change the sequence in the position that is out of boundaries", );
    const char currentChar = getEditedSequenceChar(position);
    if (U2Msa::GAP_CHAR == currentChar) {
        CHECK(newChar != U2Msa::GAP_CHAR, );
        replaceGapToCharInEditedSequence(os, position, newChar);
    } else if (U2Msa::GAP_CHAR == newChar) {
        replaceCharToGapInEditedSequence(os, position);
    } else {
        CHECK(newChar != currentChar, );
        replaceCharToCharInEditedSequence(os, position, newChar);
    }
}

MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentRowData::getExplicitCopy(const MultipleChromatogramAlignmentData *mcaData) const {
    return MultipleChromatogramAlignmentRow(new MultipleChromatogramAlignmentRowData(NULL == mcaData ? this->mcaData : mcaData, *this));
}

bool MultipleChromatogramAlignmentRowData::operator ==(const MultipleChromatogramAlignmentRowData &mcaRowData) const {
    return MatchExactly == DNASequenceUtils::compare(predictedSequence, mcaRowData.predictedSequence) &&
            MatchExactly == DNASequenceUtils::compare(editedSequence, mcaRowData.editedSequence) &&
            predictedSequenceCachedGapModel == mcaRowData.predictedSequenceCachedGapModel &&
            editedSequenceCachedGapModel == mcaRowData.editedSequenceCachedGapModel &&
            chromatogram == mcaRowData.chromatogram;
}

bool MultipleChromatogramAlignmentRowData::operator ==(const MultipleAlignmentRowData &maRowData) const {
    try {
        return  (*this == dynamic_cast<const MultipleChromatogramAlignmentRowData &>(maRowData));
    } catch (std::bad_cast) {
        FAIL("Can't cast MultipleAlignmentRowData to MultipleChromatogramAlignmentRowData", true);
    }
}

bool MultipleChromatogramAlignmentRowData::operator !=(const MultipleChromatogramAlignmentRowData &mcaRowData) const {
    return !operator ==(mcaRowData);
}

bool MultipleChromatogramAlignmentRowData::operator !=(const MultipleAlignmentRowData &maRowData) const {
    return !operator ==(maRowData);
}

void MultipleChromatogramAlignmentRowData::crop(U2OpStatus &os, qint64 startPosition, qint64 count) {
    const qint64 endPosition = startPosition + count;
    const qint64 initialRowLength = getRowLength();
    SAFE_POINT_EXT(0 <= startPosition && startPosition < initialRowLength,
                   os.setError(QString("Position is out of row boudaries: start position '%1', row length '%2'").arg(startPosition).arg(initialRowLength)), );
    SAFE_POINT_EXT(0 < count,
                   os.setError(QString("Crop window size is incorrect: crop size '%1'").arg(count)), );

    const qint64 predictedSequenceCropStartPosition = MsaRowUtils::getUngappedPosition(predictedSequenceCachedGapModel, getPredictedSequenceLength(), startPosition);
    const qint64 predictedSequenceCropEndPosition = MsaRowUtils::getUngappedPosition(predictedSequenceCachedGapModel, getPredictedSequenceLength(), endPosition);
    const qint64 editedSequenceCropStartPosition = MsaRowUtils::getUngappedPosition(editedSequenceCachedGapModel, getEditedSequenceLength(), startPosition);
    const qint64 editedSequenceCropEndPosition = MsaRowUtils::getUngappedPosition(editedSequenceCachedGapModel, getEditedSequenceLength(), endPosition);

    ChromatogramUtils::crop(chromatogram, predictedSequenceCropStartPosition, predictedSequenceCropEndPosition - predictedSequenceCropStartPosition);
    DNASequenceUtils::crop(predictedSequence, predictedSequenceCropStartPosition, predictedSequenceCropEndPosition - predictedSequenceCropStartPosition);
    DNASequenceUtils::crop(editedSequence, editedSequenceCropStartPosition, editedSequenceCropEndPosition - editedSequenceCropStartPosition);

    MsaRowUtils::chopGapModel(predictedSequenceCachedGapModel, U2Region(startPosition, endPosition - startPosition));
    MsaRowUtils::chopGapModel(editedSequenceCachedGapModel, U2Region(startPosition, endPosition - startPosition));

    extractCommonGapModel(predictedSequenceCachedGapModel, editedSequenceCachedGapModel);
    removeTrailingGaps();

    workingArea = workingArea.intersect(U2Region(startPosition, endPosition - startPosition));
}

MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentRowData::mid(U2OpStatus &os, qint64 mcaVisiblePosition, qint64 count) const {
    MultipleChromatogramAlignmentRow row = getExplicitCopy();
    row->crop(os, mcaVisiblePosition, count);
    return row;
}

bool MultipleChromatogramAlignmentRowData::isCommonGap(qint64 position) const {
    return MsaRowUtils::isGap(getCoreLength(), commonGapModel, position);
}

void MultipleChromatogramAlignmentRowData::extractCommonGapModel(const U2MsaRowGapModel &predictedSequenceGapModel, const U2MsaRowGapModel &editedSequenceGapModel) {
    MsaRowUtils::getGapModelsDifference(predictedSequenceGapModel, editedSequenceGapModel, commonGapModel, predictedSequenceGapModelDifference, editedSequenceGapModelDifference);
}

void MultipleChromatogramAlignmentRowData::removeTrailingGaps() {
    const qint64 rowLengthWithoutTrailing = getRowLengthWithoutTrailing();
    MsaRowUtils::chopGapModel(predictedSequenceCachedGapModel, rowLengthWithoutTrailing);
    MsaRowUtils::chopGapModel(editedSequenceCachedGapModel, rowLengthWithoutTrailing);
    extractCommonGapModel(predictedSequenceCachedGapModel, editedSequenceCachedGapModel);
}

void MultipleChromatogramAlignmentRowData::updateCachedGapModels() const {
    predictedSequenceCachedGapModel = MsaRowUtils::insertGapModel(commonGapModel, predictedSequenceGapModelDifference);
    editedSequenceCachedGapModel = MsaRowUtils::insertGapModel(commonGapModel, editedSequenceGapModelDifference);
}

qint64 MultipleChromatogramAlignmentRowData::getPredictedSequenceGuaranteedGapsLength() const {
    return MsaRowUtils::getGapsLength(predictedSequenceGapModelDifference);
}

qint64 MultipleChromatogramAlignmentRowData::getEditedSequenceGuaranteedGapsLength() const {
    return MsaRowUtils::getGapsLength(editedSequenceGapModelDifference);
}

U2MsaRowGapModel MultipleChromatogramAlignmentRowData::getPredictedSequenceCoreGapModel() const {
    U2MsaRowGapModel predictedSequenceCoreGapModel = predictedSequenceCachedGapModel;
    MsaRowUtils::shiftGapModel(predictedSequenceCoreGapModel, -MsaRowUtils::getCoreStart(commonGapModel));
    return predictedSequenceCoreGapModel;
}

U2MsaRowGapModel MultipleChromatogramAlignmentRowData::getPredictedSequenceWorkingAreaGapModel() const {
    U2MsaRowGapModel predictedSequenceWorkingAreaGapModel = predictedSequenceCachedGapModel;
    MsaRowUtils::chopGapModel(predictedSequenceWorkingAreaGapModel, getWorkingAreaRegion());
    return predictedSequenceWorkingAreaGapModel;
}

U2MsaRowGapModel MultipleChromatogramAlignmentRowData::getEditedSequenceCoreGapModel() const {
    U2MsaRowGapModel editedSequenceCoreGapModel = editedSequenceCachedGapModel;
    MsaRowUtils::shiftGapModel(editedSequenceCoreGapModel, -MsaRowUtils::getCoreStart(commonGapModel));
    return editedSequenceCoreGapModel;
}

U2MsaRowGapModel MultipleChromatogramAlignmentRowData::getEditedSequenceWorkingAreaGapModel() const {
    U2MsaRowGapModel editedSequenceWorkingAreaGapModel = editedSequenceCachedGapModel;
    MsaRowUtils::chopGapModel(editedSequenceWorkingAreaGapModel, getWorkingAreaRegion());
    return editedSequenceWorkingAreaGapModel;
}

U2MsaRowGapModel MultipleChromatogramAlignmentRowData::getCommonCoreGapModel() const {
    U2MsaRowGapModel commonCoreGapModel = commonGapModel;
    MsaRowUtils::shiftGapModel(commonCoreGapModel, -MsaRowUtils::getCoreStart(commonGapModel));
    return commonCoreGapModel;
}

char MultipleChromatogramAlignmentRowData::getPredictedSequenceCoreChar(qint64 corePosition) const {
    SAFE_POINT(0 <= corePosition && corePosition <= getCoreLength(), "Position is out of the predicted sequence core boundaries", U2Msa::GAP_CHAR);
    return MsaRowUtils::charAt(predictedSequence.seq, getPredictedSequenceCoreGapModel(), corePosition);
}

char MultipleChromatogramAlignmentRowData::getEditedSequenceCoreChar(qint64 corePosition) const {
    SAFE_POINT(0 <= corePosition && corePosition <= getCoreLength(), "Position is out of the edited sequence core boundaries", U2Msa::GAP_CHAR);
    return MsaRowUtils::charAt(editedSequence.seq, getEditedSequenceCoreGapModel(), corePosition);
}

qint64 MultipleChromatogramAlignmentRowData::getPredictedSequenceDataLength() const {
    return getPredictedSequenceLength() + MsaRowUtils::getGapsLength(predictedSequenceGapModelDifference);
}

qint64 MultipleChromatogramAlignmentRowData::getEditedSequenceDataLength() const {
    return getEditedSequenceLength() + MsaRowUtils::getGapsLength(editedSequenceGapModelDifference);
}

void MultipleChromatogramAlignmentRowData::replaceGapToCharInEditedSequence(U2OpStatus &os, qint64 position, char newChar) {
    const qint64 sequencePosition = MsaRowUtils::getUngappedPosition(editedSequenceCachedGapModel, getEditedSequenceLength(), position, true);
    SAFE_POINT_EXT(0 <= sequencePosition && sequencePosition < editedSequence.length(), os.setError("The edited sequence data position is out of boundaries"), );
    editedSequence.seq.insert(sequencePosition + 1, newChar);

    if (U2Msa::GAP_CHAR == getPredictedSequenceChar(position)) {
        const qint64 predictedSequenceDataLength = getPredictedSequenceDataLength();
        const qint64 dataPosition = MsaRowUtils::getUngappedPosition(commonGapModel, predictedSequenceDataLength, position, true);
        MsaRowUtils::removeGaps(os, commonGapModel, getRowLengthWithoutTrailing(), position, 1);
        MsaRowUtils::insertGaps(os, predictedSequenceGapModelDifference, predictedSequenceDataLength, dataPosition, 1);
    } else {
        const qint64 editedSequenceDataLength = getEditedSequenceDataLength();
        const qint64 dataPosition = MsaRowUtils::getUngappedPosition(commonGapModel, editedSequenceDataLength, position, true);
        MsaRowUtils::insertGaps(os, editedSequenceGapModelDifference, editedSequenceDataLength, dataPosition, 1);
    }
    updateCachedGapModels();
}

void MultipleChromatogramAlignmentRowData::replaceCharToGapInEditedSequence(U2OpStatus &os, qint64 position) {
    const qint64 sequencePosition = MsaRowUtils::getUngappedPosition(editedSequenceCachedGapModel, getEditedSequenceLength(), position);
    SAFE_POINT_EXT(-1 != sequencePosition, os.setError("Expect a symbol on the requested poosition, got a gap"), );
    SAFE_POINT_EXT(0 <= sequencePosition && sequencePosition < editedSequence.length(), os.setError("The edited sequence data position is out of boundaries"), );
    editedSequence.seq.remove(sequencePosition, 1);

    if (U2Msa::GAP_CHAR == getPredictedSequenceCoreChar(position)) {
        const qint64 predictedSequenceDataLength = getPredictedSequenceDataLength();
        const qint64 dataPosition = MsaRowUtils::getUngappedPosition(commonGapModel, predictedSequenceDataLength, position, true);
        MsaRowUtils::insertGaps(os, commonGapModel, getRowLengthWithoutTrailing(), position, 1);
        MsaRowUtils::removeGaps(os, predictedSequenceGapModelDifference, predictedSequenceDataLength, dataPosition, 1);
    } else {
        const qint64 editedSequenceDataLength = getEditedSequenceDataLength();
        const qint64 dataPosition = MsaRowUtils::getUngappedPosition(commonGapModel, editedSequenceDataLength, position, true);
        MsaRowUtils::insertGaps(os, editedSequenceGapModelDifference, editedSequenceDataLength, dataPosition, 1);
    }
    updateCachedGapModels();
}

void MultipleChromatogramAlignmentRowData::replaceCharToCharInEditedSequence(U2OpStatus &os, qint64 position, char newChar) {
    const qint64 sequencePosition = MsaRowUtils::getUngappedPosition(editedSequenceCachedGapModel, getEditedSequenceLength(), position);
    SAFE_POINT_EXT(-1 != sequencePosition, os.setError("Expect a symbol on the requested poosition, got a gap"), );
    SAFE_POINT_EXT(0 <= sequencePosition && sequencePosition < editedSequence.length(), os.setError("The edited sequence data position is out of boundaries"), );
    editedSequence.seq[static_cast<int>(sequencePosition)] = newChar;
}

void MultipleChromatogramAlignmentRowData::copy(const MultipleChromatogramAlignmentRowData &mcaRowData) {
    chromatogram = mcaRowData.chromatogram;
    predictedSequence = mcaRowData.predictedSequence;
    editedSequence = mcaRowData.editedSequence;
    commonGapModel = mcaRowData.commonGapModel;
    predictedSequenceCachedGapModel = mcaRowData.predictedSequenceCachedGapModel;
    editedSequenceCachedGapModel = mcaRowData.editedSequenceCachedGapModel;
    predictedSequenceGapModelDifference = mcaRowData.predictedSequenceGapModelDifference;
    editedSequenceGapModelDifference = mcaRowData.editedSequenceGapModelDifference;
    workingArea = mcaRowData.workingArea;
    mcaData = mcaRowData.mcaData;
}

}   // namespace U2
