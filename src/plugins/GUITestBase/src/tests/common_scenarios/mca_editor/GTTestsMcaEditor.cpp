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

#include <base_dialogs/ColorDialogFiller.h>
#include <base_dialogs/DefaultDialogFiller.h>
#include <base_dialogs/FontDialogFiller.h>
#include <base_dialogs/GTFileDialog.h>
#include <base_dialogs/MessageBoxFiller.h>
#include <drivers/GTKeyboardDriver.h>
#include <drivers/GTMouseDriver.h>
#include <primitives/GTAction.h>
#include <primitives/GTCheckBox.h>
#include <primitives/GTComboBox.h>
#include <primitives/GTLineEdit.h>
#include <primitives/GTMenu.h>
#include <primitives/GTRadioButton.h>
#include <primitives/GTSpinBox.h>
#include <primitives/GTToolbar.h>
#include <primitives/GTWidget.h>
#include <primitives/PopupChooser.h>
#include <system/GTClipboard.h>
#include <system/GTFile.h>
#include <utils/GTKeyboardUtils.h>
#include <utils/GTThread.h>

#include <QApplication>
#include <QList>

#include <U2View/MaEditorNameList.h>

#include "GTTestsMcaEditor.h"
#include "runnables/ugene/plugins/external_tools/AlignToReferenceBlastDialogFiller.h"

#include "GTUtilsLog.h"
#include "GTUtilsMcaEditor.h"
#include "GTUtilsMcaEditorSequenceArea.h"
#include "GTUtilsMsaEditor.h"
#include "GTUtilsNotifications.h"
#include "GTUtilsOptionPanelMca.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsSequenceView.h"

namespace U2 {

namespace GUITest_common_scenarios_mca_editor {
//using namespace HI

const QString namesOfRow[16] = {"SZYD_Cas9_CR50",
                                "SZYD_Cas9_CR51",
                                "SZYD_Cas9_CR52",
                                "SZYD_Cas9_CR53",
                                "SZYD_Cas9_CR54",
                                "SZYD_Cas9_CR55",
                                "SZYD_Cas9_CR56",
                                "SZYD_Cas9_CR60",
                                "SZYD_Cas9_CR61",
                                "SZYD_Cas9_CR62",
                                "SZYD_Cas9_CR63",
                                "SZYD_Cas9_CR64",
                                "SZYD_Cas9_CR65",
                                "SZYD_Cas9_CR66",
                                "SZYD_Cas9_5B70",
                                "SZYD_Cas9_5B71" };

GUI_TEST_CLASS_DEFINITION(test_0001) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb]
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/reference.gb
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/reference.gb");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)]
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state :
    //"reference_sanger_reads_alignment.ugenedb" in the Project View with object :
    //-{'mc' Aligned reads} for multiple chromatogram alignment object in Project View
    GTUtilsProject::checkProject(os);
    GTUtilsProjectTreeView::openView(os);
    bool check = GTUtilsProjectTreeView::checkItem(os, "Aligned reads");
    CHECK_SET_ERR(check, "'Aligned reads' is not present in the project view");

    //    Expected state : Reference name is  "KM099231" at the editor left corner
    //    Expected state : 16 reads are present
    int rows = GTUtilsMcaEditor::getReadsCount(os);
    CHECK_SET_ERR(rows == 16, QString("Incorrect rows quantity, expected: 16, current: %1").arg(rows));
    //    16 reads with names "SZYD_Cas9_CR50"..."SZYD_Cas9_CR56", "SZYD_Cas9_CR60"..."SZYD_Cas9_CR66", "SZYD_Cas9_CR70" and "SZYD_Cas9_CR71"
    QList<QString> rowNames = GTUtilsMcaEditor::getReadsNames(os);
    foreach(QString rowName, rowNames) {
        bool checkCurrentName = false;
        for (int i = 0; i < 16; i++) {
            QString currentName = namesOfRow[i];
            if (rowName == currentName) {
                checkCurrentName = true;
                break;
            }
        }
        CHECK_SET_ERR(checkCurrentName, QString("Name %1 is missing").arg(rowName));
    }

    //5. Report with info
    GTUtilsNotifications::waitForNotification(os, false, "Aligned reads (16)");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //number of filtered sequences with the names: 4
    GTUtilsNotifications::waitForNotification(os, false, "Filtered by quality (4)");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //No Еrrors in the Log
    QStringList errors = GTUtilsLog::getErrors(os, GTLogTracer("error"));
    CHECK_SET_ERR(errors.isEmpty(), QString("Some errors found"));
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/reference.gb
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/reference.gb");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Uncheck "Add to project" option
            GTCheckBox::setChecked(os, "addToProjectCheckbox", false);

