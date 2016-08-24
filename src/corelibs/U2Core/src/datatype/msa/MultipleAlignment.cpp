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
#include <U2Core/U2SafePoints.h>

#include "MultipleAlignment.h"
#include "MultipleAlignmentInfo.h"

namespace U2 {

const char MultipleSequenceAlignment::GapChar = '-';

// Helper class to call MultipleAlignmentData state check
class MaStateCheck {
public:
    MaStateCheck(const MultipleSequenceAlignmentData *ma)
        : ma(ma)
    {

    }

    ~MaStateCheck() {
#ifdef _DEBUG
        ma->check();
#else
        Q_UNUSED(ma);
#endif
    }

private:
    const MultipleSequenceAlignmentData *ma;
};

MultipleSequenceAlignment::MultipleSequenceAlignment(MultipleSequenceAlignmentData *msaData)
    : msaData(msaData)
{

}

MultipleSequenceAlignment::MultipleSequenceAlignment(const QString &name, const DNAAlphabet *alphabet, const QList<MultipleSequenceAlignmentRow> &rows)
    : msaData(new MultipleSequenceAlignmentData(name, alphabet, rows))
{

}

MultipleSequenceAlignment::MultipleSequenceAlignment(const MultipleSequenceAlignmentData &msaData)
    : msaData(new MultipleSequenceAlignmentData(msaData))
{

}

MultipleSequenceAlignment::~MultipleSequenceAlignment() {

}

MultipleSequenceAlignmentData * MultipleSequenceAlignment::data() const {
    return msaData.data();
}

MultipleSequenceAlignmentData & MultipleSequenceAlignment::operator*() {
    return *msaData;
}

const MultipleSequenceAlignmentData & MultipleSequenceAlignment::operator*() const {
    return *msaData;
}

MultipleSequenceAlignmentData *MultipleSequenceAlignment::operator->() {
    return msaData.data();
}

const MultipleSequenceAlignmentData * MultipleSequenceAlignment::operator->() const {
    return msaData.data();
}

void MultipleSequenceAlignment::operator+=(const MultipleSequenceAlignment &other) {
    *msaData += *other.msaData;
}

MultipleSequenceAlignment MultipleSequenceAlignment::clone() const {
    return msaData->getCopy();
}

MultipleSequenceAlignmentData::MultipleSequenceAlignmentData(const QString &name, const DNAAlphabet *alphabet, const QList<MultipleSequenceAlignmentRow> &rows)
    : alphabet(alphabet),
      rows(rows),
      length(0)
{
    MaStateCheck check(this);
    Q_UNUSED(check);

    SAFE_POINT(NULL == alphabet || !name.isEmpty(), "Incorrect parameters in MultipleAlignmentData ctor", );        // TODO: check the condition, it is strange

    setName(name);
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

MultipleSequenceAlignmentData::~MultipleSequenceAlignmentData() {

}

const MultipleSequenceAlignmentData & MultipleSequenceAlignmentData::operator=(const MultipleSequenceAlignmentData &other) {
    copy(other);
    return *this;
}

void MultipleSequenceAlignmentData::clear() {
    MaStateCheck check(this);
    Q_UNUSED(check);
    rows.clear();
    length = 0;
}

QString MultipleSequenceAlignmentData::getName() const {
    return MultipleAlignmentInfo::getName(info);
}

void MultipleSequenceAlignmentData::setName(const QString &newName) {
    MultipleAlignmentInfo::setName(info, newName);
}

const DNAAlphabet * MultipleSequenceAlignmentData::getAlphabet() const {
    return alphabet;
}

void MultipleSequenceAlignmentData::setAlphabet(const DNAAlphabet *_alphabet) {
    SAFE_POINT(NULL != _alphabet, "Internal error: attempted to set NULL alphabet for the alignment",);
    alphabet = _alphabet;
}

QVariantMap MultipleSequenceAlignmentData::getInfo() const {
    return info;
}

void MultipleSequenceAlignmentData::setInfo(const QVariantMap &_info) {
    info = _info;
}

bool MultipleSequenceAlignmentData::isEmpty() const {
    return getLength() == 0;
}

int MultipleSequenceAlignmentData::getLength() const {
    return length;
}

void MultipleSequenceAlignmentData::setLength(int newLength) {
    SAFE_POINT(newLength >= 0, QString("Internal error: attempted to set length '%1' for an alignment").arg(newLength), );

    MaStateCheck check(this);
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

bool MultipleSequenceAlignmentData::trim(bool removeLeadingGaps) {
    MaStateCheck check(this);
    Q_UNUSED(check);

    bool result = false;

    if (removeLeadingGaps) {
        // Verify if there are leading columns of gaps
        // by checking the first gap in each row
        qint64 leadingGapColumnsNum = 0;
        foreach (const MultipleSequenceAlignmentRow &row, rows) {
            if (row->getGapModel().count() > 0) {
                const U2MsaGap firstGap = row->getGapModel().first();
                if (firstGap.offset > 0) {
                    leadingGapColumnsNum = 0;
                    break;
                } else {
                    leadingGapColumnsNum = (leadingGapColumnsNum == 0 ? firstGap.gap : qMin(leadingGapColumnsNum, firstGap.gap));
                }
            } else {
                leadingGapColumnsNum = 0;
                break;
            }
        }

        // If there are leading gap columns, remove them
        if (leadingGapColumnsNum > 0) {
            for (int i = 0; i < rows.count(); ++i) {
                U2OpStatusImpl os;
                rows[i]->removeData(0, leadingGapColumnsNum, os);
                CHECK_OP(os, false);
                result = true;
            }
        }
    }

    // Verify right side of the alignment (trailing gaps and rows' lengths)
    int newLength = 0;
    foreach (const MultipleSequenceAlignmentRow &row, rows) {
        newLength = qMax(row->getRowLengthWithoutTrailing(), newLength);
    }

    if (newLength != length) {
        length = newLength;
        result = true;
    }

    return result;
}

bool MultipleSequenceAlignmentData::simplify() {
    MaStateCheck check(this);
    Q_UNUSED(check);

    int newLen = 0;
    bool changed = false;
    for (int i = 0, n = rows.size(); i < n; i++) {
        changed = rows[i]->simplify() || changed;
        newLen = qMax(newLen, rows[i]->getCoreEnd());
    }

    if (!changed) {
        assert(length == newLen);
        return false;
    }

    length = newLen;
    return true;
}

class CompareMsaRowsByName {
public:
    CompareMsaRowsByName(bool asc = true)
        : asc(asc)
    {

    }

    bool operator()(const MultipleSequenceAlignmentRow &row1, const MultipleSequenceAlignmentRow &row2) const {
        bool res = QString::compare(row1->getName(), row2->getName(), Qt::CaseInsensitive) > 0;
        return asc ? !res : res;
    }

private:
    bool asc;
};

void MultipleSequenceAlignmentData::sortRowsByName(bool asc) {
    MaStateCheck check(this);
    Q_UNUSED(check);
    qStableSort(rows.begin(), rows.end(), CompareMsaRowsByName(asc));
}

bool MultipleSequenceAlignmentData::sortRowsBySimilarity(QVector<U2Region> &united) {
    QList<MultipleSequenceAlignmentRow> oldRows = rows;
    QList<MultipleSequenceAlignmentRow> sortedRows;
    while (!oldRows.isEmpty()) {
        const MultipleSequenceAlignmentRow row = oldRows.takeFirst();
        sortedRows.append(row);
        int start = sortedRows.size() - 1;
        int len = 1;
        QMutableListIterator<MultipleSequenceAlignmentRow> iter(oldRows);
        while (iter.hasNext()) {
            const MultipleSequenceAlignmentRow &next = iter.next();
            if (next->isRowContentEqual(row)) {
                sortedRows.append(next);
                iter.remove();
                ++len;
            }
        }
        if (len > 1) {
            united.append(U2Region(start, len));
        }
    }
    if (rows != sortedRows) {
        rows = sortedRows;
        return true;
    }
    return false;
}

MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::getRow(int rowIndex) {
    int rowsCount = rows.count();
    SAFE_POINT(0 != rowsCount, "No rows", getEmptyRow());
    SAFE_POINT(rowIndex >= 0 && (rowIndex < rowsCount), "Internal error: unexpected row index was passed to MAlignmnet::getRow", getEmptyRow());
    return rows[rowIndex];
}

const MultipleSequenceAlignmentRow &MultipleSequenceAlignmentData::getRow(int rowIndex) const {
    static MultipleSequenceAlignmentRow emptyRow = getEmptyRow();
    int rowsCount = rows.count();
    SAFE_POINT(0 != rowsCount, "No rows", emptyRow);
    SAFE_POINT(rowIndex >= 0 && (rowIndex < rowsCount), "Internal error: unexpected row index was passed to MAlignmnet::getRow", emptyRow);
    return rows[rowIndex];
}

const MultipleSequenceAlignmentRow &MultipleSequenceAlignmentData::getRow(const QString &name) const {
    static MultipleSequenceAlignmentRow emptyRow = getEmptyRow();
    for (int i = 0; i < rows.count(); i++) {
        if (rows[i]->getName() == name) {
            return rows[i];
        }
    }
    SAFE_POINT(false, "Internal error: row name passed to MAlignmnet::getRow function not exists", emptyRow);
}

const QList<MultipleSequenceAlignmentRow> & MultipleSequenceAlignmentData::getRows() const {
    return rows;
}

MultipleSequenceAlignmentRow MultipleSequenceAlignmentData::getRowByRowId(qint64 rowId, U2OpStatus &os) const {
    static MultipleSequenceAlignmentRow emptyRow = getEmptyRow();
    foreach (const MultipleSequenceAlignmentRow &row, rows) {
        if (row->getRowId() == rowId) {
            return row;
        }
    }
    os.setError("Failed to find a row in an alignment");
    return emptyRow;
}

QList<qint64> MultipleSequenceAlignmentData::getRowsIds() const {
    QList<qint64> rowIds;
    foreach (const MultipleSequenceAlignmentRow &row, rows) {
        rowIds.append(row->getRowId());
    }
    return rowIds;
}

QStringList MultipleSequenceAlignmentData::getRowNames() const {
    QStringList rowNames;
    foreach (const MultipleSequenceAlignmentRow &row, rows) {
        rowNames.append(row->getName());
    }
    return rowNames;
}

int MultipleSequenceAlignmentData::getRowIndexByRowId(qint64 rowId, U2OpStatus &os) const {
    for (int rowIndex = 0; rowIndex < rows.size(); ++rowIndex) {
        if (rows.at(rowIndex)->getRowId() == rowId) {
            return rowIndex;
        }
    }
    os.setError("Invalid row id");
    return MultipleAlignmentRowData::INVALID_ROW_ID;
}

void MultipleSequenceAlignmentData::insertGaps(int row, int pos, int count, U2OpStatus &os) {
    if (row >= getNumRows() || row < 0 || pos > length || pos < 0 || count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed"
                              " to MultipleAlignmentData::insertGaps: row index '%1', pos '%2', count '%3'").arg(row).arg(pos).arg(count));
        os.setError("Failed to insert gaps into an alignment");
        return;
    }

    if (pos == length) {
        // add trailing gaps --> just increase alignment len
        length += count;
        return;
    }

    MaStateCheck check(this);
    Q_UNUSED(check);

    if (pos >= rows[row]->getRowLengthWithoutTrailing()) {
        length += count;
        return;
    }
    rows[row]->insertGaps(pos, count, os);

    int rowLength = rows[row]->getRowLengthWithoutTrailing();
    length = qMax(length, rowLength);
}

void MultipleSequenceAlignmentData::removeRowData(int row, int pos, int count, U2OpStatus &os) {
    if (row >= getNumRows() || row < 0 || pos > length || pos < 0 || count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed"
            " to MultipleAlignmentData::removeChars: row index '%1', pos '%2', count '%3'").arg(row).arg(pos).arg(count));
        os.setError("Failed to remove chars from an alignment");
        return;
    }

    MaStateCheck check(this);
    Q_UNUSED(check);

    rows[row]->removeData(pos, count, os);
}

