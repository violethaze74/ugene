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

    const MultipleSequenceAlignment getMsa() const;
    const MultipleSequenceAlignment getMsaCopy() const;

    /** GObject methods */
    virtual GObject * clone(const U2DbiRef &dstDbiRef, U2OpStatus &os, const QVariantMap &hints = QVariantMap()) const;

    /** Const getters */
    char charAt(int seqNum, int pos) const;
    const MultipleSequenceAlignmentRow getMsaRow(int row) const;

    /** Methods to work with rows */
    void updateRow(U2OpStatus &os, int rowIdx, const QString &name, const QByteArray &seqBytes, const U2MsaRowGapModel &gapModel);

    /** Replace character in row and change alphabet, if it does not contain the character
    */
    void replaceCharacter(int startPos, int rowIndex, char newChar);

private:
    void loadAlignment(U2OpStatus &os);
    void updateCachedRows(U2OpStatus &os, const QList<qint64> &rowIds);
    void updateDatabase(U2OpStatus &os, const MultipleSequenceAlignment &ma);
};

}   // namespace U2

#endif // _U2_MULTIPLE_SEQUENCE_ALIGNMENT_OBJECT_H_
