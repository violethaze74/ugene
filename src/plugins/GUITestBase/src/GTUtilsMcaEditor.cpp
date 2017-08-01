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

#include <drivers/GTMouseDriver.h>

#include <U2Core/U2SafePoints.h>

#include <U2Gui/MainWindow.h>

#include <U2Core/McaDbiUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2View/MaEditorNameList.h>
#include <U2View/McaEditorSequenceArea.h>

#include "GTUtilsMdi.h"
#include "GTUtilsMcaEditor.h"
#include "GTUtilsMcaEditorSequenceArea.h"

namespace U2 {
#define GT_CLASS_NAME "GTUtilsMcaEditor"

#define GT_METHOD_NAME "getSequenceNameRect"
McaEditorSequenceArea * GTUtilsMcaEditor::getSequenceArea(GUITestOpStatus &os) {
    QWidget *activeWindow = GTUtilsMdi::activeWindow(os);
    CHECK_OP(os, NULL);

    McaEditorSequenceArea *result = qobject_cast<McaEditorSequenceArea*>(GTWidget::findWidget(os, "mca_editor_sequence_area", activeWindow));
    GT_CHECK_RESULT(NULL != result, "MsaEditorSequenceArea is not found", NULL);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getRowsQuantity"
int GTUtilsMcaEditor::getRowsQuantity(HI::GUITestOpStatus &os) {
    McaEditorSequenceArea* ref = getSequenceArea(os);
    MultipleChromatogramAlignmentObject* mcaObj = ref->getEditor()->getMaObject();
    GT_CHECK_RESULT(NULL != mcaObj, "MultipleChromatogramAlignmentObject is not found", 0);
    U2OpStatus2Log status;
    return McaDbiUtils::getMcaRows(status, mcaObj->getEntityRef()).size();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getRowNames"
QList<QString> GTUtilsMcaEditor::getRowNames(HI::GUITestOpStatus &os) {
    McaEditorSequenceArea* ref = getSequenceArea(os);
    const MultipleChromatogramAlignment mca = ref->getEditor()->getMaObject()->getMca();
    return mca->getRowNames();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickReadName"
void GTUtilsMcaEditor::clickReadName(HI::GUITestOpStatus &os, const QString &readName, Qt::MouseButton mouseButton) {
    moveToReadName(os, readName);
    GTMouseDriver::click(mouseButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getNameListArea"
MaEditorNameList * GTUtilsMcaEditor::getNameListArea(GUITestOpStatus &os) {
    QWidget *activeWindow = GTUtilsMdi::activeWindow(os);
    CHECK_OP(os, NULL);

    MaEditorNameList *result = GTWidget::findExactWidget<MaEditorNameList*>(os, "msa_editor_name_list", activeWindow);
    GT_CHECK_RESULT(NULL != result, "MaGraphOverview is not found", NULL);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getReadNameRect"
QRect GTUtilsMcaEditor::getReadNameRect(GUITestOpStatus &os, const QString &readName) {
    MaEditorNameList *nameList = getNameListArea(os);
    GT_CHECK_RESULT(NULL != nameList, "McaEditorNameList not found", QRect());

    const QStringList names = GTUtilsMcaEditorSequenceArea::getVisibleNames(os);
    const int rowNumber = names.indexOf(readName);
    GT_CHECK_RESULT(0 <= rowNumber, QString("Read '%1' not found").arg(readName), QRect());
    return getReadNameRect(os, rowNumber);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getRedNameRect"
QRect GTUtilsMcaEditor::getReadNameRect(GUITestOpStatus &os, int rowNumber) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(0 <= rowNumber, QString("Read '%1' not found").arg(rowNumber), QRect());

    MaEditorNameList *nameList = getNameListArea(os);
    GT_CHECK_RESULT(NULL != nameList, "McaEditorNameList not found", QRect());

    const int rowHeight = GTUtilsMcaEditorSequenceArea::getRowHeight(os, rowNumber);

    return QRect(nameList->mapToGlobal(QPoint(0, rowHeight * rowNumber)), nameList->mapToGlobal(QPoint(nameList->width(), rowHeight * (rowNumber + 1))));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "moveToReadName"
void GTUtilsMcaEditor::moveToReadName(GUITestOpStatus &os, const QString &readName) {
    const QRect sequenceNameRect = getReadNameRect(os, readName);
    GTMouseDriver::moveTo(sequenceNameRect.center());
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
