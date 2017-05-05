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

#ifndef _U2_MA_EDITOR_SEQUENCE_AREA_
#define _U2_MA_EDITOR_SEQUENCE_AREA_

#include <QColor>
#include <QTimer>
#include <QPainter>
#include <QWidget>

#include <U2Core/U2Region.h>
#include <U2Core/MultipleAlignment.h>

#include <U2Gui/GScrollBar.h>

#include "MaEditorSelection.h"
#include "../MaEditor.h"
#include "../MsaEditorUserModStepController.h"

class QRubberBand;

namespace U2 {

#define SETTINGS_ROOT QString("msaeditor/")
#define SETTINGS_COLOR_NUCL     "color_nucl"
#define SETTINGS_COLOR_AMINO    "color_amino"
#define SETTINGS_COLOR_RAW      "color_raw"
#define SETTINGS_HIGHLIGHT_NUCL     "highlight_nucl"
#define SETTINGS_HIGHLIGHT_AMINO    "highlight_amino"
#define SETTINGS_HIGHLIGHT_RAW      "highlight_raw"
#define SETTINGS_COPY_FORMATTED "copyformatted"

class GScrollBar;
class MaEditor;
class MaEditorWgt;
class SequenceAreaRenderer;

class MaModificationInfo;
class MsaColorScheme;
class MsaColorSchemeFactory;
class MsaHighlightingScheme;
class MsaHighlightingSchemeFactory;

class U2VIEW_EXPORT MaEditorSequenceArea : public QWidget {
    Q_OBJECT
    friend class SequenceAreaRenderer;
public:
    MaEditorSequenceArea(MaEditorWgt* ui, GScrollBar* hb, GScrollBar* vb);
    virtual ~MaEditorSequenceArea();

    MaEditor* getEditor() const { return editor; }

public:
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

public:
    // y dimension -> sequences

    // SANGER_TODO: start point matters!
    virtual int countHeightForSequences(bool countClipped) const;

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
    virtual U2Region getSequenceYRange(int seqNum, int firstVisibleRow, bool useVirtualCoords) const;
    virtual U2Region getSequenceYRange(int startSeq, int count) const;

    virtual int getSequenceNumByY(int y) const;
    int getYBySequenceNum(int sequenceNum) const; // SANGER_TODO: works wrong for MCA!

    void setFirstVisibleSequence(int seq);

public:
    U2Region getRowsAt(int seq) const;

    QPair<QString, int> getGappedColumnInfo() const;

    bool isAlignmentEmpty() const;

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

    virtual void setSelection(const MaEditorSelection& sel, bool newHighlightSelection = false);

    virtual void moveSelection(int dx, int dy, bool allowSelectionResize = false);

    virtual void adjustReferenceLength(U2OpStatus& os) {}

    void cancelSelection();

    U2Region getSelectedRows() const;

    int getHeight();

    QString getCopyFormatedAlgorithmId() const;
    void setCopyFormatedAlgorithmId(const QString& algoId);

    virtual void deleteCurrentSelection();

    /**
     * Shifts currently selected region to @shift.
     * If @shift > 0, the region is moved to the right and "true" is returned.
     * If @shift <= 0, the region is moved to the left only for the available number
     * of columns (i.e. the columns with gaps). The returned value specifies
     * whether the region was actually moved in this case.
     */
    bool shiftSelectedRegion(int shift);

public:
    void centerPos(const QPoint& pos);
    void centerPos(int pos);

    void setFont(const QFont& f);

    GScrollBar* getVBar() const {return svBar;}
    GScrollBar* getHBar() const {return shBar;}

    void updateHBarPosition(int base, bool repeatAction = false);
    void updateVBarPosition(int seq, bool repeatAction = false);

    void onVisibleRangeChanged();

    bool isAlignmentLocked();

    void drawVisibleContent(QPainter& p);

    bool drawContent(QPainter &p, const QRect &area);
    bool drawContent(QPainter &p, const U2Region& region, const QList<qint64> &seqIdx);

    bool drawContent(QPainter& p);
    bool drawContent(QPixmap& pixmap);
    bool drawContent(QPixmap& pixmap, const U2Region& region, const QList<qint64>& seqIdx);

    void highlightCurrentSelection();

    QString exportHighlighting(int startPos, int endPos, int startingIndex, bool keepGaps, bool dots, bool transpose);

    MsaColorScheme * getCurrentColorScheme() const;
    MsaHighlightingScheme * getCurrentHighlightingScheme() const;
    bool getUseDotsCheckedState() const;


public slots:
    void sl_changeColorSchemeOutside(const QString &id);
    void sl_changeCopyFormat(const QString& alg);
    void sl_changeColorScheme();
    void sl_delCurrentSelection();
    void sl_fillCurrentSelectionWithGaps();

protected slots:
    virtual void sl_buildStaticMenu(GObjectView* v, QMenu* m);
    virtual void sl_buildStaticToolbar(GObjectView* v, QToolBar* t);
    virtual void sl_buildContextMenu(GObjectView* v, QMenu* m);

