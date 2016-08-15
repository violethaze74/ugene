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

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "MultipleChromatogramAlignment.h"

namespace U2 {

// Helper class to call MultipleChromatogramAlignmentData state check
class McaStateCheck {
public:
    McaStateCheck(const MultipleChromatogramAlignmentData* mca)
        : mca(mca)
    {

    }

    ~McaStateCheck() {
#ifdef _DEBUG
        mca->check();
#endif
    }

private:
    const MultipleChromatogramAlignmentData *mca;
};

namespace {

QList<MultipleAlignmentRow> toMaRows(const QList<MultipleChromatogramAlignmentRow> &mcaRows) {
    QList<MultipleAlignmentRow> maRows;
    foreach (const MultipleChromatogramAlignmentRow &mcaRow, mcaRows) {
        maRows << mcaRow;
    }
    return maRows;
}

}

const MultipleChromatogramAlignment MultipleChromatogramAlignmentData::EMPTY_MCA(new MultipleChromatogramAlignmentData);
const MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::EMPTY_ROW(new MultipleChromatogramAlignmentRowData);

MultipleChromatogramAlignmentData::MultipleChromatogramAlignmentData(const QString &name, const DNAAlphabet *alphabet, const QList<MultipleChromatogramAlignmentRow> &rows)
    : MultipleAlignmentData(name, alphabet, toMaRows(rows))
{

}

MultipleChromatogramAlignmentData::MultipleChromatogramAlignmentData(const MultipleChromatogramAlignmentData &mca)
    : MultipleAlignmentData(mca)
{

}

MultipleChromatogramAlignment MultipleChromatogramAlignmentData::createMca(const QString &name, const DNAAlphabet *alphabet, const QList<MultipleChromatogramAlignmentRow> &rows) {
    return MultipleChromatogramAlignment(new MultipleChromatogramAlignmentData(name, alphabet, rows));
}

char MultipleChromatogramAlignmentData::charAt(int rowIndex, int pos) const {
//    SAFE_POINT(0 <= rowIndex && rowIndex < getRows().size(), QString("Row index is out of range: %1").arg(rowIndex), MultipleAlignmentData::GapChar);
//    return getMcaRow(rowIndex)->charAt(pos);
    return MultipleAlignmentData::GapChar;
}

void MultipleChromatogramAlignmentData::setRowContent(int row, const QByteArray &rawData, int offset) {
//    SAFE_POINT(row >= 0 && row < getNumRows(),
//               QString("Incorrect row index '%1' was passed to MultipleChromatogramAlignmentData::setRowContent: "
//                       "the number of rows is '%2'").arg(row).arg(getNumRows()), );
//    McaStateCheck check(this);
//    Q_UNUSED(check);

//    getMcaRow(row)->setRowContent(rawData, offset);
//    setLength(qMax(getLength(), rawData.size() + offset));
}

void MultipleChromatogramAlignmentData::toUpperCase() {
//    for (int i = 0, n = getNumRows(); i < n; i++) {
//        getMsaRow(i)->toUpperCase();
//    }
}

MultipleChromatogramAlignment MultipleChromatogramAlignmentData::mid(int start, int len) const {
    SAFE_POINT(start >= 0 && start + len <= getLength(),
        QString("Incorrect parameters were passed to MultipleChromatogramAlignmentData::mid: "
        "start '%1', len '%2', the alignment length is '%3'").arg(start).arg(len).arg(getLength()),
        getEmptyMca());

    MultipleChromatogramAlignment res(new MultipleChromatogramAlignmentData(getName(), getAlphabet()));
    McaStateCheck check(res.data());
    Q_UNUSED(check);

    U2OpStatus2Log os;
    foreach (const MultipleChromatogramAlignmentRow &row, getMcaRows()) {
        MultipleChromatogramAlignmentRow mRow = row->mid(start, len, os);
        mRow->setParentAlignment(res);
        res->addRowPrivate(mRow, len, res->getRows().size());
    }
    res->setLength(len);
    return res;
}

void MultipleChromatogramAlignmentData::addRow(const QString &name, const QByteArray &bytes) {
    MultipleChromatogramAlignmentRow newRow = createSequenceRow(name, bytes);
    addRowPrivate(newRow, bytes.size(), -1);
}

void MultipleChromatogramAlignmentData::addRow(const QString &name, const QByteArray &bytes, int rowIndex) {
    MultipleChromatogramAlignmentRow newRow = createSequenceRow(name, bytes);
    addRowPrivate(newRow, bytes.size(), rowIndex);
}

void MultipleChromatogramAlignmentData::addRow(const U2MaRow &rowInDb, const DNASequence &sequence, U2OpStatus &os) {
    MultipleChromatogramAlignmentRow newRow = createSequenceRow(rowInDb, sequence, rowInDb.gaps, os);
    CHECK_OP(os, );
    addRowPrivate(newRow, rowInDb.length, -1);
}

void MultipleChromatogramAlignmentData::addRow(const QString &name, const DNASequence &sequence, const U2MaRowGapModel &gaps, U2OpStatus &os) {
    U2MaRow row;
    row.rowId = MultipleAlignmentRowData::INVALID_ROW_ID;

    MultipleChromatogramAlignmentRow newRow = createSequenceRow(row, sequence, gaps, os);
    CHECK_OP(os, );

    newRow->setName(name);
    const int len = MsaRowUtils::getRowLength(sequence.seq, gaps);
    addRowPrivate(newRow, len, -1);
}

void MultipleChromatogramAlignmentData::replaceChars(int row, char origChar, char resultChar) {
    SAFE_POINT(row >= 0 && row < getNumRows(), QString("Incorrect row index '%1' in MultipleChromatogramAlignmentData::replaceChars").arg(row), );

    if (origChar == resultChar) {
        return;
    }

    U2OpStatus2Log os;
    getMcaRow(row)->replaceChars(origChar, resultChar, os);
}

void MultipleChromatogramAlignmentData::appendChars(int row, const char* str, int len) {
    SAFE_POINT(0 <= row && row < getNumRows(), QString("Incorrect row index '%1' in MultipleChromatogramAlignmentData::appendChars").arg(row), );
    appendChars(row, getMcaRow(row)->getRowLength(), str, len);
}

void MultipleChromatogramAlignmentData::appendChars(int row, int afterPos, const char *str, int len) {
    SAFE_POINT(0 <= row && row < getNumRows(), QString("Incorrect row index '%1' in MultipleChromatogramAlignmentData::appendChars").arg(row), );

    const MultipleChromatogramAlignmentRow appendedRow = createSequenceRow("", QByteArray(str, len));
    U2OpStatus2Log os;
    getMcaRow(row)->append(appendedRow, afterPos, os);
    CHECK_OP(os, );

    setLength(qMax(getLength(), afterPos + len));
}

MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::getMcaRow(int i) {
    return getRow(i).dynamicCast<MultipleChromatogramAlignmentRowData>();
}

const MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::getMcaRow(int i) const {
    return getRow(i).dynamicCast<MultipleChromatogramAlignmentRowData>();
}

const MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::getMcaRow(const QString &name) const {
    return getRow(name).dynamicCast<MultipleChromatogramAlignmentRowData>();
}

QList<MultipleChromatogramAlignmentRow> MultipleChromatogramAlignmentData::getMcaRows() const {
    QList<MultipleChromatogramAlignmentRow> mcaRows;
    foreach (const MultipleAlignmentRow &maRow, getRows()) {
        mcaRows << maRow.dynamicCast<MultipleChromatogramAlignmentRowData>();
    }
    return mcaRows;
}

const MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::getMcaRowByRowId(qint64 rowId, U2OpStatus &os) const {
    return getRowByRowId(rowId, os).dynamicCast<MultipleChromatogramAlignmentRowData>();
}

MultipleAlignment MultipleChromatogramAlignmentData::getCopy() const {
    return getExplicitCopy();
}

MultipleChromatogramAlignment MultipleChromatogramAlignmentData::getExplicitCopy() const {
    return MultipleChromatogramAlignment(new MultipleChromatogramAlignmentData(*this));
}

MultipleChromatogramAlignment MultipleChromatogramAlignmentData::getEmptyMca() {
    return MultipleChromatogramAlignment(EMPTY_MCA->getExplicitCopy());
}

MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::getEmptyRow() {
    return MultipleChromatogramAlignmentRow(EMPTY_ROW->getCopy());
}

MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::createSequenceRow(const QString &name, const QByteArray &rawData) const {
//    U2MaRow row;
//    row.rowId = MultipleAlignmentRowData::INVALID_ROW_ID;
//    return MultipleChromatogramAlignmentRow(new MultipleChromatogramAlignmentRowData(row, name, rawData, this));
    return MultipleChromatogramAlignmentRow();
}

MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::createSequenceRow(const U2MaRow &rowInDb, const DNASequence &sequence, const U2MaRowGapModel &gaps, U2OpStatus &os) {
//    QString errorDescr = "Failed to create a multiple alignment row";
//    if (-1 != sequence.constSequence().indexOf(GapChar)) {
//        coreLog.trace("Attempted to create an alignment row from a sequence with gaps");
//        os.setError(errorDescr);
//        return MultipleChromatogramAlignmentRow();
//    }

//    int length = sequence.length();
//    foreach (const U2MaGap &gap, gaps) {
//        if (gap.offset > length || !gap.isValid()) {
//            coreLog.trace("Incorrect gap model was passed to MultipleChromatogramAlignmentRow::createRow");
//            os.setError(errorDescr);
//            return MultipleChromatogramAlignmentRow();
//        }
//        length += gap.gap;
//    }

//    return MultipleChromatogramAlignmentRow(new MultipleChromatogramAlignmentRowData(rowInDb, sequence, gaps, this));
    return MultipleChromatogramAlignmentRow();
}

MultipleAlignmentRow MultipleChromatogramAlignmentData::createRow(const MultipleAlignmentRow &row) const {
    return MultipleChromatogramAlignmentRow(new MultipleChromatogramAlignmentRowData(row.dynamicCast<MultipleChromatogramAlignmentRowData>(), this));
}

}   // namespace U2
