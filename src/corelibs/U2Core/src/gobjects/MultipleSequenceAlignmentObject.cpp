/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
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

#include <U2Core/DocumentModel.h>
#include <U2Core/GHints.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/MultipleSequenceAlignmentExporter.h>
#include <U2Core/MultipleSequenceAlignmentImporter.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "MultipleSequenceAlignmentObject.h"

namespace U2 {

MultipleSequenceAlignmentObject::MultipleSequenceAlignmentObject(const QString &name,
                                                                 const U2EntityRef &msaRef,
                                                                 const QVariantMap &hintsMap,
                                                                 const MultipleSequenceAlignment &alnData)
    : MultipleAlignmentObject(GObjectTypes::MULTIPLE_SEQUENCE_ALIGNMENT, name, msaRef, hintsMap, alnData)
{

}

const MultipleSequenceAlignment MultipleSequenceAlignmentObject::getMsa() const {
    return getMultipleAlignment().dynamicCast<MultipleSequenceAlignment>();
}

const MultipleSequenceAlignment MultipleSequenceAlignmentObject::getMsaCopy() const {
    return getMsa()->getExplicitCopy();
}

GObject * MultipleSequenceAlignmentObject::clone(const U2DbiRef &dstDbiRef, U2OpStatus &os, const QVariantMap &hints) const {
    DbiOperationsBlock opBlock(dstDbiRef, os);
    Q_UNUSED(opBlock);
    CHECK_OP(os, NULL);

    QScopedPointer<GHintsDefaultImpl> gHints(new GHintsDefaultImpl(getGHintsMap()));
    gHints->setAll(hints);
    const QString dstFolder = gHints->get(DocumentFormat::DBI_FOLDER_HINT, U2ObjectDbi::ROOT_FOLDER).toString();

    MultipleSequenceAlignment msa = getMsa()->getExplicitCopy();
    MultipleSequenceAlignmentObject *clonedObj = MultipleSequenceAlignmentImporter::createAlignment(dstDbiRef, dstFolder, msa, os);
    CHECK_OP(os, NULL);

    clonedObj->setGHints(gHints.take());
    clonedObj->setIndexInfo(getIndexInfo());
    return clonedObj;
}

bool MultipleSequenceAlignmentObject::isRegionEmpty(int startPos, int startRow, int numChars, int numRows) const {
    const MultipleSequenceAlignment msa = getMultipleAlignment();
    bool isBlockEmpty = true;
    for (int row = startRow; row < startRow + numRows && isBlockEmpty; ++row) {
        for (int pos = startPos; pos < startPos + numChars; ++pos) {
            if (!msa->isGap(row, pos)) {
                isBlockEmpty = false;
                break;
            }
        }
    }
    return isBlockEmpty;
}

char MultipleSequenceAlignmentObject::charAt(int seqNum, qint64 position) const {
    return getMultipleAlignment()->charAt(seqNum, position);
}

const MultipleSequenceAlignmentRow MultipleSequenceAlignmentObject::getMsaRow(int row) const {
    return getRow(row).dynamicCast<MultipleSequenceAlignmentRow>();
}

void MultipleSequenceAlignmentObject::insertGap(const U2Region &rows, int pos, int count) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", );
    const MultipleSequenceAlignment &msa = getMultipleAlignment();
    int startSeq = rows.startPos;
    int endSeq = startSeq + rows.length;

    QList<qint64> rowIdsToInsert;
    for (int i = startSeq; i < endSeq; ++i) {
        qint64 rowId = msa->getRow(i)->getRowId();
        rowIdsToInsert.append(rowId);
    }

    U2OpStatus2Log os;
    MsaDbiUtils::insertGaps(entityRef, rowIdsToInsert, pos, count, os);
    SAFE_POINT_OP(os, );

    MaModificationInfo mi;
    mi.rowListChanged = false;
    mi.modifiedRowIds = rowIdsToInsert;
    updateCachedMultipleAlignment(mi);
}

