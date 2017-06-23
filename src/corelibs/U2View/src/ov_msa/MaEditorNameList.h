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

#ifndef _U2_MSA_EDITOR_NAME_LIST_H_
#define _U2_MSA_EDITOR_NAME_LIST_H_

#include <QMenu>
#include <QRubberBand>
#include <QScrollBar>

#include <U2Core/MultipleSequenceAlignment.h>
#include <U2Core/U2Region.h>

#include "MSACollapsibleModel.h"
#include "PhyTrees/MSAEditorTreeViewer.h"

namespace U2 {

class GObjectView;
class MaEditor;
class MaEditorSelection;
class MaEditorWgt;
class MaModificationInfo;
class McaEditorWgt;

class McaEditor;

class U2VIEW_EXPORT MaEditorNameList: public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(MaEditorNameList)
public:
    MaEditorNameList(MaEditorWgt* ui, QScrollBar* nhBar);
    virtual ~MaEditorNameList();

    QSize getCanvasSize(const QList<int> &seqIdx) const;

    void drawNames(QPixmap &pixmap, const QList<int> &seqIdx, bool drawSelection = false);
    void drawNames(QPainter &painter, const QList<int> &seqIdx, bool drawSelection = false);

protected slots:
    void sl_completeRedraw();

private slots:
    virtual void sl_buildStaticMenu(GObjectView* v, QMenu* m);
    virtual void sl_buildContextMenu(GObjectView* v, QMenu* m);
    void sl_copyCurrentSequence();
    void sl_editSequenceName();
    void sl_lockedStateChanged();
    void sl_removeSequence();
    void sl_selectReferenceSequence();
    void sl_alignmentChanged(const MultipleAlignment &, const MaModificationInfo&);
    void sl_vScrollBarActionPerfermed();
    void sl_completeUpdate();
    void sl_onGroupColorsChanged(const GroupColorSchema&);

protected slots:
    virtual void sl_selectionChanged(const MaEditorSelection& current, const MaEditorSelection& prev);

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

    bool                completeRedraw;

protected:
    void drawContent(QPainter& p);
public:
    qint64 sequenceIdAtPos(const QPoint &p);
    void clearGroupsSelections();

    virtual U2Region getSelection() const;

signals:
    void si_sequenceNameChanged(QString prevName, QString newName);
    void si_startMaChanging();
    void si_stopMaChanging(bool modified);

protected:
    virtual void setSelection(int startSeq, int count);
    virtual bool isRowInSelection(int row) const;

    void updateActions();
    void buildMenu(QMenu* m);
    void updateSelection(int newSeqNum);
    void moveSelectedRegion( int shift );
    void drawAll();

    void drawSelection(QPainter& p);
    void drawSequenceItem(QPainter &painter, const QString &text, const U2Region &yRange, bool selected, bool isReference);
    virtual void drawSequenceItem(QPainter &painter, int rowIndex, const U2Region &yRange, const QString &text, bool selected);

    void drawCollapsibileSequenceItem(QPainter &painter, const QString &name, const QRect &rect,
                                      bool selected, bool collapsed, bool isReference);
    void drawChildSequenceItem(QPainter &painter, const QString &name, const QRect &rect,
                                        bool selected, bool isReference);

    // SANGER_TODO: drawSequenceItem should use these methods
    void drawBackground(QPainter& p, const QString& name, const QRect& rect, bool isReferece);
    void drawText(QPainter& p, const QString& name, const QRect& rect, bool selected);
    void drawCollapsePrimitive(QPainter& p, bool collapsed, const QRect& rect);

    virtual void drawRefSequence(QPainter &p, QRect r);

    QFont getFont(bool selected) const;
    QRect calculateTextRect(const U2Region& yRange, bool selected) const;
    QRect calculateButtonRect(const QRect& itemRect) const;

    QObject*            labels; // used in GUI tests
    MaEditorWgt*        ui;
    QScrollBar*         nhBar;
    int                 curRowNumber;
    int                 startSelectingRowNumber;
    QPoint              selectionStartPoint;
    bool                scribbling;
    bool                shifting;
    bool                singleSelecting;
    GroupColorSchema    groupColors;

    QRubberBand*        rubberBand;
    QAction*            editSequenceNameAction;
    QAction*            copyCurrentSequenceAction;
    QAction*            removeSequenceAction;
    QPixmap*            cachedView;

    static const int CROSS_SIZE = 9;
    static const int CHILDREN_OFFSET = 8;
protected:
    MaEditor*          editor;

};

class McaEditorNameList : public MaEditorNameList {
    Q_OBJECT
public:
    McaEditorNameList(McaEditorWgt* ui, QScrollBar* nhBar);

protected slots:
    void sl_selectionChanged(const MaEditorSelection& current, const MaEditorSelection& prev);

signals:
    void si_selectionChanged();

protected:
    void drawSequenceItem(QPainter &painter, int rowIndex, const U2Region &yRange, const QString &text, bool selected);
    U2Region getSelection() const;

    void setSelection(int startSeq, int count);
    bool isRowInSelection(int row) const;

private:
    McaEditor* getEditor() const;

    U2Region localSelection;
};

class MsaEditorNameList : public MaEditorNameList {
public:
    MsaEditorNameList(MaEditorWgt* ui, QScrollBar* nhBar);

private:
    MSAEditor* getEditor() const;
};

}//namespace
#endif