            //5. Push "..." in "Result aligment"
            GTLineEdit::setText(os, "outputLineEdit", "...", QApplication::activeModalWidget());

            //6. In "Select output file" dialog set file name "Sanger" in test directory without  extension.ugenedb
            GTUtilsTaskTreeView::waitTaskFinished(os);
            QStringList path;
            path << sandBoxDir + "Sanger";
            GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils_list(os, path));

            //7.Push Save
            GTWidget::click(os, GTWidget::findExactWidget<QToolButton*>(os, "setOutputButton"));

            //Expected state : "Result alignment" is filled <path> / Sanger.ugenedb
            output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutputContains = output.contains("Sanger.ugenedb");
            CHECK_SET_ERR(checkOutputContains, QString("incorrect output line"));

            //8. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state : No Еrrors in the Log
    //Be sure that file <path>/Sanger.ugenedb is created on the disk
    QStringList errors = GTUtilsLog::getErrors(os, GTLogTracer("error"));
    CHECK_SET_ERR(errors.isEmpty(), QString("Some errors found"));

    //9. Open <path> / "Sanger.ugenedb" in the project
    GTFileDialog::openFile(os, sandBoxDir + "Sanger.ugenedb");

    //    Expected state :
    //"reference_sanger_reads_alignment.ugenedb" in the Project View with object :
    //-{'mc' Aligned reads} for multiple chromatogram alignment object in Project View
    //    Expected state : Aligment editor has been opened
    GTUtilsProject::checkProject(os);
    GTUtilsProjectTreeView::openView(os);
    //Expected state : Reference name is  "KM099231" at the editor left corner

    //    Expected state : 16 reads are present
    int rows = GTUtilsMcaEditor::getReadsCount(os);
    CHECK_SET_ERR(rows == 16, QString("Incorrect rows quantity, expected: 16, current: %1").arg(rows));

    //16 reads with names "SZYD_Cas9_CR50"..."SZYD_Cas9_CR56", "SZYD_Cas9_CR60"..."SZYD_Cas9_CR66", "SZYD_Cas9_5B70" and "SZYD_Cas9_5B71"
    QList<QString> rowNames = GTUtilsMcaEditor::getReadsNames(os);
    foreach(QString rowName, rowNames) {
        bool checkCurrentName = false;
        for (int i = 0; i < 16; i++) {
            QString currentName = namesOfRow[i];
            if (rowName == currentName) {
                checkCurrentName = true;
                break;
            }
        }
        CHECK_SET_ERR(checkCurrentName, QString("Name %1 is missing").arg(rowName));
    }

    //No Еrrors in the Log
    errors = GTUtilsLog::getErrors(os, GTLogTracer("error"));
    CHECK_SET_ERR(errors.isEmpty(), QString("Some errors found"));
}

GUI_TEST_CLASS_DEFINITION(test_0006) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  /test/general/_common_data/alphabets/standard_dna_rna_amino_1000.fa
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/alphabets/standard_dna_rna_amino_1000.fa");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Uncheck "Add to project" option
            GTCheckBox::setChecked(os, "addToProjectCheckbox", false);

            //5. Push "..." in "Result aligment"
            GTLineEdit::setText(os, "outputLineEdit", "...", QApplication::activeModalWidget());

            //6. In "Select output file" dialog set file name "Sanger" in test directory without  extension.ugenedb
            GTUtilsTaskTreeView::waitTaskFinished(os);
            QStringList path;
            path << sandBoxDir + "Sanger";
            GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils_list(os, path));

            //7.Push Save
            GTWidget::click(os, GTWidget::findExactWidget<QToolButton*>(os, "setOutputButton"));

            //Expected state : "Result alignment" is filled <path> / Sanger.ugenedb
            output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutputContains = output.contains("Sanger.ugenedb");
            CHECK_SET_ERR(checkOutputContains, QString("incorrect output line"));

            //8. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTLogTracer trace;
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state: Error: More than one sequence in the reference file:  <path>/alphabets/standard_dna_rna_amino_1000.fa
    GTUtilsLog::checkContainsError(os, trace, QString("Task {Align Sanger reads to reference} finished with error: More than one sequence in the reference file:"));
}

