/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2019 UniPro <ugene@unipro.ru>
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
#include <QDir>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QWizard>

#include <base_dialogs/DefaultDialogFiller.h>
#include <base_dialogs/MessageBoxFiller.h>
#include <drivers/GTKeyboardDriver.h>
#include <drivers/GTMouseDriver.h>
#include <primitives/GTCheckBox.h>
#include <primitives/GTComboBox.h>
#include <primitives/GTGroupBox.h>
#include <primitives/GTLineEdit.h>
#include "primitives/GTMainWindow.h"
#include <primitives/GTMenu.h>
#include <primitives/GTRadioButton.h>
#include <primitives/GTSpinBox.h>
#include <primitives/GTTableView.h>
#include <primitives/GTTabWidget.h>
#include <primitives/GTTextEdit.h>
#include <primitives/GTToolbar.h>
#include <primitives/GTTreeWidget.h>
#include <primitives/PopupChooser.h>
#include <system/GTClipboard.h>
#include <system/GTFile.h>
#include <utils/GTKeyboardUtils.h>
#include <utils/GTThread.h>

#include <U2Core/HttpFileAdapter.h>

#include <U2View/DetView.h>

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

#include "../../workflow_designer/src/WorkflowViewItems.h"

#include "runnables/ugene/corelibs/U2Gui/AlignShortReadsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/AppSettingsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/DownloadRemoteFileDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditAnnotationDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditSettingsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/FindRepeatsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ImportAPRFileDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/ExtractSelectedAsMSADialogFiller.h"
#include "runnables/ugene/corelibs/U2View/utils_smith_waterman/SmithWatermanDialogBaseFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSelectedSequenceFromAlignmentDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSequencesDialogFiller.h"
#include "runnables/ugene/plugins/enzymes/ConstructMoleculeDialogFiller.h"
#include "runnables/ugene/plugins/enzymes/DigestSequenceDialogFiller.h"
#include "runnables/ugene/plugins/enzymes/FindEnzymesDialogFiller.h"
#include "runnables/ugene/plugins/external_tools/AlignToReferenceBlastDialogFiller.h"
#include "runnables/ugene/plugins/external_tools/TrimmomaticDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/WizardFiller.h"
#include "runnables/ugene/ugeneui/DocumentFormatSelectorDialogFiller.h"
#include "runnables/ugene/ugeneui/SaveProjectDialogFiller.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/CreateElementWithCommandLineToolFiller.h"
#include "runnables/ugene/plugins/workflow_designer/StartupDialogFiller.h"

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

GUI_TEST_CLASS_DEFINITION(test_6033) {
    //    1. Open 'human_T1.fa'
    GTFileDialog::openFile(os, dataDir + "/samples/FASTA", "human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Select the whole sequence.
    GTUtilsSequenceView::selectSequenceRegion(os, 1, 199950);

    //3. Copy translation.
    GTKeyboardDriver::keyClick('t', Qt::ControlModifier);

    //4. Open the Project View, paste the data.
    if (!GTUtilsProjectTreeView::isVisible(os)) {
        GTUtilsProjectTreeView::openView(os);
    }

    GTUtilsProjectTreeView::click(os, "human_T1.fa");
    GTKeyboardDriver::keyClick('v', Qt::ControlModifier);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    bool correct = false;
    foreach(const QString& name, GTUtilsProjectTreeView::getDocuments(os).keys()) {
        if (name.contains("clipboard") && name.contains(".seq")) {
            correct = true;
            break;
        }
    }

    CHECK_SET_ERR(correct, "Incorrect paste operation");
}
GUI_TEST_CLASS_DEFINITION(test_6038_1) {
    //    1. Open WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //    2. Add 'Improve Reads with Trimmomatic', 'Classify Sequences with CLARK', 'Classify Sequences with Kraken' and 'Filter by Classification' elements to the scene.
    const QString trimmomaticName = "Improve Reads with Trimmomatic";
    WorkflowProcessItem *trimmomaticElement = GTUtilsWorkflowDesigner::addElement(os, trimmomaticName);

    //    3. Each element has 'Input data' parameter. Set it to 'PE reads'.
    //    Expected state: 'Improve Reads with Trimmomatic' element has two input slots ('Input FASTQ URL 1' and 'Input FASTQ URL 2') and two output slots ('Output FASTQ URL 1' and 'Output FASTQ URL 2');
    {
        GTUtilsWorkflowDesigner::click(os, trimmomaticElement);
        GTUtilsWorkflowDesigner::setParameter(os, "Input data", "PE reads", GTUtilsWorkflowDesigner::comboValue);

        QTableWidget *inputPortTable = GTUtilsWorkflowDesigner::getInputPortsTable(os, 0);
        CHECK_SET_ERR(NULL != inputPortTable, "inputPortTable is NULL");

        QStringList inputSlotsNames;
        for (int i = 0; i < GTTableView::rowCount(os, inputPortTable); i++) {
            inputSlotsNames << GTTableView::data(os, inputPortTable, i, 0);
        }

        CHECK_SET_ERR(inputSlotsNames.contains("Input FASTQ URL 1"), QString("'Input FASTQ URL 1' slot not found in element '%1'").arg(trimmomaticName));
        CHECK_SET_ERR(inputSlotsNames.contains("Input FASTQ URL 2"), QString("'Input FASTQ URL 2' slot not found in element '%1'").arg(trimmomaticName));

        QTableWidget *outputPortTable = GTUtilsWorkflowDesigner::getOutputPortsTable(os, 0);
        CHECK_SET_ERR(NULL != outputPortTable, "outputPortTable is NULL");

        QStringList outputSlotsNames;
        for (int i = 0; i < GTTableView::rowCount(os, outputPortTable); i++) {
            outputSlotsNames << GTTableView::data(os, outputPortTable, i, 0);
        }

        CHECK_SET_ERR(outputSlotsNames.contains("Output FASTQ URL 1 (by Improve Reads with Trimmomatic)"), QString("'Output FASTQ URL 1 (by Improve Reads with Trimmomatic)' slot not found in element '%1'").arg(trimmomaticName));
        CHECK_SET_ERR(outputSlotsNames.contains("Output FASTQ URL 2 (by Improve Reads with Trimmomatic)"), QString("'Output FASTQ URL 2 (by Improve Reads with Trimmomatic)' slot not found in element '%1'").arg(trimmomaticName));
    }
    //    4. Set 'Input data' parameter in each element to 'SE reads' in 'Improve Reads with Trimmomatic' element.
    //    Expected state: 'Improve Reads with Trimmomatic' element has one input slot ('Input FASTQ URL 1') and one output slot ('Output FASTQ URL 1');
    //
    {
        GTWidget::click(os, GTWidget::findWidget(os, "sceneView"));
        GTUtilsWorkflowDesigner::click(os, trimmomaticElement);
        GTUtilsWorkflowDesigner::setParameter(os, "Input data", "SE reads", GTUtilsWorkflowDesigner::comboValue);

        QTableWidget *inputPortTable = GTUtilsWorkflowDesigner::getInputPortsTable(os, 0);
        CHECK_SET_ERR(NULL != inputPortTable, "inputPortTable is NULL");

        QStringList inputSlotsNames;
        for (int i = 0; i < GTTableView::rowCount(os, inputPortTable); i++) {
            inputSlotsNames << GTTableView::data(os, inputPortTable, i, 0);
        }

        CHECK_SET_ERR(inputSlotsNames.contains("Input FASTQ URL 1"), QString("'Input FASTQ URL 1' slot not found in element '%1'").arg(trimmomaticName));
        CHECK_SET_ERR(!inputSlotsNames.contains("Input FASTQ URL 2"), QString("'Input FASTQ URL 2' slot unexpectedly found in element '%1'").arg(trimmomaticName));

        QTableWidget *outputPortTable = GTUtilsWorkflowDesigner::getOutputPortsTable(os, 0);
        CHECK_SET_ERR(NULL != outputPortTable, "outputPortTable is NULL");

        QStringList outputSlotsNames;
        for (int i = 0; i < GTTableView::rowCount(os, outputPortTable); i++) {
            outputSlotsNames << GTTableView::data(os, outputPortTable, i, 0);
        }

        CHECK_SET_ERR(outputSlotsNames.contains("Output FASTQ URL 1 (by Improve Reads with Trimmomatic)"), QString("'Output FASTQ URL 1 (by Improve Reads with Trimmomatic)' slot not found in element '%1'").arg(trimmomaticName));
        CHECK_SET_ERR(!outputSlotsNames.contains("Output FASTQ URL 2 (by Improve Reads with Trimmomatic)"), QString("'Output FASTQ URL 2 (by Improve Reads with Trimmomatic)' slot unexpectedly found in element '%1'").arg(trimmomaticName));

    }

    //    5. Click 'Validate workflow' button on the toolbar.
    GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTUtilsWorkflowDesigner::validateWorkflow(os);

    //    Expected state: there could be errors, but there are neither errors not warnings about not connected slots.
    QSet<QString> acceptableErrors = QSet<QString>()
        << QString("%1: The mandatory \"Input FASTQ URL 1\" slot is not connected.").arg(trimmomaticName)
        << QString("%1: Required parameter is not set: Trimming steps").arg(trimmomaticName);

    QSet<QString> actualErrors = GTUtilsWorkflowDesigner::getErrors(os).toSet();
    CHECK_SET_ERR(acceptableErrors.size() == actualErrors.size(), QString("Unexpected errors number, expected: %1, current: %2").
        arg(acceptableErrors.size()).
        arg(actualErrors.size()));
}

GUI_TEST_CLASS_DEFINITION(test_6038_2) {
    //    1. Open WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //    2. Add 'Improve Reads with Trimmomatic', 'Classify Sequences with CLARK', 'Classify Sequences with Kraken' and 'Filter by Classification' elements to the scene.
    const QString clarkName = "Classify Sequences with CLARK";

    WorkflowProcessItem *clarkElement = GTUtilsWorkflowDesigner::addElement(os, clarkName);

    //    3. Each element has 'Input data' parameter. Set it to 'PE reads'.
    //    Expected state: 'Classify Sequences with CLARK' elements have two input slots ('Input URL 1' and 'Input URL 2') and some output slots;

        GTUtilsWorkflowDesigner::click(os, clarkElement);
        GTUtilsWorkflowDesigner::setParameter(os, "Input data", "PE reads", GTUtilsWorkflowDesigner::comboValue);

        QTableWidget *inputPortTable = GTUtilsWorkflowDesigner::getInputPortsTable(os, 0);
        CHECK_SET_ERR(NULL != inputPortTable, "inputPortTable is NULL");

        QStringList inputSlotsNames;
        for (int i = 0; i < GTTableView::rowCount(os, inputPortTable); i++) {
            inputSlotsNames << GTTableView::data(os, inputPortTable, i, 0);
        }

        CHECK_SET_ERR(inputSlotsNames.contains("Input URL 1"), QString("'Input URL 1' slot not found in element '%1'").arg(clarkName));
        CHECK_SET_ERR(inputSlotsNames.contains("Input URL 2"), QString("'Input URL 2' slot not found in element '%1'").arg(clarkName));

        //    4. Set 'Input data' parameter in each element to 'SE reads or contigs' ('SE reads' in 'Improve Reads with Trimmomatic' element).
        //    Expected state: 'Classify Sequences with Kraken' and 'Classify Sequences with CLARK' elements have one input slot ('Input URL 1') and some output slots;
        //                    'Improve Reads with Trimmomatic' element has one input slot ('Input FASTQ URL 1') and one output slot ('Output FASTQ URL 1');
        //                    'Filter by Classification' element has two input slots (one of them is 'Input URL 1'), and one output slot ('Output URL 1').
        {
            GTWidget::click(os, GTWidget::findWidget(os, "sceneView"));
            GTUtilsWorkflowDesigner::click(os, clarkElement);
            GTUtilsWorkflowDesigner::setParameter(os, "Input data", "SE reads or contigs", GTUtilsWorkflowDesigner::comboValue);

            QTableWidget *inputPortTable = GTUtilsWorkflowDesigner::getInputPortsTable(os, 0);
            CHECK_SET_ERR(NULL != inputPortTable, "inputPortTable is NULL");

            QStringList inputSlotsNames;
            for (int i = 0; i < GTTableView::rowCount(os, inputPortTable); i++) {
                inputSlotsNames << GTTableView::data(os, inputPortTable, i, 0);
            }

            CHECK_SET_ERR(inputSlotsNames.contains("Input URL 1"), QString("'Input URL 1' slot not found in element '%1'").arg(clarkName));
            CHECK_SET_ERR(!inputSlotsNames.contains("Input URL 2"), QString("'Input URL 2' slot unexpectedly found in element '%1'").arg(clarkName));
        }
    //    5. Click 'Validate workflow' button on the toolbar.
    GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTUtilsWorkflowDesigner::validateWorkflow(os);

    //    Expected state: there could be errors, but there are neither errors not warnings about not connected slots.
    QSet<QString> acceptableErrors;

#ifdef Q_OS_WIN
    acceptableErrors = QSet<QString>()
        << QString("%1: The mandatory \"Input URL 1\" slot is not connected.").arg(clarkName)
        << QString("%1: External tool \"CLARK\" is not set. You can set it in Settings -> Preferences -> External Tools").arg(clarkName)
        << QString("%1: External tool \"CLARK-l\" is not set. You can set it in Settings -> Preferences -> External Tools").arg(clarkName)
        << QString("%1: Required parameter is not set: Database").arg(clarkName);
#else
    acceptableErrors = QSet<QString>()
        << QString("%1: The mandatory \"Input URL 1\" slot is not connected.").arg(clarkName)
        << QString("%1: Required parameter is not set: Database").arg(clarkName);
#endif

    QSet<QString> actualErrors = GTUtilsWorkflowDesigner::getErrors(os).toSet();
    CHECK_SET_ERR(acceptableErrors.size() == actualErrors.size(), QString("Unexpected errors number, expected: %1, current: %2").
        arg(acceptableErrors.size()).
        arg(actualErrors.size()));
}

GUI_TEST_CLASS_DEFINITION(test_6038_4) {

        //    1. Open WD.
        GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

        //    2. Add 'Filter by Classification' element to the scene.
        const QString filterName = "Filter by Classification";

        WorkflowProcessItem *filterElement = GTUtilsWorkflowDesigner::addElement(os, filterName);

        //    3. Each element has 'Input data' parameter. Set it to 'PE reads'.
        //    Expected state:  'Filter by Classification' element has three input slots (two of them are 'Input URL 1' and 'Input URL 2'), and two output slots ('Output URL 1' and 'Output URL 2').

    {
        GTUtilsWorkflowDesigner::click(os, filterElement);
        GTUtilsWorkflowDesigner::setParameter(os, "Input data", "PE reads", GTUtilsWorkflowDesigner::comboValue);

        QTableWidget *inputPortTable = GTUtilsWorkflowDesigner::getInputPortsTable(os, 0);
        CHECK_SET_ERR(NULL != inputPortTable, "inputPortTable is NULL");

        QStringList inputSlotsNames;
        for (int i = 0; i < GTTableView::rowCount(os, inputPortTable); i++) {
            inputSlotsNames << GTTableView::data(os, inputPortTable, i, 0);
        }

        CHECK_SET_ERR(inputSlotsNames.contains("Input URL 1"), QString("'Input URL 1' slot not found in element '%1'").arg(filterName));
        CHECK_SET_ERR(inputSlotsNames.contains("Input URL 2"), QString("'Input URL 2' slot not found in element '%1'").arg(filterName));

        QTableWidget *outputPortTable = GTUtilsWorkflowDesigner::getOutputPortsTable(os, 0);
        CHECK_SET_ERR(NULL != outputPortTable, "outputPortTable is NULL");

        QStringList outputSlotsNames;
        for (int i = 0; i < GTTableView::rowCount(os, outputPortTable); i++) {
            outputSlotsNames << GTTableView::data(os, outputPortTable, i, 0);
        }

        CHECK_SET_ERR(outputSlotsNames.contains("Output URL 1 (by Filter by Classification)"), QString("'Output URL 1 (by Filter by Classification)' slot not found in element '%1'").arg(filterName));
        CHECK_SET_ERR(outputSlotsNames.contains("Output URL 2 (by Filter by Classification)"), QString("'Output URL 2 (by Filter by Classification)' slot not found in element '%1'").arg(filterName));
    }

    //    4. Set 'Input data' parameter in each element to 'SE reads or contigs' ('SE reads' in 'Improve Reads with Trimmomatic' element).
    //    Expected state: 'Filter by Classification' element has two input slots (one of them is 'Input URL 1'), and one output slot ('Output URL 1').

    {
        GTWidget::click(os, GTWidget::findWidget(os, "sceneView"));
        GTUtilsWorkflowDesigner::click(os, filterElement);
        GTUtilsWorkflowDesigner::setParameter(os, "Input data", "SE reads or contigs", GTUtilsWorkflowDesigner::comboValue);

        QTableWidget *inputPortTable = GTUtilsWorkflowDesigner::getInputPortsTable(os, 0);
        CHECK_SET_ERR(NULL != inputPortTable, "inputPortTable is NULL");

        QStringList inputSlotsNames;
        for (int i = 0; i < GTTableView::rowCount(os, inputPortTable); i++) {
            inputSlotsNames << GTTableView::data(os, inputPortTable, i, 0);
        }

        CHECK_SET_ERR(inputSlotsNames.contains("Input URL 1"), QString("'Input URL 1' slot not found in element '%1'").arg(filterName));
        CHECK_SET_ERR(!inputSlotsNames.contains("Input URL 2"), QString("'Input URL 2' slot unexpectedly found in element '%1'").arg(filterName));

        QTableWidget *outputPortTable = GTUtilsWorkflowDesigner::getOutputPortsTable(os, 0);
        CHECK_SET_ERR(NULL != outputPortTable, "outputPortTable is NULL");

        QStringList outputSlotsNames;
        for (int i = 0; i < GTTableView::rowCount(os, outputPortTable); i++) {
            outputSlotsNames << GTTableView::data(os, outputPortTable, i, 0);
        }

        CHECK_SET_ERR(outputSlotsNames.contains("Output URL 1 (by Filter by Classification)"), QString("'Output URL 1 (by Filter by Classification)' slot not found in element '%1'").arg(filterName));
        CHECK_SET_ERR(!outputSlotsNames.contains("Output URL 2 (by Filter by Classification)"), QString("'Output URL 2 (by Filter by Classification)' slot unexpectedly found in element '%1'").arg(filterName));
    }

    //    5. Click 'Validate workflow' button on the toolbar.
    GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTUtilsWorkflowDesigner::validateWorkflow(os);

    //    Expected state: there could be errors, but there are neither errors not warnings about not connected slots.
    QSet<QString> acceptableErrors = QSet<QString>()
        << QString("%1: The mandatory \"Input URL 1\" slot is not connected.").arg(filterName)
        << QString("%1: Taxonomy classification data from NCBI are not available").arg(filterName);
    QSet<QString> actualErrors = GTUtilsWorkflowDesigner::getErrors(os).toSet();
    CHECK_SET_ERR(acceptableErrors.size() == actualErrors.size(), QString("Unexpected errors number, expected: %1, current: %2").
        arg(acceptableErrors.size()).
        arg(actualErrors.size()));
}

GUI_TEST_CLASS_DEFINITION(test_6038_3) {

        //    1. Open WD.
        GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

        //    2. Add 'Classify Sequences with Kraken' element to the scene.
        const QString krakenName = "Classify Sequences with Kraken";
        WorkflowProcessItem *krakenElement = GTUtilsWorkflowDesigner::addElement(os, krakenName);


        //    3. Each element has 'Input data' parameter. Set it to 'PE reads'.
        //    Expected state: 'Classify Sequences with Kraken' element has two input slots ('Input URL 1' and 'Input URL 2') and some output slots;

    {
        GTUtilsWorkflowDesigner::click(os, krakenElement);
        GTUtilsWorkflowDesigner::setParameter(os, "Input data", "PE reads", GTUtilsWorkflowDesigner::comboValue);

        QTableWidget *inputPortTable = GTUtilsWorkflowDesigner::getInputPortsTable(os, 0);
        CHECK_SET_ERR(NULL != inputPortTable, "inputPortTable is NULL");

        QStringList inputSlotsNames;
        for (int i = 0; i < GTTableView::rowCount(os, inputPortTable); i++) {
            inputSlotsNames << GTTableView::data(os, inputPortTable, i, 0);
        }

        CHECK_SET_ERR(inputSlotsNames.contains("Input URL 1"), QString("'Input URL 1' slot not found in element '%1'").arg(krakenName));
        CHECK_SET_ERR(inputSlotsNames.contains("Input URL 2"), QString("'Input URL 2' slot not found in element '%1'").arg(krakenName));
    }

    //    4. Set 'Input data' parameter in each element to 'SE reads or contigs
    //    Expected state: 'Classify Sequences with Kraken' and some output slots;

    {
        GTWidget::click(os, GTWidget::findWidget(os, "sceneView"));
        GTUtilsWorkflowDesigner::click(os, krakenElement);
        GTUtilsWorkflowDesigner::setParameter(os, "Input data", "SE reads or contigs", GTUtilsWorkflowDesigner::comboValue);

        QTableWidget *inputPortTable = GTUtilsWorkflowDesigner::getInputPortsTable(os, 0);
        CHECK_SET_ERR(NULL != inputPortTable, "inputPortTable is NULL");

        QStringList inputSlotsNames;
        for (int i = 0; i < GTTableView::rowCount(os, inputPortTable); i++) {
            inputSlotsNames << GTTableView::data(os, inputPortTable, i, 0);
        }

        CHECK_SET_ERR(inputSlotsNames.contains("Input URL 1"), QString("'Input URL 1' slot not found in element '%1'").arg(krakenName));
        CHECK_SET_ERR(!inputSlotsNames.contains("Input URL 2"), QString("'Input URL 2' slot unexpectedly found in element '%1'").arg(krakenName));
    }

    //    5. Click 'Validate workflow' button on the toolbar.
    GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTUtilsWorkflowDesigner::validateWorkflow(os);

    //    Expected state: there could be errors, but there are neither errors not warnings about not connected slots.
    QSet<QString> acceptableErrors;

#ifdef Q_OS_WIN
    acceptableErrors = QSet<QString>()
        << QString("%1: The mandatory \"Input URL 1\" slot is not connected.").arg(krakenName)
        << QString("%1: External tool \"kraken\" is not set. You can set it in Settings -> Preferences -> External Tools").arg(krakenName)
        << QString("%1: Required parameter is not set: Database").arg(krakenName)
        << QString("%1: The database folder \"\" doesn't exist.").arg(krakenName);
#else
    acceptableErrors = QSet<QString>()
        << QString("%1: The mandatory \"Input URL 1\" slot is not connected.").arg(krakenName)
        << QString("%1: Required parameter is not set: Database").arg(krakenName)
        << QString("%1: The database folder \"\" doesn't exist.").arg(krakenName);
#endif

    QSet<QString> actualErrors = GTUtilsWorkflowDesigner::getErrors(os).toSet();
    CHECK_SET_ERR(acceptableErrors.size() == actualErrors.size(), QString("Unexpected errors number, expected: %1, current: %2").
        arg(acceptableErrors.size()).
        arg(actualErrors.size()));
}

GUI_TEST_CLASS_DEFINITION(test_6043) {
//    1. Open "_common_data/ugenedb/sec1_9_ugenedb.ugenedb".
//    Expected state: the assembly is successfully opened, the coverage calculation finished, UGENE doens't crash
    const QString filePath = sandBoxDir + "test_6043.ugenedb";
    GTFile::copy(os, testDir + "_common_data/ugenedb/sec1_9_ugenedb.ugenedb", filePath);

    GTFileDialog::openFile(os, filePath);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();
    GTUtilsTaskTreeView::waitTaskFinished(os);

    const bool assemblyExists = GTUtilsProjectTreeView::checkItem(os, "sec1_and_others");
    CHECK_SET_ERR(assemblyExists, "Assembly object is not found in the project view");
}

GUI_TEST_CLASS_DEFINITION(test_6047) {
    //1. Open and convert APR file
    GTUtilsDialog::waitForDialog(os, new ImportAPRFileFiller(os, false, sandBoxDir + "test_6047", "MSF"));
    GTFileDialog::openFile(os, testDir + "_common_data/apr/HCVtest.apr");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Check msa length and number of sequences
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getLength(os) == 488, "Unexpected length of msa");
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getNameList(os).size() == 231, "Unexpected quantaty of sequences");
}

