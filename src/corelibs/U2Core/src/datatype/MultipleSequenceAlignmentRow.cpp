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

MultipleSequenceAlignmentRow::MultipleSequenceAlignmentRow()
    : QSharedDataPointer<MultipleSequenceAlignmentRowData>(new MultipleSequenceAlignmentRowData)
{
    init();
}

MultipleSequenceAlignmentRow::MultipleSequenceAlignmentRow(const MultipleAlignmentRow &maRow)
    : QSharedDataPointer<MultipleSequenceAlignmentRowData>(dynamic_cast<MultipleSequenceAlignmentRowData *>(const_cast<MultipleAlignmentRowData *>(maRow.constData())))
{
    init();
}

MultipleSequenceAlignmentRow::MultipleSequenceAlignmentRow(const MultipleSequenceAlignmentRow &msaRow)
    : QSharedDataPointer<MultipleSequenceAlignmentRowData>(msaRow)
{
    init();
}

MultipleSequenceAlignmentRow::MultipleSequenceAlignmentRow(MultipleSequenceAlignmentRowData *msaRowData)
    : QSharedDataPointer<MultipleSequenceAlignmentRowData>(msaRowData)
{
    init();
}

void MultipleSequenceAlignmentRow::init() {
    if (NULL == constData()) {
        *this = new MultipleSequenceAlignmentRowData();
    }
}

MultipleSequenceAlignmentRow::operator MultipleAlignmentRow() const {
    return MultipleAlignmentRow(const_cast<MultipleSequenceAlignmentRowData *>(constData()));
}

MultipleSequenceAlignmentRowData::MultipleSequenceAlignmentRowData(const MultipleSequenceAlignment *msa)
    : MultipleAlignmentRowData(msa)
{

}

MultipleSequenceAlignmentRowData::MultipleSequenceAlignmentRowData(const U2MaRow &rowInDb,
                                                                   const DNASequence &sequence,
                                                                   const U2MaRowGapModel &gaps,
                                                                   const MultipleSequenceAlignment *msa)
    : MultipleAlignmentRowData(rowInDb, gaps, msa),
      sequence(sequence)
{
    removeTrailingGaps();
}

MultipleSequenceAlignmentRowData::MultipleSequenceAlignmentRowData(const U2MaRow &rowInDb, const QString &rowName, const QByteArray &rawData, const MultipleSequenceAlignment *msa)
    : MultipleAlignmentRowData(rowInDb, U2MaRowGapModel(), msa)
{
    QByteArray sequenceData;
    U2MaRowGapModel gapModel;
    MsaDbiUtils::splitBytesToCharsAndGaps(rawData, sequenceData, gapModel);
    sequence = DNASequence(rowName, sequenceData);
    setGapModel(gapModel);
}

MultipleSequenceAlignmentRowData::MultipleSequenceAlignmentRowData(const MultipleSequenceAlignmentRow &row, const MultipleSequenceAlignment *msa)
    : MultipleAlignmentRowData((MultipleAlignmentRow)row, msa),
      sequence(row->sequence)
{

}

QString MultipleSequenceAlignmentRowData::getName() const {
    return sequence.getName();
}

void MultipleSequenceAlignmentRowData::setName(const QString &name) {
    sequence.setName(name);
}

const DNASequence & MultipleSequenceAlignmentRowData::getSequence() const {
    return sequence;
}

void MultipleSequenceAlignmentRowData::setSequence(const DNASequence &newSequence) {
    SAFE_POINT(!newSequence.constSequence().contains(MultipleSequenceAlignment::GapChar), "The sequence must be without gaps", );
    sequence = newSequence;
}

QByteArray MultipleSequenceAlignmentRowData::toByteArray(int length, U2OpStatus &os) const {
    if (length < getCoreEnd()) {
        coreLog.trace("Incorrect length was passed to MultipleSequenceAlignmentRowData::toByteArray");
        os.setError("Failed to get row data");
        return QByteArray();
    }

    const U2MaRowGapModel &gapModel = getGapModel();

    if (gapModel.isEmpty() && sequence.length() == length) {
        return sequence.constSequence();
    }

    QByteArray bytes = joinCharsAndGaps(true, true);

    // Append additional gaps, if necessary
    if (length > bytes.count()) {
        QByteArray gapsBytes;
        gapsBytes.fill(MultipleSequenceAlignment::GapChar, length - bytes.count());
        bytes.append(gapsBytes);
    } else if (length < bytes.count()) {
        // cut extra bytes
        bytes = bytes.left(length);
    }

    return bytes;
}