GUI_TEST_CLASS_DEFINITION(test_0008) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  /test/general/_common_data/alphabets/standard_dna_rna_1000.fa
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/alphabets/standard_rna_1000.fa");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Uncheck "Add to project" option
            GTCheckBox::setChecked(os, "addToProjectCheckbox", false);

            //5. Push "..." in "Result aligment"
            GTLineEdit::setText(os, "outputLineEdit", "...", QApplication::activeModalWidget());

            //6. In "Select output file" dialog set file name "Sanger" in test directory without  extension.ugenedb
            GTUtilsTaskTreeView::waitTaskFinished(os);
            QStringList path;
            path << sandBoxDir + "Sanger";
            GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils_list(os, path));

            //7.Push Save
            GTWidget::click(os, GTWidget::findExactWidget<QToolButton*>(os, "setOutputButton"));

            //Expected state : "Result alignment" is filled <path> / Sanger.ugenedb
            output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutputContains = output.contains("Sanger.ugenedb");
            CHECK_SET_ERR(checkOutputContains, QString("incorrect output line"));

            //8. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTLogTracer trace;
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state: Error: The input reference sequence 'seq3' contains characters that don't belong to DNA alphabet.
    GTUtilsLog::checkContainsError(os, trace, QString("Task {Align Sanger reads to reference} finished with error: The input reference sequence 'seq3' contains characters that don't belong to DNA alphabet."));
}

GUI_TEST_CLASS_DEFINITION(test_0010) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb]
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/reference.gb
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/reference.gb");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)]
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //5. Select "SZYD_Cas9_5B71" read
    GTUtilsMcaEditor::clickReadName(os, "SZYD_Cas9_5B71");

    //6. Select position 2120
    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));

    //Expected state : aligned read sequence with chromatogram, black triangle, directed down and right before read name
    bool isChromatogramShown = GTUtilsMcaEditorSequenceArea::isChromatogramShown(os, "SZYD_Cas9_5B71");
    CHECK_SET_ERR(isChromatogramShown, "Error: alighed read sequence should be displayed with chromatogram")

    //7. Push black triangle
    GTUtilsMcaEditorSequenceArea::clickCollapseTriangle(os, "SZYD_Cas9_5B71", true);

    //Expected state : now it is white triangle, directed to the right
    //Alighed read sequence has been displayed without chromatogram
    isChromatogramShown = GTUtilsMcaEditorSequenceArea::isChromatogramShown(os, "SZYD_Cas9_5B71");
    CHECK_SET_ERR(!isChromatogramShown, "Error: alighed read sequence should be displayed without chromatogram")

    //8. Push white triangle
    GTUtilsMcaEditorSequenceArea::clickCollapseTriangle(os, "SZYD_Cas9_5B71", false);

    //Expected state : now it is black triangle again
    //Alighed read sequence has been displayed with chromatogram
    isChromatogramShown = GTUtilsMcaEditorSequenceArea::isChromatogramShown(os, "SZYD_Cas9_5B71");
    CHECK_SET_ERR(isChromatogramShown, "Error: alighed read sequence should be displayed with chromatogram")
}

GUI_TEST_CLASS_DEFINITION(test_0011) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb]
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/reference.gb
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/reference.gb");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)]
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state : "Show/Hide Chromatograms" button is in pressed state
    //All reads are expanded, chromatograms if any are displayed for each reads
    QList<QString> rows = GTUtilsMcaEditor::getReadsNames(os);
    foreach(QString row, rows) {
        bool isChromatogramShown = GTUtilsMcaEditorSequenceArea::isChromatogramShown(os, row);
        CHECK_SET_ERR(isChromatogramShown, QString("Error: read %1 should be displayed with chromatogram").arg(row));
    }

    //5. Push "Show/Hide Chromatograms" button in the main menu
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "chromatograms"));

    //Expected state : "Show/Hide Chromatograms" button is in normal state
    //All reads are collapsed
    foreach(QString row, rows) {
        bool isChromatogramShown = GTUtilsMcaEditorSequenceArea::isChromatogramShown(os, row);
        CHECK_SET_ERR(!isChromatogramShown, QString("Error: read %1 should be displayed without chromatogram").arg(row));
    }

    //6. Push "Show/Hide Chromatograms" button again
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "chromatograms"));

    //All reads are expanded, chromatograms if any are displayed for each reads
    //Expected state : All reads are expanded, chromatograms(if any) are displayed for each reads
    foreach(QString row, rows) {
        bool isChromatogramShown = GTUtilsMcaEditorSequenceArea::isChromatogramShown(os, row);
        CHECK_SET_ERR(isChromatogramShown, QString("Error: read %1 should be displayed with chromatogram").arg(row));
    }
}

