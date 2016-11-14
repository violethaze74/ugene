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

qint64 MultipleChromatogramAlignmentRowData::getPredictedSequenceDataLength() const {
    return predictedSequence.length();
}

qint64 MultipleChromatogramAlignmentRowData::getEditedSequenceDataLength() const {
    return editedSequence.length();
}

qint64 MultipleChromatogramAlignmentRowData::getChromatogramDataLength() const {
    return chromatogram.traceLength;
}

qint64 MultipleChromatogramAlignmentRowData::getPredictedSequenceWorkingAreaDataLength() const {
    return getWorkingAreaLength() - MsaRowUtils::getGapsLength(getPredictedSequenceWorkingAreaGapModel());
}

qint64 MultipleChromatogramAlignmentRowData::getEditedSequenceWorkingAreaDataLength() const {
    return getWorkingAreaLength() - MsaRowUtils::getGapsLength(getEditedSequenceWorkingAreaGapModel());
}

qint64 MultipleChromatogramAlignmentRowData::getChromatogramWorkingAreaDataLength() const {
    return getPredictedSequenceWorkingAreaDataLength();
}

qint64 MultipleChromatogramAlignmentRowData::getCoreStart() const {
    return MsaRowUtils::getCoreStart(commonGapModel);
}

qint64 MultipleChromatogramAlignmentRowData::getCoreLength() const {
    qint64 gappedDataLength = predictedSequence.length();
    foreach (const U2MsaGap &gap, predictedSequenceGapModelDifference) {
        if (gap.offset > gappedDataLength) {
            break;
        }
        gappedDataLength += gap.gap;
    }

    qint64 lengthWithoutTrailing = gappedDataLength;
    foreach (const U2MsaGap &gap, commonGapModel) {
        if (gap.offset > lengthWithoutTrailing) {
            break;
        }
        lengthWithoutTrailing += gap.gap;
    }

    return lengthWithoutTrailing - getCoreStart();
    return predictedSequence.length() + MsaRowUtils::getGapsLength(getPredictedSequenceCoreGapModel());
}

qint64 MultipleChromatogramAlignmentRowData::getWorkingAreaLength() const {
    return workingArea.length;
}

qint64 MultipleChromatogramAlignmentRowData::getRowLength() const {
    return mcaData->getLength();
}

qint64 MultipleChromatogramAlignmentRowData::getRowLengthWithoutTrailing() const {
    return getCoreStart() + getCoreLength();
}

U2Region MultipleChromatogramAlignmentRowData::getCoreRegion() const {
    return U2Region(getCoreStart(), getCoreLength());
}

U2Region MultipleChromatogramAlignmentRowData::getWorkingAreaRegion() const {
    return workingArea;
}

char MultipleChromatogramAlignmentRowData::charAt(int pos) const {
    // SANGER_TODO: check gaps!
    return MsaRowUtils::charAt(editedSequence.seq, editedSequenceCachedGapModel, pos);
}

char MultipleChromatogramAlignmentRowData::getPredictedSequenceDataChar(qint64 dataPosition) const {
    SAFE_POINT(0 <= dataPosition && dataPosition <= getPredictedSequenceDataLength(), "Position is out of the predicted sequence data boundaries", U2Msa::GAP_CHAR);
    return predictedSequence.seq.at(dataPosition);
}

char MultipleChromatogramAlignmentRowData::getPredictedSequenceCoreChar(qint64 corePosition) const {
    SAFE_POINT(0 <= corePosition && corePosition <= getCoreLength(), "Position is out of the predicted sequence core boundaries", U2Msa::GAP_CHAR);
    return MsaRowUtils::charAt(predictedSequence.seq, getPredictedSequenceCoreGapModel(), corePosition);
}

char MultipleChromatogramAlignmentRowData::getPredictedSequenceWorkingAreaChar(qint64 workingAreaPosition) const {
    SAFE_POINT(0 <= workingAreaPosition && workingAreaPosition <= getWorkingAreaLength(), "Position is out of the working area boundaries", U2Msa::GAP_CHAR);
    return getPredictedSequenceCoreChar(workingAreaPosition + workingArea.startPos);
}

