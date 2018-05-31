/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2018 UniPro <ugene@unipro.ru>
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

#include <QCoreApplication>
#include <QString>
#include <QWidget>

namespace U2 {
namespace LocalWorkflow {

class TrimmomaticSettingsWidget;

struct TrimmomaticSteps {
    static const QStringList name;
};

class TrimmomaticBaseController : public QObject {
    Q_OBJECT
public:
    TrimmomaticBaseController();

    virtual const QString getName() const = 0;
    virtual const QString getDescription() const = 0;
    virtual QWidget* createWidget() = 0;
    virtual QString generateReport() const = 0;

    QWidget* getSettingsWidget() const;
    bool isOkEnable();
    void setOkEnable(bool isEnable);

    static QString getDefaultDescription();

    signals:
        void si_checkOkEnable();

protected:
    QWidget* settingsWidget;
    bool okEnable;
};

class TrimmomaticIlluminaClipController : public TrimmomaticBaseController {
public:
    TrimmomaticIlluminaClipController();

    const QString getName() const;
    const QString getDescription() const;
    QWidget* createWidget();
    QString generateReport() const;

    QString getFileName() const;
    QString getMismatches() const;
    QString getPalindromeThreshold() const;
    QString getSimpleThreshold() const;

    void setFileName(const QString& fileName);
    void setMismatches(const QString& mismatches);
    void setPalindromeThreshold(const QString& palindromeThreshold);
    void setSimpleThreshold(const QString& simpleThreshold);

    void setNullPointerToWidget();

private:
    static const QString NAME;
    static const QString DESCRIPTION;

    QString fileNameValue;
    QString mismatchesValue;
    QString palindromeThresholdValue;
    QString simpleThresholdValue;
};

}
}