GUI_TEST_CLASS_DEFINITION(test_0012_1) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb]
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/reference.gb
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/reference.gb");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)]
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //5. Select "SZYD_Cas9_5B71" read
    GTUtilsMcaEditor::clickReadName(os, "SZYD_Cas9_5B71");

    //6. Select position 2120
    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));

    //7. Push "Show/Hide Trace"combo from context menu
    //Expected state : Combo with selected "A", "C", "G", "T" items and "Show all" item appeared
    QList<QStringList> paths;
    QList<QStringList> onlyLetterPaths;
    paths.append(QStringList() << "Appearance" << "Show/hide trace" << "A");
    onlyLetterPaths.append(QStringList() << "Appearance" << "Show/hide trace" << "A");
    paths.append(QStringList() << "Appearance" << "Show/hide trace" << "C");
    onlyLetterPaths.append(QStringList() << "Appearance" << "Show/hide trace" << "C");
    paths.append(QStringList() << "Appearance" << "Show/hide trace" << "G");
    onlyLetterPaths.append(QStringList() << "Appearance" << "Show/hide trace" << "G");
    paths.append(QStringList() << "Appearance" << "Show/hide trace" << "T");
    onlyLetterPaths.append(QStringList() << "Appearance" << "Show/hide trace" << "T");
    paths.append(QStringList() << "Appearance" << "Show/hide trace" << "Show all");
    //GTUtilsDialog::waitForDialog(os, new PopupCheckerByText(os, paths, PopupChecker::CheckOptions(PopupChecker::Exists)));
    QStringList menuPath;
    menuPath << "Appearance" << "Show/hide trace";
    QStringList itemsNames;
    itemsNames << "A" << "C" << "G" << "T" << "Show all";
    GTUtilsDialog::waitForDialog(os, new PopupCheckerByText(os, menuPath, itemsNames, PopupChecker::CheckOptions(PopupChecker::Exists)));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //8. Select "A"
    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Appearance" << "Show/hide trace" << "A"));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state : Green trace vanished from all chromatograms

    //9. Select "C"
    //Expected state : "A" is unchecked
    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    QStringList intermediateCheck;
    intermediateCheck << "A";
    GTUtilsDialog::waitForDialog(os, new PopupCheckerByText(os, menuPath, intermediateCheck, PopupChecker::CheckOptions(PopupChecker::IsUnchecked)));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Appearance" << "Show/hide trace" << "C"));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state : Blue trace vanished from all chromatograms

    //10. Select "G"
    //Expected state : "A" and "C"  are unchecked
    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    intermediateCheck << "C";
    GTUtilsDialog::waitForDialog(os, new PopupCheckerByText(os, menuPath, intermediateCheck, PopupChecker::CheckOptions(PopupChecker::IsUnchecked)));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Appearance" << "Show/hide trace" << "G"));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state : Grey trace vanished from all chromatograms

    //11. Select "T"
    //Expected state : "A", "C", "G"  are unchecked
    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));\
    intermediateCheck << "G";
    GTUtilsDialog::waitForDialog(os, new PopupCheckerByText(os, menuPath, intermediateCheck, PopupChecker::CheckOptions(PopupChecker::IsUnchecked)));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Appearance" << "Show/hide trace" << "T"));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state : Red trace vanished from all chromatograms; there are no any traces on the screen

    //12. Select "All"
    //Expected state : All four traces are restored for all chromatograms
    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    intermediateCheck << "T";
    GTUtilsDialog::waitForDialog(os, new PopupCheckerByText(os, menuPath, intermediateCheck, PopupChecker::CheckOptions(PopupChecker::IsUnchecked)));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Appearance" << "Show/hide trace" << "Show all"));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);


    //13. Expand combo once more
    //Expected state : All four letters are checked
    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    GTUtilsDialog::waitForDialog(os, new PopupCheckerByText(os, menuPath, intermediateCheck, PopupChecker::CheckOptions(PopupChecker::IsChecked)));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
}

