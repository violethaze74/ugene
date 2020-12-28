/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2020 UniPro <ugene@unipro.ru>
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

#include "GTTestsAssemblyExtractConsensus.h"

#include <QApplication>
#include <QFileInfo>

#include "GTUtilsDashboard.h"
#include "GTUtilsLog.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsWizard.h"
#include "GTUtilsWorkflowDesigner.h"
#include "base_dialogs/DefaultDialogFiller.h"
#include "base_dialogs/GTFileDialog.h"
#include "base_dialogs/MessageBoxFiller.h"
#include "drivers/GTKeyboardDriver.h"
#include "primitives/GTComboBox.h"
#include "primitives/GTLineEdit.h"
#include "primitives/GTMenu.h"
#include "primitives/GTToolbar.h"
#include "primitives/GTWidget.h"
#include "runnables/ugene/plugins/workflow_designer/WizardFiller.h"
#include "utils/GTUtilsDialog.h"

namespace {
    enum class Algo {
        DEFAULT,
        SAMTOOLS
    };
}    // namespace

namespace U2 {

namespace GUITest_assembly_extract_consensus {

struct ExtractConsensusWizardScenario : CustomScenario {
    QStringList inpPaths;
    Algo algo;
    bool keepGaps;
    QString outFile;

    ExtractConsensusWizardScenario(const QStringList &assembliesPath = QStringList(),
                                   const Algo algo = Algo::DEFAULT,
                                   const bool keepGaps = true,
                                   const QString &outputFileName = "consensus.fa")
        : inpPaths(assembliesPath), algo(algo), keepGaps(keepGaps), outFile(outputFileName) {
    }

    void run(HI::GUITestOpStatus &os) {
        QWidget *const dialog = QApplication::activeModalWidget();
        CHECK_SET_ERR(dialog != nullptr, "activeModalWidget is nullptr");
        {
            auto *const urlleInp = GTWidget::findExactWidget<QLineEdit *>(os, "Assembly widget", dialog);
            CHECK_SET_ERR(urlleInp != nullptr, "Assembly widget is nullptr");
            QString inpText;
            foreach (const QString &inp, inpPaths) {
                inpText += QFileInfo(inp).absoluteFilePath();
                inpText += ";";
            }
            if (inpText.length() > 0) {
                inpText.remove(inpText.length() - 1, 1);
            }
            GTLineEdit::setText(os, urlleInp, inpText);
        }
        {
            auto *const cbAlgo = GTWidget::findExactWidget<QComboBox *>(os, "Algorithm widget", dialog);
            CHECK_SET_ERR(cbAlgo != nullptr, "Algorithm widget is nullptr");
            GTComboBox::selectItemByIndex(os, cbAlgo, static_cast<int>(algo));
        }
        {
            auto *const cbKeepGaps = GTWidget::findExactWidget<QComboBox *>(os, "Keep gaps widget", dialog);
            CHECK_SET_ERR(cbKeepGaps != nullptr, "Keep gaps widget is nullptr");
            GTComboBox::selectItemByIndex(os, cbKeepGaps, static_cast<int>(keepGaps));
        }
        {
            auto *const urlleOut = GTWidget::findExactWidget<QLineEdit *>(os, "Output file widget", dialog);
            CHECK_SET_ERR(urlleOut != nullptr, "Output file widget is nullptr");
            GTLineEdit::setText(os, urlleOut, outFile);
        }
        GTUtilsWizard::clickButton(os, GTUtilsWizard::Run);
    }
};

GUI_TEST_CLASS_DEFINITION(sorted_bam_test_0001) {
    const GTLogTracer lt;
    const QString path = dataDir + "samples/Assembly/chrM.sorted.bam";

    //  1. Select "Tools->NGS data analysis->Extract consensus from assemblies..."
    //  2. Set "samples/Assembly/chrM.sorted.bam" as an input
    //  3. Click "Run"
    GTUtilsDialog::waitForDialog(os, new WizardFiller(
        os, "Extract Consensus Wizard", new ExtractConsensusWizardScenario(QStringList() << path)
    ));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools"
                                                << "NGS data analysis"
                                                << "Extract consensus from assemblies...");