GUI_TEST_CLASS_DEFINITION(test_6058_1) {
    //1. Open file human_t1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Check following items in sequence view translations menu
    //"31. Blastocrithidia Nuclear"
    //"28. Condylostoma Nuclear"
    //"30. Peritrich Nuclear"
    //"27. Karyorelict Nuclear"
    //"25. Candidate Division SR1 and Gracilibacteria Code"
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"));
    GTWidget::click(os, GTWidget::findWidget(os, "AminoToolbarButton", GTWidget::findWidget(os, "ADV_single_sequence_widget_0")));
    QMenu *menu = qobject_cast<QMenu *>(QApplication::activePopupWidget());
    QStringList actionText;
    foreach(QAction *a, menu->actions()) {
        actionText.append(a->text());
    }
    CHECK_SET_ERR(actionText.contains("31. Blastocrithidia Nuclear"), "expected translation not found");
    CHECK_SET_ERR(actionText.contains("28. Condylostoma Nuclear"), "expected translation not found");
    CHECK_SET_ERR(actionText.contains("30. Peritrich Nuclear"), "expected translation not found");
    CHECK_SET_ERR(actionText.contains("27. Karyorelict Nuclear"), "expected translation not found");
    CHECK_SET_ERR(actionText.contains("25. Candidate Division SR1 and Gracilibacteria Code"), "expected translation not found");

    //just for closing popup menu
    GTMenu::clickMenuItemByName(os, menu, QStringList() << "14. The Alternative Flatworm Mitochondrial Code");
}

GUI_TEST_CLASS_DEFINITION(test_6058_2) {
    //1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Check "30. Peritrich Nuclear" "Genetic code" parameter option in "Classify Sequences with DIAMOND" WD element
    WorkflowProcessItem *diamondElement = GTUtilsWorkflowDesigner::addElement(os, "Classify Sequences with DIAMOND", true);
    GTGlobals::sleep();
    WorkflowProcessItem *orfMarker = GTUtilsWorkflowDesigner::addElementByUsingNameFilter(os, "ORF Marker");
    GTGlobals::sleep();
    GTUtilsWorkflowDesigner::click(os, diamondElement);
    GTGlobals::sleep();
    GTUtilsWorkflowDesigner::setParameter(os, "Genetic code", "30. Peritrich Nuclear", GTUtilsWorkflowDesigner::comboValue);
    GTGlobals::sleep();

    //3. Check "27. Karyorelict Nuclear" "Genetic code" parameter option in "ORF Marker" WD element
    GTUtilsWorkflowDesigner::click(os, orfMarker);
    GTGlobals::sleep();
    GTUtilsWorkflowDesigner::setParameter(os, "Genetic code", "27. Karyorelict Nuclear", GTUtilsWorkflowDesigner::comboValue);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_6062) {
    class InnerOs : public GUITestOpStatus {
    public:
        void setError(const QString & err) {
            innerError = err;
        }

        QString getError() const {
            return innerError;
        }

        bool hasError() const {
            return !innerError.isEmpty();
        }

        void reset() {
            innerError.clear();
        }

    private:
        QString innerError;
    };

//    1. Open WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    2. Add 'Map Reads with BWA' element to the scene.
    WorkflowProcessItem *bwaElement = GTUtilsWorkflowDesigner::addElement(os, "Map Reads with BWA", true);

//    3. Click to the element.
    GTUtilsWorkflowDesigner::click(os, bwaElement);

//    Expected state: 'Library' attribute has value 'Single-end'; there is one table in 'Input data' widget, which contains information about input ports.
    const QString actualAttributeValue = GTUtilsWorkflowDesigner::getParameter(os, "Library");
    const QString expectedAttributeValue = "Single-end";
    CHECK_SET_ERR(expectedAttributeValue == actualAttributeValue,
                  QString("An unexpected default value of 'Library' attribute: expected '%1', got '%2'")
                  .arg(expectedAttributeValue).arg(actualAttributeValue));

    InnerOs innerOs;

    QTableWidget *inputPortTable1 = GTUtilsWorkflowDesigner::getInputPortsTable(innerOs, 0);
    CHECK_OP_SET_ERR(innerOs, "Table for the first input port not found");
    CHECK_SET_ERR(NULL != inputPortTable1, "inputPortTable1 is NULL");

    QTableWidget *inputPortTable2 = GTUtilsWorkflowDesigner::getInputPortsTable(innerOs, 1);
    CHECK_SET_ERR(innerOs.hasError(), "Table for the second input port unexpectedly found");
    CHECK_SET_ERR(NULL == inputPortTable2, "Table for the second input port unexpectedly found");

    innerOs.reset();

//    4. Set 'Library' attribute value to 'Paired-end'.
    GTUtilsWorkflowDesigner::setParameter(os, "Library", "Paired-end", GTUtilsWorkflowDesigner::comboValue);

//    Expected state: there are two tables in 'Input data' widget.
    inputPortTable1 = GTUtilsWorkflowDesigner::getInputPortsTable(innerOs, 0);
    CHECK_OP_SET_ERR(innerOs, "Table for the first input port not found");
    CHECK_SET_ERR(NULL != inputPortTable1, "inputPortTable1 is NULL");

    inputPortTable2 = GTUtilsWorkflowDesigner::getInputPortsTable(innerOs, 1);
    CHECK_OP_SET_ERR(innerOs, "Table for the second input port not found");
    CHECK_SET_ERR(NULL != inputPortTable2, "Table for the second input port not found");

//    4. Set 'Library' attribute value to 'Single-end'.
    GTUtilsWorkflowDesigner::clickParameter(os, "Output folder");
    GTUtilsWorkflowDesigner::setParameter(os, "Library", "Single-end", GTUtilsWorkflowDesigner::comboValue);

//    Expected state: there is one table in 'Input data' widget.
    inputPortTable1 = GTUtilsWorkflowDesigner::getInputPortsTable(innerOs, 0);
    CHECK_OP_SET_ERR(innerOs, "Table for the first input port not found");
    CHECK_SET_ERR(NULL != inputPortTable1, "inputPortTable1 is NULL");

    inputPortTable2 = GTUtilsWorkflowDesigner::getInputPortsTable(innerOs, 1);
    CHECK_SET_ERR(innerOs.hasError(), "Table for the second input port unexpectedly found");
    CHECK_SET_ERR(NULL == inputPortTable2, "Table for the second input port unexpectedly found");
}

