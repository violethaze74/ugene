/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2023 UniPro <ugene@unipro.ru>
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

#include "GTTestsProjectUserLocking.h"
#include <base_dialogs/GTFileDialog.h>
#include <drivers/GTKeyboardDriver.h>
#include <drivers/GTMouseDriver.h>
#include <primitives/GTWidget.h>

#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QRadioButton>

#include <U2Core/DocumentModel.h>

#include <U2View/AnnotatedDNAViewFactory.h>

#include "GTGlobals.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsTaskTreeView.h"
#include "primitives/GTMenu.h"
#include "primitives/PopupChooser.h"
#include "runnables/ugene/ugeneui/CreateNewProjectWidgetFiller.h"

namespace U2 {

namespace GUITest_common_scenarios_project_user_locking {
using namespace HI;

GUI_TEST_CLASS_DEFINITION(test_0001) {
    class CreateAnnotationDialogComboBoxChecker : public Filler {
    public:
        CreateAnnotationDialogComboBoxChecker()
            : Filler("CreateAnnotationDialog") {
        }
        void commonScenario() override {
            QWidget* dialog = GTWidget::getActiveModalWidget();
            auto comboBox = GTWidget::findComboBox("cbExistingTable", dialog);
            CHECK_SET_ERR(comboBox->count() == 0, "ComboBox is not empty");
            GTUtilsDialog::clickButtonBox(QDialogButtonBox::Cancel);
        }
    };

    GTFileDialog::openFile(testDir + "_common_data/scenarios/project/proj5.uprj");
    GTUtilsTaskTreeView::waitTaskFinished();

    GTMouseDriver::moveTo(GTUtilsProjectTreeView::getItemCenter("NC_001363 features"));
    GTMouseDriver::doubleClick();
    GTUtilsTaskTreeView::waitTaskFinished();
    GTUtilsSequenceView::checkSequenceViewWindowIsActive();

    GTUtilsDialog::add(new CreateAnnotationDialogComboBoxChecker());
    GTKeyboardDriver::keyClick('n', Qt::ControlModifier);
    GTUtilsDialog::checkNoActiveWaiters();
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    class CreateAnnotationDialogComboBoxChecker : public Filler {
    public:
        CreateAnnotationDialogComboBoxChecker()
            : Filler("CreateAnnotationDialog") {
        }
        void commonScenario() override {
            QWidget* dialog = GTWidget::getActiveModalWidget();
            auto comboBox = GTWidget::findComboBox("cbExistingTable", dialog);
            CHECK_SET_ERR(comboBox->count() != 0, "ComboBox is empty");
            GTUtilsDialog::clickButtonBox(QDialogButtonBox::Cancel);
        }
    };
    GTFileDialog::openFile(testDir + "_common_data/scenarios/project/", "proj3.uprj");
    GTUtilsTaskTreeView::waitTaskFinished();

    QModelIndex item = GTUtilsProjectTreeView::findIndex("1.gb");
    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter("1.gb");

    GTUtilsDialog::waitForDialog(new PopupChooser({"openInMenu", "action_open_view"}));
    GTMouseDriver::moveTo(itemPos);
    GTMouseDriver::click(Qt::RightButton);
    GTUtilsTaskTreeView::waitTaskFinished();
    GTUtilsSequenceView::checkSequenceViewWindowIsActive();
    QIcon itemIconBefore = qvariant_cast<QIcon>(item.data(Qt::DecorationRole));

    GTUtilsDialog::waitForDialog(new PopupChooser({ACTION_DOCUMENT__UNLOCK}));
    GTMouseDriver::moveTo(itemPos);
    GTMouseDriver::click(Qt::RightButton);

    QIcon itemIconAfter = qvariant_cast<QIcon>(item.data(Qt::DecorationRole));
    CHECK_SET_ERR(itemIconBefore.cacheKey() != itemIconAfter.cacheKey(), "Lock icon has not disappear");

    GTUtilsDialog::waitForDialog(new CreateAnnotationDialogComboBoxChecker());
    GTKeyboardDriver::keyClick('n', Qt::ControlModifier);
    GTUtilsDialog::checkNoActiveWaiters();

    GTUtilsDialog::waitForDialog(new PopupChooser({ACTION_DOCUMENT__LOCK}));
    GTMouseDriver::moveTo(itemPos);
    GTMouseDriver::click(Qt::RightButton);
    GTUtilsDialog::checkNoActiveWaiters();
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    QIcon roDocumentIcon(":/core/images/ro_document.png");
    QIcon documentIcon(":/core/images/document.png");

    GTUtilsProject::openFile(testDir + "_common_data/scenarios/project/proj2.uprj");
    GTUtilsProjectTreeView::checkProjectViewIsOpened();

    GTUtilsDocument::checkDocument("1.gb");

    GTMouseDriver::moveTo(GTUtilsProjectTreeView::getItemCenter("NC_001363 features"));
    GTMouseDriver::doubleClick();
    GTUtilsDocument::checkDocument("1.gb", AnnotatedDNAViewFactory::ID);

    QModelIndex item = GTUtilsProjectTreeView::findIndex("1.gb");
    QIcon icon = GTUtilsProjectTreeView::getIcon(item);

    QImage foundImage = icon.pixmap(32, 32).toImage();
    QImage expectedImage = documentIcon.pixmap(32, 32).toImage();
    CHECK_SET_ERR(expectedImage == foundImage, "Icon is locked");

    GTUtilsDialog::waitForDialog(new PopupChooser({ACTION_DOCUMENT__LOCK}));
    GTMouseDriver::moveTo(GTUtilsProjectTreeView::getItemCenter("1.gb"));
    GTMouseDriver::click(Qt::RightButton);

    icon = GTUtilsProjectTreeView::getIcon(item);
    foundImage = icon.pixmap(32, 32).toImage();
    expectedImage = roDocumentIcon.pixmap(32, 32).toImage();
    CHECK_SET_ERR(expectedImage == foundImage, "Icon is unlocked");

    GTUtilsDialog::waitForDialog(new SaveProjectAsDialogFiller("proj2", testDir + "_common_data/scenarios/sandbox/proj2"));
    GTMenu::clickMainMenuItem({"File", "Save project as..."});
    GTUtilsDialog::checkNoActiveWaiters();
    GTUtilsTaskTreeView::waitTaskFinished();

    GTMenu::clickMainMenuItem({"File", "Close project"});
    GTUtilsProjectTreeView::checkProjectViewIsClosed();

    GTUtilsProject::openFile(testDir + "_common_data/scenarios/sandbox/proj2.uprj");
    GTUtilsProjectTreeView::checkProjectViewIsOpened();
    GTUtilsDocument::checkDocument("1.gb");

    item = GTUtilsProjectTreeView::findIndex("1.gb");
    icon = GTUtilsProjectTreeView::getIcon(item);
    foundImage = icon.pixmap(32, 32).toImage();
    expectedImage = roDocumentIcon.pixmap(32, 32).toImage();
    CHECK_SET_ERR(expectedImage == foundImage, "Icon is unlocked");
}

GUI_TEST_CLASS_DEFINITION(test_0005) {
    GTUtilsProject::openFile(dataDir + "samples/ABIF/A01.abi");
    GTUtilsProject::openFile(dataDir + "samples/Genbank/sars.gb");
    Document* d = GTUtilsDocument::getDocument("A01.abi");
    CHECK_SET_ERR(!d->isModificationAllowed(StateLockModType_AddChild), QString("Enable to perform locking/unlocking for : %1").arg(d->getName()));

    d = GTUtilsDocument::getDocument("sars.gb");
    CHECK_SET_ERR(d->isModificationAllowed(StateLockModType_AddChild), QString("Enable to perform locking/unlocking for : %1").arg(d->getName()));
}
}  // namespace GUITest_common_scenarios_project_user_locking

}  // namespace U2
