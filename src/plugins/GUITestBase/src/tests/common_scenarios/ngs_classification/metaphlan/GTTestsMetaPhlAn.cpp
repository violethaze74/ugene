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

#include "GTTestsMetaPhlAn.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QTreeWidget>

#include <primitives/GTLineEdit.h>
#include <primitives/GTMenu.h>
#include <primitives/GTWidget.h>

#include <GTUtilsTaskTreeView.h>

#include "runnables/ugene/corelibs/U2Gui/AppSettingsDialogFiller.h"

#include "U2Test/UGUITest.h"

namespace U2 {

namespace GUITest_common_scenarios_mg_metaphlan_external_tool {
using namespace HI;

const QString ET_PYTHON = "python";
const QString ET_NUMPY = "numpy";
const QString ET_BOWTIE_2_ALIGNER = "Bowtie 2 aligner";
const QString ET_BOWTIE_2_BUILD = "Bowtie 2 build indexer";
const QString ET_METAPHLAN = "MetaPhlAn2";
const QString UTIL_SCRIPT = "/utils/read_fastx.py";
const QString PATH_PYTHON_WITHOUT_NUMPY = "/_common_data/regression/6253/python_without_numpy";

void checkExternalToolValid(GUITestOpStatus &os, const QString& toolName, const bool shouldBeValid) {
    const bool isToolValid = AppSettingsDialogFiller::isExternalToolValid(os, toolName);
    if (isToolValid != shouldBeValid) {
        os.setError(QString("%1 %2 valid, but %3 be").arg(toolName)
                                                     .arg(shouldBeValid ? "isn't" : "is")
                                                     .arg(shouldBeValid ? "should" : "shoudn't"));
    }
}

void checkUtilScript(GUITestOpStatus &os, const bool shouldBeValid) {
    QString pathToMetaphlan = QDir::toNativeSeparators(AppSettingsDialogFiller::getExternalToolPath(os, ET_METAPHLAN));
    QString pathToMetaphlanDir = QFileInfo(pathToMetaphlan).absolutePath();
    QString utilNativeSeparators = QDir::toNativeSeparators(UTIL_SCRIPT);
    if (QFileInfo::exists(pathToMetaphlanDir + utilNativeSeparators) != shouldBeValid) {
        os.setError(QString("Unitl script %1 doesn't exist").arg(utilNativeSeparators));
    }
}

void checkDependedTools(GUITestOpStatus &os, const QString& tool, const QStringList& toolList) {
    QStringList absentTools;
    foreach(const QString& str, toolList) {
        bool isOk = AppSettingsDialogFiller::isToolDescriptionContainsString(os, tool, str);
        if (!isOk) {
            absentTools << str;
        }
    }

    if (!absentTools.isEmpty()) {
        QString error;
        bool isSingleToolAbsent = absentTools.size() == 1;
        error += QString("%1 tool should be depended on the following %2: ").arg(tool).arg(isSingleToolAbsent ? "tool" : "tools");
        foreach(const QString& t, absentTools) {
            error += QString("%1 ,").arg(t);
        }
        error = error.left(error.size() - 1);
        os.setError(error);
    }
}

QString getPythonWithoutNumpyPath() {
    return UGUITest::dataDir + QDir::toNativeSeparators(PATH_PYTHON_WITHOUT_NUMPY);
}

GUI_TEST_CLASS_DEFINITION(test_0001) {
    class Custom : public CustomScenario {
        void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "AppSettingsDialogFiller isn't found");

            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::ExternalTools);

            //"python" is installed.
            checkExternalToolValid(os, ET_PYTHON, true);

            //"numpy" python module is installed.
            checkExternalToolValid(os, ET_NUMPY, true);

            //"bowtie-align" executable is specified in UGENE.
            checkExternalToolValid(os, ET_BOWTIE_2_ALIGNER, true);

            //"bowtie-build" executable is not specified in UGENE.
            AppSettingsDialogFiller::setExternalToolPath(os, ET_BOWTIE_2_BUILD, sandBoxDir);
            checkExternalToolValid(os, ET_BOWTIE_2_BUILD, false);

            //"utils/read_fastq.py" is present in the metaphlan tool folder.
            checkUtilScript(os, true);

            //"MetaPhlAn2" external tool is specified in UGENE.
            //Expected state: "MetaPhlAn2" tool is present and valid.
            checkExternalToolValid(os, ET_METAPHLAN, true);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    //1. Open "UGENE Application Settings", select "External Tools" tab.
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new Custom()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings" << "Preferences...", GTGlobals::UseMouse);

    CHECK_SET_ERR(!os.hasError(), os.getError());
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    class Custom : public CustomScenario {
        void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "AppSettingsDialogFiller isn't found");

            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::ExternalTools);

            //python" is not installed.
            AppSettingsDialogFiller::setExternalToolPath(os, ET_PYTHON, sandBoxDir);
            checkExternalToolValid(os, ET_PYTHON, false);

            //"numpy" python module is installed.
            checkExternalToolValid(os, ET_NUMPY, false);

            //"bowtie-align" executable is specified in UGENE.
            checkExternalToolValid(os, ET_BOWTIE_2_ALIGNER, true);

            //"utils/read_fastq.py" is present in the metaphlan tool folder.
            checkUtilScript(os, true);

