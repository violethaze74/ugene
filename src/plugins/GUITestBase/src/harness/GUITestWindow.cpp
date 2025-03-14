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

#include "GUITestWindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QMouseEvent>
#include <QRadioButton>
#include <QToolButton>

#include <U2Core/Log.h>
#include <U2Core/U2SafePoints.h>

#define CLASS_NAME_CN 0
#define OBJ_NAME_CN 1
#define ACT_NAME_CN 2
#define ACT_TEXT_CN 3

namespace U2 {

GUITestingWindow::GUITestingWindow()
    : isCodeGenerated(true), isFillerGenerated(false) {
    setParent(AppContext::getMainWindow()->getQMainWindow(), Qt::Window);
    setObjectName("GUITestingWindow");
    prepareGUI();

    filter = new EventFilter(this);
    filter->setParent(this);
    QApplication::instance()->installEventFilter(filter);
}

void GUITestingWindow::prepareGUI() {
    setupUi(this);

    tableWidget->setItem(CLASS_NAME_CN, 0, new QTableWidgetItem());
    tableWidget->setItem(OBJ_NAME_CN, 0, new QTableWidgetItem());
    tableWidget->setItem(ACT_NAME_CN, 0, new QTableWidgetItem());
    tableWidget->setItem(ACT_TEXT_CN, 0, new QTableWidgetItem());

    connect(pushButton, SIGNAL(clicked()), this, SLOT(sl_getParentInfo()));
    connect(pushButton_2, SIGNAL(clicked()), this, SLOT(sl_switchCodeGeneration()));
    connect(pushButton_3, SIGNAL(clicked()), this, SLOT(sl_readyToGenerateFiller()));
    show();
}

void GUITestingWindow::updateTable() {
    tableWidget->item(CLASS_NAME_CN, 0)->setText(filter->getClassName());
    tableWidget->item(OBJ_NAME_CN, 0)->setText(filter->getObjName());
    tableWidget->item(ACT_NAME_CN, 0)->setText(filter->getActionName());
    tableWidget->item(ACT_TEXT_CN, 0)->setText(filter->getActionText());
    label->setText("");
}

void GUITestingWindow::appendCode(QString s) {
    textEdit->append(s);
}

void GUITestingWindow::sl_getParentInfo() {
    QObject* o = filter->getBufferObject();
    if (o == nullptr || o->parent() == nullptr) {
        label->setText("parent is NULL!");
        return;
    }
    filter->getInfo(qobject_cast<QWidget*>(filter->getBufferObject()->parent()));
    updateTable();
}

void GUITestingWindow::sl_switchCodeGeneration() {
    isCodeGenerated = !isCodeGenerated;
    if (isCodeGenerated) {
        pushButton_2->setText("switch off code generation");
    } else {
        pushButton_2->setText("switch on code generation");
    }
}

void GUITestingWindow::sl_readyToGenerateFiller() {
    isFillerGenerated = true;
    pushButton_3->setEnabled(false);
    pushButton_3->setText("ready to generate");
}

/**
*****************
*CODE GENERATION*
*****************
*/

bool EventFilter::eventFilter(QObject* obj, QEvent* event) {
    m = dynamic_cast<QMouseEvent*>(event);

    if (m != nullptr && (event->type() == QEvent::MouseButtonPress)) {
        if (m->button() == prevButton && m->globalPos() == prevPos) {
            return QObject::eventFilter(obj, event);
        }
        prevButton = m->button();
        prevPos = m->globalPos();
        QObject* o = obj;
        // do nothing if GuiTestWindow is clicked
        while (o != nullptr) {
            if (o->objectName() == "GUITestingWindowWindow") {
                return QObject::eventFilter(obj, event);
            }
            o = o->parent();
        }
        generateMouseMessage();
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}

void EventFilter::generateMouseMessage() {
    CHECK_EXT(m, coreLog.error(QString("MouseEvent is NULL %1:%2").arg(__FILE__).arg(__LINE__)), );

    // widget info
    QWidget* w = QApplication::widgetAt(m->globalPos());
    if (w != nullptr) {
        getInfo(w);
        gtw->updateTable();
        if (QApplication::activeModalWidget() != nullptr && gtw->is_FillerGenerated()) {
            gtw->appendCode(generateFillerHeader());
            gtw->appendCode(generateFillerSource());
            gtw->setFillerGenerated(false);
            gtw->pushButton_3->setEnabled(true);
            gtw->pushButton_3->setText("generate filler");
        }

        if (gtw->is_CodeGenerated()) {
            gtw->appendCode(generateCode(w));
        }
    }
}

void EventFilter::getInfo(QWidget* w) {
    CHECK(w != nullptr, );
    setBufferObject(w);
    auto menu = qobject_cast<QMenu*>(w);
    if (menu != nullptr) {
        CHECK(m != nullptr, )
        QAction* menuAct = menu->actionAt(menu->mapFromGlobal(m->globalPos()));
        if (menuAct != nullptr) {
            className = menuAct->metaObject()->className();
            objName = menuAct->objectName();
            actionName = menuAct->objectName();
            text = menuAct->text();
            return;
        }
    }

    auto menuBar = qobject_cast<QMenuBar*>(w);
    if (menuBar != nullptr) {
        CHECK(m != nullptr, )
        QAction* menuBarAct = menuBar->actionAt(menuBar->mapFromGlobal(m->globalPos()));
        if (menuBarAct != nullptr) {
            className = menuBarAct->metaObject()->className();
            objName = menuBarAct->objectName();
            actionName = menuBarAct->objectName();
            text = menuBarAct->text();
            return;
        }
    }

    className = w->metaObject()->className();
    objName = w->objectName();
    auto toolButton = qobject_cast<QToolButton*>(w);
    if (toolButton != nullptr && toolButton->defaultAction()) {
        actionName = toolButton->defaultAction()->objectName();
        text = toolButton->defaultAction()->text();
        return;
    }
    actionName = "";
    text = "";
}

QString EventFilter::generateCode(QWidget* w) {
    QString result("");

    // comparing previous focus widget with current.
    if (focusWidget != nullptr && QApplication::focusWidget() != focusWidget) {
        result.append(setValuesWhenFocusGone(focusWidget));
    }

    focusWidget = w;

    auto menuBar = qobject_cast<QMenuBar*>(w);
    if (menuBar != nullptr) {
        result.append(menuBarCode(menuBar));
        return result;
    }

    auto menu = qobject_cast<QMenu*>(w);
    if (menu != nullptr) {
        result.append(menuCode(menu));
        return result;
    }

    auto check = qobject_cast<QCheckBox*>(w);
    if (check) {
        result.append(checkBoxCode(check));
        return result;
    }

    auto radio = qobject_cast<QRadioButton*>(w);
    if (radio) {
        result.append(radioButtonCode(radio));
        return result;
    }

    auto toolButton = qobject_cast<QToolButton*>(w);
    if (toolButton) {
        result.append(toolButtonCode(toolButton));
        return result;
    }

    if (m && m->button() == Qt::RightButton) {
        result.append(contextMenuCode(w));
        return result;
    }
    return result;
}

QString EventFilter::setValuesWhenFocusGone(QWidget* w) {
    CHECK(w != nullptr, "");
    QString result("");

    auto spin = qobject_cast<QSpinBox*>(w);
    if (spin != nullptr) {
        result.append(QString("auto spin = qobject_cast<QSpinBox*>(GTWidget::findWidget(\"%1\"));\n").arg(spin->objectName()));
        result.append(QString("CHECK_SET_ERR(spin != NULL, \"%1 not found!\");\n").arg(spin->objectName()));
        result.append(QString("GTSpinBox::setValue(spin , %1 , GTGlobals::UseKeyBoard);\n\n").arg(spin->value()));
        focusWidget = nullptr;
        return result;
    }

    auto doubleSpin = qobject_cast<QDoubleSpinBox*>(w);
    if (doubleSpin != nullptr) {
        result.append(QString("auto spin = qobject_cast<QDoubleSpinBox*>(GTWidget::findWidget(\"%1\"));\n").arg(doubleSpin->objectName()));
        result.append(QString("CHECK_SET_ERR(spin != NULL, \"%1 not found!\");\n").arg(doubleSpin->objectName()));
        result.append(QString("QDoubleSpinBox::setValue(spin , %1 , GTGlobals::UseKeyBoard);\n\n").arg(doubleSpin->value()));
        focusWidget = nullptr;
        return result;
    }

    auto combo = qobject_cast<QComboBox*>(w);
    if (combo) {
        QString name = w->objectName();
        if (name == "qt_scrollarea_viewport") {
            return "";
        }
        result.append(QString("auto combo = qobject_cast<QComboBox*>(GTWidget::findWidget(\"%1\"));\n").arg(combo->objectName()));
        result.append(QString("CHECK_SET_ERR(combo != NULL, \"%1 not found!\");\n").arg(combo->objectName()));
        result.append(QString("GTComboBox::selectItemByText(combo , \"%1\");\n\n").arg(combo->currentText()));
        focusWidget = nullptr;
        return result;
    }
    return "";
}

QString EventFilter::menuBarCode(QMenuBar* menuBar) const {
    QString result("");
    CHECK(m != nullptr, "");
    CHECK(menuBar != nullptr, "");
    QAction* menuBarAct = menuBar->actionAt(menuBar->mapFromGlobal(m->globalPos()));
    if (menuBarAct != nullptr) {
        result.append(QString("QMenu* menu = GTMenu::showMainMenu(\"%1\");\n").arg(menuBarAct->objectName()));
    }
    return result;
}

QString EventFilter::menuCode(QMenu* menu) {
    QString result("");
    CHECK(m != nullptr, "");
    CHECK(menu != nullptr, "");

    QAction* menuAct = menu->actionAt(menu->mapFromGlobal(m->globalPos()));
    if (menuAct != nullptr) {
        if (menuAct->menu()) {
            if (!isSubmenuClicked) {
                result.append(QString("GTMenu::clickMenuItemByName(menu, QStringList() << \"%1\"").arg(menuAct->objectName()));
                isSubmenuClicked = true;
                return result;
            }
            result.append(QString(" << \"%1\"").arg(menuAct->objectName()));
            return result;
        } else {
            if (isSubmenuClicked) {
                result.append(QString(" << \"%1\");\n\n").arg(menuAct->objectName()));
                isSubmenuClicked = false;
                return result;
            } else {
                result.append(QString("GTMenu::clickMenuItemByName(menu, QStringList() << \"%1\");\n\n").arg(menuAct->objectName()));
                return result;
            }
        }
    }
    return result;
}

QString EventFilter::checkBoxCode(QCheckBox* check) const {
    QString result("");
    CHECK(check != nullptr, "");

    result.append(QString("auto check = qobject_cast<QCheckBox*>(GTWidget::findWidget(\"%1\"));\n").arg(check->objectName()));
    result.append(QString("CHECK_SET_ERR(check != NULL, \"%1 not found!\");\n").arg(check->objectName()));
    result.append(QString("GTCheckBox::setChecked(check, %1);\n\n").arg(!check->isChecked()));

    return result;
}

QString EventFilter::radioButtonCode(QRadioButton* radio) const {
    QString result("");
    CHECK(radio != nullptr, "");

    result.append(QString("QRadioButton* radio = GTRadioButton::getRadioButtonByText(\"%1\");\n").arg(radio->text()));
    result.append(QString("CHECK_SET_ERR(radio != NULL, \"%1 not found!\");\n").arg(radio->objectName()));
    result.append(QString("GTRadioButton::click(radio);\n\n"));

    return result;
}

QString EventFilter::toolButtonCode(QToolButton* toolButton) const {
    QString result("");
    CHECK(toolButton != nullptr, "");

    if (!toolButton->objectName().isEmpty()) {
        result.append(QString("GTWidget::click(GTWidget::findWidget(\"%1\"))\n").arg(toolButton->objectName()));
    } else {
        CHECK(toolButton->defaultAction() != nullptr, "")
        result.append(QString("QAbstractButton* button = GTAction::button(\"%1\");\n").arg(toolButton->defaultAction()->objectName()));
        result.append(QString("GTWidget::click(button));\n\n"));

        if (toolButton->defaultAction()->menu() != nullptr) {
            result.append(QString("GTGlobals::sleep(200)"));
            result.append(QString("auto menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());\n\n"));
        }
    }
    return result;
}

QString EventFilter::contextMenuCode(QWidget* w) const {
    QString result("");
    CHECK(w != nullptr, "");

    if (!w->objectName().isEmpty() && !w->objectName().startsWith("qt_")) {
        result.append(QString("QMenu* menu = GTMenu::showContextMenu(GTWidget::findWidget(\"%1\"));\n").arg(w->objectName()));
    } else {
        result.append(QString("QMenu* menu = GTMenu::showContextMenu(GTWidget::findWidget(write widget name here));\n"));
    }
    return result;
}

/***********************************FILLERS GENERATION*********************************/

#define COMBO_VAR(name) "combo_" + name + "_text"
#define LINEEDIT_VAR(name) "line_" + name + "_text"
#define SPIN_VAR(name) "spin_" + name + "_value"
#define DOUBLE_SPIN_VAR(name) "doubleSpin_" + name + "_value"
#define CHECK_BOX_VAR(name) "checkBox_" + name + "_checked"
#define RADIO_BUTTON_VAR(name) "radio_" + name + "_clicked"
#define BUTTON_VAR(name) "button_" + name + "_clicked"
#define GROUP_BOX_VAR(name) "groupBox_" + name + "_checked"

/**********HEADER**************/
QString EventFilter::generateFillerHeader() {
    QString result;
    result.append("\n\nHEADER\n\n");
    auto dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    QString fillerName = dialog->objectName() + "Filler";

    result.append("#include \"GTUtilsDialog.h\"\n"
                  "#include \"base_dialogs/GTFileDialog.h\"\n\n");
    result.append(QString("namespace U2 {\n"
                          "\n"
                          "class %1 : public Filler {\n"
                          "public:\n"
                          "    class Parameters {\n"
                          "        public:\n"
                          "        Parameters():\n")
                      .arg(fillerName));

    result.append(generateParametersConstructorCode());

    result.append(generateParametersVariablesCode());

    result.append(QString("\n    %1(U2OpStatus &os, Parameters* parameters) :\n"
                          "        Filler(\"%2\"),\n"
                          "        parameters(parameters) {\n"
                          "            CHECK_SET_ERR(parameters, \"Invalid filler parameters: NULL pointer\");\n"
                          "    }\n\n")
                      .arg(fillerName)
                      .arg(dialog->objectName()));
    result.append("virtual void run();\n\n");
    result.append("private:\n\n"
                  "Parameters* parameters;\n\n"
                  "};\n\n"
                  "}\n");
    result.append("\n\nEND OF HEADER\n\n");
    return result;
}

/**************SOURCE******************/
QString EventFilter::generateFillerSource() const {
    QString result;
    result.append("\n\nSOURCE\n\n");
    auto dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    QString fillerName = dialog->objectName() + "Filler";

    result.append(QString("#include \"%1.h\"\n"
                          "#include \"primitives/GTWidget.h\"\n"
                          "#include \"primitives/GTSpinBox.h\"\n"
                          "#include \"api/GTDoubleSpinBox.h\"\n"
                          "#include \"api/GTCheckBox.h\"\n"
                          "#include \"primitives/GTLineEdit.h\"\n"
                          "#include \"primitives/GTComboBox.h\"\n"
                          "#include \"api/GTRadioButton.h\"\n"
                          "#include <QApplication>\n"
                          "#include <QGroupBox>\n"
                          "#include <QComboBox>\n\n")
                      .arg(fillerName));

    result.append(QString("namespace U2 {\n\n"
                          "#define GT_CLASS_NAME \"GTUtilsDialog::%1\"\n"
                          "#define GT_METHOD_NAME \"run\"\n\n"
                          "void %1::run() {\n")
                      .arg(fillerName));
    result.append("    QWidget* dialog = QApplication::activeModalWidget();\n"
                  "    GT_CHECK(dialog, \"activeModalWidget is NULL\");\n\n");

    result.append(generateWidgetsProcessing());

    result.append("}\n"
                  "#undef GT_METHOD_NAME\n"
                  "#undef GT_CLASS_NAME\n\n"
                  "}\n\n");
    result.append("\n\nEND OF SOURCE\n\n");
    return result;
}

QString EventFilter::generateParametersConstructorCode() {
    QString result;
    QList<QWidget*> list = QApplication::activeModalWidget()->findChildren<QWidget*>();

    foreach (QWidget* child, list) {
        if (child->objectName() == "" || child->objectName() == "qt_spinbox_lineedit") {
            continue;
        }
        QString s = defaultVarValuesCode(child);
        if (!s.isEmpty()) {
            result.append("            ");
            result.append(s);
        }
    }

    // cut off last ',' and '\n'
    result.chop(2);
    result.append("{}\n\n\n");
    return result;
}

QString EventFilter::defaultVarValuesCode(QWidget* widget) const {
    if (qobject_cast<QCheckBox*>(widget) != nullptr) {
        return defaultVarValuesCode(qobject_cast<QCheckBox*>(widget));
    }

    if (qobject_cast<QComboBox*>(widget) != nullptr) {
        return defaultVarValuesCode(qobject_cast<QComboBox*>(widget));
    }

    if (qobject_cast<QLineEdit*>(widget) != nullptr) {
        return defaultVarValuesCode(qobject_cast<QLineEdit*>(widget));
    }

    if (qobject_cast<QSpinBox*>(widget) != nullptr) {
        return defaultVarValuesCode(qobject_cast<QSpinBox*>(widget));
    }

    if (qobject_cast<QDoubleSpinBox*>(widget) != nullptr) {
        return defaultVarValuesCode(qobject_cast<QDoubleSpinBox*>(widget));
    }

    if (qobject_cast<QGroupBox*>(widget) != nullptr) {
        return defaultVarValuesCode(qobject_cast<QGroupBox*>(widget));
    }

    if (qobject_cast<QToolButton*>(widget) != nullptr) {
        return defaultVarValuesCode(qobject_cast<QToolButton*>(widget));
    }

    if (qobject_cast<QPushButton*>(widget) != nullptr) {
        return defaultVarValuesCode(qobject_cast<QPushButton*>(widget));
    }

    if (qobject_cast<QRadioButton*>(widget) != nullptr) {
        return defaultVarValuesCode(qobject_cast<QRadioButton*>(widget));
    }

    return "";
}

QString EventFilter::defaultVarValuesCode(QCheckBox* checkBox) const {
    return QString("%1(%2),\n").arg(CHECK_BOX_VAR(checkBox->objectName())).arg(checkBox->isChecked());
}

QString EventFilter::defaultVarValuesCode(QGroupBox* groupBox) const {
    return QString("%1(%2),\n").arg(GROUP_BOX_VAR(groupBox->objectName())).arg(groupBox->isChecked());
}

QString EventFilter::defaultVarValuesCode(QComboBox* combo) const {
    return QString("%1(\"%2\"),\n").arg(COMBO_VAR(combo->objectName())).arg(combo->currentText());
}

QString EventFilter::defaultVarValuesCode(QLineEdit* line) const {
    return QString("%1(\"\"),\n").arg(LINEEDIT_VAR(line->objectName()));
}

QString EventFilter::defaultVarValuesCode(QSpinBox* spinBox) const {
    return QString("%1(%2),\n").arg(SPIN_VAR(spinBox->objectName())).arg(spinBox->value());
}

QString EventFilter::defaultVarValuesCode(QDoubleSpinBox* spinBox) const {
    return QString("%1(%2),\n").arg(DOUBLE_SPIN_VAR(spinBox->objectName())).arg(spinBox->value());
}

QString EventFilter::defaultVarValuesCode(QToolButton* tool) const {
    return QString("%1(false),\n").arg(BUTTON_VAR(tool->objectName()));
}

QString EventFilter::defaultVarValuesCode(QPushButton* push) const {
    return QString("%1(false),\n").arg(BUTTON_VAR(push->objectName()));
}

QString EventFilter::defaultVarValuesCode(QRadioButton* radio) const {
    return QString("%1(false),\n").arg(RADIO_BUTTON_VAR(radio->objectName()));
}

QString EventFilter::generateParametersVariablesCode() const {
    QString result;
    QList<QWidget*> list = QApplication::activeModalWidget()->findChildren<QWidget*>();

    foreach (QWidget* child, list) {
        if (child->objectName() == "" || child->objectName() == "qt_spinbox_lineedit") {
            continue;
        }
        QString s = widgetVariableCode(child);
        if (!s.isEmpty()) {
            result.append("        ");
            result.append(s);
        }
    }
    result.append("    };\n\n");
    return result;
}

QString EventFilter::widgetVariableCode(QWidget* widget) const {
    if (qobject_cast<QCheckBox*>(widget) != nullptr) {
        return widgetVariableCode(qobject_cast<QCheckBox*>(widget));
    }

    if (qobject_cast<QComboBox*>(widget) != nullptr) {
        return widgetVariableCode(qobject_cast<QComboBox*>(widget));
    }

    if (qobject_cast<QLineEdit*>(widget) != nullptr) {
        return widgetVariableCode(qobject_cast<QLineEdit*>(widget));
    }

    if (qobject_cast<QSpinBox*>(widget) != nullptr) {
        return widgetVariableCode(qobject_cast<QSpinBox*>(widget));
    }

    if (qobject_cast<QDoubleSpinBox*>(widget) != nullptr) {
        return widgetVariableCode(qobject_cast<QDoubleSpinBox*>(widget));
    }

    if (qobject_cast<QGroupBox*>(widget) != nullptr) {
        return widgetVariableCode(qobject_cast<QGroupBox*>(widget));
    }

    if (qobject_cast<QToolButton*>(widget) != nullptr) {
        return widgetVariableCode(qobject_cast<QToolButton*>(widget));
    }

    if (qobject_cast<QPushButton*>(widget) != nullptr) {
        return widgetVariableCode(qobject_cast<QPushButton*>(widget));
    }

    if (qobject_cast<QRadioButton*>(widget) != nullptr) {
        return widgetVariableCode(qobject_cast<QRadioButton*>(widget));
    }

    return "";
}

QString EventFilter::widgetVariableCode(QCheckBox* checkBox) const {
    return QString("bool %1;\n").arg(CHECK_BOX_VAR(checkBox->objectName()));
}

QString EventFilter::widgetVariableCode(QGroupBox* groupBox) const {
    return QString("int %1;\n").arg(GROUP_BOX_VAR(groupBox->objectName()));
}

QString EventFilter::widgetVariableCode(QComboBox* comboBox) const {
    return QString("QString %1;\n").arg(COMBO_VAR(comboBox->objectName()));
}

QString EventFilter::widgetVariableCode(QLineEdit* line) const {
    return QString("QString %1;\n").arg(LINEEDIT_VAR(line->objectName()));
}

QString EventFilter::widgetVariableCode(QSpinBox* spinBox) const {
    return QString("int %1;\n").arg(SPIN_VAR(spinBox->objectName()));
}

QString EventFilter::widgetVariableCode(QDoubleSpinBox* spinBox) const {
    return QString("double %1;\n").arg(DOUBLE_SPIN_VAR(spinBox->objectName()));
}

QString EventFilter::widgetVariableCode(QToolButton* tool) const {
    return QString("bool %1;\n").arg(BUTTON_VAR(tool->objectName()));
}

QString EventFilter::widgetVariableCode(QPushButton* push) const {
    return QString("bool %1;\n").arg(BUTTON_VAR(push->objectName()));
}

QString EventFilter::widgetVariableCode(QRadioButton* radio) const {
    return QString("bool %1;\n").arg(RADIO_BUTTON_VAR(radio->objectName()));
}

QString EventFilter::generateWidgetsProcessing() const {
    QString result;
    QList<QWidget*> list = QApplication::activeModalWidget()->findChildren<QWidget*>();

    foreach (QWidget* child, list) {
        if (classes.contains(child->metaObject()->className())) {
            if (child->objectName() == "" || child->objectName() == "qt_spinbox_lineedit") {
                continue;
            }
            result.append(widgetsProcessingCode(child));
        }
    }
    return result;
}

QString EventFilter::widgetsProcessingCode(QWidget* widget) const {
    if (qobject_cast<QCheckBox*>(widget) != nullptr) {
        return widgetsProcessingCode(qobject_cast<QCheckBox*>(widget));
    }

    if (qobject_cast<QComboBox*>(widget) != nullptr) {
        return widgetsProcessingCode(qobject_cast<QComboBox*>(widget));
    }

    if (qobject_cast<QLineEdit*>(widget) != nullptr) {
        return widgetsProcessingCode(qobject_cast<QLineEdit*>(widget));
    }

    if (qobject_cast<QSpinBox*>(widget) != nullptr) {
        return widgetsProcessingCode(qobject_cast<QSpinBox*>(widget));
    }

    if (qobject_cast<QDoubleSpinBox*>(widget) != nullptr) {
        return widgetsProcessingCode(qobject_cast<QDoubleSpinBox*>(widget));
    }

    if (qobject_cast<QGroupBox*>(widget) != nullptr) {
        return widgetsProcessingCode(qobject_cast<QGroupBox*>(widget));
    }

    if (qobject_cast<QToolButton*>(widget) != nullptr) {
        return widgetsProcessingCode(qobject_cast<QToolButton*>(widget));
    }

    if (qobject_cast<QPushButton*>(widget) != nullptr) {
        return widgetsProcessingCode(qobject_cast<QPushButton*>(widget));
    }

    if (qobject_cast<QRadioButton*>(widget) != nullptr) {
        return widgetsProcessingCode(qobject_cast<QRadioButton*>(widget));
    }

    return "";
}

QString EventFilter::widgetsProcessingCode(QCheckBox* checkBox) const {
    return QString("    auto %1 = qobject_cast<QCheckBox*>(GTWidget::findWidget(\"%1\", dialog));\n"
                   "    GT_CHECK(%1, \"%1 is NULL\");\n"
                   "    GTCheckBox::setChecked(%1, parameters->%2);\n\n")
        .arg(checkBox->objectName())
        .arg(CHECK_BOX_VAR(checkBox->objectName()));
}

QString EventFilter::widgetsProcessingCode(QGroupBox* groupBox) const {
    return QString("    auto %1 = qobject_cast<QGroupBox*>(GTWidget::findWidget(\"%1\", dialog));\n"
                   "    %1->setChecked(parameters->%2);\n\n")
        .arg(groupBox->objectName())
        .arg(GROUP_BOX_VAR(groupBox->objectName()));
}

QString EventFilter::widgetsProcessingCode(QComboBox* combo) const {
    return QString("    auto %1 = qobject_cast<QComboBox*>(GTWidget::findWidget(\"%1\", dialog));\n"
                   "    GT_CHECK(%1, \"%1 is NULL\");\n"
                   "    GTComboBox::selectItemByText(%1, parameters->%2);\n\n")
        .arg(combo->objectName())
        .arg(COMBO_VAR(combo->objectName()));
}

QString EventFilter::widgetsProcessingCode(QLineEdit* line) const {
    return QString("    auto %1 = qobject_cast<QLineEdit*>(GTWidget::findWidget(\"%1\", dialog));\n"
                   "    GT_CHECK(%1, \"%1 is NULL\");\n"
                   "    GTLineEdit::setText(%1, parameters->%2);\n\n")
        .arg(line->objectName())
        .arg(LINEEDIT_VAR(line->objectName()));
}

QString EventFilter::widgetsProcessingCode(QSpinBox* spinBox) const {
    return QString("    auto %1 = qobject_cast<QSpinBox*>(GTWidget::findWidget(\"%1\", dialog));\n"
                   "    GT_CHECK(%1, \"%1 is NULL\")\n;"
                   "    GTSpinBox::setValue(%1, parameters->%2);\n\n")
        .arg(spinBox->objectName())
        .arg(SPIN_VAR(spinBox->objectName()));
}

QString EventFilter::widgetsProcessingCode(QDoubleSpinBox* spinBox) const {
    return QString("    auto %1 = qobject_cast<QDoubleSpinBox*>(GTWidget::findWidget(\"%1\", dialog));\n"
                   "    GT_CHECK(%1, \"%1 is NULL\")\n;"
                   "    GTDoubleSpinBox::setValue(%1, parameters->%2);\n\n")
        .arg(spinBox->objectName())
        .arg(DOUBLE_SPIN_VAR(spinBox->objectName()));
}

QString EventFilter::widgetsProcessingCode(QToolButton* tool) const {
    return QString("    if(parameters->%2){\n"
                   "        GTWidget::click(GTWidget::findWidget(\"%1\"));\n"
                   "    }\n\n")
        .arg(tool->objectName())
        .arg(BUTTON_VAR(tool->objectName()));
}

QString EventFilter::widgetsProcessingCode(QPushButton* push) const {
    return QString("    if(parameters->%2){\n"
                   "        GTWidget::click(GTWidget::findWidget(\"%1\"));\n"
                   "    }\n\n")
        .arg(push->objectName())
        .arg(BUTTON_VAR(push->objectName()));
}

QString EventFilter::widgetsProcessingCode(QRadioButton* radio) const {
    return QString("    auto %1 = qobject_cast<QRadioButton*>(GTWidget::findWidget(\"%1\", dialog));\n"
                   "    GT_CHECK(%1, \"%1 not found\");\n"
                   "    GTRadioButton::click(%1);\n\n")
        .arg(radio->objectName());
}

EventFilter::EventFilter(GUITestingWindow* _w)
    : m(nullptr), focusWidget(nullptr), isSubmenuClicked(false), gtw(_w), bufferObj(nullptr) {
    classes << "QComboBox"
            << "QLineEdit"
            << "QToolButton"
            << "QPushButton"
            << "QSpinBox"
            << "QDoubleSpinBox"
            << "QRadioButton"
            << "QCheckBox"
            << "QGroupBox";
}

}  // namespace U2
