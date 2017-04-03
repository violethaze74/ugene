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

#ifndef _U2_MSA_HIGHLIGHTING_SCHEME_H_
#define _U2_MSA_HIGHLIGHTING_SCHEME_H_

#include <U2Core/global.h>

class QColor;

namespace U2 {

class MultipleAlignmentObject;
class MsaHighlightingSchemeFactory;

class U2ALGORITHM_EXPORT MsaHighlightingScheme : public QObject {
    Q_OBJECT
public:
    MsaHighlightingScheme(QObject *parent, const MsaHighlightingSchemeFactory *factory, MultipleAlignmentObject *maObj);

    virtual void process(const char refChar, char &seqChar, QColor &color, bool &highlight, int refCharColumn, int refCharRow) const;
    const MsaHighlightingSchemeFactory * getFactory() const;

    void setUseDots(bool use);
    bool getUseDots() const;

    virtual void applySettings(const QVariantMap &settings);
    virtual QVariantMap getSettings() const;

    static const QString EMPTY_NUCL;
    static const QString EMPTY_AMINO;
    static const QString EMPTY_RAW;
    static const QString AGREEMENTS_NUCL;
    static const QString AGREEMENTS_AMINO;
    static const QString DISAGREEMENTS_NUCL;
    static const QString DISAGREEMENTS_AMINO;
    static const QString TRANSITIONS_NUCL;
    static const QString TRANSVERSIONS_NUCL;
    static const QString GAPS_NUCL;
    static const QString GAPS_AMINO;
    static const QString GAPS_RAW;
    static const QString CONSERVATION_NUCL;
    static const QString CONSERVATION_AMINO;

    static const QString THRESHOLD_PARAMETER_NAME;
    static const QString LESS_THAN_THRESHOLD_PARAMETER_NAME;

protected:
    const MsaHighlightingSchemeFactory *factory;
    MultipleAlignmentObject *maObj;
    bool useDots;
};

class U2ALGORITHM_EXPORT MsaHighlightingSchemeFactory : public QObject {
    Q_OBJECT
public:
    MsaHighlightingSchemeFactory(QObject *parent, const QString &id, const QString &name, DNAAlphabetType alphabetType,
                                 bool refFree = false, bool needThreshold = false);

    virtual MsaHighlightingScheme * create(QObject *parent, MultipleAlignmentObject *maObj) const = 0;

    const QString & getId() const;
    const QString getName(bool nameWithAlphabet = false) const;
    DNAAlphabetType getAlphabetType() const;
    bool isRefFree() const;
    bool isNeedThreshold() const;

private:
    QString         id;
    QString         name;
    DNAAlphabetType alphabetType;
    bool            refFree;
    bool            needThreshold;
};

class U2ALGORITHM_EXPORT MsaHighlightingSchemeRegistry : public QObject {
    Q_OBJECT
public:
    MsaHighlightingSchemeRegistry();
    ~MsaHighlightingSchemeRegistry();

    static QStringList getExcludedIdsFromRawAlphabetSchemes();

    MsaHighlightingSchemeFactory * getMsaHighlightingSchemeFactoryById(const QString &id) const;
    QList<MsaHighlightingSchemeFactory *> getMsaHighlightingSchemes(DNAAlphabetType alphabetType) const;

private:
    QList<MsaHighlightingSchemeFactory *> schemes;
};

}   // namespace U2

#endif // _U2_MSA_HIGHLIGHTING_SCHEME_H_
