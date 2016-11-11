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

#include <U2Core/DbiConnection.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GHints.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/McaDbiUtils.h>
#include <U2Core/MultipleChromatogramAlignmentExporter.h>
#include <U2Core/MultipleChromatogramAlignmentImporter.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "MultipleChromatogramAlignmentObject.h"

namespace U2 {

MultipleChromatogramAlignmentObject::MultipleChromatogramAlignmentObject(const QString &name,
                                                                         const U2EntityRef &mcaRef,
                                                                         const QVariantMap &hintsMap,
                                                                         const MultipleChromatogramAlignment &mca)
    : MultipleAlignmentObject(GObjectTypes::MULTIPLE_CHROMATOGRAM_ALIGNMENT, name, mcaRef, hintsMap, mca)
{

}

GObject * MultipleChromatogramAlignmentObject::clone(const U2DbiRef &dstDbiRef, U2OpStatus &os, const QVariantMap &hints) const {
    DbiOperationsBlock opBlock(dstDbiRef, os);
    Q_UNUSED(opBlock);
    CHECK_OP(os, NULL);

    QScopedPointer<GHintsDefaultImpl> gHints(new GHintsDefaultImpl(getGHintsMap()));
    gHints->setAll(hints);
    const QString dstFolder = gHints->get(DocumentFormat::DBI_FOLDER_HINT, U2ObjectDbi::ROOT_FOLDER).toString();

    MultipleChromatogramAlignment mca = getMcaCopy();
    MultipleChromatogramAlignmentObject *clonedObject = MultipleChromatogramAlignmentImporter::createAlignment(os, dstDbiRef, dstFolder, mca);
    CHECK_OP(os, NULL);

    clonedObject->setGHints(gHints.take());
    clonedObject->setIndexInfo(getIndexInfo());
    return clonedObject;
}

const MultipleChromatogramAlignment MultipleChromatogramAlignmentObject::getMca() const {
    return getMultipleAlignment().dynamicCast<MultipleChromatogramAlignment>();
}

const MultipleChromatogramAlignment MultipleChromatogramAlignmentObject::getMcaCopy() const {
    return getMca()->getCopy();
}

const MultipleChromatogramAlignmentRow MultipleChromatogramAlignmentObject::getMcaRow(int row) const {
    return getRow(row).dynamicCast<MultipleChromatogramAlignmentRow>();
}

void MultipleChromatogramAlignmentObject::loadAlignment(U2OpStatus &os) {
    MultipleChromatogramAlignmentExporter mcaExporter;
    cachedMa = mcaExporter.getAlignment(os, entityRef.dbiRef, entityRef.entityId);
}

void MultipleChromatogramAlignmentObject::updateCachedRows(U2OpStatus &os, const QList<qint64> &rowIds) {
    MultipleChromatogramAlignment cachedMca = cachedMa.dynamicCast<MultipleChromatogramAlignment>();

    MultipleChromatogramAlignmentExporter mcaExporter;
    QMap<qint64, McaRowMemoryData> mcaRowsMemoryData = mcaExporter.getMcaRowMemoryData(os, entityRef.dbiRef, entityRef.entityId, rowIds);
    SAFE_POINT_OP(os, );
    foreach (const qint64 rowId, mcaRowsMemoryData.keys()) {
        const int rowIndex = cachedMca->getRowIndexByRowId(rowId, os);
        SAFE_POINT_OP(os, );
        cachedMca->setRowContent(os, rowIndex, mcaRowsMemoryData[rowId]);
        SAFE_POINT_OP(os, );
        cachedMca->renameRow(rowIndex, mcaRowsMemoryData[rowId].editedSequence.getName());
    }
}

void MultipleChromatogramAlignmentObject::updateDatabase(U2OpStatus &os, const MultipleAlignment &ma) {
    const MultipleChromatogramAlignment mca = ma.dynamicCast<MultipleChromatogramAlignment>();
    McaDbiUtils::updateMca(os, entityRef, mca);
}

void MultipleChromatogramAlignmentObject::renameMaPrivate(U2OpStatus &os, const U2EntityRef &mcaRef, const QString &newName) {
    McaDbiUtils::renameMca(os, mcaRef, newName);
}

void MultipleChromatogramAlignmentObject::removeRowPrivate(U2OpStatus &os, const U2EntityRef &mcaRef, qint64 rowId) {
    McaDbiUtils::removeRow(os, mcaRef, rowId);
}

void MultipleChromatogramAlignmentObject::renameRowPrivate(U2OpStatus &os, const U2EntityRef &mcaRef, qint64 rowId, const QString &newName) {
    McaDbiUtils::renameRow(os, mcaRef, rowId, newName);
}

void MultipleChromatogramAlignmentObject::moveRowsPrivate(U2OpStatus &os, const U2EntityRef &mcaRef, const QList<qint64> &rowsToMove, int delta) {
    McaDbiUtils::moveRows(os, mcaRef, rowsToMove, delta);
}

void MultipleChromatogramAlignmentObject::updateRowsOrderPrivate(U2OpStatus &os, const U2EntityRef &mcaRef, const QList<qint64> &rowsOrder) {
    McaDbiUtils::updateRowsOrder(os, mcaRef, rowsOrder);
}

qint64 MultipleChromatogramAlignmentObject::getMaLengthPrivate(U2OpStatus &os, const U2EntityRef &mcaRef) {
    return McaDbiUtils::getMcaLength(os, mcaRef);
}

U2AlphabetId MultipleChromatogramAlignmentObject::getMaAlphabetPrivate(U2OpStatus &os, const U2EntityRef &mcaRef) {
    return McaDbiUtils::getMcaAlphabet(os, mcaRef);
}

}   // namespace U2