            //Expected state: "MetaPhlAn2" tool is present, but invalid.
            checkExternalToolValid(os, ET_METAPHLAN, false);

            //Expected state: There is a message about tools "python" and "numpy".
            checkDependedTools(os, ET_METAPHLAN, QStringList() << ET_PYTHON << ET_NUMPY);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    //1. Open "UGENE Application Settings", select "External Tools" tab.
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new Custom()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings" << "Preferences...", GTGlobals::UseMouse);

    CHECK_SET_ERR(!os.hasError(), os.getError());
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    class Custom : public CustomScenario {
        void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "AppSettingsDialogFiller isn't found");

            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::ExternalTools);

            //python" is installed.
            AppSettingsDialogFiller::setExternalToolPath(os, ET_PYTHON, getPythonWithoutNumpyPath());
            checkExternalToolValid(os, ET_PYTHON, true);

            //"numpy" python module is not installed.
            checkExternalToolValid(os, ET_NUMPY, false);

            //"bowtie-align" executable is specified in UGENE.
            checkExternalToolValid(os, ET_BOWTIE_2_ALIGNER, true);

            //"utils/read_fastq.py" is present in the metaphlan tool folder.
            checkUtilScript(os, true);

            //Expected state: "MetaPhlAn2" tool is present, but invalid.
            checkExternalToolValid(os, ET_METAPHLAN, false);

            //Expected state: There is a message about "numpy" tool.
            checkDependedTools(os, ET_METAPHLAN, QStringList() << ET_NUMPY);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    //1. Open "UGENE Application Settings", select "External Tools" tab.
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new Custom()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings" << "Preferences...", GTGlobals::UseMouse);

    CHECK_SET_ERR(!os.hasError(), os.getError());
}

GUI_TEST_CLASS_DEFINITION(test_0004) {
    class Custom : public CustomScenario {
        void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "AppSettingsDialogFiller isn't found");

            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::ExternalTools);

            //python" is installed.
            checkExternalToolValid(os, ET_PYTHON, true);

            //"numpy" python module is installed.
            checkExternalToolValid(os, ET_NUMPY, true);

            //"bowtie-align" executable is not specified in UGENE.
            AppSettingsDialogFiller::setExternalToolPath(os, ET_BOWTIE_2_ALIGNER, sandBoxDir);
            checkExternalToolValid(os, ET_BOWTIE_2_ALIGNER, false);

            //"utils/read_fastq.py" is present in the metaphlan tool folder.
            checkUtilScript(os, true);

            //Expected state: "MetaPhlAn2" tool is present, but invalid.
            checkExternalToolValid(os, ET_METAPHLAN, false);

            //Expected state: There is a message about "bowtie2-align" tool.
            checkDependedTools(os, ET_METAPHLAN, QStringList() << ET_BOWTIE_2_ALIGNER);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    //1. Open "UGENE Application Settings", select "External Tools" tab.
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new Custom()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings" << "Preferences...", GTGlobals::UseMouse);

    CHECK_SET_ERR(!os.hasError(), os.getError());
}

GUI_TEST_CLASS_DEFINITION(test_0005) {
    class Custom : public CustomScenario {
        void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "AppSettingsDialogFiller isn't found");

            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::ExternalTools);

            //python" is installed.
            checkExternalToolValid(os, ET_PYTHON, true);

            //"numpy" python module is installed.
            checkExternalToolValid(os, ET_NUMPY, true);

            //"bowtie-align" executable is specified in UGENE.
            checkExternalToolValid(os, ET_BOWTIE_2_ALIGNER, true);

            //"utils/read_fastq.py" is not present in the metaphlan tool folder.
            checkUtilScript(os, false);

            //Expected state: "MetaPhlAn2" tool is present, but invalid.
            checkExternalToolValid(os, ET_METAPHLAN, false);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    //1. Open "UGENE Application Settings", select "External Tools" tab.
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new Custom()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings" << "Preferences...", GTGlobals::UseMouse);

    CHECK_SET_ERR(!os.hasError(), os.getError());
}

GUI_TEST_CLASS_DEFINITION(test_0006) {
    class Custom : public CustomScenario {
        void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "AppSettingsDialogFiller isn't found");

            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::ExternalTools);

            //python" is installed.
            AppSettingsDialogFiller::setExternalToolPath(os, ET_PYTHON, getPythonWithoutNumpyPath());
            checkExternalToolValid(os, ET_PYTHON, true);

            //"numpy" python module is not installed.
            checkExternalToolValid(os, ET_NUMPY, false);

            //"bowtie-align" executable is specified in UGENE.
            checkExternalToolValid(os, ET_BOWTIE_2_ALIGNER, true);

            //"utils/read_fastq.py" is present in the metaphlan tool folder.
            checkUtilScript(os, true);

            //Expected state: "MetaPhlAn2" tool is present, but invalid.
            checkExternalToolValid(os, ET_METAPHLAN, false);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    //1. Open "UGENE Application Settings", select "External Tools" tab.
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new Custom()));
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings" << "Preferences...", GTGlobals::UseMouse);

    CHECK_SET_ERR(!os.hasError(), os.getError());
}


} // namespace GUITest_common_scenarios_mg_metaphlan_external_tool
} // namespace U2
