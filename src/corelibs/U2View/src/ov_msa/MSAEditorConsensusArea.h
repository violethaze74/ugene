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

#ifndef _U2_MSA_EDITOR_CONSENSUS_AREA_H_
#define _U2_MSA_EDITOR_CONSENSUS_AREA_H_

#include <QWidget>

#include <U2Core/MultipleSequenceAlignment.h>
#include <U2Core/U2Region.h>

#include "MaConsensusMismatchController.h"
#include "MaEditorConsensusAreaSettings.h"
#include "MSAEditorConsensusCache.h"

class QHelpEvent;
class QMenu;
class QPainter;
class QToolBar;

namespace U2 {

class GObjectView;
class MaConsensusAreaRenderer;
class MaEditor;
class MaEditorSelection;
class MaEditorWgt;
class MaModificationInfo;
class MSAConsensusAlgorithm;
class MSAConsensusAlgorithmFactory;

class U2VIEW_EXPORT MSAEditorConsensusArea : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(MSAEditorConsensusArea)

public:
    MSAEditorConsensusArea(MaEditorWgt* ui);
    ~MSAEditorConsensusArea();

    U2Region getRulerLineYRange() const;

    void setConsensusAlgorithm(MSAConsensusAlgorithmFactory* algo);
    void setConsensusAlgorithmConsensusThreshold(int val);

    const MaEditorConsensusAreaSettings &getDrawSettings() const;
    void setDrawSettings(const MaEditorConsensusAreaSettings& settings);

    MSAConsensusAlgorithm* getConsensusAlgorithm() const;

    QSharedPointer<MSAEditorConsensusCache> getConsensusCache();

    MaConsensusMismatchController* getMismatchController() { return mismatchController; }

    MaEditorWgt *getEditorWgt() const;

    QSize getCanvasSize(const U2Region &region, const MaEditorConsElements &elements) const;

    void drawContent(QPainter &painter);
    void drawContent(QPainter &painter,
                     const QList<int> &seqIdx,
                     const U2Region &region,
                     const MaEditorConsensusAreaSettings &consensusSettings);

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
    void si_mismatchRedrawRequired();

private slots:
    void sl_selectionChanged(const MaEditorSelection& current, const MaEditorSelection& prev);
    void sl_alignmentChanged();
    void sl_changeConsensusAlgorithm(const QString& algoId);
    void sl_changeConsensusThreshold(int val);
    void sl_visibleAreaChanged();
    void sl_onConsensusThresholdChanged(int newValue);

    void sl_buildStaticMenu(GObjectView* v, QMenu* m);
    void sl_buildStaticToolbar(GObjectView* v, QToolBar* t);
    void sl_buildContextMenu(GObjectView* v, QMenu* m);
    void sl_copyConsensusSequence();
    void sl_copyConsensusSequenceWithGaps();
    void sl_configureConsensusAction();
    void sl_zoomOperationPerformed(bool resizeModeChanged);
    void sl_completeRedraw();

    void setupFontAndHeight();

private:
    QString createToolTip(QHelpEvent* he) const;
    void restoreLastUsedConsensusThreshold();
    QString getLastUsedAlgoSettingsKey() const;
    QString getThresholdSettingsKey(const QString& factoryId) const;

    void buildMenu(QMenu* m);

    void updateSelection(int newPos);

    MSAConsensusAlgorithmFactory* getConsensusAlgorithmFactory();
    void updateConsensusAlgorithm();

    MaEditor*           editor;
    MaEditorWgt*        ui;

    MaEditorConsensusAreaSettings consensusSettings;
    MaConsensusAreaRenderer *renderer;

    QAction*            copyConsensusAction;
    QAction*            copyConsensusWithGapsAction;
    QAction*            configureConsensusAction;
    int                 curPos;
    bool                scribbling;
    bool                selecting;

    QSharedPointer<MSAEditorConsensusCache>         consensusCache;
    MaConsensusMismatchController*  mismatchController;

    bool                            completeRedraw;
    QPixmap*                        cachedView;
};

}//namespace
#endif