char MultipleChromatogramAlignmentRowData::getPredictedSequenceChar(qint64 mcaVisiblePosition) const {
    SAFE_POINT(0 <= mcaVisiblePosition && mcaVisiblePosition <= getRowLength(), "Position is out of the row boundaries", U2Msa::GAP_CHAR);
    return getPredictedSequenceWorkingAreaChar(mapVisiblePositionToCorePosition(mcaVisiblePosition));
}

char MultipleChromatogramAlignmentRowData::getEditedSequenceDataChar(qint64 dataPosition) const {
    SAFE_POINT(0 <= dataPosition && dataPosition <= getEditedSequenceDataLength(), "Position is out of the edited sequence data boundaries", U2Msa::GAP_CHAR);
    return editedSequence.seq.at(dataPosition);
}

char MultipleChromatogramAlignmentRowData::getEditedSequenceCoreChar(qint64 corePosition) const {
    SAFE_POINT(0 <= corePosition && corePosition <= getCoreLength(), "Position is out of the edited sequence core boundaries", U2Msa::GAP_CHAR);
    return MsaRowUtils::charAt(editedSequence.seq, getEditedSequenceCoreGapModel(), corePosition);
}

char MultipleChromatogramAlignmentRowData::getEditedSequenceWorkingAreaChar(qint64 workingAreaPosition) const {
    SAFE_POINT(0 <= workingAreaPosition && workingAreaPosition <= getWorkingAreaLength(), "Position is out of the working area boundaries", U2Msa::GAP_CHAR);
    return getEditedSequenceCoreChar(workingAreaPosition + workingArea.startPos);
}

char MultipleChromatogramAlignmentRowData::getEditedSequenceChar(qint64 mcaVisiblePosition) const {
    SAFE_POINT(0 <= mcaVisiblePosition && mcaVisiblePosition <= getRowLength(), "Position is out of the row boundaries", U2Msa::GAP_CHAR);
    return getEditedSequenceWorkingAreaChar(mapVisiblePositionToCorePosition(mcaVisiblePosition));
}

ushort MultipleChromatogramAlignmentRowData::getChromatogramDataValue(DNAChromatogram::Trace trace, qint64 dataPosition) const {
    SAFE_POINT(0 <= dataPosition && dataPosition <= getChromatogramDataLength(), "Position is out of the chromatogram data boundaries", 0);
    return chromatogram.getValue(trace, dataPosition);
}

ushort MultipleChromatogramAlignmentRowData::getChromatogramCoreValue(DNAChromatogram::Trace trace, qint64 corePosition) const {
    SAFE_POINT(0 <= corePosition && corePosition <= getCoreLength(), "Position is out of the chromatogram core boundaries", 0);
    const qint64 dataPosition = MsaRowUtils::getUngappedPosition(predictedSequenceCachedGapModel, chromatogram.traceLength, corePosition);
    CHECK(-1 != dataPosition, DNAChromatogram::INVALID_VALUE);
    return getChromatogramDataValue(trace, dataPosition);
}

ushort MultipleChromatogramAlignmentRowData::getChromatogramWorkingAreaValue(DNAChromatogram::Trace trace, qint64 workingAreaPosition) const {
    SAFE_POINT(0 <= workingAreaPosition && workingAreaPosition <= getWorkingAreaLength(), "Position is out of the working area boundaries", 0);
    return getChromatogramCoreValue(trace, workingAreaPosition + workingArea.startPos);
}

ushort MultipleChromatogramAlignmentRowData::getChromatogramValue(DNAChromatogram::Trace trace, qint64 mcaVisiblePosition) const {
    SAFE_POINT(0 <= mcaVisiblePosition && mcaVisiblePosition <= getRowLength(), "Position is out of the row boundaries", U2Msa::GAP_CHAR);
    return getChromatogramWorkingAreaValue(trace, mapVisiblePositionToCorePosition(mcaVisiblePosition));
}

void MultipleChromatogramAlignmentRowData::insertGap(U2OpStatus &os, qint64 mcaVisiblePosition) {
    insertGaps(os, mcaVisiblePosition, 1);
}