    void sl_alignmentChanged(const MultipleAlignment &ma, const MaModificationInfo &modInfo);

    void sl_onHScrollMoved(int pos);
    void sl_onVScrollMoved(int pos);

    void sl_completeUpdate();

    void sl_triggerUseDots();
    void sl_useDots();

    void sl_registerCustomColorSchemes();
    void sl_colorSchemeFactoryUpdated();
    void sl_setDefaultColorScheme();
    void sl_changeHighlightScheme();

    void sl_replaceSelectedCharacter();
    void sl_changeSelectionColor();

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
    void setCursorPos(const QPoint& p);
    void setCursorPos(int x, int y);
    void setCursorPos(int pos);

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void wheelEvent (QWheelEvent * event);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);

    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

protected:
    virtual void initRenderer() = 0;
    virtual void updateActions() = 0;
    virtual void drawBackground(QPainter& p) { Q_UNUSED(p); }

    /**
     * Inserts a region consisting of gaps only before the selection. The inserted region width
     * is specified by @countOfGaps parameter if 0 < @countOfGaps, its height is equal to the
     * current selection's height.
     *
     * If there is no selection in MSA then the method does nothing.
     *
     * If -1 == @countOfGaps then the inserting region width is equal to
     * the selection's width. If 1 > @countOfGaps and -1 != @countOfGaps then nothing happens.
     */
    void insertGapsBeforeSelection( int countOfGaps = -1 );

    /**
     * Reverse operation for @insertGapsBeforeSelection( ),
     * removes the region preceding the selection if it consists of gaps only.
     *
     * If there is no selection in MSA then the method does nothing.
     *
     * @countOfGaps specifies maximum width of the removed region.
     * If -1 == @countOfGaps then count of removed gap columns is equal to
     * the selection width. If 1 > @countOfGaps and -1 != @countOfGaps then nothing happens.
     */
    void removeGapsPrecedingSelection( int countOfGaps = -1 );

    /*
     * Interrupts the tracking of MSA modifications caused by a region shifting,
     * also stops shifting. The method is used to keep consistence of undo/redo stack.
     */
    void cancelShiftTracking( );

    void drawAll();
    void validateRanges();          //called on resize/refont like events

    virtual void buildMenu(QMenu* m);
    void updateColorAndHighlightSchemes();

    void initColorSchemes(MsaColorSchemeFactory* defaultColorSchemeFactory);

    void registerCommonColorSchemes();

    void initHighlightSchemes(MsaHighlightingSchemeFactory* hsf, DNAAlphabetType atype);

    MsaColorSchemeFactory * getDefaultColorSchemeFactory();
    virtual void getColorAndHighlightingIds(QString &csid, QString &hsid, DNAAlphabetType atype);
    void applyColorScheme(const QString &id);

    void updateHScrollBar();
    void updateVScrollBar();

    bool checkState() const;

    void processCharacterInEditMode(QKeyEvent *e);
    void processCharacterInEditMode(char newCharacter);
    void replaceChar(char newCharacter);
    virtual void insertChar(char ) {}
    void exitFromEditCharacterMode();

    void deleteOldCustomSchemes();

    virtual void updateCollapsedGroups(const MaModificationInfo&) {}

protected:
    enum MaMode {
        ViewMode,
        ReplaceCharMode,
        InsertCharMode
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

    bool                editingEnabled;
    bool                shifting;
    bool                selecting;
    Qt::MouseButton     prevPressedButton;
    QPoint              origin; // global window coordinates
    QPoint              cursorPos; // mouse cursor position in alignment coordinates
    MaEditorSelection   selection; // selection with rows indexes in collapsible model coordinates
    MaEditorSelection   baseSelection; // selection with rows indexes in absolute coordinates

    int                 msaVersionBeforeShifting;

    QAction*            useDotsAction;

    QList<QAction*>     colorSchemeMenuActions;
    QList<QAction* >    customColorSchemeMenuActions;
    QList<QAction* >    highlightingSchemeMenuActions;

    QAction*            replaceCharacterAction;
    QAction*            fillWithGapsinsSymAction;

    // The member is intended for tracking MSA changes (handling U2UseCommonUserModStep objects)
    // that does not fit into one method, e.g. shifting MSA region with mouse.
    // If the changing action fits within one method it's recommended using
    // the U2UseCommonUserModStep object explicitly.
    MsaEditorUserModStepController changeTracker;
};

} // namespace

#endif // _U2_MA_EDITOR_SEQUENCE_AREA_

