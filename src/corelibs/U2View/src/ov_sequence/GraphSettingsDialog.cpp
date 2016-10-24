/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <U2Core/U2SafePoints.h>

#include <U2Gui/HelpButton.h>
#include <U2Core/QObjectScopedPointer.h>

#include "ADVGraphModel.h"
#include "GraphSettingsDialog.h"
#include "WindowStepSelectorWidget.h"

#define BACKGROUND_COLOR "QPushButton { background-color : %1;}"

namespace U2 {

GraphSettingsDialog::GraphSettingsDialog( GSequenceGraphDrawer* d, const U2Region& range, QWidget* parent )
:QDialog(parent), colorMap(d->getColors())
{
    const GSequenceGraphWindowData& windowData = d->getWindowData();
    const GSequenceGraphMinMaxCutOffData& cutOffData = d->getCutOffData();
    wss = new WindowStepSelectorWidget(this, range, windowData.window, windowData.step);
    mms = new MinMaxSelectorWidget(this, cutOffData.minEdge, cutOffData.maxEdge, cutOffData.enableCuttoff);
    
    
    QFormLayout* form = wss->getFormLayout();
    foreach(const QString& key, colorMap.keys()) {
        QPushButton* colorChangeButton = new QPushButton();
        colorChangeButton->setObjectName(key);
        colorChangeButton->setFixedSize(QSize(25,25));
        connect(colorChangeButton, SIGNAL(clicked()), SLOT(sl_onPickColorButtonClicked()));
        QColor color = colorMap.value(key);
        colorChangeButton->setStyleSheet(QString(BACKGROUND_COLOR).arg(color.name()));
        form->addRow(QString("%1:").arg(key), colorChangeButton);
    }
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    buttonBox->setObjectName("buttonBox");

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch(10);
    buttonsLayout->addWidget(buttonBox);

    QVBoxLayout* l = new QVBoxLayout();
    l->addWidget(wss);
    l->addWidget(mms);
    l->addLayout(buttonsLayout);

    setLayout(l);
    setWindowTitle(tr("Graph Settings"));
    setWindowIcon(QIcon(":core/images/graphs.png"));

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setMinimumWidth(400);

    QPushButton* okButton = buttonBox->button(QDialogButtonBox::Ok);
    QPushButton* cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    new HelpButton(this, buttonBox, "18223033");

    connect(cancelButton, SIGNAL(clicked()), SLOT(sl_onCancelClicked()));
    connect(okButton, SIGNAL(clicked()), SLOT(sl_onOkClicked()));

    okButton->setDefault(true);
    setObjectName("GraphSettingsDialog");
}

void GraphSettingsDialog::sl_onPickColorButtonClicked()
{
    QPushButton* colorButton = qobject_cast<QPushButton*> (sender());
    SAFE_POINT(colorButton, "Button for color is NULL", );

    QString colorName = colorButton->objectName();
    QColor initial = colorMap.value(colorName);

    QObjectScopedPointer<QColorDialog> CD = new QColorDialog(initial, this);
    CD->setOption(QColorDialog::DontUseNativeDialog, qgetenv(ENV_GUI_TEST) == "1");
    CD->exec();
    CHECK(!CD.isNull(), );

    if (CD->result() == QDialog::Accepted){
        QColor newColor = CD->selectedColor();
        colorMap[colorName] = newColor;
        colorButton->setStyleSheet(QString(BACKGROUND_COLOR).arg(newColor.name()));
    }
}

void GraphSettingsDialog::sl_onCancelClicked()
{
    reject();
}

void GraphSettingsDialog::sl_onOkClicked()
{
    QString err = wss->validate();
    QString mmerr = mms->validate();
    if (err.isEmpty() && mmerr.isEmpty()) {
        accept();
        return;
    }
    QMessageBox::critical(this, windowTitle(), err.append(' ').append(mmerr));
}

} // namespace
