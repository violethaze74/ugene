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

#ifndef _U2_MULTIPLE_ALIGNMENT_OBJECT_H_
#define _U2_MULTIPLE_ALIGNMENT_OBJECT_H_

#include <U2Core/GObject.h>
#include <U2Core/MaModificationInfo.h>
#include <U2Core/MultipleAlignment.h>

namespace U2 {

const int GAP_COLUMN_ONLY = -1;

#define MOBJECT_MIN_FONT_SIZE 8
#define MOBJECT_MAX_FONT_SIZE 18
#define MOBJECT_MIN_COLUMN_WIDTH 1

#define MOBJECT_SETTINGS_ROOT QString("msaeditor/")
#define MOBJECT_SETTINGS_COLOR_NUCL     "color_nucl"
#define MOBJECT_SETTINGS_COLOR_AMINO    "color_amino"
#define MOBJECT_SETTINGS_FONT_FAMILY    "font_family"
#define MOBJECT_SETTINGS_FONT_SIZE      "font_size"
#define MOBJECT_SETTINGS_FONT_ITALIC    "font_italic"
#define MOBJECT_SETTINGS_FONT_BOLD      "font_bold"
#define MOBJECT_SETTINGS_ZOOM_FACTOR    "zoom_factor"

#define MOBJECT_DEFAULT_FONT_FAMILY "Verdana"
#define MOBJECT_DEFAULT_FONT_SIZE 10
#define MOBJECT_DEFAULT_ZOOM_FACTOR 1.0f

class MaSavedState {
public:
    MaSavedState();
    ~MaSavedState();

    bool hasState() const;
    const MultipleAlignment takeState();
    void setState(const MultipleAlignment &ma);

private:
    MultipleAlignment *lastState;
};

class U2CORE_EXPORT MultipleAlignmentObject : public GObject {
    Q_OBJECT
public:
    MultipleAlignmentObject(const QString &gobjectType,
                            const QString &name,
                            const U2EntityRef &maRef,
                            const QVariantMap &hintsMap,
                            const MultipleAlignment &alignment);
    ~MultipleAlignmentObject();

    /** Sets type of modifications tracking for the alignment */
    void setTrackMod(U2OpStatus &os, U2TrackModType trackMod);

    const MultipleAlignment& getMultipleAlignment() const;
    void setMultipleAlignment(const MultipleAlignment &ma, MaModificationInfo mi = MaModificationInfo(), const QVariantMap &hints = QVariantMap());

    const MultipleAlignment getMultipleAlignmentCopy() const;

    /** GObject methods */
    void setGObjectName(const QString &newName);

    /** Const getters */
    const DNAAlphabet * getAlphabet() const;
    qint64 getLength() const;
    qint64 getNumRows() const;
    const MultipleAlignmentRow getRow(int row) const;
    int getRowPosById(qint64 rowId) const;
    virtual char charAt(int seqNum, qint64 position) const = 0;

    /** Methods to work with rows */
    void removeRow(int rowIdx);
    void renameRow(int rowIdx, const QString &newName);
    void moveRowsBlock(int firstRow, int numRows, int delta);

    /**
     * Updates the rows order.
     * There must be one-to-one correspondence between the specified rows IDs
     * and rows IDs of the alignment.
     */
    void updateRowsOrder(U2OpStatus &os, const QList<qint64> &rowIds);

    void updateCachedMultipleAlignment(const MaModificationInfo &mi = MaModificationInfo(), const QList<qint64> &removedRowIds = QList<qint64>());
    void sortRowsByList(const QStringList &order);

    void saveState();
    void releaseState();

signals:
    void si_startMaUpdating();
    void si_alignmentChanged(const MultipleAlignment &maBefore, const MaModificationInfo &modInfo);
    void si_alignmentBecomesEmpty(bool isEmpty);
    void si_completeStateChanged(bool complete);
    void si_rowsRemoved(const QList<qint64> &rowIds);
    void si_invalidateAlignmentObject();
    void si_alphabetChanged(const MaModificationInfo &mi, const DNAAlphabet *prevAlphabet);

protected:
    virtual void loadAlignment(U2OpStatus &os) = 0;
    virtual void updateCachedRows(U2OpStatus &os, const QList<qint64> &rowIds) = 0;
    virtual void updateDatabase(U2OpStatus &os, const MultipleAlignment &ma) = 0;

    virtual void renameMaPrivate(U2OpStatus &os, const U2EntityRef &maRef, const QString &newName) = 0;
    virtual void removeRowPrivate(U2OpStatus &os, const U2EntityRef &maRef, qint64 rowId) = 0;
    virtual void renameRowPrivate(U2OpStatus &os, const U2EntityRef &maRef, qint64 rowId, const QString &newName) = 0;
    virtual void moveRowsPrivate(U2OpStatus &os, const U2EntityRef &maRef, const QList<qint64> &rowsToMove, int delta) = 0;
    virtual void updateRowsOrderPrivate(U2OpStatus &os, const U2EntityRef &maRef, const QList<qint64> &rowsOrder) = 0;
    virtual qint64 getMaLengthPrivate(U2OpStatus &os, const U2EntityRef &msaRef) = 0;
    virtual U2AlphabetId getMaAlphabetPrivate(U2OpStatus &os, const U2EntityRef &maRef) = 0;

    MultipleAlignment cachedMa;

    static const int GAP_COLUMN_ONLY;

private:
    void loadDataCore(U2OpStatus &os);

    MaSavedState savedState;
};

}   // namespace U2

#endif // _U2_MULTIPLE_ALIGNMENT_OBJECT_H_
