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

// Helper class to call MultipleSequenceAlignment state check
class MsaStateCheck {
public:
    MsaStateCheck(const MultipleSequenceAlignment* msa)
        : msa(msa)
    {

    }

    ~MsaStateCheck() {
#ifdef _DEBUG
        msa->check();
#endif
    }

private:
    const MultipleSequenceAlignment *msa;
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

MultipleSequenceAlignment::MultipleSequenceAlignment(const QString &name, const DNAAlphabet *alphabet, const QList<MultipleSequenceAlignmentRow> &rows)
    : MultipleAlignment(name, alphabet, toMaRows(rows))
{

}

MultipleSequenceAlignment::MultipleSequenceAlignment(const MultipleSequenceAlignment &msa)
    : MultipleAlignment(msa)
{

}

char MultipleSequenceAlignment::charAt(int rowIndex, int pos) const {
    SAFE_POINT(0 <= rowIndex && rowIndex < getRows().size(), QString("Row index is out of range: %1").arg(rowIndex), MultipleAlignment::GapChar);
    return getMsaRow(rowIndex)->charAt(pos);
}

void MultipleSequenceAlignment::setRowContent(int row, const QByteArray &rawData, int offset) {
    SAFE_POINT(row >= 0 && row < getNumRows(),
               QString("Incorrect row index '%1' was passed to MultipleSequenceAlignment::setRowContent: "
                       "the number of rows is '%2'").arg(row).arg(getNumRows()), );
    MsaStateCheck check(this);
    Q_UNUSED(check);

    getMsaRow(row)->setRowContent(rawData, offset);
    setLength(qMax(getLength(), rawData.size() + offset));
}

void MultipleSequenceAlignment::toUpperCase() {
    for (int i = 0, n = getNumRows(); i < n; i++) {
        getMsaRow(i)->toUpperCase();
    }
}

MultipleSequenceAlignment MultipleSequenceAlignment::mid(int start, int len) const {
    static MultipleSequenceAlignment emptyAlignment;
    SAFE_POINT(start >= 0 && start + len <= getLength(),
        QString("Incorrect parameters were passed to MultipleSequenceAlignment::mid: "
        "start '%1', len '%2', the alignment length is '%3'").arg(start).arg(len).arg(getLength()),
        emptyAlignment);

    MultipleSequenceAlignment res(getName(), getAlphabet());
    MsaStateCheck check(&res);
    Q_UNUSED(check);

    U2OpStatus2Log os;
    foreach (const MultipleSequenceAlignmentRow &row, getMsaRows()) {
        MultipleSequenceAlignmentRow mRow = row->mid(start, len, os);
        mRow->setParentAlignment(&res);
        res.addRowPrivate(mRow, len, res.getRows().size());
    }
    res.setLength(len);
    return res;
}

void MultipleSequenceAlignment::addRow(const QString &name, const QByteArray &bytes) {
    MultipleSequenceAlignmentRow newRow = createSequenceRow(name, bytes);
    addRowPrivate(newRow, bytes.size(), -1);
}

void MultipleSequenceAlignment::addRow(const QString &name, const QByteArray &bytes, int rowIndex) {
    MultipleSequenceAlignmentRow newRow = createSequenceRow(name, bytes);
    addRowPrivate(newRow, bytes.size(), rowIndex);
}

void MultipleSequenceAlignment::addRow(const U2MaRow &rowInDb, const DNASequence &sequence, U2OpStatus &os) {
    MultipleSequenceAlignmentRow newRow = createSequenceRow(rowInDb, sequence, rowInDb.gaps, os);
    CHECK_OP(os, );
    addRowPrivate(newRow, rowInDb.length, -1);
}

void MultipleSequenceAlignment::addRow(const QString &name, const DNASequence &sequence, const U2MaRowGapModel &gaps, U2OpStatus &os) {
    U2MaRow row;
    row.rowId = MultipleAlignmentRowData::INVALID_ROW_ID;

    MultipleSequenceAlignmentRow newRow = createSequenceRow(row, sequence, gaps, os);
    CHECK_OP(os, );

    newRow->setName(name);
    const int len = MsaRowUtils::getRowLength(sequence.seq, gaps);
    addRowPrivate(newRow, len, -1);
}

void MultipleSequenceAlignment::replaceChars(int row, char origChar, char resultChar) {
    SAFE_POINT(row >= 0 && row < getNumRows(), QString("Incorrect row index '%1' in MultipleSequenceAlignment::replaceChars").arg(row), );

    if (origChar == resultChar) {
        return;
    }

    U2OpStatus2Log os;
    getMsaRow(row)->replaceChars(origChar, resultChar, os);
}

void MultipleSequenceAlignment::appendChars(int row, const char* str, int len) {
    SAFE_POINT(0 <= row && row < getNumRows(), QString("Incorrect row index '%1' in MultipleSequenceAlignment::appendChars").arg(row), );
    appendChars(row, getMsaRow(row)->getRowLength(), str, len);
}

void MultipleSequenceAlignment::appendChars(int row, int afterPos, const char *str, int len) {
    SAFE_POINT(0 <= row && row < getNumRows(), QString("Incorrect row index '%1' in MultipleSequenceAlignment::appendChars").arg(row), );

    const MultipleSequenceAlignmentRow appendedRow = createSequenceRow("", QByteArray(str, len));
    U2OpStatus2Log os;
    getMsaRow(row)->append(appendedRow, afterPos, os);
    CHECK_OP(os, );

    setLength(qMax(getLength(), afterPos + len));
}

MultipleSequenceAlignmentRow & MultipleSequenceAlignment::getMsaRow(int i) {
    return (MultipleSequenceAlignmentRow &)(getRow(i));
}

const MultipleSequenceAlignmentRow &MultipleSequenceAlignment::getMsaRow(int i) const {
    return (const MultipleSequenceAlignmentRow &)(getRow(i));
}

QList<MultipleSequenceAlignmentRow> MultipleSequenceAlignment::getMsaRows() const {
    QList<MultipleSequenceAlignmentRow> msaRows;
    foreach (const MultipleAlignmentRow &maRow, getRows()) {
        msaRows << maRow;
    }
    return msaRows;
}

MultipleAlignment * MultipleSequenceAlignment::clone() {
    return new MultipleSequenceAlignment(*this);
}

MultipleSequenceAlignmentRow MultipleSequenceAlignment::createSequenceRow(const QString &name, const QByteArray &rawData) const {
    U2MaRow row;
    row.rowId = MultipleAlignmentRowData::INVALID_ROW_ID;
    return MultipleSequenceAlignmentRow(new MultipleSequenceAlignmentRowData(row, name, rawData, this));
}

MultipleSequenceAlignmentRow MultipleSequenceAlignment::createSequenceRow(const U2MaRow &rowInDb, const DNASequence &sequence, const U2MaRowGapModel &gaps, U2OpStatus &os) {
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

MultipleAlignmentRow MultipleSequenceAlignment::createRow(const MultipleAlignmentRow &row) const {
    return MultipleSequenceAlignmentRow(new MultipleSequenceAlignmentRowData((MultipleSequenceAlignmentRow)row, this));
}

static bool _registerMeta() {
    qRegisterMetaType<MultipleSequenceAlignment>("MultipleSequenceAlignment");
    return true;
}

bool MultipleSequenceAlignment::registerMeta = _registerMeta();

} // namespace U2
