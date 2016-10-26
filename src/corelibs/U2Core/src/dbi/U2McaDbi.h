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

#ifndef _U2_MCA_DBI_H_
#define _U2_MCA_DBI_H_

#include <U2Core/U2Dbi.h>
#include <U2Core/U2Mca.h>

namespace U2 {

class DNAChromatogram;
class DNASequence;

/**
    An interface to obtain access to multiple chromatogram alignment
*/
class U2McaDbi : public U2ChildDbi {
protected:
    U2McaDbi(U2Dbi *rootDbi);

public:
    /** Reads Mca objects by id */
    virtual U2Msa getMcaObject(const U2DataId &mcaId, U2OpStatus &os) = 0;

    /** Returns the number of rows in the MCA */
    virtual qint64 getNumOfRows(const U2DataId &mcaId, U2OpStatus &os) = 0;

    /** Returns all MCA rows */
    virtual QList<U2McaRow> getRows(const U2DataId &mcaId, U2OpStatus &os) = 0;

    /** Return a row with the specified ID */
    virtual U2McaRow getRow(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) = 0;

    /** Returns the list of rows IDs in the database for the specified MCA (in increasing order) */
    virtual QList<qint64> getRowsOrder(const U2DataId &mcaId, U2OpStatus &os) = 0;

    /** Return the MCA alphabet */
    virtual U2AlphabetId getMcaAlphabet(const U2DataId &mcaId, U2OpStatus &os) = 0;

    /** Return the MCA length */
    virtual qint64 getMcaLength(const U2DataId &mcaId, U2OpStatus &os) = 0;

    /**
     * Creates a new empty MCA object
     * Requires: U2DbiFeature_WriteMca feature support
     */
    virtual U2DataId createMcaObject(const QString &folder, const QString &name, const U2AlphabetId &alphabet, U2OpStatus &os) = 0;
    virtual U2DataId createMcaObject(const QString &folder, const QString &name, const U2AlphabetId &alphabet, int length, U2OpStatus &os) = 0;

    /**
     * Updates the multiple alignment name
     * Requires: U2DbiFeature_WriteMca feature support
     */
    virtual void updateMcaName(const U2DataId &mcaId, const QString &name, U2OpStatus &os) = 0;

    /**
     * Updates the multiple alignment alphabet
     * Requires: U2DbiFeature_WriteMca feature support
     */
    virtual void updateMcaAlphabet(const U2DataId &mcaId, const U2AlphabetId &alphabet, U2OpStatus &os) = 0;

    /**
     * Adds rows to the MCA
     * Requires: U2DbiFeature_WriteMca feature support
     */
    virtual void addRows(const U2DataId &mcaId, QList<U2McaRow> &rows, U2OpStatus &os) = 0;

    /**
     * Adds a row to the MCA
     * If 'posInMca' equals to '-1' the row is appended to the end of the MCA,
     * otherwise it is inserted to the specified position and all positions are updated.
     * Requires: U2DbiFeature_WriteMca feature support
     */
    virtual void addRow(const U2DataId &mcaId, qint64 posInMca, U2McaRow &row, U2OpStatus &os) = 0;

    /**
     * Removes rows from MCA
     * Automatically removes affected sequences that are not anymore located in some folder nor Mca object
     * Requires: U2DbiFeature_WriteMca feature support
     */
    virtual void removeRows(const U2DataId &mcaId, const QList<qint64> &rowIds, U2OpStatus &os) = 0;

    /**
     * Removes the row gaps, the row.
     * Also removes the record that the msa is a parent of the row sequence
     * and attempts to remove the sequence after it.
     * Requires: U2DbiFeature_WriteMca feature support
     */
    virtual void removeRow(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) = 0;

    /** Updates name of the sequence of the row. */
    virtual void updateRowName(const U2DataId &mcaId, qint64 rowId, const QString &newName, U2OpStatus &os) = 0;

    /**
     * Updates a row with the specified ID and its sequence.
     * Requires: U2DbiFeature_WriteMca feature support
     */
    virtual void updateRowContent(const U2DataId &mcaId,
                                  qint64 rowId,
                                  const DNAChromatogram &chromatogram,
                                  const DNASequence &predictedSequence,
                                  const U2MsaRowGapModel &predictedSequenceGapModel,
                                  const DNASequence &editedSequence,
                                  const U2MsaRowGapModel &editedSequenceGapModel,
                                  U2OpStatus &os) = 0;

    /**
     * Updates a chromatogram with the specified ID in a row.
     * Be careful, keep the row consistency (core length of all parts should be the same)!
     * Requires: U2DbiFeature_WriteMca feature support
     */
    virtual void updateRowChromatogram(const U2DataId &mcaId,
                                  qint64 rowId,
                                  qint64 sequenceId,
                                  const QByteArray &sequenceData,
                                  const U2MsaRowGapModel &gapModel,
                                  U2OpStatus &os) = 0;

    /**
     * Updates a sequence with the specified ID in a row.
     * Be careful, keep the row consistency (core length of all parts should be the same)!
     * Requires: U2DbiFeature_WriteMca feature support
     */
    virtual void updateRowSequence(const U2DataId &mcaId,
                                  qint64 rowId,
                                  qint64 sequenceId,
                                  const QByteArray &sequenceData,
                                  const U2MsaRowGapModel &gapModel,
                                  U2OpStatus &os) = 0;

    /**
     * Removes all previous values and sets a new gap model for gap model owner in a row in a MCA
     * Requires: U2DbiFeature_WriteMca feature support
     */
    virtual void updateGapModel(const U2DataId &mcaId, qint64 msaRowId, qint64 gapModelOwner, const U2MsaRowGapModel &gapModel, U2OpStatus &os) = 0;

    /**
     * Updates positions of the rows in the database according to the order in the list
     * Be careful, all IDs must exactly match IDs of the MCA!
     * Requires: U2DbiFeature_WriteMca feature support
     */
    virtual void setNewRowsOrder(const U2DataId &mcaId, const QList<qint64> &rowIds, U2OpStatus &os) = 0;

    /** Updates a part of the Mca object info - the length */
    virtual void updateMcaLength(const U2DataId &mcaId, qint64 length, U2OpStatus &os) = 0;
};

}   // namespace U2

#endif // _U2_MCA_DBI_H_