void MultipleChromatogramAlignmentRowData::insertGaps(U2OpStatus &os, qint64 mcaVisiblePosition, qint64 count) {
    MsaRowUtils::insertGaps(os, commonGapModel, getRowLengthWithoutTrailing(), mapVisiblePositionToRealPosition(mcaVisiblePosition), count);
    updateCachedGapModels();
}

void MultipleChromatogramAlignmentRowData::removeGap(U2OpStatus &os, qint64 mcaVisiblePosition) {
    removeGaps(os, mcaVisiblePosition, 1);
}

void MultipleChromatogramAlignmentRowData::removeGaps(U2OpStatus &os, qint64 mcaVisiblePosition, qint64 count) {
    MsaRowUtils::removeGaps(os, commonGapModel, getRowLengthWithoutTrailing(), mapVisiblePositionToRealPosition(mcaVisiblePosition), count);
}

void MultipleChromatogramAlignmentRowData::replaceCharInEditedSequence(U2OpStatus &os, qint64 mcaVisiblePosition, char newChar) {
    const qint64 corePosition = mapVisiblePositionToCorePosition(mcaVisiblePosition);
    SAFE_POINT(-1 != corePosition, "An attempt to change the sequence in the position that is out of boundaries", );
    const char currentChar = getEditedSequenceChar(mcaVisiblePosition);
    if (U2Msa::GAP_CHAR == currentChar) {
        CHECK(newChar != U2Msa::GAP_CHAR, );
        replaceGapToCharInEditedSequence(os, corePosition, newChar);
    } else if (U2Msa::GAP_CHAR == newChar) {
        replaceCharToGapInEditedSequence(os, corePosition, newChar);
    } else {
        CHECK(newChar != currentChar, );
        replaceCharToCharInEditedSequence(os, corePosition, newChar);
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

void MultipleChromatogramAlignmentRowData::crop(U2OpStatus &os, qint64 mcaVisiblePosition, qint64 count) {
    const qint64 realStartPosition = mapVisiblePositionToRealPosition(mcaVisiblePosition);
    const qint64 realEndPosition = mapVisiblePositionToRealPosition(mcaVisiblePosition + count);
    const qint64 initialRowLength = getRowLength();
    SAFE_POINT_EXT(0 <= realStartPosition && realStartPosition < initialRowLength,
                   os.setError(QString("Position is out of row boudaries: start position '%1', row length '%2'").arg(mcaVisiblePosition).arg(initialRowLength)), );
    SAFE_POINT_EXT(0 < count,
                   os.setError(QString("Crop window size is incorrect: crop size '%1'").arg(count)), );

    const qint64 predictedSequenceCropStartPosition = MsaRowUtils::getUngappedPosition(predictedSequenceCachedGapModel, getPredictedSequenceDataLength(), realStartPosition);
    const qint64 predictedSequenceCropEndPosition = MsaRowUtils::getUngappedPosition(predictedSequenceCachedGapModel, getPredictedSequenceDataLength(), realEndPosition);
    const qint64 editedSequenceCropStartPosition = MsaRowUtils::getUngappedPosition(editedSequenceCachedGapModel, getEditedSequenceDataLength(), realStartPosition);
    const qint64 editedSequenceCropEndPosition = MsaRowUtils::getUngappedPosition(editedSequenceCachedGapModel, getEditedSequenceDataLength(), realEndPosition);

    ChromatogramUtils::crop(chromatogram, predictedSequenceCropStartPosition, predictedSequenceCropEndPosition - predictedSequenceCropStartPosition);
    DNASequenceUtils::crop(predictedSequence, predictedSequenceCropStartPosition, predictedSequenceCropEndPosition - predictedSequenceCropStartPosition);
    DNASequenceUtils::crop(editedSequence, editedSequenceCropStartPosition, editedSequenceCropEndPosition - editedSequenceCropStartPosition);

    MsaRowUtils::chopGapModel(predictedSequenceCachedGapModel, U2Region(realStartPosition, realEndPosition - realStartPosition));
    MsaRowUtils::chopGapModel(editedSequenceCachedGapModel, U2Region(realStartPosition, realEndPosition - realStartPosition));

    extractCommonGapModel(predictedSequenceCachedGapModel, editedSequenceCachedGapModel);
    removeTrailingGaps();

    workingArea = workingArea.intersect(U2Region(realStartPosition, realEndPosition - realStartPosition));
}

MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentRowData::mid(U2OpStatus &os, qint64 mcaVisiblePosition, qint64 count) const {
    MultipleChromatogramAlignmentRow row = getExplicitCopy();
    row->crop(os, mcaVisiblePosition, count);
    return row;
}

bool MultipleChromatogramAlignmentRowData::isCommonGap(qint64 mcaVisiblePosition) const {
    const qint64 realPosition = mapVisiblePositionToRealPosition(mcaVisiblePosition);
    return MsaRowUtils::isGap(getCoreLength(), commonGapModel, realPosition);
}

void MultipleChromatogramAlignmentRowData::extractCommonGapModel(const U2MsaRowGapModel &predictedSequenceGapModel, const U2MsaRowGapModel &editedSequenceGapModel) {
    MsaRowUtils::getGapModelsDifference(predictedSequenceGapModel, editedSequenceGapModel, commonGapModel, predictedSequenceGapModelDifference, editedSequenceGapModelDifference);
}

namespace {

void removeTrailingGapsInDifference(qint64 dataLength, qint64 commonCoreGapModelLength, U2MsaRowGapModel &difference) {
    U2MsaRowGapModel cutDifference;
    qint64 accumulatedGapsLength = 0;
    foreach (const U2MsaGap &gap, difference) {
        if (gap.offset <= dataLength + commonCoreGapModelLength + accumulatedGapsLength) {
            cutDifference << gap;
        }
    }
    difference = cutDifference;
}

}

void MultipleChromatogramAlignmentRowData::removeTrailingGaps() {
//    qint64 accumulatedCoreLength = getPredictedSequenceDataLength();
//    foreach (const U2MsaGap &gap, predictedSequenceGapModelDifference) {
//        if (gap.offset > accumulatedCoreLength) {
//            break;
//        }
//        accumulatedCoreLength += gap.gap;
//    }

//    MsaRowUtils::chopGapModel(predictedSequenceGapModelDifference, accumulatedCoreLength);
//    MsaRowUtils::chopGapModel(editedSequenceGapModelDifference, accumulatedCoreLength);

//    qint64 rowLength = getCoreStart() + accumulatedCoreLength;
//    MsaRowUtils::chopGapModel(commonGapModel, rowLength);

//    updateCachedGapModels();

    const U2MsaRowGapModel commonCoreGapModel = getCommonCoreGapModel();
    const qint64 commonCoreGapModelLength = MsaRowUtils::getGapsLength(commonCoreGapModel);

    removeTrailingGapsInDifference(getPredictedSequenceDataLength(), commonCoreGapModelLength, predictedSequenceGapModelDifference);
    removeTrailingGapsInDifference(getEditedSequenceDataLength(), commonCoreGapModelLength, editedSequenceGapModelDifference);

    const qint64 rowLength = getCoreStart() + getPredictedSequenceDataLength() + getPredictedSequenceGuaranteedGapsLength();
    MsaRowUtils::chopGapModel(commonGapModel, rowLength);
    updateCachedGapModels();
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

qint64 MultipleChromatogramAlignmentRowData::mapVisiblePositionToCorePosition(qint64 mcaVisiblePosition) const {
    const qint64 realPosition = mapVisiblePositionToRealPosition(mcaVisiblePosition);
    CHECK(getCoreRegion().contains(realPosition), -1);
    return realPosition - getCoreStart();
}

qint64 MultipleChromatogramAlignmentRowData::mapVisiblePositionToRealPosition(qint64 mcaVisiblePosition) const {
    return mcaVisiblePosition + workingArea.startPos;
}

U2MsaRowGapModel MultipleChromatogramAlignmentRowData::getPredictedSequenceCoreGapModel() const {
    U2MsaRowGapModel predictedSequenceCoreGapModel = predictedSequenceCachedGapModel;
    MsaRowUtils::chopGapModel(predictedSequenceCoreGapModel, getCoreRegion());
    return predictedSequenceCoreGapModel;
}

U2MsaRowGapModel MultipleChromatogramAlignmentRowData::getPredictedSequenceWorkingAreaGapModel() const {
    U2MsaRowGapModel predictedSequenceWorkingAreaGapModel = predictedSequenceCachedGapModel;
    MsaRowUtils::chopGapModel(predictedSequenceWorkingAreaGapModel, getWorkingAreaRegion());
    return predictedSequenceWorkingAreaGapModel;
}

U2MsaRowGapModel MultipleChromatogramAlignmentRowData::getEditedSequenceCoreGapModel() const {
    U2MsaRowGapModel editedSequenceCoreGapModel = editedSequenceCachedGapModel;
    MsaRowUtils::chopGapModel(editedSequenceCoreGapModel, getCoreRegion());
    return editedSequenceCoreGapModel;
}

U2MsaRowGapModel MultipleChromatogramAlignmentRowData::getEditedSequenceWorkingAreaGapModel() const {
    U2MsaRowGapModel editedSequenceWorkingAreaGapModel = editedSequenceCachedGapModel;
    MsaRowUtils::chopGapModel(editedSequenceWorkingAreaGapModel, getWorkingAreaRegion());
    return editedSequenceWorkingAreaGapModel;
}

U2MsaRowGapModel MultipleChromatogramAlignmentRowData::getCommonCoreGapModel() const {
    U2MsaRowGapModel commonCoreGapModel = commonGapModel;
    MsaRowUtils::chopGapModel(commonCoreGapModel, getCoreRegion());
    return commonCoreGapModel;
}

void MultipleChromatogramAlignmentRowData::replaceGapToCharInEditedSequence(U2OpStatus &os, qint64 corePosition, char newChar) {
    const qint64 dataPosition = MsaRowUtils::getUngappedPosition(editedSequenceCachedGapModel, getEditedSequenceDataLength(), corePosition, true);
    SAFE_POINT(0 <= dataPosition && dataPosition < editedSequence.length(), "The edited sequence data position is out of boundaries", );
    editedSequence.seq.insert(dataPosition + 1, newChar);

    if (U2Msa::GAP_CHAR == getPredictedSequenceCoreChar(corePosition)) {
        MsaRowUtils::removeGaps(os, commonGapModel, getRowLengthWithoutTrailing(), corePosition + getCoreStart(), 1);
        MsaRowUtils::insertGaps(os, predictedSequenceGapModelDifference, getCoreLength(), corePosition, 1);
    } else {
        MsaRowUtils::insertGaps(os, editedSequenceGapModelDifference, getCoreLength(), corePosition, 1);
    }
    updateCachedGapModels();
}

void MultipleChromatogramAlignmentRowData::replaceCharToGapInEditedSequence(U2OpStatus &os, qint64 corePosition, char newChar) {
    const qint64 dataPosition = MsaRowUtils::getUngappedPosition(editedSequenceCachedGapModel, getEditedSequenceDataLength(), corePosition);
    SAFE_POINT(-1 != dataPosition, "Expect a symbol on the requested poosition, got a gap", );
    SAFE_POINT(0 <= dataPosition && dataPosition < editedSequence.length(), "The edited sequence data position is out of boundaries", );
    editedSequence.seq.remove(dataPosition, 1);

    if (U2Msa::GAP_CHAR == getPredictedSequenceCoreChar(corePosition)) {
        MsaRowUtils::insertGaps(os, commonGapModel, getRowLengthWithoutTrailing(), corePosition + getCoreStart(), 1);
        MsaRowUtils::removeGaps(os, predictedSequenceGapModelDifference, getCoreLength(), corePosition, 1);
    } else {
        MsaRowUtils::insertGaps(os, editedSequenceGapModelDifference, getCoreLength(), corePosition, 1);
    }
    updateCachedGapModels();
}

void MultipleChromatogramAlignmentRowData::replaceCharToCharInEditedSequence(U2OpStatus &os, qint64 corePosition, char newChar) {
    const qint64 dataPosition = MsaRowUtils::getUngappedPosition(editedSequenceCachedGapModel, getEditedSequenceDataLength(), corePosition);
    SAFE_POINT(-1 != dataPosition, "Expect a symbol on the requested poosition, got a gap", );
    SAFE_POINT(0 <= dataPosition && dataPosition < editedSequence.length(), "The edited sequence data position is out of boundaries", );
    editedSequence.seq[static_cast<int>(dataPosition)] = newChar;
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
