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

class GScrollBar;
class MaEditorSequenceArea;
class MSACollapsibleItemModel;
class MaEditor;
class MaEditorConsensusArea;
class MaEditorNameList;
class MSAEditorOffsetsViewController;
class MaEditorOverviewArea;
class MSAEditorStatusWidget;
class MsaUndoRedoFramework;
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
    MaEditorConsensusArea*          getConsensusArea() { return consArea; }
    MaEditorOverviewArea*           getOverviewArea() { return overviewArea; }
    MSAEditorOffsetsViewController* getOffsetsViewController() { return offsetsView; }

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
    void si_startMsaChanging();
    void si_stopMsaChanging(bool modified = false);

public slots:
    void sl_saveScreenshot();

protected:
    virtual void initWidgets();
    virtual void initActions();

    virtual void initSeqArea(GScrollBar* shBar, GScrollBar* cvBar) = 0;
    virtual void initOverviewArea() = 0;
    virtual void initNameList(QScrollBar* nhBar) = 0;
    virtual void initConsensusArea() = 0;

protected:
    MaEditor*                       editor;
    MaEditorSequenceArea*           seqArea;
    MaEditorNameList*               nameList;
    MaEditorConsensusArea*          consArea;
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

    QAction                         *delSelectionAction;
    QAction                         *copySelectionAction;
    QAction                         *copyFormattedSelectionAction;
    QAction                         *pasteAction;
};

} // namespace

#endif // _U2_MA_EDITOR_WGT_H_

