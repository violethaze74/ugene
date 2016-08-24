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

#include "MultipleSequenceAlignmentRow.h"
#include "MultipleSequenceAlignmentRowData.h"

namespace U2 {

MultipleSequenceAlignmentRow::MultipleSequenceAlignmentRow(MultipleSequenceAlignmentRowData *msaRowData)
    : msaRowData(msaRowData)
{

}

MultipleSequenceAlignmentRow::MultipleSequenceAlignmentRow()
    : msaRowData(new MultipleSequenceAlignmentRowData)
{

}

MultipleSequenceAlignmentRow::MultipleSequenceAlignmentRow(const MultipleSequenceAlignmentData *msa)
    : msaRowData(new MultipleSequenceAlignmentRowData(msa))
{

}

MultipleSequenceAlignmentRow::MultipleSequenceAlignmentRow(const U2MsaRow &rowInDb, const DNASequence &sequence, const U2MsaRowGapModel &gaps, const MultipleSequenceAlignmentData *msaData)
    : msaRowData(new MultipleSequenceAlignmentRowData(rowInDb, sequence, gaps, msaData))
{

}

MultipleSequenceAlignmentRow::MultipleSequenceAlignmentRow(const U2MsaRow &rowInDb, const QString &rowName, const QByteArray &rawData, const MultipleSequenceAlignmentData *msaData)
    : msaRowData(new MultipleSequenceAlignmentRowData(rowInDb, rowName, rawData, msaData))
{

}

MultipleSequenceAlignmentRow::MultipleSequenceAlignmentRow(const MultipleSequenceAlignmentRow &row, const MultipleSequenceAlignmentData *msaData)
    : msaRowData(new MultipleSequenceAlignmentRowData(row, msa))
{

}

MultipleSequenceAlignmentRow::~MultipleSequenceAlignmentRow() {

}

MultipleSequenceAlignmentRowData * MultipleSequenceAlignmentRow::data() const {
    return msaRowData.data();
}

MultipleSequenceAlignmentRowData & MultipleSequenceAlignmentRow::operator*() {
    return *msaRowData;
}

const MultipleSequenceAlignmentRowData & MultipleSequenceAlignmentRow::operator*() const {
    return *msaRowData;
}

MultipleSequenceAlignmentRowData * MultipleSequenceAlignmentRow::operator->() {
    return msaRowData.data();
}

const MultipleSequenceAlignmentRowData * MultipleSequenceAlignmentRow::operator->() const {
    return msaRowData.data();
}

MultipleSequenceAlignmentRow MultipleSequenceAlignmentRow::clone() const {
    return msaRowData->clone();
}

}   // namespace U2
