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

#ifndef _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_OBJECT_H_
#define _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_OBJECT_H_

#include <U2Core/MaModificationInfo.h>
#include <U2Core/MultipleAlignmentObject.h>
#include <U2Core/MultipleChromatogramAlignment.h>
#include <U2Core/GObject.h>

namespace U2 {

class MultipleAlignment;

class U2CORE_EXPORT MultipleChromatogramAlignmentObject : public MultipleAlignmentObject {
    Q_OBJECT
public:
    MultipleChromatogramAlignmentObject(const QString &name,
                                        const U2EntityRef &mcaRef,
                                        const QVariantMap &hintsMap = QVariantMap(),
                                        const MultipleChromatogramAlignment &mca = MultipleChromatogramAlignment());

    GObject * clone(const U2DbiRef &dstDbiRef, U2OpStatus &os, const QVariantMap &hints = QVariantMap()) const;

    char charAt(int seqNum, qint64 position) const;

    const MultipleChromatogramAlignment getMca() const;
    const MultipleChromatogramAlignment getMcaCopy() const;

    const MultipleChromatogramAlignmentRow getMcaRow(int row) const;

    void updateCachedMca(const MaModificationInfo &mi = MaModificationInfo(), const QList<qint64> &removedRowIds = QList<qint64>());
    void sortRowsByList(const QStringList &order);

    void saveState();
    void releaseState();

private:
    void loadAlignment(U2OpStatus &os);
    void updateCachedRows(U2OpStatus &os, const QList<qint64> &rowIds);
    void updateDatabase(U2OpStatus &os, const MultipleAlignment &ma);

    void renameMaPrivate(U2OpStatus &os, const U2EntityRef &mcaRef, const QString &newName);
    void removeRowPrivate(U2OpStatus &os, const U2EntityRef &mcaRef, qint64 rowId);
    void renameRowPrivate(U2OpStatus &os, const U2EntityRef &mcaRef, qint64 rowId, const QString &newName);
    void moveRowsPrivate(U2OpStatus &os, const U2EntityRef &mcaRef, const QList<qint64> &rowsToMove, int delta);
    void updateRowsOrderPrivate(U2OpStatus &os, const U2EntityRef &mcaRef, const QList<qint64> &rowsOrder);
    qint64 getMaLengthPrivate(U2OpStatus &os, const U2EntityRef &mcaRef);
    U2AlphabetId getMaAlphabetPrivate(U2OpStatus &os, const U2EntityRef &mcaRef);
};

}   // namespace U2

#endif // _U2_MULTIPLE_CHROMATOGRAM_ALIGNMENT_OBJECT_H_