GUI_TEST_CLASS_DEFINITION(test_0012_2) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb]
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/reference.gb
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/reference.gb");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)]
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //5. Select "SZYD_Cas9_5B71" read
    GTUtilsMcaEditor::clickReadName(os, "SZYD_Cas9_5B71");

    //6. Select position 2120
    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));

    //7. Push "Show/Hide Trace"combo from main menu
    //Expected state : Combo with selected "A", "C", "G", "T" items and "Show all" item appeared
    const QStringList menuPath = QStringList() << "Actions" << "Appearance" << "Show/hide trace";
    const QStringList itemsNames = QStringList() << "A" << "C" << "G" << "T" << "Show all";
    GTMenu::checkMainMenuItemsState(os, menuPath, itemsNames, PopupChecker::CheckOption(PopupChecker::Exists));

    //8. Select "A"
    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Appearance" << "Show/hide trace" << "A");;

    //Expected state : Green trace vanished from all chromatograms

    //9. Select "C"
    //Expected state : "A" is unchecked
    QStringList intermediateCheck = QStringList() << "A";
    GTMenu::checkMainMenuItemsState(os, menuPath, intermediateCheck, PopupChecker::CheckOption(PopupChecker::IsUnchecked));

    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Appearance" << "Show/hide trace" << "C");;

    //Expected state : Blue trace vanished from all chromatograms

    //10. Select "G"
    //Expected state : "A" and "C"  are unchecked
    intermediateCheck << "C";
    GTMenu::checkMainMenuItemsState(os, menuPath, intermediateCheck, PopupChecker::CheckOption(PopupChecker::IsUnchecked));

    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Appearance" << "Show/hide trace" << "G");;

    //Expected state : Grey trace vanished from all chromatograms

    //11. Select "T"
    //Expected state : "A", "C", "G"  are unchecked
    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    intermediateCheck << "G";
    GTMenu::checkMainMenuItemsState(os, menuPath, intermediateCheck, PopupChecker::CheckOption(PopupChecker::IsUnchecked));

    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Appearance" << "Show/hide trace" << "T");;

    //Expected state : Red trace vanished from all chromatograms; there are no any traces on the screen

    //12. Select "All"
    //Expected state : All four traces are restored for all chromatograms
    intermediateCheck << "T";
    GTMenu::checkMainMenuItemsState(os, menuPath, intermediateCheck, PopupChecker::CheckOption(PopupChecker::IsUnchecked));

    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Appearance" << "Show/hide trace" << "Show all");;

    //13. Expand combo once more
    //Expected state : All four letters are checked
    GTMenu::checkMainMenuItemsState(os, menuPath, intermediateCheck, PopupChecker::CheckOption(PopupChecker::IsChecked));

    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
}

GUI_TEST_CLASS_DEFINITION(test_0013_1) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb]
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/reference.gb
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/reference.gb");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)]
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //5. Select "SZYD_Cas9_CR50" read
    GTUtilsMcaEditor::clickReadName(os, "SZYD_Cas9_CR50");

    //6. Push Remove seuence(s) button on main menu
    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Edit" << "Remove read"));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state : the read is deleted
    QStringList reads = GTUtilsMcaEditor::getReadsNames(os);
    bool isReadWasDelete = true;
    foreach(QString read, reads) {
        if (read == "SZYD_Cas9_CR50") {
            isReadWasDelete = false;
        }
    }
    CHECK_SET_ERR(isReadWasDelete, "Error: read SZYD_Cas9_CR50 was not delete");

    //7. Select 3 reads using Shift modifier
    GTUtilsMcaEditor::clickReadName(os, "SZYD_Cas9_5B70");
    GTKeyboardDriver::keyPress(Qt::Key_Shift);
    GTUtilsMcaEditor::clickReadName(os, "SZYD_Cas9_5B71");
    GTUtilsMcaEditor::clickReadName(os, "SZYD_Cas9_CR51");
    GTKeyboardDriver::keyRelease(Qt::Key_Shift);

    //8. Push Remove seuence(s) button
    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Edit" << "Remove read"));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state : 3 reads are deleted
    //Expected state : No corresponding reads in the map
    reads = GTUtilsMcaEditor::getReadsNames(os);
    isReadWasDelete = true;
    foreach(QString read, reads) {
        if (read == "SZYD_Cas9_5B70" || read == "SZYD_Cas9_5B71" || read == "SZYD_Cas9_CR51") {
            isReadWasDelete = false;
        }
    }
    CHECK_SET_ERR(isReadWasDelete, "Error: read SZYD_Cas9_CR50 was not delete");

    //9. Push undo
    GTUtilsMcaEditor::undo(os);

    //Expected state : 3 reads are restored in the same place
    //Expected state : Map is restored too
    reads = GTUtilsMcaEditor::getReadsNames(os);
    int restoreReadsCount = 0;
    QStringList removedReads;
    removedReads << "SZYD_Cas9_5B70" << "SZYD_Cas9_5B71" << "SZYD_Cas9_5B70";
    foreach(QString read, reads) {
        if (read == "SZYD_Cas9_5B70") {
            restoreReadsCount++;
            removedReads.removeOne("SZYD_Cas9_5B70");
        }
        if (read == "SZYD_Cas9_5B71") {
            restoreReadsCount++;
            removedReads.removeOne("SZYD_Cas9_5B71");
        }
        if (read == "SZYD_Cas9_CR51") {
            restoreReadsCount++;
            removedReads.removeOne("SZYD_Cas9_CR51");
        }
    }
    CHECK_SET_ERR(restoreReadsCount == 3, QString("Error: read %1 was not restore").arg(removedReads.first()));
}

