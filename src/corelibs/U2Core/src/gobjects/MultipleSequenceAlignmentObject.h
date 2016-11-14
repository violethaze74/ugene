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

#ifndef _U2_MULTIPLE_SEQUENCE_ALIGNMENT_OBJECT_H_
#define _U2_MULTIPLE_SEQUENCE_ALIGNMENT_OBJECT_H_

#include <U2Core/MultipleAlignmentObject.h>
#include <U2Core/MultipleSequenceAlignment.h>

namespace U2 {

class U2CORE_EXPORT MultipleSequenceAlignmentObject : public MultipleAlignmentObject {
    Q_OBJECT
public:
    MultipleSequenceAlignmentObject(const QString &name,
                                    const U2EntityRef &msaRef,
                                    const QVariantMap &hintsMap = QVariantMap(),
                                    const MultipleSequenceAlignment &msaData = MultipleSequenceAlignment());

    const MultipleSequenceAlignment& getMultipleAlignment() const;
    // SANGER_TODO: rename??
    const MultipleSequenceAlignment getMsaCopy() const;

    /** GObject methods */
    virtual GObject * clone(const U2DbiRef &dstDbiRef, U2OpStatus &os, const QVariantMap &hints = QVariantMap()) const;

    /** Const getters */
    bool isRegionEmpty(int x, int y, int width, int height) const;
    char charAt(int seqNum, int pos) const;
    const MultipleSequenceAlignmentRow getMsaRow(int row) const;

    /** Methods that modify the gap model only */
    void insertGap(const U2Region &rows, int pos, int nGaps);

    /**
     * Removes gap region that extends from the @pos column and is no longer than @maxGaps.
     * If the region starting from @pos and having width of @maxGaps includes any non-gap symbols
     * then its longest subset starting from @pos and containing gaps only is removed.
     *
     * If the given region is a subset of a trailing gaps area then nothing happens.
     */
    int deleteGap(U2OpStatus &os, const U2Region &rows, int pos, int maxGaps);

    /**
     * Updates a gap model of the alignment.
     * The map must contain valid row IDs and corresponding gap models.
     */
    void updateGapModel(U2OpStatus &os, const U2MsaMapGapModel &rowsGapModel);
    void updateGapModel(const QList<MultipleSequenceAlignmentRow> &sourceRows);

    U2MsaMapGapModel getGapModel() const;

    /** Method that affect the whole alignment, including sequences
     */
    void removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows, bool track = true);

    void crop(const U2Region &window, const QSet<QString> &rowNames);
    /**
     * Performs shift of the region specified by parameters @startPos (leftmost column number),
     * @startRow (top row number), @nBases (region width), @nRows (region height) in no more
     * than @shift bases.
     *
     * @startPos and @startRow must be non-negative numbers, @nBases and @nRows - strictly
     * positive. The sign of @shift parameter specifies the direction of shifting: positive
     * for right direction, negative for left one. If 0 == @shift nothing happens.
     *
     * Shifting to the left may be performed only if a region preceding the selection
     * and having the same height consists of gaps only. In this case selected region
     * is moved to the left in the width of the preceding gap region but no more
     * than |@shift| bases.
     *
     * Returns shift size, besides sign of the returning value specifies direction of the shift
     */
    int shiftRegion(int startPos, int startRow, int nBases, int nRows, int shift);
    void deleteColumnWithGaps(U2OpStatus &os, int requiredGapCount);
    void deleteColumnWithGaps(int requiredGapCount = GAP_COLUMN_ONLY);
    QList<qint64> getColumnsWithGaps(int requiredGapCount = GAP_COLUMN_ONLY) const;

    /** Methods to work with rows */
    void updateRow(U2OpStatus &os, int rowIdx, const QString &name, const QByteArray &seqBytes, const U2MsaRowGapModel &gapModel);

    /** Replace character in row and change alphabet, if it does not contain the character
    */
    void replaceCharacter(int startPos, int rowIndex, char newChar);

private:
    void loadAlignment(U2OpStatus &os);
    void updateCachedRows(U2OpStatus &os, const QList<qint64> &rowIds);
    void updateDatabase(U2OpStatus &os, const MultipleAlignment &ma);

    void renameMaPrivate(U2OpStatus &os, const U2EntityRef &msaRef, const QString &newName);
    void removeRowPrivate(U2OpStatus &os, const U2EntityRef &msaRef, qint64 rowId);
    void renameRowPrivate(U2OpStatus &os, const U2EntityRef &msaRef, qint64 rowId, const QString &newName);
    void moveRowsPrivate(U2OpStatus &os, const U2EntityRef &msaRef, const QList<qint64> &rowsToMove, int delta);
    void updateRowsOrderPrivate(U2OpStatus &os, const U2EntityRef &msaRef, const QList<qint64> &rowsOrder);
    qint64 getMaLengthPrivate(U2OpStatus &os, const U2EntityRef &msaRef);
    U2AlphabetId getMaAlphabetPrivate(U2OpStatus &os, const U2EntityRef &msaRef);

    /**
     * Returns maximum count of subsequent gap columns in the region that starts from column
     * with @pos number, has width of @maxGaps and includes the rows specified by @rows.
     * @maxGaps, @pos are to be non-negative numbers. Gap columns should finish in column
     * having @pos + @maxGaps number, otherwise 0 is returned. If the region is located
     * in the MSA trailing gaps area, then 0 is returned.
     */
    int getMaxWidthOfGapRegion(U2OpStatus &os, const U2Region &rows, int pos, int maxGaps);
};

}   // namespace U2

#endif // _U2_MULTIPLE_SEQUENCE_ALIGNMENT_OBJECT_H_
