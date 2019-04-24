/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2019 UniPro <ugene@unipro.ru>
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

#ifndef _U2_CREATE_EXTERNAL_PROCESS_DIALOG_H_
#define _U2_CREATE_EXTERNAL_PROCESS_DIALOG_H_

#include <U2Lang/Attribute.h>
#include <U2Lang/ConfigurationEditor.h>
#include <U2Lang/Datatype.h>

#include "ui_ExternalProcessWorkerDialog.h"

namespace U2 {

class ExternalProcessConfig;

class CreateExternalProcessDialog: public QWizard {
    Q_OBJECT
public:
    enum DialogMode {
        Creating,
        Editing
    };

    CreateExternalProcessDialog(QWidget *p = nullptr);
    CreateExternalProcessDialog(QWidget *p, ExternalProcessConfig *cfg, bool lastPage);
    ~CreateExternalProcessDialog();
    ExternalProcessConfig* config() const {return cfg;}
    bool validate();
    //bool isComplete() const;

public slots:
    void accept();

private slots:
    void sl_addInput();
    void sl_addOutput();
    void sl_deleteInput();
    void sl_deleteOutput();
    void sl_addAttribute();
    void sl_deleteAttribute();
    void sl_generateTemplateString();
   // void sl_updateButtons();
    void sl_validateName(const QString &);
    void sl_validateCmdLine(const QString &);
    void validateDataModel(const QModelIndex & i1 = QModelIndex(), const QModelIndex & i2 = QModelIndex());
    void validateAttributeModel(const QModelIndex & i1 = QModelIndex(), const QModelIndex & i2 = QModelIndex());
    void validateNextPage();
    void sl_validatePage(int);
    //void sl_OK();

private:
    virtual void showEvent(QShowEvent *event);
    void init();
    bool validateProcessName(const QString &name, QString &error);
    void applyConfig(ExternalProcessConfig *existingConfig);

    Ui_CreateExternalProcessWorkerDialog ui;
    ExternalProcessConfig *initialCfg;
    ExternalProcessConfig *cfg;
    DialogMode mode;
    bool lastPage;
    QString descr1;

    static const int INFO_STRINGS_NUM = 5;
};

}   // namespace U2

#endif // _U2_CREATE_EXTERNAL_PROCESS_DIALOG_H_
