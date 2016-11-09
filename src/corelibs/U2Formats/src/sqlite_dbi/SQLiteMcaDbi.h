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

#ifndef _U2_SQLITE_MCA_DBI_H_
#define _U2_SQLITE_MCA_DBI_H_

#include "SQLiteDbi.h"
#include "SQLiteObjectDbi.h"

#include <U2Core/U2AbstractDbi.h>

namespace U2 {

class SQLiteMcaDbi : public U2McaDbi, public SQLiteChildDBICommon {
public:
    SQLiteMcaDbi(SQLiteDbi *dbi);

    /** Creates all required tables */
    void initSqlSchema(U2OpStatus &os);

    /** Reads Mca objects by id */
    U2Msa getMcaObject(const U2DataId &id, U2OpStatus &os);

    /** Returns the number of rows of the MCA (value cached in Mca table) */
    qint64 getNumOfRows(const U2DataId &mcaId, U2OpStatus &os);

    /** Returns all rows of a MCA with the specified IDs */
    QList<U2McaRow> getRows(const U2DataId &mcaId, U2OpStatus &os);

    /** Returns a row with the specified ID */
    U2McaRow getRow(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os);

    /** Returns the list of rows IDs in the database for the specified MCA (in increasing order) */
    QList<qint64> getRowsOrder(const U2DataId &mcaId, U2OpStatus &os);

    /** Return the MCA alphabet */
    U2AlphabetId getMcaAlphabet(const U2DataId &mcaId, U2OpStatus &os);

    /** Returns length stored in Mca table */
    qint64 getMcaLength(const U2DataId &mcaId, U2OpStatus &os);

    /**
     * Creates a new empty multiple alignment in the database.
     * The folder must exist in the database.
     * The number of rows and the length of the alignment are set to 0.
     * Returns the assigned id.
     */
    U2DataId createMcaObject(const QString &folder, const QString &name, const U2AlphabetId &alphabet, U2OpStatus &os);
    U2DataId createMcaObject(const QString &folder, const QString &name,const U2AlphabetId &alphabet, int length, U2OpStatus &os);

    /**
     * Updates the multiple alignment name.
     * Increments the alignment version.
     * Tracks modifications, if required.
     */
    void updateMcaName(const U2DataId &mcaId, const QString &name, U2OpStatus &os);

    /**
     * Updates the multiple alignment alphabet.
     * Increments the alignment version.
     * Tracks modifications, if required.
     */
    void updateMcaAlphabet(const U2DataId &mcaId, const U2AlphabetId &alphabet, U2OpStatus &os);

    /**
     * Creates rows (and gap models for them) in the database.
     * Enlarges msa length, if 'length' of any of the 'rows' is greater than current msa length.
     * Recalculates 'length' of the 'rows'.
     * The rows are appended to the end of the MCA.
     * Assigns MCA as a parent for all the sequences.
     * If a row ID equals "-1", sets a valid ID to the passed U2MaRow instances.
     * Updates the number of rows of the MCA.
     * Updates the alignment length.
     * Increments the alignment version.
     * Tracks modifications, if required.
     */
    void addRows(const U2DataId &mcaId, QList<U2McaRow> &rows, U2OpStatus &os);

    /**
     * Creates a new row and gap model records in the database.
     * Enlarges msa length, if 'row.length' is greater than current msa length.
     * Ignores 'row.rowId'.
     * Recalculates 'row.length' and and 'row.rowId'. Sets valid to the passed U2McaRow instance.
     * If 'posInMca' equals to '-1' the row is appended to the end of the MCA,
     * otherwise it is inserted to the specified position and all positions are updated.
     * Assigns MCA as a parent for the sequence.
     * Updates the number of rows of the MCA.
     * Updates the alignment length.
     * Increments the alignment version.
     * Tracks modifications, if required.
     */
    void addRow(const U2DataId &mcaId, qint64 posInMca, U2McaRow &row, U2OpStatus &os);

    /**
     * Removes rows for the specified alignment and with the specified ids
     * from the database.
     * Updates the number of rows of the MCA.
     * Updates all rows positions for the alignment.
     * Updates the alignment length.
     * Tracks modifications, if required.
     */
    void removeRows(const U2DataId &mcaId, const QList<qint64> &rowIds, U2OpStatus &os);

