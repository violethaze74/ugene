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

#include <QSet>

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2Region.h>

#include "MultipleSequenceAlignmentData.h"
#include "MultipleSequenceAlignmentRow.h"
#include "MultipleSequenceAlignmentRowData.h"

namespace U2 {

// Helper class to call MultipleSequenceAlignmentData state check
class MAStateCheck {
public:
    MAStateCheck(const MultipleSequenceAlignmentData *msa)
        : msa(msa)
    {

    }

    ~MAStateCheck() {
#ifdef _DEBUG
        ma->check();
#else
        Q_UNUSED(ma);
#endif
    }

private:
    const MultipleSequenceAlignmentData *msa;
};

MultipleSequenceAlignmentData::MultipleSequenceAlignmentData(const QString &name, const DNAAlphabet *alphabet, const QList<MultipleSequenceAlignmentRow> &rows)
    : alphabet(alphabet),
      rows(rows)
{
    MAStateCheck check(this);
    Q_UNUSED(check);

    SAFE_POINT(alphabet == NULL || !name.isEmpty(), "Incorrect parameters in MultipleSequenceAlignmentData ctor", );

    setName(name);

    length = 0;
    for (int i = 0, n = rows.size(); i < n; i++) {
        length = qMax(length, rows[i]->getCoreEnd());
    }
}

MultipleSequenceAlignmentData::MultipleSequenceAlignmentData(const MultipleSequenceAlignmentData &msaData)
    : alphabet(NULL),
      length(0)
{
    copy(msaData);
}

MultipleSequenceAlignmentData & MultipleSequenceAlignmentData::operator=(const MultipleSequenceAlignment &msa) {
    return *this = *msa;
}

MultipleSequenceAlignmentData & MultipleSequenceAlignmentData::operator=(const MultipleSequenceAlignmentData &msaData) {
    copy(msaData);
    return *this;
}

void MultipleSequenceAlignmentData::setAlphabet(const DNAAlphabet *newAlphabet) {
    SAFE_POINT(NULL != newAlphabet, "Internal error: attempted to set NULL alphabet fro an alignment", );
    alphabet = newAlphabet;
}

QVariantMap MultipleSequenceAlignmentData::getInfo() const {
    return info;
}

void MultipleSequenceAlignmentData::setInfo(const QVariantMap &newInfo) {
    info = newInfo;
}

bool MultipleSequenceAlignmentData::isEmpty() const {
    return getLength() == 0;
}

int MultipleSequenceAlignmentData::getLength() const {
    return length;
}

bool MultipleSequenceAlignmentData::trim(bool removeLeadingGaps) {
    MAStateCheck check(this);
    Q_UNUSED(check);

    bool result = false;

    if ( removeLeadingGaps ) {
        // Verify if there are leading columns of gaps
        // by checking the first gap in each row
        qint64 leadingGapColumnsNum = 0;
        foreach (const MultipleSequenceAlignmentRow &row, rows) {
            if (row.getGapModel().count() > 0) {
                const U2MsaGap firstGap = row.getGapModel().first();
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
            for (int i = 0; i < rows.count(); ++i) {
                rows[i]->removeChars(0, leadingGapColumnsNum, os);
                CHECK_OP(os, true);
                result = true;
            }
        }
    }

    // Verify right side of the alignment (trailing gaps and rows' lengths)
    int newLength = 0;
    foreach (const MultipleSequenceAlignmentRow &row, rows) {
        if (newLength == 0) {
            newLength = row->getRowLengthWithoutTrailing();
        } else {
            newLength = qMax(row->getRowLengthWithoutTrailing(), newLength);
        }
    }

    if (newLength != length) {
        length = newLength;
        result = true;
    }

    return result;
}

bool MultipleSequenceAlignmentData::simplify() {
    MAStateCheck check(this);
    Q_UNUSED(check);

    int newLen = 0;
    bool changed = false;
    for (int i = 0, n = rows.size(); i < n; i++) {
        changed |= rows[i]->simplify();
        newLen = qMax(newLen, rows[i]->getCoreEnd());
    }

    if (!changed) {
        assert(length == newLen);
        return false;
    }
    length = newLen;
    return true;
}

bool MultipleSequenceAlignmentData::hasEmptyGapModel() const {
    foreach (const MultipleSequenceAlignmentRow &row, rows) {
        if (!row->getGapModel().isEmpty()) {
            return false;
        }
    }
    return true;
}

bool MultipleSequenceAlignmentData::hasEqualLength() const {
    const int defaultSequenceLength = -1;
    int sequenceLength = defaultSequenceLength;
    for (int i = 0, n = rows.size(); i < n; ++i) {
        if (defaultSequenceLength != sequenceLength && sequenceLength != rows[i]->getUngappedLength()) {
            return false;
        } else {
            sequenceLength = rows[i]->getUngappedLength();
        }
    }
    return true;
}

void MultipleSequenceAlignmentData::clear() {
    MAStateCheck check(this);
    Q_UNUSED(check);

    rows.clear();
    length = 0;
}

QString MultipleSequenceAlignmentData::getName() const {
    return MultipleSequenceAlignmentInfo::getName(info);
}

void MultipleSequenceAlignmentData::setName(const QString &newName) {
    MultipleSequenceAlignmentInfo::setName(info, newName);
}

const DNAAlphabet * MultipleSequenceAlignmentData::getAlphabet() const {
    return alphabet;
}

MultipleSequenceAlignment MultipleSequenceAlignmentData::mid(int start, int len) const {
    SAFE_POINT(start >= 0 && start + len <= length,
               QString("Incorrect parameters were passed to MultipleSequenceAlignmentData::mid: "
                       "start '%1', len '%2', the alignment length is '%3'").arg(start).arg(len).arg(length),
               MultipleSequenceAlignment);

    MultipleSequenceAlignment res(getName(), alphabet);
    MAStateCheck check(res.data());
    Q_UNUSED(check);

    U2OpStatus2Log os;
    foreach (const MultipleSequenceAlignmentRow &row, rows) {
        MultipleSequenceAlignmentRow mRow = row->mid(start, len, os);
        mRow->setParentAlignment(res);
        res->rows << mRow;
    }
    res->length = len;
    return res;
}

U2MsaListGapModel MultipleSequenceAlignmentData::getGapModel() const {
    U2MsaListGapModel gapModel;
    foreach (const MultipleSequenceAlignmentRow &row, rows) {
        gapModel << row->getGapModel();
    }
    return gapModel;
}

MultipleSequenceAlignmentData & MultipleSequenceAlignmentData::operator+=(const MultipleSequenceAlignmentData &msaData) {
    MAStateCheck check(this);
    Q_UNUSED(check);

    SAFE_POINT(msaData.alphabet == alphabet, "Different alphabets in MultipleSequenceAlignmentData::operator+=", *this);

    int nSeq = getNumRows();
    SAFE_POINT(msaData.getNumRows() == nSeq, "Different number of rows in MultipleSequenceAlignmentData::operator+=", *this);

    U2OpStatus2Log os;
    for (int i = 0; i < nSeq; i++) {
        rows[i]->append(msaData.rows[i], length, os);
    }

    length += msaData.length;
    return *this;
}

bool MultipleSequenceAlignmentData::operator==(const MultipleSequenceAlignmentData &other) const {
    bool lengthsAreEqual = (length == other.length);
    bool alphabetsAreEqual = (alphabet == other.alphabet);
    bool rowsAreEqual = (rows == other.rows);
    return lengthsAreEqual && alphabetsAreEqual && rowsAreEqual;
}

bool MultipleSequenceAlignmentData::operator!=(const MultipleSequenceAlignmentData &other) const {
    return !operator==(other);
}

bool MultipleSequenceAlignmentData::crop(const U2Region &region, const QSet<QString> &rowNames, U2OpStatus &os) {
    if (!(region.startPos >= 0 && region.length > 0 && region.length < length && region.startPos < length)) {
        os.setError(QString("Incorrect region was passed to MultipleSequenceAlignmentData::crop, "
                            "startPos '%1', length '%2'").arg(region.startPos).arg(region.length));
        return false;
    }

    int cropLen = region.length;
    if (region.endPos() > length) {
        cropLen -= (region.endPos() - length);
    }

    MAStateCheck check(this);
    Q_UNUSED(check);

    QList<MultipleSequenceAlignmentRow> newList;
    for (int i = 0 ; i < rows.size(); i++) {
        MultipleSequenceAlignmentRow row = rows[i].clone();
        const QString rowName = row->getName();
        if (rowNames.contains(rowName)){
            row->crop(region.startPos, cropLen, os);
            CHECK_OP(os, false);
            newList << row;
        }
    }
    rows = newList;

    length = cropLen;
    return true;
}

bool MultipleSequenceAlignmentData::crop(const U2Region &region, U2OpStatus &os) {
    return crop(region, getRowNames().toSet(), os);
}

bool MultipleSequenceAlignmentData::crop(int start, int count, U2OpStatus &os) {
    return crop(U2Region(start, count), os);
}

MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::createRow(const QString &name, const QByteArray &bytes) {
    QByteArray newSequenceBytes;
    QList<U2MsaGap> newGapsModel;

    MultipleSequenceAlignmentRowData::splitBytesToCharsAndGaps(bytes, newSequenceBytes, newGapsModel);
    DNASequence newSequence(name, newSequenceBytes);

    U2MsaRow row;
    row.rowId = MultipleSequenceAlignmentRow::invalidRowId();

    return MultipleSequenceAlignmentRow(row, newSequence, newGapsModel, this);
}

MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::createRow(const U2MsaRow &rowInDb, const DNASequence &sequence, const QList<U2MsaGap> &gaps, U2OpStatus &os) {
    QString errorDescr = "Failed to create a multiple alignment row";
    if (-1 != sequence.constSequence().indexOf(MultipleSequenceAlignmentData_GapChar)) {
        coreLog.trace("Attempted to create an alignment row from a sequence with gaps");
        os.setError(errorDescr);
        return MultipleSequenceAlignmentRow();
    }

    int length = sequence.length();
    foreach (const U2MsaGap &gap, gaps) {
        if (gap.offset > length || !gap.isValid()) {
            coreLog.trace("Incorrect gap model was passed to MultipleSequenceAlignmentData::createRow");
            os.setError(errorDescr);
            return MultipleSequenceAlignmentRow();
        }
        length += gap.gap;
    }

    return MultipleSequenceAlignmentRow(rowInDb, sequence, gaps, this);
}

MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::createRow(const MultipleSequenceAlignmentRow &row) {
    return MultipleSequenceAlignmentRow(row, this);
}


void MultipleSequenceAlignmentData::addRowPrivate(const MultipleSequenceAlignmentRow &row, int rowLenWithTrailingGaps, int rowIndex) {
    MAStateCheck check(this);
    Q_UNUSED(check);

    length = qMax(rowLenWithTrailingGaps, length);
    int idx = rowIndex == -1 ? getNumRows() : qBound(0, rowIndex, getNumRows());
    rows.insert(idx, row);
}

void MultipleSequenceAlignmentData::addRow(const QString &name, const QByteArray &bytes) {
    MultipleSequenceAlignmentRow newRow = createRow(name, bytes);
    addRowPrivate(newRow, bytes.size(), -1);
}

void MultipleSequenceAlignmentData::addRow(const QString &name, const QByteArray &bytes, int rowIndex) {
    MultipleSequenceAlignmentRow newRow = createRow(name, bytes);
    addRowPrivate(newRow, bytes.size(), rowIndex);
}

void MultipleSequenceAlignmentData::addRow(const U2MsaRow &rowInDb, const DNASequence &sequence, U2OpStatus &os) {
    MultipleSequenceAlignmentRow newRow = createRow(rowInDb, sequence, rowInDb.gaps, os);
    CHECK_OP(os, );
    addRowPrivate(newRow, rowInDb.length, -1);
}

void MultipleSequenceAlignmentData::addRow(const QString &name, const DNASequence &sequence, const QList<U2MsaGap> &gaps, U2OpStatus &os) {
    U2MsaRow row;
    row.rowId = MultipleSequenceAlignmentRow::invalidRowId();

    MultipleSequenceAlignmentRow newRow = createRow(row, sequence, gaps, os);
    CHECK_OP(os, );

    int len = sequence.length();
    foreach (const U2MsaGap &gap, gaps) {
        len += gap.gap;
    }

    newRow->setName(name);
    addRowPrivate(newRow, len, -1);
}

void MultipleSequenceAlignmentData::removeRow(int rowIndex, U2OpStatus &os) {
    if (rowIndex < 0 || rowIndex >= getNumRows()) {
        coreLog.trace(QString("Internal error: incorrect parameters was passed to MultipleSequenceAlignmentData::removeRow, "
            "rowIndex '%1', the number of rows is '%2'").arg(rowIndex).arg(getNumRows()));
        os.setError("Failed to remove a row");
        return;
    }

    MAStateCheck check(this);
    Q_UNUSED(check);

    rows.removeAt(rowIndex);

    if (rows.isEmpty()) {
        length = 0;
    }
}

void MultipleSequenceAlignmentData::insertGaps(int row, int pos, int count, U2OpStatus &os) {
    if (row >= getNumRows() || row < 0 || pos > length || pos < 0 || count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed "
            "to MultipleSequenceAlignmentData::insertGaps: row index '%1', pos '%2', count '%3'").arg(row).arg(pos).arg(count));
        os.setError("Failed to insert gaps into an alignment");
        return;
    }