int MultipleSequenceAlignmentObject::deleteGap(U2OpStatus &os, const U2Region &rows, int pos, int maxGaps) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", 0);

    const int removingGapColumnCount = getMaxWidthOfGapRegion(os, rows, pos, maxGaps);
    SAFE_POINT_OP(os, 0);
    if (0 == removingGapColumnCount) {
        return 0;
    } else if (removingGapColumnCount < maxGaps) {
        pos += maxGaps - removingGapColumnCount;
    }
    QList<qint64> modifiedRowIds;
    modifiedRowIds.reserve(rows.length);

    MultipleSequenceAlignment msa = getMsa()->getExplicitCopy();
    // iterate through given rows to update each of them in DB
    for (int rowCount = rows.startPos; rowCount < rows.endPos(); ++rowCount) {
        msa->removeChars(rowCount, pos, removingGapColumnCount, os);
        CHECK_OP(os, 0);

        const MultipleSequenceAlignmentRow row = msa->getMsaRow(rowCount);
        MaDbiUtils::updateRowGapModel(entityRef, row->getRowId(), row->getGapModel(), os);
        CHECK_OP(os, 0);
        modifiedRowIds << row->getRowId();
    }
    if (rows.startPos == 0 && rows.length == getNumRows()) {
        // delete columns
        MaDbiUtils::updateMaLength(entityRef, getLength() - removingGapColumnCount, os);
        CHECK_OP(os, 0);
    }

    MaModificationInfo mi;
    mi.rowListChanged = false;
    mi.modifiedRowIds = modifiedRowIds;
    updateCachedMultipleAlignment(mi);
    return removingGapColumnCount;
}

void MultipleSequenceAlignmentObject::updateGapModel(U2OpStatus &os, const U2MsaMapGapModel &rowsGapModel) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", );

    const MultipleSequenceAlignment msa = getMultipleAlignment();

    const QList<qint64> rowIds = msa->getRowsIds();
    QList<qint64> modifiedRowIds;
    foreach (qint64 rowId, rowsGapModel.keys()) {
        if (!rowIds.contains(rowId)) {
            os.setError("Can't update gaps of a multiple alignment");
            return;
        }

        MaDbiUtils::updateRowGapModel(entityRef, rowId, rowsGapModel.value(rowId), os);
        CHECK_OP(os, );
        modifiedRowIds.append(rowId);
    }

    MaModificationInfo mi;
    mi.rowListChanged = false;
    updateCachedMultipleAlignment(mi);
}

void MultipleSequenceAlignmentObject::updateGapModel(const QList<MultipleSequenceAlignmentRow> &sourceRows) {
    const QList<MultipleSequenceAlignmentRow> oldRows = getMsa()->getMsaRows();

    SAFE_POINT(oldRows.count() == sourceRows.count(), "Different rows count", );

    U2MsaMapGapModel newGapModel;
    QList<MultipleSequenceAlignmentRow>::ConstIterator oldRowsIterator = oldRows.begin();
    QList<MultipleSequenceAlignmentRow>::ConstIterator sourceRowsIterator = sourceRows.begin();
    for (; oldRowsIterator != oldRows.end(); oldRowsIterator++, sourceRowsIterator++) {
        newGapModel[(*oldRowsIterator)->getRowId()] = (*sourceRowsIterator)->getGapModel();
    }

    U2OpStatus2Log os;
    updateGapModel(os, newGapModel);
}

U2MsaMapGapModel MultipleSequenceAlignmentObject::getGapModel() const {
    U2MsaMapGapModel rowsGapModel;
    foreach (const MultipleSequenceAlignmentRow &curRow, getMsa()->getMsaRows()) {
        rowsGapModel[curRow->getRowId()] = curRow->getGapModel();
    }
    return rowsGapModel;
}

