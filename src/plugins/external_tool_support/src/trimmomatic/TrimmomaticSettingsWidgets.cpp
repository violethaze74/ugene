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

#include "TrimmomaticSettingsWidgets.h"
#include "TrimmomaticStepsFactory.h"

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/Log.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/HelpButton.h>

#include <QFile>
#include <QFileInfo>

namespace U2 {
namespace LocalWorkflow {

/******************************************************************/
/*TrimmomaticDefaultSettingsWidget*/
/******************************************************************/

TrimmomaticDefaultSettingsWidget::TrimmomaticDefaultSettingsWidget() {
    Ui_Default::setupUi(this);
}

/******************************************************************/
/*TrimmomaticIlluminaClipSettingsWidget*/
/******************************************************************/

TrimmomaticIlluminaClipSettingsWidget::TrimmomaticIlluminaClipSettingsWidget(TrimmomaticIlluminaClipController* _owner) {
    owner = _owner;
    Ui_IlluminaClip::setupUi(this);

    connect(mismatches, SIGNAL(textChanged(QString)), this, SLOT(sl_textChanged()));
    connect(palindromeThreshold, SIGNAL(textChanged(QString)), this, SLOT(sl_textChanged()));
    connect(simpleThreshold, SIGNAL(textChanged(QString)), this, SLOT(sl_textChanged()));

    const QString mismatchesValue = owner->getMismatches();
    const QString palindromeThresholdValue = owner->getPalindromeThreshold();
    const QString simpleThresholdValue = owner->getSimpleThreshold();
    const QString optionalParametrsValue = owner->getOptionalParametrs();
    const bool isOptionalParametrsValue = owner->optionalSettingsEnabled();
    mismatches->setText(mismatchesValue);
    palindromeThreshold->setText(palindromeThresholdValue);
    simpleThreshold->setText(simpleThresholdValue);
    optionalSettingsDialog = new TrimmomaticOptionalSettings(isOptionalParametrsValue, optionalParametrsValue);

    connect(pushButton, SIGNAL(clicked()), optionalSettingsDialog, SLOT(sl_showDialog()));
    
    initSaveController();
}

TrimmomaticIlluminaClipSettingsWidget::~TrimmomaticIlluminaClipSettingsWidget() {
    owner->setFileName(fileName->text());
    owner->setMismatches(mismatches->text());
    owner->setPalindromeThreshold(palindromeThreshold->text());
    owner->setSimpleThreshold(simpleThreshold->text());
    owner->setOptionalParametrs(optionalSettingsDialog->getParametrs());
    owner->setOptionalSettingsEnabled(optionalSettingsDialog->useOptionalSettings());
    owner->setNullPointerToWidget();
    delete optionalSettingsDialog;
}

QString TrimmomaticIlluminaClipSettingsWidget::getFileName() const {
    return fileName->text();
}

QString TrimmomaticIlluminaClipSettingsWidget::getMismatches() const {
    return mismatches->text();
}

QString TrimmomaticIlluminaClipSettingsWidget::getPalindromeThreshold() const {
    return palindromeThreshold->text();
}

QString TrimmomaticIlluminaClipSettingsWidget::getSimpleThreshold() const {
    return simpleThreshold->text();
}

QString TrimmomaticIlluminaClipSettingsWidget::getOptionalParametrs() const {
    CHECK(optionalSettingsDialog->useOptionalSettings(), QString());

    return optionalSettingsDialog->getParametrs();
}

void TrimmomaticIlluminaClipSettingsWidget::initSaveController() {
    SaveDocumentControllerConfig conf;
    conf.defaultFormatId = BaseDocumentFormats::FASTA;
    conf.defaultDomain = QDir::currentPath() + "/data/adapters/illumina";
    conf.defaultFileName = owner->getFileName();
    conf.saveTitle = tr("Open File");
    conf.fileNameEdit = fileName;
    conf.fileDialogButton = pushButton_2;

    QList<DocumentFormatId> formats = QList<DocumentFormatId>() << BaseDocumentFormats::FASTA;

    saveController = new SaveDocumentController(conf, formats, this);
}

void TrimmomaticIlluminaClipSettingsWidget::sl_textChanged() {
    bool isOkEnable = true;
    bool ok = false;
    const int mismatchesInt = mismatches->text().toInt(&ok);
    bool warning = ok & mismatchesInt >= 0;
    isOkEnable = isOkEnable && warning;
    GUIUtils::setWidgetWarning(mismatches, !warning);
    owner->setMismatches(mismatches->text());

    const int palindromeThresholdInt = palindromeThreshold->text().toInt(&ok);
    warning = ok & palindromeThresholdInt > 0;
    isOkEnable = isOkEnable && warning;
    GUIUtils::setWidgetWarning(palindromeThreshold, !warning);
    owner->setPalindromeThreshold(palindromeThreshold->text());

    const int simpleThresholdInt = simpleThreshold->text().toInt(&ok);
    warning = ok & simpleThresholdInt > 0;
    isOkEnable = isOkEnable && warning;
    GUIUtils::setWidgetWarning(simpleThreshold, !warning);
    owner->setSimpleThreshold(simpleThreshold->text());

    owner->setOkEnable(isOkEnable);
}

/******************************************************************/
/*TrimmomaticOptionalSettings*/
/******************************************************************/

static const QString DEFAULT_PARAMETRS = ":8:false";

TrimmomaticOptionalSettings::TrimmomaticOptionalSettings(const bool isChecked, const QString& parametrs) {
    Ui_IlluminaClipSettings::setupUi(this);
    new HelpButton(this, buttonBox, "21433685");

    SAFE_POINT(parametrs.contains(":"), "Colon is unexpectably absent", );

    const int colon = parametrs.indexOf(":", 1);
    const QString minLengthSpinStringValue = parametrs.mid(1, colon - 1);
    bool ok = false;
    const int minLengthSpinValue = minLengthSpinStringValue.toInt(&ok);
    SAFE_POINT(ok, "Unexpected minimal adapter length parameter", );

    const QString keepBothComboStringValue = parametrs.right(parametrs.size() - colon - 1);
    SAFE_POINT(keepBothComboStringValue == "true" || keepBothComboStringValue == "false",
        "Unexpected combo box parameter", );

    minLengthSpin->setValue(minLengthSpinValue);
    keepBothCombo->setCurrentIndex(keepBothComboStringValue == "false" ? 0 : 1);
    groupBox->setChecked(isChecked);
}

bool TrimmomaticOptionalSettings::useOptionalSettings() const {
    return groupBox->isChecked();
}

QString TrimmomaticOptionalSettings::getParametrs() const {
    QString result = QString(":%1:%2")
        .arg(QString::number(minLengthSpin->value()))
        .arg(keepBothCombo->currentText().toLower());

    return result;
}

void TrimmomaticOptionalSettings::sl_showDialog() {
    const bool isChecked = groupBox->isChecked();
    const int spinValue = minLengthSpin->value();
    const int comboIndex = keepBothCombo->currentIndex();

    if (QDialog::Rejected == exec()) {
        groupBox->setChecked(isChecked);
        minLengthSpin->setValue(spinValue);
        keepBothCombo->setCurrentIndex(comboIndex);
    } 
}

}
}