    //  4. Wait for workflow finished
    GTUtilsTaskTreeView::waitTaskFinished(os, 100000);
    //  Expected state: There should be no errors in the log
    CHECK_SET_ERR(!lt.hasErrors(), "Errors in log: " + lt.getJoinedErrorString());
    //  There should be no notifications in the dashboard
    CHECK_SET_ERR(!GTUtilsDashboard::hasNotifications(os), "Unexpected notification");

    //  5. Return to workflow and call the Extract consensus wizard
    GTUtilsWorkflowDesigner::returnToWorkflow(os);
    //  6. Fill in the dialog :
    //      {Algorithm} SAMtools
    //      {Keep gaps} False
    //      {Output file}
    //     And click "Run"
    GTUtilsDialog::waitForDialog(os, new WizardFiller(
        os,
        "Extract Consensus Wizard",
        new ExtractConsensusWizardScenario(QStringList() << path, Algo::SAMTOOLS, false, "")
    ));
    GTToolbar::clickButtonByTooltipOnToolbar(os, MWTOOLBAR_ACTIVEMDI, "Show wizard");

    //  7. Wait for workflow finished
    GTUtilsTaskTreeView::waitTaskFinished(os, 100000);
    //  Expected state: There should be no errors in the log
    CHECK_SET_ERR(!lt.hasErrors(), "Errors in log: " + lt.getJoinedErrorString());
    //  There should be no notifications in the dashboard
    CHECK_SET_ERR(!GTUtilsDashboard::hasNotifications(os), "Unexpected notification");
}

GUI_TEST_CLASS_DEFINITION(ugenedb_test_0001) {
    const GTLogTracer lt;
    const QString path = testDir + "_common_data/ugenedb/scerevisiae.bam.ugenedb";

    //  1. Select "Tools->NGS data analysis->Extract consensus from assemblies..."
    //  2. Set "_common_data/ugenedb/scerevisiae.bam.ugenedb" as an input
    //  3. Click "Run"
    GTUtilsDialog::waitForDialog(os, new WizardFiller(
        os, "Extract Consensus Wizard", new ExtractConsensusWizardScenario(QStringList() << path)
    ));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools"
                                                << "NGS data analysis"
                                                << "Extract consensus from assemblies...");

    //  4. Wait for workflow finished
    GTUtilsTaskTreeView::waitTaskFinished(os, 100000);
    //  Expected state: There should be no errors in the log
    CHECK_SET_ERR(!lt.hasErrors(), "Errors in log: " + lt.getJoinedErrorString());
    //  There should be no notifications in the dashboard
    CHECK_SET_ERR(!GTUtilsDashboard::hasNotifications(os), "Unexpected notification");

    //  5. Return to workflow and call the Extract consensus wizard
    GTUtilsWorkflowDesigner::returnToWorkflow(os);
    //  6. Fill in the dialog :
    //      {Algorithm} SAMtools
    //      {Keep gaps} False
    //      {Output file}
    //     And click "Run"
    GTUtilsDialog::waitForDialog(os, new WizardFiller(
        os,
        "Extract Consensus Wizard",
        new ExtractConsensusWizardScenario(QStringList() << path, Algo::SAMTOOLS, false, "")
    ));
    GTToolbar::clickButtonByTooltipOnToolbar(os, MWTOOLBAR_ACTIVEMDI, "Show wizard");

    //  7. Wait for workflow finished
    GTUtilsTaskTreeView::waitTaskFinished(os, 100000);
    //  Expected state: There should be no errors in the log
    CHECK_SET_ERR(!lt.hasErrors(), "Errors in log: " + lt.getJoinedErrorString());
    //  There should be no notifications in the dashboard
    CHECK_SET_ERR(!GTUtilsDashboard::hasNotifications(os), "Unexpected notification");
}

