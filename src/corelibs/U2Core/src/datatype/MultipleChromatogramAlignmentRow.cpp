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

#include <U2Core/ChromatogramUtils.h>
#include <U2Core/DNAChromatogram.h>
#include <U2Core/DNASequence.h>
#include <U2Core/DNASequenceUtils.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>

#include "MultipleChromatogramAlignment.h"
#include "MultipleChromatogramAlignmentRow.h"

namespace U2 {

MultipleChromatogramAlignmentRowData::MultipleChromatogramAlignmentRowData()
    : MultipleAlignmentRowData()
{

}

MultipleChromatogramAlignmentRowData::MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentData *mca)
    : MultipleAlignmentRowData(mca)
{

}

MultipleChromatogramAlignmentRowData::MultipleChromatogramAlignmentRowData(const U2MaRow &rowInDb,
                                                                           const DNAChromatogram &chromatogram,
                                                                           const U2MaRowGapModel &gaps,
                                                                           const MultipleChromatogramAlignmentData *mca)
    : MultipleAlignmentRowData(rowInDb, gaps, mca),
      chromatogram(chromatogram)
{
    removeTrailingGaps();
}

MultipleChromatogramAlignmentRowData::MultipleChromatogramAlignmentRowData(const MultipleChromatogramAlignmentRow &row, const MultipleChromatogramAlignmentData *mca)
    : MultipleAlignmentRowData(row, mca),
      name(row->name),
      chromatogram(row->chromatogram),
      predictedSequence(row->predictedSequence),
      editedSequence(row->editedSequence)
{

}

QString MultipleChromatogramAlignmentRowData::getName() const {
    return name;
}

void MultipleChromatogramAlignmentRowData::setName(const QString &newName) {
    name = newName;
}

const DNAChromatogram & MultipleChromatogramAlignmentRowData::getChromatogram() const {
    return chromatogram;
}

const DNASequence & MultipleChromatogramAlignmentRowData::getSequence(SequenceType sequenceType) const {
    static const DNASequence emptySequence;
    const DNASequence *sequence = selectSequence(sequenceType);
    SAFE_POINT(NULL != sequence, "An unknown sequence type", emptySequence);
    return *sequence;
}

void MultipleChromatogramAlignmentRowData::setChromatogram(const DNAChromatogram &newChromatogram) {
    chromatogram = newChromatogram;
}

void MultipleChromatogramAlignmentRowData::setSequence(SequenceType sequenceType, const DNASequence &newSequence) {
    DNASequence *sequence = selectSequence(sequenceType);
    SAFE_POINT(NULL != sequence, "An unknown sequence type", );
    *sequence = newSequence;
}

QByteArray MultipleChromatogramAlignmentRowData::toByteArray(SequenceType sequenceType, int length, U2OpStatus &os) const {
    if (length < getCoreEnd()) {
        coreLog.trace("Incorrect length was passed to MultipleChromatogramAlignmentRowData::sequenceToByteArray");
        os.setError("Failed to get row data");
        return QByteArray();
    }

    const DNASequence *sequence = selectSequence(sequenceType);
    SAFE_POINT_EXT(NULL != sequence, os.setError("An unknown sequence type"), "");

    const U2MaRowGapModel &gapModel = getGapModel();

    if (gapModel.isEmpty() && sequence->length() == length) {
        return sequence->constSequence();
    }

    QByteArray bytes = joinCharsAndGaps(sequenceType, true, true);

    // Append additional gaps, if necessary
    if (length > bytes.count()) {
        QByteArray gapsBytes;
        gapsBytes.fill(MultipleAlignmentData::GapChar, length - bytes.count());
        bytes.append(gapsBytes);
    } else if (length < bytes.count()) {
        // cut extra bytes
        bytes = bytes.left(length);
    }

    return bytes;
}

QByteArray MultipleChromatogramAlignmentRowData::getCore(SequenceType sequenceType) const {
    return joinCharsAndGaps(sequenceType, false, false);
}

QByteArray MultipleChromatogramAlignmentRowData::getData(SequenceType sequenceType) const {
    return joinCharsAndGaps(sequenceType, true, true);
}

char MultipleChromatogramAlignmentRowData::charAt(SequenceType sequenceType, int pos) const {
    const DNASequence *sequence = selectSequence(sequenceType);
    SAFE_POINT(NULL != sequence, "An unknown sequence type", MultipleAlignmentData::GapChar);
    return MsaRowUtils::charAt(sequence->seq, getGapModel(), pos);
}

ushort MultipleChromatogramAlignmentRowData::traceValueAt(Trace trace, int pos) const {
    int ungappedPos = MsaRowUtils::getUngappedPosition(getGapModel(), pos, false);
    if (-1 == ungappedPos) {
        return 0;
    }

    switch (trace) {
    case Trace_A:
        SAFE_POINT(0 < ungappedPos && ungappedPos < chromatogram.A.size(), QString("Position is out of range: %1").arg(ungappedPos), 0);
        return chromatogram.A[ungappedPos];
    case Trace_C:
        SAFE_POINT(0 < ungappedPos && ungappedPos < chromatogram.C.size(), QString("Position is out of range: %1").arg(ungappedPos), 0);
        return chromatogram.C[ungappedPos];
    case Trace_G:
        SAFE_POINT(0 < ungappedPos && ungappedPos < chromatogram.G.size(), QString("Position is out of range: %1").arg(ungappedPos), 0);
        return chromatogram.G[ungappedPos];
    case Trace_T:
        SAFE_POINT(0 < ungappedPos && ungappedPos < chromatogram.T.size(), QString("Position is out of range: %1").arg(ungappedPos), 0);
        return chromatogram.T[ungappedPos];
    default:
        SAFE_POINT(false, "An unknown trace type", 0);
    }
}