GUI_TEST_CLASS_DEFINITION(test_6066) {
//    1. Open "data/samples/Genbank/murine.gb".
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/murine.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    2. Select "Edit" -> "Annotations settings on sequence editing..." menu item in the Details View context menu.
//    3. Choose "Split (separate annotations parts)" and press "OK".
    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Edit" << "Annotations settings on sequence editing..."));
    GTUtilsDialog::waitForDialog(os, new EditSettingsDialogFiller(os, EditSettingsDialogFiller::SplitSeparateAnnotationParts, false));
    GTWidget::click(os, GTUtilsSequenceView::getDetViewByNumber(os), Qt::RightButton);

//    4. Turn on the editing mode.
    GTUtilsSequenceView::enableEditingMode(os);

//    5. Set cursor after position 60.
    GTUtilsSequenceView::setCursor(os, 60);

//    6. Click Space key.
    GTKeyboardDriver::keyClick(Qt::Key_Space);

//    Expected state: a gap is inserted, the annotation is split.
    // Do not check it here, to avoid view state changing

//    7. Doubleclick the first part if the split annotation and click Delete key.
    GTUtilsSequenceView::clickAnnotationDet(os, "misc_feature", 2, 0, true);
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick(Qt::Key_Delete);
    GTGlobals::sleep();

//    Expected state: the annotation is removed from the Details View and from the Annotations Tree View.
    // Do not check it here, to avoid view state changing

//    8. Doubleclick the second part of the split annotation.
    GTUtilsSequenceView::clickAnnotationDet(os, "misc_feature", 3, 0, true);

//    Expected state: UGENE doesn't crash.
}

GUI_TEST_CLASS_DEFINITION(test_6071) {
    //1. Open "data/samples/Genbank/murine.gb".
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/murine.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Select 2-th CDS annotation in the Zoom view
    GTUtilsSequenceView::clickAnnotationPan(os, "CDS", 2970);

    //3. Scroll to the 3874 coordinate.
    GTUtilsSequenceView::goToPosition(os, 3874);
    GTGlobals::sleep();

    DetView* dw = GTUtilsSequenceView::getDetViewByNumber(os);
    const U2Region firstVisibleRange = dw->getVisibleRange();

    //4. Click on 2-th CDS join annotation
    GTUtilsSequenceView::clickAnnotationDet(os, "CDS", 3412);
    GTGlobals::sleep();

    //Expected: visible range was not changed
    const U2Region secondVisibleRange = dw->getVisibleRange();
    CHECK_SET_ERR(firstVisibleRange == secondVisibleRange, "Visible range was changed after clicking on the annotation");
}

GUI_TEST_CLASS_DEFINITION(test_6078) {
    //1. Open human_T1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Select 1 - 10 chars
    GTUtilsSequenceView::selectSequenceRegion(os, 1, 10);
    GTKeyboardUtils::copy(os);

    //3. Enable edit mode
    GTUtilsSequenceView::enableEditingMode(os);

    //4. Set the cursor to the 5-th pos
    GTUtilsSequenceView::setCursor(os, 5);

    //5. Press paste
    GTKeyboardUtils::paste(os);
    GTGlobals::sleep();

    //Expected: cursor on the 15-th pos
    const qint64 pos = GTUtilsSequenceView::getCursor(os);
    CHECK_SET_ERR(pos == 15, QString("Incorrect cursor position, expected: 15, current: %1").arg(pos));
}

GUI_TEST_CLASS_DEFINITION(test_6083) {
    //    1. open document samples/CLUSTALW/COI.aln
        GTUtilsProject::openFiles(os, dataDir + "samples/CLUSTALW/COI.aln");
    //    2. Select first sequence
        GTUtilsMSAEditorSequenceArea::click(os,QPoint(0,0));
        GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_EXPORT<<"Save sequence",GTGlobals::UseKey));
        Runnable* r = new ExportSelectedSequenceFromAlignment(os,testDir + "_common_data/scenarios/sandbox/",ExportSelectedSequenceFromAlignment::Ugene_db,true);
        GTUtilsDialog::waitForDialog(os, r);

        GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
        GTGlobals::sleep();
        GTUtilsTaskTreeView::waitTaskFinished(os);

        GTUtilsProjectTreeView::click(os, "Phaneroptera_falcata.ugenedb");
        GTKeyboardDriver::keyClick( Qt::Key_Delete);
        GTGlobals::sleep();
}


GUI_TEST_CLASS_DEFINITION(test_6087) {
    //1. Open  samples/MMDB/1CRN.prt
    GTFileDialog::openFile(os, dataDir + "samples/MMDB/1CRN.prt");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Try to select a region.
    GTUtilsSequenceView::selectSequenceRegion(os, 10, 20);

    //Expected: ugene was not crashed
    QVector<U2Region> regions = GTUtilsSequenceView::getSelection(os);
    CHECK_SET_ERR(regions.size() == 1, "Unexpected selection");
}

GUI_TEST_CLASS_DEFINITION(test_6102) {
    // 1. Open "data/samples/Genbank/murine.gb".
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/murine.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //    2) Run Smith-waterman search using:
        class Scenario : public CustomScenario {
            void run(HI::GUITestOpStatus &os) {
                QWidget *dialog = QApplication::activeModalWidget();
                CHECK_SET_ERR(NULL != dialog, "Active modal widget is NULL");
                GTTextEdit::setText(os, GTWidget::findExactWidget<QTextEdit *>(os, "teditPattern", dialog), "RPHP*VAS*LK*RHFARHGKIHN*E*KSSDQGQ");

                GTRadioButton::click(os, "radioTranslation", dialog);

                GTTabWidget::setCurrentIndex(os, GTWidget::findExactWidget<QTabWidget *>(os, "tabWidget", dialog), 1);
                //    3. Open tab "Input and output"
                            GTTabWidget::setCurrentIndex(os, GTWidget::findExactWidget<QTabWidget *>(os, "tabWidget", dialog), 1);

                //    4. Chose in the combobox "Multiple alignment"
                            GTComboBox::setIndexWithText(os, GTWidget::findExactWidget<QComboBox *>(os, "resultViewVariants", dialog), "Multiple alignment");
                GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
            }
        };

        GTUtilsDialog::waitForDialog(os, new SmithWatermanDialogFiller(os, new Scenario));
        GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Analyze" << "Find pattern [Smith-Waterman]...", GTGlobals::UseMouse);
        GTUtilsTaskTreeView::waitTaskFinished(os);

        GTUtilsProjectTreeView::doubleClickItem(os, "P1_NC_1.aln");
        GTUtilsTaskTreeView::waitTaskFinished(os);

        const bool isAlphabetAmino = GTUtilsMsaEditor::getEditor(os)->getMaObject()->getAlphabet()->isAmino();
        CHECK_SET_ERR(isAlphabetAmino, "Alphabet is not amino");
}

GUI_TEST_CLASS_DEFINITION(test_6118) {
    //1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //2. Make workflow "Read FASTQ File with SE Reads" -> "Improve Reads with Trimmomatic"
    const QString readSEName = "Read FASTQ File with SE Reads";
    const QString trimmomaticName = "Improve Reads with Trimmomatic";

    WorkflowProcessItem *readSEElement = GTUtilsWorkflowDesigner::addElement(os, readSEName);
    WorkflowProcessItem *trimmomaticElement = GTUtilsWorkflowDesigner::addElement(os, trimmomaticName);
    GTUtilsWorkflowDesigner::connect(os, readSEElement, trimmomaticElement);


    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(NULL != dialog, "Active modal widget is NULL");

            //3. Add two "ILLUMINACLIP" steps with adapters with similar filenames located in different directories to Trimmomatic worker.
            GTWidget::click(os, GTWidget::findWidget(os, "buttonAdd"));
            QMenu *menu = qobject_cast<QMenu*>(GTWidget::findWidget(os, "stepsMenu"));
            GTMenu::clickMenuItemByName(os, menu, QStringList() << "ILLUMINACLIP");
            GTKeyboardDriver::keyClick(Qt::Key_Escape);
            GTGlobals::sleep(500);

            GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, testDir + "_common_data/regression/6118/TruSeq3-SE.fa"));
            GTWidget::click(os, GTWidget::findWidget(os, "tbBrowse", dialog));
            GTGlobals::sleep(500);

            GTWidget::click(os, GTWidget::findWidget(os, "buttonAdd"));
            menu = qobject_cast<QMenu*>(GTWidget::findWidget(os, "stepsMenu"));
            GTMenu::clickMenuItemByName(os, menu, QStringList() << "ILLUMINACLIP");
            GTKeyboardDriver::keyClick(Qt::Key_Escape);
            GTGlobals::sleep(500);

            GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, testDir + "_common_data/regression/6118/deeperDir/TruSeq3-SE.fa"));
            GTWidget::click(os, GTWidget::findWidget(os, "tbBrowse", dialog));

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTUtilsWorkflowDesigner::click(os, readSEElement);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/FASTQ/eas.fastq");

    GTUtilsWorkflowDesigner::click(os, trimmomaticElement);
    GTUtilsDialog::waitForDialog(os, new DefaultDialogFiller(os, "TrimmomaticPropertyDialog", QDialogButtonBox::Ok, new Scenario()));
    QTableView* table = GTWidget::findExactWidget<QTableView*>(os, "table");
    GTMouseDriver::moveTo(GTTableView::getCellPoint(os, table, 1, 1));
    GTMouseDriver::click();
    GTGlobals::sleep();
    GTWidget::click(os, GTWidget::findWidget(os, "trimmomaticPropertyToolButton", table));
    GTGlobals::sleep(500);

    //4. Run this workflow.
    //Expected state : there are no errors during execution.
    GTLogTracer l;
    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    CHECK_SET_ERR(!l.hasError(), "Errors in the log");
}

GUI_TEST_CLASS_DEFINITION(test_6135) {

    // Open "COI.aln".
    // Rename the second sequence to "Phaneroptera_falcata".
    // Current state: There are two sequences with name "Phaneroptera_falcata" (the first and the second one).
    // Select "Export -> Save subalignment" in the context menu.
    // Select only one "Phaneroptera_falcata" sequence and click "Extract".
    // Expected state: one selected sequence was exported.

    GTUtilsProject::openFiles(os, dataDir + "samples/CLUSTALW/COI.aln");

    class custom: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTUtilsMSAEditorSequenceArea::renameSequence(os, "Isophya_altaica_EF540820", "Phaneroptera_falcata");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<MSAE_MENU_EXPORT<<"Save subalignment"));
    GTUtilsDialog::waitForDialog(os,new ExtractSelectedAsMSADialogFiller(os, new custom()));

    GTMenu::showContextMenu(os,GTWidget::findWidget(os,"msa_editor_sequence_area"));

    GTMouseDriver::moveTo(GTUtilsProjectTreeView::getItemCenter(os, "COI.aln"));;
    GTMouseDriver::click();

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));

    GTKeyboardDriver::keyClick(Qt::Key_Delete);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    const QStringList sequencesNameList = GTUtilsMSAEditorSequenceArea::getNameList(os);

    CHECK_SET_ERR(sequencesNameList.length() == 1, "Length of namelist is not 1! Length: " + QString::number(sequencesNameList.length()));
}



