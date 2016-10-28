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

#include <U2Core/MsaRowUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "MultipleChromatogramAlignment.h"
#include "MultipleSequenceAlignmentInfo.h"

namespace U2 {

MultipleChromatogramAlignment::MultipleChromatogramAlignment()
    : mcaData(new MultipleChromatogramAlignmentData())
{

}

MultipleChromatogramAlignment::MultipleChromatogramAlignment(MultipleChromatogramAlignmentData *mcaData)
    : mcaData(mcaData)
{

}

MultipleChromatogramAlignment::MultipleChromatogramAlignment(const QString &name, const DNAAlphabet *alphabet, const QList<MultipleChromatogramAlignmentRow> &rows)
    : mcaData(new MultipleChromatogramAlignmentData(name, alphabet, rows))
{

}

MultipleChromatogramAlignmentData * MultipleChromatogramAlignment::data() const {
    return mcaData.data();
}

MultipleChromatogramAlignmentData & MultipleChromatogramAlignment::operator*() {
    return *mcaData;
}

const MultipleChromatogramAlignmentData & MultipleChromatogramAlignment::operator*() const {
    return *mcaData;
}

MultipleChromatogramAlignmentData * MultipleChromatogramAlignment::operator->() {
    return mcaData.data();
}

const MultipleChromatogramAlignmentData * MultipleChromatogramAlignment::operator->() const {
    return mcaData.data();
}

MultipleChromatogramAlignment MultipleChromatogramAlignment::clone() const {
    return mcaData->getCopy();
}

MultipleChromatogramAlignmentData::MultipleChromatogramAlignmentData()
{
    // TODO
}

MultipleChromatogramAlignmentData::MultipleChromatogramAlignmentData(const QString &name, const DNAAlphabet *alphabet, const QList<MultipleChromatogramAlignmentRow> &rows)
{
    // TODO
}

MultipleChromatogramAlignmentData::MultipleChromatogramAlignmentData(const MultipleChromatogramAlignmentData &mcaData)
{
    // TODO
}

MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::getRow(int rowIndex) {
    const int rowsCount = rows.count();
    SAFE_POINT(0 != rowsCount, "No rows", MultipleChromatogramAlignmentRow());
    SAFE_POINT(rowIndex >= 0 && (rowIndex < rowsCount), "Internal error: unexpected row index was passed to MultipleChromatogramAlignmentRow::getRow", MultipleChromatogramAlignmentRow());
    return rows[rowIndex];
}

const MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::getRow(int rowIndex) const {
    const int rowsCount = rows.count();
    SAFE_POINT(0 != rowsCount, "No rows", MultipleChromatogramAlignmentRow());
    SAFE_POINT(rowIndex >= 0 && (rowIndex < rowsCount), "Internal error: unexpected row index was passed to MultipleChromatogramAlignmentRow::getRow", MultipleChromatogramAlignmentRow());
    return rows[rowIndex];
}

const MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentData::getRow(const QString &name) const {
    for (int i = 0; i < rows.count(); i++) {
        if (rows[i]->getRowName() == name) {
            return rows[i];
        }
    }

    FAIL("Internal error: row name passed to MultipleChromatogramAlignmentData::getRow function not exists", MultipleChromatogramAlignmentRow());
}

const QList<MultipleChromatogramAlignmentRow> &MultipleChromatogramAlignmentData::getRows() const {
    return rows;
}

MultipleChromatogramAlignmentData & MultipleChromatogramAlignmentData::operator=(const MultipleChromatogramAlignment &msa) {
    return *this = *msa;
}

MultipleChromatogramAlignmentData & MultipleChromatogramAlignmentData::operator=(const MultipleChromatogramAlignmentData &msaData) {
    copy(msaData);
    return *this;
}

void MultipleChromatogramAlignmentData::clear() {
    rows.clear();
    length = 0;
}

bool MultipleChromatogramAlignmentData::isEmpty() const {
    return getLength() == 0;
}

bool MultipleChromatogramAlignmentData::isCommonGap(qint64 position, int rowNumber) const {
    SAFE_POINT(0 <= position && position < length, "Position is out of boundaries", true);
    SAFE_POINT(0 <= rowNumber && rowNumber < rows.size(), "Row number is out of boundaries", true);
    //! TODO: ???
//    rows[rowNumber]->
}

QString MultipleChromatogramAlignmentData::getName() const {
    return MultipleSequenceAlignmentInfo::getName(info);
}

void MultipleChromatogramAlignmentData::setName(const QString &newName) {
    MultipleSequenceAlignmentInfo::setName(info, newName);
}

const DNAAlphabet * MultipleChromatogramAlignmentData::getAlphabet() const {
    return alphabet;
}

void MultipleChromatogramAlignmentData::setAlphabet(const DNAAlphabet *newAlphabet) {
    SAFE_POINT(NULL != newAlphabet, "Internal error: attempted to set NULL alphabet for an alignment", );
    alphabet = newAlphabet;
}

QVariantMap MultipleChromatogramAlignmentData::getInfo() const {
    return info;
}

void MultipleChromatogramAlignmentData::setInfo(const QVariantMap &newInfo) {
    info = newInfo;
}

qint64 MultipleChromatogramAlignmentData::getLength() const {
    return length;
}

void MultipleChromatogramAlignmentData::setLength(int newLength) {
    // TODO: determine what 'crop' means for the row
    SAFE_POINT(newLength >= 0, QString("Internal error: attempted to set length '%1' for an alignment").arg(newLength), );

    if (newLength >= length) {
        length = newLength;
        return;
    }

    U2OpStatus2Log os;
    for (int i = 0, n = getNumRows(); i < n; i++) {
//        rows[i]->crop(0, newLength, os);
        CHECK_OP(os, );
    }
    length = newLength;
}

int MultipleChromatogramAlignmentData::getNumRows() const {
    return rows.size();
}

bool MultipleChromatogramAlignmentData::trim(bool removeLeadingGaps) {
    bool result = false;

    if (removeLeadingGaps) {
        // Verify if there are leading columns of gaps
        // by checking the first gap in each row
        qint64 leadingGapColumnsNum = 0;
        foreach (const MultipleChromatogramAlignmentRow &row, rows) {
            if (row->getCommonGapModel().count() > 0) {
                const U2MsaGap firstGap = row->getCommonGapModel().first();
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
                rows[i]->removeGaps(os, 0, leadingGapColumnsNum);
                CHECK_OP(os, true);
                result = true;
            }
        }
    }

    // Verify right side of the alignment (trailing gaps and rows' lengths)
    qint64 newLength = 0;
    foreach (const MultipleChromatogramAlignmentRow &row, rows) {
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

bool MultipleChromatogramAlignmentData::simplify() {
    qint64 newLen = 0;
    bool changed = false;
    for (int i = 0, n = rows.size(); i < n; i++) {
        changed |= !rows[i]->getCommonGapModel().isEmpty();
        rows[i]->setGapModel(U2MsaRowGapModel());
        newLen = qMax(newLen, rows[i]->getRowLengthWithoutTrailing());
    }

    if (!changed) {
        assert(length == newLen);
        return false;
    }
    length = newLen;
    return true;
}

class CompareMsaRowsByName {    // Copypasted from MultipleSequenceAlignment.cpp
public:
    CompareMsaRowsByName(MultipleChromatogramAlignmentData::Order order = MultipleChromatogramAlignmentData::Ascending)
        : order(order)
    {

    }

    bool operator()(const MultipleChromatogramAlignmentRow &row1, const MultipleChromatogramAlignmentRow &row2) const {
        const bool res = QString::compare(row1->getRowName(), row2->getRowName(), Qt::CaseInsensitive) > 0;
        return order == MultipleChromatogramAlignmentData::Ascending ? !res : res;
    }

private:
    MultipleChromatogramAlignmentData::Order order;
};


void MultipleChromatogramAlignmentData::sortRowsByName(MultipleChromatogramAlignmentData::Order order) {
    qStableSort(rows.begin(), rows.end(), CompareMsaRowsByName(order));
}

bool MultipleChromatogramAlignmentData::sortRowsBySimilarity() {
    // TODO
    QList<MultipleChromatogramAlignmentRow> oldRows = rows;
    QList<MultipleChromatogramAlignmentRow> sortedRows;
    while (!oldRows.isEmpty()) {
        const MultipleChromatogramAlignmentRow row = oldRows.takeFirst();
        sortedRows << row;
        QMutableListIterator<MultipleChromatogramAlignmentRow> iter(oldRows);
        while (iter.hasNext()) {
            const MultipleChromatogramAlignmentRow &next = iter.next();
//            if (next->isRowContentEqual(row)) {
//                sortedRows << next;
//                iter.remove();
//            }
        }
    }
    if (rows != sortedRows) {
        rows = sortedRows;
        return true;
    }
    return false;
}

QStringList MultipleChromatogramAlignmentData::getRowNames() const {
    QStringList rowNames;
    foreach (const MultipleChromatogramAlignmentRow &row, rows) {
        rowNames << row->getRowName();
    }
    return rowNames;
}

void MultipleChromatogramAlignmentData::insertGaps(int row, qint64 pos, qint64 count, U2OpStatus &os) {
    if (row >= getNumRows() || row < 0 || pos > length || pos < 0 || count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed "
            "to MultipleChromatogramAlignmentData::insertGaps: row index '%1', pos '%2', count '%3'").arg(row).arg(pos).arg(count));
        os.setError("Failed to insert gaps into an alignment");
        return;
    }

    if (pos == length) {
        // add trailing gaps --> just increase alignment len
        length += count;
        return;
    }

    if (pos >= rows[row]->getRowLengthWithoutTrailing()) {
        length += count;
        return;
    }
    rows[row]->insertGaps(os, pos, count);

    const qint64 rowLength = rows[row]->getRowLengthWithoutTrailing();
    length = qMax(length, rowLength);
}

void MultipleChromatogramAlignmentData::renameRow(int row, const QString &name) {
    SAFE_POINT(row >= 0 && row < getNumRows(),
        QString("Incorrect row index '%1' was passed to MultipleChromatogramAlignmentData::renameRow: "
        "the number of rows is '%2'").arg(row).arg(getNumRows()), );
    SAFE_POINT(!name.isEmpty(),
        "Incorrect parameter 'name' was passed to MultipleChromatogramAlignmentData::renameRow: "
        "Can't set the name of a row to an empty string", );
    rows[row]->setRowName(name);
}

bool MultipleChromatogramAlignmentData::crop(const U2Region &region, const QSet<QString> &rowNames, U2OpStatus &os) {
    if (!(region.startPos >= 0 && region.length > 0 && region.length < length && region.startPos < length)) {
        os.setError(QString("Incorrect region was passed to MultipleChromatogramAlignmentData::crop, "
                            "startPos '%1', length '%2'").arg(region.startPos).arg(region.length));
        return false;
    }

    int cropLen = region.length;
    if (region.endPos() > length) {
        cropLen -= (region.endPos() - length);
    }

    QList<MultipleChromatogramAlignmentRow> newList;
    for (int i = 0 ; i < rows.size(); i++) {
        MultipleChromatogramAlignmentRow row = rows[i].clone();
        const QString rowName = row->getRowName();
        if (rowNames.contains(rowName)) {
//            row->crop(region.startPos, cropLen, os);      // TODO: check, if mca crop is needed at all
            CHECK_OP(os, false);
            newList << row;
        }
    }
    rows = newList;

    length = cropLen;
    return true;
}

bool MultipleChromatogramAlignmentData::crop(const U2Region &region, U2OpStatus &os) {
    // TODO
//    return crop(region, getRowNames().toSet(), os);
    return true;
}

bool MultipleChromatogramAlignmentData::crop(int start, int count, U2OpStatus &os) {
    // TODO
//    return crop(U2Region(start, count), os);
    return true;
}

MultipleChromatogramAlignment MultipleChromatogramAlignmentData::mid(qint64 start, qint64 len) const {
    SAFE_POINT(start >= 0 && start + len <= length,
               QString("Incorrect parameters were passed to MultipleChromatogramAlignmentData::mid: "
                       "start '%1', len '%2', the alignment length is '%3'").arg(start).arg(len).arg(length),
               MultipleChromatogramAlignment());

    MultipleChromatogramAlignment res(getName(), alphabet);

    U2OpStatus2Log os;
    foreach (const MultipleChromatogramAlignmentRow &row, rows) {
        // TODO: decide, if anybody needs 'mid' method
//        MultipleChromatogramAlignmentRow mRow = row->mid(start, len, os);
//        mRow->setParentAlignment(res);
//        res->rows << mRow;
    }
    res->length = len;
    return res;
}

void MultipleChromatogramAlignmentData::removeRow(int rowIndex, U2OpStatus &os) {
    if (rowIndex < 0 || rowIndex >= getNumRows()) {
        coreLog.trace(QString("Internal error: incorrect parameters was passed to MultipleChromatogramAlignmentData::removeRow, "
            "rowIndex '%1', the number of rows is '%2'").arg(rowIndex).arg(getNumRows()));
        os.setError("Failed to remove a row");
        return;
    }

    rows.removeAt(rowIndex);

    if (rows.isEmpty()) {
        length = 0;
    }
}

void MultipleChromatogramAlignmentData::moveRowsBlock(int startRow, int numRows, int delta) {
    // Assumption: numRows is rather big, delta is small (1~2)
    // It's more optimal to move abs(delta) of rows then the block itself

    int i = 0;
    int k = qAbs(delta);

    SAFE_POINT((delta > 0 && startRow + numRows + delta - 1 < rows.length())
               || (delta < 0 && startRow + delta >= 0),
               QString("Incorrect parameters in MultipleChromatogramAlignmentData::moveRowsBlock: "
                       "startRow: '%1', numRows: '%2', delta: '%3'").arg(startRow).arg(numRows).arg(delta), );

    QList<MultipleChromatogramAlignmentRow> toMove;
    int fromRow = delta > 0 ? startRow + numRows : startRow + delta;

    while (i < k) {
        const MultipleChromatogramAlignmentRow row = rows.takeAt(fromRow);
        toMove << row;
        i++;
    }

    int toRow = delta > 0 ? startRow : startRow + numRows - k;

    while (toMove.count() > 0) {
        int n = toMove.count();
        MultipleChromatogramAlignmentRow row = toMove[n - 1];
        toMove.removeAt(n - 1);
        rows.insert(toRow, row);
    }
}

bool MultipleChromatogramAlignmentData::operator==(const MultipleChromatogramAlignmentData &mcaData) const {
    const bool lengthsAreEqual = (length == mcaData.length);
    const bool alphabetsAreEqual = (alphabet == mcaData.alphabet);
    const bool rowsAreEqual = (rows == mcaData.rows);
    return lengthsAreEqual && alphabetsAreEqual && rowsAreEqual;
}

bool MultipleChromatogramAlignmentData::operator!=(const MultipleChromatogramAlignmentData &mcaData) const {
    return !operator==(mcaData);
}

bool MultipleChromatogramAlignmentData::sortRowsByList(const QStringList &newNamesOrder) {
    const QStringList namesOrder = getRowNames();
    foreach (const QString &rowName, namesOrder) {
        CHECK(newNamesOrder.contains(rowName), false);
    }

    QList<MultipleChromatogramAlignmentRow> sortedRows;
    foreach (const QString &rowName, newNamesOrder) {
        int rowIndex = namesOrder.indexOf(rowName);
        if (rowIndex >= 0) {
            sortedRows << rows[rowIndex];
        }
    }

    rows = sortedRows;
    return true;
}

U2MsaListGapModel MultipleChromatogramAlignmentData::getPredictedSequencesGapModel() const {
    U2MsaListGapModel gapModel;
    foreach (const MultipleChromatogramAlignmentRow &row, rows) {
        gapModel << row->getPredictedSequenceGapModel();
    }
    return gapModel;
}

U2MsaListGapModel MultipleChromatogramAlignmentData::getEditedSequencesGapModel() const {
    U2MsaListGapModel gapModel;
    foreach (const MultipleChromatogramAlignmentRow &row, rows) {
        gapModel << row->getEditedSequenceGapModel();
    }
    return gapModel;
}

namespace {
    U2MsaRowGapModel uniteGapModels(const U2MsaListGapModel &gapModels) {
        U2MsaRowGapModel unitedGapModel;
        foreach (const U2MsaRowGapModel &rowGapModel, gapModels) {
            // TODO
        }
        return unitedGapModel;
    }
}

void MultipleChromatogramAlignmentData::setRowGapModel(int rowNumber, const U2MsaRowGapModel &gapModel) {
    U2MsaListGapModel guaranteedGapModel;
    foreach (const MultipleChromatogramAlignmentRow &row, rows) {
        guaranteedGapModel << row->getEditedSequenceGuaranteedGapModel();
    }
    const U2MsaRowGapModel unitedGuaranteedGapModel;
    const U2MsaRowGapModel gapModelWithGuaranteedGaps = MsaRowUtils::insertGapModel(gapModel, unitedGuaranteedGapModel);
    getRow(rowNumber)->setGapModel(gapModelWithGuaranteedGaps);
}

void MultipleChromatogramAlignmentData::setGapModel(const U2MsaListGapModel &gapModel) {
    // TODO
}

void MultipleChromatogramAlignmentData::addRow(const DNAChromatogram &chromatogram, const DNASequence &predictedSequence, const U2MsaRowGapModel &gapModel) {
    MultipleChromatogramAlignmentRow row(this, chromatogram, predictedSequence, gapModel);
    addRowPrivate(row);
}

void MultipleChromatogramAlignmentData::addRow(const DNAChromatogram &chromatogram,
                                               const DNASequence &predictedSequence,
                                               const U2MsaRowGapModel &predictedSequenceGapModel,
                                               const DNASequence &editedSequence,
                                               const U2MsaRowGapModel &editedSequenceGapModel,
                                               const U2Region &workingArea) {
    MultipleChromatogramAlignmentRow row(this, chromatogram, predictedSequence, predictedSequenceGapModel, editedSequence, editedSequenceGapModel, workingArea);
    addRowPrivate(row);
}

void MultipleChromatogramAlignmentData::addRow(const QString &rowName, const DNAChromatogram &chromatogram, const QByteArray &predictedSequenceRawData) {
    MultipleChromatogramAlignmentRow row(this, rowName, chromatogram, predictedSequenceRawData);
    addRowPrivate(row);
}

MultipleChromatogramAlignment MultipleChromatogramAlignmentData::getCopy() const {
    return MultipleChromatogramAlignment(new MultipleChromatogramAlignmentData(*this));
}

void MultipleChromatogramAlignmentData::copy(const MultipleChromatogramAlignmentData &mcaData) {
    clear();

    alphabet = mcaData.alphabet;
    length = mcaData.length;
    info = mcaData.info;

    for (int i = 0; i < mcaData.rows.size(); i++) {
        const MultipleChromatogramAlignmentRow row = mcaData.rows[i]->getCopy(this);
        addRowPrivate(row);
    }
}

void MultipleChromatogramAlignmentData::addRowPrivate(const MultipleChromatogramAlignmentRow &row) {
    length = qMax(length, row->getRowLengthWithoutTrailing());
    rows << row;
}

}   // namespace U2
