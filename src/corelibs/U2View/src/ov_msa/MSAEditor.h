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
#include "MaEditor.h"

namespace U2 {

class MultipleSequenceAlignmentObject;
class PhyTreeObject;
class MSAEditorUI;
class MSAEditorSequenceArea;
class MSAEditorConsensusArea;
class MaEditorNameList;
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

class U2VIEW_EXPORT MSAEditor : public MaEditor {
    Q_OBJECT
    Q_DISABLE_COPY(MSAEditor)

    friend class OpenSavedMSAEditorTask;
    friend class MSAEditorTreeViewerUI;
    friend class SequenceAreaRenderer;
    friend class SequenceWithChromatogramAreaRenderer;

public:
    MSAEditor(const QString& viewName, MultipleSequenceAlignmentObject* obj);
    ~MSAEditor();

    MultipleSequenceAlignmentObject* getMaObject() const { return qobject_cast<MultipleSequenceAlignmentObject*>(maObject); }

    virtual void buildStaticToolbar(QToolBar* tb);

    virtual void buildStaticMenu(QMenu* m);

    virtual Task* updateViewTask(const QString& stateName, const QVariantMap& stateData);

    virtual QVariantMap saveState();

    MSAEditorUI* getUI() const { return qobject_cast<MSAEditorUI*>(ui); }

    int getFirstVisibleBase() const;

    //Return alignment row that is displayed on target line in MSAEditor
    const MultipleSequenceAlignmentRow getRowByLineNumber(int lineNumber) const;

    void copyRowFromSequence(U2SequenceObject *seqObj, U2OpStatus &os);

    PairwiseAlignmentWidgetsSettings* getPairwiseAlignmentWidgetsSettings() const { return pairwiseAlignmentWidgetsSettings; }

    MSAEditorTreeManager* getTreeManager() {return &treeManager;}

    void buildTree();

    QString getReferenceRowName() const;

    char getReferenceCharAt(int pos) const;

protected slots:
    void sl_onContextMenuRequested(const QPoint & pos);

    void sl_buildTree();
    void sl_align();
    void sl_addToAlignment();
    void sl_setSeqAsReference();
    void sl_unsetReferenceSeq();

    void sl_onSeqOrderChanged(const QStringList& order);
    void sl_showTreeOP();
    void sl_hideTreeOP();
    void sl_rowsRemoved(const QList<qint64> &rowIds);

protected:
    QWidget* createWidget();
    bool eventFilter(QObject* o, QEvent* e);
    virtual bool onObjectRemoved(GObject* obj);
    virtual void onObjectRenamed(GObject* obj, const QString& oldName);
    virtual bool onCloseEvent();

private:
    void addExportMenu(QMenu* m);
    void addTreeMenu(QMenu* m);
    void addAdvancedMenu(QMenu* m);
    void addStatisticsMenu(QMenu* m);

    virtual void updateActions();

    void initDragAndDropSupport();
    void alignSequencesFromObjectsToAlignment(const QList<GObject*>& objects);
    void alignSequencesFromFilesToAlignment();

    QAction*          buildTreeAction;
    QAction*          alignAction;
    QAction*          alignSequencesToAlignmentAction;
    QAction*          setAsReferenceSequenceAction;
    QAction*          unsetReferenceSequenceAction;

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

    MSAEditor* getEditor() const { return qobject_cast<MSAEditor* >(editor); }

    MSAEditorSequenceArea* getSequenceArea() const;

    void createDistanceColumn(MSADistanceMatrix* matrix);

    void addTreeView(GObjectViewWindow* treeView);

    void setSimilaritySettings(const SimilarityStatisticsSettings* settings);

    void refreshSimilarityColumn();

    void showSimilarity();
    void hideSimilarity();

    const MsaEditorAlignmentDependentWidget* getSimilarityWidget(){return similarityStatistics;}

    MSAEditorTreeViewer* getCurrentTree() const;

    MSAEditorMultiTreeViewer* getMultiTreeViewer(){return multiTreeViewer;}

private slots:
    void sl_onTabsCountChanged(int tabsCount);
signals:
    void si_showTreeOP();
    void si_hideTreeOP();

protected:
    void initSeqArea(GScrollBar* shBar, GScrollBar* cvBar);
    void initOverviewArea();
    void initNameList(QScrollBar *nhBar);

private:
    MsaEditorSimilarityColumn*         dataList;
    MSAEditorMultiTreeViewer*          multiTreeViewer;
    MsaEditorAlignmentDependentWidget* similarityStatistics;
    MSAEditorTreeViewer*               treeViewer;
};

}//namespace;

#endif
