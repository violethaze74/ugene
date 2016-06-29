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

#include <U2Core/DNASequenceUtils.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>

#include "MultipleSequenceAlignment.h"
#include "MultipleSequenceAlignmentRow.h"

namespace U2 {

const qint64 MultipleSequenceAlignmentRow::INVALID_ROW_ID = -1;

MultipleSequenceAlignmentRow::MultipleSequenceAlignmentRow(const U2MsaRow &rowInDb,
                                                           const DNASequence &sequence,
                                                           const U2MaRowGapModel &gaps,
                                                           MultipleSequenceAlignment *alignment)
    : alignment(alignment),
      sequence(sequence),
      gaps(gaps),
      initialRowInDb(rowInDb)
{
    SAFE_POINT(alignment != NULL, "Parent MultipleSequenceAlignment is NULL", );
    removeTrailingGaps();
}

MultipleSequenceAlignmentRow::MultipleSequenceAlignmentRow(MultipleSequenceAlignment *al)
    : alignment(al),
      sequence(DNASequence()),
      initialRowInDb(U2MsaRow())
{
    initialRowInDb.rowId = INVALID_ROW_ID;
    removeTrailingGaps();
}

MultipleSequenceAlignmentRow::MultipleSequenceAlignmentRow(const MultipleSequenceAlignmentRow &row, MultipleSequenceAlignment *al)
    : alignment(al),
      sequence(row.sequence),
      gaps(row.gaps),
      initialRowInDb(row.initialRowInDb)
{
    SAFE_POINT(alignment != NULL, "Parent MultipleSequenceAlignment is NULL", );
}

QByteArray MultipleSequenceAlignmentRow::toByteArray(int length, U2OpStatus &os) const {
    if (length < getCoreEnd()) {
        coreLog.trace("Incorrect length was passed to MultipleSequenceAlignmentRow::toByteArray");
        os.setError("Failed to get row data");
        return QByteArray();
    }

    if (gaps.isEmpty() && sequence.length() == length) {
        return sequence.constSequence();
    }

    QByteArray bytes = joinCharsAndGaps(true, true);

    // Append additional gaps, if necessary
    if (length > bytes.count()) {
        QByteArray gapsBytes;
        gapsBytes.fill(MultipleSequenceAlignment::GapChar, length - bytes.count());
        bytes.append(gapsBytes);
    }
    if (length < bytes.count()) {
        // cut extra trailing gaps
        bytes = bytes.left(length);
    }

    return bytes;
}

int MultipleSequenceAlignmentRow::getRowLength() const {
    SAFE_POINT(alignment != NULL, "Parent MultipleSequenceAlignment is NULL", getRowLengthWithoutTrailing());
    return alignment->getLength();
}

QByteArray MultipleSequenceAlignmentRow::getCore() const {
    return joinCharsAndGaps(false, false);
}

QByteArray MultipleSequenceAlignmentRow::getData() const {
    return joinCharsAndGaps(true, true);
}

void MultipleSequenceAlignmentRow::splitBytesToCharsAndGaps(const QByteArray &input, QByteArray &seqBytes, U2MaRowGapModel &gapsModel) {
    MsaDbiUtils::splitBytesToCharsAndGaps(input, seqBytes, gapsModel);
}

void MultipleSequenceAlignmentRow::addOffsetToGapModel(U2MaRowGapModel &gapModel, int offset) {
    if (0 == offset) {
        return;
    }

    if (!gapModel.isEmpty()) {

        U2MaGap &firstGap = gapModel[0];
        if (0 == firstGap.offset) {
            firstGap.gap += offset;
        } else {
            SAFE_POINT(offset >= 0, "Negative gap offset", );
            U2MaGap beginningGap(0, offset);
            gapModel.insert(0, beginningGap);
        }

        // Shift other gaps
        if (gapModel.count() > 1) {
            for (int i = 1; i < gapModel.count(); ++i) {
                qint64 newOffset = gapModel[i].offset + offset;
                SAFE_POINT(newOffset >= 0, "Negative gap offset", );
                gapModel[i].offset = newOffset;
            }
        }
    } else {
        SAFE_POINT(offset >= 0, "Negative gap offset", );
        U2MaGap gap(0, offset);
        gapModel.append(gap);
    }
}

QByteArray MultipleSequenceAlignmentRow::joinCharsAndGaps(bool keepOffset, bool keepTrailingGaps) const {
    QByteArray bytes = sequence.constSequence();
    int beginningOffset = 0;

    if (gaps.isEmpty()) {
        return bytes;
    }

    for (int i = 0; i < gaps.size(); ++i) {
        QByteArray gapsBytes;
        if (!keepOffset && (0 == gaps[i].offset)) {
            beginningOffset = gaps[i].gap;
            continue;
        }

        gapsBytes.fill(MultipleSequenceAlignment::GapChar, gaps[i].gap);
        bytes.insert(gaps[i].offset - beginningOffset, gapsBytes);
    }
    SAFE_POINT(alignment != NULL, "Parent MultipleSequenceAlignment is NULL", QByteArray());
    if (keepTrailingGaps && bytes.size() < alignment->getLength()) {
        QByteArray gapsBytes;
        gapsBytes.fill(MultipleSequenceAlignment::GapChar, alignment->getLength() - bytes.size());
        bytes.append(gapsBytes);
    }

    return bytes;
}

int MultipleSequenceAlignmentRow::getCoreLength() const {
    int coreStart = getCoreStart();
    int coreEnd = getCoreEnd();
    int length = coreEnd - coreStart;
    SAFE_POINT(length >= 0, QString("Internal error in MAlignamentRow: coreEnd is %1, coreStart is %2").arg(coreEnd).arg(coreStart), length);
    return length;
}

void MultipleSequenceAlignmentRow::append(const MultipleSequenceAlignmentRow &anotherRow, int lengthBefore, U2OpStatus &os) {
    int rowLength = getRowLengthWithoutTrailing();

    if (lengthBefore < rowLength) {
        coreLog.trace(QString("Internal error: incorrect length '%1' were passed to MultipleSequenceAlignmentRow::append, coreEnd is '%2'").arg(lengthBefore).arg(getCoreEnd()));
        os.setError("Failed to append one row to another");
        return;
    }

    // Gap between rows
    if (lengthBefore > rowLength) {
        gaps.append(U2MaGap(getRowLengthWithoutTrailing(), lengthBefore - getRowLengthWithoutTrailing()));
    }

    // Merge gaps
    U2MaRowGapModel anotherRowGaps = anotherRow.getGapModel();
    for (int i = 0; i < anotherRowGaps.count(); ++i) {
        anotherRowGaps[i].offset += lengthBefore;
    }
    gaps.append(anotherRowGaps);
    mergeConsecutiveGaps();

    // Merge sequences
    DNASequenceUtils::append(sequence, anotherRow.sequence, os);
}

U2MsaRow MultipleSequenceAlignmentRow::getRowDbInfo() const {
    U2MsaRow row;
    row.rowId = initialRowInDb.rowId;
    row.sequenceId = initialRowInDb.sequenceId;
    row.gstart = 0;
    row.gend = sequence.length();
    row.gaps = gaps;
    row.length = getRowLengthWithoutTrailing();
    return row;
}

void MultipleSequenceAlignmentRow::setRowDbInfo(const U2MsaRow &dbRow) {
    initialRowInDb = dbRow;
}

void MultipleSequenceAlignmentRow::setRowContent(const QByteArray &bytes, int offset) {
    QByteArray newSequenceBytes;
    U2MaRowGapModel newGapsModel;

    splitBytesToCharsAndGaps(bytes, newSequenceBytes, newGapsModel);
    DNASequence newSequence(getName(), newSequenceBytes);

    addOffsetToGapModel(newGapsModel, offset);

    sequence = newSequence;
    gaps = newGapsModel;
    removeTrailingGaps();
}

QString MultipleSequenceAlignmentRow::getName() const {
    return sequence.getName();
}

void MultipleSequenceAlignmentRow::setName(const QString &name) {
    sequence.setName(name);
}

void MultipleSequenceAlignmentRow::setGapModel(const U2MaRowGapModel &newGapModel) {
    gaps = newGapModel;
    removeTrailingGaps();
}

void MultipleSequenceAlignmentRow::setSequence(const DNASequence &newSequence) {
    SAFE_POINT(!newSequence.constSequence().contains(MultipleSequenceAlignment::GapChar), "The sequence must be without gaps", );
    sequence = newSequence;
}

qint64 MultipleSequenceAlignmentRow::getRowId() const {
    return initialRowInDb.rowId;
}

void MultipleSequenceAlignmentRow::setRowId(qint64 rowId) {
    initialRowInDb.rowId = rowId;
}

void MultipleSequenceAlignmentRow::setSequenceId(const U2DataId &sequenceId) {
    initialRowInDb.sequenceId = sequenceId;
}

char MultipleSequenceAlignmentRow::charAt(int pos) const {
    return MsaRowUtils::charAt(sequence.seq, gaps, pos);
}

void MultipleSequenceAlignmentRow::insertGaps(int pos, int count, U2OpStatus &os) {
    if (count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed to MultipleSequenceAlignmentRow::insertGaps, pos '%1', count '%2'").arg(pos).arg(count));
        os.setError("Failed to insert gaps into a row");
        return;
    }

    if (pos < 0 || pos >= getRowLengthWithoutTrailing()) {
        return;
    }

    if (0 == pos) {
        addOffsetToGapModel(gaps, count);
    } else {
        // A gap is near
        if (MultipleSequenceAlignment::GapChar == charAt(pos) ||
                MultipleSequenceAlignment::GapChar == charAt(pos - 1)) {
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
                }
                else {
                    found = true;
                    U2MaGap newGap(pos, count);
                    gaps.insert(i, newGap);
                    indexGreaterGaps = i;
                    break;
                }
            }

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

void MultipleSequenceAlignmentRow::mergeConsecutiveGaps() {
    U2MaRowGapModel newGapModel;
    if (gaps.isEmpty()) {
        return;
    }

    newGapModel << gaps[0];
    int indexInNewGapModel = 0;
    for (int i = 1; i < gaps.count(); ++i) {
        int previousGapEnd = newGapModel[indexInNewGapModel].offset + newGapModel[indexInNewGapModel].gap - 1;
        int currectGapStart = gaps[i].offset;
        SAFE_POINT(currectGapStart > previousGapEnd, "Incorrect gap model during merging consecutive gaps",);
        if (currectGapStart == previousGapEnd + 1) {
            // Merge gaps
            qint64 newGapLength = newGapModel[indexInNewGapModel].gap + gaps[i].gap;
            SAFE_POINT(newGapLength > 0, "Non-positive gap length", )
            newGapModel[indexInNewGapModel].gap = newGapLength;
        } else {
            // Add the gap to the list
            newGapModel << gaps[i];
            indexInNewGapModel++;
        }
    }
    gaps = newGapModel;
}

void MultipleSequenceAlignmentRow::removeTrailingGaps() {
    if (gaps.isEmpty()) {
        return;
    }

    // If the last char in the row is gap, remove the last gap
    if (MultipleSequenceAlignment::GapChar == charAt(MsaRowUtils::getRowLength(sequence.constData(), gaps) - 1)) {
        gaps.removeLast();
    }
}

void MultipleSequenceAlignmentRow::removeChars(int pos, int count, U2OpStatus &os) {
    if (pos < 0 || count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed to MultipleSequenceAlignmentRow::removeChars, pos '%1', count '%2'").arg(pos).arg(count));
        os.setError("Can't remove chars from a row");
        return;
    }

    if (pos >= getRowLengthWithoutTrailing()) {
        return;
    }

    if (pos < getRowLengthWithoutTrailing()) {
        int startPosInSeq = -1;
        int endPosInSeq = -1;
        getStartAndEndSequencePositions(pos, count, startPosInSeq, endPosInSeq);

        // Remove inside a gap
        if ((startPosInSeq < endPosInSeq) && (-1 != startPosInSeq) && (-1 != endPosInSeq)) {
            DNASequenceUtils::removeChars(sequence, startPosInSeq, endPosInSeq, os);
            CHECK_OP(os, );
        }
    }

    // Remove gaps from the gaps model
    removeGapsFromGapModel(pos, count);

    removeTrailingGaps();
    mergeConsecutiveGaps();
}

bool MultipleSequenceAlignmentRow::isRowContentEqual(const MultipleSequenceAlignmentRow &row) const {
    if (MatchExactly == DNASequenceUtils::compare(sequence, row.getSequence())) {
        if (sequence.length() == 0) {
            return true;
        } else {
            U2MaRowGapModel firstRowGaps = gaps;
            if  (!firstRowGaps.isEmpty() && (MultipleSequenceAlignment::GapChar == charAt(0))) {
                firstRowGaps.removeFirst();
            }

            U2MaRowGapModel secondRowGaps = row.getGapModel();
            if (!secondRowGaps.isEmpty() && (MultipleSequenceAlignment::GapChar == row.charAt(0))) {
                secondRowGaps.removeFirst();
            }

            if (firstRowGaps == secondRowGaps) {
                return true;
            }
        }
    }

    return false;
}

int MultipleSequenceAlignmentRow::getUngappedPosition(int pos) const {
    return MsaRowUtils::getUngappedPosition(sequence.seq, gaps, pos);
}

int MultipleSequenceAlignmentRow::getBaseCount(int before) const {
    const int rowLength = MsaRowUtils::getRowLength(sequence.seq, gaps);
    const int trimmedRowPos = before < rowLength ? before : rowLength;
    return MsaRowUtils::getUngappedPosition(sequence.seq, gaps, trimmedRowPos, true);
}

void MultipleSequenceAlignmentRow::getStartAndEndSequencePositions(int pos, int count, int &startPosInSeq, int &endPosInSeq) {
    int rowLengthWithoutTrailingGap = getRowLengthWithoutTrailing();
    SAFE_POINT(pos < rowLengthWithoutTrailingGap,
        QString("Incorrect position '%1' in MultipleSequenceAlignmentRow::getStartAndEndSequencePosition, row length without trailing gaps is '%2'!")
               .arg(pos).arg(rowLengthWithoutTrailingGap), );

    // Remove chars from the sequence
    // Calculate start position in the sequence
    if (MultipleSequenceAlignment::GapChar == charAt(pos)) {
        int i = 1;
        while (MultipleSequenceAlignment::GapChar == charAt(pos + i)) {
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
        endPosInSeq = getUngappedLength();
    } else {
        if (MultipleSequenceAlignment::GapChar == charAt(endRegionPos)) {
            int i = 1;
            while (MultipleSequenceAlignment::GapChar == charAt(endRegionPos + i)) {
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

void MultipleSequenceAlignmentRow::removeGapsFromGapModel(int pos, int count) {
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

void MultipleSequenceAlignmentRow::setParentAlignment(MultipleSequenceAlignment *newAl) {
    alignment = newAl;
}

void MultipleSequenceAlignmentRow::crop(int pos, int count, U2OpStatus &os) {
    if (pos < 0 || count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed to MultipleSequenceAlignmentRow::crop, startPos '%1', length '%2', row length '%3'!")
                      .arg(pos).arg(count).arg(getRowLength()));
        os.setError("Can't crop a row");
        return;
    }

    int initialRowLength = getRowLength();
    int initialSeqLength = getUngappedLength();

    if (pos >= getRowLengthWithoutTrailing()) {
        // Clear the row content
        DNASequenceUtils::makeEmpty(sequence);
    } else {
        int startPosInSeq = -1;
        int endPosInSeq = -1;
        getStartAndEndSequencePositions(pos, count, startPosInSeq, endPosInSeq);

        // Remove inside a gap
        if ((startPosInSeq <= endPosInSeq) && (-1 != startPosInSeq) && (-1 != endPosInSeq)) {
            if (endPosInSeq < initialSeqLength) {
                DNASequenceUtils::removeChars(sequence, endPosInSeq, getUngappedLength(), os);
                CHECK_OP(os, );
            }

            if (startPosInSeq > 0) {
                DNASequenceUtils::removeChars(sequence, 0, startPosInSeq, os);
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

MultipleSequenceAlignmentRow MultipleSequenceAlignmentRow::mid(int pos, int count, U2OpStatus &os) const {
    MultipleSequenceAlignmentRow row = *this;
    row.crop(pos, count, os);
    return row;
}

bool MultipleSequenceAlignmentRow::operator==(const MultipleSequenceAlignmentRow &row) const {
    return isRowContentEqual(row);
}

void MultipleSequenceAlignmentRow::toUpperCase() {
    DNASequenceUtils::toUpperCase(sequence);
}

bool gapLessThan(const U2MaGap &gap1, const U2MaGap &gap2) {
    return gap1.offset < gap2.offset;
}

void MultipleSequenceAlignmentRow::replaceChars(char origChar, char resultChar, U2OpStatus &os) {
    if (MultipleSequenceAlignment::GapChar == origChar) {
        coreLog.trace("The original char can't be a gap in MultipleSequenceAlignmentRow::replaceChars");
        os.setError("Failed to replace chars in an alignment row");
        return;
    }

    if (MultipleSequenceAlignment::GapChar == resultChar) {
        // Get indexes of all 'origChar' characters in the row sequence
        QList<int> gapsIndexes;
        for (int i = 0; i < getRowLength(); i++) {
            if (origChar == charAt(i)) {
                gapsIndexes.append(i);
            }
        }

        if (gapsIndexes.isEmpty()) {
            return; // There is nothing to replace
        }

        // Remove all 'origChar' characters from the row sequence
        sequence.seq.replace(origChar, "");

        // Re-calculate the gaps model
        U2MaRowGapModel newGapsModel = gaps;
        for (int i = 0; i < gapsIndexes.size(); ++i) {
            int index = gapsIndexes[i];
            U2MaGap gap(index, 1);
            newGapsModel.append(gap);
        }
        qSort(newGapsModel.begin(), newGapsModel.end(), gapLessThan);

        // Replace the gaps model with the new one
        gaps = newGapsModel;
        mergeConsecutiveGaps();
    } else {
        // Just replace all occurrences of 'origChar' by 'resultChar'
        sequence.seq.replace(origChar, resultChar);
    }
}

}   // namespace U2