GUI_TEST_CLASS_DEFINITION(test_6136) {
    // 1. Open "test/scenarios/_common_data/genbank/target_gene_new.gb".
    GTFileDialog::openFile(os, testDir + "_common_data/genbank/target_gene_new.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Open "In Silico PCR" tab
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::InSilicoPcr);

    //3. Fill values:
    //Forward: primer - TTTGGATCCAGCATCACCATCACCATCACGATCAAATAGAAGCAATG, mismathches - 27
    //Reverse: primer - AAACCTAGGTACGTAGTGGTAGTGGTAGTGCTAGTTTATCTTCGTTAC, mismathches - 27
    GTUtilsOptionPanelSequenceView::setForwardPrimer(os, "TTTGGATCCAGCATCACCATCACCATCACGATCAAATAGAAGCAATG");
    GTUtilsOptionPanelSequenceView::setForwardPrimerMismatches(os, 27);
    GTUtilsOptionPanelSequenceView::setReversePrimer(os, "AAACCTAGGTACGTAGTGGTAGTGGTAGTGCTAGTTTATCTTCGTTAC");
    GTUtilsOptionPanelSequenceView::setReversePrimerMismatches(os, 27);

    //4. Press "find product(s)"
    GTUtilsOptionPanelSequenceView::pressFindProducts(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected: there is one product was found
    const int count = GTUtilsOptionPanelSequenceView::productsCount(os);
    CHECK_SET_ERR(count == 1, QString("Unexpected products quantity, expected: 1, current: %1").arg(count));

    //5. Press "Extract product"
    GTUtilsOptionPanelSequenceView::pressExtractProduct(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected: Sequence length = 423
    const int length = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(length == 423, QString("Unexpected sequence length, expected: 423, current: %1").arg(length));
    GTGlobals::sleep(200);

    //Check annotaions
    foreach(const int i, QList<int>() << 30 << 376) {
        GTUtilsSequenceView::clickAnnotationPan(os, "Misc. Feature", i, 0, true);
        GTGlobals::sleep();
        QVector<U2Region> sel = GTUtilsSequenceView::getSelection(os);
        CHECK_SET_ERR(sel.size() == 1, QString("Unexpected selection annotation regions, expected: 1, current: %1").arg(sel.size()));
    }

    foreach(const int i, QList<int>() << 1 << 376) {
        GTUtilsSequenceView::clickAnnotationPan(os, "misc_feature", i, 0, true);
        QVector<U2Region> sel = GTUtilsSequenceView::getSelection(os);
        CHECK_SET_ERR(sel.size() == 1, QString("Unexpected selection primer annotation regions, expected: 1, current: %1").arg(sel.size()));
    }
}

GUI_TEST_CLASS_DEFINITION(test_6167) {
    //1. Change workflow designer output folder to sandbox
    class Custom : public CustomScenario {
    void run(HI::GUITestOpStatus &os){
        QWidget *dialog = QApplication::activeModalWidget();

        QTreeWidget* tree = GTWidget::findExactWidget<QTreeWidget*>(os, "tree", dialog);
        CHECK_SET_ERR(tree != NULL, "QTreeWidget unexpectedly not found");

        AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::WorkflowDesigner);

        QLineEdit* workflowOutputEdit = GTWidget::findExactWidget<QLineEdit*>(os, "workflowOutputEdit", dialog);
        CHECK_SET_ERR(workflowOutputEdit != NULL, "QLineEdit unexpectedly not found");

        GTLineEdit::setText(os, workflowOutputEdit, sandBoxDir);

        GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
    }
    };

    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new Custom()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings" << "Preferences...", GTGlobals::UseMouse);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Open "test\_common_data\regression\6167\6167.uwl" and run
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::loadWorkflow(os, testDir + "_common_data/regression/6167/6167.uwl");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsWorkflowDesigner::addInputFile(os, "Read File URL(s)", dataDir + "samples/FASTQ/eas.fastq");

    class TrimmomaticCustomScenario : public CustomScenario {
    void run(HI::GUITestOpStatus& os) {
        QWidget *dialog = QApplication::activeModalWidget();

        QToolButton* addButton = GTWidget::findExactWidget<QToolButton*>(os, "buttonAdd", dialog);
        CHECK_SET_ERR(addButton != NULL, "addButton unexpectedly not found");

        GTWidget::click(os, addButton);
        GTGlobals::sleep(200);
        for (int i = 0; i < 4; i++) {
            GTKeyboardDriver::keyClick(Qt::Key_Down);
            GTGlobals::sleep(200);
        }

        GTKeyboardDriver::keyClick(Qt::Key_Enter);
        GTGlobals::sleep(500);
        GTKeyboardDriver::keyClick(Qt::Key_Enter);
        GTGlobals::sleep(200);
        GTKeyboardDriver::keyClick(Qt::Key_Escape);
        GTGlobals::sleep(200);

        GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
    }
    };

    GTUtilsDialog::waitForDialog(os, new TrimmomaticDialogFiller(os, new TrimmomaticCustomScenario()));
    GTUtilsWorkflowDesigner::click(os, "Trimmomatic 1");
    GTUtilsWorkflowDesigner::setParameter(os, "Trimming steps", "", GTUtilsWorkflowDesigner::customDialogSelector);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new TrimmomaticDialogFiller(os, new TrimmomaticCustomScenario()));
    GTUtilsWorkflowDesigner::click(os, "Trimmomatic 2");
    GTUtilsWorkflowDesigner::setParameter(os, "Trimming steps", "", GTUtilsWorkflowDesigner::customDialogSelector);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected: There are no adapter files in the output directory
    QDir sandbox(sandBoxDir);
    QStringList filter = QStringList() << "????.??.??_?\?-??";
    QStringList sandboxEntry = sandbox.entryList(filter, QDir::AllEntries);
    CHECK_SET_ERR(sandboxEntry.size() == 1, QString("Unexpected nomber of folders, expected: 1, current62: %1").arg(sandboxEntry.size()));

    QString insideSandbox(sandBoxDir + sandboxEntry.first());
    QDir insideSandboxDir(insideSandbox);
    QStringList resultDirs = insideSandboxDir.entryList();
    CHECK_SET_ERR(resultDirs.size() == 5, QString("Unexpected number of result folders, expected: 5, current: %1").arg(resultDirs.size()));
}

GUI_TEST_CLASS_DEFINITION(test_6204) {
    //1. Open the WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::loadWorkflow(os, testDir + "_common_data/scenarios/_regression/6204/6204.uwl");

    //2. add 3 big alignments to "Read Alignment 1" worker ""_common_data/clustal/100_sequences.aln", ""_common_data/clustal/non_unique_row_names.aln"
    GTUtilsWorkflowDesigner::addInputFile(os, "Read Alignment 1", testDir + "_common_data/clustal/100_sequences.aln");
    GTUtilsWorkflowDesigner::addInputFile(os, "Read Alignment 1", testDir + "_common_data/clustal/non_unique_row_names.aln");
    GTUtilsWorkflowDesigner::addInputFile(os, "Read Alignment", testDir + "_common_data/clustal/COI na.aln");
    GTUtilsWorkflowDesigner::runWorkflow(os);

    // There is no message "Task is in progress.." after finished task where 2 notifications are present
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(100);
    HI::HIWebElement el = GTUtilsDashboard::findElement(os, "The workflow task has been finished");
    CHECK_SET_ERR(el.geometry() != QRect(), QString("Element with desired text not found"));
}

GUI_TEST_CLASS_DEFINITION(test_6207) {
    //1. Open the WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    //2. Compose scheme read fastq with PE reads -> Filter by Classification
    GTUtilsWorkflowDesigner::addElement(os, "Read FASTQ File with SE Reads", true);
    GTUtilsWorkflowDesigner::addElement(os, "Filter by Classification", true);
    GTUtilsWorkflowDesigner::connect(os, GTUtilsWorkflowDesigner::getWorker(os, "Read FASTQ File with SE Reads"),
                                         GTUtilsWorkflowDesigner::getWorker(os, "Filter by Classification"));
    //3. Set eas.fastq as input data
    //GTUtilsWorkflowDesigner::click(os, "Read FASTQ File with PE Reads");
    GTUtilsWorkflowDesigner::addInputFile(os, "Read FASTQ File with SE Reads", dataDir + "samples/FASTQ/eas.fastq");

    //4. Validate scheme. Count errors
    GTUtilsWorkflowDesigner::validateWorkflow(os);
    int errorCount = GTUtilsWorkflowDesigner::getErrors(os).size();
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(Qt::Key_Enter);
    GTUtilsWorkflowDesigner::click(os, "Filter by Classification");
    GTWidget::click(os, GTWidget::findExactWidget<QGroupBox*>(os, "inputPortBox"), Qt::LeftButton, QPoint(7,7));

    //6. In the Property Editor change value of the "Input URL 1" slot to empty. Don't change focus.
    QTableWidget* table1 = GTUtilsWorkflowDesigner::getInputPortsTable(os, 0);
    //GTUtilsWorkflowDesigner::setTableValue(os, "Source URL", "<empty>", GTUtilsWorkflowDesigner::comboValue, table1);
    GTUtilsWorkflowDesigner::setTableValue(os, "Input URL 1", "<empty>", GTUtilsWorkflowDesigner::comboValue, table1);

    //7. Validate workflow, count errors
    GTUtilsWorkflowDesigner::validateWorkflow(os);
    GTKeyboardDriver::keyClick(Qt::Key_Enter);

    //Expected state: error counter contains 1 error more
    CHECK_SET_ERR(GTUtilsWorkflowDesigner::getErrors(os).size() == 1 + errorCount, QString("Workflow validation error count doesn't match. Expected error count %1, actual %2.")
                  .arg(QString::number(1 + errorCount)).arg(QString::number(GTUtilsWorkflowDesigner::getErrors(os).size())));
}

GUI_TEST_CLASS_DEFINITION(test_6212) {
    //1. Open the WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //2. Add "Read File URL data" and "Improve Reads with Trimmomatic" elements and connect them.
    const QString readFileName = "Read File URL(s)";
    const QString trimmomaticName = "Improve Reads with Trimmomatic";
    WorkflowProcessItem* readFileNameElement = GTUtilsWorkflowDesigner::addElement(os, readFileName);
    WorkflowProcessItem* trimmomaticElement = GTUtilsWorkflowDesigner::addElement(os, trimmomaticName);
    GTUtilsWorkflowDesigner::addInputFile(os, readFileName, dataDir + "samples/FASTQ/eas.fastq");
    GTUtilsWorkflowDesigner::connect(os, readFileNameElement, trimmomaticElement);
    GTUtilsWorkflowDesigner::click(os, trimmomaticName);
    QTableWidget* table1 = GTUtilsWorkflowDesigner::getInputPortsTable(os, 0);
    CHECK_SET_ERR(table1 != NULL, "QTableWidget isn't found");

    GTUtilsWorkflowDesigner::setTableValue(os, "Input FASTQ URL 1", "Dataset name (by Read File URL(s))", GTUtilsWorkflowDesigner::comboValue, table1);

    //3. Click on the Trimmomatic element, then click on the "Configure steps" parameter in the Property Editor, click on the appeared browse button in the value field.
    class TrimmomaticCustomScenario : public CustomScenario {
        void run(HI::GUITestOpStatus& os) {
            QWidget *dialog = QApplication::activeModalWidget();

            QDialogButtonBox* buttonBox = GTWidget::findExactWidget<QDialogButtonBox*>(os, "buttonBox", dialog);
            CHECK_SET_ERR(buttonBox != NULL, "QDialogButtonBox unexpectedly not found");

            QPushButton* buttonOk =  buttonBox->button(QDialogButtonBox::Ok);
            CHECK_SET_ERR(buttonOk != NULL, "buttonOk unexpectedly not found");
            CHECK_SET_ERR(!buttonOk->isEnabled(), "buttonOk should be disabled");

            //4. Close the dialog
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };

    GTUtilsDialog::waitForDialog(os, new TrimmomaticDialogFiller(os, new TrimmomaticCustomScenario()));
    GTUtilsWorkflowDesigner::click(os, trimmomaticName);
    GTUtilsWorkflowDesigner::setParameter(os, "Trimming steps", "", GTUtilsWorkflowDesigner::customDialogSelector);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    //5. Click "Validate workflow".
    GTUtilsWorkflowDesigner::validateWorkflow(os);

    //Expected state: Validation doesn't pass, there is an error about absent steps.
    QStringList errors = GTUtilsWorkflowDesigner::getErrors(os);
    CHECK_SET_ERR(errors.size() == 1, QString("Unexpected errors number, expected: 1, current: %1").arg(errors.size()));
    CHECK_SET_ERR(errors.first() == "Improve Reads with Trimmomatic: Required parameter is not set: Trimming steps", "Unexpected error in the log. Is should be something about Trimming steps");

    GTKeyboardDriver::keyClick(Qt::Key_Enter);
    GTGlobals::sleep(1000);
}

GUI_TEST_CLASS_DEFINITION(test_6225) {
    QString filePath = testDir + "_common_data/sanger/alignment_short.ugenedb";
    QString fileName = "sanger_alignment_short.ugenedb";

    //1. Copy to 'sandbox' and open alignment_short.ugenedb
    GTFile::copy(os, filePath, sandBoxDir + "/" + fileName);
    GTFileDialog::openFile(os, sandBoxDir, fileName);

    //2. Open Consensus tab
    GTUtilsOptionPanelMca::openTab(os, GTUtilsOptionPanelMca::Consensus);

    //3. Choose FASTA as file format
    GTUtilsOptionPanelMca::setFileFormat(os, GTUtilsOptionPanelMca::FASTA);

    //4. Click Export
    GTUtilsOptionPanelMca::pushExportButton(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    const int size = GTUtilsProjectTreeView::getDocuments(os).size();
    CHECK_SET_ERR(size == 2, QString("Unexpected documents number; expected: 2, current: %1").arg(size));
}

GUI_TEST_CLASS_DEFINITION(test_6226) {

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Align));
    AlignShortReadsFiller::UgeneGenomeAlignerParams parameters(testDir + "_common_data/fasta/reference.fa", QStringList());
    parameters.samOutput = false;
    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os, &parameters));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/reads.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_6229) {

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Align));
    AlignShortReadsFiller::UgeneGenomeAlignerParams parameters(testDir + "_common_data/fasta/reference.fa", QStringList());
    parameters.samOutput = false;
    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os, &parameters));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/reads.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << "unassociateReferenceAction", PopupChecker::IsEnabled));
    GTWidget::click(os, GTWidget::findWidget(os, "Assembly reference sequence area"), Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_6230) {

    //    2. Select "Tools" -> Sanger data analysis" -> "Map reads to reference...".
    //    3. Set "_common_data/sanger/reference.gb" as reference, "_common_data/sanger/sanger_*.ab1" as reads. Accept the dialog.
    //    Expected state: the task fails.
    //    4. After the task finish open the report.
    //    Expected state: there is an error message in the report: "The task uses a temporary folder to process the data. The folder path is required not to have spaces. Please set up an appropriate path for the "Temporary files" parameter on the "Directories" tab of the UGENE Application Settings.".
        class Scenario : public CustomScenario {
            void run(HI::GUITestOpStatus &os) {
                QWidget *dialog = QApplication::activeModalWidget();
                CHECK_SET_ERR(NULL != dialog, "activeModalWidget is NULL");

                AlignToReferenceBlastDialogFiller::setReference(os, testDir + "_common_data/sanger/reference.gb", dialog);

                QStringList reads;
                for (int i = 1; i < 21; i++) {
                    reads << QString(testDir + "_common_data/sanger/sanger_%1.ab1").arg(i, 2, 10, QChar('0'));
                }
                AlignToReferenceBlastDialogFiller::setReads(os, reads, dialog);
                AlignToReferenceBlastDialogFiller::setDestination(os, sandBoxDir + "test_6230/test_6230.ugenedb", dialog);

                GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
            }
        };

        GTUtilsDialog::waitForDialog(os, new AlignToReferenceBlastDialogFiller(os, new Scenario));
        GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Sanger data analysis" << "Map reads to reference...");
        GTUtilsTaskTreeView::waitTaskFinished(os);

        GTUtilsTaskTreeView::waitTaskFinished(os);
        GTUtilsProjectTreeView::checkItem(os, "test_6230.ugenedb");

}

