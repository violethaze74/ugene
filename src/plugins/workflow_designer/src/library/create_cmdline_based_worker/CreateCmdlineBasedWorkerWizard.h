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

#ifndef _U2_CREATE_CMDLINE_BASED_WORKER_WIZARD_H_
#define _U2_CREATE_CMDLINE_BASED_WORKER_WIZARD_H_

#include <QWizard>

#include "ui_CreateCmdlineBasedWorkerWizardAttributesPage.h"
#include "ui_CreateCmdlineBasedWorkerWizardCommandTemplatePage.h"
#include "ui_CreateCmdlineBasedWorkerWizardInputOutputPage.h"
#include "ui_CreateCmdlineBasedWorkerWizardNamePage.h"
#include "library/CfgExternalToolModel.h"

namespace U2 {

class ExternalProcessConfig;

class CreateCmdlineBasedWorkerWizard : public QWizard {
    Q_OBJECT
public:
    explicit CreateCmdlineBasedWorkerWizard(ExternalProcessConfig *initialConfig, QWidget *parent = nullptr);
    ~CreateCmdlineBasedWorkerWizard() override;

    ExternalProcessConfig *takeConfig();

    static const QString PAGE_TITLE_STYLE_SHEET;

    static const QString ATTRIBUTES_DATA_FIELD;
    static const QString ATTRIBUTES_IDS_FIELD;
    static const QString ATTRIBUTES_NAMES_FIELD;
    static const QString COMMAND_TEMPLATE_DESCRIPTION_FIELD;
    static const QString COMMAND_TEMPLATE_FIELD;
    static const QString INPUTS_DATA_FIELD;
    static const QString INPUTS_IDS_FIELD;
    static const QString INPUTS_NAMES_FIELD;
    static const QString OUTPUTS_DATA_FIELD;
    static const QString OUTPUTS_IDS_FIELD;
    static const QString OUTPUTS_NAMES_FIELD;
    static const QString WORKER_DESCRIPTION_FIELD;
    static const QString WORKER_ID_FIELD;
    static const QString WORKER_NAME_FIELD;

private slots:
    void accept() override;

private:
    void init();
    ExternalProcessConfig *createActualConfig() const;

    ExternalProcessConfig *initialConfig;
    ExternalProcessConfig *config;
};

class CreateCmdlineBasedWorkerWizardNamePage : public QWizardPage, private Ui_CreateCmdlineBasedWorkerWizardNamePage {
public:
    CreateCmdlineBasedWorkerWizardNamePage(ExternalProcessConfig *initialConfig);

    void initializePage() override;
    bool validatePage() override;

private:
    ExternalProcessConfig *initialConfig;

    static char const * const WORKER_ID_PROPERTY;
};

class CreateCmdlineBasedWorkerWizardInputOutputPage : public QWizardPage, private Ui_CreateCmdlineBasedWorkerWizardInputOutputPage {
    Q_OBJECT
public:
    CreateCmdlineBasedWorkerWizardInputOutputPage(ExternalProcessConfig *initialConfig);

    void initializePage() override;
    bool isComplete() const override;

signals:
    void si_inputsChanged();
    void si_outputsChanged();

private slots:
    void sl_addInput();
    void sl_deleteInput();
    void sl_addOutput();
    void sl_deleteOutput();
    void sl_updateInputsProperties();
    void sl_updateOutputsProperties();

private:
    static void initModel(QAbstractItemModel *model, const QList<DataConfig> &dataConfigs);

    ExternalProcessConfig *initialConfig;

    CfgExternalToolModel *inputsModel;
    CfgExternalToolModel *outputsModel;

    static char const * const INPUTS_DATA_PROPERTY;
    static char const * const INPUTS_IDS_PROPERTY;
    static char const * const INPUTS_NAMES_PROPERTY;
    static char const * const OUTPUTS_DATA_PROPERTY;
    static char const * const OUTPUTS_IDS_PROPERTY;
    static char const * const OUTPUTS_NAMES_PROPERTY;
};

class CreateCmdlineBasedWorkerWizardAttributesPage : public QWizardPage, private Ui_CreateCmdlineBasedWorkerWizardAttributesPage {
    Q_OBJECT
public:
    CreateCmdlineBasedWorkerWizardAttributesPage(ExternalProcessConfig *initialConfig);

    void initializePage() override;
    bool isComplete() const override;

signals:
    void si_attributesChanged();

private slots:
    void sl_addAttribute();
    void sl_deleteAttribute();
    void sl_updateAttributes();

private:
    static void initModel(QAbstractItemModel *model, const QList<AttributeConfig> &attributeConfigs);

    ExternalProcessConfig *initialConfig;

    CfgExternalToolModelAttributes *model;

    static char const * const ATTRIBUTES_DATA_PROPERTY;
    static char const * const ATTRIBUTES_IDS_PROPERTY;
    static char const * const ATTRIBUTES_NAMES_PROPERTY;
};

class CreateCmdlineBasedWorkerWizardCommandTemplatePage : public QWizardPage, private Ui_CreateCmdlineBasedWorkerWizardCommandTemplatePage {
    Q_OBJECT
public:
    CreateCmdlineBasedWorkerWizardCommandTemplatePage(ExternalProcessConfig *initialConfig);

    void initializePage() override;
    bool isComplete() const override;
    bool validatePage() override;

private:
    ExternalProcessConfig *initialConfig;
};

}   // namespace U2

#endif // _U2_CREATE_CMDLINE_BASED_WORKER_WIZARD_H_
