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

#include "MaEditorSelection.h"
#include "../MaEditor.h"

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

class MsaColorScheme;
class MsaColorSchemeFactory;
class MsaHighlightingScheme;
class MsaHighlightingSchemeFactory;

class MaEditorSequenceArea : public QWidget {
    Q_OBJECT
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

    void setSelection(const MaEditorSelection& sel, bool newHighlightSelection = false);

    void moveSelection(int dx, int dy, bool allowSelectionResize = false);

    void cancelSelection();

    U2Region getSelectedRows() const;

    int getHeight();

    QString getCopyFormatedAlgorithmId() const;
    void setCopyFormatedAlgorithmId(const QString& algoId);

    virtual void deleteCurrentSelection() = 0;

public:
    void centerPos(const QPoint& pos);
    void centerPos(int pos);

    void setFont(const QFont& f);

    GScrollBar* getVBar() const {return svBar;}
    GScrollBar* getHBar() const {return shBar;}

    void updateHBarPosition(int base);
    void updateVBarPosition(int seq);

    void onVisibleRangeChanged();

    bool isAlignmentLocked();

    void drawVisibleContent(QPainter& p);

    bool drawContent(QPainter &p, const QRect &area);
    bool drawContent(QPainter &p, const U2Region& region, const QList<qint64> &seqIdx);

    bool drawContent(QPainter& p);
    bool drawContent(QPixmap& pixmap);
    bool drawContent(QPixmap& pixmap, const U2Region& region, const QList<qint64>& seqIdx);

    QString exportHighligtning(int startPos, int endPos, int startingIndex, bool keepGaps, bool dots, bool transpose);

    MsaColorScheme * getCurrentColorScheme() const;
    MsaHighlightingScheme * getCurrentHighlightingScheme() const;
    bool getUseDotsCheckedState() const;

public slots:
    void sl_changeColorSchemeOutside(const QString &name);
    void sl_changeCopyFormat(const QString& alg);
    void sl_changeColorScheme();
    void sl_delCurrentSelection();

protected slots:
    void sl_triggerUseDots();
    void sl_useDots();

    void sl_registerCustomColorSchemes();
    void sl_colorSchemeFactoryUpdated();
    void sl_setDefaultColorScheme();
    void sl_changeHighlightScheme();

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
    void mousePressEvent(QMouseEvent *);

protected:
    virtual void initRenderer() = 0;
    virtual void updateActions() = 0;

    void drawAll();
    void drawFocus(QPainter& p);
    void drawSelection(QPainter &p);

    void validateRanges();          //called on resize/refont like events

    virtual void buildMenu(QMenu* m);
    void updateColorAndHighlightSchemes();

    void initColorSchemes(MsaColorSchemeFactory* defaultColorSchemeFactory);
    void registerCommonColorSchemes();
    void initHighlightSchemes(MsaHighlightingSchemeFactory* hsf, DNAAlphabetType atype);

    MsaColorSchemeFactory * getDefaultColorSchemeFactory();
    void getColorAndHighlightingIds(QString &csid, QString &hsid, DNAAlphabetType atype, bool isFirstInitialization);
    void applyColorScheme(const QString &id);

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

    int                 msaVersionBeforeShifting;

    QAction*        useDotsAction;

    QList<QAction*>     colorSchemeMenuActions;
    QList<QAction* >    customColorSchemeMenuActions;
    QList<QAction* >    highlightingSchemeMenuActions;
};

} // namespace

#endif // _U2_MA_EDITOR_SEQUENCE_AREA_