GUI_TEST_CLASS_DEFINITION(test_6232_1) {
    //1. Open "STEP1_pFUS2_a2a_5.gb" sequence.
    GTFileDialog::openFile(os, testDir + "_common_data/regression/6232/STEP1_pFUS2_a2a_5.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Select "Actions > Analyze > Find restriction sites", check "Esp3I" enzyme in the appeared dialog, click "OK".
    GTUtilsDialog::waitForDialog(os, new FindEnzymesDialogFiller(os, QStringList() << "Esp3I"));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Analyze" << "Find restriction sites...", GTGlobals::UseMouse);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Select "Actions > Cloning > Digest into fragments".Add "Esp3I" to the "Selected enzymes" in the appeared dialog, click "OK".
    GTUtilsDialog::waitForDialog(os, new DigestSequenceDialogFiller(os));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Cloning" << "Digest into fragments...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state : the corresponding "Fragment" annotations have been created
    QStringList groupNames = GTUtilsAnnotationsTreeView::getGroupNames(os);
    CHECK_SET_ERR(groupNames.contains("fragments  (0, 2)"), "The group \"fragments  (0, 2)\" is unexpectedly absent");
}

GUI_TEST_CLASS_DEFINITION(test_6232_2) {
    //1. Open "STEP1_pFUS2_a2a_5_2.gb" sequence.
    GTFileDialog::openFile(os, testDir + "_common_data/regression/6232/STEP1_pFUS2_a2a_5_2.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Select "Actions > Analyze > Find restriction sites", check "Esp3I" enzyme in the appeared dialog, click "OK".
    GTUtilsDialog::waitForDialog(os, new FindEnzymesDialogFiller(os, QStringList() << "Esp3I"));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Analyze" << "Find restriction sites...", GTGlobals::UseMouse);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Select "Actions > Cloning > Digest into fragments".Add "Esp3I" to the "Selected enzymes" in the appeared dialog, click "OK".
    GTUtilsDialog::waitForDialog(os, new DigestSequenceDialogFiller(os));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Cloning" << "Digest into fragments...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state: "left_end_seq" qualifier of the first fragment and "right_end_seq" of the second fragment should have "TGAC" value
    /*QTreeWidgetItem* fragment1 = GTUtilsAnnotationsTreeView::findItem(os, "Fragment 1");
    CHECK_SET_ERR(fragment1 != NULL, "Fragment 1 is not found");
*/
    QString firstValue = GTUtilsAnnotationsTreeView::getQualifierValue(os, "left_end_seq", "Fragment 1");
    CHECK_SET_ERR(firstValue == "TGAC", QString("Unexpected qualifier value of the first fragment, expected: TGAC, current: %1").arg(firstValue));

    QString secondValue = GTUtilsAnnotationsTreeView::getQualifierValue(os, "right_end_seq", "Fragment 2");
    CHECK_SET_ERR(secondValue == "TGAC", QString("Unexpected qualifier value of the first fragment, expected: TGAC, current: %1").arg(secondValue));
}

GUI_TEST_CLASS_DEFINITION(test_6232_3) {
    //1. Open "STEP1_pFUS2_a2a_5.gb" sequence.
    GTFileDialog::openFile(os, testDir + "_common_data/regression/6232/STEP1_pFUS2_a2a_5.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Select "Actions > Analyze > Find restriction sites", check "Esp3I" enzyme in the appeared dialog, click "OK".
    GTUtilsDialog::waitForDialog(os, new FindEnzymesDialogFiller(os, QStringList() << "Esp3I"));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Analyze" << "Find restriction sites...", GTGlobals::UseMouse);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Select "Actions > Cloning > Digest into fragments".Add "Esp3I" to the "Selected enzymes", disable "Circular Molecule" checkBox and click "OK".
    class Scenario : public CustomScenario {
        void run(GUITestOpStatus& os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(NULL != dialog, "activeModalWidget is NULL");

            GTCheckBox::setChecked(os, "circularBox", false, dialog);
            GTWidget::click(os, GTWidget::findWidget(os, "addAllButton", dialog));
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTUtilsDialog::waitForDialog(os, new DigestSequenceDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Cloning" << "Digest into fragments...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected: despite the sequence is circular, fragments were found without the gap it the junction  point
    //Expected: there are two annotations with the following regions were created - U2Region(2, 2467) and U2Region(2473, 412)
    QList<U2Region> regions = GTUtilsAnnotationsTreeView::getAnnotatedRegionsOfGroup(os, "fragments  (0, 2)");
    CHECK_SET_ERR(regions.size() == 2, QString("Unexpected number of fragments, expected: 2, current: %1").arg(regions.size()));
    CHECK_SET_ERR(regions.first() == U2Region(2, 2467), QString("Unexpected fragment region, expected: start = 2, length = 2467; current: start = %1, length = %2").arg(regions.first().startPos).arg(regions.first().length));
    CHECK_SET_ERR(regions.last() == U2Region(2473, 412), QString("Unexpected fragment region, expected: start = 2473, length = 412; current: start = %1, length = %2").arg(regions.last().startPos).arg(regions.last().length));
}

GUI_TEST_CLASS_DEFINITION(test_6232_4) {
    //1. Open "STEP1_pFUS2_a2a_5_not_circular.gb" sequence.
    GTFileDialog::openFile(os, testDir + "_common_data/regression/6232/STEP1_pFUS2_a2a_5_not_circular.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Select "Actions > Analyze > Find restriction sites", check "Esp3I" enzyme in the appeared dialog, click "OK".
    GTUtilsDialog::waitForDialog(os, new FindEnzymesDialogFiller(os, QStringList() << "Esp3I"));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Analyze" << "Find restriction sites...", GTGlobals::UseMouse);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Select "Actions > Cloning > Digest into fragments".Add "Esp3I" to the "Selected enzymes" checkBox and click "OK".
    GTUtilsDialog::waitForDialog(os, new DigestSequenceDialogFiller(os));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Cloning" << "Digest into fragments...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //4. Select "Actions > Cloning > Construct molecule...". Click "Add all" button, click "OK".
    QList<ConstructMoleculeDialogFiller::Action> actions;
    actions << ConstructMoleculeDialogFiller::Action(ConstructMoleculeDialogFiller::AddAllFragments, "");
    actions << ConstructMoleculeDialogFiller::Action(ConstructMoleculeDialogFiller::ClickOk, "");
    GTUtilsDialog::waitForDialog(os, new ConstructMoleculeDialogFiller(os, actions));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Cloning" << "Construct molecule...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state: the first fragment begins from 3 base, there are "AC" bases before it
    QList<U2Region> regions = GTUtilsAnnotationsTreeView::getAnnotatedRegionsOfGroup(os, "STEP1_pFUS2_a2a_5 Fragment 1  (0, 1)");
    CHECK_SET_ERR(regions.size() == 1, QString("Unexpected number of fragments, expected: 1, current: %1").arg(regions.size()));
    CHECK_SET_ERR(regions.first().startPos == 2, QString("Unexpected fragment startPos, expected: 2; current: %1").arg(regions.first().startPos));

    QString beginning = GTUtilsSequenceView::getBeginOfSequenceAsString(os, 2);
    CHECK_SET_ERR(beginning == "AC", QString("Unexpected beginning, expected: AC, currecnt: %1").arg(beginning));
}

GUI_TEST_CLASS_DEFINITION(test_6233) {
    //1. Find the link to the ET downloadp page in the application settings
    class Custom : public CustomScenario {
        void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();

            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::ExternalTools);
            QLabel* selectToolPackLabel = GTWidget::findExactWidget<QLabel*>(os, "selectToolPackLabel");
            CHECK_SET_ERR(selectToolPackLabel != NULL, "selectToolPackLabel unexpectedly not found");

            QPoint pos(selectToolPackLabel->pos().x(), selectToolPackLabel->pos().y());
            QPoint globalPos = selectToolPackLabel->mapToGlobal(pos);
#ifdef Q_OS_LINUX
            globalPos.setY(globalPos.y() - 20);
#endif
            GTMouseDriver::moveTo(globalPos);
            const int xpos = globalPos.x();
            GTClipboard::clear(os);
            for (int i = 0; i < 7; i++) {
                for (int j = 0; j < 20; j++) {
                    GTGlobals::sleep(100);
                    QPoint mousePos(GTMouseDriver::getMousePosition());
#ifdef Q_OS_WIN
                    globalPos = QPoint(mousePos.x() + 11, mousePos.y() + 1);
#else
                    globalPos = QPoint(mousePos.x() + 10, mousePos.y());
#endif
                    GTMouseDriver::moveTo(globalPos);
                    Qt::CursorShape shape = selectToolPackLabel->cursor().shape();
                    if (shape != Qt::ArrowCursor) {
                        GTMouseDriver::click(Qt::RightButton);
                        GTGlobals::sleep(200);
                        GTKeyboardDriver::keyClick(Qt::Key_Down);
                        GTGlobals::sleep(200);
                        GTKeyboardDriver::keyClick(Qt::Key_Enter);
                        clip = GTClipboard::text(os);
                        if (!clip.isEmpty()) {
                            break;
                        }
                    }
                }
                if (!clip.isEmpty()) {
                    break;
                }
                GTGlobals::sleep(25);
                globalPos = QPoint(xpos, globalPos.y() + 5);
                GTMouseDriver::moveTo(globalPos);
            }
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }

    public:
        QString clip;
    };

    Custom* c = new Custom();
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, c));
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings" << "Preferences...", GTGlobals::UseMouse);

    //2. Pull html by the link
    HttpFileAdapterFactory* factory = new HttpFileAdapterFactory;
    IOAdapter* adapter = factory->createIOAdapter();
    bool isOpened = adapter->open(GUrl(c->clip), IOAdapterMode_Read);
    CHECK_SET_ERR(isOpened, "HttpFileAdapter unexpectedly wasn't opened");

    char* data = new char[128];
    bool isNotFound = false;
    bool eof = false;

    while (!isNotFound && !eof) {
        int read = adapter->readLine(data, 128);
        QString d(data);
        isNotFound = d.contains("Page not found");
        eof = read == 0;
    }

    CHECK_SET_ERR(!isNotFound, "The External Tools page is not found");
}

GUI_TEST_CLASS_DEFINITION(test_6235_1) {
    //1. Open "_common_data/regression/6235/6235_1.gb" sequence.
    GTFileDialog::openFile(os, testDir + "_common_data/regression/6235/6235_1.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Select "Actions > Analyze > Find restriction sites", check "Esp3I" enzyme in the appeared dialog, click "OK".
    GTUtilsDialog::waitForDialog(os, new FindEnzymesDialogFiller(os, QStringList() << "Esp3I"));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Analyze" << "Find restriction sites...", GTGlobals::UseMouse);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Select "Actions > Cloning > Digest into fragments". Add "Esp3I" to the "Selected enzymes" in the appeared dialog, check "Circular molecule", click "OK".
    class Scenario : public CustomScenario {
    public:
        void run(HI::GUITestOpStatus& os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(NULL != dialog, "activeModalWidget is NULL");

            GTWidget::click(os, GTWidget::findWidget(os, "addAllButton", dialog));

            GTCheckBox::setChecked(os, "circularBox", true, dialog);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTUtilsDialog::waitForDialog(os, new DigestSequenceDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Cloning" << "Digest into fragments...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state: there are two annotations with the following regions were created - U2Region(2, 2467) and U2Region(2473, 410)
    QList<U2Region> regions = GTUtilsAnnotationsTreeView::getAnnotatedRegionsOfGroup(os, "fragments  (0, 2)");
    CHECK_SET_ERR(regions.size() == 2, QString("Unexpected number of fragments, expected: 2, current: %1").arg(regions.size()));
    CHECK_SET_ERR(regions.first() == U2Region(2, 2467), QString("Unexpected fragment region, expected: start = 2, length = 2467; current: start = %1, length = %2").arg(regions.first().startPos).arg(regions.first().length));
    CHECK_SET_ERR(regions.last() == U2Region(2473, 410), QString("Unexpected fragment region, expected: start = 2473, length = 410; current: start = %1, length = %2").arg(regions.last().startPos).arg(regions.last().length));
}

GUI_TEST_CLASS_DEFINITION(test_6235_2) {
    //1. Open "_common_data/regression/6235/6235_1.gb" sequence.
    GTFileDialog::openFile(os, testDir + "_common_data/regression/6235/6235_1.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Select "Actions > Analyze > Find restriction sites", check "Esp3I" enzyme in the appeared dialog, click "OK".
    GTUtilsDialog::waitForDialog(os, new FindEnzymesDialogFiller(os, QStringList() << "Esp3I"));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Analyze" << "Find restriction sites...", GTGlobals::UseMouse);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Select "Actions > Cloning > Digest into fragments". Add "Esp3I" to the "Selected enzymes" in the appeared dialog, uncheck "Circular molecule", click "OK".
    class Scenario : public CustomScenario {
    public:
        void run(HI::GUITestOpStatus& os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(NULL != dialog, "activeModalWidget is NULL");

            GTWidget::click(os, GTWidget::findWidget(os, "addAllButton", dialog));

            GTCheckBox::setChecked(os, "circularBox", false, dialog);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTUtilsDialog::waitForDialog(os, new DigestSequenceDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Cloning" << "Digest into fragments...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state: there are two annotations with the following regions were created - U2Region(2, 2467) and U2Region(2473, 412)
    QList<U2Region> regions = GTUtilsAnnotationsTreeView::getAnnotatedRegionsOfGroup(os, "fragments  (0, 2)");
    CHECK_SET_ERR(regions.size() == 2, QString("Unexpected number of fragments, expected: 2, current: %1").arg(regions.size()));
    CHECK_SET_ERR(regions.first() == U2Region(2, 2467), QString("Unexpected fragment region, expected: start = 2, length = 2467; current: start = %1, length = %2").arg(regions.first().startPos).arg(regions.first().length));
    CHECK_SET_ERR(regions.last() == U2Region(2473, 412), QString("Unexpected fragment region, expected: start = 2473, length = 412; current: start = %1, length = %2").arg(regions.last().startPos).arg(regions.last().length));
}

GUI_TEST_CLASS_DEFINITION(test_6235_3) {
    //1. Open "_common_data/regression/6235/6235_2.gb" sequence.
    GTFileDialog::openFile(os, testDir + "_common_data/regression/6235/6235_2.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Select "Actions > Analyze > Find restriction sites", check "Esp3I" enzyme in the appeared dialog, click "OK".
    GTUtilsDialog::waitForDialog(os, new FindEnzymesDialogFiller(os, QStringList() << "Esp3I"));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Analyze" << "Find restriction sites...", GTGlobals::UseMouse);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Select "Actions > Cloning > Digest into fragments". Add "Esp3I" to the "Selected enzymes" in the appeared dialog, check "Circular molecule", click "OK".
    class Scenario : public CustomScenario {
    public:
        void run(HI::GUITestOpStatus& os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(NULL != dialog, "activeModalWidget is NULL");

            GTWidget::click(os, GTWidget::findWidget(os, "addAllButton", dialog));

            GTCheckBox::setChecked(os, "circularBox", true, dialog);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTUtilsDialog::waitForDialog(os, new DigestSequenceDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Cloning" << "Digest into fragments...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state: there are two annotations with the following regions were created - U2Region(2, 2467) and U2Region(2473, 412)
    QList<U2Region> regions = GTUtilsAnnotationsTreeView::getAnnotatedRegionsOfGroup(os, "fragments  (0, 2)");
    CHECK_SET_ERR(regions.size() == 2, QString("Unexpected number of fragments, expected: 2, current: %1").arg(regions.size()));
    CHECK_SET_ERR(regions.first() == U2Region(416, 2467), QString("Unexpected fragment region, expected: start = 416, length = 2467; current: start = %1, length = %2").arg(regions.first().startPos).arg(regions.first().length));
    CHECK_SET_ERR(regions.last() == U2Region(2, 410), QString("Unexpected fragment region, expected: start = 2, length = 410; current: start = %1, length = %2").arg(regions.last().startPos).arg(regions.last().length));
}

GUI_TEST_CLASS_DEFINITION(test_6235_4) {
    //1. Open "_common_data/regression/6235/6235_2.gb" sequence.
    GTFileDialog::openFile(os, testDir + "_common_data/regression/6235/6235_2.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Select "Actions > Analyze > Find restriction sites", check "Esp3I" enzyme in the appeared dialog, click "OK".
    GTUtilsDialog::waitForDialog(os, new FindEnzymesDialogFiller(os, QStringList() << "Esp3I"));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Analyze" << "Find restriction sites...", GTGlobals::UseMouse);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Select "Actions > Cloning > Digest into fragments". Add "Esp3I" to the "Selected enzymes" in the appeared dialog, uncheck "Circular molecule", click "OK".
    class Scenario : public CustomScenario {
    public:
        void run(HI::GUITestOpStatus& os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(NULL != dialog, "activeModalWidget is NULL");

            GTWidget::click(os, GTWidget::findWidget(os, "addAllButton", dialog));

            GTCheckBox::setChecked(os, "circularBox", false, dialog);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTUtilsDialog::waitForDialog(os, new DigestSequenceDialogFiller(os, new Scenario));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Cloning" << "Digest into fragments...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state: there are two annotations with the following regions were created - U2Region(2, 2467) and U2Region(2473, 412)
    QList<U2Region> regions = GTUtilsAnnotationsTreeView::getAnnotatedRegionsOfGroup(os, "fragments  (0, 2)");
    CHECK_SET_ERR(regions.size() == 2, QString("Unexpected number of fragments, expected: 2, current: %1").arg(regions.size()));
    CHECK_SET_ERR(regions.first() == U2Region(416, 2467), QString("Unexpected fragment region, expected: start = 416, length = 2467; current: start = %1, length = %2").arg(regions.first().startPos).arg(regions.first().length));
    CHECK_SET_ERR(regions.last() == U2Region(0, 412), QString("Unexpected fragment region, expected: start = 0, length = 412; current: start = %1, length = %2").arg(regions.last().startPos).arg(regions.last().length));
}

GUI_TEST_CLASS_DEFINITION(test_6236) {
    //1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    //2. Compose workflow read sequence -> Remote blase
    WorkflowProcessItem *readElement = GTUtilsWorkflowDesigner::addElement(os, "Read Sequence", true);
    WorkflowProcessItem *remoteBlast = GTUtilsWorkflowDesigner::addElementByUsingNameFilter(os, "Remote BLAST");
    GTUtilsWorkflowDesigner::connect(os, readElement, remoteBlast);

    //3. Set the input sequence file: "data/samples/Genbank/NC_014267.1.gb".
    GTMouseDriver::moveTo(GTUtilsWorkflowDesigner::getItemCenter(os, "Read Sequence"));
    GTMouseDriver::click();
    GTGlobals::sleep(300);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Genbank/NC_014267.1.gb");

    GTLogTracer l;
    //4. run workflow
    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTGlobals::sleep(60000);
    GTUtilsWorkflowDesigner::stopWorkflow(os);

    //5. Check id of the blast job in log
    bool desiredMessage = l.checkMessage("Downloading from https://blast.ncbi.nlm.nih.gov/Blast.cgi?CMD=Get&FORMAT_TYPE=XML&RID");
    CHECK_SET_ERR(desiredMessage, "No expected message in the log");
}

GUI_TEST_CLASS_DEFINITION(test_6238) {
    QString fastqFile = testDir + "_common_data/regression/6238/eas.fastq";
    QString sandboxFastqFile = sandBoxDir + "eas.fastq";
    QString badFastqFile = testDir + "_common_data/regression/6238/6238.fastq";
    //1. Open "data/samples/FASTQ/eas.fastq".
    GTFile::copy(os, fastqFile, sandboxFastqFile);

    GTUtilsProject::openMultiSequenceFileAsSequences(os, sandboxFastqFile);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Open the file in some external text editor.
    QFile file(sandboxFastqFile);
    QFile badFile(badFastqFile);
    file.open(QFile::ReadWrite);
    badFile.open(QFile::ReadOnly);

    //3. Replace the file content with the content if the file "_common_data/regression/6238/6238.fastq".
    //Expected state : UGENE offers to reload the modified file.
    //4. Accept the offering.
    //Expected state: the file reloading failed, an error notification appeared, there are error messages in the log.
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::YesAll));
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Separate));
    GTUtilsNotifications::waitForNotification(os, false, "The text file can't be read. Check the file encoding and make sure the file is not corrupted");
    QByteArray badData = badFile.readAll();
    file.write(badData);
    file.close();
    badFile.close();
    GTGlobals::sleep(10000);
}

GUI_TEST_CLASS_DEFINITION(test_6240) {
    //1. Open WD. This step allows us to prevent a bad case, when, at the first opening of WD, the dialog "Choose output directory" appears and the filler below is catching it
    class Scenario : public CustomScenario {
    public:
        void run(HI::GUITestOpStatus& os) {
            QWidget *wizard = GTWidget::getActiveModalWidget(os);
            CHECK_SET_ERR(wizard != NULL, "Wizard isn't found");

            GTUtilsWizard::setParameter(os, "Input file(s)", dataDir + "samples/Assembly/chrM.sam");
            GTUtilsWizard::clickButton(os, GTUtilsWizard::Run);
        }
    };
    //2. Open "Tools" -> "NGS data analysis" -> "Reads quality control..." workflow
    //3. Choose "samples/Assembly/chrM.sam" as input and click "Run"
    GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new StartupDialogFiller(os));
    GTUtilsDialog::waitForDialog(os, new WizardFiller(os, "Quality Control by FastQC Wizard", new Scenario()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "NGS data analysis" << "Reads quality control...");
    GTGlobals::sleep();

    //Expected: The dashboard appears
    QWebView* dashboard = GTUtilsDashboard::getDashboard(os);
    CHECK_SET_ERR(dashboard != NULL, "Dashboard isn't found");
}

GUI_TEST_CLASS_DEFINITION(test_6243) {
    //1. Select "File" -> "Access remove database...".
    //2 Select "ENSEMBL" database. Use any sample ID as "Resource ID". Accept the dialog.
    //Do it twice, for two different ids
    QList<QString> ensemblyIds = QList<QString>() << "ENSG00000205571" << "ENSG00000146463";
    foreach(const QString& id, ensemblyIds) {
        QList<DownloadRemoteFileDialogFiller::Action> actions;
        actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetResourceIds, QStringList() << id);
        actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetDatabase, "ENSEMBL");
        actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::EnterSaveToDirectoryPath, sandBoxDir);
        actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::ClickOk, "");

        GTUtilsDialog::waitForDialog(os, new DownloadRemoteFileDialogFiller(os, actions));
        GTMenu::clickMainMenuItem(os, QStringList() << "File" << "Access remote database...", GTGlobals::UseMouse);
        GTUtilsTaskTreeView::waitTaskFinished(os);
        GTGlobals::sleep();
    }

    //Expected state: the sequences are downloaded. The files names contain the sequence ID.
    QString first = QString("%1.fa").arg(ensemblyIds.first());
    QString second = QString("%1.fa").arg(ensemblyIds.last());
    CHECK_SET_ERR(GTUtilsProjectTreeView::checkItem(os, first), QString("The sequence %1 is absent in the project tree view").arg(first));
    CHECK_SET_ERR(GTUtilsProjectTreeView::checkItem(os, second), QString("The sequence %1 is absent in the project tree view").arg(second));
}

GUI_TEST_CLASS_DEFINITION(test_6247) {
    class Scenario : public CustomScenario {
        void run(HI::GUITestOpStatus& os) {
            QWidget *dialog = GTWidget::getActiveModalWidget(os);
            CHECK_SET_ERR(dialog != NULL, "Dialog isn't found");

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };
    //1. Open "_common_data/sanger/alignment.ugenedb".
    const QString filePath = sandBoxDir + "alignment.ugenedb";
    GTFile::copy(os, testDir + "_common_data/sanger/alignment.ugenedb", filePath);
    GTFileDialog::openFile(os, filePath);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Open "Export consensus" tab, set "../sandbox/Mapped reads_consensus.txt" to the "Export to file" field and click export
    QString exportToFile = sandBoxDir + "Aligned reads_consensus.txt";
    GTUtilsOptionPanelMca::setExportFileName(os, exportToFile);
    GTUtilsDialog::waitForDialog(os, new DocumentFormatSelectorDialogFiller(os, new Scenario));
    GTUtilsOptionPanelMca::pushExportButton(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Open "alignment.ugenedb" again
    GTUtilsProjectTreeView::doubleClickItem(os, "alignment.ugenedb");

    //4. And again open "Export consensus" tab, and click export
    GTUtilsDialog::waitForDialog(os, new DocumentFormatSelectorDialogFiller(os, new Scenario));
    GTUtilsOptionPanelMca::pushExportButton(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected: there are 3 documents in the project tree: "alignment.ugenedb", "Aligned reads_consensus.txt" and "Aligned reads_consensus_1.txt"
    QMap<QString, QStringList> docs = GTUtilsProjectTreeView::getDocuments(os);
    CHECK_SET_ERR(docs.size() == 3, QString("Unexpected docs number, expected: 3, current: %1").arg(docs.size()));
    CHECK_SET_ERR(docs.keys().contains("alignment.ugenedb"), "alignment.ugenedb in unexpectably absent");
    CHECK_SET_ERR(docs.keys().contains("Aligned reads_consensus.txt"), "alignment.ugenedb in unexpectably absent");
    CHECK_SET_ERR(docs.keys().contains("Aligned reads_consensus_1.txt"), "alignment.ugenedb in unexpectably absent");
}

GUI_TEST_CLASS_DEFINITION(test_6249_1) {
    //1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    //2. Compose workflow read file urls -> Fastqc quality control
    GTUtilsWorkflowDesigner::addElement(os, "Read File URL(s)", true);
    GTUtilsWorkflowDesigner::addElement(os, "FastQC Quality Control", true);
    GTUtilsWorkflowDesigner::connect(os, GTUtilsWorkflowDesigner::getWorker(os, "Read File URL(s)"),
                                         GTUtilsWorkflowDesigner::getWorker(os, "FastQC Quality Control"));

    //3. Set the input sequence files: "data\samples\FASTQ\eas.fastq" and "data\samples\Assembly\chrM.sam"
    GTMouseDriver::moveTo(GTUtilsWorkflowDesigner::getItemCenter(os, "Read File URL(s)"));
    GTMouseDriver::click();
    GTGlobals::sleep(300);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/FASTQ/eas.fastq");
    GTGlobals::sleep(300);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Assembly/chrM.sam");

    //4. Run workflow, and check result files on dashboard
    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QStringList outFiles = GTUtilsDashboard::getOutputFiles(os);

    CHECK_SET_ERR(outFiles.contains("eas_fastqc.html"), QString("Output files not contains desired file eas_fastqc.html"));
    CHECK_SET_ERR(outFiles.contains("chrM_fastqc.html"), QString("Output files not contains desired file chrM_fastqc.html"));
}

GUI_TEST_CLASS_DEFINITION(test_6249_2) {
    //1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    //2. Compose workflow read file urls -> Fastqc quality control
    GTUtilsWorkflowDesigner::addElement(os, "Read File URL(s)", true);
    GTUtilsWorkflowDesigner::addElement(os, "FastQC Quality Control", true);
    GTUtilsWorkflowDesigner::connect(os,    GTUtilsWorkflowDesigner::getWorker(os, "Read File URL(s)"),
                                            GTUtilsWorkflowDesigner::getWorker(os, "FastQC Quality Control"));

    //3. Set the input sequence files: "data\samples\FASTQ\eas.fastq" and "data\samples\FASTQ\eas.fastq"
    GTMouseDriver::moveTo(GTUtilsWorkflowDesigner::getItemCenter(os, "Read File URL(s)"));
    GTMouseDriver::click();
    GTGlobals::sleep(300);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/FASTQ/eas.fastq");
    GTGlobals::sleep(300);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Assembly/chrM.sam");

    //4. Set parameter "Output file" to any location
    GTMouseDriver::moveTo(GTUtilsWorkflowDesigner::getItemCenter(os, "FastQC Quality Control"));
    GTMouseDriver::click();
    GTGlobals::sleep(300);
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", QDir(sandBoxDir).absolutePath() + "/test_6249_2_zzzz.html", GTUtilsWorkflowDesigner::textValue);

    //5. Run workflow, and check result files on dashboard
    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QStringList outFiles = GTUtilsDashboard::getOutputFiles(os);

    CHECK_SET_ERR(outFiles.contains("test_6249_2_zzzz.html"), QString("Output files not contains desired file test_6249_2_zzzz.html"));
    CHECK_SET_ERR(outFiles.contains("test_6249_2_zzzz_1.html"), QString("Output files not contains desired file test_6249_2_zzzz_1.html"));
}

GUI_TEST_CLASS_DEFINITION(test_6249_3) {
    //1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    //2. Compose workflow read file urls -> Fastqc quality control x2
    GTUtilsWorkflowDesigner::addElement(os, "Read File URL(s)", true);
    GTUtilsWorkflowDesigner::addElement(os, "FastQC Quality Control", true);
    GTUtilsWorkflowDesigner::addElement(os, "FastQC Quality Control", true);
    GTUtilsWorkflowDesigner::connect(os, GTUtilsWorkflowDesigner::getWorker(os, "Read File URL(s)"),
        GTUtilsWorkflowDesigner::getWorker(os, "FastQC Quality Control"));
    GTUtilsWorkflowDesigner::connect(os, GTUtilsWorkflowDesigner::getWorker(os, "Read File URL(s)"),
        GTUtilsWorkflowDesigner::getWorker(os, "FastQC Quality Control 1"));

    //3. Set the input sequence files: "data\samples\FASTQ\eas.fastq" and "data\samples\Assembly\chrM.sam"
    GTMouseDriver::moveTo(GTUtilsWorkflowDesigner::getItemCenter(os, "Read File URL(s)"));
    GTMouseDriver::click();
    GTGlobals::sleep(300);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/FASTQ/eas.fastq");
    GTGlobals::sleep(300);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Assembly/chrM.sam");

    //4. Run workflow, and check result files on dashboard
    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QStringList outFiles = GTUtilsDashboard::getOutputFiles(os);

    CHECK_SET_ERR(outFiles.contains("eas_fastqc.html"), QString("Output files not contains desired file eas_fastqc.html"));
    CHECK_SET_ERR(outFiles.contains("chrM_fastqc.html"), QString("Output files not contains desired file chrM_fastqc.html"));
    CHECK_SET_ERR(outFiles.contains("eas_fastqc_1.html"), QString("Output files not contains desired file eas_fastqc_1.html"));
    CHECK_SET_ERR(outFiles.contains("chrM_fastqc_1.html"), QString("Output files not contains desired file chrM_fastqc_1.html"));
}

GUI_TEST_CLASS_DEFINITION(test_6256) {
    //1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    QString tempDir = QDir(sandBoxDir + "test_6256").absolutePath();

    class Custom : public CustomScenario {
        void run(HI::GUITestOpStatus &os) {
            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::WorkflowDesigner);
            QWidget *dialog = QApplication::activeModalWidget();
            QDir().mkpath(tempDir);
            GTFile::setReadOnly(os, tempDir);
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit *>(os, "workflowOutputEdit", dialog), tempDir);
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    public:
        QString tempDir;
    };
    //2. Open application settings and change workflow output directory to nonexistent path
    Custom* c = new Custom();
    c->tempDir = tempDir;
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, c));
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings" << "Preferences...", GTGlobals::UseMouse);
    //3. Add "Read File URL" element and validate workflow
    //Expected state: there are 2 erorrs after validation
    GTUtilsWorkflowDesigner::addElement(os, "Read File URL(s)", true);
    GTUtilsWorkflowDesigner::validateWorkflow(os);
    GTGlobals::sleep(1000);
    GTKeyboardDriver::keyClick(Qt::Key_Enter);
    GTGlobals::sleep(1000);
    GTFile::setReadWrite(os, tempDir);

    CHECK_SET_ERR(GTUtilsWorkflowDesigner::getErrors(os).size() == 2, "Unexpected number of errors");
}

GUI_TEST_CLASS_DEFINITION(test_6262) {
    //1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //2. Add "Filter Annotations by Name", "Filter Annotations by Name" and connect them
    WorkflowProcessItem* element1 = GTUtilsWorkflowDesigner::addElement(os, "Filter Annotations by Name");
    WorkflowProcessItem* element2 = GTUtilsWorkflowDesigner::addElement(os, "Filter Annotations by Name");

    //3. Check Input port.
    CHECK_SET_ERR(!GTGroupBox::getChecked(os, "inputPortBox"), "Input Ports table isn't closed");
    GTGroupBox::setChecked(os, "inputPortBox", true);
   /* GTUtilsWorkflowDesigner::click(os, "Filter Annotations by Name");
    CHECK_SET_ERR(GTGroupBox::getChecked(os, "inputPortBox"), "Input Ports table isn't opened");
    GTUtilsWorkflowDesigner::click(os, "Filter Annotations by Name 1");
    CHECK_SET_ERR(GTGroupBox::getChecked(os, "inputPortBox"), "Input Ports table isn't opened");
    GTUtilsWorkflowDesigner::click(os, "Filter Annotations by Name");
    CHECK_SET_ERR(GTGroupBox::getChecked(os, "inputPortBox"), "Input Ports table isn't opened");

    //4. Check Input port.
    GTGroupBox::setChecked(os, "inputPortBox", false);
    GTUtilsWorkflowDesigner::click(os, "Filter Annotations by Name");
    CHECK_SET_ERR(!GTGroupBox::getChecked(os, "inputPortBox"), "Input Ports table isn't closed");
    GTUtilsWorkflowDesigner::click(os, "Filter Annotations by Name 1");
    CHECK_SET_ERR(!GTGroupBox::getChecked(os, "inputPortBox"), "Input Ports table isn't closed");
    GTUtilsWorkflowDesigner::click(os, "Filter Annotations by Name");
    CHECK_SET_ERR(!GTGroupBox::getChecked(os, "inputPortBox"), "Input Ports table isn't closed");

    //5. Check Output port.
    CHECK_SET_ERR(!GTGroupBox::getChecked(os, "outputPortBox"), "Output Ports table isn't closed");
    GTGroupBox::setChecked(os, "outputPortBox", true);
    GTUtilsWorkflowDesigner::click(os, element1);
    CHECK_SET_ERR(GTGroupBox::getChecked(os, "outputPortBox"), "Output Ports table isn't opened");
    GTUtilsWorkflowDesigner::click(os, element2);
    CHECK_SET_ERR(GTGroupBox::getChecked(os, "outputPortBox"), "Output Ports table isn't opened");
    GTUtilsWorkflowDesigner::click(os, element1);
    CHECK_SET_ERR(GTGroupBox::getChecked(os, "outputPortBox"), "Output Ports table isn't opened");

    //6. Check Output port.
    GTGroupBox::setChecked(os, "outputPortBox", false);
    GTUtilsWorkflowDesigner::click(os, element1);
    CHECK_SET_ERR(!GTGroupBox::getChecked(os, "outputPortBox"), "Output Ports table isn't closed");
    GTUtilsWorkflowDesigner::click(os, element2);
    CHECK_SET_ERR(!GTGroupBox::getChecked(os, "outputPortBox"), "Output Ports table isn't closed");
    GTUtilsWorkflowDesigner::click(os, element1);
    CHECK_SET_ERR(!GTGroupBox::getChecked(os, "outputPortBox"), "Output Ports table isn't closed");
    */
}

GUI_TEST_CLASS_DEFINITION(test_6277) {
//    The test checks that the second column of a table with annotations colors on the "Annotations Highlighting" options panel tab in Sequence View is wide enough.
//    UGENE behaviour differed if it was build with Qt5.4 and Qt5.7

//    1. Open "data/samples/Genbank/murine.gb".
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/murine.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    2. Open "Annotations Highlighting" options panel tab.
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::AnnotationsHighlighting);

//    Expected state: the second column's width is 60 pixels; it took at least 20% of the table width; the first column took all available space.
    QTreeWidget *table = GTWidget::findExactWidget<QTreeWidget *>(os, "OP_ANNOT_HIGHLIGHT_TREE");
    CHECK_SET_ERR(nullptr != table, "OP_ANNOT_HIGHLIGHT_TREE widget is NULL");

    QScrollBar *scrollBar = table->verticalScrollBar();
    const int scrollBarWidth = nullptr == scrollBar ? 0 : (scrollBar->isVisible() ? scrollBar->width() : 0);

    // These numbers are defined in the widget stylesheet in AnnotHighlightTree.cpp
    const int MARGIN_LEFT = 5;
    const int MARGIN_RIGHT = 10;
    const int BORDER_WIDTH = 1;
    const int MAGIC_NUMBER = scrollBarWidth + MARGIN_LEFT + MARGIN_RIGHT + 2 * BORDER_WIDTH;

    const int COLOR_COLUMN_NUMBER = 1;
    int colorColumnWidth = table->columnWidth(COLOR_COLUMN_NUMBER);
    int totalTableWidth = table->width();

    const int EXPECTED_COLOR_COLUMN_WIDTH = 60;
    CHECK_SET_ERR(EXPECTED_COLOR_COLUMN_WIDTH == colorColumnWidth,
                  QString("Color column width is incorrect: expected %1, got %2")
                  .arg(EXPECTED_COLOR_COLUMN_WIDTH).arg(colorColumnWidth));

    CHECK_SET_ERR(static_cast<double>(colorColumnWidth) / totalTableWidth >= 0.2,
                  QString("Color column is too narrow: it's width is %1, the table width is %2")
                  .arg(colorColumnWidth).arg(totalTableWidth));

    const int ANNOTATION_NAME_COLUMN_NUMBER = 0;
    int annotationNameColumnWidth = table->columnWidth(ANNOTATION_NAME_COLUMN_NUMBER);
    CHECK_SET_ERR(annotationNameColumnWidth == totalTableWidth - colorColumnWidth - MAGIC_NUMBER,
                  QString("Annotation name column isn't stretched: it's width is %1, width of the color column is %2, "
                          "the table width is %3")
                  .arg(annotationNameColumnWidth).arg(colorColumnWidth).arg(totalTableWidth));

//    3. Drag and drop the options panel tab's left border to enlarge it.
    GTUtilsOptionsPanel::resizeToMaximum(os);

//    Expected state: the second column's width is 60 pixels; the first column took all available space.
    colorColumnWidth = table->columnWidth(COLOR_COLUMN_NUMBER);
    totalTableWidth = table->width();
    annotationNameColumnWidth = table->columnWidth(ANNOTATION_NAME_COLUMN_NUMBER);

    CHECK_SET_ERR(EXPECTED_COLOR_COLUMN_WIDTH == colorColumnWidth,
                  QString("Color column width is incorrect after resizing: expected %1, got %2")
                  .arg(EXPECTED_COLOR_COLUMN_WIDTH).arg(colorColumnWidth));

    CHECK_SET_ERR(annotationNameColumnWidth == totalTableWidth - colorColumnWidth - MAGIC_NUMBER,
                  QString("Annotation name column isn't stretched after resizing: it's width is %1, width of the color column is %2, "
                          "the table width is %3")
                  .arg(annotationNameColumnWidth).arg(colorColumnWidth).arg(totalTableWidth));
}

GUI_TEST_CLASS_DEFINITION(test_6279) {
    class Custom : public CustomScenario {
    public:
        virtual void run(HI::GUITestOpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "dialog not found");

            QLineEdit *lineEdit = dialog->findChild<QLineEdit*>("leAnnotationName");
            CHECK_SET_ERR(lineEdit != NULL, "line edit leAnnotationName not found");

            QRadioButton* gbFormatLocation = dialog->findChild<QRadioButton*>("rbGenbankFormat");
            CHECK_SET_ERR(gbFormatLocation != NULL, "radio button rbGenbankFormat not found");

            QLineEdit *lineEdit1 = dialog->findChild<QLineEdit*>("leLocation");
            CHECK_SET_ERR(lineEdit1 != NULL, "line edit leLocation not found");

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };
    //1. Open murine.gb
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/murine.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    //2. Click CDS annotation on pan view
    GTUtilsSequenceView::clickAnnotationPan(os, "CDS", 2970, 0, true);
    //3. Press F2 to open Edit annotation dialog
    GTUtilsDialog::waitForDialog(os, new EditAnnotationFiller(os, new Custom()));
    GTKeyboardDriver::keyClick(Qt::Key_F2);
    GTGlobals::sleep(1000);
}

GUI_TEST_CLASS_DEFINITION(test_6283) {
    class Custom : public CustomScenario {
        void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "AppSettingsDialogFiller isn't found");

            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::ExternalTools);

            //2. Open a python tab
            AppSettingsDialogFiller::isExternalToolValid(os, "python");

            //Expected:: Bio module is valid
            bool isToolValid = true;
#ifndef Q_OS_WIN
            isToolValid = AppSettingsDialogFiller::isExternalToolValid(os, "Bio");
#endif
            if (!isToolValid) {
                os.setError("Bio is not valid");
            }

            //Expected: Bio module version is 1.73
            bool hasVerion = true;
#ifndef Q_OS_WIN
            hasVerion = AppSettingsDialogFiller::isToolDescriptionContainsString(os, "Bio", "Version: 1.73");
#endif
            if (!hasVerion) {
                os.setError("Incorrect Bio version");
            }

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    //1. Open "UGENE Application Settings", select "External Tools" tab.
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new Custom()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings" << "Preferences...", GTGlobals::UseMouse);

    CHECK_SET_ERR(!os.hasError(), os.getError());

}

GUI_TEST_CLASS_DEFINITION(test_6291) {
    //1. Open murine.gb
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/murine.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    //2. Click CDS annotation on pan view
    //GTUtilsSequenceView::clickAnnotationPan(os, "CDS", 2970, 0, true);
    //3. Select qualifier
    QString qValue = GTUtilsAnnotationsTreeView::getQualifierValue(os, "product", GTUtilsAnnotationsTreeView::findItem(os, "CDS"));
    //QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "db_xref");
    GTUtilsAnnotationsTreeView::clickItem(os, "product", 1, false);
    //4. Click active action "Copy qualifier..." in menu actions
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Copy/Paste" << "Copy qualifier 'product' value", GTGlobals::UseMouse);
    QString actualValue = GTClipboard::text(os);
    CHECK_SET_ERR(actualValue == qValue, QString("Qualifier text %1 differs with expected %2.").arg(actualValue).arg(qValue));
}

GUI_TEST_CLASS_DEFINITION(test_6301) {
    class Custom : public CustomScenario {
        void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "AppSettingsDialogFiller isn't found");

            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::ExternalTools);

            //Expected: SPAdes description contains the following string - "Version: 3.13.0"
            const bool hasVersion = AppSettingsDialogFiller::isToolDescriptionContainsString(os, "SPAdes", "Version: 3.13.0");
            if (!hasVersion) {
                os.setError("Unexpected SPAdes version");
            }

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    //1. Open "UGENE Application Settings", select "External Tools" tab.
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new Custom()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings" << "Preferences...", GTGlobals::UseMouse);

    CHECK_SET_ERR(!os.hasError(), os.getError());

}

GUI_TEST_CLASS_DEFINITION(test_6309) {

    class SetToolUrlScenario : public CustomScenario {
    public:
        SetToolUrlScenario() : CustomScenario() {}

        void run(HI::GUITestOpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");
            QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
            CHECK_SET_ERR(box != NULL, "buttonBox is NULL");
            QPushButton* pushButton = box->button(QDialogButtonBox::Ok);
            CHECK_SET_ERR(pushButton != NULL, "pushButton is NULL");

            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::ExternalTools);
            QString tabixPath = AppSettingsDialogFiller::getExternalToolPath(os, "Tabix");
            QDir tabixDir(tabixPath);
            tabixDir.cdUp();
            tabixDir.cdUp();
            QString extToolsPath = tabixDir.absolutePath();
            AppSettingsDialogFiller::setExternalToolsDir(os, extToolsPath);

            CHECK_SET_ERR(pushButton->isEnabled() == false , "pushButton is enabled");
            GTUtilsTaskTreeView::waitTaskFinished(os);
            CHECK_SET_ERR(pushButton->isEnabled() == true, "pushButton is disabled");
            GTWidget::click(os, pushButton);
        }
    };
    //1. Open UGENE
    //2. Open menu Settings->Preferences select page 'External tools'
    //3. Press top '...' button and select folder with external tools
    //Expected state: Ok button and left Tree element with preferences pages are disabled while external tools validating

    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new SetToolUrlScenario()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings" << "Preferences...");
}

GUI_TEST_CLASS_DEFINITION(test_6314) {
    //1. Copy "_common_data/clustal/align.aln" to sandbox and open it
    const QString filePath = sandBoxDir + "test_6043.aln";
    GTFile::copy(os, testDir + "_common_data/clustal/align.aln", filePath);

    GTFileDialog::openFile(os, filePath);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Rename the first sequence with some very long name (more than 150 chars)
    QString veryLongName;
    for (int i = 0; i < 200; i++) {
        veryLongName += "Q";
    }
    GTUtilsMSAEditorSequenceArea::renameSequence(os, "IXI_234", veryLongName);

    //3. Save sequence and close the project
    GTUtilsDialog::waitForDialog(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
    GTMenu::clickMainMenuItem(os, QStringList() << "File" << "Save all", GTGlobals::UseMouse);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsProject::closeProject(os);

    //4. Open the file again
    GTFileDialog::openFile(os, filePath);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected: the length of the first sequence name is 150 chars
    QString name = GTUtilsMSAEditorSequenceArea::getVisibleNames(os).first();

    CHECK_SET_ERR(name.size() == 150,
                  QString("Unexpected sequence name length, expected: 150, current: %1")
                          .arg(name.size()));
}

GUI_TEST_CLASS_DEFINITION(test_6334) {
    //1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //2. Add "Read FASTQ Files with PE Reads", "Classify Sequences with MetaPhlAn2" and connect them
    WorkflowProcessItem* readElement = GTUtilsWorkflowDesigner::addElement(os, "Read FASTQ Files with PE Reads");
    WorkflowProcessItem* metaphlan2Element = GTUtilsWorkflowDesigner::addElement(os, "Classify Sequences with MetaPhlAn2");
    GTUtilsWorkflowDesigner::connect(os, readElement, metaphlan2Element);

    //3. Set "Classify" parameters: Inpet data = PE reads
    GTUtilsWorkflowDesigner::click(os, metaphlan2Element);
    GTUtilsWorkflowDesigner::setParameter(os, "Input data", "PE reads", GTUtilsWorkflowDesigner::comboValue);

    //4. Set the "Input URL 2" slot to "empty"
    QTableWidget* table = GTUtilsWorkflowDesigner::getInputPortsTable(os, 0);
    CHECK_SET_ERR(table != nullptr, "Input Ports table isn't found");

    GTUtilsWorkflowDesigner::setTableValue(os, "Input URL 2", "<empty>", GTUtilsWorkflowDesigner::comboValue, table);

    //5. Click validateThe mandatory "Input URL 2" slot is not connected.
    GTUtilsWorkflowDesigner::click(os, metaphlan2Element);
    GTUtilsWorkflowDesigner::validateWorkflow(os);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(Qt::Key_Enter);

    //Expected: there is the following validation error appeared:
    //Classify Sequences with MetaPhlAn2: The mandatory "Input URL 2" slot is not connected."), "Expected error isn't found
    QStringList errors = GTUtilsWorkflowDesigner::getErrors(os);
    CHECK_SET_ERR(errors.contains("Classify Sequences with MetaPhlAn2: The mandatory \"Input URL 2\" slot is not connected."), "Expected error isn't found");
}

GUI_TEST_CLASS_DEFINITION(test_6350) {
    //1. Open "human_T1.fa"
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Mark the sequence as circular
    GTUtilsProjectTreeView::markSequenceAsCircular(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134)");

    //3. Select a joined region, which contains a part in the end and a part in the beginning
    GTUtilsSequenceView::selectSeveralRegionsByDialog(os, "150000..199950,1..50000");

    //4. Export selected region as sequence
    GTUtilsDialog::waitForDialog(os, new ExportSelectedRegionFiller(os, sandBoxDir + "human_T1_reg.fa", QString()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Export" << "Export selected sequence region...", GTGlobals::UseMouse);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected: there is the only sequence in the exported file
    QStringList list = GTUtilsProjectTreeView::getDocuments(os).value("human_T1_reg.fa");
    CHECK_SET_ERR(list.size() == 1,
                  QString("Unexpected sequence number, expected: 1, current: %1")
                          .arg(list.size()));
    CHECK_SET_ERR(list.first() == "[s] region [150000 199950]",
                  QString("Unexpected sequence name, expected: [s] region [150000 199950], current %1")
                          .arg(list.first()));
}

GUI_TEST_CLASS_DEFINITION(test_6378) {
    //1. Remove Python from external tools
    class Custom : public CustomScenario {
        void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "AppSettingsDialogFiller isn't found");
            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::ExternalTools);
            AppSettingsDialogFiller::clearToolPath(os, "python");
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    //1. Open "UGENE Application Settings", select "External Tools" tab.
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new Custom()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings" << "Preferences...", GTGlobals::UseMouse);

    //2. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //3. Place Metaphlan worker on scene
    GTUtilsWorkflowDesigner::addElement(os, "Classify Sequences with MetaPhlAn2");

    //4. Validate scheme
    GTUtilsWorkflowDesigner::validateWorkflow(os);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(Qt::Key_Enter);
    GTGlobals::sleep();
    //Expected state: validation contains message: Classify Sequences with MetaPhlAn2: External tool "Bio" is not set
    GTUtilsWorkflowDesigner::checkErrorList(os, "Classify Sequences with MetaPhlAn2: External tool \"Bio\" is not set");
}

GUI_TEST_CLASS_DEFINITION(test_6397) {
    //1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //2. Place repeat worker
    GTUtilsWorkflowDesigner::addElement(os, "Find Repeats");

    //Expected state: default value for "Apply 'Max distance' attribute" is True
    GTUtilsWorkflowDesigner::click(os, GTUtilsWorkflowDesigner::getWorker(os, "Find Repeats"));
    QString defaultAttr = GTUtilsWorkflowDesigner::getParameter(os,  "Apply 'Max distance' attribute");
    CHECK_SET_ERR(defaultAttr == "True","Attribute value isn't 'True'");

    //3. Set "Max distance" parameter to 0
    GTUtilsWorkflowDesigner::setParameter(os, "Max distance", "0", GTUtilsWorkflowDesigner::spinValue, GTGlobals::UseKey);
    GTUtilsWorkflowDesigner::click(os, GTUtilsWorkflowDesigner::getWorker(os, "Find Repeats"));
    GTGlobals::sleep();
    GTUtilsWorkflowDesigner::clickParameter(os, "Max distance");

    QList<QWidget*> list;
    foreach(QWidget *w, GTMainWindow::getMainWindowsAsWidget(os)) {
        list.append(w);
    }

    QSpinBox *qsb = nullptr;
    foreach (QWidget *w, list) {
        foreach (QObject *o, w->findChildren<QObject*>()) {
            qsb = qobject_cast<QSpinBox*>(o);
            if (qsb != nullptr) {
                break;
            }
        }
        if (qsb != nullptr) {
            break;
        }
    }

    //Expected state: it set successfully, ensure that 0 is minimum value
    QString maxDistance = GTUtilsWorkflowDesigner::getParameter(os,  "Max distance", true);
    CHECK_SET_ERR(maxDistance == "0 bp", "Attribute value isn't 0 bp");
    CHECK_SET_ERR(qsb->minimum() == 0, "Minimum value isn't 0");

    //4. Open human_t1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    class Custom : public CustomScenario {
        void run(HI::GUITestOpStatus &os){
            GTGlobals::sleep(1000);
            QWidget* dialog = QApplication::activeModalWidget();

            QSpinBox *maxDistanceBox = qobject_cast<QSpinBox *>(GTWidget::findWidget(os, "maxDistBox", dialog));
            GTSpinBox::checkLimits(os, maxDistanceBox, 0, 1000000);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };

    //5. Open repeat finder dialog
    //Expected state: minimum value for max distance combobox is 1
    GTUtilsDialog::waitForDialog(os, new FindRepeatsDialogFiller(os, new Custom()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Analyze" << "Find repeats...", GTGlobals::UseMouse);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_6398) {
    //1. Open "_common_data/regression/6398/6398.gtf" file
    //Expected: 5 similarity points of the 'GTF" format
    GTUtilsDialog::waitForDialog(os, new DocumentFormatSelectorDialogFiller(os, "GTF", 5, 1));
    GTFileDialog::openFile(os, testDir + "_common_data/regression/6398/6398.gtf");
    GTUtilsTaskTreeView::waitTaskFinished(os);
}

GUI_TEST_CLASS_DEFINITION(test_6459) {
    //1. Open "data/samples/Genbank/human_T1.fa".
    //2. Open "Search in Sequence" options panel tab.
    //3. Set "Substitute" algorithm.Check "Search with ambiguous bases" checkbox.
    //4. Ensure that the search is performed on both strands(it is the default value).
    //5. Enter the following pattern : "YYYGYY".
    //Expected result: 2738 results are found.

    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    SchedulerListener listener;
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::Search);
    GTUtilsOptionPanelSequenceView::setAlgorithm(os, "Substitute");
    GTUtilsOptionPanelSequenceView::setSearchWithAmbiguousBases(os);
    GTUtilsOptionPanelSequenceView::enterPattern(os, "YYYGYY");

    GTUtilsTaskTreeView::waitTaskFinished(os);
    CHECK_SET_ERR(GTUtilsOptionPanelSequenceView::checkResultsText(os, "Results: 1/2738"), "Results string not match");
}

GUI_TEST_CLASS_DEFINITION(test_6488_1) {
//    1. Open Workflow Designer.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

//    2. Click "Create element with command line tool" button on the toolbar.
//    3. Fill the dialog with the following data:
//        Element name: "UGENE-6488 test element"
//        Command line tool: Integrated external tool "python"
//        No inputs, parameters and outputs
//        Command: "just a command"
//        Element description on the scene: "description on the scene"
//        Detailed element description: "detailed element description"
//    4. Accept the dialog.
//    Expected state: the element was created and put on the scene.
    CreateElementWithCommandLineToolFiller::ElementWithCommandLineSettings settings;
    settings.elementName = "UGENE-6488 test element";
    settings.tooltype = CreateElementWithCommandLineToolFiller::CommandLineToolType::IntegratedExternalTool;
    settings.tool = "python";
    settings.command = "just a command";
    settings.description = "detailed element description";
    settings.prompter = "description on the scene";
    GTUtilsDialog::waitForDialog(os, new CreateElementWithCommandLineToolFiller(os, settings));
    GTToolbar::clickButtonByTooltipOnToolbar(os, MWTOOLBAR_ACTIVEMDI, "Create element with command line tool");

//    5. Click on the element on the scene.
    GTUtilsWorkflowDesigner::click(os, "UGENE-6488 test element 1");

//    6. Call a context menu on the element on the scene.
//    7. Select "Edit configuration..." menu item.
//    8. Go to the "Command" page in the wizard.
//    9. Set "a modified command" text as command.
//    10. Go to the last page, accept the dialog.
    class ModifyScenario : public CustomScenario {
        void run(GUITestOpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(nullptr != dialog, "Active modal widget is nullptr");

            QWizard *wizard = qobject_cast<QWizard *>(dialog);
            CHECK_SET_ERR(nullptr != wizard, "Can't cast current dialog to QWizard");

            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));

            GTTextEdit::setText(os, GTWidget::findExactWidget<QTextEdit *>(os, "teCommand", dialog), "a modified command");

            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));

            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
            GTWidget::click(os, wizard->button(QWizard::FinishButton));
        }
    };

    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, { "Edit configuration..." }));
    GTUtilsDialog::waitForDialog(os, new CreateElementWithCommandLineToolFiller(os, new ModifyScenario()));
    GTUtilsWorkflowDesigner::click(os, "UGENE-6488 test element 1", QPoint(), Qt::RightButton);

//    11. Edit the element again.
//    12. Go to the "Command" page in the wizard.
//    Expected state: the command is "a modified command".
    class CheckScenario : public CustomScenario {
        void run(GUITestOpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(nullptr != dialog, "Active modal widget is nullptr");

            QWizard *wizard = qobject_cast<QWizard *>(dialog);
            CHECK_SET_ERR(nullptr != wizard, "Can't cast current dialog to QWizard");

            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));

            const QString expectedText = "a modified command";
            const QString actualText = GTTextEdit::getText(os, GTWidget::findExactWidget<QTextEdit *>(os, "teCommand", dialog));
            CHECK_SET_ERR(actualText == expectedText, QString("Unexpected command text: expected '%1', got '%2'").arg(expectedText).arg(actualText));

            GTKeyboardDriver::keyClick(Qt::Key_Escape);
        }
    };

    GTUtilsWorkflowDesigner::addElement(os, "UGENE-6488 test element 1");

    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, { "Edit configuration..." }));
    GTUtilsDialog::waitForDialog(os, new CreateElementWithCommandLineToolFiller(os, new CheckScenario()));
    GTUtilsWorkflowDesigner::click(os, "UGENE-6488 test element 1", QPoint(), Qt::RightButton);

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_6488_2) {
//    1. Open Workflow Designer.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

//    2. Click "Create element with command line tool" button on the toolbar.
//    3. Fill the dialog with the following data:
//        Element name: "UGENE-6488 test element"
//        Command line tool: Integrated external tool "python"
//        No inputs, parameters and outputs
//        Command: "just a command"
//        Element description on the scene: "description on the scene"
//        Detailed element description: "detailed element description"
//    4. Accept the dialog.
//    Expected state: the element was created and put on the scene.
    CreateElementWithCommandLineToolFiller::ElementWithCommandLineSettings settings;
    settings.elementName = "UGENE-6488 test element 2";
    settings.tooltype = CreateElementWithCommandLineToolFiller::CommandLineToolType::IntegratedExternalTool;
    settings.tool = "python";
    settings.command = "just a command";
    settings.description = "detailed element description";
    settings.prompter = "description on the scene";
    GTUtilsDialog::waitForDialog(os, new CreateElementWithCommandLineToolFiller(os, settings));
    GTToolbar::clickButtonByTooltipOnToolbar(os, MWTOOLBAR_ACTIVEMDI, "Create element with command line tool");

//    5. Click on the element on the scene.
    GTUtilsWorkflowDesigner::click(os, "UGENE-6488 test element 2");

//    6. Call a context menu on the element on the scene.
//    7. Select "Edit configuration..." menu item.
//    8. Go to the "Element appearance" page in the wizard.
//    9. Set "a modified description on the scene" text as element description on the scene.
//    10. Go to the last page, accept the dialog.
    class ModifyScenario : public CustomScenario {
        void run(GUITestOpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(nullptr != dialog, "Active modal widget is nullptr");

            QWizard *wizard = qobject_cast<QWizard *>(dialog);
            CHECK_SET_ERR(nullptr != wizard, "Can't cast current dialog to QWizard");

            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));

            GTTextEdit::setText(os, GTWidget::findExactWidget<QTextEdit *>(os, "tePrompter", dialog), "a modified description on the scene");

            GTWidget::click(os, wizard->button(QWizard::NextButton));

            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
            GTWidget::click(os, wizard->button(QWizard::FinishButton));
        }
    };

    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, { "Edit configuration..." }));
    GTUtilsDialog::waitForDialog(os, new CreateElementWithCommandLineToolFiller(os, new ModifyScenario()));
    GTUtilsWorkflowDesigner::click(os, "UGENE-6488 test element 2", QPoint(), Qt::RightButton);