GUI_TEST_CLASS_DEFINITION(multiple_inp_test_0001) {
    struct ExtractConsensusWizardWithAddScenario : CustomScenario {
        QStringList inpPaths;
        Algo algo;
        bool keepGaps;
        QString outFile;

        ExtractConsensusWizardWithAddScenario(const QStringList &assembliesPath,
                                              const Algo algo = Algo::DEFAULT,
                                              const bool keepGaps = true,
                                              const QString &outputFileName = "consensus.fa")
            : inpPaths(assembliesPath), algo(algo), keepGaps(keepGaps), outFile(outputFileName) {
        }

        void run(HI::GUITestOpStatus &os) {
            QWidget *const dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is nullptr");
            {
                auto *const urlleInp = GTWidget::findExactWidget<QLineEdit *>(os, "Assembly widget", dialog);
                CHECK_SET_ERR(urlleInp != nullptr, "Assembly widget is nullptr");
                QString inpText;
                foreach (const QString &inp, inpPaths) {
                    inpText += QFileInfo(inp).absoluteFilePath();
                    inpText += ";";
                }
                if (inpText.length() > 0) {
                    inpText.remove(inpText.length() - 1, 1);
                }
                GTLineEdit::setText(os, urlleInp, inpText);
            }
            {
                auto *const cbAlgo = GTWidget::findExactWidget<QComboBox *>(os, "Algorithm widget", dialog);
                CHECK_SET_ERR(cbAlgo != nullptr, "Algorithm widget is nullptr");
                GTComboBox::selectItemByIndex(os, cbAlgo, static_cast<int>(algo));
            }
            {
                auto *const cbKeepGaps = GTWidget::findExactWidget<QComboBox *>(os, "Keep gaps widget", dialog);
                CHECK_SET_ERR(cbKeepGaps != nullptr, "Keep gaps widget is nullptr");
                GTComboBox::selectItemByIndex(os, cbKeepGaps, static_cast<int>(keepGaps));
            }
            {
                auto *const urlleOut = GTWidget::findExactWidget<QLineEdit *>(os, "Output file widget", dialog);
                CHECK_SET_ERR(urlleOut != nullptr, "Output file widget is nullptr");
                GTLineEdit::setText(os, urlleOut, outFile);
            }
            {
                auto *const tbAddInp = GTWidget::findButtonByText(os, "Add", dialog);
                CHECK_SET_ERR(tbAddInp != nullptr, "Add button is nullptr");
                tbAddInp->click();
                GTUtilsDialog::waitForDialog(os,
                                             new GTFileDialogUtils(os,
                                                                   testDir + "_common_data/bam/small.bam.sorted.bam"));
            }
            GTUtilsWizard::clickButton(os, GTUtilsWizard::Run);
        }
    };

    const GTLogTracer lt;
    const QString path1 = testDir + "_common_data/ugenedb/scerevisiae.bam.ugenedb";
    const QString path2 = dataDir + "samples/Assembly/chrM.sorted.bam";

    //  1. Select "Tools->NGS data analysis->Extract consensus from assemblies..."
    //  2. Set "_common_data/ugenedb/scerevisiae.bam.ugenedb" and "samples/Assembly/chrM.sorted.bam" as an input
    //  3. Add "_common_data/bam/small.bam.sorted.bam" to assemblies
    //  4. Click "Run"
    GTUtilsDialog::waitForDialog(os, new WizardFiller(
        os, "Extract Consensus Wizard", new ExtractConsensusWizardWithAddScenario(QStringList() << path1 << path2)
    ));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools"
                                                << "NGS data analysis"
                                                << "Extract consensus from assemblies...");

    //  5. Wait for workflow finished
    GTUtilsTaskTreeView::waitTaskFinished(os, 100000);
    //  Expected state: There should be no errors in the log
    CHECK_SET_ERR(!lt.hasErrors(), "Errors in log: " + lt.getJoinedErrorString());
    //  There should be no notifications in the dashboard
    CHECK_SET_ERR(!GTUtilsDashboard::hasNotifications(os), "Unexpected notification");

    //  6. Return to workflow and call the Extract consensus wizard
    GTUtilsWorkflowDesigner::returnToWorkflow(os);
    //  7. Fill in the dialog :
    //      {Algorithm} SAMtools
    //      {Keep gaps} False
    //      {Output file}
    //     And click "Run"
    GTUtilsDialog::waitForDialog(os, new WizardFiller(
        os,
        "Extract Consensus Wizard",
        new ExtractConsensusWizardWithAddScenario(QStringList() << path1 << path2, Algo::SAMTOOLS, false, "")
    ));
    GTToolbar::clickButtonByTooltipOnToolbar(os, MWTOOLBAR_ACTIVEMDI, "Show wizard");

    //  8. Wait for workflow finished
    GTUtilsTaskTreeView::waitTaskFinished(os, 100000);
    //  Expected state: There should be no errors in the log
    CHECK_SET_ERR(!lt.hasErrors(), "Errors in log: " + lt.getJoinedErrorString());
    //  There should be no notifications in the dashboard
    CHECK_SET_ERR(!GTUtilsDashboard::hasNotifications(os), "Unexpected notification");
}