namespace {

template<typename T>
inline QList<T> mergeLists(const QList<T> &first, const QList<T> &second) {
    QList<T> result = first;
    foreach (const T &item, second) {
        if (!result.contains(item)) {
            result.append(item);
        }
    }
    return result;
}

QList<qint64> getRowsAffectedByDeletion(const MultipleSequenceAlignment &ma, const QList<qint64> &removedRowIds) {
    QList<qint64> rowIdsAffectedByDeletion;
    U2OpStatus2Log os;
    const QList<qint64> maRows = ma->getRowsIds();
    int previousRemovedRowIndex = -1;
    foreach (qint64 removedRowId, removedRowIds) {
        if (-1 != previousRemovedRowIndex) {
            const int currentRemovedRowIndex = ma->getRowIndexByRowId(removedRowId, os);
            SAFE_POINT_OP(os, QList<qint64>());
            SAFE_POINT(currentRemovedRowIndex > previousRemovedRowIndex, "Rows order violation", QList<qint64>());
            const int countOfUnchangedRowsBetween = currentRemovedRowIndex - previousRemovedRowIndex - 1;
            if (0 < countOfUnchangedRowsBetween) {
                for (int middleRowIndex = previousRemovedRowIndex + 1; middleRowIndex < currentRemovedRowIndex; ++middleRowIndex) {
                    rowIdsAffectedByDeletion += maRows[middleRowIndex];
                }
            }
        }
        previousRemovedRowIndex = ma->getRowIndexByRowId(removedRowId, os);
        SAFE_POINT_OP(os, QList<qint64>());
    }
    const int lastDeletedRowIndex = ma->getRowIndexByRowId(removedRowIds.last(), os);
    SAFE_POINT_OP(os, QList<qint64>());
    if (lastDeletedRowIndex < maRows.size() - 1) { // if the last removed row was not in the bottom of the msa
        rowIdsAffectedByDeletion += maRows.mid(lastDeletedRowIndex + 1);
    }
    return rowIdsAffectedByDeletion;
}

}

void MultipleSequenceAlignmentObject::removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows, bool track) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", );

    QList<qint64> modifiedRowIds;
    const MultipleSequenceAlignment &ma = getMultipleAlignment();
    const QList<MultipleSequenceAlignmentRow> &maRows = ma->getMsaRows();
    SAFE_POINT(nRows > 0 && startRow >= 0 && startRow + nRows <= maRows.size() && startPos + nBases <= ma->getLength(), "Invalid parameters", );
    QList<MultipleSequenceAlignmentRow>::ConstIterator it = maRows.begin() + startRow;
    QList<MultipleSequenceAlignmentRow>::ConstIterator end = it + nRows;
    for (; it != end; it++) {
        modifiedRowIds << (*it)->getRowId();
    }

    U2OpStatus2Log os;
    MsaDbiUtils::removeRegion(entityRef, modifiedRowIds, startPos, nBases, os);
    SAFE_POINT_OP(os, );

    QList<qint64> removedRows;
    if (removeEmptyRows) {
        removedRows = MsaDbiUtils::removeEmptyRows(entityRef, modifiedRowIds, os);
        SAFE_POINT_OP(os, );
        if (!removedRows.isEmpty()) { // suppose that if at least one row in msa was removed then
            // all the rows below it were changed
            const QList<qint64> rowIdsAffectedByDeletion = getRowsAffectedByDeletion(ma, removedRows);
            foreach (qint64 removedRowId, removedRows) { // removed rows ain't need to be update
                modifiedRowIds.removeAll(removedRowId);
            }
            modifiedRowIds = mergeLists(modifiedRowIds, rowIdsAffectedByDeletion);
        }
    }

    if (track || !removedRows.isEmpty()) {
        MaModificationInfo mi;
        mi.modifiedRowIds = modifiedRowIds;
        updateCachedMultipleAlignment(mi, removedRows);
    }

    if (!removedRows.isEmpty()) {
        emit si_rowsRemoved(removedRows);
    }
}

void MultipleSequenceAlignmentObject::crop(const U2Region &window, const QSet<QString> &rowNames) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", );
    const MultipleSequenceAlignment &ma = getMultipleAlignment();

    QList<qint64> rowIds;
    for (int i = 0; i < ma->getNumRows(); ++i) {
        QString rowName = ma->getRow(i)->getName();
        if (rowNames.contains(rowName)) {
            qint64 rowId = ma->getRow(i)->getRowId();
            rowIds.append(rowId);
        }
    }

    U2OpStatus2Log os;
    MsaDbiUtils::crop(entityRef, rowIds, window.startPos, window.length, os);
    SAFE_POINT_OP(os, );

    updateCachedMultipleAlignment();
}

