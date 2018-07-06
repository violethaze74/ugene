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

#include <QLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QMessageBox>

#include <U2Core/QObjectScopedPointer.h>
#include <U2Gui/HelpButton.h>

#include "SpadesDelegate.h"
#include "SpadesWorker.h"

namespace U2 {
namespace LocalWorkflow {

/********************************************************************/
/*SpadesDelegate*/
/********************************************************************/

static const QString PLACEHOLDER("Configure input type");

SpadesDelegate::SpadesDelegate(QObject *parent)
    : PropertyDelegate(parent)
{
}

QVariant SpadesDelegate::getDisplayValue(const QVariant &) const {
    return PLACEHOLDER;
}

PropertyDelegate* SpadesDelegate::clone() {
    return new SpadesDelegate(parent());
}

QWidget* SpadesDelegate::createEditor(QWidget *parent,
                        const QStyleOptionViewItem &/*option*/,
                        const QModelIndex &/*index*/) const {
    SpadesPropertyWidget* editor = new SpadesPropertyWidget(parent);
    connect(editor, SIGNAL(si_valueChanged(QVariant)), SLOT(sl_commit()));
    return editor;
}

PropertyWidget* SpadesDelegate::createWizardWidget(U2OpStatus &,
                                                   QWidget *parent) const {
    return new SpadesPropertyWidget(parent);
}

void SpadesDelegate::setEditorData(QWidget *editor,
                                const QModelIndex &index) const {
    const QVariant value = index.model()->data(index, ConfigurationEditor::ItemValueRole);
    SpadesPropertyWidget* propertyWidget =
                            qobject_cast<SpadesPropertyWidget*>(editor);
    propertyWidget->setValue(value);
}

void SpadesDelegate::setModelData(QWidget *editor,
                                       QAbstractItemModel *model,
                                       const QModelIndex &index) const {
    SpadesPropertyWidget* propertyWidget =
                            qobject_cast<SpadesPropertyWidget*>(editor);
    model->setData(index, propertyWidget->value(),
                            ConfigurationEditor::ItemValueRole);
}

void SpadesDelegate::sl_commit() {
    SpadesPropertyWidget* editor =
                        qobject_cast<SpadesPropertyWidget*>(sender());
    CHECK(editor != NULL, );
    emit commitData(editor);
}

/********************************************************************/
/*SpadesPropertyWidget*/
/********************************************************************/

SpadesPropertyWidget::SpadesPropertyWidget(QWidget* parent, DelegateTags* tags)
                : PropertyWidget(parent, tags) {
    lineEdit = new QLineEdit(this);
    lineEdit->setPlaceholderText(PLACEHOLDER);
    lineEdit->setObjectName("spadesPropertyLineEdit");
    lineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    lineEdit->setReadOnly(true);

    addMainWidget(lineEdit);

    toolButton = new QToolButton(this);
    toolButton->setObjectName("spadescPropertyToolButton");
    toolButton->setText("...");
    toolButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connect(toolButton, SIGNAL(clicked()), SLOT(sl_showDialog()));
    layout()->addWidget(toolButton);

    setObjectName("spadesPropertyWidget");
}

QVariant SpadesPropertyWidget::value() {
    return QVariant::fromValue<QMap<QString, QVariant>>(dialogValue);
}

void SpadesPropertyWidget::setValue(const QVariant& value) {
    dialogValue = value.toMap();
}

void SpadesPropertyWidget::sl_showDialog() {
    QObjectScopedPointer<SpadesPropertyDialog> dialog
        (new SpadesPropertyDialog(dialogValue, this));

    if (QDialog::Accepted == dialog->exec()) {
        CHECK(!dialog.isNull(), );

        dialogValue = dialog->getValue();
        emit si_valueChanged(value());
    }
}

/********************************************************************/
/*SpadesPropertyDialog*/
/********************************************************************/

static const QString MESSAGE_BOX_ERROR = QApplication::tr("At least one of the required input ports should be set in the \"Input data\" parameter.", "SpadesPropertyDialog");

SpadesPropertyDialog::SpadesPropertyDialog(const QMap<QString, QVariant> &value,
    QWidget *parent) : QDialog(parent) {
    setupUi(this);

    new HelpButton(this, buttonBox, HelpButton::INVALID_VALUE);
    setValue(value);
}

QMap<QString, QVariant> SpadesPropertyDialog::getValue() const {
    QMap<QString, QVariant> result;

    //requaired
    if (needRequiredSequencingPlatform()) {
        result.insert(SpadesWorkerFactory::REQUIRED_SEQUENCING_PLATFORM_ID,
                      sequencingPlatformComboBox->currentText());

        if (pairEndCheckBox->isChecked()) {
            result.insert(SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST[0],
                          QString("%1:%2").arg(pairEndReadsDirectionComboBox->currentText())
                                          .arg(pairEndReadsTypeComboBox->currentText()));
        }
        if (higntQualityCheckBox->isChecked()) {
            result.insert(SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST[2],
                          QString("%1:%2").arg(hightQualityReadsDirectionComboBox->currentText())
                                          .arg(hightQualityReadsTypeComboBox->currentText()));
        }
        if (unpairedReadsCheckBox->isChecked()) {
            result.insert(SpadesWorkerFactory::IN_PORT_ID_LIST[0], "");
        }
    }
    if (pacBioCcsCheckBox->isChecked()) {
        result.insert(SpadesWorkerFactory::IN_PORT_ID_LIST[1], "");
    }

    //additional
    if (needAdditionalSequencingPlatform()) {
        result.insert(SpadesWorkerFactory::ADDITIONAL_SEQUENCING_PLATFORM_ID,
                      sequencingPlatformAdditionalComboBox->currentText());

        if (matePairsCheckBox->isChecked()) {
            result.insert(SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST[1],
                          QString("%1:%2").arg(matePairsReadsDirectionComboBox->currentText())
                                          .arg(matePairsTypeComboBox->currentText()));
        }
    }

    if (pacBioClrCheckBox->isChecked()) {
        result.insert(SpadesWorkerFactory::IN_PORT_ID_LIST[2], "");
    }
    if (oxfordNanoporeCheckBox->isChecked()) {
        result.insert(SpadesWorkerFactory::IN_PORT_ID_LIST[3], "");
    }
    if (sangerReadsCheckBox->isChecked()) {
        result.insert(SpadesWorkerFactory::IN_PORT_ID_LIST[4], "");
    }
    if (trustedContigsCheckBox->isChecked()) {
        result.insert(SpadesWorkerFactory::IN_PORT_ID_LIST[5], "");
    }
    if (untrustedContigsCheckBox->isChecked()) {
        result.insert(SpadesWorkerFactory::IN_PORT_ID_LIST[6], "");
    }

    return result;
}

void SpadesPropertyDialog::setValue(const QMap<QString, QVariant> &value) {
    //requaired
    if (value.contains(SpadesWorkerFactory::REQUIRED_SEQUENCING_PLATFORM_ID)) {
        const QString platform = value.value(SpadesWorkerFactory::REQUIRED_SEQUENCING_PLATFORM_ID).toString();
        sequencingPlatformComboBox->setCurrentText(platform);

        if (value.contains(SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST[0])) {
            pairEndCheckBox->setChecked(true);
            const QString pairEndValue = value.value(SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST[0]).toString();
            const QStringList pairEndSplittedValues = pairEndValue.split(":");
            pairEndReadsDirectionComboBox->setCurrentText(pairEndSplittedValues.first());
            pairEndReadsTypeComboBox->setCurrentText(pairEndSplittedValues.last());
        }
        if (value.contains(SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST[2])) {
            higntQualityCheckBox->setChecked(true);
            const QString higntQualityValue = value.value(SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST[0]).toString();
            const QStringList higntQualitySplittedValues = higntQualityValue.split(":");
            hightQualityReadsDirectionComboBox->setCurrentText(higntQualitySplittedValues.first());
            hightQualityReadsTypeComboBox->setCurrentText(higntQualitySplittedValues.last());
        }

        unpairedReadsCheckBox->setChecked(value.contains(SpadesWorkerFactory::IN_PORT_ID_LIST[0]));
    }
    pacBioCcsCheckBox->setChecked(value.contains(SpadesWorkerFactory::IN_PORT_ID_LIST[1]));

    //additional
    if (value.contains(SpadesWorkerFactory::ADDITIONAL_SEQUENCING_PLATFORM_ID)) {
        const QString additionalPlatform = value.value(SpadesWorkerFactory::ADDITIONAL_SEQUENCING_PLATFORM_ID).toString();
        sequencingPlatformAdditionalComboBox->setCurrentText(additionalPlatform);

        if (value.contains(SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST[1])) {
            matePairsCheckBox->setChecked(true);
            const QString matePairsValue = value.value(SpadesWorkerFactory::IN_PORT_PAIRED_ID_LIST[1]).toString();
            const QStringList matePairsSplittedValues = matePairsValue.split(":");
            matePairsReadsDirectionComboBox->setCurrentText(matePairsSplittedValues.first());
            matePairsTypeComboBox->setCurrentText(matePairsSplittedValues.last());
        }
    }
    pacBioClrCheckBox->setChecked(value.contains(SpadesWorkerFactory::IN_PORT_ID_LIST[2]));
    oxfordNanoporeCheckBox->setChecked(value.contains(SpadesWorkerFactory::IN_PORT_ID_LIST[3]));
    sangerReadsCheckBox->setChecked(value.contains(SpadesWorkerFactory::IN_PORT_ID_LIST[4]));
    trustedContigsCheckBox->setChecked(value.contains(SpadesWorkerFactory::IN_PORT_ID_LIST[5]));
    untrustedContigsCheckBox->setChecked(value.contains(SpadesWorkerFactory::IN_PORT_ID_LIST[6]));
}

bool SpadesPropertyDialog::someRequaredParemetrWasChecked() const {
    return pairEndCheckBox->isChecked() ||
           higntQualityCheckBox->isChecked() ||
           unpairedReadsCheckBox->isChecked() ||
           pacBioCcsCheckBox->isChecked();
}

bool SpadesPropertyDialog::needRequiredSequencingPlatform() const {
    return pairEndCheckBox->isChecked() ||
           higntQualityCheckBox->isChecked() ||
           unpairedReadsCheckBox->isChecked();
}

bool SpadesPropertyDialog::needAdditionalSequencingPlatform() const {
    return matePairsCheckBox->isChecked();
}

void SpadesPropertyDialog::accept() {
    CHECK_EXT(someRequaredParemetrWasChecked(),
              QMessageBox::critical(this, windowTitle(), MESSAGE_BOX_ERROR), );

    QDialog::accept();
}


} // namespace LocalWorkflow
} // namespace U2
