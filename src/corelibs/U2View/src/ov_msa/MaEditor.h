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

#ifndef _U2_MA_EDITOR_
#define _U2_MA_EDITOR_

#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class MaEditorWgt;
class MultipleSequenceAlignmentObject;


class SNPSettings {
public:
    SNPSettings();
    QPoint clickPoint;
    qint64 seqId;
    QVariantMap highlightSchemeSettings;
};

class MaEditor : public GObjectView {
    Q_OBJECT
public:
    enum ResizeMode {
        ResizeMode_FontAndContent, ResizeMode_OnlyContent
    };
    static const float zoomMult; // SANGER_TODO: should be dependable on the view

public:
    MaEditor(GObjectViewFactoryId factoryId, const QString& viewName, GObject* obj);

    MultipleSequenceAlignmentObject* getMSAObject() const { return msaObject; }

    MaEditorWgt* getUI() const { return ui; }

    const QFont& getFont() const { return font; }

    ResizeMode getResizeMode() const { return resizeMode; }

    int getAlignmentLen() const;

    int getNumSequences() const;

    bool isAlignmentEmpty() const;

    const QRect& getCurrentSelection() const;

    int getRowHeight() const;
    int getSequenceRowHeight() const; // SANGER_TODO: order the methods

    int getColumnWidth() const;

    QVariantMap getHighlightingSettings(const QString &highlightingFactoryId) const;

    void saveHighlightingSettings(const QString &highlightingFactoryId, const QVariantMap &settingsMap = QVariantMap());

    qint64 getReferenceRowId() const { return snp.seqId; }

    QString getReferenceRowName() const;

    void setReference(qint64 sequenceId);

    void updateReference();

    void resetCollapsibleModel(); // SANGER_TODO: collapsible shouldn't be here

signals:
    void si_fontChanged(const QFont& f);
    void si_zoomOperationPerformed(bool resizeModeChanged);
    void si_referenceSeqChanged(qint64 referenceId);
    void si_sizeChanged(int newHeight, bool isMinimumSize, bool isMaximumSize);
    void si_completeUpdate();

protected:
    virtual QWidget* createWidget() = 0;

protected:
    MultipleSequenceAlignmentObject*    msaObject;
    MaEditorWgt*                        ui;

    QFont       font;
    ResizeMode  resizeMode;
    SNPSettings snp;
    float       zoomFactor;

    QAction*          exportHighlightedAction;
};

} // namespace

#endif // _U2_MA_EDITOR_
