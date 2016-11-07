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

#ifndef _U2_MSA_EDITOR_H_
#define _U2_MSA_EDITOR_H_

#include <U2Algorithm/CreatePhyTreeSettings.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/PhyTree.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2Region.h>

#include <U2Gui/ObjectViewModel.h>

#include <U2View/UndoRedoFramework.h>

#include <QMenu>
#include <QSplitter>
#include <QTabWidget>
#include <QVariantMap>

#include "PhyTrees/MSAEditorTreeManager.h"

#include "view_rendering/MaEditorWgt.h"


namespace U2 {

class MultipleSequenceAlignmentObject;
class PhyTreeObject;
class MSAEditorUI;
class MSAEditorSequenceArea;
class MSAEditorConsensusArea;
class MSAEditorNameList;
class MSAEditorOffsetsViewController;
class MSAEditorOverviewArea;
class MSAEditorStatusWidget;
class PhyTreeGeneratorLauncherTask;
class MSAEditorTreeViewer;
class MSACollapsibleItemModel;
class MsaEditorSimilarityColumn;
class MSADistanceMatrix;
class MSASNPHighligtingScheme;
class SimilarityStatisticsSettings;
class MsaEditorAlignmentDependentWidget;
class TreeViewer;
class MSAEditorMultiTreeViewer;
class PairwiseAlignmentTask;


#define MSAE_MENU_COPY          "MSAE_MENU_COPY"
#define MSAE_MENU_EDIT          "MSAE_MENU_EDIT"
#define MSAE_MENU_EXPORT        "MSAE_MENU_EXPORT"
#define MSAE_MENU_VIEW          "MSAE_MENU_VIEW"
#define MSAE_MENU_ALIGN         "MSAE_MENU_ALIGN"
#define MSAE_MENU_TREES         "MSAE_MENU_TREES"
#define MSAE_MENU_STATISTICS    "MSAE_MENU_STATISTICS"
#define MSAE_MENU_ADVANCED      "MSAE_MENU_ADVANCED"
#define MSAE_MENU_LOAD          "MSAE_MENU_LOAD_SEQ"

class SNPSettings {
public:
    SNPSettings() : seqId(U2MsaRow::INVALID_ROW_ID) { }
    QPoint clickPoint;
    qint64 seqId;
    QVariantMap highlightSchemeSettings;
};

class PairwiseAlignmentWidgetsSettings {
public:
    PairwiseAlignmentWidgetsSettings()
        : firstSequenceId(U2MsaRow::INVALID_ROW_ID),
        secondSequenceId(U2MsaRow::INVALID_ROW_ID), inNewWindow(true),
        pairwiseAlignmentTask(NULL), showSequenceWidget(true), showAlgorithmWidget(false),
        showOutputWidget(false), sequenceSelectionModeOn(false)
    {

    }

    qint64 firstSequenceId;
    qint64 secondSequenceId;
    QString algorithmName;
    bool inNewWindow;
    QString resultFileName;
    PairwiseAlignmentTask* pairwiseAlignmentTask;
    bool showSequenceWidget;
    bool showAlgorithmWidget;
    bool showOutputWidget;
    bool sequenceSelectionModeOn;

    QVariantMap customSettings;
};

class U2VIEW_EXPORT MSAEditor : public GObjectView {
    Q_OBJECT
    Q_DISABLE_COPY(MSAEditor)

    friend class OpenSavedMSAEditorTask;
    friend class MSAEditorTreeViewerUI;
    friend class SequenceAreaRenderer;
    friend class SequenceWithChromatogramAreaRenderer;

public:
    MSAEditor(const QString& viewName, GObject* obj);
    ~MSAEditor();

    virtual void buildStaticToolbar(QToolBar* tb);

    virtual void buildStaticMenu(QMenu* m);

    virtual Task* updateViewTask(const QString& stateName, const QVariantMap& stateData);

    virtual QVariantMap saveState();

    virtual OptionsPanel* getOptionsPanel(){return optionsPanel;}

    MultipleSequenceAlignmentObject* getMSAObject() const {return msaObject;}

    MSAEditorUI* getUI() const {return ui;}

    int getAlignmentLen() const;

    int getNumSequences() const;

    bool isAlignmentEmpty() const;

    const QRect& getCurrentSelection() const;

    const QFont& getFont() const {return font;}
    int getFirstVisibleBase() const;

    //Return alignment row that is displayed on target line in MSAEditor
    const MultipleSequenceAlignmentRow getRowByLineNumber(int lineNumber) const;

    float getZoomFactor() const {return zoomFactor;}

    enum ResizeMode {
        ResizeMode_FontAndContent, ResizeMode_OnlyContent
    };

    ResizeMode getResizeMode() const { return resizeMode; }

    int getRowHeight() const;
    int getSequenceRowHeight() const;

    int getColumnWidth() const;

    void copyRowFromSequence(U2SequenceObject *seqObj, U2OpStatus &os);
    void createDistanceColumn(MSADistanceMatrix* algo);

    static const float zoomMult;

    void setReference(qint64 sequenceId);
    qint64 getReferenceRowId() const { return snp.seqId; }
    QVariantMap getHighlightingSettings(const QString &highlightingFactoryId) const;
    void saveHighlightingSettings(const QString &highlightingFactoryId, const QVariantMap &settingsMap = QVariantMap());
    QString getReferenceRowName() const;
    void updateReference();

