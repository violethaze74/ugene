/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2021 UniPro <ugene@unipro.ru>
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

#include "GTTestsRegressionScenarios_7001_8000.h"

#include <QFileInfo>

#include "primitives/GTMenu.h"
#include "primitives/GTWidget.h"
#include "runnables/ugene/corelibs/U2Gui/AppSettingsDialogFiller.h"
#include "utils/GTUtilsDialog.h"

namespace U2 {

namespace GUITest_regression_scenarios {
using namespace HI;

GUI_TEST_CLASS_DEFINITION(test_7003) {
    // 1. Ensure that 'UGENE_EXTERNAL_TOOLS_VALIDATION_BY_PATH_ONLY' is not set to "1"
    // 2. Open "UGENE Application Settings", select "External Tools" tab
    // 3. Add the 'dumb.sh' or 'dumb.cmd' as a Python executable
    // 4. Check that validation fails

    CHECK_SET_ERR(qgetenv("UGENE_EXTERNAL_TOOLS_VALIDATION_BY_PATH_ONLY") != "1",
                  "UGENE_EXTERNAL_TOOLS_VALIDATION_BY_PATH_ONLY is set to 1, but expected 0")

    class CheckPythonInvalidation : public CustomScenario {
        void run(GUITestOpStatus &os) override {
            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::ExternalTools);

            QString toolPath = testDir + "_common_data/regression/7003/dumb.";
            #ifdef Q_OS_WIN
            toolPath += "cmd";
            #else
            toolPath += "sh";
            #endif

            AppSettingsDialogFiller::setExternalToolPath(os, "python", QFileInfo(toolPath).absoluteFilePath());
            CHECK_SET_ERR(!AppSettingsDialogFiller::isExternalToolValid(os, "python"),
                          "Python module is expected to be invalid, but in fact it is valid")

            GTUtilsDialog::clickButtonBox(os, GTWidget::getActiveModalWidget(os), QDialogButtonBox::Cancel);
        }
    };

    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new CheckPythonInvalidation()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings"
                                                << "Preferences...",
                              GTGlobals::UseMouse);
}

}    // namespace GUITest_regression_scenarios

}    // namespace U2
