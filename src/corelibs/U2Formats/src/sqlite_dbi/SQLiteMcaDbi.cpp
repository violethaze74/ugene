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

#include <U2Core/DatatypeSerializeUtils.h>
#include <U2Core/DNASequence.h>
#include <U2Core/McaRowInnerData.h>
#include <U2Core/RawDataUdrSchema.h>
#include <U2Core/U2DbiPackUtils.h>
#include <U2Core/U2SafePoints.h>

#include "SQLiteMcaDbi.h"
#include "SQLiteSequenceDbi.h"

namespace U2 {

SQLiteMcaDbi::SQLiteMcaDbi(SQLiteDbi *dbi)
    : U2McaDbi(dbi),
      SQLiteChildDBICommon(dbi)
{

}

void SQLiteMcaDbi::initSqlSchema(U2OpStatus &os) {
    CHECK_OP(os, );

    // MCA object
    SQLiteQuery("CREATE TABLE Mca (object INTEGER PRIMARY KEY, length INTEGER NOT NULL, "
                "alphabet TEXT NOT NULL, numOfRows INTEGER NOT NULL, "
                "FOREIGN KEY(object) REFERENCES Object(id) ON DELETE CASCADE)", db, os).execute();

     // MCA object row
     //   mca      - mca object id
     //   rowId    - id of the row in the mca
     //   chromatogram - chromatogram boject id
     //   predictedSequence - predicted sequence object id
     //   editedSequence - edited sequence object id
     //   pos      - positional number of a row in the mca (initially, equals 'id', but can be changed, e.g. in GUI by moving rows)
     //   workingAreaStart   - offset of the first element in the row core
     //   workingAreaEnd     - offset of the last element in the row core (non-inclusive)
     //   length   - sequence and gaps length (trailing gap are not taken into account)
    SQLiteQuery("CREATE TABLE McaRow (mca INTEGER NOT NULL, rowId INTEGER NOT NULL, chromatogram INTEGER NOT NULL,"
                " predictedSequence INTEGER NOT NULL, editedSequence INTEGER NOT NULL,"
                " pos INTEGER NOT NULL, workingAreaStart INTEGER NOT NULL, workingAreaEnd INTEGER NOT NULL, length INTEGER NOT NULL,"
                " PRIMARY KEY(mca, rowId),"
                " FOREIGN KEY(mca) REFERENCES Mca(object) ON DELETE CASCADE, "
                " FOREIGN KEY(chromatogram) REFERENCES Object(id) ON DELETE CASCADE, "
                " FOREIGN KEY(predictedSequence) REFERENCES Sequence(object) ON DELETE CASCADE, "
                " FOREIGN KEY(editedSequence) REFERENCES Sequence(object) ON DELETE CASCADE)", db, os).execute();

    // TODO: review indecies
    SQLiteQuery("CREATE INDEX McaRow_mca_rowId ON McaRow(mca, rowId)", db, os).execute();
    SQLiteQuery("CREATE INDEX McaRow_length ON McaRow(length)", db, os).execute();
    SQLiteQuery("CREATE INDEX McaRow_chromatogram_predictedSequence_editedSequence ON McaRow(chromatogram, predictedSequence, editedSequence)", db, os).execute();
    SQLiteQuery("CREATE INDEX McaRow_chromatogram ON McaRow(chromatogram)", db, os).execute();
    SQLiteQuery("CREATE INDEX McaRow_predictedSequence ON McaRow(predictedSequence)", db, os).execute();
    SQLiteQuery("CREATE INDEX McaRow_editedSequence ON McaRow(editedSequence)", db, os).execute();

     // Gap info for a MCA row:
     //   mca       - mca object id
     //   rowId     - id of the row in the mca
     //   relatedObjectId - id of the object in the mca row this gap has a relation to
     //   gapStart  - start of the gap, the coordinate is relative to the predictedSequence start
     //   gapEnd    - end of the gap, the coordinate is relative to the predictedSequence start (non-inclusive)
    SQLiteQuery("CREATE TABLE McaRowGap (mca INTEGER NOT NULL, rowId INTEGER NOT NULL, relatedObjectId INTEGER NOT NULL, "
                "gapStart INTEGER NOT NULL, gapEnd INTEGER NOT NULL, "
                "FOREIGN KEY(mca, rowId) REFERENCES McaRow(mca, rowId) ON DELETE CASCADE, "
                "FOREIGN KEY(relatedObjectId) REFERENCES Object(id) ON DELETE CASCADE)",
                db, os).execute();

    // TODO: review indecies
    SQLiteQuery("CREATE INDEX McaRowGap_mca_rowId_relatedObjectId ON McaRowGap(mca, rowId, relatedObjectId)", db, os).execute();
    SQLiteQuery("CREATE INDEX McaRowGap_mca_rowId ON McaRowGap(mca, rowId)", db, os).execute();
}

U2Mca SQLiteMcaDbi::getMcaObject(const U2DataId &mcaId, U2OpStatus &os) {
    U2Mca res;
    dbi->getSQLiteObjectDbi()->getObject(res, mcaId, os);
    SAFE_POINT_OP(os, res);

    SQLiteQuery query("SELECT length, alphabet FROM Mca WHERE object = ?1", db, os);
    query.bindDataId(1, mcaId);
    if (query.step()) {
        res.length = query.getInt64(0);
        res.alphabet = query.getString(1);
        query.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca object not found"));
    }
    return res;
}

qint64 SQLiteMcaDbi::getNumOfRows(const U2DataId &mcaId, U2OpStatus &os) {
    qint64 res = 0;
    SQLiteQuery q("SELECT numOfRows FROM Mca WHERE object = ?1", db, os);
    CHECK_OP(os, res);

    q.bindDataId(1, mcaId);
    if (q.step()) {
        res = q.getInt64(0);
        q.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca object not found"));
    }
    return res;
}

QList<U2McaRow> SQLiteMcaDbi::getRows(const U2DataId &mcaId, U2OpStatus &os) {
    QList<U2McaRow> res;
    SQLiteQuery query("SELECT rowId, chromatogram, predictedSequence, editedSequence, workingAreaStart, workingAreaEnd, length FROM McaRow WHERE mca = ?1 ORDER BY pos", db, os);
    query.bindDataId(1, mcaId);

    SQLiteQuery gapQuery("SELECT gapStart, gapEnd FROM McaRowGap WHERE mca = ?1 AND rowId = ?2 AND relatedObjectId = ?3 ORDER BY gapStart", db, os);
    while (query.step()) {
        U2McaRow row;
        row.rowId = query.getInt64(0);
        row.chromatogramId = query.getDataId(1, U2Type::Chromatogram);
        row.predictedSequenceId = query.getDataId(2, U2Type::Sequence);
        row.sequenceId = query.getDataId(3, U2Type::Sequence);
        row.gstart = query.getInt64(4);
        row.gend = query.getInt64(5);
        row.length = query.getInt64(6);

        // predictedSequence gap model
        gapQuery.reset();
        gapQuery.bindDataId(1, mcaId);
        gapQuery.bindInt64(2, row.rowId);
        gapQuery.bindInt64(3, U2DbiUtils::toDbiId(row.predictedSequenceId));
        while (gapQuery.step()) {
            U2MsaGap gap;
            gap.offset = gapQuery.getInt64(0);
            gap.gap = gapQuery.getInt64(1) - gap.offset;
            SAFE_POINT_EXT(gap.isValid(), os.setError("An invalid gap is stored in the database"), res);
            row.predictedSequenceGaps.append(gap);
        }

        // editedSequence gap model
        gapQuery.reset();
        gapQuery.bindDataId(1, mcaId);
        gapQuery.bindInt64(2, row.rowId);
        gapQuery.bindInt64(3, U2DbiUtils::toDbiId(row.sequenceId));
        while (gapQuery.step()) {
            U2MsaGap gap;
            gap.offset = gapQuery.getInt64(0);
            gap.gap = gapQuery.getInt64(1) - gap.offset;
            SAFE_POINT_EXT(gap.isValid(), os.setError("An invalid gap is stored in the database"), res);
            row.gaps.append(gap);
        }

        SAFE_POINT_OP(os, res);
        res.append(row);
    }
    return res;
}

U2McaRow SQLiteMcaDbi::getRow(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    U2McaRow res;
    SQLiteQuery query("SELECT chromatogram, predictedSequence, editedSequence, workingAreaStart, workingAreaEnd, length FROM McaRow WHERE mca = ?1 AND rowId = ?2", db, os);
    SAFE_POINT_OP(os, res);

    query.bindDataId(1, mcaId);
    query.bindInt64(2, rowId);
    if (query.step()) {
        res.rowId = rowId;
        res.chromatogramId = query.getDataId(0, U2Type::Chromatogram);
        res.predictedSequenceId = query.getDataId(1, U2Type::Sequence);
        res.sequenceId = query.getDataId(2, U2Type::Sequence);
        res.gstart = query.getInt64(3);
        res.gend = query.getInt64(4);
        res.length = query.getInt64(5);
        query.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca row not found"));
        SAFE_POINT_OP(os, res);
    }

    SQLiteQuery gapQuery("SELECT gapStart, gapEnd FROM McaRowGap WHERE mca = ?1 AND rowId = ?2 AND relatedObjectId = ?3 ORDER BY gapStart", db, os);
    SAFE_POINT_OP(os, res);

    // predictedSequence gap model
    gapQuery.bindDataId(1, mcaId);
    gapQuery.bindInt64(2, rowId);
    gapQuery.bindInt64(3, U2DbiUtils::toDbiId(res.predictedSequenceId));
    while (gapQuery.step()) {
        U2MsaGap gap;
        gap.offset = gapQuery.getInt64(0);
        gap.gap = gapQuery.getInt64(1) - gap.offset;
        res.predictedSequenceGaps.append(gap);
    }

    // editedSequence gap model
    gapQuery.reset();
    gapQuery.bindDataId(1, mcaId);
    gapQuery.bindInt64(2, rowId);
    gapQuery.bindInt64(3, U2DbiUtils::toDbiId(res.sequenceId));
    while (gapQuery.step()) {
        U2MsaGap gap;
        gap.offset = gapQuery.getInt64(0);
        gap.gap = gapQuery.getInt64(1) - gap.offset;
        res.gaps.append(gap);
    }

    return res;
}

QList<qint64> SQLiteMcaDbi::getRowsOrder(const U2DataId &mcaId, U2OpStatus &os) {
    QList<qint64> res;
    SQLiteQuery query("SELECT rowId FROM McaRow WHERE mca = ?1 ORDER BY pos", db, os);
    query.bindDataId(1, mcaId);
    qint64 rowId = U2McaRow::INVALID_ROW_ID;
    while (query.step()) {
        rowId = query.getInt64(0);
        res.append(rowId);
    }
    return res;
}

U2AlphabetId SQLiteMcaDbi::getMcaAlphabet(const U2DataId &mcaId, U2OpStatus &os) {
    QString alphabetName;
    SQLiteQuery query("SELECT alphabet FROM Mca WHERE object = ?1", db, os);
    query.bindDataId(1, mcaId);
    if (query.step()) {
        alphabetName = query.getString(0);
        query.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca object not found"));
    }

    return U2AlphabetId(alphabetName);
}

qint64 SQLiteMcaDbi::getMcaLength(const U2DataId &mcaId, U2OpStatus &os) {
    qint64 res = 0;
    SQLiteQuery query("SELECT length FROM Mca WHERE object = ?1", db, os);
    CHECK_OP(os, res);

    query.bindDataId(1, mcaId);
    if (query.step()) {
        res = query.getInt64(0);
        query.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca object not found"));
    }

    return res;
}

U2DataId SQLiteMcaDbi::createMcaObject(const QString &folder, const QString &name, const U2AlphabetId &alphabet, U2OpStatus &os) {
    return createMcaObject(folder, name, alphabet, 0, os);
}

U2DataId SQLiteMcaDbi::createMcaObject(const QString &folder, const QString &name, const U2AlphabetId &alphabet, int length, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    U2Msa mca;
    mca.visualName = name;
    mca.alphabet = alphabet;
    mca.length = length;

    // Create the object
    dbi->getSQLiteObjectDbi()->createObject(mca, folder, U2DbiObjectRank_TopLevel, os);
    CHECK_OP(os, U2DataId());

    // Create a record in the Mca table
    SQLiteQuery q("INSERT INTO Mca(object, length, alphabet, numOfRows) VALUES(?1, ?2, ?3, ?4)", db, os);
    CHECK_OP(os, U2DataId());

    q.bindDataId(1, mca.id);
    q.bindInt64(2, mca.length);
    q.bindString(3, mca.alphabet.id);
    q.bindInt64(4, 0); // no rows
    q.insert();

    return mca.id;
}

void SQLiteMcaDbi::updateMcaName(const U2DataId &mcaId, const QString &name, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    U2Object mcaObj;
    dbi->getSQLiteObjectDbi()->getObject(mcaObj, mcaId, os);
    CHECK_OP(os, );

    SQLiteObjectDbiUtils::renameObject(dbi, mcaObj, name, os);
}

void SQLiteMcaDbi::updateMcaAlphabet(const U2DataId &mcaId, const U2AlphabetId &alphabet, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    ModificationAction updateAction(dbi, mcaId);
    U2TrackModType trackMod = updateAction.prepare(os);
    CHECK_OP(os, );

    // Get modDetails, if required
    QByteArray modDetails;
    if (TrackOnUpdate == trackMod) {
        U2Msa mcaObj = getMcaObject(mcaId, os);
        CHECK_OP(os, );
        modDetails = PackUtils::packAlphabetDetails(mcaObj.alphabet, alphabet);
    }

    // Update the alphabet
    SQLiteQuery query("UPDATE Mca SET alphabet = ?1 WHERE object = ?2", db, os);
    CHECK_OP(os, );

    query.bindString(1, alphabet.id);
    query.bindDataId(2, mcaId);
    query.update(1);

    // Increment version; track the modification, if required
    updateAction.addModification(mcaId, U2ModType::mcaUpdatedAlphabet, modDetails, os);
    SAFE_POINT_OP(os, );

    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void SQLiteMcaDbi::addRows(const U2DataId &mcaId, QList<U2McaRow> &rows, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    ModificationAction updateAction(dbi, mcaId);
    U2TrackModType trackMod = updateAction.prepare(os);
    CHECK_OP(os, );

    // Add the rows
    qint64 numOfRows = getNumOfRows(mcaId, os);
    CHECK_OP(os, );

    QList<qint64> posInMca;
    for (int i = 0; i < rows.count(); i++) {
        posInMca << i + numOfRows;
    }

    qint64 maxRowId = getMaximumRowId(mcaId, os);
    for (int i = 0; i < rows.count(); ++i) {
        rows[i].rowId = maxRowId + i + 1;
    }

    QByteArray modDetails;
    if (TrackOnUpdate == trackMod) {
        modDetails = PackUtils::packRows(posInMca, rows);
    }

    addRowsCore(mcaId, posInMca, rows, os);
    CHECK_OP(os, );

    // Update mca length
    qint64 maxLength = 0;
    foreach (const U2McaRow &row, rows) {
        maxLength = qMax(maxLength, row.length);
    }

    if (maxLength > getMcaLength(mcaId, os)) {
        updateMcaLength(updateAction, mcaId, maxLength, os);
        CHECK_OP(os, );
    }

    // Update track mod type for child objects
    if (TrackOnUpdate == trackMod) {
        foreach (const U2McaRow &row, rows) {
            dbi->getObjectDbi()->setTrackModType(row.chromatogramId, TrackOnUpdate, os);
            dbi->getObjectDbi()->setTrackModType(row.predictedSequenceId, TrackOnUpdate, os);
            dbi->getObjectDbi()->setTrackModType(row.sequenceId, TrackOnUpdate, os);
            CHECK_OP(os, );
        }
    }

    // Increment version; track the modification, if required
    updateAction.addModification(mcaId, U2ModType::mcaAddedRows, modDetails, os);
    SAFE_POINT_OP(os, );

    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void SQLiteMcaDbi::addRow(const U2DataId &mcaId, qint64 posInMca, U2McaRow &row, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    ModificationAction updateAction(dbi, mcaId);
    U2TrackModType trackMod = updateAction.prepare(os);
    CHECK_OP(os, );

    row.rowId = getMaximumRowId(mcaId, os) + 1;
    CHECK_OP(os, );

    addRowCore(mcaId, posInMca, row, os);
    CHECK_OP(os, );

    QByteArray modDetails;
    if (TrackOnUpdate == trackMod) {
        modDetails = PackUtils::packRow(posInMca, row);
    }

    if (row.length > getMcaLength(mcaId, os)) {
        updateMcaLength(updateAction, mcaId, row.length, os);
    }

    // Update track mod type for child objects
    if (TrackOnUpdate == trackMod) {
        dbi->getObjectDbi()->setTrackModType(row.chromatogramId, TrackOnUpdate, os);
        dbi->getObjectDbi()->setTrackModType(row.predictedSequenceId, TrackOnUpdate, os);
        dbi->getObjectDbi()->setTrackModType(row.sequenceId, TrackOnUpdate, os);
        CHECK_OP(os, );
    }

    // Increment version; track the modification, if required
    updateAction.addModification(mcaId, U2ModType::mcaAddedRow, modDetails, os);
    SAFE_POINT_OP(os, );

    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void SQLiteMcaDbi::removeRows(const U2DataId &mcaId, const QList<qint64> &rowIds, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    ModificationAction updateAction(dbi, mcaId);
    U2TrackModType trackMod = updateAction.prepare(os);
    CHECK_OP(os, );

    QByteArray modDetails;
    int numOfRows = getNumOfRows(mcaId, os);
    if (TrackOnUpdate == trackMod) {
        QList<qint64> posInMca;
        QList<U2McaRow> rows;
        foreach (qint64 rowId, rowIds) {
            posInMca << getPosInMca(mcaId, rowId, os);
            CHECK_OP(os, );
            rows << getRow(mcaId, rowId, os);
            CHECK_OP(os, );
        }
        modDetails = PackUtils::packRows(posInMca, rows);
    }

    const bool removeChildObjects = (TrackOnUpdate != trackMod);
    removeRowsCore(mcaId, rowIds, removeChildObjects, os);

    if (numOfRows == rowIds.count()) {
        updateMcaLength(updateAction, mcaId, 0, os);
    }

    // Increment version; track the modification, if required
    updateAction.addModification(mcaId, U2ModType::mcaRemovedRows, modDetails, os);
    SAFE_POINT_OP(os, );

    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void SQLiteMcaDbi::removeRow(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    ModificationAction updateAction(dbi, mcaId);
    U2TrackModType trackMod = updateAction.prepare(os);
    CHECK_OP(os, );

    QByteArray modDetails;
    if (TrackOnUpdate == trackMod) {
        U2McaRow removedRow = getRow(mcaId, rowId, os);
        CHECK_OP(os, );
        qint64 posInMca = getPosInMca(mcaId, rowId, os);
        CHECK_OP(os, );
        modDetails = PackUtils::packRow(posInMca, removedRow);
    }

    const bool removeChildObjects = (TrackOnUpdate != trackMod);
    removeRowCore(mcaId, rowId, removeChildObjects, os);
    CHECK_OP(os, );

    // Increment version; track the modification, if required
    updateAction.addModification(mcaId, U2ModType::mcaRemovedRow, modDetails, os);
    SAFE_POINT_OP(os, );

    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void SQLiteMcaDbi::deleteRowsData(const U2DataId &mcaId, U2OpStatus &os) {
    static const QString deleteObjectString = "DELETE FROM Object WHERE id IN (SELECT chromatogram, predictedSequence, editedSequence FROM McaRow WHERE mca = ?1)";
    SQLiteQuery deleteObjectQeury(deleteObjectString, db, os);
    deleteObjectQeury.bindDataId(1, mcaId);
    deleteObjectQeury.execute();
}

void SQLiteMcaDbi::updateRowName(const U2DataId &mcaId, qint64 rowId, const QString &newName, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    ModificationAction updateAction(dbi, mcaId);
    updateAction.prepare(os);
    SAFE_POINT_OP(os, );

    const U2DataId editedSequenceId = getEditedSequenceIdByRowId(mcaId, rowId, os);
    SAFE_POINT_OP(os, );

    U2Sequence seqObject = dbi->getSequenceDbi()->getSequenceObject(editedSequenceId, os);
    SAFE_POINT_OP(os, );

    SQLiteObjectDbiUtils::renameObject(updateAction, dbi, seqObject, newName, os);
    SAFE_POINT_OP(os, );

    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void SQLiteMcaDbi::updateRowContent(const U2DataId &mcaId,
                                    qint64 rowId,
                                    const DNAChromatogram &chromatogram,
                                    const DNASequence &predictedSequence,
                                    const U2MsaRowGapModel &predictedSequenceGapModel,
                                    const DNASequence &editedSequence,
                                    const U2MsaRowGapModel &editedSequenceGapModel,
                                    U2OpStatus &os) {
    SAFE_POINT_EXT(chromatogram.traceLength == predictedSequence.length(), os.setError("Chromatogram length doesn't equal predicted sequence length"), );
    SAFE_POINT_EXT(calculateRowLength(predictedSequence.length(), predictedSequenceGapModel) == calculateRowLength(editedSequence.length(), editedSequenceGapModel),
                   os.setError("Predicted sequence length doesn't equal edited sequence length"), );

    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    ModificationAction updateAction(dbi, mcaId);
    updateAction.prepare(os);
    SAFE_POINT_OP(os, );

    // Get the row object
    U2McaRow row = getRow(mcaId, rowId, os);
    SAFE_POINT_OP(os, );

    QVariantMap hints;
    // Update the chromatogram data
    // TODO: make chromatogram updating be undoable
    const QByteArray serializedChromatogram = DNAChromatogramSerializer::serialize(chromatogram);
    RawDataUdrSchema::writeContent(serializedChromatogram, U2EntityRef(dbi->getDbiRef(), row.chromatogramId), os);
    SAFE_POINT_OP(os, );

    // Update the predicted sequence data
    dbi->getSQLiteSequenceDbi()->updateSequenceData(updateAction, row.predictedSequenceId, U2_REGION_MAX, predictedSequence.seq, hints, os);
    SAFE_POINT_OP(os, );

    // Update the edited sequence data
    dbi->getSQLiteSequenceDbi()->updateSequenceData(updateAction, row.sequenceId, U2_REGION_MAX, editedSequence.seq, hints, os);
    SAFE_POINT_OP(os, );

    // Update the row object
    U2McaRow newRow(row);
    const qint64 seqLength = predictedSequence.length();
    // TODO: add a possibility to set the working area with other parameters
    newRow.gstart = 0;
    newRow.gend = seqLength;
    newRow.length = calculateRowLength(seqLength, predictedSequenceGapModel);
    updateRowInfo(updateAction, mcaId, newRow, os);
    SAFE_POINT_OP(os, );

    // Update the gap models
    // WARNING: this update must go after the row info update to recalculate the mca length properly
    updateGapModel(updateAction, mcaId, rowId, row.predictedSequenceId, predictedSequenceGapModel, os);
    SAFE_POINT_OP(os, );
    updateGapModel(updateAction, mcaId, rowId, row.sequenceId, editedSequenceGapModel, os);
    SAFE_POINT_OP(os, );

    // Save tracks, if required; increment versions
    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void SQLiteMcaDbi::updateRowContent(const U2DataId &mcaId, qint64 rowId, const McaRowMemoryData &rowMemoryData, U2OpStatus &os) {
    // TODO: add a possibility to set the working area with other parameters
    updateRowContent(mcaId,
                     rowId,
                     rowMemoryData.chromatogram,
                     rowMemoryData.predictedSequence,
                     rowMemoryData.predictedSequenceGapModel,
                     rowMemoryData.editedSequence,
                     rowMemoryData.editedSequenceGapModel,
                     os);
}

void SQLiteMcaDbi::updateRowChromatogram(const U2DataId &mcaId, qint64 rowId, const DNAChromatogram &chromatogram, U2OpStatus &os) {
    // TODO: all
//    SAFE_POINT_EXT(calculateRowLength(predictedSequence.length(), predictedSequenceGapModel) == calculateRowLength(editedSequence, editedSequenceGapModel),
//                   os.setError("Predicted sequence length doesn't equal edited sequence length"), );

//    SQLiteTransaction t(db, os);
//    Q_UNUSED(t);

//    ModificationAction updateAction(dbi, mcaId);
//    updateAction.prepare(os);
//    SAFE_POINT_OP(os, );

//    // Get the row object
//    U2McaRow row = getRow(mcaId, rowId, os);
//    SAFE_POINT_OP(os, );
//    SAFE_POINT_EXT(calculateRowLength(chromatogram.traceLength, gapModel) == row.length, os.setError("Chromatogram core length doesn't equal row length"), );

//    QVariantMap hints;
//    // Update the chromatogram data
//    // TODO
//    const QByteArray serializedChromatogram = DNAChromatogramSerializer::serialize(chromatogram);
//    RawDataUdrSchema::writeContent(serializedChromatogram, U2EntityRef(dbi->getDbiRef(), row.chromatogramId), os);
//    SAFE_POINT_OP(os, );

//    // Update the predicted sequence data
//    dbi->getSQLiteSequenceDbi()->updateSequenceData(updateAction, row.predictedSequenceId, U2_REGION_MAX, predictedSequence.seq, hints, os);
//    SAFE_POINT_OP(os, );

//    // Update the edited sequence data
//    dbi->getSQLiteSequenceDbi()->updateSequenceData(updateAction, row.sequenceId, U2_REGION_MAX, editedSequence.seq, hints, os);
//    SAFE_POINT_OP(os, );

//    // Update the row object
//    U2McaRow newRow(row);
//    const qint64 seqLength = predictedSequence.length();
//    newRow.gstart = 0;
//    newRow.gend = seqLength;
//    newRow.length = calculateRowLength(seqLength, predictedSequenceGapModel);
//    updateRowInfo(updateAction, mcaId, newRow, os);
//    SAFE_POINT_OP(os, );

//    // Update the gap models
//    // WARNING: this update must go after the row info update to recalculate the mca length properly
//    updateGapModel(updateAction, mcaId, rowId, row.predictedSequenceId, predictedSequenceGapModel, os);
//    SAFE_POINT_OP(os, );
//    updateGapModel(updateAction, mcaId, rowId, row.editedSequenceId, editedSequenceGapModel, os);
//    SAFE_POINT_OP(os, );

//    // Save tracks, if required; increment versions
//    updateAction.complete(os);
//    SAFE_POINT_OP(os, );
}

void SQLiteMcaDbi::updateRowSequence(const U2DataId &mcaId, qint64 rowId, qint64 sequenceId, const QByteArray &sequenceData, const U2MsaRowGapModel &gapModel, U2OpStatus &os) {
    // TODO: all
}

void SQLiteMcaDbi::updateGapModel(const U2DataId &mcaId, qint64 mcaRowId, qint64 gapModelOwner, const U2MsaRowGapModel &gapModel, U2OpStatus &os) {
    // TODO: all
}

void SQLiteMcaDbi::updateMcaLength(const U2DataId &mcaId, qint64 length, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    ModificationAction updateAction(dbi, mcaId);
    updateAction.prepare(os);
    SAFE_POINT_OP(os, );

    updateMcaLength(updateAction, mcaId, length, os);

    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void SQLiteMcaDbi::setNewRowsOrder(const U2DataId &mcaId, const QList<qint64> &rowIds, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    ModificationAction updateAction(dbi, mcaId);
    U2TrackModType trackMod = updateAction.prepare(os);
    CHECK_OP(os, );

    QByteArray modDetails;
    if (TrackOnUpdate == trackMod) {
        QList<qint64> oldOrder = getRowsOrder(mcaId, os);
        CHECK_OP(os, );
        modDetails = PackUtils::packRowOrderDetails(oldOrder, rowIds);
    }

    // Check that row IDs number is correct (if required, can be later removed for efficiency)
    qint64 numOfRows = getNumOfRows(mcaId, os);
    CHECK_OP(os, );
    SAFE_POINT(numOfRows == rowIds.count(), "Incorrect number of row IDs", );

    // Set the new order
    setNewRowsOrderCore(mcaId, rowIds, os);
    CHECK_OP(os, );

    // Increment version; track the modification, if required
    updateAction.addModification(mcaId, U2ModType::mcaSetNewRowsOrder, modDetails, os);
    SAFE_POINT_OP(os, );

    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void SQLiteMcaDbi::undo(const U2DataId &mcaId, qint64 modType, const QByteArray &modDetails, U2OpStatus &os) {
    if (U2ModType::mcaUpdatedAlphabet == modType) {
        undoUpdateMcaAlphabet(mcaId, modDetails, os);
    } else if (U2ModType::mcaAddedRows == modType) {
        undoAddRows(mcaId, modDetails, os);
    } else if (U2ModType::mcaAddedRow == modType) {
        undoAddRow(mcaId, modDetails, os);
    } else if (U2ModType::mcaRemovedRows == modType) {
        undoRemoveRows(mcaId, modDetails, os);
    } else if (U2ModType::mcaRemovedRow == modType) {
        undoRemoveRow(mcaId, modDetails, os);
    } else if (U2ModType::mcaUpdatedRowInfo == modType) {
        undoUpdateRowInfo(mcaId, modDetails, os);
    } else if (U2ModType::mcaUpdatedGapModel == modType) {
        undoUpdateGapModel(mcaId, modDetails, os);
    } else if (U2ModType::mcaSetNewRowsOrder == modType) {
        undoSetNewRowsOrder(mcaId, modDetails, os);
    } else if (U2ModType::mcaLengthChanged == modType) {
        undoMcaLengthChange(mcaId, modDetails, os);
    } else {
        os.setError(QString("Unexpected modification type '%1'").arg(QString::number(modType)));
        return;
    }
}

void SQLiteMcaDbi::redo(const U2DataId &mcaId, qint64 modType, const QByteArray &modDetails, U2OpStatus &os) {
    if (U2ModType::mcaUpdatedAlphabet == modType) {
        redoUpdateMcaAlphabet(mcaId, modDetails, os);
    } else if (U2ModType::mcaAddedRows == modType) {
        redoAddRows(mcaId, modDetails, os);
    } else if (U2ModType::mcaAddedRow == modType) {
        redoAddRow(mcaId, modDetails, os);
    } else if (U2ModType::mcaRemovedRows == modType) {
        redoRemoveRows(mcaId, modDetails, os);
    } else if (U2ModType::mcaRemovedRow == modType) {
        redoRemoveRow(mcaId, modDetails, os);
    } else if (U2ModType::mcaUpdatedRowInfo == modType) {
        redoUpdateRowInfo(mcaId, modDetails, os);
    } else if (U2ModType::mcaUpdatedGapModel == modType) {
        redoUpdateGapModel(mcaId, modDetails, os);
    } else if (U2ModType::mcaSetNewRowsOrder == modType) {
        redoSetNewRowsOrder(mcaId, modDetails, os);
    } else if (U2ModType::mcaLengthChanged == modType) {
        redoMcaLengthChange(mcaId, modDetails, os);
    } else {
        os.setError(QString("Unexpected modification type '%1'").arg(QString::number(modType)));
        return;
    }
}

void SQLiteMcaDbi::addMcaRowAndGaps(const U2DataId &mcaId, qint64 posInMca, U2McaRow &row, U2OpStatus &os) {
    createMcaRow(mcaId, posInMca, row, os);
    CHECK_OP(os, );

    foreach (const U2MsaGap& gap, row.predictedSequenceGaps) {
        createMcaRowGap(mcaId, row.rowId, row.predictedSequenceId, gap, os);
        CHECK_OP(os, );
    }

    foreach (const U2MsaGap& gap, row.gaps) {
        createMcaRowGap(mcaId, row.rowId, row.sequenceId, gap, os);
        CHECK_OP(os, );
    }

    dbi->getSQLiteObjectDbi()->setParent(mcaId, row.sequenceId, os);
}

void SQLiteMcaDbi::createMcaRow(const U2DataId &mcaId, qint64 posInMca, U2McaRow &mcaRow, U2OpStatus &os) {
    assert(posInMca >= 0);

    // Calculate the row length
    qint64 rowLength = calculateRowLength(mcaRow.gend - mcaRow.gstart, mcaRow.gaps);

    // Insert the data
    SQLiteQuery query("INSERT INTO McaRow(mca, rowId, chromatogram, predictedSequence, editedSequence, pos, workingAreaStart, workingAreaEnd, length)"
        " VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9)", db, os);
    CHECK_OP(os, );

    query.bindDataId(1, mcaId);
    query.bindInt64(2, mcaRow.rowId);
    query.bindDataId(3, mcaRow.chromatogramId);
    query.bindDataId(4, mcaRow.predictedSequenceId);
    query.bindDataId(5, mcaRow.sequenceId);
    query.bindInt64(6, posInMca);
    query.bindInt64(7, mcaRow.gstart);
    query.bindInt64(8, mcaRow.gend);
    query.bindInt64(9, rowLength);
    query.insert();
}

void SQLiteMcaDbi::createMcaRowGap(const U2DataId &mcaId, qint64 mcaRowId, const U2DataId &relatedObjectId, const U2MsaGap &mcaGap, U2OpStatus &os) {
    // TODO: signature was changed, fix method calls
    SQLiteTransaction t(db, os);

    static const QString queryString("INSERT INTO McaRowGap(mca, rowId, relatedObjectId, gapStart, gapEnd) VALUES(?1, ?2, ?3, ?4, ?5)");
    QSharedPointer<SQLiteQuery> query = t.getPreparedQuery(queryString, db, os);
    CHECK_OP(os, );

    query->bindDataId(1, mcaId);
    query->bindInt64(2, mcaRowId);
    query->bindInt64(3, U2DbiUtils::toDbiId(relatedObjectId));
    query->bindInt64(4, mcaGap.offset);
    query->bindInt64(5, mcaGap.offset + mcaGap.gap);
    query->insert();
}

void SQLiteMcaDbi::removeMcaRowAndGaps(const U2DataId &mcaId, qint64 rowId, bool removeChilds, U2OpStatus &os) {
    const U2DataId chromatogramId = getChromatogramIdByRowId(mcaId, rowId, os);
    const U2DataId predictedSequenceId = getPredictedSequenceIdByRowId(mcaId, rowId, os);
    const U2DataId editedSequenceId = getEditedSequenceIdByRowId(mcaId, rowId, os);
    CHECK_OP(os, );

    removeRecordsFromMcaRowGap(mcaId, rowId, os);
    removeRecordFromMcaRow(mcaId, rowId, os);

    dbi->getSQLiteObjectDbi()->removeParent(mcaId, chromatogramId, removeChilds, os);
    dbi->getSQLiteObjectDbi()->removeParent(mcaId, predictedSequenceId, removeChilds, os);
    dbi->getSQLiteObjectDbi()->removeParent(mcaId, editedSequenceId, removeChilds, os);
}

void SQLiteMcaDbi::removeRecordsFromMcaRowGap(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    SQLiteTransaction t(db, os);

    static const QString queryString("DELETE FROM McaRowGap WHERE mca = ?1 AND rowId = ?2");
    QSharedPointer<SQLiteQuery> query = t.getPreparedQuery(queryString, db, os);
    CHECK_OP(os, );

    query->bindDataId(1, mcaId);
    query->bindInt64(2, rowId);
    query->update();
}

void SQLiteMcaDbi::removeRecordFromMcaRow(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    SQLiteTransaction t(db, os);

    static const QString queryString("DELETE FROM McaRow WHERE mca = ?1 AND rowId = ?2");
    QSharedPointer<SQLiteQuery> query = t.getPreparedQuery(queryString, db, os);
    CHECK_OP(os, );

    query->bindDataId(1, mcaId);
    query->bindInt64(2, rowId);
    query->update(1);
}

void SQLiteMcaDbi::updateNumOfRows(const U2DataId &mcaId, qint64 numOfRows, U2OpStatus &os) {
    SQLiteQuery query("UPDATE Mca SET numOfRows = ?1 WHERE object = ?2", db, os);
    SAFE_POINT_OP(os, );

    query.bindInt64(1, numOfRows);
    query.bindDataId(2, mcaId);
    query.update(1);
}

void SQLiteMcaDbi::recalculateRowsPositions(const U2DataId &mcaId, U2OpStatus &os) {
    QList<U2McaRow> rows = getRows(mcaId, os);
    CHECK_OP(os, );

    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    SQLiteQuery query("UPDATE McaRow SET pos = ?1 WHERE mca = ?2 AND rowId = ?3", db, os);
    CHECK_OP(os, );

    for (int i = 0, n = rows.count(); i < n; ++i) {
        qint64 rowId = rows[i].rowId;
        query.reset();
        query.bindInt64(1, i);
        query.bindDataId(2, mcaId);
        query.bindInt64(3, rowId);
        query.execute();
    }
}

qint64 SQLiteMcaDbi::calculateRowLength(qint64 seqLength, const QList<U2MsaGap> &gaps) {
    // TODO: check, if trailing gaps should not be skipped
    qint64 res = seqLength;
    foreach (const U2MsaGap &gap, gaps) {
        if (gap.offset < res) { // ignore trailing gaps
            res += gap.gap;
        }
    }
    return res;
}

qint64 SQLiteMcaDbi::getRowSequenceLength(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    // TODO: check, if this method should be rewritten
    qint64 res = 0;
    SQLiteQuery query("SELECT gstart, gend FROM McaRow WHERE mca = ?1 AND rowId = ?2", db, os);
    CHECK_OP(os, res);

    query.bindDataId(1, mcaId);
    query.bindInt64(2, rowId);
    if (query.step()) {
        qint64 startInSeq = query.getInt64(0);
        qint64 endInSeq = query.getInt64(1);
        res = endInSeq - startInSeq;
        query.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca row not found"));
    }

    return res;
}

void SQLiteMcaDbi::updateRowLength(const U2DataId &mcaId, qint64 rowId, qint64 newLength, U2OpStatus &os) {
    SQLiteQuery query("UPDATE McaRow SET length = ?1 WHERE mca = ?2 AND rowId = ?3", db, os);
    CHECK_OP(os, );

    query.bindInt64(1, newLength);
    query.bindDataId(2, mcaId);
    query.bindInt64(3, rowId);
    query.update(1);
}

U2DataId SQLiteMcaDbi::getChromatogramIdByRowId(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    U2DataId res;
    SQLiteQuery query("SELECT chromatogram FROM McaRow WHERE mca = ?1 AND rowId = ?2", db, os);
    CHECK_OP(os, res);

    query.bindDataId(1, mcaId);
    query.bindInt64(2, rowId);
    if (query.step()) {
        res = query.getDataId(0, U2Type::Chromatogram);
        query.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca row not found"));
    }

    return res;
}

U2DataId SQLiteMcaDbi::getPredictedSequenceIdByRowId(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    U2DataId res;
    SQLiteQuery query("SELECT predictedSequence FROM McaRow WHERE mca = ?1 AND rowId = ?2", db, os);
    CHECK_OP(os, res);

    query.bindDataId(1, mcaId);
    query.bindInt64(2, rowId);
    if (query.step()) {
        res = query.getDataId(0, U2Type::Sequence);
        query.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca row not found"));
    }

    return res;
}

U2DataId SQLiteMcaDbi::getEditedSequenceIdByRowId(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    U2DataId res;
    SQLiteQuery query("SELECT editedSequence FROM McaRow WHERE mca = ?1 AND rowId = ?2", db, os);
    CHECK_OP(os, res);

    query.bindDataId(1, mcaId);
    query.bindInt64(2, rowId);
    if (query.step()) {
        res = query.getDataId(0, U2Type::Sequence);
        query.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca row not found"));
    }

    return res;
}

qint64 SQLiteMcaDbi::getPosInMca(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    SQLiteQuery query("SELECT pos FROM McaRow WHERE mca = ?1 AND rowId = ?2", db, os);
    CHECK_OP(os, -1);
    query.bindDataId(1, mcaId);
    query.bindInt64(2, rowId);
    if (query.step()) {
        qint64 result = query.getInt64(0);
        query.ensureDone();
        return result;
    }
    if (!os.hasError()) {
        os.setError(QString("No row with id '%1' in mca '%2'").arg(QString::number(rowId)).arg(mcaId.data()));
    }
    return -1;
}

qint64 SQLiteMcaDbi::getMaximumRowId(const U2DataId &mcaId, U2OpStatus &os) {
    qint64 maxRowId = 0;
    SQLiteQuery query("SELECT MAX(rowId) FROM McaRow WHERE mca = ?1", db, os);
    SAFE_POINT_OP(os, 0);

    query.bindDataId(1, mcaId);
    query.getInt64(1);
    if (query.step()) {
        maxRowId = query.getInt64(0);
    }

    return maxRowId;
}

void SQLiteMcaDbi::updateGapModelCore(const U2DataId &mcaId, qint64 mcaRowId, const U2DataId &relatedObjectId, const U2MsaRowGapModel &gapModel, U2OpStatus &os) {
    // TODO: signature was changed, update method calls
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    // Remove obsolete gaps of the row
    removeRecordsFromMcaRowGap(mcaId, mcaRowId, os);
    CHECK_OP(os, );

    // Store the new gap model
    foreach (const U2MsaGap &gap, gapModel) {
        createMcaRowGap(mcaId, mcaRowId, relatedObjectId, gap, os);
        CHECK_OP(os, );
    }

    // Update the row length (without trailing gaps)
    qint64 rowSequenceLength = getRowSequenceLength(mcaId, mcaRowId, os);
    CHECK_OP(os, );

    // TODO: check if trailing gaps sbhouldn't be skipped
    qint64 newRowLength = calculateRowLength(rowSequenceLength, gapModel);
    updateRowLength(mcaId, mcaRowId, newRowLength, os);
    CHECK_OP(os, );
}

void SQLiteMcaDbi::addRowSubcore(const U2DataId &mcaId, qint64 numOfRows, const QList<qint64> &rowsOrder, U2OpStatus &os) {
    // Re-calculate position, if needed
    setNewRowsOrderCore(mcaId, rowsOrder, os);
    CHECK_OP(os, );

    // Update the number of rows of the MCA
    updateNumOfRows(mcaId, numOfRows, os);
}

void SQLiteMcaDbi::addRowCore(const U2DataId &mcaId, qint64 posInMca, U2McaRow &row, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    // Append the row to the end, if "-1"
    qint64 numOfRows = getNumOfRows(mcaId, os);
    CHECK_OP(os, );

    QList<qint64> rowsOrder;
    if (-1 == posInMca) {
        posInMca = numOfRows;
    } else {
        rowsOrder = getRowsOrder(mcaId, os);
        CHECK_OP(os, );
        SAFE_POINT(rowsOrder.count() == numOfRows, "Incorrect number of rows", );
    }
    SAFE_POINT(posInMca >= 0 && posInMca <= numOfRows, "Incorrect input position", );

    // Create the row
    addMcaRowAndGaps(mcaId, posInMca, row, os);
    CHECK_OP(os, );

    // Update the alignment length
    // TODO: check length
    row.length = calculateRowLength(row.gend - row.gstart, row.gaps);
    if (posInMca != numOfRows) {
        rowsOrder.insert(posInMca, row.rowId);
    }
    addRowSubcore(mcaId, numOfRows + 1, rowsOrder, os);
}

void SQLiteMcaDbi::addRowsCore(const U2DataId &mcaId, const QList<qint64> &posInMca, QList<U2McaRow> &rows, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    qint64 numOfRows = getNumOfRows(mcaId, os);
    CHECK_OP(os, );

    QList<qint64> rowsOrder = getRowsOrder(mcaId, os);
    CHECK_OP(os, );
    SAFE_POINT(rowsOrder.count() == numOfRows, "Incorrect number of rows", );

    // Add new rows
    QList<qint64>::ConstIterator pi = posInMca.begin();
    QList<U2McaRow>::Iterator ri = rows.begin();
    for (; ri != rows.end(); ri++, pi++) {
        qint64 pos = *pi;
        if (-1 == pos) {
            pos = numOfRows;
        }
        SAFE_POINT(pos >= 0 && pos <= numOfRows, "Incorrect input position", );
        addMcaRowAndGaps(mcaId, pos, *ri, os);
        CHECK_OP(os, );
        // TODO: check row length
        ri->length = calculateRowLength(ri->gend - ri->gstart, ri->gaps);
        numOfRows++;
        rowsOrder.insert(pos, ri->rowId);
    }

    addRowSubcore(mcaId, numOfRows, rowsOrder, os);
}

void SQLiteMcaDbi::removeRowSubcore(const U2DataId &mcaId, qint64 numOfRows, U2OpStatus &os) {
    // Update the number of rows
    updateNumOfRows(mcaId, numOfRows, os);
    CHECK_OP(os, );

    // Re-calculate the rows positions
    recalculateRowsPositions(mcaId, os);
}

void SQLiteMcaDbi::removeRowCore(const U2DataId &mcaId, qint64 rowId, bool removeChilds, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    // Get and verify the number of rows
    qint64 numOfRows = getNumOfRows(mcaId, os);
    CHECK_OP(os, );
    SAFE_POINT(numOfRows > 0, "Empty MCA", );

    // Remove the row
    removeMcaRowAndGaps(mcaId, rowId, removeChilds, os);
    CHECK_OP(os, );

    removeRowSubcore(mcaId, numOfRows - 1, os);
}

void SQLiteMcaDbi::removeRowsCore(const U2DataId &mcaId, const QList<qint64> &rowIds, bool removeChilds, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    qint64 numOfRows = getNumOfRows(mcaId, os);
    CHECK_OP(os, );
    SAFE_POINT(numOfRows >= rowIds.count() , "Incorrect rows to remove", );

    for (int i = 0; i < rowIds.count(); ++i) {
        removeMcaRowAndGaps(mcaId, rowIds[i], removeChilds, os);
        CHECK_OP(os, );
    }

    removeRowSubcore(mcaId, numOfRows - rowIds.size(), os);
}

void SQLiteMcaDbi::setNewRowsOrderCore(const U2DataId &mcaId, const QList<qint64> rowIds, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    SQLiteQuery query("UPDATE McaRow SET pos = ?1 WHERE mca = ?2 AND rowId = ?3", db, os);
    CHECK_OP(os, );

    for (int i = 0, n = rowIds.count(); i < n; ++i) {
        qint64 rowId = rowIds[i];
        query.reset();
        query.bindInt64(1, i);
        query.bindDataId(2, mcaId);
        query.bindInt64(3, rowId);
        query.execute();
    }
}

void SQLiteMcaDbi::updateRowInfoCore(const U2DataId &mcaId, const U2McaRow &row, U2OpStatus &os) {
    SQLiteQuery query("UPDATE McaRow SET chromatogram = ?1, predictedSequence = ?2, editedSequence = ?3, gstart = ?4, gend = ?5 WHERE mca = ?6 AND rowId = ?7", db, os);
    SAFE_POINT_OP(os, );

    query.bindDataId(1, row.chromatogramId);
    query.bindDataId(2, row.predictedSequenceId);
    query.bindDataId(3, row.sequenceId);
    query.bindInt64(4, row.gstart);
    query.bindInt64(5, row.gend);
    query.bindDataId(6, mcaId);
    query.bindInt64(7, row.rowId);
    query.update(1);
}

void SQLiteMcaDbi::updateMcaLengthCore(const U2DataId &mcaId, qint64 length, U2OpStatus &os) {
    SQLiteQuery query("UPDATE Mca SET length = ?1 WHERE object = ?2", db, os);
    CHECK_OP(os, );

    query.bindInt64(1, length);
    query.bindDataId(2, mcaId);
    query.execute();
}

void SQLiteMcaDbi::updateMcaAlphabetCore(const U2DataId &mcaId, const U2AlphabetId &alphabetId, U2OpStatus &os) {
    SQLiteQuery query("UPDATE Mca SET alphabet = ?1 WHERE object = ?2", db, os);
    CHECK_OP(os, );

    query.bindString(1, alphabetId.id);
    query.bindDataId(2, mcaId);
    query.update(1);
}

void SQLiteMcaDbi::undoUpdateMcaAlphabet(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2AlphabetId oldAlphabet;
    U2AlphabetId newAlphabet;
    bool ok = PackUtils::unpackAlphabetDetails(modDetails, oldAlphabet, newAlphabet);
    if (!ok) {
        os.setError("An error occurred during updating an alignment alphabet");
        return;
    }

    updateMcaAlphabetCore(mcaId, oldAlphabet, os);
}

void SQLiteMcaDbi::undoAddRows(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    QList<qint64> posInMca;
    QList<U2MsaRow> rows;
    bool ok = PackUtils::unpackRows(modDetails, posInMca, rows);
    if (!ok) {
        os.setError("An error occurred during reverting adding of rows");
        return;
    }

    QList<qint64> rowIds;
    foreach (const U2McaRow &row, rows) {
        rowIds << row.rowId;
    }
    removeRowsCore(mcaId, rowIds, false, os);
}

void SQLiteMcaDbi::undoAddRow(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2McaRow row;
    qint64 posInMca = U2McaRow::INVALID_ROW_ID;
    bool ok = PackUtils::unpackRow(modDetails, posInMca, row);
    if (!ok) {
        os.setError("An error occurred during reverting addition of a row");
        return;
    }

    removeRowCore(mcaId, row.rowId, false, os);
}

void SQLiteMcaDbi::undoRemoveRows(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    QList<qint64> posInMca;
    QList<U2McaRow> rows;
    bool ok = PackUtils::unpackRows(modDetails, posInMca, rows);
    if (!ok) {
        os.setError("An error occurred during reverting removing of rows");
        return;
    }

    addRowsCore(mcaId, posInMca, rows, os);
}

void SQLiteMcaDbi::undoRemoveRow(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2McaRow row;
    qint64 posInMca = U2McaRow::INVALID_ROW_ID;
    bool ok = PackUtils::unpackRow(modDetails, posInMca, row);
    if (!ok) {
        os.setError("An error occurred during reverting removing of a row");
        return;
    }

    addRowCore(mcaId, posInMca, row, os);
}

void SQLiteMcaDbi::undoUpdateGapModel(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    qint64 rowId = 0;
    U2DataId relatedObjectId;
    QList<U2MsaGap> oldGaps;
    QList<U2MsaGap> newGaps;
    bool ok = PackUtils::unpackGapDetails(modDetails, rowId, relatedObjectId, oldGaps, newGaps);
    if (!ok) {
        os.setError("An error occurred during updating an alignment gaps");
        return;
    }

    updateGapModelCore(mcaId, rowId, relatedObjectId, oldGaps, os);
}

void SQLiteMcaDbi::undoSetNewRowsOrder(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    QList<qint64> oldOrder;
    QList<qint64> newOrder;
    bool ok = PackUtils::unpackRowOrderDetails(modDetails, oldOrder, newOrder);
    if (!ok) {
        os.setError("An error occurred during updating an alignment row order");
        return;
    }

    // Set the old order
    setNewRowsOrderCore(mcaId, oldOrder, os);
}

void SQLiteMcaDbi::undoUpdateRowInfo(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2McaRow oldRow;
    U2McaRow newRow;
    bool ok = PackUtils::unpackRowInfoDetails(modDetails, oldRow, newRow);
    if (!ok) {
        os.setError("An error occurred during updating a row info");
        return;
    }

    // TODO: review checks
    SAFE_POINT(oldRow.rowId == newRow.rowId, "Incorrect rowId", );
    SAFE_POINT(oldRow.chromatogramId == newRow.chromatogramId, "Incorrect chromatogramId", );
    SAFE_POINT(oldRow.predictedSequenceId == newRow.predictedSequenceId, "Incorrect predictedSequenceId", );
    SAFE_POINT(oldRow.sequenceId == newRow.sequenceId, "Incorrect sequenceId", );

    updateRowInfoCore(mcaId, oldRow, os);
}

void SQLiteMcaDbi::undoMcaLengthChange(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    qint64 oldLen;
    qint64 newLen;

    bool ok = PackUtils::unpackAlignmentLength(modDetails, oldLen, newLen);
    CHECK_EXT(ok, os.setError(U2DbiL10n::tr("An error occurred during updating an mca length")), );

    updateMcaLengthCore(mcaId, oldLen, os);
}

void SQLiteMcaDbi::redoUpdateMcaAlphabet(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2AlphabetId oldAlphabet;
    U2AlphabetId newAlphabet;
    bool ok = PackUtils::unpackAlphabetDetails(modDetails, oldAlphabet, newAlphabet);
    if (!ok) {
        os.setError("An error occurred during updating an alignment alphabet");
        return;
    }

    updateMcaAlphabetCore(mcaId, newAlphabet, os);
}

void SQLiteMcaDbi::redoAddRows(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    QList<qint64> posInMca;
    QList<U2McaRow> rows;
    bool ok = PackUtils::unpackRows(modDetails, posInMca, rows);
    if (!ok) {
        os.setError("An error occurred during reverting adding of rows");
        return;
    }

    addRowsCore(mcaId, posInMca, rows, os);
}

void SQLiteMcaDbi::redoAddRow(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2McaRow row;
    qint64 posInMca = U2McaRow::INVALID_ROW_ID;
    bool ok = PackUtils::unpackRow(modDetails, posInMca, row);
    if (!ok) {
        os.setError("An error occurred during addition of a row");
        return;
    }

    addRowCore(mcaId, posInMca, row, os);
}

void SQLiteMcaDbi::redoRemoveRows(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    QList<qint64> posInMca;
    QList<U2McaRow> rows;
    bool ok = PackUtils::unpackRows(modDetails, posInMca, rows);
    if (!ok) {
        os.setError("An error occurred during reverting removing of rows");
        return;
    }

    QList<qint64> rowIds;
    foreach (const U2McaRow &row, rows) {
        rowIds << row.rowId;
    }
    removeRowsCore(mcaId, rowIds, false, os);
}

void SQLiteMcaDbi::redoRemoveRow(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2McaRow row;
    qint64 posInMca = U2McaRow::INVALID_ROW_ID;
    bool ok = PackUtils::unpackRow(modDetails, posInMca, row);
    if (!ok) {
        os.setError("An error occurred during reverting removing of a row");
        return;
    }

    removeRowCore(mcaId, row.rowId, false, os);
}

void SQLiteMcaDbi::redoUpdateGapModel(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    qint64 rowId = U2McaRow::INVALID_ROW_ID;
    U2DataId relatedObjectId;
    U2MsaRowGapModel oldGaps;
    U2MsaRowGapModel newGaps;
    bool ok = PackUtils::unpackGapDetails(modDetails, rowId, relatedObjectId, oldGaps, newGaps);
    if (!ok) {
        os.setError("An error occurred during updating an alignment gaps");
        return;
    }

    updateGapModelCore(mcaId, rowId, relatedObjectId, newGaps, os);
}

void SQLiteMcaDbi::redoSetNewRowsOrder(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    QList<qint64> oldOrder;
    QList<qint64> newOrder;
    bool ok = PackUtils::unpackRowOrderDetails(modDetails, oldOrder, newOrder);
    if (!ok) {
        os.setError("An error occurred during updating an alignment row order");
        return;
    }

    setNewRowsOrderCore(mcaId, newOrder, os);
}

void SQLiteMcaDbi::redoUpdateRowInfo(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2McaRow oldRow;
    U2McaRow newRow;
    bool ok = PackUtils::unpackRowInfoDetails(modDetails, oldRow, newRow);
    if (!ok) {
        os.setError("An error occurred during updating a row info");
        return;
    }

    // TODO: review checks
    SAFE_POINT(oldRow.rowId == newRow.rowId, "Incorrect rowId", );
    SAFE_POINT(oldRow.chromatogramId == newRow.chromatogramId, "Incorrect chromatogramId", );
    SAFE_POINT(oldRow.predictedSequenceId == newRow.predictedSequenceId, "Incorrect predictedSequenceId", );
    SAFE_POINT(oldRow.sequenceId == newRow.sequenceId, "Incorrect sequenceId", );

    updateRowInfoCore(mcaId, newRow, os);
}

void SQLiteMcaDbi::redoMcaLengthChange(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    qint64 oldLen = 0;
    qint64 newLen = 0;

    bool ok = PackUtils::unpackAlignmentLength(modDetails, oldLen, newLen);
    CHECK_EXT(ok, os.setError(U2DbiL10n::tr("An error occurred during updating an mca length")), );

    updateMcaLengthCore(mcaId, newLen, os);
}

void SQLiteMcaDbi::updateRowInfo(ModificationAction &updateAction, const U2DataId &mcaId, const U2McaRow &row, U2OpStatus &os) {
    QByteArray modDetails;
    if (TrackOnUpdate == updateAction.getTrackModType()) {
        U2McaRow oldRow = getRow(mcaId, row.rowId, os);
        SAFE_POINT_OP(os, );

        modDetails = PackUtils::packRowInfoDetails(oldRow, row);
    }

    updateRowInfoCore(mcaId, row, os);
    SAFE_POINT_OP(os, );

    // Track the modification, if required; add the object to the list (versions of the objects will be incremented on the updateAction completion)
    updateAction.addModification(mcaId, U2ModType::mcaUpdatedRowInfo, modDetails, os);
    SAFE_POINT_OP(os, );
}

void SQLiteMcaDbi::updateGapModel(ModificationAction &updateAction, const U2DataId &mcaId, qint64 mcaRowId, const U2DataId &relatedObjectId, const U2MsaRowGapModel &gapModel, U2OpStatus &os) {
    // TODO: check the function
    QByteArray gapsDetails;
    if (TrackOnUpdate == updateAction.getTrackModType()) {
        U2McaRow row = getRow(mcaId, mcaRowId, os);
        SAFE_POINT_OP(os, );
        gapsDetails = PackUtils::packGapDetails(mcaRowId, relatedObjectId, row.gaps, gapModel);
    }

    updateGapModelCore(mcaId, mcaRowId, relatedObjectId, gapModel, os);
    SAFE_POINT_OP(os, );

    qint64 len = 0;
    foreach(const U2MsaGap &gap, gapModel) {
        len += gap.gap;
    }
    len += getRowSequenceLength(mcaId, mcaRowId, os);
    SAFE_POINT_OP(os, );
    if (len > getMcaLength(mcaId, os)) {
        updateMcaLength(updateAction, mcaId, len, os);
    }
    SAFE_POINT_OP(os, );

    // Track the modification, if required; add the object to the list (versions of the objects will be incremented on the updateAction completion)
    updateAction.addModification(mcaId, U2ModType::mcaUpdatedGapModel, gapsDetails, os);
    SAFE_POINT_OP(os, );
}

void SQLiteMcaDbi::updateMcaLength(ModificationAction &updateAction, const U2DataId &mcaId, qint64 length, U2OpStatus &os) {
    QByteArray modDetails;
    if (TrackOnUpdate == updateAction.getTrackModType()) {
        qint64 oldMcaLen = getMcaLength(mcaId, os);
        CHECK_OP(os, );
        modDetails = PackUtils::packAlignmentLength(oldMcaLen, length);
    }

    updateMcaLengthCore(mcaId, length, os);
    SAFE_POINT_OP(os, )

    updateAction.addModification(mcaId, U2ModType::mcaLengthChanged, modDetails, os);
    SAFE_POINT_OP(os, );
}

}   // namespace U2
