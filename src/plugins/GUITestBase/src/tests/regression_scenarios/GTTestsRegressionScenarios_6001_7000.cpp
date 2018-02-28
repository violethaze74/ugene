/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2018 UniPro <ugene@unipro.ru>
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

#include "GTTestsRegressionScenarios_6001_7000.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsAssemblyBrowser.h"
#include "GTUtilsBookmarksTreeView.h"
#include "GTUtilsCircularView.h"
#include "GTUtilsDashboard.h"
#include "GTUtilsDocument.h"
#include "GTUtilsExternalTools.h"
#include "GTUtilsLog.h"
#include "GTUtilsMcaEditor.h"
#include "GTUtilsMcaEditorSequenceArea.h"
#include "GTUtilsMcaEditorStatusWidget.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditor.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsNotifications.h"
#include "GTUtilsOptionPanelMca.h"
#include "GTUtilsOptionPanelMSA.h"
#include "GTUtilsOptionPanelSequenceView.h"
#include "GTUtilsOptionsPanel.h"
#include "GTUtilsPcr.h"
#include "GTUtilsPhyTree.h"
#include "GTUtilsPrimerLibrary.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsSharedDatabaseDocument.h"
#include "GTUtilsStartPage.h"
#include "GTUtilsTask.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsWizard.h"
#include "GTUtilsWorkflowDesigner.h"

#include "runnables/ugene/corelibs/U2Gui/ImportAPRFileDialogFiller.h"

namespace U2 {

namespace GUITest_regression_scenarios {
using namespace HI;

GUI_TEST_CLASS_DEFINITION(test_6031) {
    //1. Open samples/APR/gyrA.apr in read-only mode
    GTUtilsDialog::waitForDialog(os, new ImportAPRFileFiller(os, true));
    GTFileDialog::openFile(os, dataDir + "samples/APR/gyrA.apr");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected: there is no prefix \27: before name of sequences
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    foreach(const QString& name, nameList) {
        CHECK_SET_ERR(!name.startsWith("\27"), QString("Unexpected start of the name"));
    }
}

GUI_TEST_CLASS_DEFINITION(test_6043) {
//    1. Open "_common_data/ugenedb/sec1_9_ugenedb.ugenedb".
//    Expected state: the assembly is successfully opened, the coverage calculation finished, UGENE doens't crash
    GTFileDialog::openFile(os, testDir + "_common_data/ugenedb/sec1_9_ugenedb.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();
    GTUtilsTaskTreeView::waitTaskFinished(os);

    const bool assemblyExists = GTUtilsProjectTreeView::checkItem(os, "sec1_and_others");
    CHECK_SET_ERR(assemblyExists, "Assembly object is not found in the project view");
}

} // namespace GUITest_regression_scenarios

} // namespace U2