    if (pos == length) {
        // add trailing gaps --> just increase alignment len
        length += count;
        return;
    }

    MAStateCheck check(this);
    Q_UNUSED(check);

    if (pos >= rows[row]->getRowLengthWithoutTrailing()) {
        length += count;
        return;
    }
    rows[row]->insertGaps(pos, count, os);

    int rowLength = rows[row]->getRowLengthWithoutTrailing();
    length = qMax(length, rowLength);
}

void MultipleSequenceAlignmentData::appendChars(int row, const char *str, int len) {
    SAFE_POINT(0 <= row && row < getNumRows(),
        QString("Incorrect row index '%1' in MultipleSequenceAlignmentData::appendChars").arg(row), );

    MultipleSequenceAlignmentRow appendedRow = createRow("", QByteArray(str, len));

    int rowLength = rows[row]->getRowLength();;

    U2OpStatus2Log os;
    rows[row]->append(appendedRow, rowLength, os);
    CHECK_OP(os, );

    length = qMax(length, rowLength + len);
}

void MultipleSequenceAlignmentData::appendChars(int row, int afterPos, const char *str, int len) {
    SAFE_POINT(0 <= row && row < getNumRows(),
        QString("Incorrect row index '%1' in MultipleSequenceAlignmentData::appendChars").arg(row), );

    MultipleSequenceAlignmentRow appendedRow = createRow("", QByteArray(str, len));

    U2OpStatus2Log os;
    rows[row].append(appendedRow, afterPos, os);
    CHECK_OP(os, );

    length = qMax(length, afterPos + len);
}

