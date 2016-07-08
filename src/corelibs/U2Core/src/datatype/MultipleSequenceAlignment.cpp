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

#include <U2Core/MsaRowUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "MultipleSequenceAlignment.h"

namespace U2 {

// Helper class to call MultipleSequenceAlignmentData state check
class MsaStateCheck {
public:
    MsaStateCheck(const MultipleSequenceAlignmentData* msa)
        : msa(msa)
    {

    }

    ~MsaStateCheck() {
#ifdef _DEBUG
        msa->check();
#endif
    }

private:
    const MultipleSequenceAlignmentData *msa;
};

namespace {

QList<MultipleAlignmentRow> toMaRows(const QList<MultipleSequenceAlignmentRow> &msaRows) {
    QList<MultipleAlignmentRow> maRows;
    foreach (const MultipleSequenceAlignmentRow &msaRow, msaRows) {
        maRows << msaRow;
    }
    return maRows;
}

}

const MultipleSequenceAlignment MultipleSequenceAlignmentData::EMPTY_MSA(new MultipleSequenceAlignmentData);
const MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::EMPTY_ROW(new MultipleSequenceAlignmentRowData);

MultipleSequenceAlignmentData::MultipleSequenceAlignmentData(const QString &name, const DNAAlphabet *alphabet, const QList<MultipleSequenceAlignmentRow> &rows)
    : MultipleAlignmentData(name, alphabet, toMaRows(rows))
{

}

MultipleSequenceAlignmentData::MultipleSequenceAlignmentData(const MultipleSequenceAlignmentData &msa)
    : MultipleAlignmentData(msa)
{

}

MultipleSequenceAlignment MultipleSequenceAlignmentData::createMsa(const QString &name, const DNAAlphabet *alphabet, const QList<MultipleSequenceAlignmentRow> &rows) {
    return MultipleSequenceAlignment(new MultipleSequenceAlignmentData(name, alphabet, rows));
}

char MultipleSequenceAlignmentData::charAt(int rowIndex, int pos) const {
    SAFE_POINT(0 <= rowIndex && rowIndex < getRows().size(), QString("Row index is out of range: %1").arg(rowIndex), MultipleAlignmentData::GapChar);
    return getMsaRow(rowIndex)->charAt(pos);
}

void MultipleSequenceAlignmentData::setRowContent(int row, const QByteArray &rawData, int offset) {
    SAFE_POINT(row >= 0 && row < getNumRows(),
               QString("Incorrect row index '%1' was passed to MultipleSequenceAlignmentData::setRowContent: "
                       "the number of rows is '%2'").arg(row).arg(getNumRows()), );
    MsaStateCheck check(this);
    Q_UNUSED(check);

    getMsaRow(row)->setRowContent(rawData, offset);
    setLength(qMax(getLength(), rawData.size() + offset));
}

void MultipleSequenceAlignmentData::toUpperCase() {
    for (int i = 0, n = getNumRows(); i < n; i++) {
        getMsaRow(i)->toUpperCase();
    }
}

MultipleSequenceAlignment MultipleSequenceAlignmentData::mid(int start, int len) const {
    SAFE_POINT(start >= 0 && start + len <= getLength(),
        QString("Incorrect parameters were passed to MultipleSequenceAlignmentData::mid: "
        "start '%1', len '%2', the alignment length is '%3'").arg(start).arg(len).arg(getLength()),
        getEmptyMsa());

    MultipleSequenceAlignment res(new MultipleSequenceAlignmentData(getName(), getAlphabet()));
    MsaStateCheck check(res.data());
    Q_UNUSED(check);

    U2OpStatus2Log os;
    foreach (const MultipleSequenceAlignmentRow &row, getMsaRows()) {
        MultipleSequenceAlignmentRow mRow = row->mid(start, len, os);
        mRow->setParentAlignment(res);
        res->addRowPrivate(mRow, len, res->getRows().size());
    }
    res->setLength(len);
    return res;
}

void MultipleSequenceAlignmentData::addRow(const QString &name, const QByteArray &bytes) {
    MultipleSequenceAlignmentRow newRow = createSequenceRow(name, bytes);
    addRowPrivate(newRow, bytes.size(), -1);
}

void MultipleSequenceAlignmentData::addRow(const QString &name, const QByteArray &bytes, int rowIndex) {
    MultipleSequenceAlignmentRow newRow = createSequenceRow(name, bytes);
    addRowPrivate(newRow, bytes.size(), rowIndex);
}

void MultipleSequenceAlignmentData::addRow(const U2MaRow &rowInDb, const DNASequence &sequence, U2OpStatus &os) {
    MultipleSequenceAlignmentRow newRow = createSequenceRow(rowInDb, sequence, rowInDb.gaps, os);
    CHECK_OP(os, );
    addRowPrivate(newRow, rowInDb.length, -1);
}

void MultipleSequenceAlignmentData::addRow(const QString &name, const DNASequence &sequence, const U2MaRowGapModel &gaps, U2OpStatus &os) {
    U2MaRow row;
    row.rowId = MultipleAlignmentRowData::INVALID_ROW_ID;

    MultipleSequenceAlignmentRow newRow = createSequenceRow(row, sequence, gaps, os);
    CHECK_OP(os, );

    newRow->setName(name);
    const int len = MsaRowUtils::getRowLength(sequence.seq, gaps);
    addRowPrivate(newRow, len, -1);
}

void MultipleSequenceAlignmentData::replaceChars(int row, char origChar, char resultChar) {
    SAFE_POINT(row >= 0 && row < getNumRows(), QString("Incorrect row index '%1' in MultipleSequenceAlignmentData::replaceChars").arg(row), );

    if (origChar == resultChar) {
        return;
    }

    U2OpStatus2Log os;
    getMsaRow(row)->replaceChars(origChar, resultChar, os);
}

void MultipleSequenceAlignmentData::appendChars(int row, const char* str, int len) {
    SAFE_POINT(0 <= row && row < getNumRows(), QString("Incorrect row index '%1' in MultipleSequenceAlignmentData::appendChars").arg(row), );
    appendChars(row, getMsaRow(row)->getRowLength(), str, len);
}

void MultipleSequenceAlignmentData::appendChars(int row, int afterPos, const char *str, int len) {
    SAFE_POINT(0 <= row && row < getNumRows(), QString("Incorrect row index '%1' in MultipleSequenceAlignmentData::appendChars").arg(row), );

    const MultipleSequenceAlignmentRow appendedRow = createSequenceRow("", QByteArray(str, len));
    U2OpStatus2Log os;
    getMsaRow(row)->append(appendedRow, afterPos, os);
    CHECK_OP(os, );

    setLength(qMax(getLength(), afterPos + len));
}

MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::getMsaRow(int i) {
    return getRow(i).dynamicCast<MultipleSequenceAlignmentRowData>();
}

const MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::getMsaRow(int i) const {
    return getRow(i).dynamicCast<MultipleSequenceAlignmentRowData>();
}

const MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::getMsaRow(const QString &name) const {
    return getRow(name).dynamicCast<MultipleSequenceAlignmentRowData>();
}

QList<MultipleSequenceAlignmentRow> MultipleSequenceAlignmentData::getMsaRows() const {
    QList<MultipleSequenceAlignmentRow> msaRows;
    foreach (const MultipleAlignmentRow &maRow, getRows()) {
        msaRows << maRow.dynamicCast<MultipleSequenceAlignmentRowData>();
    }
    return msaRows;
}

const MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::getMsaRowByRowId(qint64 rowId, U2OpStatus &os) const {
    return getRowByRowId(rowId, os).dynamicCast<MultipleSequenceAlignmentRowData>();
}

MultipleSequenceAlignmentData *MultipleSequenceAlignmentData::explicitClone() const {
    return new MultipleSequenceAlignmentData(*this);
}

MultipleSequenceAlignment MultipleSequenceAlignmentData::getEmptyMsa() {
    return MultipleSequenceAlignment(EMPTY_MSA->explicitClone());
}

MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::getEmptyRow() {
    return MultipleSequenceAlignmentRow(EMPTY_ROW->explicitClone());
}

MultipleAlignmentData * MultipleSequenceAlignmentData::clone() const {
    return explicitClone();
}

MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::createSequenceRow(const QString &name, const QByteArray &rawData) const {
    U2MaRow row;
    row.rowId = MultipleAlignmentRowData::INVALID_ROW_ID;
    return MultipleSequenceAlignmentRow(new MultipleSequenceAlignmentRowData(row, name, rawData, this));
}

MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::createSequenceRow(const U2MaRow &rowInDb, const DNASequence &sequence, const U2MaRowGapModel &gaps, U2OpStatus &os) {
    QString errorDescr = "Failed to create a multiple alignment row";
    if (-1 != sequence.constSequence().indexOf(GapChar)) {
        coreLog.trace("Attempted to create an alignment row from a sequence with gaps");
        os.setError(errorDescr);
        return MultipleSequenceAlignmentRow();
    }

    int length = sequence.length();
    foreach (const U2MaGap &gap, gaps) {
        if (gap.offset > length || !gap.isValid()) {
            coreLog.trace("Incorrect gap model was passed to MultipleSequenceAlignmentRow::createRow");
            os.setError(errorDescr);
            return MultipleSequenceAlignmentRow();
        }
        length += gap.gap;
    }

    return MultipleSequenceAlignmentRow(new MultipleSequenceAlignmentRowData(rowInDb, sequence, gaps, this));
}

MultipleAlignmentRow MultipleSequenceAlignmentData::createRow(const MultipleAlignmentRow &row) const {
    return MultipleSequenceAlignmentRow(new MultipleSequenceAlignmentRowData(row.dynamicCast<MultipleSequenceAlignmentRowData>(), this));
}

} // namespace U2
