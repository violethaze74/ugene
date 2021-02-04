/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2020 UniPro <ugene@unipro.ru>
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

#ifndef _U2_EXPORT_MSA_TO_MSA_DIALOG_H_
#define _U2_EXPORT_MSA_TO_MSA_DIALOG_H_

#include <U2Core/global.h>

#include "ui_ExportMSA2MSADialog.h"

namespace U2 {

class SaveDocumentController;

class ExportMSA2MSADialog : public QDialog, Ui_ExportMSA2MSADialog {
    Q_OBJECT
public:
    ExportMSA2MSADialog(const QString &defaultFileName, const DocumentFormatId &f, bool wholeAlignmentOnly, QWidget *p);

    /*!
     * \class UnknownAmino
     * \brief Enum class, which indicates what char unknown amino bases should be
     */
    enum class UnknownAmino {
        X,
        Gap
    };

    /*!
     * \class TranslationFrame
     * \brief Frame to translate (reverse-complement if minus)
     */
    enum class TranslationFrame {
        One = 1,
        Two = 2,
        Three = 3,
        MinusOne = -1,
        MinusTwo = -2,
        MinusThree = -3
    };

    void updateModel();
    DocumentFormatId formatId;
    QString file;
    /*!
     * Include gaps of trim before translating
     */
    bool includeGaps = false;
    /*!
     * The character unknown amino acids should be translated to
     */
    UnknownAmino unknownAmino = UnknownAmino::X;
    /*!
     * The character we need to start translate from (minus - reverce-complemented)
     */
    TranslationFrame translationFrame = TranslationFrame::One;
    bool addToProjectFlag;
    QString translationTable;
    bool exportWholeAlignment;

private slots:
    void sl_exportClicked();

private:
    void initSaveController(const QString &defaultFileName, const DocumentFormatId &defaultFormatId);

    SaveDocumentController *saveController;
    QList<QString> tableID;
};

}    // namespace U2

#endif