void MultipleSequenceAlignmentData::appendRow(int rowNumber, const MultipleSequenceAlignmentRow &row, bool ignoreTrailingGaps, U2OpStatus &os) {
    appendRow(rowNumber, ignoreTrailingGaps ? rows[rowNumber].getRowLengthWithoutTrailing() : rows[rowNumber].getRowLength(), row, os);
}

void MultipleSequenceAlignmentData::appendRow(int rowNumber, int afterPos, const MultipleSequenceAlignmentRow &row, U2OpStatus &os) {
    SAFE_POINT(0 <= rowNumber && rowNumber < getNumRows(),
        QString("Incorrect row index '%1' in MultipleSequenceAlignmentData::appendRow").arg(rowNumber), );

    rows[rowNumber].append(row, afterPos, os);
    CHECK_OP(os, );

    length = qMax(length, afterPos + row.getRowLength());
}

void MultipleSequenceAlignmentData::removeChars(int rowNumber, int pos, int count, U2OpStatus &os) {
    if (rowNumber >= getNumRows() || rowNumber < 0 || pos > length || pos < 0 || count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed "
            "to MultipleSequenceAlignmentData::removeChars: row index '%1', pos '%2', count '%3'").arg(rowNumber).arg(pos).arg(count));
        os.setError("Failed to remove chars from an alignment");
        return;
    }

    MAStateCheck check(this);
    Q_UNUSED(check);

    rows[rowNumber]->removeChars(pos, count, os);
}

