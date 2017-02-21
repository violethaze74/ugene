/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
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

#include <U2Core/BaseDocumentFormats.h>

#include <U2Gui/SaveDocumentController.h>

#include "ExportMsa2McaDialog.h"

namespace U2 {

ExportMsa2McaDialog::ExportMsa2McaDialog(const QString &defaultFileName, QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    initSaveController(defaultFileName);
}

QString ExportMsa2McaDialog::getSavePath() const {
    return saveController->getSaveFileName();
}

void ExportMsa2McaDialog::initSaveController(const QString &defaultFileName) {
    SaveDocumentControllerConfig config;
    config.defaultFileName = defaultFileName;
    config.defaultFormatId = BaseDocumentFormats::UGENEDB;
    config.fileDialogButton = pbBrowse;
    config.fileNameEdit = lePath;
    config.parentWidget = this;
    config.saveTitle = tr("Export MSA to MCA");

    DocumentFormatConstraints formatConstraints;
    formatConstraints.supportedObjectTypes << GObjectTypes::MULTIPLE_CHROMATOGRAM_ALIGNMENT;
    formatConstraints.addFlagToSupport(DocumentFormatFlag_SupportWriting);

    saveController = new SaveDocumentController(config, formatConstraints, this);
}

}   // namespace U2
