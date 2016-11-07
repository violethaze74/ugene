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

#include "MaEditor.h"

#include "view_rendering/MaEditorWgt.h"

#include <U2Core/Counter.h>
#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2View/MSAEditorSequenceArea.h>

namespace U2 {

SNPSettings::SNPSettings()
    : seqId(U2MsaRow::INVALID_ROW_ID) {
}

const float MaEditor::zoomMult = 1.25;

MaEditor::MaEditor(GObjectViewFactoryId factoryId, const QString &viewName, GObject *obj)
    : GObjectView(factoryId, viewName),
      ui(NULL),
      exportHighlightedAction(NULL)
{
    msaObject = qobject_cast<MultipleSequenceAlignmentObject*>(obj);
    objects.append(msaObject);

    onObjectAdded(msaObject);

    requiredObjects.append(msaObject);
    GCOUNTER(cvar,tvar,factoryId);

    if (!U2DbiUtils::isDbiReadOnly(msaObject->getEntityRef().dbiRef)) {
        U2OpStatus2Log os;
        msaObject->setTrackMod(os, TrackOnUpdate);
    }
}

int MaEditor::getAlignmentLen() const {
    return msaObject->getLength();
}

int MaEditor::getNumSequences() const {
    return msaObject->getNumRows();
}

bool MaEditor::isAlignmentEmpty() const {
    return getAlignmentLen() == 0 || getNumSequences() == 0;
}

const QRect& MaEditor::getCurrentSelection() const {
    return ui->getSequenceArea()->getSelection().getRect();
}

int MaEditor::getRowHeight() const {
    QFontMetrics fm(font, ui);
    int chromHeigth = 100; // SANGER_TODO: set const chrom height
    return (fm.height() + chromHeigth /** showChromatograms */)* zoomMult;
}

int MaEditor::getSequenceRowHeight() const {
    QFontMetrics fm(font, ui);
    return fm.height() * zoomMult;
}

int MaEditor::getColumnWidth() const {
    QFontMetrics fm(font, ui);
    int width =  fm.width('W') * zoomMult;

    width = (int)(width * zoomFactor);
    width = qMax(width, MOBJECT_MIN_COLUMN_WIDTH);

    return width;
}

QVariantMap MaEditor::getHighlightingSettings(const QString &highlightingFactoryId) const {
    const QVariant v = snp.highlightSchemeSettings.value(highlightingFactoryId);
    if (v.isNull()) {
        return QVariantMap();
    } else {
        CHECK(v.type() == QVariant::Map, QVariantMap());
        return v.toMap();
    }
}

void MaEditor::saveHighlightingSettings( const QString &highlightingFactoryId, const QVariantMap &settingsMap /* = QVariant()*/ ) {
    snp.highlightSchemeSettings.insert(highlightingFactoryId, QVariant(settingsMap));
}

QString MaEditor::getReferenceRowName() const {
    const MultipleSequenceAlignment alignment = getMSAObject()->getMsa();
    U2OpStatusImpl os;
    const int refSeq = alignment->getRowIndexByRowId(getReferenceRowId(), os);
    return (U2MsaRow::INVALID_ROW_ID != refSeq) ? alignment->getRowNames().at(refSeq)
        : QString();
}

void MaEditor::setReference(qint64 sequenceId) {
    if(sequenceId == U2MsaRow::INVALID_ROW_ID){
        exportHighlightedAction->setDisabled(true);
    }else{
        exportHighlightedAction->setEnabled(true);
    }
    if(snp.seqId != sequenceId) {
        snp.seqId = sequenceId;
        emit si_referenceSeqChanged(sequenceId);
    }
    //REDRAW OTHER WIDGETS
}

void MaEditor::updateReference(){
    if(msaObject->getRowPosById(snp.seqId) == -1){
        setReference(U2MsaRow::INVALID_ROW_ID);
    }
}

void MaEditor::resetCollapsibleModel() {
    MSACollapsibleItemModel *collapsibleModel = ui->getCollapseModel();
    SAFE_POINT(NULL != collapsibleModel, "NULL collapsible model!", );
    collapsibleModel->reset();
}

} // namespace
