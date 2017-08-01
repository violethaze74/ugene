/**
* UGENE - Integrated Bioinformatics Tools.
* Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
* http://ugene.net
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

#ifndef GTUTILSMCAEDITORSEQUENCEAREA_H
#define GTUTILSMCAEDITORSEQUENCEAREA_H

#include <GTGlobals.h>

#include <U2View/McaEditorSequenceArea.h>

namespace U2 {

class GTUtilsMcaEditorSequenceArea {
public:
    static McaEditorSequenceArea* getSequenceArea(GUITestOpStatus &os);
    static QStringList getVisibleNames(GUITestOpStatus &os);
    static int getRowHeight(GUITestOpStatus &os, int rowNumber);
    static void clickToPosition(GUITestOpStatus &os, const QPoint &globalMaPosition);
    static void scrollToPosition(GUITestOpStatus &os, const QPoint &position);
    static void clickCollapseTriangle(GUITestOpStatus &os, QString seqName, bool showChromatogram);
    static bool isChromatogramShown(GUITestOpStatus &os, QString seqName);
    static QStringList getNameList(GUITestOpStatus &os);
    static void callContextMenu(GUITestOpStatus &os, const QPoint &innerCoords = QPoint());
    static void moveTo(GUITestOpStatus &os, const QPoint &p);
    static QPoint convertCoordinates(GUITestOpStatus &os, const QPoint p);
};

}//namespace
#endif // GTUTILSMSAEDITORSEQUENCEAREA_H