GUI_TEST_CLASS_DEFINITION(test_0013_2) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb]
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/reference.gb
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/reference.gb");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)]
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //5. Select "SZYD_Cas9_CR50" read
    GTUtilsMcaEditor::clickReadName(os, "SZYD_Cas9_CR50");

    //6. Push Remove seuence(s) button on context menu
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Edit" << "Remove read");;

    //Expected state : the read is deleted
    QStringList reads = GTUtilsMcaEditor::getReadsNames(os);
    bool isReadWasDelete = true;
    foreach(QString read, reads) {
        if (read == "SZYD_Cas9_CR50") {
            isReadWasDelete = false;
        }
    }
    CHECK_SET_ERR(isReadWasDelete, "Error: read SZYD_Cas9_CR50 was not delete");

    //7. Select 3 reads using Shift modifier
    GTUtilsMcaEditor::clickReadName(os, "SZYD_Cas9_5B70");
    GTKeyboardDriver::keyPress(Qt::Key_Shift);
    GTUtilsMcaEditor::clickReadName(os, "SZYD_Cas9_5B71");
    GTUtilsMcaEditor::clickReadName(os, "SZYD_Cas9_CR51");
    GTKeyboardDriver::keyRelease(Qt::Key_Shift);

    //8. Push Remove seuence(s) button
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Edit" << "Remove read");;

    //Expected state : 3 reads are deleted
    //Expected state : No corresponding reads in the map
    reads = GTUtilsMcaEditor::getReadsNames(os);
    isReadWasDelete = true;
    foreach(QString read, reads) {
        if (read == "SZYD_Cas9_5B70" || read == "SZYD_Cas9_5B71" || read == "SZYD_Cas9_CR51") {
            isReadWasDelete = false;
        }
    }
    CHECK_SET_ERR(isReadWasDelete, "Error: read SZYD_Cas9_CR50 was not delete");

    //9. Push undo
    GTUtilsMcaEditor::undo(os);

    //Expected state : 3 reads are restored in the same place
    //Expected state : Map is restored too
    reads = GTUtilsMcaEditor::getReadsNames(os);
    int restoreReadsCount = 0;
    QStringList removedReads;
    removedReads << "SZYD_Cas9_5B70" << "SZYD_Cas9_5B71" << "SZYD_Cas9_5B70";
    foreach(QString read, reads) {
        if (read == "SZYD_Cas9_5B70") {
            restoreReadsCount++;
            removedReads.removeOne("SZYD_Cas9_5B70");
        }
        if (read == "SZYD_Cas9_5B71") {
            restoreReadsCount++;
            removedReads.removeOne("SZYD_Cas9_5B71");
        }
        if (read == "SZYD_Cas9_CR51") {
            restoreReadsCount++;
            removedReads.removeOne("SZYD_Cas9_CR51");
        }
    }
    CHECK_SET_ERR(restoreReadsCount == 3, QString("Error: read %1 was not restore").arg(removedReads.first()));
}

GUI_TEST_CLASS_DEFINITION(test_0013_3) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb]
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/reference.gb
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/reference.gb");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)]
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //5. Select any symbol  in the "SZYD_Cas9_CR50" read
    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(5500, 2));

    //6. Push Remove seuence(s) from main or context menu
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Edit" << "Remove read");

    //Expected state: the read is deleted
    QStringList reads = GTUtilsMcaEditor::getReadsNames(os);
    bool isReadWasDelete = true;
    foreach(QString read, reads) {
        if (read == "SZYD_Cas9_CR50") {
            isReadWasDelete = false;
        }
    }
    CHECK_SET_ERR(isReadWasDelete, "Error: read SZYD_Cas9_CR50 was not delete");
}

