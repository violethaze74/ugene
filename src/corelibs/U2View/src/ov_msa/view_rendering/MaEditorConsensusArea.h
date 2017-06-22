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

#ifndef _U2_MA_EDITOR_CONSENSUS_AREA_H_
#define _U2_MA_EDITOR_CONSENSUS_AREA_H_

#include <QWidget>

#include <U2Core/MultipleSequenceAlignment.h>
#include <U2Core/U2Region.h>

#include "../MSAEditorConsensusCache.h"

class QHelpEvent;
class QMenu;
class QPainter;
class QToolBar;


namespace U2 {

class GObjectView;
class MSAConsensusAlgorithm;
class MSAConsensusAlgorithmFactory;

class MaEditor;
class MaEditorSelection;
class MaEditorWgt;
class MaModificationInfo;

enum MaEditorConsElement {
    MSAEditorConsElement_HISTOGRAM,
    MSAEditorConsElement_CONSENSUS_TEXT,
    MSAEditorConsElement_RULER
};

class MaEditorConsensusAreaSettings {
public:
    MaEditorConsensusAreaSettings();

    bool isVisible(const MaEditorConsElement element) const;

    QList<MaEditorConsElement>      order;
    QMap<MaEditorConsElement, bool> visibility;
    bool                            highlightMismatches; // valid only for mca
};

class MaEditorConsensusArea : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(MaEditorConsensusArea)
public:
    MaEditorConsensusArea(MaEditorWgt* ui);
    virtual ~MaEditorConsensusArea();

    U2Region getRullerLineYRange() const;

    void setConsensusAlgorithm(MSAConsensusAlgorithmFactory* algo);
    void setConsensusAlgorithmConsensusThreshold(int val);

    MSAConsensusAlgorithm* getConsensusAlgorithm() const;

    QSharedPointer<MSAEditorConsensusCache> getConsensusCache();

    void paintFullConsensus(QPixmap &pixmap);
    void paintFullConsensus(QPainter& p);

    void paintConsenusPart(QPixmap & pixmap, const U2Region &region, const QList<qint64> &seqIdx);
    void paintConsenusPart(QPainter& p, const U2Region &region, const QList<qint64> &seqIdx);

    void paintRulerPart(QPixmap &pixmap, const U2Region &region);
    void paintRulerPart(QPainter &p, const U2Region &region);

protected:
    bool event(QEvent* e);
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

signals:
    void si_consensusAlgorithmChanged(const QString& algoId);
    void si_consensusThresholdChanged(int value);
    // SANGER_TODO: should be moved to McaEditorConsensusArea
    void si_mismatchRedrawRequired();

protected slots:
    void sl_startChanged(const QPoint&, const QPoint&);
    void sl_selectionChanged(const MaEditorSelection& current, const MaEditorSelection& prev);
    void sl_alignmentChanged();
    void sl_changeConsensusAlgorithm(const QString& algoId);
    void sl_changeConsensusThreshold(int val);
    void sl_onScrollBarActionTriggered( int scrollAction );
    void sl_onConsensusThresholdChanged(int newValue);

    void sl_buildStaticMenu(GObjectView* v, QMenu* m);
    virtual void sl_buildStaticToolbar(GObjectView* , QToolBar* ) {}
    void sl_buildContextMenu(GObjectView* v, QMenu* m);
    void sl_copyConsensusSequence();
    void sl_copyConsensusSequenceWithGaps();
    void sl_configureConsensusAction();
    void sl_zoomOperationPerformed(bool resizeModeChanged);

    void setupFontAndHeight();

public:
    void drawContent(QPainter& painter);

protected:
    QString createToolTip(QHelpEvent* he) const;
    void restoreLastUsedConsensusThreshold();
    QString getLastUsedAlgoSettingsKey() const;
    QString getThresholdSettingsKey(const QString& factoryId) const;

    virtual void buildMenu(QMenu* m);

    void updateSelection(int newPos);

    void drawConsensus(QPainter& p);
    void drawConsensus(QPainter& p, int startPos, int lastPos, bool useVirtualCoords = false);

    void drawConsensusChar(QPainter& p, int pos, int firstVisiblePos, bool selected, bool useVirtualCoords = false);
    void drawConsensusChar(QPainter& p, int pos, int firstVisiblePos, char consChar,
                           bool selected, bool useVirtualCoords = false);
    virtual bool highlightConsensusChar(int pos);

    void drawRuler(QPainter& p, int start = -1, int end = -1, bool drawFull = false);

    void drawHistogram(QPainter& p);
    void drawHistogram(QPainter& p, int firstBase, int lastBase);

    void drawSelection(QPainter& p);


    U2Region getYRange(MaEditorConsElement e) const;
    int getYRangeLength(MaEditorConsElement e) const;

    MSAConsensusAlgorithmFactory* getConsensusAlgorithmFactory();
    void updateConsensusAlgorithm();

    MaEditor*           editor;
    MaEditorWgt*        ui;
    QFont               rulerFont;
    int                 rulerFontHeight;
    QAction*            copyConsensusAction;
    QAction*            copyConsensusWithGapsAction;
    QAction*            configureConsensusAction;
    int                 curPos;
    bool                scribbling, selecting;

    QSharedPointer<MSAEditorConsensusCache> consensusCache;

    bool                                    completeRedraw;
    mutable MaEditorConsensusAreaSettings   drawSettings;
    QPixmap*                                cachedView;
};

} // namespace

#endif // _U2_MA_EDITOR_CONSENSUS_AREA_H_