void MultipleSequenceAlignmentData::removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows) {
    SAFE_POINT(startPos >= 0 && startPos + nBases <= length && nBases > 0,
        QString("Incorrect parameters were passed to MultipleSequenceAlignmentData::removeRegion: startPos '%1', "
        "nBases '%2', the length is '%3'").arg(startPos).arg(nBases).arg(length),);
    SAFE_POINT(startRow >= 0 && startRow + nRows <= getNumRows() && nRows > 0,
        QString("Incorrect parameters were passed to MultipleSequenceAlignmentData::removeRegion: startRow '%1', "
        "nRows '%2', the number of rows is '%3'").arg(startRow).arg(nRows).arg(getNumRows()),);

    MAStateCheck check(this);
    Q_UNUSED(check);

    U2OpStatus2Log os;
    for (int i = startRow + nRows; --i >= startRow;) {
        rows[i]->removeChars(startPos, nBases, os);
        SAFE_POINT_OP(os, );

        if (removeEmptyRows && (0 == rows[i]->getSequence().length())) {
            rows.removeAt(i);
        }
    }

    if (nRows == rows.size()) {
        // full columns were removed
        length -= nBases;
        if (length == 0) {
            rows.clear();
        }
    }
}

void MultipleSequenceAlignmentData::setLength(int newLength) {
    SAFE_POINT(newLength >= 0, QString("Internal error: attempted to set length '%1' for an alignment").arg(newLength), );

    MAStateCheck check(this);
    Q_UNUSED(check);

    if (newLength >= length) {
        length = newLength;
        return;
    }

    U2OpStatus2Log os;
    for (int i = 0, n = getNumRows(); i < n; i++) {
        rows[i]->crop(0, newLength, os);
        CHECK_OP(os, );
    }
    length = newLength;
}

