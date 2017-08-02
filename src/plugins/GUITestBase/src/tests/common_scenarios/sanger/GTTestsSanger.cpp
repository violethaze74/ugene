/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
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

#include <QApplication>
#include <QCheckBox>
#include <QFileInfo>
#include <QLineEdit>
#include <QSpinBox>

#include <base_dialogs/GTFileDialog.h>
#include <base_dialogs/MessageBoxFiller.h>
#include <primitives/GTCheckBox.h>
#include <primitives/GTLineEdit.h>
#include <primitives/GTMenu.h>
#include <primitives/GTSpinBox.h>
#include <primitives/GTWidget.h>

#include "GTTestsSanger.h"
#include "GTUtilsDashboard.h"
#include "GTUtilsLog.h"
#include "GTUtilsMcaEditor.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsWizard.h"
#include "GTUtilsWorkflowDesigner.h"
#include "runnables/ugene/plugins/external_tools/AlignToReferenceBlastDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/WizardFiller.h"

namespace U2 {

namespace GUITest_common_scenarios_sanger {
using namespace HI;
GUI_TEST_CLASS_DEFINITION(test_0001) {
    GTLogTracer l;

    AlignToReferenceBlastDialogFiller::Settings settings;
    settings.referenceUrl = testDir + "_common_data/sanger/reference.gb";
    for (int i = 5; i <= 7; i++) {
        settings.readUrls << QString(testDir + "_common_data/sanger/sanger_%1.ab1").arg(i, 2, 10, QChar('0'));
    }
    settings.outAlignment = QFileInfo(sandBoxDir + "sanger_test_0001").absoluteFilePath();

    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(settings, os));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Reads quality control and alignment...");

    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    class CheckerFiller : public Filler {
    public:
        CheckerFiller(HI::GUITestOpStatus &os, const AlignToReferenceBlastDialogFiller::Settings& settings)
            : Filler(os, "AlignToReferenceBlastDialog"),
              settings(settings)
        {}

        virtual void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");

            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);

            QLineEdit* reference = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "referenceLineEdit", dialog));
            CHECK_SET_ERR(reference, "referenceLineEdit is NULL");
            GTLineEdit::setText(os, reference, settings.referenceUrl);

            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);

            QWidget* addReadButton = GTWidget::findWidget(os, "addReadButton");
            CHECK_SET_ERR(addReadButton, "addReadButton is NULL");
            foreach (const QString& read, settings.readUrls) {
                GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, read));
                GTWidget::click(os, addReadButton);
                GTGlobals::sleep();
            }

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    private:
        AlignToReferenceBlastDialogFiller::Settings settings;
    };

    GTLogTracer l;

    AlignToReferenceBlastDialogFiller::Settings settings;
    settings.referenceUrl = testDir + "_common_data/sanger/reference.gb";
    settings.readUrls << testDir + "_common_data/sanger/sanger_05.ab1";

    GTUtilsDialog::waitForDialog(os, new CheckerFiller(os, settings));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Reads quality control and alignment...");

    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    GTLogTracer l;

    AlignToReferenceBlastDialogFiller::Settings settings;
    settings.referenceUrl = testDir + "_common_data/sanger/reference.gb";
    for (int i = 11; i <= 13; i++) {
        settings.readUrls << QString(testDir + "_common_data/sanger/sanger_%1.ab1").arg(i, 2, 10, QChar('0'));
    }
    settings.outAlignment = QFileInfo(sandBoxDir + "sanger_test_0003").absoluteFilePath();

    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(settings, os));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Reads quality control and alignment...");

    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsLog::checkContainsError(os, l, "No read satisfy minimum identity criteria");
    GTUtilsProject::checkProject(os, GTUtilsProject::NotExists);

    settings.minIdentity = 30;

    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(settings, os));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Reads quality control and alignment...");

    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsProjectTreeView::checkItem(os, "sanger_test_0003");
}

