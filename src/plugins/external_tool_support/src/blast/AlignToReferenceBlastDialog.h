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

#ifndef _U2_ALIGN_TO_REFERENCE_BLAST_DIALOG_H_
#define _U2_ALIGN_TO_REFERENCE_BLAST_DIALOG_H_

#include "ui_AlignToReferenceBlastDialog.h"

#include <U2Core/Task.h>

#include <U2Gui/U2SavableWidget.h>

#include <QDialog>


namespace U2 {

class CmdlineInOutTaskRunner;
class SaveDocumentController;
class LoadDocumentTask;

class AlignToReferenceBlastCmdlineTask : public Task {
    Q_OBJECT
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
    virtual ReportResult report();

    static const QString ALIGN_TO_REF_CMDLINE;
    static const QString TRIM_ARG;
    static const QString MIN_LEN_ARG;
    static const QString THRESHOLD_ARG;
    static const QString READS_ARG;
    static const QString MIN_IDENTITY_ARG;
    static const QString REF_ARG;
    static const QString RESULT_ALIGNMENT_ARG;

private:
    Settings settings;
    CmdlineInOutTaskRunner *cmdlineTask;
    LoadDocumentTask *loadRef;
};

class AlignToReferenceBlastDialog : public QDialog, public Ui_AlignToReferenceBlastDialog {
    Q_OBJECT
public:
    AlignToReferenceBlastDialog(QWidget* parent);
    AlignToReferenceBlastCmdlineTask::Settings getSettings() const;

public slots:
    void accept();

private slots:
    void sl_setReference();
    void sl_addRead();
    void sl_removeRead();
    void sl_referenceChanged(const QString &);

private:
    void initSaveController();
    void connectSlots();
    bool fitsDefaultPattern(const QString &name) const;

    SaveDocumentController *saveController;
    AlignToReferenceBlastCmdlineTask::Settings settings;
    U2SavableWidget savableWidget;

    static QStringList lastUsedReadsUrls;
    static const QString defaultOutputName;
};

} // namespace

#endif // _U2_ALIGN_TO_REFERENCE_BLAST_DIALOG_H_