void MultipleChromatogramAlignmentRowData::toUpperCase() {
    DNASequenceUtils::toUpperCase(predictedSequence);
    DNASequenceUtils::toUpperCase(editedSequence);
}

void MultipleChromatogramAlignmentRowData::replaceChars(char origChar, char resultChar, U2OpStatus &os) {
    replaceChars(Predicted, origChar, resultChar, os);
    replaceChars(Edited, origChar, resultChar, os);
}

void MultipleChromatogramAlignmentRowData::replaceChars(MultipleChromatogramAlignmentRowData::SequenceType sequenceType, char origChar, char resultChar, U2OpStatus &os) {
    if (MultipleAlignmentData::GapChar == origChar) {
        coreLog.trace("The original char can't be a gap in MultipleChromatogramAlignmentRowData::replaceChars");
        os.setError("Failed to replace chars in an alignment row");
        return;
    }

    DNASequence *sequence = selectSequence(sequenceType);
    SAFE_POINT_EXT(NULL != sequence, os.setError("An unknown sequence type"), );

    if (MultipleAlignmentData::GapChar == resultChar) {
        // Get indexes of all 'origChar' characters in the row sequence
        QList<int> gapsIndexes;
        for (int i = 0; i < getRowLength(); i++) {
            if (origChar == charAt(sequenceType, i)) {
                gapsIndexes.append(i);
            }
        }

        if (gapsIndexes.isEmpty()) {
            return; // There is nothing to replace
        }

        // Remove all 'origChar' characters from the row sequence
        sequence->seq.replace(origChar, "");

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
        sequence->seq.replace(origChar, resultChar);
    }
}

MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentRowData::mid(int pos, int count, U2OpStatus &os) const {
    MultipleChromatogramAlignmentRow row = getCopy();
    row->crop(pos, count, os);
    return row;
}

MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentRowData::getCopy() const {
    return MultipleChromatogramAlignmentRow(new MultipleChromatogramAlignmentRowData(*this));
}

int MultipleChromatogramAlignmentRowData::getDataLength() const {
    return chromatogram.traceLength;
}

void MultipleChromatogramAlignmentRowData::appendDataCore(const MultipleAlignmentRow &anotherRow) {
    const MultipleChromatogramAlignmentRowData *mcaRow = dynamic_cast<const MultipleChromatogramAlignmentRowData *>(anotherRow.data());
    ChromatogramUtils::append(chromatogram, mcaRow->chromatogram);
    DNASequenceUtils::append(predictedSequence, mcaRow->predictedSequence);
    DNASequenceUtils::append(editedSequence, mcaRow->editedSequence);
}

void MultipleChromatogramAlignmentRowData::removeDataCore(int startPosInData, int endPosInData, U2OpStatus &os) {
    ChromatogramUtils::remove(os, chromatogram, startPosInData, endPosInData);
    DNASequenceUtils::removeChars(predictedSequence, startPosInData, endPosInData, os);
    DNASequenceUtils::removeChars(editedSequence, startPosInData, endPosInData, os);
}

bool MultipleChromatogramAlignmentRowData::isDataEqual(const MultipleAlignmentRowData &rowData) const {
    const MultipleChromatogramAlignmentRowData &mcaRowData = dynamic_cast<const MultipleChromatogramAlignmentRowData &>(rowData);
    return ChromatogramUtils::areEqual(chromatogram, mcaRowData.chromatogram) &&
            MatchExactly == DNASequenceUtils::compare(predictedSequence, mcaRowData.predictedSequence) &&
            MatchExactly == DNASequenceUtils::compare(editedSequence, mcaRowData.editedSequence);
}

QByteArray MultipleChromatogramAlignmentRowData::joinCharsAndGaps(SequenceType sequenceType, bool keepLeadingGaps, bool keepTrailingGaps) const {
    const DNASequence *sequence = selectSequence(sequenceType);
    SAFE_POINT(NULL != sequence, "An unknown sequence type", "");
    return MsaRowUtils::joinCharsAndGaps(*sequence, getGapModel(), getRowLength(), keepLeadingGaps, keepTrailingGaps);
}

DNASequence * MultipleChromatogramAlignmentRowData::selectSequence(MultipleChromatogramAlignmentRowData::SequenceType sequenceType) {
    switch (sequenceType) {
    case Predicted:
        return &predictedSequence;
    case Edited:
        return &editedSequence;
    default:
        assert(false);
        return NULL;
    }
}

const DNASequence * MultipleChromatogramAlignmentRowData::selectSequence(SequenceType sequenceType) const {
    switch (sequenceType) {
    case Predicted:
        return &predictedSequence;
    case Edited:
        return &editedSequence;
    default:
        assert(false);
        return NULL;
    }
}

}   // namespace U2
