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

#include <QWidget>

#include <U2Gui/SaveDocumentController.h>

#include "ui_Default.h"
#include "ui_IlluminaClip.h"

namespace U2 {
namespace LocalWorkflow {
    
class TrimmomaticIlluminaClipController;

class TrimmomaticDefaultSettingsWidget : public QWidget, private Ui_Default {
    Q_DECLARE_TR_FUNCTIONS(TrimmomaticSettingsWidget)
public:
    TrimmomaticDefaultSettingsWidget();
};

class TrimmomaticIlluminaClipSettingsWidget : public QWidget, private Ui_IlluminaClip {
    Q_OBJECT
public:
    TrimmomaticIlluminaClipSettingsWidget(TrimmomaticIlluminaClipController* _owner);
    ~TrimmomaticIlluminaClipSettingsWidget();

    QString getFileName();
    QString getMismatches();
    QString getPalindromeThreshold();
    QString getSimpleThreshold();

private slots:
    void sl_textChanged();

private:
    void initSaveController();
    SaveDocumentController* saveController;
    TrimmomaticIlluminaClipController* owner;
};

}
}