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

#ifndef _U2_MA_EDITOR_SEQUENCE_AREA_
#define _U2_MA_EDITOR_SEQUENCE_AREA_

#include <QColor>
#include <QTimer>
#include <QWidget>

#include <U2Core/U2Region.h>

#include <U2Gui/GScrollBar.h>

#include "../MaEditor.h"

class QRubberBand;

namespace U2 {

class GScrollBar;
class MaEditor;
class MaEditorWgt;
class SequenceAreaRenderer;

class MsaColorScheme;
class MsaHighlightingScheme;

//SANGER_TODO: no need to export
class MaEditorSelection {
public:
    MaEditorSelection() { }
    MaEditorSelection(int left, int top, int width, int height) : selArea(left,top,width,height) { }
    MaEditorSelection(const QPoint& topLeft, const QPoint& bottomRight) : selArea(topLeft, bottomRight) { }
    MaEditorSelection(const QPoint& topLeft, int width, int height) : selArea(topLeft, QSize(width,height)) { }

    // consider that selection may consist of several unconnected areas
    bool isContiniuous() const { return true; }

    bool isNull() const {return selArea.isNull(); }

    QPoint topLeft() const { return selArea.topLeft(); }

    const QRect& getRect() const {return selArea; }

    int x() const { return selArea.x(); }

    int y() const { return selArea.y(); }

    int width() const { return selArea.width(); }

    int height() const { return selArea.height(); }

    bool operator==(const MaEditorSelection& other) const {
        return selArea == other.selArea;
    }

    MaEditorSelection intersected(const MaEditorSelection& selection) const {
        QRect r = selArea.intersected(selection.selArea);
        return MaEditorSelection(r);
    }

private:
    explicit MaEditorSelection(QRect& rect) : selArea(rect) { }
    QRect selArea;
};

class MaEditorSequenceArea : public QWidget {
    Q_OBJECT
public:
    MaEditorSequenceArea(MaEditorWgt* ui, GScrollBar* hb, GScrollBar* vb);
    virtual ~MaEditorSequenceArea();

    virtual MaEditor* getEditor() const { return editor; }

    // x dimension -> positions
    int countWidthForBases(bool countClipped, bool forOffset = false) const;

    int getFirstVisibleBase() const;

    int getLastVisibleBase(bool countClipped, bool forOffset = false) const;

    int getNumVisibleBases(bool countClipped, bool forOffset = false) const;

    U2Region getBaseXRange(int pos, bool useVirtualCoords) const;
    U2Region getBaseXRange(int pos, int firstVisiblePos, bool useVirtualCoords) const;

    int getColumnNumByX(int x, bool selecting = false) const;
    int getXByColumnNum(int columnNum) const;

    void setFirstVisibleBase(int pos);

    // y dimension -> sequences
    int countHeightForSequences(bool countClipped) const;

    int getFirstVisibleSequence() const;

    int getLastVisibleSequence(bool countClipped) const;
    /*
     * Returns count of sequences that are visible on a screen.
     * @countClipped specifies whether include to result count or not last partially displayed row.
     */
    int getNumVisibleSequences(bool countClipped) const;
    /*
     * Returns count of sequences that are drawn on the widget by taking into account
     * collapsed rows.
     */
    int getNumDisplayedSequences() const;

    U2Region getSequenceYRange(int seqNum, bool useVirtualCoords) const;
    U2Region getSequenceYRange(int seqNum, int firstVisibleRow, bool useVirtualCoords) const;

    int getSequenceNumByY(int y) const;
    int getYBySequenceNum(int sequenceNum) const;

    void setFirstVisibleSequence(int seq);

    // SANGER_TODO: move to cpp
    bool isAlignmentEmpty() const { return editor->isAlignmentEmpty(); }

    bool isPosInRange(int p) const;

    bool isSeqInRange(int s) const;

    bool isInRange(const QPoint& p) const;

    bool isVisible(const QPoint& p, bool countClipped) const;

    bool isPosVisible(int pos, bool countClipped) const;

    bool isSeqVisible(int seq, bool countClipped) const;

    int coordToPos(int x) const;

    // returns valid position only for visible area
    QPoint coordToPos(const QPoint& coord) const;

    // returns valid position if coords are out of visible area
    QPoint coordToAbsolutePos(const QPoint& coord) const;
    QPoint coordToAbsolutePosOutOfRange(const QPoint& coord) const;

    const MaEditorSelection& getSelection() const;

    void updateSelection(const QPoint& newMousePos);

    // update selection when collapsible model changed
    void updateSelection();

    void setSelection(const MaEditorSelection& sel, bool newHighlightSelection = false);

    void moveSelection(int dx, int dy, bool allowSelectionResize = false);

    void cancelSelection();

    U2Region getSelectedRows() const;

    int getHeight();

public:
    GScrollBar* getVBar() const {return svBar;}

    GScrollBar* getHBar() const {return shBar;}

    void onVisibleRangeChanged();

    bool isAlignmentLocked();

signals:
    void si_startChanged(const QPoint& p, const QPoint& prev);
    void si_selectionChanged(const MaEditorSelection& current, const MaEditorSelection& prev);
    void si_selectionChanged(const QStringList& selectedRows);
    void si_highlightingChanged();
    void si_visibleRangeChanged(QStringList visibleSequences, int reqHeight);
    void si_visibleRangeChanged();
    void si_startMsaChanging();
    void si_stopMsaChanging(bool msaUpdated);
    void si_copyFormattedChanging(bool enabled);

protected:
    virtual void initRenderer() {}
    virtual void updateActions() = 0;

    void updateHScrollBar();
    void updateVScrollBar();

    bool checkState() const;

    void exitFromEditCharacterMode();
    void deleteOldCustomSchemes();

protected:
    enum MaMode {
        ViewMode,
        EditCharacterMode
    };

    MaEditor*       editor;
    MaEditorWgt*    ui;

    MsaColorScheme*         colorScheme;
    MsaHighlightingScheme*  highlightingScheme;
    bool                    highlightSelection;

    GScrollBar*     shBar;
    GScrollBar*     svBar;
    QRubberBand*    rubberBand;

    SequenceAreaRenderer*   renderer;

    QPixmap*        cachedView;
    bool            completeRedraw;

    int             startPos; //first visible x pos
    int             startSeq; //first visible y pos
    MaMode          msaMode;
    QTimer          editModeAnimationTimer;
    QColor          selectionColor;

    bool                shifting;
    bool                selecting;
    Qt::MouseButton     prevPressedButton;
    QPoint              origin; // global window coordinates
    QPoint              cursorPos; // mouse cursor position in alignment coordinates
    MaEditorSelection   selection; // selection with rows indexes in collapsible model coordinates
    MaEditorSelection   baseSelection; // selection with rows indexes in absolute coordinates
    QList<int>          selectedRows;
    QStringList         selectedRowNames;
    int                 msaVersionBeforeShifting;

    QList<QAction*>     colorSchemeMenuActions;
    QList<QAction* >    customColorSchemeMenuActions;
    QList<QAction* >    highlightingSchemeMenuActions;
};

} // namespace

#endif // _U2_MA_EDITOR_SEQUENCE_AREA_

