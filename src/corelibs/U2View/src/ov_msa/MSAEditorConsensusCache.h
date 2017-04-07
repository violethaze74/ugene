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

#ifndef _U2_MSA_EDITOR_CONSENSUS_CACHE_H_
#define _U2_MSA_EDITOR_CONSENSUS_CACHE_H_

#include <QBitArray>
#include <QObject>
#include <QVector>

#include <U2Core/MultipleSequenceAlignment.h>

namespace U2 {

class MaEditor;

class MSAConsensusAlgorithm;
class MSAConsensusAlgorithmFactory;
class MaModificationInfo;
class MultipleAlignmentObject;
class U2OpStatus;

class U2VIEW_EXPORT MSAEditorConsensusCache : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(MSAEditorConsensusCache)
public:
    MSAEditorConsensusCache(QObject* p, MultipleAlignmentObject* aliObj, MSAConsensusAlgorithmFactory* algo);
    ~MSAEditorConsensusCache();

    char getConsensusChar(int pos);

    int getConsensusCharPercent(int pos);

    int getConsensusLength() const { return cache.size(); }

    void setConsensusAlgorithm(MSAConsensusAlgorithmFactory* algo);

    MSAConsensusAlgorithm* getConsensusAlgorithm() const {return algorithm;}

    QByteArray getConsensusLine(bool withGaps);
signals:
    void si_cachedItemUpdated(int pos, char c);
    void si_cacheResized(int newSize);

private slots:
    void sl_alignmentChanged();
    void sl_thresholdChanged(int newValue);
    void sl_invalidateAlignmentObject();

private:
    struct CacheItem {
        CacheItem(char c = '-', int tc = 0) : topChar(c), topPercent(tc){}
        char    topChar;
        char    topPercent;
    };


    void updateCacheItem(int pos);

    int                         curCacheSize;
    QVector<CacheItem>          cache;
    QBitArray                   updateMap;
    MultipleAlignmentObject*    aliObj;
    MSAConsensusAlgorithm*      algorithm;
};

class MaConsensusMismatchController : public QObject {
    Q_OBJECT
public:
    MaConsensusMismatchController(QObject* p,
                                  QSharedPointer<MSAEditorConsensusCache> consCache,
                                  MaEditor* editor);
    bool isMismatch(int pos) const;

private slots:
    void sl_updateItem(int pos, char c);
    void sl_resize(int newSize);

private:
    QBitArray mismatchCache;
    QSharedPointer<MSAEditorConsensusCache> consCache;
    MaEditor* editor;
};

}//namespace;

#endif