    /**
     * Removes a row with the specified ID for the specified alignment.
     * Updates the number of rows of the MCA.
     * Updates all rows positions for the alignment.
     * Updates the alignment length.
     * Increments the alignment version.
     * Tracks modifications, if required.
     */
    void removeRow(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os);

    /**
     * Removes all rows from the alignment with the specified id.
     * Does not update the alignment length.
     * Does not increment the alignment version.
     * USE this method only on MCA deletion
     */
    void deleteRowsData(const U2DataId &mcaId, U2OpStatus &os);

    /**
     * Updates name of the sequence of the row.
     * Increments the alignment version.
     * Tracks modifications, if required.
     */
    void updateRowName(const U2DataId &mcaId, qint64 rowId, const QString &newName, U2OpStatus &os);

    /**
     * Updates a row with the specified ID and its chromatogram and sequences.
     * The sequences should be without gaps
     * Increments the alignment version.
     */
    void updateRowContent(const U2DataId &mcaId,
                          qint64 rowId,
                          const DNAChromatogram &chromatogram,
                          const DNASequence &predictedSequence,
                          const U2MsaRowGapModel &predictedSequenceGapModel,
                          const DNASequence &editedSequence,
                          const U2MsaRowGapModel &editedSequenceGapModel,
                          U2OpStatus &os);

    void updateRowContent(const U2DataId &mcaId,
                          qint64 rowId,
                          const McaRowMemoryData &rowMemoryData,
                          U2OpStatus &os);

    /**
     * Updates a chromatogram with the specified ID in a row.
     * Be careful, keep the row consistency (core length of all parts should be the same)!
     * Increments the alignment version.
     */
    void updateRowChromatogram(const U2DataId &mcaId,
                               qint64 rowId,
                               const DNAChromatogram &chromatogram,
                               U2OpStatus &os);

    /**
     * Updates a sequence with the specified ID in a row.
     * Sequence must be without gaps
     * Be careful, keep the row consistency (core length of all parts should be the same)!
     * Increments the alignment version.
     */
    void updateRowSequence(const U2DataId &mcaId,
                           qint64 rowId,
                           qint64 sequenceId,
                           const QByteArray &sequenceData,
                           const U2MsaRowGapModel &gapModel,
                           U2OpStatus &os);

    /**
     * Removes all previous values and sets a new gap model for a row in a MCA.
     * Updates the alignment length.
     * Increments the alignment version.
     */
    void updateGapModel(const U2DataId &mcaId, qint64 msaRowId, qint64 gapModelOwner, const U2MsaRowGapModel &gapModel, U2OpStatus &os);


    /** Updates a part of the Mca object info - the length */
    void updateMcaLength(const U2DataId &mcaId, qint64 length, U2OpStatus &os);

    /**
     * Updates positions of the rows in the database according to the order in the list
     * Be careful, all IDs must exactly match IDs of the MCA!
     * Increments the alignment version.
     */
    void setNewRowsOrder(const U2DataId &mcaId, const QList<qint64> &rowIds, U2OpStatus &os);

    /** Undo the operation for the MCA. */
    void undo(const U2DataId &mcaId, qint64 modType, const QByteArray &modDetails, U2OpStatus &os);

    /** Redo the operation for the MCA. */
    void redo(const U2DataId &mcaId, qint64 modType, const QByteArray &modDetails, U2OpStatus &os);

private:
    /**
     * Creates new records in McaRow and McaRowGap tables for the added row, and
     * sets the parent of the sequence object to the MCA object.
     * Sets the assigned ID to the passed U2MaRow instance.
     */
    void addMcaRowAndGaps(const U2DataId &mcaId, qint64 posInMca, U2McaRow &row, U2OpStatus &os);

    /**
     * Adds a new MCA row into database.
     * To add a gap for the row, use the "createMcaRowGap" method.
     */
    void createMcaRow(const U2DataId &mcaId, qint64 posInMca, U2McaRow &mcaRow, U2OpStatus &os);

    /** Adds a new gap for a MCA row into database. */
    void createMcaRowGap(const U2DataId &mcaId, qint64 mcaRowId, const U2DataId &relatedObjectId, const U2MsaGap &mcaGap, U2OpStatus &os);

    /** Removes records from McaRow and McaRowGap tables for the row. */
    void removeMcaRowAndGaps(const U2DataId &mcaId, qint64 rowId, bool removeChilds, U2OpStatus &os);