    PairwiseAlignmentWidgetsSettings* getPairwiseAlignmentWidgetsSettings() const { return pairwiseAlignmentWidgetsSettings; }

    MSAEditorTreeManager* getTreeManager() {return &treeManager;}

    void buildTree();

    void resetCollapsibleModel();

    void exportHighlighted(){sl_exportHighlighted();}

public slots:
    void sl_zoomIn();
    void sl_zoomOut();
    void sl_resetZoom();

signals:
    void si_fontChanged(const QFont& f);
    void si_zoomOperationPerformed(bool resizeModeChanged);
    void si_referenceSeqChanged(qint64 referenceId);
    void si_sizeChanged(int newHeight, bool isMinimumSize, bool isMaximumSize);
    void si_completeUpdate();

protected slots:
    void sl_saveAlignment();
    void sl_saveAlignmentAs();
    void sl_onContextMenuRequested(const QPoint & pos);
    void sl_zoomToSelection();
    void sl_changeFont();
    void sl_buildTree();
    void sl_align();
    void sl_addToAlignment();
    void sl_setSeqAsReference();
    void sl_unsetReferenceSeq();
    void sl_exportHighlighted();
    void sl_lockedStateChanged();
    void sl_showHideChromatograms(bool show);

    void sl_onSeqOrderChanged(const QStringList& order);
    void sl_showTreeOP();
    void sl_hideTreeOP();
    void sl_rowsRemoved(const QList<qint64> &rowIds);

protected:
    virtual QWidget* createWidget();
    bool eventFilter(QObject* o, QEvent* e);
    virtual bool onObjectRemoved(GObject* obj);
    virtual void onObjectRenamed(GObject* obj, const QString& oldName);
    virtual bool onCloseEvent();

private:
    void addCopyMenu(QMenu* m);
    void addEditMenu(QMenu* m);
    void addExportMenu(QMenu* m);
    void addViewMenu(QMenu* m);
    void addAlignMenu(QMenu* m);
    void addTreeMenu(QMenu* m);
    void addAdvancedMenu(QMenu* m);
    void addStatisticsMenu(QMenu* m);
    void addLoadMenu(QMenu* m);
    void setFont(const QFont& f);
    void calcFontPixelToPointSizeCoef();
    void updateActions();
    void setFirstVisibleBase(int firstPos);
    void setZoomFactor(float newZoomFactor) {zoomFactor = newZoomFactor;}
    void initDragAndDropSupport();
    void alignSequencesFromObjectsToAlignment(const QList<GObject*>& objects);
    void alignSequencesFromFilesToAlignment();

    MultipleSequenceAlignmentObject* msaObject;
    MSAEditorUI*      ui;
    QFont             font;
    ResizeMode        resizeMode;
    float             zoomFactor;
    float             fontPixelToPointSize;
    bool              showChromatograms;

    QAction*          saveAlignmentAction;
    QAction*          saveAlignmentAsAction;
    QAction*          zoomInAction;
    QAction*          zoomOutAction;
    QAction*          zoomToSelectionAction;
    QAction*          showOverviewAction;
    QAction*          showChromatogramsAction;
    QAction*          changeFontAction;
    QAction*          resetFontAction;
    QAction*          buildTreeAction;
    QAction*          saveScreenshotAction;
    QAction*          alignAction;
    QAction*          alignSequencesToAlignmentAction;
    QAction*          setAsReferenceSequenceAction;
    QAction*          unsetReferenceSequenceAction;
    QAction*          exportHighlightedAction;

    QToolBar*         toolbar;

    SNPSettings snp;
    PairwiseAlignmentWidgetsSettings* pairwiseAlignmentWidgetsSettings;
    MSAEditorTreeManager           treeManager;
};

// U2VIEW_EXPORT: GUITesting uses MSAEditorUI
class U2VIEW_EXPORT MSAEditorUI : public MaEditorWgt {
    Q_OBJECT
    //todo: make public accessors:
    friend class MSAEditorTreeViewer;
    friend class MsaEditorSimilarityColumn;

public:
    MSAEditorUI(MSAEditor* editor);

    void createDistanceColumn(MSADistanceMatrix* matrix);

    void addTreeView(GObjectViewWindow* treeView);

    void setSimilaritySettings(const SimilarityStatisticsSettings* settings);

    void refreshSimilarityColumn();

    void showSimilarity();
    void hideSimilarity();

    const MsaEditorAlignmentDependentWidget* getSimilarityWidget(){return similarityStatistics;}

    MSAEditorTreeViewer* getCurrentTree() const;

    MSAEditorMultiTreeViewer* getMultiTreeViewer(){return multiTreeViewer;}

public slots:
    void sl_saveScreenshot();
private slots:
    void sl_onTabsCountChanged(int tabsCount);
signals:
    void si_showTreeOP();
    void si_hideTreeOP();

    void si_startMsaChanging();
    void si_stopMsaChanging(bool modifyed = false);

private:
    MsaEditorSimilarityColumn*         dataList;
    MSAEditorMultiTreeViewer*          multiTreeViewer;
    MsaEditorAlignmentDependentWidget* similarityStatistics;
    MSAEditorTreeViewer*               treeViewer;
};

}//namespace;

#endif