GUI_TEST_CLASS_DEFINITION(test_0014) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb]
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/reference.gb
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/reference.gb");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)]
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //5. Select vertical slider between reads named and read area
    //6. Move the slider to the left until part of the name becomes invisible
    GTUtilsMcaEditorSequenceArea::moveTheBorderBetweenAlignmentAndRead(os, -250);
    GTUtilsMcaEditorSequenceArea::moveTheBorderBetweenAlignmentAndRead(os, 5);

    //Expected state : Horizontal scrolls bar appears
    QScrollBar *hscroll = GTWidget::findExactWidget<QScrollBar *>(os, "horizontal_names_scroll");
    bool isHidden = hscroll->isHidden();
    CHECK_SET_ERR(!isHidden, "Horizontal scrolls bar not found");

    //7. Move the slider to the right until all names become visible
    GTUtilsMcaEditorSequenceArea::moveTheBorderBetweenAlignmentAndRead(os, 250);
    GTUtilsMcaEditorSequenceArea::moveTheBorderBetweenAlignmentAndRead(os, -5);

    //Expected state : Horizontal scrolls bar disappears
    hscroll = GTWidget::findExactWidget<QScrollBar *>(os, "horizontal_names_scroll");
    isHidden = hscroll->isHidden();
    CHECK_SET_ERR(isHidden, "Horizontal scrolls bar is found");

}

GUI_TEST_CLASS_DEFINITION(test_0015_1) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb]
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/reference.gb
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/reference.gb");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)]
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //5. In Option panelSelect consensuns mode = Strict
    GTUtilsOptionPanelMca::setConsensusType(os, "Strict");

    //6. Push "Ctrl+Alt+v"
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "next_mismatch"));
    GTGlobals::sleep(1000);

    //Expected state : first difference between reference "A" and consensus "T"
    QString referenceChar = GTUtilsMcaEditorSequenceArea::getReferenceReg(os, 2059, 1);

    //7. Push "Jump to next variation" button twice
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "next_mismatch"));

    //Expected state : difference between reference "T" and consensus "A"
    referenceChar = GTUtilsMcaEditorSequenceArea::getReferenceReg(os, 2061, 1);

    //8. Push "Jump to next variation" from context menu
    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Navigation" << "Jump to next variation"));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state : difference between reference "A" and consensus "T"
    referenceChar = GTUtilsMcaEditorSequenceArea::getReferenceReg(os, 2062, 1);
}

GUI_TEST_CLASS_DEFINITION(test_0015_2) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/sanger_01.ab1
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/sanger_01.ab1");

            //3. Select Read: .../test/general/_common_data/sanger/sanger_01.ab1
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils* d = new GTFileDialogUtils(os, testDir + "_common_data/sanger/sanger_01.ab1");
            GTUtilsDialog::waitForDialog(os, d);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(5000);

    //5. Push "Jump to next variation" button
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "next_mismatch"));

    //Expected state : Notification "There are no variations in the consensus sequence" will be shown
    GTUtilsNotifications::waitForNotification(os, true, "There are no variations in the consensus sequence");
    GTUtilsTaskTreeView::waitTaskFinished(os);
}

GUI_TEST_CLASS_DEFINITION(test_0016) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb]
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/reference.gb
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/reference.gb");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)]
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //5. In Option panelSelect consensuns mode = Strict
    GTUtilsOptionPanelMca::setConsensusType(os, "Strict");

    //6. Push "Ctrl+Alt+Shift+v"
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "prev_mismatch"));
    GTGlobals::sleep(1000);

    //Expected state : first difference between reference "A" and consensus "T"
    QString referenceChar = GTUtilsMcaEditorSequenceArea::getReferenceReg(os, 2059, 1);

    //7. Push "Jump to previous variation" button twice
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "prev_mismatch"));

    //Expected state : difference between reference "T" and consensus "A"
    referenceChar = GTUtilsMcaEditorSequenceArea::getReferenceReg(os, 2061, 1);

    //8. Push "Jump to previous variation" from context menu
    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Navigation" << "Jump to previous variation"));
    GTUtilsMcaEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state : difference between reference "A" and consensus "T"
    referenceChar = GTUtilsMcaEditorSequenceArea::getReferenceReg(os, 2062, 1);

    //9. Push "Jump to next variation" button 3 times
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "next_mismatch"));
    GTGlobals::sleep(1000);
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "next_mismatch"));
    GTGlobals::sleep(1000);
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "next_mismatch"));
    GTGlobals::sleep(1000);

    //Expected state : The cursor jumps to the begining at the first difference reference "A" and  consensus "T"
}