int MultipleSequenceAlignmentData::getNumRows() const {
    return rows.size();
}

void MultipleSequenceAlignmentData::renameRow(int row, const QString &name) {
    SAFE_POINT(row >= 0 && row < getNumRows(),
        QString("Incorrect row index '%1' was passed to MultipleSequenceAlignmentData::renameRow: "
        "the number of rows is '%2'").arg(row).arg(getNumRows()), );
    SAFE_POINT(!name.isEmpty(),
        "Incorrect parameter 'name' was passed to MultipleSequenceAlignmentData::renameRow: "
        "Can't set the name of a row to an empty string", );
    rows[row]->setName(name);
}


void MultipleSequenceAlignmentData::replaceChars(int row, char origChar, char resultChar) {
    SAFE_POINT(row >= 0 && row < getNumRows(), QString("Incorrect row index '%1' in MultipleSequenceAlignmentData::replaceChars").arg(row), );

    if (origChar == resultChar) {
        return;
    }

    U2OpStatus2Log os;
    rows[row]->replaceChars(origChar, resultChar, os);
}

void MultipleSequenceAlignmentData::setRowContent(int rowNumber, const QByteArray &sequence, int offset) {
    SAFE_POINT(rowNumber >= 0 && rowNumber < getNumRows(),
        QString("Incorrect row index '%1' was passed to MultipleSequenceAlignmentData::setRowContent: "
        "the number of rows is '%2'").arg(rowNumber).arg(getNumRows()), );
    MAStateCheck check(this);
    Q_UNUSED(check);

    U2OpStatus2Log os;
    rows[rowNumber]->setRowContent(sequence, offset, os);
    SAFE_POINT_OP(os, );

    length = qMax(length, sequence.size() + offset);
}

