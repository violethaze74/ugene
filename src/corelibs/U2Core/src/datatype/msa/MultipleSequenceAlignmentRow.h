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

#ifndef _U2_MULTIPLE_SEQUENCE_ALIGNMENT_ROW_H_
#define _U2_MULTIPLE_SEQUENCE_ALIGNMENT_ROW_H_

#include <U2Core/U2Msa.h>

namespace U2 {

class DNASequence;
class MultipleSequenceAlignmentData;
class MultipleSequenceAlignmentRowData;

class U2CORE_EXPORT MultipleSequenceAlignmentRow {
protected:
    MultipleSequenceAlignmentRow(MultipleSequenceAlignmentRowData *msaRowData);

public:
    MultipleSequenceAlignmentRow();
    MultipleSequenceAlignmentRow(const MultipleSequenceAlignmentData *msaData);

    /** Creates a row in memory. */
    MultipleSequenceAlignmentRow(const U2MsaRow &rowInDb, const DNASequence &sequence, const U2MsaRowGapModel &gaps, const MultipleSequenceAlignmentData *msaData);
    MultipleSequenceAlignmentRow(const U2MsaRow &rowInDb, const QString &rowName, const QByteArray &rawData, const MultipleSequenceAlignmentData *msaData);
    MultipleSequenceAlignmentRow(const MultipleSequenceAlignmentRow &row, const MultipleSequenceAlignmentData *msaData);

    virtual ~MultipleSequenceAlignmentRow();

    MultipleSequenceAlignmentRowData * data() const;
    template <class Derived> inline Derived dynamicCast() const;

    MultipleSequenceAlignmentRowData & operator*();
    const MultipleSequenceAlignmentRowData & operator*() const;

    MultipleSequenceAlignmentRowData * operator->();
    const MultipleSequenceAlignmentRowData * operator->() const;

    MultipleSequenceAlignmentRow clone() const;

protected:
    QSharedPointer<MultipleSequenceAlignmentRowData> msaRowData;
};

template <class Derived>
Derived MultipleSequenceAlignmentRow::dynamicCast() const {
    return Derived(*this);
}

}   // namespace U2

#endif // _U2_MULTIPLE_SEQUENCE_ALIGNMENT_ROW_H_
