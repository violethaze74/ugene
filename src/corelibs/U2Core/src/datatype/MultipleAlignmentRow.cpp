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

#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>

#include "MultipleAlignmentRow.h"
#include "MultipleSequenceAlignment.h"

namespace U2 {

const qint64 MultipleAlignmentRowData::INVALID_ROW_ID = -1;

MultipleAlignmentRowData::MultipleAlignmentRowData(const MultipleSequenceAlignment *alignment)
    : alignment(alignment),
      initialRowInDb(U2MaRow())
{
    initialRowInDb.rowId = INVALID_ROW_ID;
}

MultipleAlignmentRowData::MultipleAlignmentRowData(const MultipleAlignmentRow &row, const MultipleSequenceAlignment *alignment)
    : alignment(alignment),
      gaps(row->gaps),
      initialRowInDb(row->initialRowInDb)
{
    SAFE_POINT(alignment != NULL, "Parent MultipleAlignment is NULL", );
}

MultipleAlignmentRowData::MultipleAlignmentRowData(const U2MaRow &rowInDb, const U2MaRowGapModel &gaps, const MultipleSequenceAlignment *alignment)
    : alignment(alignment),
      gaps(gaps),
      initialRowInDb(rowInDb)
{
    SAFE_POINT(alignment != NULL, "Parent MultipleAlignment is NULL", );
}

MultipleAlignmentRowData::~MultipleAlignmentRowData() {

}

const U2MaRowGapModel & MultipleAlignmentRowData::getGapModel() const {
    return gaps;
}

void MultipleAlignmentRowData::setGapModel(const U2MaRowGapModel &newGapModel) {
    gaps = newGapModel;
    removeTrailingGaps();
}

qint64 MultipleAlignmentRowData::getRowId() const {
    return initialRowInDb.rowId;
}

void MultipleAlignmentRowData::setRowId(qint64 rowId) {
    initialRowInDb.rowId = rowId;
}

void MultipleAlignmentRowData::setDataId(const U2DataId &dataId) {
    initialRowInDb.dataObjectId = dataId;
}

U2MaRow MultipleAlignmentRowData::getRowDbInfo() const {
    U2MaRow row;
    row.rowId = initialRowInDb.rowId;
    row.dataObjectId = initialRowInDb.dataObjectId;
    row.gstart = 0;
    row.gend = getCoreLength();
    row.gaps = gaps;
    row.length = getRowLengthWithoutTrailing();
    return row;
}

void MultipleAlignmentRowData::setRowDbInfo(const U2MaRow &dbRow) {
    initialRowInDb = dbRow;
}

int MultipleAlignmentRowData::getRowLength() const {
    SAFE_POINT(alignment != NULL, "Parent MultipleAlignment is NULL", getRowLengthWithoutTrailing());
    return alignment->getLength();
}

int MultipleAlignmentRowData::getRowLengthWithoutTrailing() const {
    return getDataLength() + MsaRowUtils::getGapsLength(gaps);
}

int MultipleAlignmentRowData::getCoreEnd() const {
    return getRowLengthWithoutTrailing();
}

int MultipleAlignmentRowData::getCoreStart() const {
    return MsaRowUtils::getCoreStart(gaps);
}

int MultipleAlignmentRowData::getCoreLength() const {
    int coreStart = getCoreStart();
    int coreEnd = getCoreEnd();
    int length = coreEnd - coreStart;
    SAFE_POINT(length >= 0, QString("Internal error in MAlignamentRow: coreEnd is %1, coreStart is %2").arg(coreEnd).arg(coreStart), length);
    return length;
}

int MultipleAlignmentRowData::getUngappedLength() const {
    return getDataLength();
}

bool MultipleAlignmentRowData::simplify() {
    if (gaps.count() > 0) {
        gaps.clear();
        return true;
    }
    return false;
}

void MultipleAlignmentRowData::append(const MultipleAlignmentRow &anotherRow, int lengthBefore, U2OpStatus &os) {
    int rowLength = getRowLengthWithoutTrailing();

    if (lengthBefore < rowLength) {
        coreLog.trace(QString("Internal error: incorrect length '%1' were passed to MultipleAlignmentRowData::append, coreEnd is '%2'").arg(lengthBefore).arg(getCoreEnd()));
        os.setError("Failed to append one row to another");
        return;
    }

    // Gap between rows
    if (lengthBefore > rowLength) {
        gaps.append(U2MaGap(getRowLengthWithoutTrailing(), lengthBefore - getRowLengthWithoutTrailing()));
    }

    // Merge gaps
    U2MaRowGapModel anotherRowGaps = anotherRow->getGapModel();
    MsaRowUtils::shiftGapModel(anotherRowGaps, lengthBefore);
    gaps.append(anotherRowGaps);
    mergeConsecutiveGaps();

    appendDataCore(anotherRow);
}

void MultipleAlignmentRowData::insertGaps(int pos, int count, U2OpStatus &os) {
    if (count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed to MultipleAlignmentRowData::insertGaps, pos '%1', count '%2'").arg(pos).arg(count));
        os.setError("Failed to insert gaps into a row");
        return;
    }

    if (pos < 0 || pos >= getRowLengthWithoutTrailing()) {
        return;
    }

    if (0 == pos) {
        MsaRowUtils::addOffsetToGapModel(gaps, count);
    } else {
        // A gap is near
        if (isGap(pos) || isGap(pos - 1)) {
            // Find the gaps and append 'count' gaps to it
            // Shift all gaps that further in the row
            for (int i = 0; i < gaps.count(); ++i) {
                if (pos >= gaps[i].offset) {
                    if (pos <= gaps[i].offset + gaps[i].gap) {
                        gaps[i].gap += count;
                    }
                } else {
                    gaps[i].offset += count;
                }
            }
        }
        // Insert between chars
        else {
            bool found = false;

            int indexGreaterGaps = 0;
            for (int i = 0; i < gaps.count(); ++i) {
                if (pos > gaps[i].offset + gaps[i].gap) {
                    continue;
                } else {
                    found = true;
                    U2MaGap newGap(pos, count);
                    gaps.insert(i, newGap);
                    indexGreaterGaps = i;
                    break;
                }
            }

            // shift all gaps after the new gap position
            // If found somewhere between existent gaps
            if (found) {
                // Shift further gaps
                for (int i = indexGreaterGaps + 1; i < gaps.count(); ++i) {
                    gaps[i].offset += count;
                }
            }
            // This is the last gap
            else {
                U2MaGap newGap(pos, count);
                gaps.append(newGap);
                return;
            }
        }
    }
}

void MultipleAlignmentRowData::removeData(int pos, int count, U2OpStatus &os) {
    if (pos < 0 || count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed to MultipleAlignmentRowData::removeChars, pos '%1', count '%2'").arg(pos).arg(count));
        os.setError("Can't remove chars from a row");
        return;
    }

    if (pos >= getRowLengthWithoutTrailing()) {
        return;
    }

    // Remove data
    if (pos < getRowLengthWithoutTrailing()) {
        int startPosInData = -1;
        int endPosInData = -1;
        getStartAndEndDataPositions(pos, count, startPosInData, endPosInData);

        // Remove inside a gap
        if ((startPosInData < endPosInData) && (-1 != startPosInData) && (-1 != endPosInData)) {
            removeDataCore(startPosInData, endPosInData, os);
            CHECK_OP(os, );
        }
    }

    // Remove gaps
    removeGapsFromGapModel(pos, count);
    removeTrailingGaps();
    mergeConsecutiveGaps();
}

int MultipleAlignmentRowData::getUngappedPosition(int pos) const {
    return MsaRowUtils::getUngappedPosition(gaps, pos);
}

int MultipleAlignmentRowData::getDataSize(int before) const {
    const int rowLength = getRowLengthWithoutTrailing();
    const int trimmedRowPos = (before < rowLength ? before : rowLength);
    return MsaRowUtils::getUngappedPosition(gaps, trimmedRowPos, true);
}

bool MultipleAlignmentRowData::isRowContentEqual(const MultipleAlignmentRow &row) const {
    if (!isDataEqual(row)) {
        return false;
    }

    U2MaRowGapModel firstRowGaps = gaps;
    if (!firstRowGaps.isEmpty() && (firstRowGaps.first().offset == 0)) {
        firstRowGaps.removeFirst();
    }

    U2MaRowGapModel secondRowGaps = row->getGapModel();
    if (!secondRowGaps.isEmpty() && (secondRowGaps.first().offset == 0)) {
        secondRowGaps.removeFirst();
    }

    return firstRowGaps == secondRowGaps;
}

bool MultipleAlignmentRowData::operator!=(const MultipleAlignmentRow &row) const {
    return !(*this == row);
}

bool MultipleAlignmentRowData::operator==(const MultipleAlignmentRow &row) const {
    return isRowContentEqual(row);
}

void MultipleAlignmentRowData::crop(int pos, int count, U2OpStatus &os) {
    if (pos < 0 || count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed to MultipleAlignmentRowData::crop, startPos '%1', length '%2', row length '%3'")
                      .arg(pos).arg(count).arg(getRowLength()));
        os.setError("Can't crop a row");
        return;
    }