QByteArray MultipleSequenceAlignmentRowData::getCore() const {
    return joinCharsAndGaps(false, false);
}

QByteArray MultipleSequenceAlignmentRowData::getData() const {
    return joinCharsAndGaps(true, true);
}

void MultipleSequenceAlignmentRowData::setRowContent(const QByteArray &bytes, int offset) {
    QByteArray newSequenceBytes;
    U2MaRowGapModel newGapsModel;

    MsaDbiUtils::splitBytesToCharsAndGaps(bytes, newSequenceBytes, newGapsModel);
    DNASequence newSequence(getName(), newSequenceBytes);

    MsaRowUtils::addOffsetToGapModel(newGapsModel, offset);

    sequence = newSequence;
    setGapModel(newGapsModel);
    removeTrailingGaps();
}

char MultipleSequenceAlignmentRowData::charAt(int pos) const {
    return MsaRowUtils::charAt(sequence.seq, getGapModel(), pos);
}

void MultipleSequenceAlignmentRowData::toUpperCase() {
    DNASequenceUtils::toUpperCase(sequence);
}

void MultipleSequenceAlignmentRowData::replaceChars(char origChar, char resultChar, U2OpStatus &os) {
    if (MultipleSequenceAlignment::GapChar == origChar) {
        coreLog.trace("The original char can't be a gap in MultipleSequenceAlignmentRowData::replaceChars");
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
        U2MaRowGapModel newGapModel = getGapModel();
        for (int i = 0; i < gapsIndexes.size(); ++i) {
            int index = gapsIndexes[i];
            U2MaGap gap(index, 1);
            newGapModel.append(gap);
        }
        qSort(newGapModel.begin(), newGapModel.end(), U2MaGap::lessThan);

        // Replace the gaps model with the new one
        setGapModel(newGapModel);
        mergeConsecutiveGaps();
    } else {
        // Just replace all occurrences of 'origChar' by 'resultChar'
        sequence.seq.replace(origChar, resultChar);
    }
}

MultipleSequenceAlignmentRow MultipleSequenceAlignmentRowData::mid(int pos, int count, U2OpStatus &os) const {
    MultipleSequenceAlignmentRow row(new MultipleSequenceAlignmentRowData(*this));
    row->crop(pos, count, os);
    return row;
}

MultipleAlignmentRowData * MultipleSequenceAlignmentRowData::clone() const {
    return new MultipleSequenceAlignmentRowData(*this);
}

int MultipleSequenceAlignmentRowData::getDataLength() const {
    return sequence.length();
}

void MultipleSequenceAlignmentRowData::appendDataCore(const MultipleAlignmentRow &anotherRow) {
    const MultipleSequenceAlignmentRowData *msaRow = dynamic_cast<const MultipleSequenceAlignmentRowData *>(anotherRow.data());
    DNASequenceUtils::append(sequence, msaRow->sequence);
}

void MultipleSequenceAlignmentRowData::removeDataCore(int startPosInData, int endPosInData, U2OpStatus &os) {
    DNASequenceUtils::removeChars(sequence, startPosInData, endPosInData, os);
}

bool MultipleSequenceAlignmentRowData::isDataEqual(const MultipleAlignmentRow &row) const {
    const MultipleSequenceAlignmentRowData *msaRow = dynamic_cast<const MultipleSequenceAlignmentRowData *>(row.data());
    return MatchExactly == DNASequenceUtils::compare(sequence, msaRow->sequence);
}

QByteArray MultipleSequenceAlignmentRowData::joinCharsAndGaps(bool keepOffset, bool keepTrailingGaps) const {
    QByteArray bytes = sequence.constSequence();
    int beginningOffset = 0;

    const U2MaRowGapModel &gapModel = getGapModel();

    if (gapModel.isEmpty()) {
        return bytes;
    }

    for (int i = 0; i < gapModel.size(); ++i) {
        QByteArray gapsBytes;
        if (!keepOffset && (0 == gapModel[i].offset)) {
            beginningOffset = gapModel[i].gap;
            continue;
        }

        gapsBytes.fill(MultipleSequenceAlignment::GapChar, gapModel[i].gap);
        bytes.insert(gapModel[i].offset - beginningOffset, gapsBytes);
    }

    if (keepTrailingGaps && (bytes.size() < getRowLength())) {
        QByteArray gapsBytes;
        gapsBytes.fill(MultipleSequenceAlignment::GapChar, getRowLength() - bytes.size());
        bytes.append(gapsBytes);
    }

    return bytes;
}

}   // namespace U2
