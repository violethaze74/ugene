/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2020 UniPro <ugene@unipro.ru>
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

#include "HoverQLabel.h"

namespace U2 {

HoverQLabel::HoverQLabel(const QString &html, const QString &_normalStyle, const QString &_hoveredStyle, const QString &objectName)
    : QLabel(html), normalStyle(_normalStyle), hoveredStyle(_hoveredStyle) {
    setCursor(Qt::PointingHandCursor);
    setObjectName(objectName);
    if (!objectName.isEmpty()) {
        normalStyle = "#" + objectName + " {" + normalStyle + "}";
        hoveredStyle = "#" + objectName + " {" + hoveredStyle + "}";
    }
    setStyleSheet(normalStyle);
}

void HoverQLabel::enterEvent(QEvent *event) {
    Q_UNUSED(event);
    setStyleSheet(hoveredStyle);
}

void HoverQLabel::leaveEvent(QEvent *event) {
    Q_UNUSED(event);
    setStyleSheet(normalStyle);
}

void HoverQLabel::mousePressEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    emit clicked();
}

}    // namespace U2