GUI_TEST_CLASS_DEFINITION(wrong_inp_test_0001) {
    const GTLogTracer lt;
    const QString path1 = dataDir + "samples/Assembly/chrM.fa";
    const QString path2 = dataDir + "samples/Assembly/chrM.sorted.bam";
    const QString dashboardErrMsg = "Unsupported document format: ";

    const auto hasDashboardNotification = [](HI::GUITestOpStatus &os, const QString &errMsg) {
        return !GTWidget::findLabelByText(os, errMsg, GTWidget::findWidget(
            os, "NotificationsDashboardWidget", GTUtilsDashboard::getDashboard(os)
        )).isEmpty();
    };

    //  1. Select "Tools->NGS data analysis->Extract consensus from assemblies..."
    //  2. Set "samples/Assembly/chrM.fa" as an input
    //  3. Click "Run"
    GTUtilsDialog::waitForDialog(os, new WizardFiller(
        os, "Extract Consensus Wizard", new ExtractConsensusWizardScenario(QStringList() << path2 << path1)
    ));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools"
                                                << "NGS data analysis"
                                                << "Extract consensus from assemblies...");

    //  4. Wait for workflow finished
    GTUtilsTaskTreeView::waitTaskFinished(os, 100000);
    //  Expected state: There should be an error about unsopported format in the log
    GTUtilsLog::checkContainsError(os, lt, dashboardErrMsg);
    //  There should be a notification about this error in the dashboard
    CHECK_SET_ERR(hasDashboardNotification(os, dashboardErrMsg),
                  "Expected dashboard notification \"" + dashboardErrMsg + "\"");

    //  5. Return to workflow and call the Extract consensus wizard
    GTUtilsWorkflowDesigner::returnToWorkflow(os);
    //  6. Clear input assemblies
    GTUtilsDialog::waitForDialog(os, new WizardFiller(
        os, "Extract Consensus Wizard", new ExtractConsensusWizardScenario()
    ));
    GTToolbar::clickButtonByTooltipOnToolbar(os, MWTOOLBAR_ACTIVEMDI, "Show wizard");

    //  7. Wait for workflow finished
    GTUtilsTaskTreeView::waitTaskFinished(os, 100000);

    //  Expected state: There should be dialog "Workflow cannot be executed"
    CHECK_SET_ERR(QApplication::activeModalWidget(), "activeModalWidget is nullptr");
    //  8. Click "Ok" in this dialog
    GTUtilsDialog::waitForDialog(os, new HI::MessageBoxDialogFiller(os, QMessageBox::Ok));
    //  There should also be an error about missing required input parameter in the workflow
    GTUtilsWorkflowDesigner::checkErrorList(
        os, "Read Assembly: Required parameter has no input urls specified: Input file(s)"
    );
}

}    // namespace GUITest_assembly_extract_consensus

}    // namespace U2
