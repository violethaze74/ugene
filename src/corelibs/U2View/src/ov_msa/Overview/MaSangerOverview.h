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

#ifndef _U2_MA_SANGER_OVERVIEW_H_
#define _U2_MA_SANGER_OVERVIEW_H_

#include "MaOverview.h"

namespace U2 {

class McaEditor;

class MaSangerOverview : public MaOverview {
    Q_OBJECT
public:
    MaSangerOverview(MaEditorWgt* ui);

    bool isValid() const;
    QPixmap getView();

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

private:
    void drawOverview(QPainter &p);
    void drawVisibleRange(QPainter &p);
    void drawSelection(QPainter &p);

    void moveVisibleRange(QPoint pos);

    void drawRead(QPainter &p, const QRect& rect, bool forward);

private:
    McaEditor* getEditor() const;
    static const int READ_HEIGHT;
};

} // namespace

#endif // _U2_MA_SANGER_OVERVIEW_H_
