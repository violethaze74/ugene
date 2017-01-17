/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include "SnpEffDatabaseDelegate.h"
#include "SnpEffDatabaseListModel.h"
#include "SnpEffSupport.h"

#include <QLayout>

namespace U2 {
namespace LocalWorkflow {

/************************************************************************/
/* SnpEffDatabaseDialog */
/************************************************************************/
SnpEffDatabaseDialog::SnpEffDatabaseDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi(this);

    tableView->setModel(SnpEffSupport::databaseModel);
    tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
}

QString SnpEffDatabaseDialog::getDatabase() const {
    QItemSelectionModel* model = tableView->selectionModel();
    SAFE_POINT(model != NULL, "Selection model is NULL", QString());
    QModelIndexList selection = model->selectedRows();
    SAFE_POINT(selection.size() == 1, "Invalid selection state", QString());
    QModelIndex index = selection.first();
    return SnpEffSupport::databaseModel->getGenome(index.row());
}

/************************************************************************/
/* SnpEffDatabasePropertyWidget */
/************************************************************************/
SnpEffDatabasePropertyWidget::SnpEffDatabasePropertyWidget(QWidget *parent, DelegateTags *tags)
    : PropertyWidget(parent, tags) {
    lineEdit = new QLineEdit(this);
    // TODO: change the translator
    lineEdit->setPlaceholderText(tr("Select the database -->"));
    lineEdit->setReadOnly(true);
    lineEdit->setObjectName("lineEdit");
    lineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    addMainWidget(lineEdit);

    toolButton = new QToolButton(this);
    toolButton->setObjectName("toolButton");
    toolButton->setText("...");
    toolButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connect(toolButton, SIGNAL(clicked()), SLOT(sl_showDialog()));
    layout()->addWidget(toolButton);

    setObjectName("SnpEffDatabasePropertyWidget");
}

QVariant SnpEffDatabasePropertyWidget::value() {
    return lineEdit->text();
}

void SnpEffDatabasePropertyWidget::setValue(const QVariant &value) {
    lineEdit->setText(value.toString());
}

void SnpEffDatabasePropertyWidget::sl_showDialog() {
    SnpEffDatabaseDialog* dlg = new SnpEffDatabaseDialog(this);
    if (dlg->exec() == QDialog::Accepted) {
        lineEdit->setText(dlg->getDatabase());
        emit si_valueChanged(lineEdit->text());
    }
    lineEdit->setFocus();
}

/************************************************************************/
/* SnpEffDatabaseDelegate */
/************************************************************************/
SnpEffDatabaseDelegate::SnpEffDatabaseDelegate(QObject *parent)
    : PropertyDelegate(parent) {
}

QWidget* SnpEffDatabaseDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
                                              const QModelIndex &/*index*/) const {
    SnpEffDatabasePropertyWidget* editor = new SnpEffDatabasePropertyWidget(parent);
    connect(editor, SIGNAL(si_valueChanged(QVariant)), SLOT(sl_commit()));
    return editor;
}

PropertyWidget * SnpEffDatabaseDelegate::createWizardWidget(U2OpStatus & /*os*/, QWidget *parent) const {
    return new SnpEffDatabasePropertyWidget(parent);
}

void SnpEffDatabaseDelegate::setEditorData(QWidget *editor,
                                           const QModelIndex &index) const {
    QVariant val = index.model()->data(index, ConfigurationEditor::ItemValueRole);
    SnpEffDatabasePropertyWidget *propertyWidget = dynamic_cast<SnpEffDatabasePropertyWidget*>(editor);
    propertyWidget->setValue(val);
}

void SnpEffDatabaseDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                          const QModelIndex &index) const {
    SnpEffDatabasePropertyWidget *propertyWidget = dynamic_cast<SnpEffDatabasePropertyWidget*>(editor);
    QString val = propertyWidget->value().toString();
    model->setData(index, val, ConfigurationEditor::ItemValueRole);
}

PropertyDelegate* SnpEffDatabaseDelegate::clone() {
    return new SnpEffDatabaseDelegate(parent());
}

void SnpEffDatabaseDelegate::sl_commit() {
    SnpEffDatabasePropertyWidget* editor = static_cast<SnpEffDatabasePropertyWidget*>(sender());
    CHECK(editor != NULL, );
    emit commitData(editor);
}

} // namespace LocalWorkflow
} // namespace U2
