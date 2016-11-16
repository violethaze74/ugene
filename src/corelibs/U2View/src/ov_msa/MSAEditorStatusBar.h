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

#ifndef _U2_MSA_EDITOR_STATUS_BAR_H_
#define _U2_MSA_EDITOR_STATUS_BAR_H_

#include <QEvent>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QVector>
#include <QWidget>

#include <U2Core/MultipleSequenceAlignment.h>

namespace U2 {

class MultipleAlignmentObject;
class MaModificationInfo;
class MaEditorSequenceArea;
class MaEditorSelection;

class MSAEditorStatusWidget : public QWidget {
    Q_OBJECT
public:
    MSAEditorStatusWidget(MultipleAlignmentObject* mobj, MaEditorSequenceArea* seqArea);

    bool eventFilter(QObject* obj, QEvent* ev);

private slots:
    void sl_alignmentChanged() {updateCoords();}
    void sl_lockStateChanged() {updateLock();}
    void sl_selectionChanged(const MaEditorSelection& , const MaEditorSelection& ){updateCoords();}
    void sl_findNext();
    void sl_findPrev();
    void sl_findFocus();

private:
    void updateCoords();
    void updateLock();
    MultipleAlignmentObject*    aliObj;
    MaEditorSequenceArea*       seqArea;
    QPixmap                     lockedIcon;
    QPixmap                     unlockedIcon;

    QPushButton*                prevButton;
    QPushButton*                nextButton;
    QLineEdit*                  searchEdit;
    QLabel*                     linesLabel;
    QLabel*                     colsLabel;
    QLabel*                     lockLabel;
    QLabel*                     posLabel;
    QPoint                      lastSearchPos;
    QAction*                    findAction;

};


}//namespace;

#endif
