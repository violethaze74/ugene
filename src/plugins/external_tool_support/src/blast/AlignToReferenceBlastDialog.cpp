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

#include <U2Gui/HelpButton.h>
#include <U2Core/CmdlineInOutTaskRunner.h>


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

    config.command = "--" + ALIGN_TO_REF_CMDLINE;
    QString argString = "--%1=\"%2\"";
    config.arguments << argString.arg(REF_ARG).arg(settings.referenceUrl);
    config.arguments << argString.arg(READS_ARG).arg(settings.readUrl.join(";"));
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
}

AlignToReferenceBlastCmdlineTask::Settings AlignToReferenceBlastDialog::getSettings() const {
    return settings;
}

void AlignToReferenceBlastDialog::accept() {
    // fill the settings
    accept();
}

} // namespace
