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

#include "MaSangerOverview.h"

#include "ov_msa/McaEditor.h"
#include "ov_msa/view_rendering/MaEditorSequenceArea.h"

#include <U2Core/U2SafePoints.h>

#include <QPainter>

namespace U2 {

const int MaSangerOverview::READ_HEIGHT = 8;

MaSangerOverview::MaSangerOverview(MaEditorWgt *ui)
    : MaOverview(ui) {
    if (!isValid()) {
        setVisible(false);
    }
    MultipleChromatogramAlignmentObject* mAlignmentObj = getEditor()->getMaObject();
    setFixedHeight(mAlignmentObj->getNumRows() * READ_HEIGHT);
    stepX = READ_HEIGHT;

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

    double stepX = width() / (double)editor->getAlignmentLen();
    double stepY = height() / (double)editor->getNumSequences();

    MultipleChromatogramAlignmentObject* mAlignmentObj = getEditor()->getMaObject();
    SAFE_POINT(NULL != mAlignmentObj, tr("Incorrect multiple alignment object!"), );
    const MultipleChromatogramAlignment ma = mAlignmentObj->getMultipleAlignment();

    for (int seq = 0; seq < editor->getNumSequences(); seq++) {
        const MultipleChromatogramAlignmentRow row =  ma->getMcaRow(seq);
        U2Region r = row->getCoreRegion();

        QRect rect;
        rect.setY( qRound( stepY * (double)seq ) );
        rect.setX( qRound( stepX * (double)r.startPos ) );

        int prev = qRound( stepY * (double)seq );
        int next = qRound( stepY * (double)(seq + 1) );
        rect.setHeight( next - prev );

        prev = qRound( stepX * (double)r.startPos);
        next = qRound( stepX * (double)(r.endPos() - 1));
        rect.setWidth( next - prev );

        drawRead(p, rect, !(seq % 2)); // SANGER_TODO: replace with "getDirection() == U2Strand::Direct"
    }

    p.setPen(Qt::gray);
    p.drawRect( rect().adjusted(0, 0, -1, -1) );
}

void MaSangerOverview::drawVisibleRange(QPainter &p) {
    if (editor->isAlignmentEmpty()) {
        setVisibleRangeForEmptyAlignment();
    } else {
        double stepX = width() / (double)editor->getAlignmentLen();
        double stepY = height() / (double)editor->getNumSequences();

        cachedVisibleRange.setX(qRound(stepX * sequenceArea->getFirstVisibleBase()));
        cachedVisibleRange.setWidth(qRound(stepX * (sequenceArea->getLastVisibleBase(true) - sequenceArea->getFirstVisibleBase() + 1)));

        cachedVisibleRange.setY(qRound(stepY * sequenceArea->getFirstVisibleSequence()));
        cachedVisibleRange.setHeight(qRound(stepY * (sequenceArea->getLastVisibleSequence(true) - sequenceArea->getFirstVisibleSequence() + 1)));

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
    double stepX = width() / (double)editor->getAlignmentLen();
    double stepY = height() / (double)editor->getNumSequences();

    const QRect& overviewRect = rect();
    QRect newVisibleRange(cachedVisibleRange);
    newVisibleRange.moveLeft(_pos.x() - (double)cachedVisibleRange.width() / 2 );

    newVisibleRange.moveTop(_pos.y() - (double)cachedVisibleRange.height() / 2 );

    if (!overviewRect.contains(newVisibleRange)) {
        // fit in overview horizontally
        if (newVisibleRange.x() < 0) {
            newVisibleRange.moveLeft(0);
        } else if (newVisibleRange.topRight().x() > overviewRect.width()) {
            newVisibleRange.moveRight(overviewRect.width());
        }

        // fit in overview vertically
        if (newVisibleRange.y() < 0) {
            newVisibleRange.moveTop(0);
        } else if (newVisibleRange.bottomRight().y() > overviewRect.height()) {
            newVisibleRange.moveBottom(overviewRect.height());
        }
    }

    int pos = qRound( newVisibleRange.x() / stepX );
    sequenceArea->setFirstVisibleBase(pos);
    pos = qRound( newVisibleRange.y() / stepY );
    sequenceArea->setFirstVisibleSequence(pos);
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
