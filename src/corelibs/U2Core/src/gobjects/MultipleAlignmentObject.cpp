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
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "MultipleAlignmentObject.h"

namespace U2 {

MaSavedState::MaSavedState()
    : lastState(NULL)
{

}

MaSavedState::~MaSavedState() {
    delete lastState;
}

bool MaSavedState::hasState() const {
    return NULL != lastState;
}

const MultipleAlignment MaSavedState::takeState() {
    const MultipleAlignment state = *lastState;
    delete lastState;
    lastState = NULL;
    return state;
}

void MaSavedState::setState(const MultipleAlignment &ma) {
    if (NULL != lastState) {
        delete lastState;
    }
    lastState = new MultipleAlignment(ma->getCopy());
}

const int MultipleAlignmentObject::GAP_COLUMN_ONLY = -1;

MultipleAlignmentObject::MultipleAlignmentObject(const QString &gobjectType,
                                                 const QString &name,
                                                 const U2EntityRef &maRef,
                                                 const QVariantMap &hintsMap,
                                                 const MultipleAlignment &alignment)
    : GObject(gobjectType, name, hintsMap),
      cachedMa(alignment->getCopy())
{
    entityRef = maRef;
    dataLoaded = false;

    if (!cachedMa->isEmpty()) {
        dataLoaded = true;
    }
}

MultipleAlignmentObject::~MultipleAlignmentObject(){
    emit si_invalidateAlignmentObject();
}

void MultipleAlignmentObject::setTrackMod(U2OpStatus &os, U2TrackModType trackMod) {
    // Prepare the connection
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, );

    U2ObjectDbi *objectDbi = con.dbi->getObjectDbi();
    SAFE_POINT(NULL != objectDbi, "NULL Object Dbi", );

    // Set the new status
    objectDbi->setTrackModType(entityRef.entityId, trackMod, os);
}

const MultipleAlignment & MultipleAlignmentObject::getMultipleAlignment() const {
    ensureDataLoaded();
    return cachedMa;
}

void MultipleAlignmentObject::setMultipleAlignment(const MultipleAlignment &newMa, MaModificationInfo mi, const QVariantMap &hints) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", );

    U2OpStatus2Log os;
    updateDatabase(os, newMa);
    SAFE_POINT_OP(os, );

    mi.hints = hints;
    updateCachedMultipleAlignment(mi);
}

void MultipleAlignmentObject::setGObjectName(const QString &newName) {
    ensureDataLoaded();
    CHECK(cachedMa->getName() != newName, );

    if (!isStateLocked()) {
        U2OpStatus2Log os;
        MsaDbiUtils::renameMsa(entityRef, newName, os);
        CHECK_OP(os, );

        updateCachedMultipleAlignment();
    } else {
        GObject::setGObjectName(newName);
        cachedMa->setName(newName);
    }
}

const DNAAlphabet* MultipleAlignmentObject::getAlphabet() const {
    return getMultipleAlignment()->getAlphabet();
}

qint64 MultipleAlignmentObject::getLength() const {
    return getMultipleAlignment()->getLength();
}

qint64 MultipleAlignmentObject::getNumRows() const {
    return getMultipleAlignment()->getNumRows();
}

const MultipleAlignmentRow MultipleAlignmentObject::getRow(int row) const {
    return getMultipleAlignment()->getRow(row);
}

int MultipleAlignmentObject::getRowPosById(qint64 rowId) const {
    return getMultipleAlignment()->getRowsIds().indexOf(rowId);
}

void MultipleAlignmentObject::removeRow(int rowIdx) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", );

    const MultipleSequenceAlignment &ma = getMultipleAlignment();
    SAFE_POINT(rowIdx >= 0 && rowIdx < ma->getNumRows(), "Invalid row index", );
    qint64 rowId = ma->getRow(rowIdx)->getRowId();

    U2OpStatus2Log os;
    MsaDbiUtils::removeRow(entityRef, rowId, os);
    SAFE_POINT_OP(os, );

    MaModificationInfo mi;
    mi.rowContentChanged = false;
    mi.alignmentLengthChanged = false;

    QList<qint64> removedRowIds;
    removedRowIds << rowId;

    updateCachedMultipleAlignment(mi, removedRowIds);
}

void MultipleAlignmentObject::renameRow(int rowIdx, const QString &newName) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", );

    const MultipleSequenceAlignment &ma = getMultipleAlignment();
    SAFE_POINT(rowIdx >= 0 && rowIdx < ma->getNumRows(), "Invalid row index", );
    qint64 rowId = ma->getRow(rowIdx)->getRowId();

    U2OpStatus2Log os;
    MsaDbiUtils::renameRow(entityRef, rowId, newName, os);
    SAFE_POINT_OP(os, );

    MaModificationInfo mi;
    mi.alignmentLengthChanged = false;
    updateCachedMultipleAlignment(mi);
}