    int initialRowLength = getRowLength();
    int initialSeqLength = getDataLength();

    if (pos >= getRowLengthWithoutTrailing()) {
        // Clear the row content
        removeDataCore(0, getDataLength(), os);
        CHECK_OP(os, );
    } else {
        int startPosInSeq = -1;
        int endPosInSeq = -1;
        getStartAndEndDataPositions(pos, count, startPosInSeq, endPosInSeq);

        // Remove inside a gap
        if ((startPosInSeq <= endPosInSeq) && (-1 != startPosInSeq) && (-1 != endPosInSeq)) {
            if (endPosInSeq < initialSeqLength) {
                removeDataCore(endPosInSeq, getDataLength(), os);
                CHECK_OP(os, );
            }

            if (startPosInSeq > 0) {
                removeDataCore(0, startPosInSeq, os);
                CHECK_OP(os, );
            }
        }
    }

    if (pos + count < initialRowLength) {
        removeGapsFromGapModel(pos + count, initialRowLength - pos - count);
    }

    if (pos > 0) {
        removeGapsFromGapModel(0, pos);
    }
    removeTrailingGaps();
}

void MultipleAlignmentRowData::removeTrailingGaps() {
    if (gaps.isEmpty()) {
        return;
    }

    MsaRowUtils::chopGapModel(gaps, getRowLengthWithoutTrailing());
}