    /** Removes all records about the row gaps from the database. */
    void removeRecordsFromMcaRowGap(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os);

    /** Removes a record about the row from the database. */
    void removeRecordFromMcaRow(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os);

    /** Updates "numOfRows" in the "Mca" table */
    void updateNumOfRows(const U2DataId &mcaId, qint64 numOfRows, U2OpStatus &os);

    /**
     * Re-calculates positions of the MCA rows to go from 0 to "numOfRows - 1",
     * the original position must be in increasing order.
     */
    void recalculateRowsPositions(const U2DataId &mcaId, U2OpStatus &os);

    /** Calculates length of the row (characters + gaps), does NOT take into account trailing gaps. */
    qint64 calculateRowLength(qint64 seqLength, const QList<U2MsaGap> &gaps);

    /** Gets length of the sequence in the row (without gaps) */
    qint64 getRowSequenceLength(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os);

    /** Updates 'length' field in McaRow for specified */
    void updateRowLength(const U2DataId &mcaId, qint64 rowId, qint64 newLength, U2OpStatus &os);

    /** Gets a child ID for the row */
    U2DataId getChromatogramIdByRowId(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os);
    U2DataId getPredictedSequenceIdByRowId(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os);
    U2DataId getEditedSequenceIdByRowId(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os);

    /** Gets position of the row in the msa */
    qint64 getPosInMca(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os);

    qint64 getMaximumRowId(const U2DataId &mcaId, U2OpStatus &os);

    ///////////////////////////////////////////////////////////
    // Core methods
    void updateGapModelCore(const U2DataId &mcaId, qint64 mcaRowId, const U2DataId &relatedObjectId, const U2MsaRowGapModel &gapModel, U2OpStatus &os);
    void addRowSubcore(const U2DataId &mcaId, qint64 numOfRows, const QList<qint64> &rowsOrder, U2OpStatus &os);
    void addRowCore(const U2DataId &mcaId, qint64 posInMca, U2McaRow &row, U2OpStatus &os);
    void addRowsCore(const U2DataId &mcaId, const QList<qint64> &posInMca, QList<U2McaRow> &rows, U2OpStatus &os);
    void removeRowSubcore(const U2DataId &mcaId, qint64 numOfRows, U2OpStatus &os);
    void removeRowCore(const U2DataId &mcaId, qint64 rowId, bool removeChilds, U2OpStatus &os);
    void removeRowsCore(const U2DataId &mcaId, const QList<qint64> &rowIds, bool removeChilds, U2OpStatus &os);
    void setNewRowsOrderCore(const U2DataId &mcaId, const QList<qint64> rowIds, U2OpStatus &os);
    void updateRowInfoCore(const U2DataId &mcaId, const U2McaRow &row, U2OpStatus &os);
    void updateMcaLengthCore(const U2DataId &mcaId, qint64 length, U2OpStatus &os);
    void updateMcaAlphabetCore(const U2DataId &mcaId, const U2AlphabetId &alphabetId, U2OpStatus &os);

    ///////////////////////////////////////////////////////////
    // Undo methods
    void undoUpdateMcaAlphabet(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void undoAddRows(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void undoAddRow(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void undoRemoveRows(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void undoRemoveRow(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void undoUpdateGapModel(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void undoSetNewRowsOrder(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void undoUpdateRowInfo(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void undoMcaLengthChange(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);

    ///////////////////////////////////////////////////////////
    // Redo methods
    void redoUpdateMcaAlphabet(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void redoAddRows(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void redoAddRow(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void redoRemoveRows(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void redoRemoveRow(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void redoUpdateGapModel(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void redoSetNewRowsOrder(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void redoUpdateRowInfo(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os);
    void redoMcaLengthChange(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus  &os);

    ///////////////////////////////////////////////////////////
    // Methods included into a multi-action
    void updateRowInfo(ModificationAction &updateAction, const U2DataId &mcaId, const U2McaRow &row, U2OpStatus &os);
    void updateGapModel(ModificationAction &updateAction, const U2DataId &mcaId, qint64 mcaRowId, const U2DataId &relatedObjectId, const U2MsaRowGapModel &gapModel, U2OpStatus &os);
    void updateMcaLength(ModificationAction &updateAction, const U2DataId &mcaId, qint64 length, U2OpStatus &os);
};

}   // namespace U2

#endif // _U2_SQLITE_MCA_DBI_H_
