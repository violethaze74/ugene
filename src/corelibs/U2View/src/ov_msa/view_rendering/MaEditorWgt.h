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

#ifndef _U2_MA_EDITOR_WGT_
#define _U2_MA_EDITOR_WGT_

#include <QWidget>

#include "MaEditorUtils.h"

namespace U2 {

class MSACollapsibleItemModel;
class MaEditor;
class MSAEditorConsensusArea;
class MSAEditorNameList;
class MSAEditorSequenceArea;
class MSAEditorOffsetsViewController;
class MSAEditorOverviewArea;
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

    // SANGER_TODO: it is for multi tree case - decide if it is neccessary
    QWidget* createLabelWidget(const QString& text = QString(), Qt::Alignment ali = Qt::AlignCenter);

    MaEditor*                       getEditor() const { return editor; }
    MSAEditorSequenceArea*          getSequenceArea() { return seqArea; }
    MSAEditorNameList*              getEditorNameList() { return nameList; }
    MSAEditorConsensusArea*         getConsensusArea() { return consArea; }
    MSAEditorOverviewArea*          getOverviewArea() { return overviewArea; }
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

protected:
    void initWidgets();
    void initActions();

protected:
    MaEditor*                       editor;
    MSAEditorSequenceArea*          seqArea;
    SequenceAreaRenderer*           saRenderer;
    MSAEditorNameList*              nameList;
    MSAEditorConsensusArea*         consArea;
    MSAEditorOverviewArea*          overviewArea;
    MSAEditorOffsetsViewController* offsetsView;
    MSAEditorStatusWidget*          statusWidget;

    QWidget*                        nameAreaContainer; // SANGER_TODO: there is no need to store the variable
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

#endif // _U2_MA_EDITOR_WGT_