int MultipleSequenceAlignmentObject::shiftRegion(int startPos, int startRow, int nBases, int nRows, int shift) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", 0);
    SAFE_POINT(!isRegionEmpty(startPos, startRow, nBases, nRows), "Region is empty", 0);
    SAFE_POINT(0 <= startPos && 0 <= startRow && 0 < nBases && 0 < nRows, "Invalid parameters of selected region encountered", 0);
    U2OpStatusImpl os;

    int n = 0;
    if (shift > 0) {
        //if last symbol selected - do not add gaps at the end
        if (!(startPos + nBases == getLength())) {
            // if some trailing gaps are selected --> save them!
            if (startPos + nBases + shift > getLength()) {
                bool increaseAlignmentLen = true;
                for (int i = startRow; i < startRow + nRows; i++) {
                    int rowLen = getMsaRow(i)->getRowLengthWithoutTrailing();
                    if (rowLen >= startPos + nBases + shift) {
                        increaseAlignmentLen = false;
                        break;
                    }
                }
                if (increaseAlignmentLen) {
                    MaDbiUtils::updateMaLength(entityRef, startPos + nBases + shift, os);
                    SAFE_POINT_OP(os, 0);
                    updateCachedMultipleAlignment();
                }
            }
        }

        insertGap(U2Region(startRow, nRows), startPos, shift);
        n = shift;
    } else if (0 < startPos) {
        if (0 > startPos + shift) {
            shift = -startPos;
        }
        n = -deleteGap(os, U2Region(startRow, nRows), startPos + shift, -shift);
        SAFE_POINT_OP(os, 0);
    }
    return n;
}

void MultipleSequenceAlignmentObject::deleteColumnWithGaps(U2OpStatus &os, int requiredGapCount) {
    QList<qint64> colsForDelete = getColumnsWithGaps(requiredGapCount);
    if (getLength() == colsForDelete.count()) {
        return;
    }

    QList<U2Region> horizontalRegionsToDelete;
    foreach (qint64 columnNumber, colsForDelete) {
        bool columnMergedWithPrevious = false;
        if (!horizontalRegionsToDelete.isEmpty()) {
            U2Region &lastRegion = horizontalRegionsToDelete.last();
            if (lastRegion.startPos == columnNumber + 1) {
                --lastRegion.startPos;
                ++lastRegion.length;
                columnMergedWithPrevious = true;
            } else if (lastRegion.endPos() == columnNumber) {
                ++lastRegion.length;
                columnMergedWithPrevious = true;
            }
        }

        if (!columnMergedWithPrevious) {
            horizontalRegionsToDelete.append(U2Region(columnNumber, 1));
        }
    }

    QList<U2Region>::const_iterator columns = horizontalRegionsToDelete.constBegin();
    const QList<U2Region>::const_iterator end = horizontalRegionsToDelete.constEnd();

    for (int counter = 0; columns != end; ++columns, counter++) {
        removeRegion((*columns).startPos, 0, (*columns).length, getNumRows(), true, (end - 1 == columns));
        os.setProgress(100 * counter / horizontalRegionsToDelete.size());
    }
    updateCachedMultipleAlignment();
}

void MultipleSequenceAlignmentObject::deleteColumnWithGaps(int requiredGapCount) {
    U2OpStatusImpl os;
    deleteColumnWithGaps(os, requiredGapCount);
    SAFE_POINT_OP(os, );
}

QList<qint64> MultipleSequenceAlignmentObject::getColumnsWithGaps(int requiredGapCount) const {
    const MultipleSequenceAlignment &ma = getMultipleAlignment();
    const int length = ma->getLength();
    if (GAP_COLUMN_ONLY == requiredGapCount) {
        requiredGapCount = ma->getNumRows();
    }
    QList<qint64> colsForDelete;
    for (int i = 0; i < length; i++) { //columns
        int gapCount = 0;
        for (int j = 0; j < ma->getNumRows(); j++) { //sequences
            if (ma->isGap(j, i)) {
                gapCount++;
            }
        }

        if (gapCount >= requiredGapCount) {
            colsForDelete.prepend(i); //invert order
        }
    }
    return colsForDelete;
}

