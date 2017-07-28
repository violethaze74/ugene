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

#include <drivers/GTKeyboardDriver.h>
#include <drivers/GTMouseDriver.h>
#include <primitives/GTToolbar.h>
#include <primitives/PopupChooser.h>
#include <system/GTClipboard.h>
#include <utils/GTKeyboardUtils.h>
#include <utils/GTThread.h>

#include <U2Core/U2SafePoints.h>

#include <U2Gui/MainWindow.h>

#include <U2Core/McaDbiUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2View/BaseWidthController.h>
#include <U2View/MSAEditorConsensusArea.h>
#include <U2View/MaEditorNameList.h>
#include <U2View/MsaEditorOverviewArea.h>
#include <U2View/MaGraphOverview.h>
#include <U2View/MaSimpleOverview.h>
#include <U2View/McaEditorSequenceArea.h>

#include "GTUtilsMdi.h"
#include "GTUtilsMcaEditor.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsOptionPanelMSA.h"
#include "GTUtilsProjectTreeView.h"
#include "api/GTMSAEditorStatusWidget.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/BuildTreeDialogFiller.h"

namespace U2 {
    using namespace HI;

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

#undef GT_CLASS_NAME

}   // namespace U2
