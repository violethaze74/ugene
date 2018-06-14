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

#include <QAbstractItemView>
#include <QListView>
#include <QMenu>

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/QObjectScopedPointer.h>

#include <U2Gui/HelpButton.h>
#include <U2Gui/MultiClickMenu.h>
#include <U2Gui/WidgetWithLocalToolbar.h>

#include "TrimmomaticDelegate.h"

namespace U2 {
namespace LocalWorkflow {

/********************************************************************/
/*TrimmomaticDelegate*/
/********************************************************************/

static const QString PLACEHOLDER("Configure steps");

TrimmomaticDelegate::TrimmomaticDelegate(QObject *parent)
    : PropertyDelegate(parent)
{
}

QVariant TrimmomaticDelegate::getDisplayValue(const QVariant &value) const {
    QString str = value.value<QString>();
    return str.isEmpty() ? PLACEHOLDER : str;
}

PropertyDelegate* TrimmomaticDelegate::clone() {
    return new TrimmomaticDelegate(parent());
}

QWidget* TrimmomaticDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &,
                                       const QModelIndex &) const {
    TrimmomaticPropertyWidget* editor = new TrimmomaticPropertyWidget(parent);
    connect(editor, SIGNAL(si_valueChanged(QVariant)), SLOT(sl_commit()));
    return editor;
}

PropertyWidget* TrimmomaticDelegate::createWizardWidget(U2OpStatus &,
                                                 QWidget *parent) const {
    return new TrimmomaticPropertyWidget(parent);
}

void TrimmomaticDelegate::setEditorData(QWidget *editor, 
                                        const QModelIndex &index) const {
    const QVariant value = index.model()->data(index, ConfigurationEditor::ItemValueRole);
    TrimmomaticPropertyWidget* propertyWidget = 
                    qobject_cast<TrimmomaticPropertyWidget*>(editor);
    propertyWidget->setValue(value);
}

void TrimmomaticDelegate::setModelData(QWidget *editor, 
                                       QAbstractItemModel *model, 
                                       const QModelIndex &index) const {
    TrimmomaticPropertyWidget* propertyWidget = 
                    qobject_cast<TrimmomaticPropertyWidget*>(editor);
    model->setData(index, propertyWidget->value(), 
                   ConfigurationEditor::ItemValueRole);
}

void TrimmomaticDelegate::sl_commit() {
    TrimmomaticPropertyWidget* editor =
        qobject_cast<TrimmomaticPropertyWidget*>(sender());
    CHECK(editor != NULL, );
    emit commitData(editor);
}

/********************************************************************/
/*TrimmomaticPropertyWidget*/
/********************************************************************/

TrimmomaticPropertyWidget::TrimmomaticPropertyWidget(QWidget* parent,
                DelegateTags* tags) : PropertyWidget(parent, tags) {
    lineEdit = new QLineEdit(this);
    lineEdit->setPlaceholderText(PLACEHOLDER);
    lineEdit->setObjectName("trimmomaticPropertyLineEdit");
    lineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    addMainWidget(lineEdit);

    toolButton = new QToolButton(this);
    toolButton->setObjectName("trimmomaticPropertyToolButton");
    toolButton->setText("...");
    toolButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connect(toolButton, SIGNAL(clicked()), SLOT(sl_showDialog()));
    layout()->addWidget(toolButton);

    setObjectName("TrimmomaticPropertyWidget");
}

QVariant TrimmomaticPropertyWidget::value() {
    return text;
}

void TrimmomaticPropertyWidget::setValue(const QVariant& value) {
    text = value.value<QString>();
    lineEdit->setText(text);
}

void TrimmomaticPropertyWidget::sl_showDialog() {
    QObjectScopedPointer<TrimmomaticPropertyDialog> dialog
                            (new TrimmomaticPropertyDialog(text, this));
    if (QDialog::Accepted == dialog->exec()) {
        CHECK(!dialog.isNull(), );
        text = dialog->getValue();
        lineEdit->setText(text);
        emit si_valueChanged(value());
    }
}

/********************************************************************/
/*TrimmomaticPropertyDialog*/
/********************************************************************/

static const int differentStepsQuantity = 11;

TrimmomaticPropertyDialog::TrimmomaticPropertyDialog(const QString &value, 
                                      QWidget *parent) : QDialog(parent) {
    setupUi(this);
    new HelpButton(this, buttonBox, "21433685");
    
    menu = new QMenu(this);
    new MultiClickMenu(menu);
    for (int i = 0; i < differentStepsQuantity - 10; i++) {//remove -10 in UGENE-6096
        QAction* step = new QAction(TrimmomaticSteps::name[i], menu->menuAction());
        menu->addAction(step);
    }
    buttonAdd->setMenu(menu);

    currentWidget = NULL;
    listSteps->setEditTriggers(QAbstractItemView::NoEditTriggers);
    textDescription->setReadOnly(true);
    enableButtons(false);
    emptySelection();
    
    connectSelectionChanged();
    connect(menu, SIGNAL(triggered(QAction*)), SLOT(sl_addStep(QAction*)));
    connect(buttonAdd, SIGNAL(pressed()), SLOT(showMenu()));
    connect(buttonUp, SIGNAL(pressed()), SLOT(sl_moveStepUp()));
    connect(buttonDown, SIGNAL(pressed()), SLOT(sl_moveStepDown()));
    connect(buttonRemove, SIGNAL(pressed()), SLOT(sl_removeStep()));
}

QString TrimmomaticPropertyDialog::getValue() const {
    QString result;
    const int size = steps.size();
    for (int i = 0; i < size; i++) {
        result += steps[i]->generateReport();
        if (i != size - 1) {
            result += " ";
        }
    }
    return result;
}

void TrimmomaticPropertyDialog::sl_checkOkEnabled() {
    bool isEnable = true;
    foreach(TrimmomaticBaseController* step, steps) {
        isEnable = isEnable && step->isOkEnable();
    }
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isEnable);
}

