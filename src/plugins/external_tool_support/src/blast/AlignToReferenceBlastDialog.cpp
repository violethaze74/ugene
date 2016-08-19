/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include "AlignToReferenceBlastDialog.h"

#include <U2Core/CmdlineInOutTaskRunner.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/U2FileDialog.h>

#include <QMessageBox>


namespace U2 {

const QString AlignToReferenceBlastCmdlineTask::ALIGN_TO_REF_CMDLINE = "align-to-reference";
const QString AlignToReferenceBlastCmdlineTask::REF_ARG = "reference";
const QString AlignToReferenceBlastCmdlineTask::READS_ARG = "reads";
const QString AlignToReferenceBlastCmdlineTask::MIN_ADENTITY_ARG = "min-adentity";
const QString AlignToReferenceBlastCmdlineTask::MIN_LEN_ARG = "min-length";
const QString AlignToReferenceBlastCmdlineTask::THRESHOLD_ARG = "threshold";
const QString AlignToReferenceBlastCmdlineTask::TRIM_ARG = "trim-both-ends";
const QString AlignToReferenceBlastCmdlineTask::RESULT_ALIGNMENT_ARG = "out-alignemnt";

AlignToReferenceBlastCmdlineTask::AlignToReferenceBlastCmdlineTask(const Settings &settings)
    : Task(tr("Align to reference workflow wrapper"), TaskFlags_NR_FOSCOE),
      settings(settings),
      cmdlineTask(NULL)
{

}

void AlignToReferenceBlastCmdlineTask::prepare() {
    CmdlineInOutTaskConfig config;

    config.command = "--task=" + ALIGN_TO_REF_CMDLINE;
    QString argString = "--%1=\"%2\"";
    config.arguments << argString.arg(REF_ARG).arg(settings.referenceUrl);
    config.arguments << argString.arg(READS_ARG).arg(settings.readUrls.join(";"));
    config.arguments << argString.arg(MIN_ADENTITY_ARG).arg(settings.minIdentity);
    config.arguments << argString.arg(MIN_LEN_ARG).arg(settings.minLength);
    config.arguments << argString.arg(THRESHOLD_ARG).arg(settings.qualityThreshold);
    config.arguments << argString.arg(TRIM_ARG).arg(settings.trimBothEnds);
    config.arguments << argString.arg(RESULT_ALIGNMENT_ARG).arg(settings.outAlignment);

    cmdlineTask = new CmdlineInOutTaskRunner(config);
    addSubTask(cmdlineTask);
}

AlignToReferenceBlastDialog::AlignToReferenceBlastDialog(QWidget *parent)
    : QDialog(parent) {
    setupUi(this);

    new HelpButton(this, buttonBox, "18220587"); //! TODO: help link

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Align"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    connectSlots();
}

AlignToReferenceBlastCmdlineTask::Settings AlignToReferenceBlastDialog::getSettings() const {
    return settings;
}

void AlignToReferenceBlastDialog::accept() {
    if (referenceLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Reference sequence is not set."));
        return;
    }
    settings.referenceUrl = referenceLineEdit->text();

    if (readsListWidget->count() == 0) {
        QMessageBox::warning(this, tr("Error"),
                             tr("No reads provided."));
        return;
    }
    QStringList readUrls;
    for (int i = 1; i < readsListWidget->count(); i++) {
        QListWidgetItem* item = readsListWidget->item(i);
        SAFE_POINT(item != NULL, "Item is NULL", );
        QString s = item->text();
        readUrls.append(s);
    }
    settings.readUrls = readUrls;

    settings.minIdentity = minIdentitySpinBox->value();
    settings.minLength = minLenSpinBox->value();
    settings.qualityThreshold = qualitySpinBox->value();
    settings.trimBothEnds = trimCheckBox->isChecked();

    if (outputLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Output file is not set."));
        return;
    }
    settings.outAlignment = outputLineEdit->text();
    settings.addResultToProject = addToProjectCheckbox->isChecked();

    QDialog::accept();
}

void AlignToReferenceBlastDialog::sl_setReference() {
    LastUsedDirHelper lod;
    QString filter;

    lod.url = U2FileDialog::getOpenFileName(this, tr("Open reference sequence"), lod.dir, filter);
    if (lod.url.isEmpty()) {
        return;
    }
    referenceLineEdit->setText(lod.url);
}

void AlignToReferenceBlastDialog::sl_addRead() {
    LastUsedDirHelper lod;
    QString filter;

    QStringList readFiles = U2FileDialog::getOpenFileNames(this, tr("Select file(s) with read(s)"), lod.dir, filter);
    if (readFiles.isEmpty()) {
        return;
    }

    foreach (const QString& read, readFiles) {
        if (readsListWidget->findItems(read, Qt::MatchExactly).isEmpty()) {
            readsListWidget->addItem(read);
        }
    }
}

void AlignToReferenceBlastDialog::sl_removeRead() {
    QList<QListWidgetItem*> selection = readsListWidget->selectedItems();
    CHECK(!selection.isEmpty(), );

    foreach (QListWidgetItem* item, selection) {
        readsListWidget->takeItem(readsListWidget->row(item));
    }
}

void AlignToReferenceBlastDialog::sl_setOutput() {
    LastUsedDirHelper lod;
    QString filter;

    lod.url = U2FileDialog::getSaveFileName(this, tr("Select output file"), lod.dir, filter);
    if (lod.url.isEmpty()) {
        return;
    }

    outputLineEdit->setText(lod.url);
}

void AlignToReferenceBlastDialog::connectSlots() {
    connect(setReferenceButton, SIGNAL(clicked(bool)), SLOT(sl_setReference()));
    connect(addReadButton, SIGNAL(clicked(bool)), SLOT(sl_addRead()));
    connect(removeReadButton, SIGNAL(clicked(bool)), SLOT(sl_removeRead()));
    connect(setOutputButton, SIGNAL(clicked(bool)), SLOT(sl_setOutput()));
}

} // namespace
