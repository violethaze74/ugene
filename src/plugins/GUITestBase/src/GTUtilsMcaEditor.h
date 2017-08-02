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

#ifndef _U2_GT_UTILS_MCA_EDITOR_H_
#define _U2_GT_UTILS_MCA_EDITOR_H_

class QLabel;

namespace HI{
class GUITestOpStatus;
}

namespace U2 {

class McaEditor;
class McaEditorNameList;
class McaEditorSequenceArea;
class McaEditorWgt;

class GTUtilsMcaEditor {
public:
    static McaEditor *getEditor(HI::GUITestOpStatus &os);
    static McaEditorWgt *getEditorUi(HI::GUITestOpStatus &os);
    static QLabel *getReferenceLabel(HI::GUITestOpStatus &os);
    static McaEditorNameList *getNameListArea(HI::GUITestOpStatus &os);
    static McaEditorSequenceArea *getSequenceArea(HI::GUITestOpStatus &os);

    static QString getReferenceLabelText(HI::GUITestOpStatus &os);

    static int getReadsCount(HI::GUITestOpStatus &os);
    static const QStringList getReadsNames(HI::GUITestOpStatus &os);
    static const QStringList getDirectReadsNames(HI::GUITestOpStatus &os);
    static const QStringList getReverseComplementReadsNames(HI::GUITestOpStatus &os);

    static QRect getReadNameRect(HI::GUITestOpStatus &os, const QString &readName);
    static QRect getReadNameRect(HI::GUITestOpStatus &os, int rowNumber);

    static void moveToReadName(HI::GUITestOpStatus &os, const QString &readName);
    static void clickReadName(HI::GUITestOpStatus &os, const QString &sequenceName, Qt::MouseButton mouseButton = Qt::LeftButton);
};

}   // namespace U2

#endif // _U2_GT_UTILS_MCA_EDITOR_H_
