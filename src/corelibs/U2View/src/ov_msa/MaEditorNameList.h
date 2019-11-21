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

#ifndef _U2_MA_EDITOR_NAME_LIST_H_
#define _U2_MA_EDITOR_NAME_LIST_H_

#include <QMenu>
#include <QRubberBand>
#include <QScrollBar>

#include <U2Core/MultipleSequenceAlignment.h>
#include <U2Core/U2Region.h>

#include "MaCollapseModel.h"
#include "PhyTrees/MSAEditorTreeViewer.h"

namespace U2 {

class GObjectView;
class MaEditor;
class MaEditorSelection;
class MaEditorWgt;
class MaModificationInfo;

class U2VIEW_EXPORT MaEditorNameList: public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(MaEditorNameList)
public:
    MaEditorNameList(MaEditorWgt* ui, QScrollBar* nhBar);
    virtual ~MaEditorNameList();

    QSize getCanvasSize(const QList<int> &seqIdx) const;

    void drawNames(QPixmap &pixmap, const QList<int> &seqIdx, bool drawSelection = false);
    void drawNames(QPainter &painter, const QList<int> &seqIdx, bool drawSelection = false);

    QAction *getEditSequenceNameAction() const;
    QAction *getRemoveSequenceAction() const;


public slots:
    void sl_removeSelectedRows();

protected slots:
    void sl_completeRedraw();

private slots:
    void sl_copyCurrentSequence();
    void sl_editSequenceName();
    void sl_lockedStateChanged();
    void sl_alignmentChanged(const MultipleAlignment &, const MaModificationInfo&);
    void sl_vScrollBarActionPerformed();
    void sl_completeUpdate();
    void sl_onGroupColorsChanged(const GroupColorSchema&);

protected slots:
    virtual void sl_selectionChanged(const MaEditorSelection& current, const MaEditorSelection& prev);
    virtual void sl_updateActions();

protected:
    virtual void updateScrollBar();

protected:
    void resizeEvent(QResizeEvent* e);
    void paintEvent(QPaintEvent* e);
    void keyPressEvent (QKeyEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void focusOutEvent(QFocusEvent* fe);
    void focusInEvent(QFocusEvent* fe);
    void wheelEvent (QWheelEvent * we);
    //todo context menu?
    int getSelectedRow() const;
    virtual QString getTextForRow(int s);
    virtual QString getSeqName(int s);
    void moveSelection(int offset);
    void scrollSelectionToView(bool fromStart);

    bool                completeRedraw;

protected:
    void drawContent(QPainter& p);
public:
    qint64 sequenceIdAtPos(const QPoint &p);
    void clearGroupsSelections();

    virtual U2Region getSelection() const;
    QFont getFont(bool selected) const;

signals:
    void si_sequenceNameChanged(QString prevName, QString newName);
    void si_startMaChanging();
    void si_stopMaChanging(bool modified);

protected:
    virtual void setSelection(int startSeq, int count);
    virtual bool isRowInSelection(int row) const;

    void moveSelectedRegion( int shift );
    void drawAll();

    void drawSelection(QPainter& p);
    void drawCursorFrame(QPainter& p, const QRect& rect);
    void drawSequenceItem(QPainter &painter, const QString &text, const U2Region &yRange, bool selected, bool focused, bool isReference);
    virtual void drawSequenceItem(QPainter &painter, int rowIndex, const U2Region &yRange, const QString &text, bool selected, bool focused);

    virtual void drawCollapsibileSequenceItem(QPainter &painter, int rowIndex, const QString &name, const QRect &rect,
                                      bool selected, bool focused, bool collapsed, bool isReference);
    void drawChildSequenceItem(QPainter &painter, const QString &name, const QRect &rect,
                                        bool selected, bool focused, bool isReference);

    void drawBackground(QPainter& p, const QString& name, const QRect& rect, bool isReferece);
    virtual void drawText(QPainter& p, const QString& name, const QRect& rect, bool selected);
    void drawCollapsePrimitive(QPainter& p, bool collapsed, const QRect& rect);

    virtual void drawRefSequence(QPainter &p, QRect r);

    void clearSelection();

    QRect calculateTextRect(const U2Region& yRange, bool selected) const;
    QRect calculateButtonRect(const QRect& itemRect) const;

    virtual int getAvailableWidth() const;

    QObject*            labels; // used in GUI tests
    MaEditorWgt*        ui;
    QScrollBar*         nhBar;
    QPoint              mousePressPoint;
    bool                dragging;
    GroupColorSchema    groupColors;

    QRubberBand*        rubberBand;
    QAction*            editSequenceNameAction;
    QAction*            copyCurrentSequenceAction;
    QAction*            removeSequenceAction;
    QPixmap*            cachedView;

    static const int CROSS_SIZE = 9;
    static const int CHILDREN_OFFSET = 8;
    static const int MARGIN_TEXT_LEFT = 5;
    static const int MARGIN_TEXT_TOP = 2;
    static const int MARGIN_TEXT_BOTTOM = 2;

protected:
    MaEditor*          editor;
};

}   // namespace U2

#endif // _U2_MA_EDITOR_NAME_LIST_H_
