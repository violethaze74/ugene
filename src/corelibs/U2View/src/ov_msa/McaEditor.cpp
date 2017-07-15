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

#include <QToolBar>

#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/OptionsPanel.h>
#include <U2Gui/OPWidgetFactoryRegistry.h>

#include "MaEditorFactory.h"
#include "McaEditor.h"
#include "ov_sequence/SequenceObjectContext.h"
#include "view_rendering/SequenceWithChromatogramAreaRenderer.h"

namespace U2 {

McaEditor::McaEditor(const QString &viewName,
                     MultipleChromatogramAlignmentObject *obj)
    : MaEditor(McaEditorFactory::ID, viewName, obj),
      referenceCtx(NULL)
{

    // SANGER_TODO: set new proper icon
    showChromatogramsAction = new QAction(QIcon(":/core/images/graphs.png"), tr("Show/hide chromatogram(s)"), this);
    showChromatogramsAction->setObjectName("chromatograms");
    showChromatogramsAction->setCheckable(true);
    showChromatogramsAction->setChecked(true);
    connect(showChromatogramsAction, SIGNAL(triggered(bool)), SLOT(sl_showHideChromatograms(bool)));

    U2OpStatusImpl os;
    foreach (const MultipleChromatogramAlignmentRow& row, obj->getMca()->getMcaRows()) {
        chromVisibility.insert(obj->getMca()->getRowIndexByRowId(row->getRowId(), os), true);
    }

    U2SequenceObject* referenceObj = obj->getReferenceObj();
    if (referenceObj) {
        // SANGER_TODO: probably can be big
        referenceCtx = new SequenceObjectContext(referenceObj, this);
    } else {
        FAIL("Trying to open McaEditor without a reference", );
    }
}

MultipleChromatogramAlignmentObject *McaEditor::getMaObject() const {
    return qobject_cast<MultipleChromatogramAlignmentObject*>(maObject);
}

McaEditorWgt *McaEditor::getUI() const {
    return qobject_cast<McaEditorWgt *>(ui);
}

void McaEditor::buildStaticToolbar(QToolBar* tb) {
    tb->addAction(zoomInAction);
    tb->addAction(zoomOutAction);
    tb->addAction(zoomToSelectionAction);
    tb->addAction(resetZoomAction);

    tb->addAction(showOverviewAction);
    tb->addAction(showChromatogramsAction);
    tb->addAction(changeFontAction);

    GObjectView::buildStaticToolbar(tb);
}

void McaEditor::buildStaticMenu(QMenu* m) {
    // SANGER_TODO: review the menus and toolbar
//    MaEditor::buildStaticMenu(m);
}

int McaEditor::getRowContentIndent(int rowId) const {
    if (isChromVisible(rowId)) {
        return SequenceWithChromatogramAreaRenderer::INDENT_BETWEEN_ROWS / 2;
    }
    return MaEditor::getRowContentIndent(rowId);
}

bool McaEditor::isChromVisible(qint64 rowId) const {
    return isChromVisible(getMaObject()->getRowPosById(rowId));
}

bool McaEditor::isChromVisible(int rowIndex) const {
    return !ui->getCollapseModel()->isItemCollapsed(rowIndex);
}

QString McaEditor::getReferenceRowName() const {
    return getMaObject()->getReferenceObj()->getSequenceName();
}

char McaEditor::getReferenceCharAt(int pos) const {
    U2OpStatus2Log os;
    SAFE_POINT(getMaObject()->getReferenceObj()->getSequenceLength() > pos, "Invalid position", '\n');
    QByteArray seqData = getMaObject()->getReferenceObj()->getSequenceData(U2Region(pos, 1), os);
    CHECK_OP(os, U2Msa::GAP_CHAR);
    return seqData.isEmpty() ? U2Msa::GAP_CHAR : seqData.at(0);
}

SequenceObjectContext* McaEditor::getReferenceContext() const {
    return referenceCtx;
}

void McaEditor::sl_onContextMenuRequested(const QPoint & /*pos*/) {
    QMenu m;

    addCopyMenu(&m);
    addViewMenu(&m);
    addEditMenu(&m);
    addExportMenu(&m);

    m.addSeparator();

    emit si_buildPopupMenu(this, &m);

    GUIUtils::disableEmptySubmenus(&m);

    m.exec(QCursor::pos());
}

void McaEditor::sl_showHideChromatograms(bool show) {
    ui->getCollapseModel()->collapseAll(!show);
    emit si_completeUpdate();
}

QWidget* McaEditor::createWidget() {
    Q_ASSERT(ui == NULL);
    ui = new McaEditorWgt(this);

    QString objName = "mca_editor_" + maObject->getGObjectName();
    ui->setObjectName(objName);

    connect(ui , SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sl_onContextMenuRequested(const QPoint &)));

    initActions();

    optionsPanel = new OptionsPanel(this);
    OPWidgetFactoryRegistry *opWidgetFactoryRegistry = AppContext::getOPWidgetFactoryRegistry();

    QList<OPFactoryFilterVisitorInterface*> filters;
    filters.append(new OPFactoryFilterVisitor(ObjViewType_ChromAlignmentEditor));

    QList<OPWidgetFactory*> opWidgetFactories = opWidgetFactoryRegistry->getRegisteredFactories(filters);
    foreach (OPWidgetFactory *factory, opWidgetFactories) {
        optionsPanel->addGroup(factory);
    }

    qDeleteAll(filters);

    return ui;
}

}   // namespace U2
