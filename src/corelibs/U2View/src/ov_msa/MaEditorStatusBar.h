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

#ifndef _U2_MSA_EDITOR_STATUS_BAR_H_
#define _U2_MSA_EDITOR_STATUS_BAR_H_

#include <QLabel>
#include <QVariant>

class QHBoxLayout;

namespace U2 {

class MultipleAlignmentObject;
class MaEditorSequenceArea;
class MaEditorSelection;

class MaEditorStatusBar : public QWidget {
    Q_OBJECT
protected:
    class TwoArgPatternLabel : public QLabel {
    public:
        TwoArgPatternLabel(QString textPattern, QString tooltipPattern,
                           QString objectName, QWidget* parent = NULL);
        TwoArgPatternLabel(QString objectName, QWidget* parent = NULL);
        void setPatterns(QString textPattern, QString tooltipPattern);
        void update(QString firstArg, QString secondArg);

    private:
        QString         textPattern;
        QString         tooltipPattern;
        QFontMetrics    fm;
    };

public:
    MaEditorStatusBar(MultipleAlignmentObject* mobj, MaEditorSequenceArea* seqArea);

private slots:
    void sl_alignmentChanged();
    void sl_lockStateChanged();
    void sl_selectionChanged(const MaEditorSelection& , const MaEditorSelection& );

protected:
    virtual void setupLayout() = 0;
    virtual void updateLabels();

private:
    void updateLock();
    QPair<QString, int> getGappedColumnInfo(const QPoint& pos) const;

protected:
    MultipleAlignmentObject*    aliObj;
    MaEditorSequenceArea*       seqArea;
    QPixmap                     lockedIcon;
    QPixmap                     unlockedIcon;

    QHBoxLayout*                layout;
    TwoArgPatternLabel*         lineLabel;
    TwoArgPatternLabel*         colomnLabel;
    TwoArgPatternLabel*         positionLabel;
    TwoArgPatternLabel*         selectionLabel;
    QLabel*                     lockLabel;

private:
    QString selectionPattern;
};



}//namespace;

#endif
