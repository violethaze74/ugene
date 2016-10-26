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
#include <U2Core/RawDataUdrSchema.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2McaDbi.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceDbi.h>

#include "MultipleChromatogramAlignmentExporter.h"

namespace U2 {

MultipleChromatogramAlignment MultipleChromatogramAlignmentExporter::getAlignment(U2OpStatus &os, const U2DbiRef &dbiRef, const U2DataId &mcaId) const {
    SAFE_POINT_EXT(!connection.isOpen(), os.setError("Connection is already opened"), MultipleChromatogramAlignment());
    connection.open(dbiRef, false, os);
    CHECK_OP(os, MultipleChromatogramAlignment());

    // Rows and their child objects
    QList<U2McaRow> rows = exportRows(os, mcaId);
    CHECK_OP(os, MultipleChromatogramAlignment());

    QList<McaRowMemoryData> mcaRowsMemoryData = exportDataOfRows(os, rows);
    CHECK_OP(os, MultipleChromatogramAlignment());
    SAFE_POINT_EXT(rows.count() == mcaRowsMemoryData.count(), os.setError("Different number of rows and sequences"), MultipleChromatogramAlignment());

    MultipleChromatogramAlignment mca;
    for (int i = 0; i < rows.count(); ++i) {
        // TODO: fix after rowDbInfo is added
        mca->addRow(mcaRowsMemoryData[i].chromatogram,
                    mcaRowsMemoryData[i].predictedSequence,
                    mcaRowsMemoryData[i].predictedSequenceGapModel,
                    mcaRowsMemoryData[i].editedSequence,
                    mcaRowsMemoryData[i].editedSequenceGapModel,
                    mcaRowsMemoryData[i].workingArea);
    }

    // Info
    QVariantMap info = exportAlignmentInfo(os, mcaId);
    CHECK_OP(os, MultipleChromatogramAlignment());

    mca->setInfo(info);

    // Alphabet, name and length
    U2Msa dbMca = exportAlignmentObject(os, mcaId);
    CHECK_OP(os, MultipleChromatogramAlignment());

    const DNAAlphabet *alphabet = U2AlphabetUtils::getById(dbMca.alphabet);
    SAFE_POINT_EXT(NULL != alphabet, os.setError(QString("Alphabet with ID '%1' not found").arg(dbMca.alphabet.id)), MultipleChromatogramAlignment());
    mca->setAlphabet(alphabet);
    mca->setName(dbMca.visualName);
    mca->setLength(dbMca.length);

    return mca;
}

QList<McaRowMemoryData> MultipleChromatogramAlignmentExporter::getMcaRowMemoryData(U2OpStatus &os, const U2DbiRef &dbiRef, const U2DataId &mcaId, const QList<qint64> rowIds) const {
    SAFE_POINT_EXT(!connection.isOpen(), os.setError("Connection is already opened"), QList<McaRowMemoryData>());
    connection.open(dbiRef, false, os);
    CHECK_OP(os, QList<McaRowMemoryData>());

    QList<U2McaRow> rows = exportRows(os, mcaId, rowIds);
    CHECK_OP(os, QList<McaRowMemoryData>());

    QList<McaRowMemoryData> rowsData = exportDataOfRows(os, rows);
    CHECK_OP(os, QList<McaRowMemoryData>());
    SAFE_POINT_EXT(rows.count() == rowsData.count(), os.setError("Different number of rows and sequences"), QList<McaRowMemoryData>());

    return rowsData;
}

QList<U2McaRow> MultipleChromatogramAlignmentExporter::exportRows(U2OpStatus &os, const U2DataId &mcaId) const {
    U2McaDbi *mcaDbi = connection.dbi->getMcaDbi();
    SAFE_POINT_EXT(NULL != mcaDbi, os.setError("NULL MCA Dbi during exporting rows of an alignment"), QList<U2McaRow>());
    return mcaDbi->getRows(mcaId, os);
}

QList<U2McaRow> MultipleChromatogramAlignmentExporter::exportRows(U2OpStatus &os, const U2DataId &mcaId, const QList<qint64> rowIds) const {
    U2McaDbi *mcaDbi = connection.dbi->getMcaDbi( );
    SAFE_POINT_EXT(NULL != mcaDbi, os.setError("NULL MCA Dbi during exporting rows of an alignment"), QList<U2McaRow>());
    QList<U2McaRow> result;
    foreach (qint64 rowId, rowIds) {
        result << mcaDbi->getRow(mcaId, rowId, os);
        SAFE_POINT_OP(os, QList<U2McaRow>());
    }
    return result;
}

QList<McaRowMemoryData> MultipleChromatogramAlignmentExporter::exportDataOfRows(U2OpStatus &os, const QList<U2McaRow> &rows) const {
    QList<McaRowMemoryData> mcaRowsMemoryData;
    mcaRowsMemoryData.reserve(rows.count());

    foreach (const U2McaRow &row, rows) {
        McaRowMemoryData mcaRowMemoryData;
        mcaRowMemoryData.chromatogram = exportChromatogram(os, row.chromatogramId);
        CHECK_OP(os, QList<McaRowMemoryData>());

        mcaRowMemoryData.predictedSequence = exportSequence(os, row.predictedSequenceId);
        CHECK_OP(os, QList<McaRowMemoryData>());

        mcaRowMemoryData.editedSequence = exportSequence(os, row.sequenceId);
        CHECK_OP(os, QList<McaRowMemoryData>());

        mcaRowMemoryData.predictedSequenceGapModel = row.predictedSequenceGaps;
        mcaRowMemoryData.editedSequenceGapModel = row.gaps;
        mcaRowMemoryData.workingArea = U2Region(row.gstart, row.gend - row.gstart);
        mcaRowMemoryData.rowLength = row.length;

        mcaRowsMemoryData << mcaRowMemoryData;
    }

    return mcaRowsMemoryData;
}

DNAChromatogram MultipleChromatogramAlignmentExporter::exportChromatogram(U2OpStatus &os, const U2DataId &chromatogramId) const {
    const U2EntityRef entityRef(connection.dbi->getDbiRef(), chromatogramId);
    const QString serializer = RawDataUdrSchema::getObject(entityRef, os).serializer;
    CHECK_OP(os, DNAChromatogram());
    SAFE_POINT_EXT(DNAChromatogramSerializer::ID == serializer, os.setError(QString("Unknown serializer id: %1").arg(serializer)), DNAChromatogram());
    const QByteArray data = RawDataUdrSchema::readAllContent(entityRef, os);
    CHECK_OP(os, DNAChromatogram());
    return DNAChromatogramSerializer::deserialize(data, os);
}

DNASequence MultipleChromatogramAlignmentExporter::exportSequence(U2OpStatus &os, const U2DataId &sequenceId) const {
    U2SequenceDbi *sequenceDbi = connection.dbi->getSequenceDbi();
    SAFE_POINT_EXT(NULL != sequenceDbi, os.setError("NULL Sequence Dbi during exporting rows sequences"), DNASequence());

    QByteArray sequenceData = sequenceDbi->getSequenceData(sequenceId, U2_REGION_MAX, os);
    CHECK_OP(os, DNASequence());

    U2Sequence dbSequence = sequenceDbi->getSequenceObject(sequenceId, os);
    CHECK_OP(os, DNASequence());

    return DNASequence(dbSequence.visualName, sequenceData);
}

QVariantMap MultipleChromatogramAlignmentExporter::exportAlignmentInfo(U2OpStatus &os, const U2DataId &mcaId) const {
    U2AttributeDbi *attributeDbi = connection.dbi->getAttributeDbi();
    SAFE_POINT_EXT(NULL != attributeDbi, os.setError("NULL Attribute Dbi during exporting an alignment info"), QVariantMap());

    QVariantMap info;
    QList<U2DataId> attributeIds = attributeDbi->getObjectAttributes(mcaId, "", os);
    CHECK_OP(os, QVariantMap());

    foreach (const U2DataId &attributeId, attributeIds) {
        const U2StringAttribute attr = attributeDbi->getStringAttribute(attributeId, os);
        CHECK_OP(os, QVariantMap());
        info.insert(attr.name, attr.value);
    }

    return info;
}

U2Msa MultipleChromatogramAlignmentExporter::exportAlignmentObject(U2OpStatus &os, const U2DataId &mcaId) const {
    U2McaDbi *mcaDbi = connection.dbi->getMcaDbi();
    SAFE_POINT_EXT(NULL != mcaDbi, os.setError("NULL MCA Dbi during exporting an alignment object"), U2Msa());
    return mcaDbi->getMcaObject(mcaId, os);
}

}   // namespace U2
