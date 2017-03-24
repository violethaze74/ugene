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

#ifndef _U2_MSA_COLOR_SCHEME_H_
#define _U2_MSA_COLOR_SCHEME_H_

#include <QColor>
#include <U2Core/global.h>

namespace U2 {

class MultipleAlignmentObject;
class MsaColorSchemeCustomFactory;
class MsaColorSchemeFactory;

class U2ALGORITHM_EXPORT ColorSchemeData {
public:
    ColorSchemeData();

    QString name;
    DNAAlphabetType type;
    bool defaultAlpType;
    QMap<char, QColor> alpColors;
};

class U2ALGORITHM_EXPORT MsaColorScheme : public QObject {
    Q_OBJECT
public:
    MsaColorScheme(QObject *parent, const MsaColorSchemeFactory *factory, MultipleAlignmentObject *maObj);

    //Get color for symbol "c" on position [seq, pos]. Variable "c" has been added for optimization.
    virtual QColor getColor(int seq, int pos, char c) const = 0;

    const MsaColorSchemeFactory * getFactory() const;

    static const QString EMPTY_NUCL;
    static const QString UGENE_NUCL;
    static const QString JALVIEW_NUCL;
    static const QString IDENTPERC_NUCL;
    static const QString IDENTPERC_NUCL_GRAY;
    static const QString CUSTOM_NUCL;

    static const QString EMPTY_AMINO;
    static const QString UGENE_AMINO;
    static const QString ZAPPO_AMINO;
    static const QString TAILOR_AMINO;
    static const QString HYDRO_AMINO;
    static const QString HELIX_AMINO;
    static const QString STRAND_AMINO;
    static const QString TURN_AMINO;
    static const QString BURIED_AMINO;
    static const QString IDENTPERC_AMINO;
    static const QString IDENTPERC_AMINO_GRAY;
    static const QString CLUSTALX_AMINO;
    static const QString CUSTOM_AMINO;

    static const QString EMPTY_RAW;

protected:
    const MsaColorSchemeFactory *   factory;
    MultipleAlignmentObject *       maObj;
};

class U2ALGORITHM_EXPORT MsaColorSchemeFactory : public QObject {
    Q_OBJECT
public:
    MsaColorSchemeFactory(QObject *parent, const QString &id, const QString &name, DNAAlphabetType alphabetType);
    virtual MsaColorScheme * create(QObject *p, MultipleAlignmentObject *obj) const = 0;

    const QString & getId() const;
    const QString getName(bool nameWithAlphabet = false) const;
    DNAAlphabetType getAlphabetType() const;

signals:
    void si_factoryChanged();

protected:
    QString         id;
    QString         name;
    DNAAlphabetType alphabetType;
};

class U2ALGORITHM_EXPORT MsaColorSchemeRegistry : public QObject {
    Q_OBJECT
public:
    MsaColorSchemeRegistry();
    ~MsaColorSchemeRegistry();

    const QList<MsaColorSchemeFactory *> & getMsaColorSchemes() const;
    const QList<MsaColorSchemeCustomFactory *> &getCustomColorSchemes() const;

    static QStringList getExcludedIdsFromRawAlphabetSchemes();

    QList<MsaColorSchemeFactory *> getMsaColorSchemes(DNAAlphabetType alphabetType) const;
    QList<MsaColorSchemeFactory *> getMsaCustomColorSchemes(DNAAlphabetType alphabetType) const;

    MsaColorSchemeCustomFactory * getMsaCustomColorSchemeFactoryById(const QString &id) const;
    MsaColorSchemeFactory * getMsaColorSchemeFactoryById(const QString &id) const;

signals:
    void si_customSettingsChanged();

private slots:
    void sl_onCustomSettingsChanged();

private:
    void addCustomScheme(const ColorSchemeData& scheme);
    void addMsaColorSchemeFactory(MsaColorSchemeFactory *commonFactory);
    void addMsaCustomColorSchemeFactory(MsaColorSchemeCustomFactory *customFactory);

    void deleteOldCustomFactories();
    void initBuiltInSchemes();
    void initCustomSchema();

    QList<MsaColorSchemeFactory *> colorers;
    QList<MsaColorSchemeCustomFactory *> customColorers;
};

}   // namespace U2

#endif // _U2_MSA_COLOR_SCHEME_H_
