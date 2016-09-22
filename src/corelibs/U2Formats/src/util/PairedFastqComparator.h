/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_PAIRED_FASTQ_COMPARATOR_H_
#define _U2_PAIRED_FASTQ_COMPARATOR_H_

#include <U2Core/global.h>
#include <U2Core/U2OpStatus.h>

#include <QFile>


namespace U2 {

/**
 * @brief The FastqSequenceInfo class
 */
class FastqSequenceInfo {
    friend class FastqFileIterator;
public:
    FastqSequenceInfo();
    FastqSequenceInfo(const QString& seqName, qint64 pos);

    QString getSeqName() const;

    bool operator ==(const FastqSequenceInfo& other);
    bool operator !=(const FastqSequenceInfo& other);

private:
    QString seqName;
    qint64  pos;
};

/**
 * @brief The FastqFileIterator class
 */
class FastqFileIterator {
public:
    FastqFileIterator(const QString& url, U2OpStatus& os); // automaticcaly set to the begining
    ~FastqFileIterator();

    FastqSequenceInfo getNext(U2OpStatus& os);
    QByteArray getSeqData(const FastqSequenceInfo& info, U2OpStatus& os);
    bool isEOF() const;

private:
    QFile file;
};

/**
 * @brief The PairedFastqComparator class
 */
class U2FORMATS_EXPORT PairedFastqComparator : public QObject {
public:
    PairedFastqComparator(const QString& inputFile_1, const QString& inputFile_2,
                          const QString& outputFile_1, const QString& outputFile_2);
    void compare(U2OpStatus& os);

private:
    void dropUntilItem(U2OpStatus& os, QList<FastqSequenceInfo>& list, const FastqSequenceInfo& untilItem);
    bool tryToFindPair(U2OpStatus& os, QList<FastqSequenceInfo>& initializer, const FastqSequenceInfo& info,
                       QList<FastqSequenceInfo>& searchIn);
    void tryToFindPairIInTail(U2OpStatus& os, FastqFileIterator& reads, QList<FastqSequenceInfo>& unpaired);
    void writeSeqInfo(U2OpStatus& os, FastqFileIterator& iterator, FastqSequenceInfo& info, QFile& outputFile);
    void writePair(U2OpStatus& os,
                   FastqFileIterator& iterator_1, FastqSequenceInfo& info_1, QFile& output_1,
                   FastqFileIterator& iterator_2, FastqSequenceInfo& info_2, QFile& output_2);

private:
    QString input_1;
    QString input_2;
    QString output_1;
    QString output_2;

    int pairedCounter;
    int droppedCounter;
};

} // namespace

#endif // _U2_PAIRED_FASTQ_COMPARATOR_H_

