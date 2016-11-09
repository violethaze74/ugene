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

#ifndef _U2_MCA_DBI_UTILS_H_
#define _U2_MCA_DBI_UTILS_H_

#include <U2Core/U2Alphabet.h>

namespace U2 {

class MultipleChromatogramAlignment;
class U2McaRow;
class U2EntityRef;
class U2OpStatus;

class U2CORE_EXPORT McaDbiUtils : public QObject {
public:
    static void renameMca(U2OpStatus &os, const U2EntityRef &mcaRef, const QString &newName);

    static qint64 getMcaLength(U2OpStatus &os, const U2EntityRef &mcaRef);
    static void updateMcaLength(U2OpStatus &os, const U2EntityRef &mcaRef, qint64 newLength);

    static U2AlphabetId getMcaAlphabet(U2OpStatus &os, const U2EntityRef &mcaRef);
    static void updateMcaAlphabet(U2OpStatus &os, const U2EntityRef &mcaRef, const U2AlphabetId &alphabetId);

    static void updateMca(U2OpStatus &os, const U2EntityRef &mcaRef, const MultipleChromatogramAlignment &mca);

    static void addRow(U2OpStatus &os, const U2EntityRef &mcaRef, qint64 posInMca, U2McaRow& row);
    static void removeRow(U2OpStatus &os, const U2EntityRef &mcaRef, qint64 rowId);
    static void renameRow(U2OpStatus &os, const U2EntityRef &mcaRef, qint64 rowId, const QString &newName);
    static void moveRows(U2OpStatus &os, const U2EntityRef &mcaRef, const QList<qint64> &rowsToMove, int delta);
    static void updateRowsOrder(U2OpStatus &os, const U2EntityRef &mcaRef, const QList<qint64> &rowsOrder);
};

}   // namespace U2

#endif // _U2_MCA_DBI_UTILS_H_