void MultipleSequenceAlignmentObject::updateRow(U2OpStatus &os, int rowIdx, const QString &name, const QByteArray &seqBytes, const U2MsaRowGapModel &gapModel) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", );

    const MultipleSequenceAlignment msa = getMultipleAlignment();
    SAFE_POINT(rowIdx >= 0 && rowIdx < msa->getNumRows(), "Invalid row index", );
    qint64 rowId = msa->getRow(rowIdx)->getRowId();

    MsaDbiUtils::updateRowContent(entityRef, rowId, seqBytes, gapModel, os);
    CHECK_OP(os, );

    MaDbiUtils::renameRow(entityRef, rowId, name, os);
    CHECK_OP(os, );
}

void MultipleSequenceAlignmentObject::replaceCharacter(int startPos, int rowIndex, char newChar) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", );
    const MultipleSequenceAlignment msa = getMultipleAlignment();
    SAFE_POINT(rowIndex >= 0 && startPos + 1 <= msa->getLength(), "Invalid parameters", );
    qint64 modifiedRowId = msa->getRow(rowIndex)->getRowId();

    U2OpStatus2Log os;
    if (newChar != U2Msa::GAP_CHAR) {
        MsaDbiUtils::replaceCharacterInRow(entityRef, modifiedRowId, startPos, newChar, os);
    } else {
        MsaDbiUtils::removeRegion(entityRef, QList<qint64>() << modifiedRowId, startPos, 1, os);
        MsaDbiUtils::insertGaps(entityRef, QList<qint64>() << modifiedRowId, startPos, 1, os);
    }
    SAFE_POINT_OP(os, );

    MaModificationInfo mi;
    mi.rowContentChanged = true;
    mi.rowListChanged = false;
    mi.alignmentLengthChanged = false;
    mi.modifiedRowIds << modifiedRowId;

    if (newChar != ' ' && !msa->getAlphabet()->contains(newChar)) {
        const DNAAlphabet *alp = U2AlphabetUtils::findBestAlphabet(QByteArray(1, newChar));
        const DNAAlphabet *newAlphabet = U2AlphabetUtils::deriveCommonAlphabet(alp, msa->getAlphabet());
        SAFE_POINT(NULL != newAlphabet, "Common alphabet is NULL", );

        if (newAlphabet->getId() != msa->getAlphabet()->getId()) {
            MaDbiUtils::updateMaAlphabet(entityRef, newAlphabet->getId(), os);
            mi.alphabetChanged = true;
            SAFE_POINT_OP(os, );
        }
    }

    updateCachedMultipleAlignment(mi);
}

void MultipleSequenceAlignmentObject::loadAlignment(U2OpStatus &os) {
    MultipleSequenceAlignmentExporter msaExporter;
    cachedMa = msaExporter.getAlignment(entityRef.dbiRef, entityRef.entityId, os);
}

void MultipleSequenceAlignmentObject::updateCachedRows(U2OpStatus &os, const QList<qint64> &rowIds) {
    MultipleSequenceAlignment cachedMsa = cachedMa.dynamicCast<MultipleSequenceAlignment>();

    MultipleSequenceAlignmentExporter msaExporter;
    QList<MsaRowReplacementData> rowsAndSeqs = msaExporter.getAlignmentRows(entityRef.dbiRef, entityRef.entityId, rowIds, os);
    SAFE_POINT_OP(os, );
    foreach (const MsaRowReplacementData &data, rowsAndSeqs) {
        const int rowIndex = cachedMsa->getRowIndexByRowId(data.row.rowId, os);
        SAFE_POINT_OP(os, );
        cachedMsa->setRowContent(rowIndex, data.sequence.seq);
        cachedMsa->setRowGapModel(rowIndex, data.row.gaps);
        cachedMsa->renameRow(rowIndex, data.sequence.getName());
    }
}

void MultipleSequenceAlignmentObject::updateDatabase(U2OpStatus &os, const MultipleAlignment &ma) {
    const MultipleSequenceAlignment msa = ma.dynamicCast<MultipleSequenceAlignment>();
    MsaDbiUtils::updateMsa(entityRef, msa, os);
}

