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

#include "McaEditor.h"
#include "MaEditorFactory.h"

#include "view_rendering/MaEditorWgt.h"

#include <QToolBar>
#include "McaEditorSequenceArea.h"

namespace U2 {

// SANGER_TODO: temporary const, should go with factory
// until there is no factory - do not create separate ID
const GObjectViewFactoryId ID = "MCAEditor";

McaEditor::McaEditor(const QString &viewName, GObject *obj)
    : MaEditor(McaEditorFactory::ID, viewName, obj) {
    showChromatograms = true; // SANGER_TODO: check if there are chromatograms

    // SANGER_TODO: set new proper icon
    showChromatogramsAction = new QAction(QIcon(":/core/images/graphs.png"), tr("Show/hide chromatogram(s)"), this);
    showChromatogramsAction->setObjectName("chromatograms");
    showChromatogramsAction->setCheckable(true);
    showChromatogramsAction->setChecked(showChromatograms);
    connect(showChromatogramsAction, SIGNAL(triggered(bool)), SLOT(sl_showHideChromatograms(bool)));
}

void McaEditor::buildStaticToolbar(QToolBar* tb) {
//    MaEditor::buildStaticToolbar(tb);
    tb->addAction(showChromatogramsAction);

    GObjectView::buildStaticToolbar(tb);
}

void McaEditor::buildStaticMenu(QMenu* m) {
    // SANGER_TODO: review the menus and toolbar
//    MaEditor::buildStaticMenu(m);
}

int McaEditor::getRowHeight() const {
    QFontMetrics fm(font, ui);
    int chromHeigth = 100; // SANGER_TODO: set const chrom height
    return (fm.height() + chromHeigth * showChromatograms)* zoomMult;
}

void McaEditor::sl_showHideChromatograms(bool show) {
    showChromatograms = show;
    emit si_completeUpdate();
}

QWidget* McaEditor::createWidget() {
    Q_ASSERT(ui == NULL);
    ui = new McaEditorWgt(this);

    initActions();

    return ui;
}

McaEditorWgt::McaEditorWgt(MaEditor *editor)
    : MaEditorWgt(editor) {
    initActions();
    initWidgets();
}

void McaEditorWgt::initSeqArea(GScrollBar* shBar, GScrollBar* cvBar) {
    seqArea = new McaEditorSequenceArea(this, shBar, cvBar);
}

} // namespace
