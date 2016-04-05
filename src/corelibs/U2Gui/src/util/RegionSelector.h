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

#ifndef _U2_REGION_SELECTOR_H_
#define _U2_REGION_SELECTOR_H_

#include <U2Core/DNASequenceSelection.h>
#include <U2Core/U2Region.h>
#include <U2Core/global.h>

#include <QDialog>
#include <QLineEdit>

class QComboBox;


namespace U2 {

class AbstractRegionSelector : public QWidget {
public:
    AbstractRegionSelector(QWidget* parent, qint64 maxLen, bool isCircularSelectionAvailable = false)
        : QWidget(parent),
          maxLen(maxLen),
          isCircularSelectionAvailable(isCircularSelectionAvailable) {}

    virtual U2Region getRegion(bool *ok = NULL) const = 0;
    virtual void setRegion(const U2Region& region) = 0;

    virtual void reset() {
        setRegion(U2Region(1, maxLen));
    }

protected:
    qint64  maxLen;
    bool    isCircularSelectionAvailable;
};

class RegionLineEdit : public QLineEdit {
    Q_OBJECT
public:
    RegionLineEdit(QWidget* p, QString actionName, qint64 defVal)
        : QLineEdit(p),
          actionName(actionName),
          defaultValue(defVal) {}

protected:
    void focusOutEvent ( QFocusEvent * event );
    void contextMenuEvent(QContextMenuEvent *);

private slots:
    void sl_onSetMinMaxValue();

private:
    const QString actionName;
    qint64 defaultValue;
};

class U2GUI_EXPORT SimpleRegionSelector : public AbstractRegionSelector {
    Q_OBJECT
public:
    SimpleRegionSelector(QWidget* p, qint64 maxLen, bool isCircularSelectionAvailable = false);

    U2Region getRegion(bool *ok = NULL) const;
    void setRegion(const U2Region& value);

    bool isWholeSequenceSelected() const; // check if it is needed

    void setMaxLength(qint64 length);
    void setMaxRegion();

    void showErrorMessage();

signals:
    void si_regionChanged(const U2Region& newRegion);

private slots:
    void sl_onRegionChanged();
    void sl_onValueEdited();

protected:
    void initLayout();
    void connectSignals();

    RegionLineEdit*       startEdit;
    RegionLineEdit*       endEdit;
};

struct RegionPreset {
    RegionPreset() {}
    RegionPreset(const QString &text, const U2Region &region)
        : text(text),
          region(region) {}
    QString text;
    U2Region region;
};

class U2GUI_EXPORT RegionSelector : public QWidget {
    Q_OBJECT
public:
    RegionSelector(QWidget* p, qint64 len = 0, bool isVertical = false,
                   DNASequenceSelection* selection = NULL,
                   bool isCircularSelectionAvailable = false,
                   QList<RegionPreset> presetRegions = QList<RegionPreset>());

    U2Region getRegion(bool *ok = NULL) const;
    bool isWholeSequenceSelected() const;

    void setCustomRegion(const U2Region& value);
    void setWholeRegionSelected();
    void setCurrentPreset(const QString &presetName);
    void reset();
    void removePreset(const QString &itemName);

    void showErrorMessage();

    static const QString WHOLE_SEQUENCE;
    static const QString SELECTED_REGION;
    static const QString CUSTOM_REGION;

signals:
    void si_regionChanged(const U2Region& newRegion);

private slots:
    void sl_onComboBoxIndexChanged(int index);
    void sl_onRegionChanged();
    void sl_onValueEdited();
    void sl_onSelectionChanged(GSelection* selection);

private:
    void initLayout();
    void init(const QList<RegionPreset> &presetRegions);
    void connectSignals();

    // Returns circular region or the first selected. If none is selected, returns full sequence range.
    U2Region getOneRegionFromSelection() const;

    qint64                maxLen;
    RegionLineEdit *      startEdit;
    RegionLineEdit *      endEdit;
    QComboBox *           comboBox;
    bool                  isVertical;
    QString               defaultItemText;
    DNASequenceSelection *selection;
    bool                  isCircularSelectionAvailable;
};

}//namespace

#endif
