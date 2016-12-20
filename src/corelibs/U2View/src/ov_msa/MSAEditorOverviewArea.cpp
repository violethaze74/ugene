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

#include "MSAEditorOverviewArea.h"

#include "MSAEditor.h"
#include "MSAEditorSequenceArea.h"
#include "Overview/MaSimpleOverview.h"
#include "Overview/MaGraphOverview.h"
#include "Overview/MaOverviewContextMenu.h"

#include <QVBoxLayout>
#include <QActionGroup>


namespace U2 {

const QString MSAEditorOverviewArea::OVERVIEW_AREA_OBJECT_NAME  = "msa_overview_area";

MSAEditorOverviewArea::MSAEditorOverviewArea(MaEditorWgt *ui) {
    setObjectName(OVERVIEW_AREA_OBJECT_NAME);

    simpleOverview = new MaSimpleOverview(ui);
    graphOverview = new MaGraphOverview(ui);

    simpleOverview->setObjectName(OVERVIEW_AREA_OBJECT_NAME + QString("_simple"));
    graphOverview->setObjectName(OVERVIEW_AREA_OBJECT_NAME + QString("_graph"));

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(simpleOverview);
    layout->addWidget(graphOverview);
    setLayout(layout);

    connect(ui, SIGNAL(customContextMenuRequested(QPoint)), SLOT(sl_onContextMenuRequested(QPoint)));
    connect(ui->getSequenceArea(), SIGNAL(si_highlightingChanged()),
            simpleOverview, SLOT(sl_highlightingChanged()));
    connect(ui->getSequenceArea(), SIGNAL(si_highlightingChanged()),
            graphOverview, SLOT(sl_highlightingChanged()));
    connect(ui->getEditor(), SIGNAL(si_referenceSeqChanged(qint64)),
            graphOverview, SLOT(sl_highlightingChanged()));
    connect(ui->getEditor(), SIGNAL(si_referenceSeqChanged(qint64)),
            simpleOverview, SLOT(sl_highlightingChanged()));

    contextMenu =  new MaOverviewContextMenu(simpleOverview, graphOverview);

    connect(contextMenu, SIGNAL(si_graphTypeSelected(MaGraphOverviewDisplaySettings::GraphType)),
            graphOverview, SLOT(sl_graphTypeChanged(MaGraphOverviewDisplaySettings::GraphType)));
    connect(contextMenu, SIGNAL(si_colorSelected(QColor)),
            graphOverview, SLOT(sl_graphColorChanged(QColor)));
    connect(contextMenu, SIGNAL(si_graphOrientationSelected(MaGraphOverviewDisplaySettings::OrientationMode)),
            graphOverview, SLOT(sl_graphOrientationChanged(MaGraphOverviewDisplaySettings::OrientationMode)));
    connect(contextMenu, SIGNAL(si_calculationMethodSelected(MaGraphCalculationMethod)),
            graphOverview, SLOT(sl_calculationMethodChanged(MaGraphCalculationMethod)));

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMaximumHeight( graphOverview->FIXED_HEIGHT + simpleOverview->FIXED_HEIGTH + 5 );
}

void MSAEditorOverviewArea::cancelRendering() {
    graphOverview->cancelRendering();
}

bool MSAEditorOverviewArea::isOverviewWidget(QWidget *wgt) const {
    if (wgt == simpleOverview || wgt == graphOverview || wgt == this) {
        return true;
    }
    return false;
}

void MSAEditorOverviewArea::sl_onContextMenuRequested(const QPoint &p) {
    if (geometry().contains(p)) {
        contextMenu->exec(QCursor::pos());
    }
}

void MSAEditorOverviewArea::sl_show() {
    setVisible( !isVisible() );
    if (graphOverview->isVisible()) {
        graphOverview->sl_unblockRendering(true);
    } else {
        graphOverview->sl_blockRendering();
        cancelRendering();
    }
}

} // namespace
