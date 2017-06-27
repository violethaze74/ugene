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

#include <QPainter>

#include <U2Core/U2SafePoints.h>

#include "MaSangerOverview.h"
#include "ov_msa/McaEditor.h"
#include "ov_msa/helpers/BaseWidthController.h"
#include "ov_msa/helpers/RowHeightController.h"
#include "ov_msa/helpers/ScrollController.h"
#include "ov_msa/view_rendering/MaEditorSequenceArea.h"

namespace U2 {

const int MaSangerOverview::READ_HEIGHT = 8;

MaSangerOverview::MaSangerOverview(MaEditorWgt *ui)
    : MaOverview(ui) {
    if (!isValid()) {
        setVisible(false);
    }
    MultipleChromatogramAlignmentObject* mAlignmentObj = getEditor()->getMaObject();
    setFixedHeight(mAlignmentObj->getNumRows() * READ_HEIGHT);      // SANGER_TODO: do something, if there are too many reads

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

bool MaSangerOverview::isValid() const {
    return true;
}

QPixmap MaSangerOverview::getView() {
    // SANGER_TODO: change
    resize(ui->width(), height());
    if (cachedView.isNull()) {
        cachedView = QPixmap(size());
        QPainter p(&cachedView);
        drawOverview(p);
    }
    return cachedView;
}

McaEditor* MaSangerOverview::getEditor() const {
    return qobject_cast<McaEditor*>(editor);
}

void MaSangerOverview::paintEvent(QPaintEvent *e) {
    QPainter p(this);
    p.drawPixmap(0, 0, getView());
    drawVisibleRange(p);
    QWidget::paintEvent(e);
}

void MaSangerOverview::resizeEvent(QResizeEvent *e) {
    cachedView = QPixmap();
    QWidget::resizeEvent(e);
    update();
}

void MaSangerOverview::drawOverview(QPainter &p) {
    p.fillRect(cachedView.rect(), Qt::white);

    if (editor->isAlignmentEmpty()) {
        return;
    }

    recalculateScale();

    MultipleChromatogramAlignmentObject* mAlignmentObj = getEditor()->getMaObject();
    SAFE_POINT(NULL != mAlignmentObj, tr("Incorrect multiple alignment object!"), );
    const MultipleChromatogramAlignment ma = mAlignmentObj->getMultipleAlignment();

    for (int seq = 0; seq < editor->getNumSequences(); seq++) {
        const MultipleChromatogramAlignmentRow row =  ma->getMcaRow(seq);
        const U2Region coreRegion = row->getCoreRegion();
        const U2Region positionRegion = ui->getBaseWidthController()->getBasesGlobalRange(coreRegion);

        QRect readRect;
        readRect.setX(qRound(positionRegion.startPos / stepX));
        readRect.setY(seq * READ_HEIGHT);
        readRect.setHeight(READ_HEIGHT);
        readRect.setWidth(positionRegion.length / stepX);

        drawRead(p, readRect, !row->isReversed()); // SANGER_TODO: replace with "getDirection() == U2Strand::Direct"
    }

    p.setPen(Qt::gray);
    p.drawRect( rect().adjusted(0, 0, -1, -1) );
}

void MaSangerOverview::drawVisibleRange(QPainter &p) {
    if (editor->isAlignmentEmpty()) {
        setVisibleRangeForEmptyAlignment();
    } else {
        recalculateScale();

        const QPoint screenPosition = ui->getScrollController()->getScreenPosition();
        const QSize screenSize = ui->getSequenceArea()->size();

        cachedVisibleRange.setX(qRound(screenPosition.x() / stepX));
        cachedVisibleRange.setWidth(qRound(screenSize.width() / stepX));
        cachedVisibleRange.setY(qRound(screenPosition.y() / stepY));
        cachedVisibleRange.setHeight(qRound(screenSize.height() / stepY));

        if (cachedVisibleRange.width() < VISIBLE_RANGE_CRITICAL_SIZE || cachedVisibleRange.height() < VISIBLE_RANGE_CRITICAL_SIZE) {
            p.setPen(Qt::red);
        }
    }

    p.fillRect(cachedVisibleRange, VISIBLE_RANGE_COLOR);
    p.drawRect(cachedVisibleRange.adjusted(0, 0, -1, -1));
}

void MaSangerOverview::drawSelection(QPainter &p) {
    p.fillRect(cachedSelection, SELECTION_COLOR);
}

void MaSangerOverview::moveVisibleRange(QPoint _pos) {
    // SANGER_TODO: this is located in the separate method in simpleoverview

    QRect newVisibleRange(cachedVisibleRange);
    const int newPosX = qBound((cachedVisibleRange.width() - 1) / 2, _pos.x(), width() - (cachedVisibleRange.width() - 1 ) / 2);
    const int newPosY = qBound((cachedVisibleRange.height() - 1) / 2, _pos.y(), height() - (cachedVisibleRange.height() - 1 ) / 2);
    const QPoint newPos(newPosX, newPosY);
    newVisibleRange.moveCenter(newPos);

    const int newHScrollBarValue = newVisibleRange.x() * stepX;
    ui->getScrollController()->setHScrollbarValue(newHScrollBarValue);
    const int newVScrollBarValue = newVisibleRange.y() * stepY;
    ui->getScrollController()->setVScrollbarValue(newVScrollBarValue);
}

void MaSangerOverview::drawRead(QPainter &p, const QRect &rect, bool forward) {
    if (forward) {
        p.setPen(Qt::SolidLine);
        p.drawLine(rect.topLeft(), rect.bottomLeft());

        // arrow
        p.drawLine(rect.right(), rect.center().y(),
                   rect.right() - READ_HEIGHT / 2,
                   rect.center().y() - READ_HEIGHT / 2);
        p.drawLine(rect.right(), rect.center().y(),
                   rect.right() - READ_HEIGHT / 2,
                   rect.center().y() + READ_HEIGHT / 2);
    } else {
        p.setPen(Qt::DashLine);
        p.drawLine(rect.topRight(), rect.bottomRight());

        // arrow
        p.drawLine(rect.left(), rect.center().y(),
                   rect.left() + READ_HEIGHT / 2,
                   rect.center().y() - READ_HEIGHT / 2);
        p.drawLine(rect.left(), rect.center().y(),
                   rect.left() + READ_HEIGHT / 2,
                   rect.center().y() + READ_HEIGHT / 2);
    }
    p.drawLine(rect.left(), rect.center().y(), rect.right(), rect.center().y());
}

} // namespace
