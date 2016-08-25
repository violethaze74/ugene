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
    return getMsa()->getCopy();
}

GObject * MultipleSequenceAlignmentObject::clone(const U2DbiRef &dstDbiRef, U2OpStatus &os, const QVariantMap &hints) const {
    DbiOperationsBlock opBlock(dstDbiRef, os);
    Q_UNUSED(opBlock);
    CHECK_OP(os, NULL);

    QScopedPointer<GHintsDefaultImpl> gHints(new GHintsDefaultImpl(getGHintsMap()));
    gHints->setAll(hints);
    const QString dstFolder = gHints->get(DocumentFormat::DBI_FOLDER_HINT, U2ObjectDbi::ROOT_FOLDER).toString();

    MultipleSequenceAlignment msa = getMsa()->getCopy();
    MultipleSequenceAlignmentObject *clonedObj = MultipleSequenceAlignmentImporter::createAlignment(dstDbiRef, dstFolder, msa, os);
    CHECK_OP(os, NULL);

    clonedObj->setGHints(gHints.take());
    clonedObj->setIndexInfo(getIndexInfo());
    return clonedObj;
}

char MultipleSequenceAlignmentObject::charAt(int seqNum, int pos) const {
    return getMsa()->charAt(seqNum, pos);
}

const MultipleSequenceAlignmentRow MultipleSequenceAlignmentObject::getMsaRow(int row) const {
    return getRow(row).dynamicCast<MultipleSequenceAlignmentRow>();
}

void MultipleSequenceAlignmentObject::updateRow(U2OpStatus &os, int rowIdx, const QString &name, const QByteArray &seqBytes, const U2MsaRowGapModel &gapModel) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", );

    const MultipleSequenceAlignment msa = getMsa();
    SAFE_POINT(rowIdx >= 0 && rowIdx < msa->getNumRows(), "Invalid row index", );
    qint64 rowId = msa->getRow(rowIdx)->getRowDbInfo().rowId;

    MsaDbiUtils::updateRowContent(entityRef, rowId, seqBytes, gapModel, os);
    CHECK_OP(os, );

    MsaDbiUtils::renameRow(entityRef, rowId, name, os);
    CHECK_OP(os, );
}

void MultipleSequenceAlignmentObject::replaceCharacter(int startPos, int rowIndex, char newChar) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", );
    const MultipleSequenceAlignment msa = getMsa();
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
            MsaDbiUtils::updateMsaAlphabet(entityRef, newAlphabet->getId(), os);
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

void MultipleSequenceAlignmentObject::updateDatabase(U2OpStatus &os, const MultipleSequenceAlignment &ma) {
    const MultipleSequenceAlignment msa = ma.dynamicCast<MultipleSequenceAlignment>();
    MsaDbiUtils::updateMsa(entityRef, msa, os);
}

}   // namespace U2