void MultipleSequenceAlignmentData::toUpperCase() {
    for (int i = 0, n = getNumRows(); i < n; i++) {
        rows[i]->toUpperCase();
    }
}

class CompareMsaRowsByName {
public:
    CompareMsaRowsByName(bool asc = true)
        : asc(asc)
    {

    }

    bool operator()(const MultipleSequenceAlignmentRow &row1, const MultipleSequenceAlignmentRow &row2) const {
        const bool res = QString::compare(row1->getName(), row2->getName(), Qt::CaseInsensitive) > 0;
        return asc ? !res : res;
    }

private:
    bool asc;
};

void MultipleSequenceAlignmentData::sortRowsByName(bool asc) {
    MAStateCheck check(this);
    Q_UNUSED(check);
    qStableSort(rows.begin(), rows.end(), CompareMsaRowsByName(asc));
}

bool MultipleSequenceAlignmentData::sortRowsBySimilarity(QVector<U2Region> &united) {
    QList<MultipleSequenceAlignmentRow> oldRows = rows;
    QList<MultipleSequenceAlignmentRow> sortedRows;
    while (!oldRows.isEmpty()) {
        const MultipleSequenceAlignmentRow row = oldRows.takeFirst();
        sortedRows << row;
        int start = sortedRows.size() - 1;
        int len = 1;
        QMutableListIterator<MultipleSequenceAlignmentRow> iter(oldRows);
        while (iter.hasNext()) {
            const MultipleSequenceAlignmentRow &next = iter.next();
            if (next->isRowContentEqual(row)) {
                sortedRows << next;
                iter.remove();
                ++len;
            }
        }
        if (len > 1) {
            united.append(U2Region(start, len));
        }
    }
    if(rows != sortedRows) {
        rows = sortedRows;
        return true;
    }
    return false;
}

const QList<MultipleSequenceAlignmentRow> & MultipleSequenceAlignmentData::getRows() const {
    return rows;
}

void MultipleSequenceAlignmentData::moveRowsBlock(int startRow, int numRows, int delta) {
    MAStateCheck check(this);
    Q_UNUSED(check);

    // Assumption: numRows is rather big, delta is small (1~2)
    // It's more optimal to move abs(delta) of rows then the block itself

    int i = 0;
    int k = qAbs(delta);

    SAFE_POINT((delta > 0 && startRow + numRows + delta - 1 < rows.length())
               || (delta < 0 && startRow + delta >= 0),
               QString("Incorrect parameters in MultipleSequenceAlignmentData::moveRowsBlock: "
                       "startRow: '%1', numRows: '%2', delta: '%3'").arg(startRow).arg(numRows).arg(delta), );

    QList<MultipleSequenceAlignmentRow> toMove;
    int fromRow = delta > 0 ? startRow + numRows : startRow + delta;

    while (i < k) {
        const MultipleSequenceAlignmentRow row = rows.takeAt(fromRow);
        toMove << append(row);
        i++;
    }

    int toRow = delta > 0 ? startRow : startRow + numRows - k;

    while (toMove.count() > 0) {
        int n = toMove.count();
        MultipleSequenceAlignmentRow row = toMove[n - 1];
        toMove.removeAt(n - 1);
        rows.insert(toRow, row);
    }
}