void MultipleSequenceAlignmentObject::renameMaPrivate(U2OpStatus &os, const U2EntityRef &msaRef, const QString &newName) {
    MaDbiUtils::renameMa(msaRef, newName, os);
}

void MultipleSequenceAlignmentObject::removeRowPrivate(U2OpStatus &os, const U2EntityRef &msaRef, qint64 rowId) {
    MsaDbiUtils::removeRow(msaRef, rowId, os);
}

void MultipleSequenceAlignmentObject::renameRowPrivate(U2OpStatus &os, const U2EntityRef &msaRef, qint64 rowId, const QString &newName) {
    MaDbiUtils::renameRow(msaRef, rowId, newName, os);
}

void MultipleSequenceAlignmentObject::moveRowsPrivate(U2OpStatus &os, const U2EntityRef &msaRef, const QList<qint64> &rowsToMove, int delta) {
    MaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
}

void MultipleSequenceAlignmentObject::updateRowsOrderPrivate(U2OpStatus &os, const U2EntityRef &msaRef, const QList<qint64> &rowsOrder) {
    MaDbiUtils::updateRowsOrder(msaRef, rowsOrder, os);
}

qint64 MultipleSequenceAlignmentObject::getMaLengthPrivate(U2OpStatus &os, const U2EntityRef &msaRef) {
    return MaDbiUtils::getMaLength(msaRef, os);
}

U2AlphabetId MultipleSequenceAlignmentObject::getMaAlphabetPrivate(U2OpStatus &os, const U2EntityRef &msaRef) {
    return MaDbiUtils::getMaAlphabet(msaRef, os);
}

int MultipleSequenceAlignmentObject::getMaxWidthOfGapRegion(U2OpStatus &os, const U2Region &rows, int pos, int maxGaps) {
    const MultipleSequenceAlignment &ma = getMultipleAlignment();
    SAFE_POINT_EXT(U2Region(0, ma->getNumRows()).contains(rows) && 0 <= pos && 0 <= maxGaps && ma->getLength() > pos,
                   os.setError("Illegal parameters of the gap region"), 0);

    const int maxRemovedGaps = qBound(0, maxGaps, ma->getLength() - pos);
    // check if there is nothing to remove
    if (0 == maxRemovedGaps) {
        return 0;
    }

    int removingGapColumnCount = maxRemovedGaps;
    bool isRegionInRowTrailingGaps = true;
    // iterate through given rows to determine the width of the continuous gap region
    for (int rowCount = rows.startPos; rowCount < rows.endPos(); ++rowCount) {
        int gapCountInCurrentRow = 0;
        // iterate through current row bases to determine gap count
        while (gapCountInCurrentRow < maxRemovedGaps) {
            if (!ma->isGap(rowCount, pos + maxGaps - gapCountInCurrentRow - 1)) {
                break;
            }
            gapCountInCurrentRow++;
        }

        // determine if the given area intersects a row in the area of trailing gaps
        if (0 != gapCountInCurrentRow && isRegionInRowTrailingGaps) {
            int trailingPosition = pos + maxRemovedGaps - gapCountInCurrentRow;
            if (ma->getLength() != trailingPosition) {
                while (ma->getLength() > trailingPosition && isRegionInRowTrailingGaps) {
                    isRegionInRowTrailingGaps &= ma->isGap(rowCount, trailingPosition);
                    ++trailingPosition;
                }
            }
        } else if (isRegionInRowTrailingGaps) {
            isRegionInRowTrailingGaps = false;
        }

        if (0 == gapCountInCurrentRow) {
            // don't do anything if there is a row without gaps
            return 0;
        }
        removingGapColumnCount = qMin(removingGapColumnCount, gapCountInCurrentRow);
    }

    if (isRegionInRowTrailingGaps) {
        if (rows.length == getNumRows() && rows.startPos == 0) {
            return qMin(getLength() - pos, (qint64)maxGaps);
        } else {
            return 0;
        }
    }

    return removingGapColumnCount;
}

}   // namespace U2