void TrimmomaticPropertyDialog::sl_selectionChanged() {
    CHECK(listSteps->selectedItems().size() == 1, );

    QListWidgetItem* currentItem = listSteps->selectedItems().first();
    const int selectedStepNum = listSteps->row(currentItem);
    CHECK(selectedStepNum != -1, );

    const int size = listSteps->count();
    SAFE_POINT(0 <= selectedStepNum && selectedStepNum < size,
        "Unexpected selected item", );

    TrimmomaticBaseController* selectedStep = steps[selectedStepNum];

    textDescription->setText(selectedStep->getDescription());

    delete currentWidget;
    currentWidget = selectedStep->createWidget();
    widgetStepSettings->layout()->addWidget(currentWidget);
}

void TrimmomaticPropertyDialog::emptySelection() {
    textDescription->setText(TrimmomaticBaseController::getDefaultDescription());

    delete currentWidget;
    currentWidget = new TrimmomaticDefaultSettingsWidget();
    widgetStepSettings->layout()->addWidget(currentWidget);
}

void TrimmomaticPropertyDialog::sl_addStep(QAction* a) {
    switch (TrimmomaticSteps::name.indexOf(a->text())) {
    case 0:
        TrimmomaticIlluminaClipController* newStep =
            new TrimmomaticIlluminaClipController;
        connect(newStep, SIGNAL(si_checkOkEnable()), this, SLOT(sl_checkOkEnabled()));
        steps << newStep;
        break;
    }

    listSteps->addItem(steps.last()->getName());
    if (steps.size() == 1) {
        enableButtons(true);
    }
}

void TrimmomaticPropertyDialog::sl_moveStepUp() {
    CHECK(listSteps->selectedItems().size() != 0, );

    const int selectedStepNum = listSteps->currentRow();
    CHECK(selectedStepNum != -1, );

    const int size = listSteps->count();
    SAFE_POINT(0 <= selectedStepNum && selectedStepNum < size,
        "Unexpected selected item", );

    CHECK(selectedStepNum != 0, );

    disconnectSelectionChanged();
    QListWidgetItem* item = listSteps->takeItem(selectedStepNum);
    SAFE_POINT_EXT(item != NULL, connectSelectionChanged(), );

    listSteps->insertItem(selectedStepNum - 1, item);
    steps.swap(selectedStepNum, selectedStepNum - 1);
    connectSelectionChanged();
    listSteps->setCurrentItem(item);
}

void TrimmomaticPropertyDialog::sl_moveStepDown() {
    CHECK(listSteps->selectedItems().size() != 0, );

    const int selectedStepNum = listSteps->currentRow();
    CHECK(selectedStepNum != -1, );

    const int size = listSteps->count();
    SAFE_POINT(0 <= selectedStepNum && selectedStepNum < size,
        "Unexpected selected item", );

    CHECK(selectedStepNum != size - 1, );

    disconnectSelectionChanged();
    QListWidgetItem* item = listSteps->takeItem(selectedStepNum);
    SAFE_POINT_EXT(item != NULL, connectSelectionChanged(), );

    listSteps->insertItem(selectedStepNum + 1, item);
    steps.swap(selectedStepNum, selectedStepNum + 1);
    connectSelectionChanged();
    listSteps->setCurrentItem(item);
}

void TrimmomaticPropertyDialog::sl_removeStep() {
    CHECK(listSteps->selectedItems().size() != 0, );

    const int selectedStepNum = listSteps->currentRow();
    CHECK(selectedStepNum != -1, );

    const int size = listSteps->count();
    SAFE_POINT(0 <= selectedStepNum && selectedStepNum < size,
        "Unexpected selected item", );

    listSteps->takeItem(selectedStepNum);
    steps.removeAt(selectedStepNum);
    sl_checkOkEnabled();
    if (steps.size() == 0) {
        enableButtons(false);
        emptySelection();
    }
}

void TrimmomaticPropertyDialog::disconnectSelectionChanged() {
    disconnect(listSteps, SIGNAL(itemSelectionChanged()),
               this, SLOT(sl_selectionChanged()));
}

void TrimmomaticPropertyDialog::connectSelectionChanged() {
    connect(listSteps, SIGNAL(itemSelectionChanged()),
            SLOT(sl_selectionChanged()));
}

void TrimmomaticPropertyDialog::enableButtons(bool setEnabled) {
    buttonUp->setEnabled(setEnabled);
    buttonDown->setEnabled(setEnabled);
    buttonRemove->setEnabled(setEnabled);
}


}   // namespace LocalWorkflow
}   // namespace U2
