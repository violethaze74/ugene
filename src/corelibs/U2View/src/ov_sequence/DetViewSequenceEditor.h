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

#ifndef _U2_DET_VIEW_SEQUENCE_EDITOR_H_
#define _U2_DET_VIEW_SEQUENCE_EDITOR_H_

#include <QObject>

#include <QColor>
#include <QDialog>
#include <QMouseEvent>
#include <QTimer>

#include <U2Core/Log.h> // TODO_SVEDIT: remove later

namespace U2 {

class DetView;
class DNASequence;
class U2SequenceObject;
class U2Region;

class DetViewSequenceEditor : public QObject {
    Q_OBJECT
public:
    // DetView can be reduced to GSequenceView or template for MA
    DetViewSequenceEditor(DetView* _view);
    ~DetViewSequenceEditor();

    void reset();

    bool eventFilter(QObject *watched, QEvent *event);

    int             getCursorPosition() const { return cursor; }
    QColor          getCursorColor() const { return cursorColor; }

    // the methods can be private!
    void setCursor(int newPos);
    void navigate(int newPos, bool shiftPressed = false);

    void insertChar(int character);
    void deleteChar(int key);

    void runModifySeqTask(U2SequenceObject* seqObj, const U2Region &region, const DNASequence &sequence);

public slots:
    void sl_editMode(bool active);

private slots:
    void sl_changeCursorColor();

private:
    int         cursor; // TODO_SVEDIT: can be separate class
    QColor      cursorColor;
    QTimer      animationTimer;
    DetView*    view;
};

} // namespace

#endif // _U2_DET_VIEW_SEQUENCE_EDITOR_H_
