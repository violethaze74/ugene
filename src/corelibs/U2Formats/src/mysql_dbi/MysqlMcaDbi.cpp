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
#include <U2Core/DNASequence.h>
#include <U2Core/McaRowInnerData.h>
#include <U2Core/RawDataUdrSchema.h>
#include <U2Core/U2DbiPackUtils.h>
#include <U2Core/U2SafePoints.h>

#include "MysqlMcaDbi.h"
#include "MysqlSequenceDbi.h"
#include "util/MysqlDbiUtils.h"
#include "util/MysqlHelpers.h"
#include "util/MysqlModificationAction.h"

namespace U2 {

MysqlMcaDbi::MysqlMcaDbi(MysqlDbi *dbi)
    : U2McaDbi(dbi),
      MysqlChildDbiCommon(dbi)
{

}

void MysqlMcaDbi::initSqlSchema(U2OpStatus &os) {
    CHECK_OP(os, );

    // MCA object
    U2SqlQuery("CREATE TABLE Mca (object INTEGER PRIMARY KEY, length INTEGER NOT NULL, "
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
    U2SqlQuery("CREATE TABLE McaRow (mca INTEGER NOT NULL, rowId INTEGER NOT NULL, chromatogram INTEGER NOT NULL,"
               " predictedSequence INTEGER NOT NULL, editedSequence INTEGER NOT NULL,"
               " pos INTEGER NOT NULL, workingAreaStart INTEGER NOT NULL, workingAreaEnd INTEGER NOT NULL, length INTEGER NOT NULL,"
               " PRIMARY KEY(mca, rowId),"
               " FOREIGN KEY(mca) REFERENCES Mca(object) ON DELETE CASCADE, "
               " FOREIGN KEY(chromatogram) REFERENCES Object(id) ON DELETE CASCADE, "
               " FOREIGN KEY(predictedSequence) REFERENCES Sequence(object) ON DELETE CASCADE, "
               " FOREIGN KEY(editedSequence) REFERENCES Sequence(object) ON DELETE CASCADE)", db, os).execute();

    U2SqlQuery("CREATE INDEX McaRow_mca_rowId ON McaRow(mca, rowId)", db, os).execute();
    U2SqlQuery("CREATE INDEX McaRow_length ON McaRow(length)", db, os).execute();
    U2SqlQuery("CREATE INDEX McaRow_chromatogram_predictedSequence_editedSequence ON McaRow(chromatogram, predictedSequence, editedSequence)", db, os).execute();
    U2SqlQuery("CREATE INDEX McaRow_chromatogram ON McaRow(chromatogram)", db, os).execute();
    U2SqlQuery("CREATE INDEX McaRow_predictedSequence ON McaRow(predictedSequence)", db, os).execute();
    U2SqlQuery("CREATE INDEX McaRow_editedSequence ON McaRow(editedSequence)", db, os).execute();

     // Gap info for a MCA row:
     //   mca       - mca object id
     //   rowId     - id of the row in the mca
     //   relatedObjectId - id of the object in the mca row this gap has a relation to
     //   gapStart  - start of the gap, the coordinate is relative to the predictedSequence start
     //   gapEnd    - end of the gap, the coordinate is relative to the predictedSequence start (non-inclusive)
    U2SqlQuery("CREATE TABLE McaRowGap (mca INTEGER NOT NULL, rowId INTEGER NOT NULL, relatedObjectId INTEGER NOT NULL, "
               "gapStart INTEGER NOT NULL, gapEnd INTEGER NOT NULL, "
               "FOREIGN KEY(mca, rowId) REFERENCES McaRow(mca, rowId) ON DELETE CASCADE, "
               "FOREIGN KEY(relatedObjectId) REFERENCES Object(id) ON DELETE CASCADE)",
               db, os).execute();

    U2SqlQuery("CREATE INDEX McaRowGap_mca_rowId_relatedObjectId ON McaRowGap(mca, rowId, relatedObjectId)", db, os).execute();
    U2SqlQuery("CREATE INDEX McaRowGap_mca_rowId ON McaRowGap(mca, rowId)", db, os).execute();
}

U2Mca MysqlMcaDbi::getMcaObject(const U2DataId &mcaId, U2OpStatus &os) {
    U2Mca res;
    dbi->getMysqlObjectDbi()->getObject(res, mcaId, os);
    SAFE_POINT_OP(os, res);

    U2SqlQuery query("SELECT length, alphabet FROM Mca WHERE object = :object", db, os);
    query.bindDataId(":object", mcaId);
    if (query.step()) {
        res.length = query.getInt64(0);
        res.alphabet = query.getString(1);
        query.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca object not found"));
    }
    return res;
}

qint64 MysqlMcaDbi::getNumOfRows(const U2DataId &mcaId, U2OpStatus &os) {
    qint64 res = 0;
    U2SqlQuery q("SELECT numOfRows FROM Mca WHERE object = :object", db, os);
    CHECK_OP(os, res);

    q.bindDataId(":object", mcaId);
    if (q.step()) {
        res = q.getInt64(0);
        q.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca object not found"));
    }
    return res;
}

QList<U2McaRow> MysqlMcaDbi::getRows(const U2DataId &mcaId, U2OpStatus &os) {
    QList<U2McaRow> res;
    U2SqlQuery query("SELECT rowId, chromatogram, predictedSequence, editedSequence, workingAreaStart, workingAreaEnd, length FROM McaRow WHERE mca = :mca ORDER BY pos", db, os);
    SAFE_POINT_OP(os, res);
    query.bindDataId(":mca", mcaId);

    static const QString gapQueryString = "SELECT gapStart, gapEnd FROM McaRowGap WHERE mca = :mca AND rowId = :rowId AND relatedObjectId = :relatedObjectId ORDER BY gapStart";
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
        U2SqlQuery predictedSequenceGapQuery(gapQueryString, db, os);
        SAFE_POINT_OP(os, res);
        predictedSequenceGapQuery.bindDataId(":mca", mcaId);
        predictedSequenceGapQuery.bindInt64(":rowId", row.rowId);
        predictedSequenceGapQuery.bindInt64(":relatedObjectId", U2DbiUtils::toDbiId(row.predictedSequenceId));
        while (predictedSequenceGapQuery.step()) {
            U2MsaGap gap;
            gap.offset = predictedSequenceGapQuery.getInt64(0);
            gap.gap = predictedSequenceGapQuery.getInt64(1) - gap.offset;
            SAFE_POINT_EXT(gap.isValid(), os.setError("An invalid gap is stored in the database"), res);
            row.predictedSequenceGaps.append(gap);
        }

        // editedSequence gap model
        U2SqlQuery edtiedSequenceGapQuery(gapQueryString, db, os);
        SAFE_POINT_OP(os, res);
        edtiedSequenceGapQuery.bindDataId(":mca", mcaId);
        edtiedSequenceGapQuery.bindInt64(":rowId", row.rowId);
        edtiedSequenceGapQuery.bindInt64(":relatedObjectId", U2DbiUtils::toDbiId(row.sequenceId));
        while (edtiedSequenceGapQuery.step()) {
            U2MsaGap gap;
            gap.offset = edtiedSequenceGapQuery.getInt64(0);
            gap.gap = edtiedSequenceGapQuery.getInt64(1) - gap.offset;
            SAFE_POINT_EXT(gap.isValid(), os.setError("An invalid gap is stored in the database"), res);
            row.gaps.append(gap);
        }

        SAFE_POINT_OP(os, res);
        res.append(row);
    }
    return res;
}

U2McaRow MysqlMcaDbi::getRow(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    U2McaRow res;
    U2SqlQuery query("SELECT chromatogram, predictedSequence, editedSequence, workingAreaStart, workingAreaEnd, length FROM McaRow WHERE mca = :mca AND rowId = :rowId", db, os);
    SAFE_POINT_OP(os, res);

    query.bindDataId(":mca", mcaId);
    query.bindInt64(":rowId", rowId);
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

    // predictedSequence gap model
    U2SqlQuery predictedSequenceGapQuery("SELECT gapStart, gapEnd FROM McaRowGap WHERE mca = :mca AND rowId = :rowId AND relatedObjectId = :relatedObjectId ORDER BY gapStart", db, os);
    SAFE_POINT_OP(os, res);
    predictedSequenceGapQuery.bindDataId(":mca", mcaId);
    predictedSequenceGapQuery.bindInt64(":rowId", rowId);
    predictedSequenceGapQuery.bindInt64(":relatedObjectId", U2DbiUtils::toDbiId(res.predictedSequenceId));
    while (predictedSequenceGapQuery.step()) {
        U2MsaGap gap;
        gap.offset = predictedSequenceGapQuery.getInt64(0);
        gap.gap = predictedSequenceGapQuery.getInt64(1) - gap.offset;
        res.predictedSequenceGaps.append(gap);
    }

    // editedSequence gap model
    U2SqlQuery editedSequenceGapQuery("SELECT gapStart, gapEnd FROM McaRowGap WHERE mca = :mca AND rowId = :rowId AND relatedObjectId = :relatedObjectId ORDER BY gapStart", db, os);
    SAFE_POINT_OP(os, res);
    editedSequenceGapQuery.bindDataId(":mca", mcaId);
    editedSequenceGapQuery.bindInt64(":rowId", rowId);
    editedSequenceGapQuery.bindInt64(":relatedObjectId", U2DbiUtils::toDbiId(res.sequenceId));
    while (editedSequenceGapQuery.step()) {
        U2MsaGap gap;
        gap.offset = editedSequenceGapQuery.getInt64(0);
        gap.gap = editedSequenceGapQuery.getInt64(1) - gap.offset;
        res.gaps.append(gap);
    }

    return res;
}

QList<qint64> MysqlMcaDbi::getRowsOrder(const U2DataId &mcaId, U2OpStatus &os) {
    QList<qint64> res;
    U2SqlQuery query("SELECT rowId FROM McaRow WHERE mca = :mca ORDER BY pos", db, os);
    query.bindDataId(":mca", mcaId);
    qint64 rowId = U2McaRow::INVALID_ROW_ID;
    while (query.step()) {
        rowId = query.getInt64(0);
        res.append(rowId);
    }
    return res;
}

U2AlphabetId MysqlMcaDbi::getMcaAlphabet(const U2DataId &mcaId, U2OpStatus &os) {
    QString alphabetName;
    U2SqlQuery query("SELECT alphabet FROM Mca WHERE object = :object", db, os);
    query.bindDataId(":object", mcaId);
    if (query.step()) {
        alphabetName = query.getString(0);
        query.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca object not found"));
    }

    return U2AlphabetId(alphabetName);
}

qint64 MysqlMcaDbi::getMcaLength(const U2DataId &mcaId, U2OpStatus &os) {
    qint64 res = 0;
    U2SqlQuery query("SELECT length FROM Mca WHERE object = :object", db, os);
    CHECK_OP(os, res);

    query.bindDataId(":object", mcaId);
    if (query.step()) {
        res = query.getInt64(0);
        query.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca object not found"));
    }

    return res;
}

U2DataId MysqlMcaDbi::createMcaObject(const QString &folder, const QString &name, const U2AlphabetId &alphabet, U2OpStatus &os) {
    return createMcaObject(folder, name, alphabet, 0, os);
}

U2DataId MysqlMcaDbi::createMcaObject(const QString &folder, const QString &name, const U2AlphabetId &alphabet, int length, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    U2Mca mca;
    mca.visualName = name;
    mca.alphabet = alphabet;
    mca.length = length;

    // Create the object
    dbi->getMysqlObjectDbi()->createObject(mca, folder, U2DbiObjectRank_TopLevel, os);
    CHECK_OP(os, U2DataId());

    // Create a record in the Mca table
    U2SqlQuery q("INSERT INTO Mca(object, length, alphabet, numOfRows) VALUES(:mca, :length, :alphabet, :numOfRows)", db, os);
    CHECK_OP(os, U2DataId());

    q.bindDataId(":mca", mca.id);
    q.bindInt64(":length", mca.length);
    q.bindString(":alphabet", mca.alphabet.id);
    q.bindInt64(":numOfRows", 0); // no rows
    q.insert();

    return mca.id;
}

void MysqlMcaDbi::updateMcaName(const U2DataId &mcaId, const QString &name, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    U2Object mcaObj;
    dbi->getMysqlObjectDbi()->getObject(mcaObj, mcaId, os);
    CHECK_OP(os, );

    MysqlDbiUtils::renameObject(dbi, mcaObj, name, os);
}

void MysqlMcaDbi::updateMcaAlphabet(const U2DataId &mcaId, const U2AlphabetId &alphabet, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    MysqlModificationAction updateAction(dbi, mcaId);
    U2TrackModType trackMod = updateAction.prepare(os);
    CHECK_OP(os, );

    // Get modDetails, if required
    QByteArray modDetails;
    if (TrackOnUpdate == trackMod) {
        U2Mca mcaObj = getMcaObject(mcaId, os);
        CHECK_OP(os, );
        modDetails = PackUtils::packAlphabetDetails(mcaObj.alphabet, alphabet);
    }

    // Update the alphabet
    U2SqlQuery query("UPDATE Mca SET alphabet = :alphabet WHERE object = :object", db, os);
    CHECK_OP(os, );

    query.bindString(":alphabet", alphabet.id);
    query.bindDataId(":object", mcaId);
    query.update();

    // Increment version; track the modification, if required
    updateAction.addModification(mcaId, U2ModType::mcaUpdatedAlphabet, modDetails, os);
    SAFE_POINT_OP(os, );

    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void MysqlMcaDbi::addRows(const U2DataId &mcaId, QList<U2McaRow> &rows, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    MysqlModificationAction updateAction(dbi, mcaId);
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

void MysqlMcaDbi::addRow(const U2DataId &mcaId, qint64 posInMca, U2McaRow &row, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    MysqlModificationAction updateAction(dbi, mcaId);
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

void MysqlMcaDbi::removeRows(const U2DataId &mcaId, const QList<qint64> &rowIds, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    MysqlModificationAction updateAction(dbi, mcaId);
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

void MysqlMcaDbi::removeRow(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    MysqlModificationAction updateAction(dbi, mcaId);
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

void MysqlMcaDbi::deleteRowsData(const U2DataId &mcaId, U2OpStatus &os) {
    static const QString deleteObjectString = "DELETE FROM Object WHERE id IN (SELECT chromatogram, predictedSequence, editedSequence FROM McaRow WHERE mca = :mca)";
    U2SqlQuery deleteObjectQeury(deleteObjectString, db, os);
    deleteObjectQeury.bindDataId(":mca", mcaId);
    deleteObjectQeury.execute();
}

void MysqlMcaDbi::updateRowName(const U2DataId &mcaId, qint64 rowId, const QString &newName, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    MysqlModificationAction updateAction(dbi, mcaId);
    updateAction.prepare(os);
    SAFE_POINT_OP(os, );

    const U2DataId editedSequenceId = getEditedSequenceIdByRowId(mcaId, rowId, os);
    SAFE_POINT_OP(os, );

    U2Sequence seqObject = dbi->getSequenceDbi()->getSequenceObject(editedSequenceId, os);
    SAFE_POINT_OP(os, );

    MysqlDbiUtils::renameObject(updateAction, dbi, seqObject, newName, os);
    SAFE_POINT_OP(os, );

    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void MysqlMcaDbi::updateRowContent(const U2DataId &mcaId,
                                    qint64 rowId,
                                    const DNAChromatogram &chromatogram,
                                    const DNASequence &predictedSequence,
                                    const U2MsaRowGapModel &predictedSequenceGapModel,
                                    const DNASequence &editedSequence,
                                    const U2MsaRowGapModel &editedSequenceGapModel,
                                    const U2Region &workingArea,
                                    U2OpStatus &os) {
    SAFE_POINT_EXT(chromatogram.traceLength == predictedSequence.length(), os.setError("Chromatogram length doesn't equal predicted sequence length"), );
    SAFE_POINT_EXT(calculateRowLength(predictedSequence.length(), predictedSequenceGapModel) == calculateRowLength(editedSequence.length(), editedSequenceGapModel),
                   os.setError("Predicted sequence length doesn't equal edited sequence length"), );

    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    MysqlModificationAction updateAction(dbi, mcaId);
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
    dbi->getMysqlSequenceDbi()->updateSequenceData(updateAction, row.predictedSequenceId, U2_REGION_MAX, predictedSequence.seq, hints, os);
    SAFE_POINT_OP(os, );

    // Update the edited sequence data
    dbi->getMysqlSequenceDbi()->updateSequenceData(updateAction, row.sequenceId, U2_REGION_MAX, editedSequence.seq, hints, os);
    SAFE_POINT_OP(os, );

    // Update the row object
    U2McaRow newRow(row);
    newRow.length = calculateRowLength(predictedSequence.length(), predictedSequenceGapModel);
    const U2Region boundedWorkingArea = workingArea.intersect(U2Region(0, newRow.length));
    newRow.gstart = boundedWorkingArea.startPos;
    newRow.gend = boundedWorkingArea.endPos();
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

void MysqlMcaDbi::updateRowContent(const U2DataId &mcaId, qint64 rowId, const McaRowMemoryData &rowMemoryData, U2OpStatus &os) {
    updateRowContent(mcaId,
                     rowId,
                     rowMemoryData.chromatogram,
                     rowMemoryData.predictedSequence,
                     rowMemoryData.predictedSequenceGapModel,
                     rowMemoryData.editedSequence,
                     rowMemoryData.editedSequenceGapModel,
                     rowMemoryData.workingArea,
                     os);
}

void MysqlMcaDbi::updateRowChromatogram(const U2DataId &mcaId, qint64 rowId, const DNAChromatogram &chromatogram, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    MysqlModificationAction updateAction(dbi, mcaId);
    U2TrackModType trackMod = updateAction.prepare(os);
    SAFE_POINT_OP(os, );
    Q_UNUSED(trackMod);

    // Get the row object
    U2McaRow row = getRow(mcaId, rowId, os);
    SAFE_POINT_OP(os, );
    SAFE_POINT_EXT(calculateRowLength(chromatogram.traceLength, row.predictedSequenceGaps) == row.length,
                   os.setError("The chromatogram length differs from the row length, can't set the chromatogram wihtout loosing the row consistency"), );

    // Update the chromatogram data
    // TODO: make chromatogram undoable
    const QByteArray serializedChromatogram = DNAChromatogramSerializer::serialize(chromatogram);
    RawDataUdrSchema::writeContent(serializedChromatogram, U2EntityRef(dbi->getDbiRef(), row.chromatogramId), os);
    SAFE_POINT_OP(os, );

    // Save tracks, if required; increment versions
    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void MysqlMcaDbi::updateRowSequence(const U2DataId &mcaId, qint64 rowId, const U2DataId &sequenceId, const QByteArray &sequenceData, const U2MsaRowGapModel &gapModel, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    MysqlModificationAction updateAction(dbi, mcaId);
    U2TrackModType trackMod = updateAction.prepare(os);
    SAFE_POINT_OP(os, );
    Q_UNUSED(trackMod);

    // Get the row object
    U2McaRow row = getRow(mcaId, rowId, os);
    SAFE_POINT_OP(os, );
    SAFE_POINT_EXT(row.predictedSequenceId == sequenceId || row.sequenceId == sequenceId,
                   os.setError("The sequnece doesn't has any relation to the row"), );
    SAFE_POINT_EXT(calculateRowLength(sequenceData.length(), gapModel) == row.length,
                   os.setError("The sequence length differs from the row length, can't set the sequence wihtout loosing the row consistency"), );

    // Update the sequence data
    QVariantMap hints;
    dbi->getMysqlSequenceDbi()->updateSequenceData(updateAction, sequenceId, U2_REGION_MAX, sequenceData, hints, os);
    SAFE_POINT_OP(os, );

    // Update the gap model
    // WARNING: this update must go after the row info update to recalculate the msa length properly
    updateGapModel(updateAction, mcaId, rowId, sequenceId, gapModel, os);
    SAFE_POINT_OP(os, );

    // Save tracks, if required; increment versions
    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void MysqlMcaDbi::updateGapModel(const U2DataId &mcaId, qint64 mcaRowId, const U2DataId &gapModelOwner, const U2MsaRowGapModel &gapModel, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    MysqlModificationAction updateAction(dbi, mcaId);
    updateAction.prepare(os);
    SAFE_POINT_OP(os, );

    updateGapModel(updateAction, mcaId, mcaRowId, gapModelOwner, gapModel, os);
    SAFE_POINT_OP(os, );

    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void MysqlMcaDbi::updateMcaLength(const U2DataId &mcaId, qint64 length, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    MysqlModificationAction updateAction(dbi, mcaId);
    updateAction.prepare(os);
    SAFE_POINT_OP(os, );

    updateMcaLength(updateAction, mcaId, length, os);

    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void MysqlMcaDbi::setNewRowsOrder(const U2DataId &mcaId, const QList<qint64> &rowIds, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    MysqlModificationAction updateAction(dbi, mcaId);
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

void MysqlMcaDbi::undo(const U2DataId &mcaId, qint64 modType, const QByteArray &modDetails, U2OpStatus &os) {
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

void MysqlMcaDbi::redo(const U2DataId &mcaId, qint64 modType, const QByteArray &modDetails, U2OpStatus &os) {
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

void MysqlMcaDbi::addMcaRowAndGaps(const U2DataId &mcaId, qint64 posInMca, U2McaRow &row, U2OpStatus &os) {
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

    dbi->getMysqlObjectDbi()->setParent(mcaId, row.chromatogramId, os);
    dbi->getMysqlObjectDbi()->setParent(mcaId, row.predictedSequenceId, os);
    dbi->getMysqlObjectDbi()->setParent(mcaId, row.sequenceId, os);
}

void MysqlMcaDbi::createMcaRow(const U2DataId &mcaId, qint64 posInMca, U2McaRow &mcaRow, U2OpStatus &os) {
    assert(posInMca >= 0);

    // Calculate the row length
    const qint64 predictedSequenceLength = getRowSequenceLength(mcaRow.predictedSequenceId, os);
    CHECK_OP(os, );
    const qint64 editedSequenceLength = getRowSequenceLength(mcaRow.sequenceId, os);
    CHECK_OP(os, );
    const qint64 chromatogramLength = getRowChromatogramLength(mcaRow.chromatogramId, os);
    CHECK_OP(os, );

    const qint64 predictedSequenceGappedLength = calculateRowLength(predictedSequenceLength, mcaRow.predictedSequenceGaps);
    SAFE_POINT_EXT(chromatogramLength == predictedSequenceLength,
                   os.setError("The chromatogram length differs from the predicted sequence length"), );
    SAFE_POINT_EXT(predictedSequenceGappedLength == calculateRowLength(editedSequenceLength, mcaRow.gaps),
                   os.setError("The row sequences have different lengths"), );

    mcaRow.length = predictedSequenceGappedLength;

    // Insert the data
    U2SqlQuery query("INSERT INTO McaRow(mca, rowId, chromatogram, predictedSequence, editedSequence, pos, workingAreaStart, workingAreaEnd, length)"
        " VALUES(:mca, :rowId, :chromatogram, :predictedSequence, :editedSequence, :pos, :workingAreaStart, :workingAreaEnd, :length)", db, os);
    CHECK_OP(os, );

    query.bindDataId(":mca", mcaId);
    query.bindInt64(":rowId", mcaRow.rowId);
    query.bindDataId(":chromatogram", mcaRow.chromatogramId);
    query.bindDataId(":predictedSequence", mcaRow.predictedSequenceId);
    query.bindDataId(":editedSequence", mcaRow.sequenceId);
    query.bindInt64(":pos", posInMca);
    query.bindInt64(":workingAreaStart", mcaRow.gstart);
    query.bindInt64(":workingAreaEnd", mcaRow.gend);
    query.bindInt64(":length", mcaRow.length);
    query.insert();
}

void MysqlMcaDbi::createMcaRowGap(const U2DataId &mcaId, qint64 mcaRowId, const U2DataId &relatedObjectId, const U2MsaGap &mcaGap, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    static const QString queryString("INSERT INTO McaRowGap(mca, rowId, relatedObjectId, gapStart, gapEnd) VALUES(:mca, :rowId, :relatedObjectId, :gapStart, :gapEnd)");
    U2SqlQuery query(queryString, db, os);
    CHECK_OP(os, );

    query.bindDataId(":mca", mcaId);
    query.bindInt64(":rowId", mcaRowId);
    query.bindInt64(":relatedObjectId", U2DbiUtils::toDbiId(relatedObjectId));
    query.bindInt64(":gapStart", mcaGap.offset);
    query.bindInt64(":gapEnd", mcaGap.offset + mcaGap.gap);
    query.insert();
}

void MysqlMcaDbi::removeMcaRowAndGaps(const U2DataId &mcaId, qint64 rowId, bool removeChilds, U2OpStatus &os) {
    const U2DataId chromatogramId = getChromatogramIdByRowId(mcaId, rowId, os);
    const U2DataId predictedSequenceId = getPredictedSequenceIdByRowId(mcaId, rowId, os);
    const U2DataId editedSequenceId = getEditedSequenceIdByRowId(mcaId, rowId, os);
    CHECK_OP(os, );

    removeRecordsFromMcaRowGap(mcaId, rowId, os);
    removeRecordFromMcaRow(mcaId, rowId, os);

    dbi->getMysqlObjectDbi()->removeParent(mcaId, chromatogramId, removeChilds, os);
    dbi->getMysqlObjectDbi()->removeParent(mcaId, predictedSequenceId, removeChilds, os);
    dbi->getMysqlObjectDbi()->removeParent(mcaId, editedSequenceId, removeChilds, os);
}

void MysqlMcaDbi::removeRecordsFromMcaRowGap(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    static const QString queryString("DELETE FROM McaRowGap WHERE mca = :mca AND rowId = :rowId");
    U2SqlQuery query(queryString, db, os);
    CHECK_OP(os, );

    query.bindDataId(":mca", mcaId);
    query.bindInt64(":rowId", rowId);
    query.update();
}

void MysqlMcaDbi::removeRecordFromMcaRow(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    static const QString queryString("DELETE FROM McaRow WHERE mca = :mca AND rowId = :rowId");
    U2SqlQuery query(queryString, db, os);
    CHECK_OP(os, );

    query.bindDataId(":mca", mcaId);
    query.bindInt64(":rowId", rowId);
    query.update();
}

void MysqlMcaDbi::updateNumOfRows(const U2DataId &mcaId, qint64 numOfRows, U2OpStatus &os) {
    U2SqlQuery query("UPDATE Mca SET numOfRows = :numOfRows WHERE object = :object", db, os);
    SAFE_POINT_OP(os, );

    query.bindInt64(":numOfRows", numOfRows);
    query.bindDataId(":object", mcaId);
    query.update();
}

void MysqlMcaDbi::recalculateRowsPositions(const U2DataId &mcaId, U2OpStatus &os) {
    QList<U2McaRow> rows = getRows(mcaId, os);
    CHECK_OP(os, );

    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    static const QString queryString = "UPDATE McaRow SET pos = :pos WHERE mca = :mca AND rowId = :rowId";

    for (int i = 0, n = rows.count(); i < n; ++i) {
        qint64 rowId = rows[i].rowId;
        U2SqlQuery query(queryString, db, os);
        SAFE_POINT_OP(os, );
        query.bindInt64(":pos", i);
        query.bindDataId(":mca", mcaId);
        query.bindInt64(":rowId", rowId);
        query.execute();
    }
}

qint64 MysqlMcaDbi::calculateRowLength(qint64 dataLength, const QList<U2MsaGap> &gaps) {
    qint64 rowLength = dataLength;
    foreach (const U2MsaGap &gap, gaps) {
        if (gap.offset <= rowLength) {
            rowLength += gap.gap;
        }
    }
    return rowLength;
}

qint64 MysqlMcaDbi::getRowDataLength(const U2DataId &mcaId, qint64 rowId, const U2DataId &childObjectId, U2OpStatus &os) {
    const U2McaRow row = getRow(mcaId, rowId, os);
    SAFE_POINT_OP(os, -1);
    SAFE_POINT_EXT(row.predictedSequenceId == childObjectId || row.sequenceId == childObjectId,
                   os.setError("The sequnece doesn't has any relation to the row"), -1);

    return getRowSequenceLength(childObjectId, os);
}

qint64 MysqlMcaDbi::getRowSequenceLength(const U2DataId &childObjectId, U2OpStatus &os) {
    SAFE_POINT_EXT(U2Type::Sequence == U2DbiUtils::toType(childObjectId),
                   os.setError(QString("Unexpected child object type: expect '%1' (sequence), got '%2'")
                   .arg(QString::number(U2Type::Sequence).arg(U2DbiUtils::toType(childObjectId)))), -1);

    const U2Sequence dbSequence = dbi->getSequenceDbi()->getSequenceObject(childObjectId, os);
    SAFE_POINT_OP(os, -1);

    return dbSequence.length;
}

qint64 MysqlMcaDbi::getRowChromatogramLength(const U2DataId &childObjectId, U2OpStatus &os) {
    SAFE_POINT_EXT(U2Type::Chromatogram == U2DbiUtils::toType(childObjectId),
                   os.setError(QString("Unexpected child object type: expect '%1' (chromatogram), got '%2'")
                   .arg(QString::number(U2Type::Chromatogram).arg(U2DbiUtils::toType(childObjectId)))), -1);

    return ChromatogramUtils::getChromatogramLength(os, U2EntityRef(dbi->getDbiRef(), childObjectId));
}

void MysqlMcaDbi::updateRowLength(const U2DataId &mcaId, qint64 rowId, qint64 newLength, U2OpStatus &os) {
    U2SqlQuery query("UPDATE McaRow SET length = :length WHERE mca = :mca AND rowId = :rowId", db, os);
    CHECK_OP(os, );

    query.bindInt64(":length", newLength);
    query.bindDataId(":mca", mcaId);
    query.bindInt64(":rowId", rowId);
    query.update();
}

U2DataId MysqlMcaDbi::getChromatogramIdByRowId(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    U2DataId res;
    U2SqlQuery query("SELECT chromatogram FROM McaRow WHERE mca = :mca AND rowId = :rowId", db, os);
    CHECK_OP(os, res);

    query.bindDataId(":mca", mcaId);
    query.bindInt64(":rowId", rowId);
    if (query.step()) {
        res = query.getDataId(0, U2Type::Chromatogram);
        query.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca row not found"));
    }

    return res;
}

U2DataId MysqlMcaDbi::getPredictedSequenceIdByRowId(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    U2DataId res;
    U2SqlQuery query("SELECT predictedSequence FROM McaRow WHERE mca = :mca AND rowId = :rowId", db, os);
    CHECK_OP(os, res);

    query.bindDataId(":mca", mcaId);
    query.bindInt64(":rowId", rowId);
    if (query.step()) {
        res = query.getDataId(0, U2Type::Sequence);
        query.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca row not found"));
    }

    return res;
}

U2DataId MysqlMcaDbi::getEditedSequenceIdByRowId(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    U2DataId res;
    U2SqlQuery query("SELECT editedSequence FROM McaRow WHERE mca = :mca AND rowId = :rowId", db, os);
    CHECK_OP(os, res);

    query.bindDataId(":mca", mcaId);
    query.bindInt64(":rowId", rowId);
    if (query.step()) {
        res = query.getDataId(0, U2Type::Sequence);
        query.ensureDone();
    } else if (!os.hasError()) {
        os.setError(U2DbiL10n::tr("Mca row not found"));
    }

    return res;
}

qint64 MysqlMcaDbi::getPosInMca(const U2DataId &mcaId, qint64 rowId, U2OpStatus &os) {
    U2SqlQuery query("SELECT pos FROM McaRow WHERE mca = :mca AND rowId = :rowId", db, os);
    CHECK_OP(os, -1);
    query.bindDataId(":mca", mcaId);
    query.bindInt64(":rowId", rowId);
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

qint64 MysqlMcaDbi::getMaximumRowId(const U2DataId &mcaId, U2OpStatus &os) {
    qint64 maxRowId = 0;
    U2SqlQuery query("SELECT MAX(rowId) FROM McaRow WHERE mca = :mca", db, os);
    SAFE_POINT_OP(os, 0);

    query.bindDataId(":mca", mcaId);
    query.getInt64(1);
    if (query.step()) {
        maxRowId = query.getInt64(0);
    }

    return maxRowId;
}

void MysqlMcaDbi::updateGapModelCore(const U2DataId &mcaId, qint64 mcaRowId, const U2DataId &relatedObjectId, const U2MsaRowGapModel &gapModel, U2OpStatus &os) {
    MysqlTransaction t(db, os);
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
    qint64 rowSequenceLength = getRowDataLength(mcaId, mcaRowId, relatedObjectId, os);
    CHECK_OP(os, );

    qint64 newRowLength = calculateRowLength(rowSequenceLength, gapModel);
    updateRowLength(mcaId, mcaRowId, newRowLength, os);
    CHECK_OP(os, );
}

void MysqlMcaDbi::addRowSubcore(const U2DataId &mcaId, qint64 numOfRows, const QList<qint64> &rowsOrder, U2OpStatus &os) {
    // Re-calculate position, if needed
    setNewRowsOrderCore(mcaId, rowsOrder, os);
    CHECK_OP(os, );

    // Update the number of rows of the MCA
    updateNumOfRows(mcaId, numOfRows, os);
}

void MysqlMcaDbi::addRowCore(const U2DataId &mcaId, qint64 posInMca, U2McaRow &row, U2OpStatus &os) {
    MysqlTransaction t(db, os);
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
    if (posInMca != numOfRows) {
        rowsOrder.insert(posInMca, row.rowId);
    }
    addRowSubcore(mcaId, numOfRows + 1, rowsOrder, os);
}

void MysqlMcaDbi::addRowsCore(const U2DataId &mcaId, const QList<qint64> &posInMca, QList<U2McaRow> &rows, U2OpStatus &os) {
    MysqlTransaction t(db, os);
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
        numOfRows++;
        rowsOrder.insert(pos, ri->rowId);
    }

    addRowSubcore(mcaId, numOfRows, rowsOrder, os);
}

void MysqlMcaDbi::removeRowSubcore(const U2DataId &mcaId, qint64 numOfRows, U2OpStatus &os) {
    // Update the number of rows
    updateNumOfRows(mcaId, numOfRows, os);
    CHECK_OP(os, );

    // Re-calculate the rows positions
    recalculateRowsPositions(mcaId, os);
}

void MysqlMcaDbi::removeRowCore(const U2DataId &mcaId, qint64 rowId, bool removeChilds, U2OpStatus &os) {
    MysqlTransaction t(db, os);
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

void MysqlMcaDbi::removeRowsCore(const U2DataId &mcaId, const QList<qint64> &rowIds, bool removeChilds, U2OpStatus &os) {
    MysqlTransaction t(db, os);
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

void MysqlMcaDbi::setNewRowsOrderCore(const U2DataId &mcaId, const QList<qint64> rowIds, U2OpStatus &os) {
    MysqlTransaction t(db, os);
    Q_UNUSED(t);

    static const QString queryString = "UPDATE McaRow SET pos = :pos WHERE mca = :mca AND rowId = :rowId";

    for (int i = 0, n = rowIds.count(); i < n; ++i) {
        qint64 rowId = rowIds[i];
        U2SqlQuery query(queryString, db, os);
        CHECK_OP(os, );
        query.bindInt64(":pos", i);
        query.bindDataId(":mca", mcaId);
        query.bindInt64(":rowId", rowId);
        query.execute();
    }
}

void MysqlMcaDbi::updateRowInfoCore(const U2DataId &mcaId, const U2McaRow &row, U2OpStatus &os) {
    U2SqlQuery query("UPDATE McaRow SET chromatogram = :chromatogram, predictedSequence = :predictedSequence, editedSequence = :editedSequence, "
                     "gstart = :workingAreaStart, gend = :workingAreaEnd WHERE mca = :mca AND rowId = :rowId", db, os);
    SAFE_POINT_OP(os, );

    query.bindDataId(":chromatogram", row.chromatogramId);
    query.bindDataId(":predictedSequence", row.predictedSequenceId);
    query.bindDataId(":editedSequence", row.sequenceId);
    query.bindInt64(":workingAreaStart", row.gstart);
    query.bindInt64(":workingAreaEnd", row.gend);
    query.bindDataId(":mca", mcaId);
    query.bindInt64(":rowId", row.rowId);
    query.update();
}

void MysqlMcaDbi::updateMcaLengthCore(const U2DataId &mcaId, qint64 length, U2OpStatus &os) {
    U2SqlQuery query("UPDATE Mca SET length = :length WHERE object = :object", db, os);
    CHECK_OP(os, );

    query.bindInt64(":length", length);
    query.bindDataId(":object", mcaId);
    query.execute();
}

void MysqlMcaDbi::updateMcaAlphabetCore(const U2DataId &mcaId, const U2AlphabetId &alphabetId, U2OpStatus &os) {
    U2SqlQuery query("UPDATE Mca SET alphabet = :alphabet WHERE object = :object", db, os);
    CHECK_OP(os, );

    query.bindString(":alphabet", alphabetId.id);
    query.bindDataId(":object", mcaId);
    query.update();
}

void MysqlMcaDbi::undoUpdateMcaAlphabet(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2AlphabetId oldAlphabet;
    U2AlphabetId newAlphabet;
    bool ok = PackUtils::unpackAlphabetDetails(modDetails, oldAlphabet, newAlphabet);
    if (!ok) {
        os.setError("An error occurred during updating an alignment alphabet");
        return;
    }

    updateMcaAlphabetCore(mcaId, oldAlphabet, os);
}

void MysqlMcaDbi::undoAddRows(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
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

void MysqlMcaDbi::undoAddRow(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2McaRow row;
    qint64 posInMca = U2McaRow::INVALID_ROW_ID;
    bool ok = PackUtils::unpackRow(modDetails, posInMca, row);
    if (!ok) {
        os.setError("An error occurred during reverting addition of a row");
        return;
    }

    removeRowCore(mcaId, row.rowId, false, os);
}

void MysqlMcaDbi::undoRemoveRows(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    QList<qint64> posInMca;
    QList<U2McaRow> rows;
    bool ok = PackUtils::unpackRows(modDetails, posInMca, rows);
    if (!ok) {
        os.setError("An error occurred during reverting removing of rows");
        return;
    }

    addRowsCore(mcaId, posInMca, rows, os);
}

void MysqlMcaDbi::undoRemoveRow(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2McaRow row;
    qint64 posInMca = U2McaRow::INVALID_ROW_ID;
    bool ok = PackUtils::unpackRow(modDetails, posInMca, row);
    if (!ok) {
        os.setError("An error occurred during reverting removing of a row");
        return;
    }

    addRowCore(mcaId, posInMca, row, os);
}

void MysqlMcaDbi::undoUpdateGapModel(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
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

void MysqlMcaDbi::undoSetNewRowsOrder(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
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

void MysqlMcaDbi::undoUpdateRowInfo(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2McaRow oldRow;
    U2McaRow newRow;
    bool ok = PackUtils::unpackRowInfoDetails(modDetails, oldRow, newRow);
    if (!ok) {
        os.setError("An error occurred during updating a row info");
        return;
    }

    SAFE_POINT(oldRow.rowId == newRow.rowId, "Incorrect rowId", );
    SAFE_POINT(oldRow.chromatogramId == newRow.chromatogramId, "Incorrect chromatogramId", );
    SAFE_POINT(oldRow.predictedSequenceId == newRow.predictedSequenceId, "Incorrect predictedSequenceId", );
    SAFE_POINT(oldRow.sequenceId == newRow.sequenceId, "Incorrect sequenceId", );

    updateRowInfoCore(mcaId, oldRow, os);
}

void MysqlMcaDbi::undoMcaLengthChange(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    qint64 oldLen;
    qint64 newLen;

    bool ok = PackUtils::unpackAlignmentLength(modDetails, oldLen, newLen);
    CHECK_EXT(ok, os.setError(U2DbiL10n::tr("An error occurred during updating an mca length")), );

    updateMcaLengthCore(mcaId, oldLen, os);
}

void MysqlMcaDbi::redoUpdateMcaAlphabet(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2AlphabetId oldAlphabet;
    U2AlphabetId newAlphabet;
    bool ok = PackUtils::unpackAlphabetDetails(modDetails, oldAlphabet, newAlphabet);
    if (!ok) {
        os.setError("An error occurred during updating an alignment alphabet");
        return;
    }

    updateMcaAlphabetCore(mcaId, newAlphabet, os);
}

void MysqlMcaDbi::redoAddRows(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    QList<qint64> posInMca;
    QList<U2McaRow> rows;
    bool ok = PackUtils::unpackRows(modDetails, posInMca, rows);
    if (!ok) {
        os.setError("An error occurred during reverting adding of rows");
        return;
    }

    addRowsCore(mcaId, posInMca, rows, os);
}

void MysqlMcaDbi::redoAddRow(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2McaRow row;
    qint64 posInMca = U2McaRow::INVALID_ROW_ID;
    bool ok = PackUtils::unpackRow(modDetails, posInMca, row);
    if (!ok) {
        os.setError("An error occurred during addition of a row");
        return;
    }

    addRowCore(mcaId, posInMca, row, os);
}

void MysqlMcaDbi::redoRemoveRows(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
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

void MysqlMcaDbi::redoRemoveRow(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2McaRow row;
    qint64 posInMca = U2McaRow::INVALID_ROW_ID;
    bool ok = PackUtils::unpackRow(modDetails, posInMca, row);
    if (!ok) {
        os.setError("An error occurred during reverting removing of a row");
        return;
    }

    removeRowCore(mcaId, row.rowId, false, os);
}

void MysqlMcaDbi::redoUpdateGapModel(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
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

void MysqlMcaDbi::redoSetNewRowsOrder(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    QList<qint64> oldOrder;
    QList<qint64> newOrder;
    bool ok = PackUtils::unpackRowOrderDetails(modDetails, oldOrder, newOrder);
    if (!ok) {
        os.setError("An error occurred during updating an alignment row order");
        return;
    }

    setNewRowsOrderCore(mcaId, newOrder, os);
}

void MysqlMcaDbi::redoUpdateRowInfo(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    U2McaRow oldRow;
    U2McaRow newRow;
    bool ok = PackUtils::unpackRowInfoDetails(modDetails, oldRow, newRow);
    if (!ok) {
        os.setError("An error occurred during updating a row info");
        return;
    }

    SAFE_POINT(oldRow.rowId == newRow.rowId, "Incorrect rowId", );
    SAFE_POINT(oldRow.chromatogramId == newRow.chromatogramId, "Incorrect chromatogramId", );
    SAFE_POINT(oldRow.predictedSequenceId == newRow.predictedSequenceId, "Incorrect predictedSequenceId", );
    SAFE_POINT(oldRow.sequenceId == newRow.sequenceId, "Incorrect sequenceId", );

    updateRowInfoCore(mcaId, newRow, os);
}

void MysqlMcaDbi::redoMcaLengthChange(const U2DataId &mcaId, const QByteArray &modDetails, U2OpStatus &os) {
    qint64 oldLen = 0;
    qint64 newLen = 0;

    bool ok = PackUtils::unpackAlignmentLength(modDetails, oldLen, newLen);
    CHECK_EXT(ok, os.setError(U2DbiL10n::tr("An error occurred during updating an mca length")), );

    updateMcaLengthCore(mcaId, newLen, os);
}

void MysqlMcaDbi::updateRowInfo(MysqlModificationAction &updateAction, const U2DataId &mcaId, const U2McaRow &row, U2OpStatus &os) {
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

void MysqlMcaDbi::updateGapModel(MysqlModificationAction &updateAction, const U2DataId &mcaId, qint64 mcaRowId, const U2DataId &relatedObjectId, const U2MsaRowGapModel &gapModel, U2OpStatus &os) {
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
    len += getRowDataLength(mcaId, mcaRowId, relatedObjectId, os);
    SAFE_POINT_OP(os, );
    if (len > getMcaLength(mcaId, os)) {
        updateMcaLength(updateAction, mcaId, len, os);
    }
    SAFE_POINT_OP(os, );

    // Track the modification, if required; add the object to the list (versions of the objects will be incremented on the updateAction completion)
    updateAction.addModification(mcaId, U2ModType::mcaUpdatedGapModel, gapsDetails, os);
    SAFE_POINT_OP(os, );
}

void MysqlMcaDbi::updateMcaLength(MysqlModificationAction &updateAction, const U2DataId &mcaId, qint64 length, U2OpStatus &os) {
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