GUI_TEST_CLASS_DEFINITION(test_0004) {
    GTLogTracer l;

    AlignToReferenceBlastDialogFiller::Settings settings;
    settings.referenceUrl = testDir + "_common_data/sanger/reference.gb";
    for (int i = 18; i <= 20; i++) {
        settings.readUrls << QString(testDir + "_common_data/sanger/sanger_%1.ab1").arg(i, 2, 10, QChar('0'));
    }
    settings.outAlignment = QFileInfo(sandBoxDir + "sanger_test_0004").absoluteFilePath();
    settings.addResultToProject = false;

    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(settings, os));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Reads quality control and alignment...");

    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsProject::checkProject(os, GTUtilsProject::NotExists);

    settings.addResultToProject = true;
    settings.outAlignment = QFileInfo(sandBoxDir + "sanger_test_0004_1").absoluteFilePath();

    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(settings, os));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Reads quality control and alignment...");

    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsProjectTreeView::checkItem(os, "sanger_test_0004_4");

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_0005) {
    GTLogTracer logTracer;

//    1. Select "Tools>Workflow Designer"
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

//    2. Open "Trim and аlign Sanger reads" sample

    class Scenario : public CustomScenario {
    public:
        void run(HI::GUITestOpStatus &os) {
//    Expected state: "Trim and Align Sanger Reads" dialog has appered
            QWidget *wizard = GTWidget::getActiveModalWidget(os);
            const QString expectedTitle = "Trim and Align Sanger Reads";
            const QString actualTitle = wizard->windowTitle();
            CHECK_SET_ERR(expectedTitle == actualTitle, QString("Wizard title is incorrect: expected '%1', got '%2'").arg(expectedTitle).arg(actualTitle));

//    3. Select Reference .../test/general/_common_data/sanger/reference.gb
            GTUtilsWizard::setParameter(os, "Reference", testDir + "_common_data/sanger/reference.gb");

//    4. Push Next
            GTUtilsWizard::clickButton(os, GTUtilsWizard::Next);

//    5. On page "Input Sanger reads" add: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files) and click "Next" button
            QStringList readsList;
            for (int i = 1; i <= 20; i++) {
                readsList << testDir + QString("_common_data/sanger/sanger_%1.ab1").arg(i, 2, 10, QChar('0'));
            }
            GTUtilsWizard::setInputFiles(os, QList<QStringList>() << readsList);

            GTUtilsWizard::clickButton(os, GTUtilsWizard::Next);

//    6. Push Next on "Trim and Filtering" page
            GTUtilsWizard::clickButton(os, GTUtilsWizard::Next);

//    7. Push Run on Results page where "alignment.ugenedb" is result database by default
            const QString expectedResultFileName = "alignment.ugenedb";
            QString actualResultFileName = GTUtilsWizard::getParameter(os, "Aligned reads file").toString();
            CHECK_SET_ERR(expectedResultFileName == actualResultFileName, QString("An incorrect result file name: expected '%1', got '%2'")
                          .arg(expectedResultFileName).arg(actualResultFileName));

            GTUtilsWizard::clickButton(os, GTUtilsWizard::Run);
        }
    };

    GTUtilsDialog::waitForDialog(os, new WizardFiller(os, "Trim and Align Sanger Reads", new Scenario()));

    GTUtilsWorkflowDesigner::addSample(os, "Trim and align Sanger reads");

//    Expected state: The workflow task has been finished successfully; "alignment.ugenedb" has created
    GTUtilsTaskTreeView::waitTaskFinished(os);

    const QStringList outputFiles = GTUtilsDashboard::getOutputFiles(os);
    CHECK_SET_ERR(1 == outputFiles.size(), QString("Too many output files, exptected 1, got %1").arg(outputFiles.size()));

    const QString expectedResultFileName = "alignment.ugenedb";
    const QString actualResultFileName = outputFiles.first();
    CHECK_SET_ERR(expectedResultFileName == actualResultFileName, QString("An incorrect output file name, exptected '%1', got '%2'")
                  .arg(expectedResultFileName).arg(actualResultFileName));

//    8. Click to "alignment.ugenedb" on the dashboard.
    GTUtilsDashboard::clickOutputFile(os, actualResultFileName);

    GTUtilsTaskTreeView::waitTaskFinished(os);

//    Expected state:
//        "alignment.ugenedb" in the Project View with object:
//        -{[mc] Aligned reads} for multiple chromatogram alignment object in Project View
    QMap<QString, QStringList> documents = GTUtilsProjectTreeView::getDocuments(os);
    CHECK_SET_ERR(1 == documents.count(), QString("An incorrect documents count: expected 1, got %1").arg(documents.count()));

    const QString actualDocumentName = documents.keys().first();
    CHECK_SET_ERR(expectedResultFileName == actualDocumentName, QString("An unexpected document name: expected '%1', got '%2'")
                  .arg(expectedResultFileName).arg(actualDocumentName));

    CHECK_SET_ERR(1 == documents.first().count(), QString("An incorrect objects count in '%1' document: expected 1, got %2")
                     .arg(documents.keys().first()).arg(documents.first().count()));

    const QString expectedObjectName = "[mc] Aligned reads";
    const QString actualObjectName = documents.first().first();
    CHECK_SET_ERR(expectedObjectName == actualObjectName, QString("An inexpected object name: expected '%1', got '%2'")
                  .arg(expectedObjectName).arg(actualObjectName));

//    Expected state: Aligment editor has been opened
    GTUtilsMcaEditor::getEditorUi(os);

//    Expected state: Reference name is  "Reference KM099231:" at the editor left corner
    const QString expectedReferenceLabelText = "Reference KM099231:";
    const QString actualReferenceLabelText = GTUtilsMcaEditor::getReferenceLabelText(os);
    CHECK_SET_ERR(expectedReferenceLabelText == actualReferenceLabelText, QString("An unexpected reference label text: expected '%1', got '%2'")
                  .arg(expectedReferenceLabelText).arg(actualReferenceLabelText));

//    Expected state: "Consensus" is placed under reference name
    // untestable

//    Expected state: 16 reads are present
    const int readsCount = GTUtilsMcaEditor::getReadsCount(os);
    CHECK_SET_ERR(16 == readsCount, QString("Unexpected count of reads: expected 16, got %1").arg(readsCount));

//                    8 reads with names "SZYD_Cas9_CR60"..."SZYD_Cas9_CR66" and "SZYD_Cas9_CR71"
    const QStringList expectedDirectReadsNames = QStringList() << "SZYD_Cas9_CR60"
                                                               << "SZYD_Cas9_CR61"
                                                               << "SZYD_Cas9_CR62"
                                                               << "SZYD_Cas9_CR63"
                                                               << "SZYD_Cas9_CR64"
                                                               << "SZYD_Cas9_CR65"
                                                               << "SZYD_Cas9_CR66"
                                                               << "SZYD_Cas9_CR71";
    const QStringList actualDirectReadsNames = GTUtilsMcaEditor::getDirectReadsNames(os);
    CHECK_SET_ERR(expectedDirectReadsNames == actualDirectReadsNames, "Direct reads names are incorrect");

//                    8 reverse reads with names "SZYD_Cas9_CR50"... "SZYD_Cas9_CR56" and "SZYD_Cas9_CR70"
    const QStringList expectedReverseComplementReadsNames = QStringList() << "SZYD_Cas9_CR50"
                                                             << "SZYD_Cas9_CR51"
                                                             << "SZYD_Cas9_CR52"
                                                             << "SZYD_Cas9_CR53"
                                                             << "SZYD_Cas9_CR54"
                                                             << "SZYD_Cas9_CR55"
                                                             << "SZYD_Cas9_CR56"
                                                             << "SZYD_Cas9_CR70";
    const QStringList actualReverseComplementReadsNames = GTUtilsMcaEditor::getReverseComplementReadsNames(os);
    CHECK_SET_ERR(expectedReverseComplementReadsNames == actualReverseComplementReadsNames, "Reverse complement reads names are incorrect");

//                    No Еrrors in the Log
    GTUtilsLog::check(os, logTracer);

//    9. Close active view
    GTUtilsMdi::closeActiveWindow(os);

//    10. Select "Open view" from context menu and select "Open new view: "Alignment Editor" from context view
    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Open view" << "Open new view: Alignment Editor", GTGlobals::UseMouse));
    GTUtilsProjectTreeView::callContextMenu(os, expectedObjectName);

//    Expected state:  Chromatogram sanger view is opened
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsMcaEditor::getEditorUi(os);
}

}   // namespace GUITest_common_scenarios_sanger
}   // namespace U2