void MultipleSequenceAlignmentData::removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows) {
    SAFE_POINT(startPos >= 0 && startPos + nBases <= length && nBases > 0,
        QString("Incorrect parameters were passed to MultipleAlignmentData::removeRegion: startPos '%1',"
        " nBases '%2', the length is '%3'").arg(startPos).arg(nBases).arg(length), );
    SAFE_POINT(startRow >= 0 && startRow + nRows <= getNumRows() && nRows > 0,
        QString("Incorrect parameters were passed to MultipleAlignmentData::removeRegion: startRow '%1',"
        " nRows '%2', the number of rows is '%3'").arg(startRow).arg(nRows).arg(getNumRows()), );

    MaStateCheck check(this);
    Q_UNUSED(check);

    U2OpStatus2Log os;
    for (int i = startRow + nRows; --i >= startRow; ) {
        rows[i]->removeData(startPos, nBases, os);
        SAFE_POINT_OP(os, );

        if (removeEmptyRows && (0 == rows[i]->getCoreLength())) {
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

void MultipleSequenceAlignmentData::renameRow(int row, const QString &name) {
    SAFE_POINT(row >= 0 && row < getNumRows(),
        QString("Incorrect row index '%1' was passed to MultipleAlignmentData::renameRow: "
        "the number of rows is '%2'").arg(row).arg(getNumRows()), );
    SAFE_POINT(!name.isEmpty(),
        "Incorrect parameter 'name' was passed to MultipleAlignmentData::renameRow: "
        "Can't set the name of a row to an empty string", );
    rows[row]->setName(name);
}

bool MultipleSequenceAlignmentData::crop(const U2Region &region, const QSet<QString> &rowNames, U2OpStatus &os) {
    if (!(region.startPos >= 0 && region.length > 0 && region.length < length && region.startPos < length)) {
        os.setError(QString("Incorrect region was passed to MultipleAlignmentData::crop, "
                            "startPos '%1', length '%2'").arg(region.startPos).arg(region.length));
        return false;
    }

    int cropLen = region.length;
    if (region.endPos() > length) {
        cropLen -= (region.endPos() - length);
    }

    MaStateCheck check(this);
    Q_UNUSED(check);

    QList<MultipleSequenceAlignmentRow> newList;
    for (int i = 0 ; i < rows.size(); i++) {
        const QString &rowName = rows[i]->getName();
        if (rowNames.contains(rowName)) {
            rows[i]->crop(region.startPos, cropLen, os);
            CHECK_OP(os, false);
            newList.append(rows[i]);
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

U2MsaListGapModel MultipleSequenceAlignmentData::getGapModel() const {
    U2MsaListGapModel gapModel;
    foreach (const MultipleSequenceAlignmentRow &row, rows) {
        gapModel << row->getGapModel();
    }
    return gapModel;
}

void MultipleSequenceAlignmentData::setRowGapModel(int rowIndex, const U2MsaRowGapModel &gapModel) {
    SAFE_POINT(rowIndex >= 0 && rowIndex < getNumRows(), "Invalid row index", );
    MultipleSequenceAlignmentRow &row = rows[rowIndex];
    row->setGapModel(gapModel);
    length = qMax(length, row->getRowLengthWithoutTrailing());
}

void MultipleSequenceAlignmentData::setRowId(int rowIndex, qint64 rowId) {
    SAFE_POINT(rowIndex >= 0 && rowIndex < getNumRows(), "Invalid row index", );
    rows[rowIndex]->setRowId(rowId);
}

void MultipleSequenceAlignmentData::setDataId(int rowIndex, const U2DataId &dataId) {
    SAFE_POINT(rowIndex >= 0 && rowIndex < getNumRows(), "Invalid row index", );
    rows[rowIndex]->setDataId(dataId);
}

void MultipleSequenceAlignmentData::removeRow(int rowIndex, U2OpStatus &os) {
    if (rowIndex < 0 || rowIndex >= getNumRows()) {
        coreLog.trace(QString("Internal error: incorrect parameters was passed to MultipleAlignmentData::removeRow, "
                              "rowIndex '%1', the number of rows is '%2'").arg(rowIndex).arg(getNumRows()));
        os.setError("Failed to remove a row");
        return;
    }

    MaStateCheck check(this);
    Q_UNUSED(check);

    rows.removeAt(rowIndex);

    if (rows.isEmpty()) {
        length = 0;
    }
}

void MultipleSequenceAlignmentData::moveRowsBlock(int startRow, int numRows, int delta) {
    MaStateCheck check(this);
    Q_UNUSED(check);

    // Assumption: numRows is rather big, delta is small (1~2)
    // It's more optimal to move abs(delta) of rows then the block itself

    int i = 0;
    int k = qAbs(delta);

    SAFE_POINT((delta > 0 && startRow + numRows + delta - 1 < rows.length())
               || (delta < 0 && startRow + delta >= 0),
               QString("Incorrect parameters in MultipleAlignmentData::moveRowsBlock: "
                       "startRow: '%1', numRows: '%2', delta: '%3'").arg(startRow).arg(numRows).arg(delta),);

    QList<MultipleSequenceAlignmentRow> toMove;
    int fromRow = delta > 0 ? startRow + numRows  : startRow + delta;

    while (i <  k) {
        const MultipleSequenceAlignmentRow row = rows.takeAt(fromRow);
        toMove.append(row);
        i++;
    }

    int toRow = delta > 0 ? startRow : startRow + numRows - k;

    while (toMove.count() > 0) {
        int n = toMove.count();
        const MultipleSequenceAlignmentRow row = toMove.takeAt(n - 1);
        rows.insert(toRow, row);
    }
}

void MultipleSequenceAlignmentData::appendRow(int rowIdx, const MultipleSequenceAlignmentRow &row, bool ignoreTrailingGaps, U2OpStatus &os) {
    appendRow(rowIdx, ignoreTrailingGaps ? rows[rowIdx]->getRowLengthWithoutTrailing()
                                         : rows[rowIdx]->getRowLength(), row, os);
}

void MultipleSequenceAlignmentData::appendRow(int rowIdx, int afterPos, const MultipleSequenceAlignmentRow &row, U2OpStatus &os) {
    SAFE_POINT(0 <= rowIdx && rowIdx < getNumRows(), QString("Incorrect row index '%1' in MultipleAlignmentData::appendRow").arg(rowIdx), );

    rows[rowIdx]->append(row, afterPos, os);
    CHECK_OP(os, );

    length = qMax(length, afterPos + row->getRowLength());
}

bool MultipleSequenceAlignmentData::hasEmptyGapModel() const {
    for (int i = 0, n = rows.size(); i < n; ++i) {
        if (!rows[i]->getGapModel().isEmpty()) {
            return false;
        }
    }
    return true;
}

bool MultipleSequenceAlignmentData::hasEqualDataLength() const {
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

const MultipleSequenceAlignmentData & MultipleSequenceAlignmentData::operator+=(const MultipleSequenceAlignmentData &ma) {
    MaStateCheck check(this);
    Q_UNUSED(check);

    SAFE_POINT(ma.alphabet == alphabet, "Different alphabets in MultipleAlignmentData::operator+=", *this);

    int nSeq = getNumRows();
    SAFE_POINT(ma.getNumRows() == nSeq, "Different number of rows in MultipleAlignmentData::operator+=", *this);

    U2OpStatus2Log os;
    for (int i = 0; i < nSeq; i++) {
        rows[i]->append(ma.rows[i], length, os);
    }

    length += ma.length;
    return *this;
}

bool MultipleSequenceAlignmentData::operator==(const MultipleSequenceAlignmentData &other) const {
    const bool lengthsAreEqual = (length == other.length);
    const bool alphabetsAreEqual = (alphabet == other.alphabet);
    bool rowsAreEqual = (rows.size() == other.rows.size());
    for (int i = 0; i < rows.size() && rowsAreEqual; i++) {
     rowsAreEqual &= (*rows[i] == *other.rows[i]);
    }
    return lengthsAreEqual && alphabetsAreEqual && rowsAreEqual;
}

bool MultipleSequenceAlignmentData::operator!=(const MultipleSequenceAlignmentData &other) const {
    return !operator==(other);
}

void MultipleSequenceAlignmentData::check() const {
#ifdef DEBUG
    assert(getNumRows() != 0 || length == 0);
    for (int i = 0, n = getNumRows(); i < n; i++) {
        assert(rows[i].getCoreEnd() <= length);
    }
#endif
}

bool MultipleSequenceAlignmentData::sortRowsByList(const QStringList &rowsOrder) {
    MaStateCheck check(this);
    Q_UNUSED(check);

    const QStringList rowNames = getRowNames();
    foreach (const QString &rowName, rowNames) {
        CHECK(rowsOrder.contains(rowName), false);
    }

    QList<MultipleSequenceAlignmentRow> sortedRows;
    foreach (const QString &rowName, rowsOrder) {
        int rowIndex = rowNames.indexOf(rowName);
        if (rowIndex >= 0) {
            sortedRows.append(rows[rowIndex]);
        }
    }

    rows = sortedRows;
    return true;
}

bool MultipleSequenceAlignmentData::isGap(int row, int pos) const {
    return rows[row]->isGap(pos);
}

void MultipleSequenceAlignmentData::copy(const MultipleSequenceAlignmentData &other) {
    clear();

    alphabet = other.alphabet;
    length = other.length;
    info = other.info;

    for (int i = 0; i < other.rows.size(); i++) {
        const MultipleSequenceAlignmentRow r = other.createRow(other.rows.at(i));
        addRowPrivate(r, other.length, i);
    }
}

char MultipleSequenceAlignmentData::charAt(int rowIndex, int pos) const {
    SAFE_POINT(0 <= rowIndex && rowIndex < getRows().size(), QString("Row index is out of range: %1").arg(rowIndex), MultipleSequenceAlignment::GapChar);
    return rows[rowIndex]->charAt(pos);
}

void MultipleSequenceAlignmentData::setRowContent(int row, const QByteArray &rawData, int offset) {
    SAFE_POINT(row >= 0 && row < getNumRows(),
               QString("Incorrect row index '%1' was passed to MultipleSequenceAlignmentData::setRowContent: "
                       "the number of rows is '%2'").arg(row).arg(getNumRows()), );
    MsaStateCheck check(this);
    Q_UNUSED(check);

    rows[rowIndex]->setRowContent(rawData, offset);
    setLength(qMax(getLength(), rawData.size() + offset));
}

void MultipleSequenceAlignmentData::toUpperCase() {
    for (int i = 0, n = getNumRows(); i < n; i++) {
        rows[rowIndex]->toUpperCase();
    }
}

MultipleSequenceAlignment MultipleSequenceAlignmentData::mid(int start, int len) const {
    SAFE_POINT(start >= 0 && start + len <= getLength(),
        QString("Incorrect parameters were passed to MultipleSequenceAlignmentData::mid: "
        "start '%1', len '%2', the alignment length is '%3'").arg(start).arg(len).arg(getLength()),
        MultipleSequenceAlignment());

    MultipleSequenceAlignment res(getName(), getAlphabet());
    MsaStateCheck check(res.data());
    Q_UNUSED(check);

    U2OpStatus2Log os;
    foreach (const MultipleSequenceAlignmentRow &row, rows) {
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

void MultipleSequenceAlignmentData::addRow(const U2MsaRow &rowInDb, const DNASequence &sequence, U2OpStatus &os) {
    MultipleSequenceAlignmentRow newRow = createSequenceRow(rowInDb, sequence, rowInDb.gaps, os);
    CHECK_OP(os, );
    addRowPrivate(newRow, rowInDb.length, -1);
}

void MultipleSequenceAlignmentData::addRow(const QString &name, const DNASequence &sequence, const U2MsaRowGapModel &gaps, U2OpStatus &os) {
    U2MsaRow row;
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
    rows[rowIndex]->replaceChars(origChar, resultChar, os);
}

void MultipleSequenceAlignmentData::appendChars(int row, const char* str, int len) {
    SAFE_POINT(0 <= row && row < getNumRows(), QString("Incorrect row index '%1' in MultipleSequenceAlignmentData::appendChars").arg(row), );
    appendChars(row, rows[rowIndex]->getRowLength(), str, len);
}

void MultipleSequenceAlignmentData::appendChars(int row, int afterPos, const char *str, int len) {
    SAFE_POINT(0 <= row && row < getNumRows(), QString("Incorrect row index '%1' in MultipleSequenceAlignmentData::appendChars").arg(row), );

    const MultipleSequenceAlignmentRow appendedRow = createSequenceRow("", QByteArray(str, len));
    U2OpStatus2Log os;
    rows[rowIndex]->append(appendedRow, afterPos, os);
    CHECK_OP(os, );

    setLength(qMax(getLength(), afterPos + len));
}

MultipleSequenceAlignment MultipleSequenceAlignmentData::getCopy() const {
    return getExplicitCopy();
}

void MultipleSequenceAlignmentData::addRowPrivate(const MultipleSequenceAlignmentRow &row, int rowLenWithTrailingGaps, int rowIndex) {
    MaStateCheck check(this);
    Q_UNUSED(check);

    length = qMax(rowLenWithTrailingGaps, length);
    int idx = rowIndex == -1 ? getNumRows() : qBound(0, rowIndex, getNumRows());
    rows.insert(idx, row);
}

}   // namespace U2
