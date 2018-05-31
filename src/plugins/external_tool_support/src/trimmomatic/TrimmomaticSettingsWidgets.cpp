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

#include <U2Gui/GUIUtils.h>

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
    mismatches->setText(mismatchesValue);
    palindromeThreshold->setText(palindromeThresholdValue);
    simpleThreshold->setText(simpleThresholdValue);

    initSaveController();
}

TrimmomaticIlluminaClipSettingsWidget::~TrimmomaticIlluminaClipSettingsWidget() {
    owner->setFileName(fileName->text());
    owner->setMismatches(mismatches->text());
    owner->setPalindromeThreshold(palindromeThreshold->text());
    owner->setSimpleThreshold(simpleThreshold->text());
    owner->setNullPointerToWidget();
}

QString TrimmomaticIlluminaClipSettingsWidget::getFileName() {
    return fileName->text();
}

QString TrimmomaticIlluminaClipSettingsWidget::getMismatches() {
    return mismatches->text();
}

QString TrimmomaticIlluminaClipSettingsWidget::getPalindromeThreshold() {
    return palindromeThreshold->text();
}

QString TrimmomaticIlluminaClipSettingsWidget::getSimpleThreshold() {
    return simpleThreshold->text();
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

}
}