//    11. Edit the element again.
//    12. Go to the "Element appearance" page in the wizard.
//    Expected state: the element description on the scene is "a modified description on the scene".
    class CheckScenario : public CustomScenario {
        void run(GUITestOpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(nullptr != dialog, "Active modal widget is nullptr");

            QWizard *wizard = qobject_cast<QWizard *>(dialog);
            CHECK_SET_ERR(nullptr != wizard, "Can't cast current dialog to QWizard");

            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));
            GTWidget::click(os, wizard->button(QWizard::NextButton));

            const QString expectedText = "a modified description on the scene";
            const QString actualText = GTTextEdit::getText(os, GTWidget::findExactWidget<QTextEdit *>(os, "tePrompter", dialog));
            CHECK_SET_ERR(actualText == expectedText, QString("Unexpected command text: expected '%1', got '%2'").arg(expectedText).arg(actualText));

            GTKeyboardDriver::keyClick(Qt::Key_Escape);
        }
    };

    GTUtilsWorkflowDesigner::addElement(os, "UGENE-6488 test element 2");

    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, { "Edit configuration..." }));
    GTUtilsDialog::waitForDialog(os, new CreateElementWithCommandLineToolFiller(os, new CheckScenario()));
    GTUtilsWorkflowDesigner::click(os, "UGENE-6488 test element 2", QPoint(), Qt::RightButton);

    GTGlobals::sleep();
}

} // namespace GUITest_regression_scenarios

} // namespace U2
