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

#include <QApplication>
#include <QClipboard>
#include <QTreeWidgetItem>

#include "GTGlobals.h"

#include "GTTestsSequenceEditMode.h"

#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMdi.h"
#include "GTUtilsNotifications.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsTaskTreeView.h"

#include "system/GTClipboard.h"

#include "utils/GTKeyboardUtils.h"
#include "utils/GTUtilsApp.h"
#include "utils/GTUtilsToolTip.h"

#include <base_dialogs/GTFileDialog.h>

#include <drivers/GTKeyboardDriver.h>
#include <drivers/GTMouseDriver.h>

#include "primitives/GTMenu.h"
#include "primitives/PopupChooser.h"
#include <primitives/GTAction.h>
#include <primitives/GTTreeWidget.h>

#include <U2Core/U2IdTypes.h>


namespace U2{

namespace GUITest_common_scenarios_sequence_edit_mode {
using namespace HI;

GUI_TEST_CLASS_DEFINITION(without_anns_test_0001) {
    //1. Open human_T1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Uncheck "Wrap sequence" and "Show compliment strans" button
    QAction* wrapMode = GTAction::findActionByText(os, "Wrap sequence");
    CHECK_SET_ERR(wrapMode != NULL, "Cannot find Wrap sequence action");
    if (wrapMode->isChecked()) {
        GTWidget::click(os, GTAction::button(os, wrapMode));
    }

    QAction* compStrand = GTAction::findActionByText(os, "Show complementary strand");
    CHECK_SET_ERR(compStrand != NULL, "Cannot find Wrap sequence action");
    if (compStrand->isChecked()) {
        GTWidget::click(os, GTAction::button(os, compStrand));
    }

    //3. Check "Edit sequence" button
    GTUtilsSequenceView::enableEditingMode(os);

    //Expected state : Blinked cursor at the sequence  beggining
    const qint64 pos = GTUtilsSequenceView::getCursor(os);
    CHECK_SET_ERR(pos == 0, QString("Unexpected cursor pos, expected: 0, current %1").arg(pos));

    //4. Print "A, C, G, T, N, gap" symbols
    GTKeyboardDriver::keyClick('A');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('C');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('G');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('T');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('N');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick(Qt::Key_Space);
    GTGlobals::sleep(100);

    //Expected state: Sequence starts with "A,C, G,T,N,gap"
    QString string = GTUtilsSequenceView::getRegionAsString(os, U2Region(1, 6));
    CHECK_SET_ERR(string == "ACGTN-", QString("Unexpected string in the begginning of the sequence, expected: ACGTN-, current: %1").arg(string));

    //5. Put cursor in "199 939" position and press "A,C, G,T,N,gap"
    GTUtilsSequenceView::setCursor(os, 199939);

    GTKeyboardDriver::keyClick('A');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('C');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('G');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('T');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('N');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick(Qt::Key_Space);
    GTGlobals::sleep(100);

    //Expected state : Sequence ends with "A,C, G,T,N,gap"
    string = GTUtilsSequenceView::getRegionAsString(os, U2Region(199939, 6));
    CHECK_SET_ERR(string == "ACGTN-", QString("Unexpected string in the ending of the sequence, expected: ACGTN-, current: %1").arg(string));
}

GUI_TEST_CLASS_DEFINITION(without_anns_test_0002) {
    //1. Open human_T1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Uncheck "Wrap sequence" and "Show compliment strans" button
    QAction* wrapMode = GTAction::findActionByText(os, "Wrap sequence");
    CHECK_SET_ERR(wrapMode != NULL, "Cannot find Wrap sequence action");
    if (wrapMode->isChecked()) {
        GTWidget::click(os, GTAction::button(os, wrapMode));
    }

    QAction* compStrand = GTAction::findActionByText(os, "Show complementary strand");
    CHECK_SET_ERR(compStrand != NULL, "Cannot find Wrap sequence action");
    if (compStrand->isChecked()) {
        GTWidget::click(os, GTAction::button(os, compStrand));
    }

    //3. Check "Edit sequence" button
    GTUtilsSequenceView::enableEditingMode(os);

    //Expected state : Blinked cursor at the sequence  beggining
    const qint64 pos = GTUtilsSequenceView::getCursor(os);
    CHECK_SET_ERR(pos == 0, QString("Unexpected cursor pos, expected: 0, current %1").arg(pos));

    //4. Print "QWER" symbols
    GTKeyboardDriver::keyClick('Q');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('W');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('E');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('R');
    GTGlobals::sleep(100);

    //Expected state: Sequence starts with "A,C, G,T,N,gap"
    QString string = GTUtilsSequenceView::getRegionAsString(os, U2Region(1, 4));
    bool hasUnexpectedSymbols = string.contains("Q") || string.contains("W") || string.contains("E") || string.contains("R");
    CHECK_SET_ERR(!hasUnexpectedSymbols, "Some unexpected symbols was inserted");

}

GUI_TEST_CLASS_DEFINITION(without_anns_test_0003) {
    //1. Open human_T1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Uncheck "Wrap sequence" and "Show compliment strans" button
    QAction* wrapMode = GTAction::findActionByText(os, "Wrap sequence");
    CHECK_SET_ERR(wrapMode != NULL, "Cannot find Wrap sequence action");
    if (wrapMode->isChecked()) {
        GTWidget::click(os, GTAction::button(os, wrapMode));
    }

    QAction* compStrand = GTAction::findActionByText(os, "Show complementary strand");
    CHECK_SET_ERR(compStrand != NULL, "Cannot find Wrap sequence action");
    if (compStrand->isChecked()) {
        GTWidget::click(os, GTAction::button(os, compStrand));
    }

    //3. Check "Edit sequence" button
    GTUtilsSequenceView::enableEditingMode(os);

    //Expected state : Blinked cursor at the sequence  beggining
    const qint64 pos = GTUtilsSequenceView::getCursor(os);
    CHECK_SET_ERR(pos == 0, QString("Unexpected cursor pos, expected: 0, current %1").arg(pos));

    //4. Print "-AAA" symbols
    GTKeyboardDriver::keyClick(Qt::Key_Space);
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('A');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('A');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('A');
    GTGlobals::sleep(100);

    //Expected state: Sequence starts with "-AAA"
    QString string = GTUtilsSequenceView::getRegionAsString(os, U2Region(1, 4));
    CHECK_SET_ERR(string == "-AAA", QString("Unexpected string, expected: -AAA, current: %1").arg(string));


    //5. Put cursor after "AAA" and push Bàckspañå 3 times
    GTUtilsSequenceView::setCursor(os, 3);
    GTKeyboardDriver::keyClick(Qt::Key_Delete);
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick(Qt::Key_Delete);
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick(Qt::Key_Delete);
    GTGlobals::sleep(100);

    //Expected state : Sequence ends with "-"
    string = GTUtilsSequenceView::getRegionAsString(os, U2Region(1, 4));
    CHECK_SET_ERR(string[0] == '-', QString("Unexpected symbol at the beginning, expected: -, current: %1").arg(string[0]));
    CHECK_SET_ERR(string == "-TTG", QString("Unexpected string, expected: -TTG, current: %1").arg(string));

    //6. Print "---" symbols at the beggining and put cursor before gap
    GTKeyboardDriver::keyClick(Qt::Key_Space);
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick(Qt::Key_Space);
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick(Qt::Key_Space);
    GTGlobals::sleep(100);
    GTUtilsSequenceView::setCursor(os, 0);

    //7. Push Bàckspañå 3 times
    GTKeyboardDriver::keyClick(Qt::Key_Backspace);
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick(Qt::Key_Backspace);
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick(Qt::Key_Backspace);
    GTGlobals::sleep(100);

    //Expected state : Nothing happens
    string = GTUtilsSequenceView::getRegionAsString(os, U2Region(1, 4));
    CHECK_SET_ERR(string == "----", QString("Unexpected string, expected: ----, current: %1").arg(string));

    //8. Push Detete 3 times
    GTKeyboardDriver::keyClick(Qt::Key_Delete);
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick(Qt::Key_Delete);
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick(Qt::Key_Delete);
    GTGlobals::sleep(100);

    //Expected state : 3 gaps are deleted
    string = GTUtilsSequenceView::getRegionAsString(os, U2Region(1, 4));
    CHECK_SET_ERR(string[0] == '-', QString("Unexpected symbol at the beginning, expected: -, current: %1").arg(string[0]));
    CHECK_SET_ERR(string == "-TTG", QString("Unexpected string, expected: -TTG, current: %1").arg(string));
}

GUI_TEST_CLASS_DEFINITION(without_anns_test_0004) {
    //1. Open human_T1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Uncheck "Wrap sequence" and "Show compliment strans" button
    QAction* wrapMode = GTAction::findActionByText(os, "Wrap sequence");
    CHECK_SET_ERR(wrapMode != NULL, "Cannot find Wrap sequence action");
    if (wrapMode->isChecked()) {
        GTWidget::click(os, GTAction::button(os, wrapMode));
    }

    QAction* compStrand = GTAction::findActionByText(os, "Show complementary strand");
    CHECK_SET_ERR(compStrand != NULL, "Cannot find Wrap sequence action");
    if (compStrand->isChecked()) {
        GTWidget::click(os, GTAction::button(os, compStrand));
    }

    //3. Check "Edit sequence" button
    GTUtilsSequenceView::enableEditingMode(os);

    //Expected state : Blinked cursor at the sequence  beggining
    const qint64 pos = GTUtilsSequenceView::getCursor(os);
    CHECK_SET_ERR(pos == 0, QString("Unexpected cursor pos, expected: 0, current %1").arg(pos));

    //4. Print "A, C, G, T, N, gap" symbols
    GTKeyboardDriver::keyClick('A');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('C');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('G');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('T');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick('N');
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick(Qt::Key_Space);
    GTGlobals::sleep(100);

    //Expected state: Sequence starts with "A,C, G,T,N,gap"
    QString string = GTUtilsSequenceView::getRegionAsString(os, U2Region(1, 6));
    CHECK_SET_ERR(string == "ACGTN-", QString("Unexpected string in the begginning of the sequence, expected: ACGTN-, current: %1").arg(string));

    //5. Select these 6 symbols and do ÑTRL+C
    GTUtilsSequenceView::selectSequenceRegion(os, 1, 6);
    GTKeyboardUtils::copy(os);

    //6. Put cursor in "199 939" position and do ÑTRL + V
    GTUtilsSequenceView::setCursor(os, 199939);
    GTKeyboardUtils::paste(os);

    //Expected state : Sequence ends with "A,C, G,T,N,gap"
    string = GTUtilsSequenceView::getRegionAsString(os, U2Region(199939, 6));
    CHECK_SET_ERR(string == "ACGTN-", QString("Unexpected string in the ending of the sequence, expected: ACGTN-, current: %1").arg(string));
}


} // namespace

} // namespace U2
