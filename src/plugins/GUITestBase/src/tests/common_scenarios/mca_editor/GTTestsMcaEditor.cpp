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


#include "GTTestsMcaEditor.h"
#include "runnables/ugene/plugins/external_tools/AlignToReferenceBlastDialogFiller.h"

#include "GTUtilsTaskTreeView.h"
#include "GTUtilsLog.h"
#include "GTUtilsNotifications.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"


namespace U2 {

namespace GUITest_common_scenarios_mca_editor {
//using namespace HI;

GUI_TEST_CLASS_DEFINITION(test_0001) {
    //1. Select "Tools>Sanger data analysis>Reads quality control and alignment"
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Reads quality control and alignment...");
    GTGlobals::sleep();

    //    Expected state : "Align Sanger Reads to Reference" has appered
    //    Expected state : "Min read identity" option by default = 80 %
    int minReadIdentity = GTSpinBox::getValue(os, "minIdentitySpinBox");
    QString expected = "80";
    CHECK_SET_ERR(QString::number(minReadIdentity) == expected, QString("incorrect Read Identity value: expected 80%, got %1").arg(minReadIdentity));

    //    Expected state : "Min read length" option by default = 0
    int minLen = GTSpinBox::getValue(os, "minLenSpinBox");
    expected = "0";
    CHECK_SET_ERR(QString::number(minLen) == expected, QString("incorrect min Lenght value: expected 0, got %1").arg(minLen));

    //    Expected state : "Quality threshold" option by default = 30
    int quality = GTSpinBox::getValue(os, "qualitySpinBox");
    expected = "30";
    CHECK_SET_ERR(QString::number(quality) == expected, QString("incorrect quality value: expected 30, got %1").arg(quality));

    //    Expected state : "Add to project" option is checked by default
    bool addToProject = GTCheckBox::getState(os, "addToProjectCheckbox");
    CHECK_SET_ERR(addToProject, QString("incorrect addToProject state: expected true, got false"));

    //    Expected state : "Result aligment" field is filled by default
    QString output = GTLineEdit::getText(os, "outputLineEdit");
    bool checkOutput = output.isEmpty();
    CHECK_SET_ERR(!checkOutput, QString("incorrect output line: is empty"));

    //    Expected state : "Trim both ends" option is checked by default
    bool trim = GTCheckBox::getState(os, "trimCheckBox");
    CHECK_SET_ERR(trim, QString("incorrect addToProject state: expected true, got false"));

    //    Expected state : "Result alignment" is pre - filled <path> / Documents / UGENE_Data / reference_sanger_reads_alignment.ugenedb]
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
    GTUtilsTaskTreeView::waitTaskFinished(os);


    //Expected state :
    //"reference_sanger_reads_alignment.ugenedb" in the Project View with object :
    //-{'mc' Aligned reads} for multiple chromatogram alignment object in Project View

   // GTUtilsProject::checkProject(os);
    //GTUtilsProjectTreeView::openView(os);
   // bool check = GTUtilsProjectTreeView::checkItem(os, "[mc] Aligned reads");
    //GTUtilsProjectTreeView::
    //    Expected state : Aligment editor has been opened

    //    Expected state : Reference name is  "KM099231" at the editor left corner
    //    Expected state : "Consensus" is placed under reference name
    //    Expected state : 16 reads are present
    //    8 reads with names "SZYD_Cas9_CR60"..."SZYD_Cas9_CR66" and "SZYD_Cas9_CR71"
    //    8 reverse reads with names "SZYD_Cas9_CR50(rev-compl)"... "SZYD_Cas9_CR56(rev-compl)" and "SZYD_Cas9_CR70(rev-compl)"

    //5. Report with info
    //GTUtilsNotifications::waitForNotification()
    //    number of successfully aligned sequences = 16
    //GTUtilsNotifications::waitForNotification(os);
    //GTUtilsTaskTreeView::waitTaskFinished(os);

    //GTUtilsNotifications::waitForNotification(os);
    //GTUtilsTaskTreeView::waitTaskFinished(os);

    //    number of filtered sequences with the names : 4
    GTUtilsNotifications::waitForNotification(os, true, "Filtered by quality (4)");
    //GTUtilsTaskTreeView::waitTaskFinished(os);

    //    No Еrrors in the Log
    //QStringList errors = GTUtilsLog::getErrors(os, GTLogTracer("error"));
    //CHECK_SET_ERR(errors.isEmpty(), QString("Some errors found"));


    //class Scenario : public CustomScenario {
    //    void run(HI::GUITestOpStatus &os) {
    //    }
    //}

}











}//namespace U2

}//namespace