void MultipleAlignmentRowData::mergeConsecutiveGaps() {
    U2MaRowGapModel newGapModel;
    if (gaps.isEmpty()) {
        return;
    }

    newGapModel << gaps[0];
    int indexInNewGapModel = 0;
    for (int i = 1; i < gaps.count(); ++i) {
        int previousGapEnd = newGapModel[indexInNewGapModel].offset + newGapModel[indexInNewGapModel].gap - 1;
        int currectGapStart = gaps[i].offset;
        SAFE_POINT(currectGapStart > previousGapEnd, "Incorrect gap model during merging consecutive gaps", );
        if (currectGapStart == previousGapEnd + 1) {
            // Merge gaps
            qint64 newGapLength = newGapModel[indexInNewGapModel].gap + gaps[i].gap;
            SAFE_POINT(newGapLength > 0, "Non-positive gap length", );
            newGapModel[indexInNewGapModel].gap = newGapLength;
        } else {
            // Add the gap to the list
            newGapModel << gaps[i];
            indexInNewGapModel++;
        }
    }
    gaps = newGapModel;
}

int MultipleAlignmentRowData::getGapsLength() const {
    return MsaRowUtils::getGapsLength(gaps);
}

void MultipleAlignmentRowData::getStartAndEndDataPositions(int pos, int count, int &startPosInSeq, int &endPosInSeq) {
    int rowLengthWithoutTrailingGap = getRowLengthWithoutTrailing();
    SAFE_POINT(pos < rowLengthWithoutTrailingGap,
        QString("Incorrect position '%1' in MultipleAlignmentRowData::getStartAndEndSequencePosition, row length without trailing gaps is '%2'")
               .arg(pos).arg(rowLengthWithoutTrailingGap), );

    // Remove chars from the sequence
    // Calculate start position in the sequence
    if (isGap(pos)) {
        int i = 1;
        while (isGap(pos + i)) {
            if (getRowLength() == pos + i) {
                break;
            }
            i++;
        }
        startPosInSeq = getUngappedPosition(pos + i);
    } else {
        startPosInSeq = getUngappedPosition(pos);
    }

    // Calculate end position in the sequence
    int endRegionPos = pos + count; // non-inclusive

    if (endRegionPos > rowLengthWithoutTrailingGap) {
        endRegionPos = rowLengthWithoutTrailingGap;
    }

    if (endRegionPos == rowLengthWithoutTrailingGap) {
        endPosInSeq = getDataLength();
    } else {
        if (isGap(endRegionPos)) {
            int i = 1;
            while (isGap(endRegionPos + i)) {
                if (getRowLength() == endRegionPos + i) {
                    break;
                }
                i++;
            }
            endPosInSeq = getUngappedPosition(endRegionPos + i);
        } else {
            endPosInSeq = getUngappedPosition(endRegionPos);
        }
    }
}

void MultipleAlignmentRowData::removeGapsFromGapModel(int pos, int count) {
    U2MaRowGapModel newGapModel;
    int endRegionPos = pos + count; // non-inclusive
    foreach (U2MaGap gap, gaps) {
        qint64 gapEnd = gap.offset + gap.gap;
        if (gapEnd < pos) {
            newGapModel << gap;
        } else if (gapEnd <= endRegionPos) {
            if (gap.offset < pos) {
                gap.gap = pos - gap.offset;
                newGapModel << gap;
            }
            // Otherwise just remove the gap (do not write to the new gap model)
        } else {
            if (gap.offset < pos) {
                gap.gap -= count;
                SAFE_POINT(gap.gap >= 0, "Non-positive gap length", );
                newGapModel << gap;
            } else if (gap.offset < endRegionPos) {
                gap.gap = gapEnd - endRegionPos;
                gap.offset = pos;
                SAFE_POINT(gap.gap > 0, "Non-positive gap length", );
                SAFE_POINT(gap.offset >= 0, "Negative gap offset", );
                newGapModel << gap;
            } else {
                // Shift the gap
                gap.offset -= count;
                SAFE_POINT(gap.offset >= 0, "Negative gap offset", );
                newGapModel << gap;
            }
        }
    }

    gaps = newGapModel;
}

void MultipleAlignmentRowData::setParentAlignment(const MultipleSequenceAlignment *newAl) {
    alignment = newAl;
}

bool MultipleAlignmentRowData::isGap(int position) const {
    return MsaRowUtils::isGap(gaps, position);
}

}   // namespace U2