QStringList MultipleSequenceAlignmentData::getRowNames() const {
    QStringList rowNames;
    foreach (const MultipleSequenceAlignmentRow &row, rows) {
        rowNames.append(row->getName());
    }
    return rowNames;
}

QList<qint64> MultipleSequenceAlignmentData::getRowsIds() const {
    QList<qint64> rowIds;
    foreach (const MultipleSequenceAlignmentRow &row, rows) {
        rowIds.append(row->getRowId());
    }
    return rowIds;
}

MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::getRowByRowId(qint64 rowId, U2OpStatus &os) const {
    foreach (const MultipleSequenceAlignmentRow &row, rows) {
        if (row->getRowId() == rowId) {
            return row;
        }
    }
    os.setError("Failed to find a row in an alignment");
    return MultipleSequenceAlignmentRow();
}

int MultipleSequenceAlignmentData::getRowIndexByRowId(qint64 rowId, U2OpStatus &os) const {
    for (int rowIndex = 0; rowIndex < rows.size(); ++rowIndex) {
        if (rows[rowIndex]->getRowId() == rowId) {
            return rowIndex;
        }
    }
    os.setError("Invalid row id");
    return MultipleSequenceAlignmentRow::invalidRowId();
}

char MultipleSequenceAlignmentData::charAt(int rowNumber, int pos) const {
    return rows[rowNumber]->charAt(pos);
}

void MultipleSequenceAlignmentData::setRowGapModel(int rowNumber, const QList<U2MsaGap> &gapModel) {
    SAFE_POINT(rowNumber >= 0 && rowNumber < getNumRows(), "Invalid row index", );
    length = qMax(length, MsaRowUtils::getGapsLength(gapModel) + rows[rowNumber]->sequence.length());
    rows[rowNumber]->setGapModel(gapModel);
}

void MultipleSequenceAlignmentData::setRowId(int rowNumber, qint64 rowId) {
    SAFE_POINT(rowNumber >= 0 && rowNumber < getNumRows(), "Invalid row index", );
    rows[rowNumber]->setRowId(rowId);
}

void MultipleSequenceAlignmentData::setSequenceId(int rowIndex, const U2DataId &sequenceId) {
    SAFE_POINT(rowIndex >= 0 && rowIndex < getNumRows(), "Invalid row index", );
    rows[rowIndex]->setSequenceId(sequenceId);
}

void MultipleSequenceAlignmentData::check() const {
#ifdef DEBUG
    assert(getNumRows() != 0 || length == 0);
    for (int i = 0, n = getNumRows(); i < n; i++) {
        assert(rows[i]->getCoreEnd() <= length);
    }
#endif
}

bool MultipleSequenceAlignmentData::sortRowsByList(const QStringList &rowsOrder) {
    MAStateCheck check(this);
    Q_UNUSED(check);

    const QStringList rowNames = getRowNames();
    foreach (const QString &rowName, rowNames) {
        CHECK(rowsOrder.contains(rowName), false);
    }

    QList<MultipleSequenceAlignmentRow> sortedRows;
    foreach (const QString &rowName, rowsOrder) {
        int rowIndex = rowNames.indexOf(rowName);
        if (rowIndex >= 0) {
            sortedRows << rows[rowIndex];
        }
    }

    rows = sortedRows;
    return true;
}

const MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::getRow(const QString &name) const{
    for (int i = 0; i < rows.count(); i++) {
        if (rows[i]->getName() == name) {
            return rows[i];
        }
    }
    SAFE_POINT(false,
        "Internal error: row name passed to MAlignmnet::getRow function not exists",
        MultipleSequenceAlignmentRow);
}

void MultipleSequenceAlignmentData::copy(const MultipleSequenceAlignmentData &other) {
    clear();

    alphabet = other.alphabet;
    length = other.length;
    info = other.info;

    for (int i = 0; i < other.rows.size(); i++) {
        const MultipleSequenceAlignmentRow row = createRow(other.rows[i]);
        addRowPrivate(row, other.length, i);
    }
}

} // namespace U2
