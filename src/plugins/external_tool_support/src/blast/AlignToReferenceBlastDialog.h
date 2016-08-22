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

#ifndef _U2_ALIGN_TO_REFERENCE_BLAST_DIALOG_H
#define _U2_ALIGN_TO_REFERENCE_BLAST_DIALOG_H

#include "ui_AlignToReferenceBlastDialog.h"

#include <U2Core/Task.h>

#include <QDialog>


namespace U2 {

class CmdlineInOutTaskRunner;

class AlignToReferenceBlastCmdlineTask : public Task {
public:
    struct Settings {
        Settings()
            : minIdentity(60),
              minLength(0),
              qualityThreshold(30),
              trimBothEnds(true),
              addResultToProject(true)
        {}
        QString referenceUrl;
        QStringList readUrls;
        int minIdentity;
        int minLength;
        int qualityThreshold;
        bool trimBothEnds;
        QString outAlignment;
        bool addResultToProject;
    };

    AlignToReferenceBlastCmdlineTask(const Settings& settings);
    void prepare();
    QList<Task*> onSubTaskFinished(Task *subTask);

    static const QString ALIGN_TO_REF_CMDLINE;
    static const QString REF_ARG;
    static const QString READS_ARG;
    static const QString MIN_ADENTITY_ARG;
    static const QString MIN_LEN_ARG;
    static const QString THRESHOLD_ARG;
    static const QString TRIM_ARG;
    static const QString RESULT_ALIGNMENT_ARG;

private:
    Settings settings;
    CmdlineInOutTaskRunner *cmdlineTask;
};

class AlignToReferenceBlastDialog : public QDialog, public Ui_AlignToReferenceBlastDialog {
    Q_OBJECT
public:
    AlignToReferenceBlastDialog(QWidget* parent);
    AlignToReferenceBlastCmdlineTask::Settings getSettings() const;

    void accept();

private slots:
    void sl_setReference();
    void sl_addRead();
    void sl_removeRead();
    void sl_setOutput();

private:
    void connectSlots();

    AlignToReferenceBlastCmdlineTask::Settings settings;
};

} // namespace

#endif // _U2_ALIGN_TO_REFERENCE_BLAST_DIALOG_H

