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

#include <QDir>

#include <U2Core/U2SafePoints.h>

#include "TrimmomaticStepsFactory.h"
#include "TrimmomaticSettingsWidgets.h"

namespace U2 {
namespace LocalWorkflow {

/**************************************************************/
/*TrimmomaticSteps*/
/**************************************************************/

const QStringList TrimmomaticSteps::name = QStringList() << "IlluminaCLIP";

/**************************************************************/
/*TrimmomaticBaseController*/
/**************************************************************/

const static QString DEFAULT_DESCRIPTION = ("<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">"
"p, li { white-space: pre-wrap; }"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:10pt;\">Click </span><img src=\":/external_tool_support/images/little_add.png\" /><span style=\" font-size:10pt;\"> and select a step. The following options are available:</span></p>"
"<ul style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;\"><li style=\" font-size:10pt;\" style=\" margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">ILLUMINACLIP: Cut adapter and other illumina-specific sequences from the read.</li>"
"<li style=\" font-size:10pt;\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">SLIDINGWINDOW: Perform a sliding window trimming, cutting once the average quality within the window falls below a threshold.</li>"
"<li style=\" font-size:10pt;\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">LEADING: Cut bases off the start of a read, if below a threshold quality.</li>"
"<li style=\" font-size:10pt;\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">TRAILING: Cut bases off the end of a read, if below a threshold quality.</li>"
"<li style=\" font-size:10pt;\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">CROP: Cut the read to a specified length.</li>"
"<li style=\" font-size:10pt;\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">HEADCROP: Cut the specified number of bases from the start of the read.</li>"
"<li style=\" font-size:10pt;\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">MINLEN: Drop the read if it is below a specified length.</li>"
"<li style=\" font-size:10pt;\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">AVGQUAL: Drop the read if the average quality is below the specified level.</li>"
"<li style=\" font-size:10pt;\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">TOPHRED33: Convert quality scores to Phred-33.</li>"
"<li style=\" font-size:10pt;\" style=\" margin-top:0px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">TOPHRED64: Convert quality scores to Phred-64.</li></ul></body></html>");

QString TrimmomaticBaseController::getDefaultDescription() {
    return DEFAULT_DESCRIPTION;
}

TrimmomaticBaseController::TrimmomaticBaseController() {
    settingsWidget = NULL;
    okEnable = true;
}

QWidget* TrimmomaticBaseController::getSettingsWidget() const {
    return settingsWidget;
}

bool TrimmomaticBaseController::isOkEnable() {
    return okEnable;
}

void TrimmomaticBaseController::setOkEnable(bool isEnable) {
    if (okEnable != isEnable) {
        okEnable = isEnable;
        emit si_checkOkEnable();
    }
}

/**************************************************************/
/*TrimmomaticIlluminaClipController*/
/**************************************************************/

const QString TrimmomaticIlluminaClipController::NAME = TrimmomaticSteps::name[0];
const QString TrimmomaticIlluminaClipController::DESCRIPTION = QObject::tr("<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">"
"p, li { white-space: pre-wrap; }"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:10.25pt; font-weight:400; font-style:normal;\">"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:10pt; font-weight:600;\">ILLUMINACLIP</span></p>"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:10pt;\">This step is used to find and remove Illumina adapters.</span></p>"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:10pt;\">Trimmomatic first compares short sections of an adapter and a read. If they match enough, the entire alignment between the read and adapter is scored. For paired-end reads, the &quot;palindrome&quot; approach is also used to improve the result. See Trimmomatic manual for details.</span></p>"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:10pt;\">Input the following values:</span></p>"
"<ul style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;\"><li style=\" font-size:10pt;\" style=\" margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Adapter sequences</span>: a FASTA file with the adapter sequences. Files for TruSeq2 (GAII machines), TruSeq3 (HiSeq and MiSeq machines) and Nextera kits for SE and PE reads are now available by default. The naming of the various sequences within the specified file determines how they are used.</li>"
"<li style=\" font-size:10pt;\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Seed mismatches</span>: the maximum mismatch count in short sections which will still allow a full match to be performed.</li>"
"<li style=\" font-size:10pt;\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Simple clip threshold</span>: a threshold for simple alignment mode. Values between 7 and 15 are recommended. A perfect match of a 12 base sequence will score just over 7, while 25 bases are needed to score 15.</li>"
"<li style=\" font-size:10pt;\" style=\" margin-top:0px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Palindrome clip threshold</span>: a threshold for palindrome alignment mode. For palindromic matches, a longer alignment is possible. Therefore the threshold can be in the range of 30. Even though this threshold is very high (requiring a match of almost 50 bases) Trimmomatic is still able to identify very, very short adapter fragments.</li></ul>"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:10pt;\">There are also two optional parameters for palindrome mode: </span><span style=\" font-size:10pt; font-weight:600;\">Min adapter length</span><span style=\" font-size:10pt;\"> and </span><span style=\" font-size:10pt; font-weight:600;\">Keep both reads</span><span style=\" font-size:10pt;\">. </span></p></body></html>");

TrimmomaticIlluminaClipController::TrimmomaticIlluminaClipController() 
    : TrimmomaticBaseController() {
    fileNameValue = QDir::currentPath() + "/data/adapters/illumina/TruSeq3-SE.fa";
    mismatchesValue = "2";
    palindromeThresholdValue = "30";
    simpleThresholdValue = "10";
}

const QString TrimmomaticIlluminaClipController::getName() const {
    return NAME;
}

const QString TrimmomaticIlluminaClipController::getDescription() const {
    return DESCRIPTION;
}

QWidget* TrimmomaticIlluminaClipController::createWidget() {
    settingsWidget = new TrimmomaticIlluminaClipSettingsWidget(this);
    return settingsWidget;
}

QString TrimmomaticIlluminaClipController::generateReport() const {
    TrimmomaticIlluminaClipSettingsWidget* widget =
        qobject_cast<TrimmomaticIlluminaClipSettingsWidget*>(settingsWidget);

    QString result = QString("%1:%2:%3:%4:%5")
        .arg(NAME)
        .arg(QFileInfo(widget == NULL ? fileNameValue : widget->getFileName()).baseName())
        .arg(widget == NULL ? mismatchesValue : widget->getMismatches())
        .arg(widget == NULL ? palindromeThresholdValue : widget->getPalindromeThreshold())
        .arg(widget == NULL ? simpleThresholdValue : widget->getSimpleThreshold());

    return result;
}

QString TrimmomaticIlluminaClipController::getFileName() const {
    return fileNameValue;
}

QString TrimmomaticIlluminaClipController::getMismatches() const {
    return mismatchesValue;
}

QString TrimmomaticIlluminaClipController::getPalindromeThreshold() const {
    return palindromeThresholdValue;
}

QString TrimmomaticIlluminaClipController::getSimpleThreshold() const {
    return simpleThresholdValue;
}

void TrimmomaticIlluminaClipController::setFileName(const QString& fileName) {
    fileNameValue = fileName;
}

void TrimmomaticIlluminaClipController::setMismatches(const QString& mismatches) {
    mismatchesValue = mismatches;
}

void TrimmomaticIlluminaClipController::setPalindromeThreshold(const QString& palindromeThreshold) {
    palindromeThresholdValue = palindromeThreshold;
}

void TrimmomaticIlluminaClipController::setSimpleThreshold(const QString& simpleThreshold) {
    simpleThresholdValue = simpleThreshold;
}

void TrimmomaticIlluminaClipController::setNullPointerToWidget() {
    settingsWidget = NULL;
}

}
}