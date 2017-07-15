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

#include <QPixmap>

#include <U2Core/U2SafePoints.h>

#include <U2View/MSAEditor.h>

#include "MaExportConsensusTab.h"

#include "MaExportConsensusTabFactory.h"

namespace U2 {

const QString MaExportConsensusTabFactory::GROUP_ID = "OP_EXPORT_CONSENSUS";
const QString MaExportConsensusTabFactory::GROUP_ICON_STR = ":core/images/consensus.png";
const QString MaExportConsensusTabFactory::GROUP_DOC_PAGE = "19759549";

MaExportConsensusTabFactory::MaExportConsensusTabFactory() {

}

QWidget * MaExportConsensusTabFactory::createWidget(GObjectView* objView) {
    SAFE_POINT(NULL != objView,
        QString("Internal error: unable to create widget for group '%1', object view is NULL.").arg(GROUP_ID),
        NULL);

    MaEditor* ma = qobject_cast<MaEditor*>(objView);
    SAFE_POINT(NULL != ma,
        QString("Internal error: unable to cast object view to MaEditor for group '%1'.").arg(GROUP_ID),
        NULL);

    MaExportConsensusTab *widget = new MaExportConsensusTab(ma);
    return widget;
}

OPGroupParameters MaExportConsensusTabFactory::getOPGroupParameters() {
    return OPGroupParameters(GROUP_ID, QPixmap(GROUP_ICON_STR), QObject::tr("Export Consensus"), GROUP_DOC_PAGE);
}

MsaExportConsensusTabFactory::MsaExportConsensusTabFactory() {
    objectViewOfWidget = ObjViewType_AlignmentEditor;
}

McaExportConsensusTabFactory::McaExportConsensusTabFactory() {
    objectViewOfWidget = ObjViewType_ChromAlignmentEditor;
}

} // namespace U2