void MultipleAlignmentObject::moveRowsBlock(int firstRow, int numRows, int shift) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", );

    QList<qint64> rowIds = getMultipleAlignment()->getRowsIds();
    QList<qint64> rowsToMove;

    for (int i = 0; i < numRows; ++i) {
        rowsToMove << rowIds[firstRow + i];
    }

    U2OpStatusImpl os;
    MsaDbiUtils::moveRows(entityRef, rowsToMove, shift, os);
    CHECK_OP(os, );

    updateCachedMultipleAlignment();
}

void MultipleAlignmentObject::updateRowsOrder(U2OpStatus &os, const QList<qint64> &rowIds) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", );

    MsaDbiUtils::updateRowsOrder(entityRef, rowIds, os);
    CHECK_OP(os, );

    MaModificationInfo mi;
    mi.alignmentLengthChanged = false;
    updateCachedMultipleAlignment(mi);
}

void MultipleAlignmentObject::updateCachedMultipleAlignment(const MaModificationInfo &mi, const QList<qint64> &removedRowIds) {
    ensureDataLoaded();
    emit si_startMaUpdating();

    MultipleSequenceAlignment maBefore = cachedMa->getCopy();
    QString oldName = maBefore->getName();

    U2OpStatus2Log os;

    if (mi.alignmentLengthChanged) {
        qint64 msaLength = MsaDbiUtils::getMsaLength(entityRef, os);
        SAFE_POINT_OP(os, );
        if (msaLength != cachedMa->getLength()) {
            cachedMa->setLength(msaLength);
        }
    }

    if (mi.alphabetChanged) {
        U2AlphabetId alphabet = MsaDbiUtils::getMsaAlphabet(entityRef, os);
        SAFE_POINT_OP(os, );
        if (alphabet.id != cachedMa->getAlphabet()->getId() && !alphabet.id.isEmpty()) {
            const DNAAlphabet *newAlphabet = U2AlphabetUtils::getById(alphabet);
            cachedMa->setAlphabet(newAlphabet);
        }
    }

     if (mi.modifiedRowIds.isEmpty() && removedRowIds.isEmpty()) { // suppose that in this case all the alignment has changed
        loadAlignment(os);
        SAFE_POINT_OP(os, );
    } else { // only specified rows were changed
        if (!removedRowIds.isEmpty()) {
            foreach (qint64 rowId, removedRowIds) {
                const int rowIndex = cachedMa->getRowIndexByRowId(rowId, os);
                SAFE_POINT_OP(os, );
                cachedMa->removeRow(rowIndex, os);
                SAFE_POINT_OP(os, );
            }
        }
        if (!mi.modifiedRowIds.isEmpty()) {
            updateCachedRows(os, mi.modifiedRowIds);
        }
    }

    setModified(true);
    if (!mi.middleState) {
        emit si_alignmentChanged(maBefore, mi);

        if (cachedMa->isEmpty() && !maBefore->isEmpty()) {
            emit si_alignmentBecomesEmpty(true);
        } else if (!cachedMa->isEmpty() && maBefore->isEmpty()) {
            emit si_alignmentBecomesEmpty(false);
        }

        const QString newName = cachedMa->getName();
        if (oldName != newName) {
            setGObjectNameNotDbi(newName);
        }
    }
    if (!removedRowIds.isEmpty()) {
        emit si_rowsRemoved(removedRowIds);
    }
    if (cachedMa->getAlphabet()->getId() != maBefore->getAlphabet()->getId()) {
        emit si_alphabetChanged(mi, maBefore->getAlphabet());
    }
}

void MultipleAlignmentObject::sortRowsByList(const QStringList &order) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked", );

    MultipleSequenceAlignment ma = getMultipleAlignment()->getCopy();
    ma->sortRowsByList(order);
    CHECK(ma->getRowsIds() != cachedMa->getRowsIds(), );

    U2OpStatusImpl os;
    MsaDbiUtils::updateRowsOrder(entityRef, ma->getRowsIds(), os);
    SAFE_POINT_OP(os, );

    MaModificationInfo mi;
    mi.alignmentLengthChanged = false;
    mi.rowContentChanged = false;
    mi.rowListChanged = false;
    updateCachedMultipleAlignment(mi);
}

void MultipleAlignmentObject::saveState(){
    const MultipleSequenceAlignment &ma = getMultipleAlignment();
    emit si_completeStateChanged(false);
    savedState.setState(ma);
}

void MultipleAlignmentObject::releaseState() {
    if (!isStateLocked()) {
        emit si_completeStateChanged(true);

        CHECK(savedState.hasState(), );
        MultipleAlignment maBefore = savedState.takeState();
        CHECK(*maBefore != *getMultipleAlignment(), );
        setModified(true);

        MaModificationInfo mi;
        emit si_alignmentChanged(maBefore, mi);

        if (cachedMa->isEmpty() && !maBefore->isEmpty()) {
            emit si_alignmentBecomesEmpty(true);
        } else if (!cachedMa->isEmpty() && maBefore->isEmpty()) {
            emit si_alignmentBecomesEmpty(false);
        }
    }
}

void MultipleAlignmentObject::loadDataCore(U2OpStatus &os) {
    DbiConnection con(entityRef.dbiRef, os);
    Q_UNUSED(con);
    CHECK_OP(os, );
    loadAlignment(os);
}

}   // namespace U2
