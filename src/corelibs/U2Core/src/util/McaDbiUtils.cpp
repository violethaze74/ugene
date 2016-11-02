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
#include <U2Core/DatatypeSerializeUtils.h>
#include <U2Core/DbiConnection.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNAChromatogram.h>
#include <U2Core/DNASequence.h>
#include <U2Core/MultipleChromatogramAlignment.h>
#include <U2Core/RawDataUdrSchema.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2McaDbi.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2SequenceUtils.h>

#include "McaDbiUtils.h"

namespace U2 {

void McaDbiUtils::renameMca(U2OpStatus &os, const U2EntityRef &mcaRef, const QString &newName) {
    CHECK_EXT(!newName.isEmpty(), os.setError(tr("Can't rename an alignment to an empty name")), );

    DbiConnection connection(mcaRef.dbiRef, os);
    CHECK_OP(os, );

    U2McaDbi *mcaDbi = connection.dbi->getMcaDbi();
    SAFE_POINT_EXT(NULL != mcaDbi, os.setError("NULL Mca Dbi"), );

    mcaDbi->updateMcaName(mcaRef.entityId, newName, os);
}

void McaDbiUtils::addRow(U2OpStatus &os, const U2EntityRef &mcaRef, qint64 posInMca, U2McaRow &row) {
    SAFE_POINT_EXT(row.hasValidChildObjectIds(), os.setError("Invalid child objects references"), );

    DbiConnection connection(mcaRef.dbiRef, os);
    CHECK_OP(os, );

    U2McaDbi *mcaDbi = connection.dbi->getMcaDbi();
    SAFE_POINT_EXT(NULL != mcaDbi, os.setError("NULL Mca Dbi"), );

    mcaDbi->addRow(mcaRef.entityId, posInMca, row, os);
    CHECK_OP(os, );
}

void McaDbiUtils::removeRow(U2OpStatus &os, const U2EntityRef &mcaRef, qint64 rowId) {
    DbiConnection connection(mcaRef.dbiRef, os);
    CHECK_OP(os, );

    U2McaDbi *mcaDbi = connection.dbi->getMcaDbi();
    SAFE_POINT_EXT(NULL != mcaDbi, os.setError("NULL Mca Dbi"), );

    mcaDbi->removeRow(mcaRef.entityId, rowId, os);
}

void McaDbiUtils::renameRow(U2OpStatus &os, const U2EntityRef &mcaRef, qint64 rowId, const QString &newName) {
    CHECK_EXT(!newName.isEmpty(), os.setError(tr("Can't rename an alignment to an empty name")), );

    DbiConnection connection(mcaRef.dbiRef, os);
    CHECK_OP(os, );

    U2McaDbi *mcaDbi = connection.dbi->getMcaDbi();
    SAFE_POINT_EXT(NULL != mcaDbi, os.setError("NULL Mca Dbi"), );

    mcaDbi->updateRowName(mcaRef.entityId, rowId, newName, os);
}

void McaDbiUtils::moveRows(U2OpStatus &os, const U2EntityRef &mcaRef, const QList<qint64> &rowsToMove, int delta) {
    DbiConnection connection(mcaRef.dbiRef, false, os);
    CHECK_OP(os, );

    U2McaDbi *mcaDbi = connection.dbi->getMcaDbi();
    SAFE_POINT_EXT(NULL != mcaDbi, os.setError("NULL Mca Dbi"), );

    if (delta == 0 || rowsToMove.isEmpty()) {
        return;
    }

    QList<U2McaRow> rows = mcaDbi->getRows(mcaRef.entityId, os);
    CHECK_OP(os, );

    QList<qint64> rowIds;
    for (int i = 0; i < rows.length(); ++i) {
        rowIds << rows[i].rowId;
    }

    QList<QPair<int, int> > from_To;

    if (delta < 0) {
        int rowIndex = rowIds.indexOf(rowsToMove.first());
        CHECK_EXT(-1 != rowIndex, os.setError("Invalid row list"), );

        int moveToIndex = rowIndex + delta >= 0 ? rowIndex + delta : 0;
        from_To.append(QPair<int, int>(rowIndex, moveToIndex));
        for (int i = 1; i < rowsToMove.length(); ++i) {
            rowIndex = rowIds.indexOf(rowsToMove[i]);
            CHECK_EXT(-1 != rowIndex, os.setError("Invalid row list"), );
            CHECK_EXT(rowIndex > from_To[i - 1].first, os.setError("List of rows to move is not ordered"), );
            moveToIndex = rowIndex + delta > from_To[i - 1].second ? rowIndex + delta : from_To[i - 1].second + 1;
            from_To.append(QPair<int, int>(rowIndex, moveToIndex));
        }
    } else {
        int rowIndex = rowIds.indexOf(rowsToMove.last());
        CHECK_EXT(-1 != rowIndex, os.setError("Invalid row list"), );
        int moveToIndex = rowIndex + delta < rowIds.length() ? rowIndex + delta : rowIds.length() - 1;
        from_To.append(QPair<int, int>(rowIndex, moveToIndex));
        for (int i = 1; i < rowsToMove.length(); ++i) {
            rowIndex = rowIds.indexOf(rowsToMove[rowsToMove.length() - i - 1]);
            CHECK_EXT(-1 != rowIndex, os.setError("Invalid row list"), );
            CHECK_EXT(rowIndex < from_To[i - 1].first, os.setError("List of rows to move is not ordered"), );
            moveToIndex = rowIndex + delta < from_To[i - 1].second ? rowIndex + delta : from_To[i - 1].second - 1;
            from_To.append(QPair<int, int>(rowIndex, moveToIndex));
        }
    }

    QPair<int, int> coords;
    foreach (coords, from_To) {
        rowIds.move(coords.first, coords.second);
    }

    mcaDbi->setNewRowsOrder(mcaRef.entityId, rowIds, os);
    CHECK_OP(os, );
}

void McaDbiUtils::updateRowsOrder(U2OpStatus &os, const U2EntityRef &mcaRef, const QList<qint64> &rowsOrder) {
    DbiConnection connection(mcaRef.dbiRef, os);
    CHECK_OP(os, );

    U2McaDbi *mcaDbi = connection.dbi->getMcaDbi();
    SAFE_POINT_EXT(NULL != mcaDbi, os.setError("NULL Mca Dbi"), );

    // Update the data
    mcaDbi->setNewRowsOrder(mcaRef.entityId, rowsOrder, os);
}

qint64 McaDbiUtils::getMcaLength(U2OpStatus &os, const U2EntityRef &mcaRef) {
    DbiConnection connection(mcaRef.dbiRef, os);
    CHECK_OP(os, -1);

    U2McaDbi *mcaDbi = connection.dbi->getMcaDbi();
    SAFE_POINT_EXT(NULL != mcaDbi, os.setError("NULL Mca Dbi"), -1);

    const qint64 length = mcaDbi->getMcaLength(mcaRef.entityId, os);
    CHECK_OP(os, -1);

    return length;
}

U2AlphabetId McaDbiUtils::getMcaAlphabet(U2OpStatus &os, const U2EntityRef &mcaRef) {
    DbiConnection connection(mcaRef.dbiRef, os);
    CHECK_OP(os, U2AlphabetId());

    U2McaDbi *mcaDbi = connection.dbi->getMcaDbi();
    SAFE_POINT_EXT(NULL != mcaDbi, os.setError("NULL Mca Dbi"), U2AlphabetId());

    const U2AlphabetId alphabet = mcaDbi->getMcaAlphabet(mcaRef.entityId, os);
    CHECK_OP(os, U2AlphabetId());

    return alphabet;
}

void McaDbiUtils::updateMca(U2OpStatus &os, const U2EntityRef &mcaRef, const MultipleChromatogramAlignment &mca) {
    // Move to the MCAImporter
    // TODO: check, if a transaction or an operation block should be started
    DbiConnection connection(mcaRef.dbiRef, os);
    CHECK_OP(os, );

    U2AttributeDbi *attributeDbi = connection.dbi->getAttributeDbi();
    SAFE_POINT_EXT(NULL != attributeDbi, os.setError("NULL Attribute Dbi"), );

    U2McaDbi *mcaDbi = connection.dbi->getMcaDbi();
    SAFE_POINT_EXT(NULL != mcaDbi, os.setError("NULL Msa Dbi"), );

    U2SequenceDbi *sequenceDbi = connection.dbi->getSequenceDbi();
    SAFE_POINT_EXT(NULL != sequenceDbi, os.setError("NULL Sequence Dbi"), );

    const DNAAlphabet *alphabet = mca->getAlphabet();
    SAFE_POINT_EXT(NULL != alphabet, os.setError("The alignment alphabet is NULL"), );

    //// UPDATE MCA
    U2Msa dbMca;
    dbMca.id = mcaRef.entityId;
    dbMca.visualName = mca->getName();
    dbMca.alphabet.id = alphabet->getId();
    dbMca.length = mca->getLength();

    mcaDbi->updateMcaName(mcaRef.entityId, mca->getName(), os);
    CHECK_OP(os, );

    mcaDbi->updateMcaAlphabet(mcaRef.entityId, alphabet->getId(), os);
    CHECK_OP(os, );

    mcaDbi->updateMcaLength(mcaRef.entityId, mca->getLength(), os);
    CHECK_OP(os, );

    //// UPDATE ROWS AND SEQUENCES
    // Get rows that are currently stored in the database
    const QList<U2McaRow> currentRows = mcaDbi->getRows(mcaRef.entityId, os);
    CHECK_OP(os, );

    QList<qint64> currentRowIds;
    QList<qint64> newRowsIds = mca->getRowsIds();
    QList<qint64> eliminatedRows;
    // TODO: get the mca folder and create child objects there
    const QString dbFolder = U2ObjectDbi::ROOT_FOLDER;

    foreach (const U2McaRow &currentRow, currentRows) {
        currentRowIds << currentRow.rowId;

        // Update data for rows with the same row and child objects IDs
        if (newRowsIds.contains(currentRow.rowId)) {
            // Update sequence and row info
            const U2McaRow newRow = mca->getMcaRowByRowId(currentRow.rowId, os)->getRowDbInfo();
            CHECK_OP(os, );

            if (newRow.chromatogramId != currentRow.chromatogramId ||
                newRow.predictedSequenceId != currentRow.predictedSequenceId ||
                newRow.sequenceId != currentRow.sequenceId) {
                // Kill the row from the current alignment, it is incorrect. New row with this ID will be created later.
                // TODO: replace with specific utils
                McaDbiUtils::removeRow(os, mcaRef, currentRow.rowId);
                CHECK_OP(os, );

                currentRowIds.removeOne(currentRow.rowId);
                continue;
            }

            const McaRowMemoryData rowMemoryData = mca->getMcaRowByRowId(newRow.rowId, os)->getRowMemoryData();
            CHECK_OP(os, );

            mcaDbi->updateRowName(mcaRef.entityId, newRow.rowId, rowMemoryData.editedSequence.getName(), os);
            CHECK_OP(os, );

            mcaDbi->updateRowContent(mcaRef.entityId, newRow.rowId, rowMemoryData, os);
            CHECK_OP(os, );
        } else {
            // Remove rows that are no more present in the alignment
            eliminatedRows.append(currentRow.rowId);
        }
    }

    mcaDbi->removeRows(mcaRef.entityId, eliminatedRows, os);
    CHECK_OP(os, );

    // Add rows that are stored in memory, but are not present in the database,
    // remember the rows order
    QList<qint64> rowsOrder;
    for (int i = 0, n = mca->getNumRows(); i < n; ++i) {
        const MultipleChromatogramAlignmentRow mcaRow = mca->getMcaRow(i);
        U2McaRow dbRow = mcaRow->getRowDbInfo();

        if (!dbRow.hasValidChildObjectIds() || !currentRowIds.contains(dbRow.rowId)) {
            // Import the child objects
            const U2EntityRef chromatogramRef = ChromatogramUtils::import(os, connection.dbi->getDbiRef(), dbFolder, mcaRow->getChromatogram());
            CHECK_OP(os, );

            const U2EntityRef predictedSequenceRef = U2SequenceUtils::import(os, connection.dbi->getDbiRef(), dbFolder, mcaRow->getPredictedSequence(), dbMca.alphabet.id);
            CHECK_OP(os, );

            const U2EntityRef editedSequenceRef = U2SequenceUtils::import(os, connection.dbi->getDbiRef(), dbFolder, mcaRow->getEditedSequence(), dbMca.alphabet.id);
            CHECK_OP(os, );

            // Create the row
            dbRow.rowId = U2MsaRow::INVALID_ROW_ID; // set the row ID automatically
            dbRow.chromatogramId = chromatogramRef.entityId;
            dbRow.predictedSequenceId = predictedSequenceRef.entityId;
            dbRow.sequenceId = editedSequenceRef.entityId;
            dbRow.gstart = mcaRow->getWorkingAreaRegion().startPos;
            dbRow.gend = mcaRow->getWorkingAreaRegion().endPos();
            dbRow.predictedSequenceGaps = mcaRow->getPredictedSequenceGapModel();
            dbRow.gaps = mcaRow->getEditedSequenceGapModel();

            McaDbiUtils::addRow(os, mcaRef, -1, dbRow);
            CHECK_OP(os, );
        }
        rowsOrder << dbRow.rowId;
    }

    //// UPDATE ROWS POSITIONS
    mcaDbi->setNewRowsOrder(mcaRef.entityId, rowsOrder, os);

    //// UPDATE ALIGNMENT ATTRIBUTES
    QVariantMap info = mca->getInfo();

    foreach (const QString &key, info.keys()) {
        QString value = info.value(key).toString();
        U2StringAttribute attribute(mcaRef.entityId, key, value);

        attributeDbi->createStringAttribute(attribute, os);
        CHECK_OP(os, );
    }
}

}   // namespace U2