GUI_TEST_CLASS_DEFINITION(test_0019) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb]
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/reference.gb
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/reference.gb");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)]
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //5. Select any read name by mouse
    QStringList visibleRows = GTUtilsMcaEditorSequenceArea::getVisibleNames(os);
    QString firstVisibleRow = visibleRows.first();
    GTUtilsMcaEditor::moveToReadName(os, firstVisibleRow);

    //6. Using 'drag'n drop' move the name in the another place and release mouse button
    const QRect sequenceNameRect = GTUtilsMcaEditor::getReadNameRect(os, firstVisibleRow);
    GTUtilsMcaEditorSequenceArea::dragAndDrop(os, QPoint(sequenceNameRect.center().x() , sequenceNameRect.y() + (2 *sequenceNameRect.height())));

    //Expected state : The read is replaced in another place
    QStringList newOrederedVisibleRows = GTUtilsMcaEditorSequenceArea::getVisibleNames(os);
    int size = visibleRows.size();
    bool isNewOrder = false;
    for (int i = 0; i < size; i++) {
        if (visibleRows[i] != newOrederedVisibleRows[i]) {
            isNewOrder = true;
            break;
        }
    }
    CHECK_SET_ERR(isNewOrder, "The order was not change");
}

GUI_TEST_CLASS_DEFINITION(test_0021) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            //Expected state : "Min read identity" option by default = 80 %
            int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
            QString expected = "80";
            CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

            //Expected state : "Quality threshold" option by default = 30
            int quality = GTSpinBox::getValue(os, "qualitySpinBox");
            expected = "30";
            CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

            //Expected state : "Add to project" option is checked by default
            bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
            CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

            //Expected state : "Result aligment" field is filled by default
            QString output = GTLineEdit::getText(os, "outputLineEdit");
            bool checkOutput = output.isEmpty();
            CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

            //Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb]
            bool checkContainsFirst = output.contains(".ugenedb", Qt::CaseInsensitive);
            bool checkContainsSecond = output.contains("sanger_reads_alignment");
            bool checkContainsThird = output.contains("UGENE_Data");
            bool checkContainsFourth = output.contains("Documents");
            bool checkContains = checkContainsFirst && checkContainsSecond && checkContainsThird &&checkContainsFourth;
            CHECK_SET_ERR(checkContains, QString("incorrect output line: do not contain default path"));

            //2. Select reference  .../test/general/_common_data/sanger/reference.gb
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit*>(os, "referenceLineEdit"), testDir + "_common_data/sanger/reference.gb");

            //3. Select Reads: .../test/general/_common_data/sanger/sanger_01.ab1-/sanger_20.ab1(20 files)]
            QStringList reads;
            for (int i = 1; i < 21; i++) {
                QString name = "sanger_";
                QString num = QString::number(i);
                if (num.size() == 1) {
                    num = "0" + QString::number(i);
                }
                name += num;
                name += ".ab1";
                reads << name;
            }
            QString readDir = testDir + "_common_data/sanger/";
            GTUtilsTaskTreeView::waitTaskFinished(os);
            GTFileDialogUtils_list* ob = new GTFileDialogUtils_list(os, readDir, reads);
            GTUtilsDialog::waitForDialog(os, ob);

            GTWidget::click(os, GTWidget::findExactWidget<QPushButton*>(os, "addReadButton"));

            //4. Push "Align" button
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //5. Select any read
    QStringList visibleRows = GTUtilsMcaEditorSequenceArea::getVisibleNames(os);
    QString firstVisibleRow = visibleRows.first();
    GTUtilsMcaEditor::clickReadName(os, firstVisibleRow);
    GTGlobals::sleep(500);

    //6. Push Esc
    GTKeyboardDriver::keyClick(Qt::Key_Escape);
    GTGlobals::sleep(500);

    //Expected state : There is no selection
    int selectedRows = GTUtilsMcaEditorSequenceArea::getSelectedRowsNum(os);
    CHECK_SET_ERR(selectedRows == 0, "Some reads are selected");

    //7. Select any region in the reference
    GTUtilsMcaEditorSequenceArea::clickToReferencePosition(os, 500);
    GTGlobals::sleep(500);

    //8. Push Esc
    GTKeyboardDriver::keyClick(Qt::Key_Escape);
    GTGlobals::sleep(500);

    //Expected state : There is no selection
    qint64 selectedNum = GTUtilsMcaEditorSequenceArea::getReferenceSelectedNum(os);
    CHECK_SET_ERR(selectedNum == -1, "Some reference character is lselected");

    //9. Select any symbol in the read
    GTUtilsMcaEditorSequenceArea::clickToPosition(os, QPoint(2120, 1));
    GTGlobals::sleep(500);

    //10. Push Esc
    GTKeyboardDriver::keyClick(Qt::Key_Escape);
    GTGlobals::sleep(500);

    //Expected state : There is no selection
    QRect selection = GTUtilsMcaEditorSequenceArea::getSelectedRect(os);
    CHECK_SET_ERR(selection == QRect(), "Some character in alignent is lselected");
}


}//namespace U2

}//namespace
