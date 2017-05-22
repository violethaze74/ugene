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

#ifndef _U2_MA_EDITOR_WGT_H_
#define _U2_MA_EDITOR_WGT_H_

#include <QWidget>

#include "MaEditorUtils.h"

class QGridLayout;
class QScrollBar;
class QVBoxLayout;

namespace U2 {

class BaseWidthController;
class DrawHelper;
class GScrollBar;
class MSACollapsibleItemModel;
class MSAEditorConsensusArea;
class MSAEditorOffsetsViewController;
class MSAEditorStatusWidget;
class MaEditor;
class MaEditorNameList;
class MaEditorOverviewArea;
class MaEditorSequenceArea;
class RowHeightController;
class MsaUndoRedoFramework;
class ScrollController;
class SequenceAreaRenderer;

/************************************************************************/
/* MaEditorWgt */
/************************************************************************/
class MaEditorWgt : public QWidget {
    Q_OBJECT
public:
    MaEditorWgt(MaEditor* editor);

    QWidget* createHeaderLabelWidget(const QString& text = QString(),
                                     Qt::Alignment ali = Qt::AlignCenter,
                                     QWidget* heightTarget = NULL);

    MaEditor*                       getEditor() const { return editor; }
    MaEditorSequenceArea*           getSequenceArea() const { return seqArea; }
    MaEditorNameList*               getEditorNameList() { return nameList; }
    MSAEditorConsensusArea*         getConsensusArea() { return consArea; }
    MaEditorOverviewArea*           getOverviewArea() { return overviewArea; }
    MSAEditorOffsetsViewController* getOffsetsViewController() { return offsetsView; }
    ScrollController *              getScrollController();
    BaseWidthController *           getBaseWidthController();
    RowHeightController *           getRowHeightController();
    DrawHelper *                    getDrawHelper();

    QAction* getUndoAction() const;
    QAction* getRedoAction() const;

    QAction* getDelSelectionAction() const { return delSelectionAction; }
    QAction* getCopySelectionAction() const { return copySelectionAction; }
    QAction* getCopyFormattedSelectionAction() const { return copyFormattedSelectionAction; }
    QAction* getPasteAction() const { return pasteAction; }

    // SANGER_TODO: should be muted in case of chromatogram (crutch!)
    // the best is to store it in the MCA widget, of course
    bool isCollapsibleMode() const { return collapsibleMode; }
    void setCollapsibleMode(bool collapse) { collapsibleMode = collapse; }
    MSACollapsibleItemModel* getCollapseModel() const { return collapseModel; }

    QWidget* getHeaderWidget() const { return seqAreaHeader; }

signals:
    void si_startMaChanging();
    void si_stopMaChanging(bool modified = false);
    void si_completeRedraw();

public slots:
    void sl_saveScreenshot();

protected:
    virtual void initWidgets();
    virtual void initActions();

    virtual void initSeqArea(GScrollBar* shBar, GScrollBar* cvBar) = 0;
    virtual void initOverviewArea() = 0;
    virtual void initNameList(QScrollBar* nhBar) = 0;

protected:
    MaEditor*                       editor;
    MaEditorSequenceArea*           seqArea;
    MaEditorNameList*               nameList;
    MSAEditorConsensusArea*         consArea;
    MaEditorOverviewArea*           overviewArea;
    MSAEditorOffsetsViewController* offsetsView;
    MSAEditorStatusWidget*          statusWidget;

    QWidget*                        nameAreaContainer;
    QWidget*                        seqAreaHeader;
    QVBoxLayout*                    seqAreaHeaderLayout;

    QGridLayout*                    seqAreaLayout;
    QVBoxLayout*                    nameAreaLayout;
    MaSplitterController            maSplitter;

    MsaUndoRedoFramework*           undoFWK;

    MSACollapsibleItemModel*        collapseModel;
    bool                            collapsibleMode;
    ScrollController *              scrollController;
    BaseWidthController *           baseWidthController;
    RowHeightController *           rowHeightController;
    DrawHelper *                    drawHelper;

    QAction                         *delSelectionAction;
    QAction                         *copySelectionAction;
    QAction                         *copyFormattedSelectionAction;
    QAction                         *pasteAction;
};

} // namespace

#endif // _U2_MA_EDITOR_WGT_H_

