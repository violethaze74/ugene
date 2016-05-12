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

#ifndef _U2_REGION_SELECTOR_CONTROLLER_H_
#define _U2_REGION_SELECTOR_CONTROLLER_H_

#include <U2Core/global.h>

#include <U2Core/U2Region.h>

#include <QLineEdit>
#include <QComboBox>


namespace U2 {

class DNASequenceSelection;
class GSelection;

struct RegionSelectorGui {
    RegionSelectorGui()
        : startLineEdit(NULL),
          endLineEdit(NULL),
          presetsComboBox(NULL) {}

    RegionSelectorGui(QLineEdit* start, QLineEdit* end, QComboBox* presets = NULL)
        : startLineEdit(start),
          endLineEdit(end),
          presetsComboBox(presets) {}

    QLineEdit* startLineEdit;
    QLineEdit* endLineEdit;
    QComboBox* presetsComboBox;
};

struct RegionPreset {
    RegionPreset() {}
    RegionPreset(const QString &text, const U2Region &region)
        : text(text),
          region(region) {}
    QString text;
    U2Region region;

    bool operator ==(const RegionPreset& other) {
        return (text == other.text);
    }
};

struct RegionSelectorSettings {
    static const QString WHOLE_SEQUENCE;
    static const QString SELECTED_REGION;
    static const QString CUSTOM_REGION;

    RegionSelectorSettings(qint64 maxLen,
                           bool isCircularSelectionAvailable = false,
                           DNASequenceSelection* selection = NULL,
                           QList<RegionPreset> presetRegions = QList<RegionPreset>(),
                           QString defaultPreset = WHOLE_SEQUENCE);

    qint64                  maxLen;
    DNASequenceSelection*   selection;
    bool                    isCircularSelectionAvailable;

    QList<RegionPreset>     presetRegions;
    QString                 defaultPreset;

    // Returns circular region or the first selected. If none is selected, returns full sequence range.
    U2Region getOneRegionFromSelection() const;
};

class RegionSelectorController : public QObject {
    Q_OBJECT
public:
    RegionSelectorController(RegionSelectorGui gui,
                             RegionSelectorSettings settings,
                             QObject* parent);

    U2Region getRegion(bool *ok = NULL) const;
    void setRegion(const U2Region& region);

    QString getPresetName() const;
    void setPreset(QString preset);
    void removePreset(QString preset);

    void reset();

    bool hasError() const;
    QString getErrorMessage() const;

signals:
    void si_regionChanged(const U2Region& newRegion);


private slots:
    void sl_regionChanged(const U2Region& newRegion);
    void sl_onPresetChanged(int index);
    void sl_onRegionChanged();
    void sl_onSelectionChanged(GSelection* selection);
    void sl_onValueEdited();

private:
    void init();
    void setupPresets();
    void connectSlots();

private:
    RegionSelectorGui       gui;
    RegionSelectorSettings  settings;
};

} // namespace

#endif // _U2_REGION_SELECTOR_CONTROLLER_H_
