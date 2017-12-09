/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
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

#include <QPushButton>
#include <QVBoxLayout>

#include <U2Designer/DatasetWidget.h>

#include <U2Gui/HelpButton.h>

#include <U2Lang/URLContainer.h>

#include "GenomicLibraryDialog.h"

namespace U2 {
namespace LocalWorkflow {

SingleDatasetController::SingleDatasetController(const Dataset &dataset, QObject *parent)
    : DatasetsController(QSet<GObjectType>(), parent),
      dataset(dataset)
{
    widgetController = new URLListController(this, &(this->dataset));
}

SingleDatasetController::~SingleDatasetController() {
    widgetController->setParent(NULL);
    delete widgetController;
}

void SingleDatasetController::renameDataset(int, const QString &, U2OpStatus &) {

}

void SingleDatasetController::deleteDataset(int) {

}

void SingleDatasetController::addDataset(const QString &, U2OpStatus &) {

}

void SingleDatasetController::onUrlAdded(URLListController *, URLContainer *) {

}

QWidget *SingleDatasetController::getWigdet() {
    return widgetController->getWidget();
}

const Dataset &SingleDatasetController::getDataset() const {
    return dataset;
}

QStringList SingleDatasetController::names() const {
    return QStringList() << dataset.getName();
}


GenomicLibraryDialog::GenomicLibraryDialog(const Dataset &dataset, QWidget *parent)
    : QDialog(parent),
      singleDatasetController(new SingleDatasetController(dataset, this))
{
    setupUi(this);
    cantainer->layout()->addWidget(singleDatasetController->getWigdet());

    new HelpButton(this, buttonBox, "42");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Select"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
}

Dataset GenomicLibraryDialog::getDataset() const {
    return singleDatasetController->getDataset();
}

}   // namespace LocalWorkflow
}   // namespace U2
