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

#include "PairedFastqComparator.h"

#include <U2Core/U2SafePoints.h>


namespace U2 {


FastqSequenceInfo::FastqSequenceInfo()
    : pos(-1) {

}

FastqSequenceInfo::FastqSequenceInfo(const QString& seqName, qint64 pos)
    : seqName(seqName),
      pos(pos) {

}

bool FastqSequenceInfo::isValid() const {
    return !seqName.isEmpty() && pos != -1;
}

QString FastqSequenceInfo::getSeqName() const {
    return seqName;
}

bool FastqSequenceInfo::operator ==(const FastqSequenceInfo& other) {
    return seqName == other.seqName;
}

bool FastqSequenceInfo::operator !=(const FastqSequenceInfo& other) {
    return seqName != other.seqName;
}

//--------------------------------------------------
const int READ_BUFF_SIZE = 1024; // it should be from the document format

FastqFileIterator::FastqFileIterator(const QString &url)
    : file(url)
{

}

FastqFileIterator::~FastqFileIterator() {
    file.close();
}

void FastqFileIterator::open(U2OpStatus &os) {
    bool ok = file.open(QIODevice::ReadOnly);
    if (!ok) {
        os.setError(PairedFastqComparator::tr("Cannot open the file '%1'.").arg(file.fileName()));
    }
}

FastqSequenceInfo FastqFileIterator::getNext(U2OpStatus& os) {
    SAFE_POINT_EXT(!isEOF(), os.setError(PairedFastqComparator::tr("The end of file")), FastqSequenceInfo());

    QByteArray buffArray(READ_BUFF_SIZE + 1, 0);
    char* buff = buffArray.data();

    qint64 currentPos = file.pos();
    file.readLine(buff, READ_BUFF_SIZE);
    SAFE_POINT_EXT(buffArray.at(0) == '@',
                   os.setError(PairedFastqComparator::tr("Invalid iterator posision. Cannot detect a sequence.")),
                   FastqSequenceInfo());
    QString seqName(buffArray);

    // skip sequence content and quality
    bool stop = false;
    while (!stop && !isEOF()) {
        qint64 pos = file.pos();
        file.readLine(buff, READ_BUFF_SIZE);
        if (buffArray.startsWith('@')) {
            file.seek(pos);
            stop = true;
        }
    }

    return FastqSequenceInfo(seqName, currentPos);
}

QString FastqFileIterator::getSeqData(const FastqSequenceInfo &info, U2OpStatus& os) {
    qint64 currentPos = file.pos();
    QString seqData("@");

    SAFE_POINT_EXT(file.seek(info.pos),
                   os.setError(PairedFastqComparator::tr("Refering to the wrong position in the file '%1'").arg(file.fileName())),
                   QByteArray());

    bool stop = false;
    QByteArray buffArray(READ_BUFF_SIZE + 1, 0);
    char* buff = buffArray.data();
    file.getChar(buff);
    while (!stop && !isEOF()) {
        file.readLine(buff, READ_BUFF_SIZE);
        if (buffArray.startsWith('@')) {
            stop = true;
        } else {
            seqData.append(buffArray);
        }
    }

    file.seek(currentPos);
    return seqData;
}

bool FastqFileIterator::isEOF() const {
    return file.atEnd();
}

PairedFastqComparator::PairedFastqComparator(const QString &inputFile_1, const QString &inputFile_2,
                                             const QString &outputFile_1, const QString &outputFile_2)
    : inputReads_1(inputFile_1),
      inputReads_2(inputFile_2),
      outputFile_1(outputFile_1),
      outputFile_2(outputFile_2),
      pairedCounter(0),
      droppedCounter(0) {

}

void PairedFastqComparator::compare(U2OpStatus &os) {

    inputReads_1.open(os);
    CHECK_OP(os, );
    inputReads_2.open(os);
    CHECK_OP(os, );

    bool ok = outputFile_1.open(QIODevice::WriteOnly);
    CHECK_EXT(ok, os.setError(tr("Cannot open the file %1").arg(outputFile_1.fileName())), );
    ok = outputFile_2.open(QIODevice::WriteOnly);
    CHECK_EXT(ok, os.setError(tr("Cannot open the file %1").arg(outputFile_2.fileName())), );

    QList<FastqSequenceInfo> unpaired_1;
    QList<FastqSequenceInfo> unpaired_2;

    FastqSequenceInfo tmp;
    while (!inputReads_1.isEOF() && !inputReads_2.isEOF() && !os.isCoR()) {

        FastqSequenceInfo seqInfo_1 = inputReads_1.getNext(os);
        FastqSequenceInfo seqInfo_2 = inputReads_2.getNext(os);

        if (seqInfo_1 == seqInfo_2) {
            writePair(os, seqInfo_1, seqInfo_2);

            droppedCounter += unpaired_1.size();
            droppedCounter += unpaired_2.size();

            unpaired_1.clear();
            unpaired_2.clear();
            continue;
        }

        if ((tmp = tryToFindPair(os, unpaired_1, seqInfo_1, unpaired_2)).isValid() && !os.isCoR()) {
            writePair(os, seqInfo_1, tmp);
            unpaired_2 << seqInfo_2;
            continue;
        }
        CHECK_OP(os, );

        if ((tmp = tryToFindPair(os, unpaired_2, seqInfo_2, unpaired_1)).isValid() && !os.isCoR()) {
            unpaired_1 << seqInfo_1;
            continue;
        }
        CHECK_OP(os, );

        unpaired_1 << seqInfo_1;
        unpaired_2 << seqInfo_2;
    }
    CHECK_OP(os, );

    // for correct counters info
    tryToFindPairIInTail(os, inputReads_1, unpaired_2, true);
    CHECK_OP(os, );
    tryToFindPairIInTail(os, inputReads_2, unpaired_1, false);
    CHECK_OP(os, );

    outputFile_1.close();
    outputFile_2.close();
}

void PairedFastqComparator::dropUntilItem(U2OpStatus& os, QList<FastqSequenceInfo>& list, const FastqSequenceInfo& untilItem) {
    CHECK(!list.isEmpty(), );
    SAFE_POINT_EXT(list.contains(untilItem), os.setError(tr("The list doesn't contains the item")), ); // no TR?

    FastqSequenceInfo info;
    do {
        info = list.takeFirst();
        droppedCounter++;
    } while (info != untilItem && !list.isEmpty());
    droppedCounter--; // the sequence that is in the pair was count
}

FastqSequenceInfo PairedFastqComparator::tryToFindPair(U2OpStatus& os,
                                                       QList<FastqSequenceInfo>& initializer, const FastqSequenceInfo& info,
                                                       QList<FastqSequenceInfo>& searchIn) {
    int index = searchIn.indexOf(info);
    if (index != -1) {
        FastqSequenceInfo result= searchIn.at(index);
        droppedCounter += initializer.size();
        initializer.clear();

        dropUntilItem(os, searchIn, info);
        return result;
    }
    return FastqSequenceInfo();
}

void PairedFastqComparator::tryToFindPairIInTail(U2OpStatus& os, FastqFileIterator& reads,
                                                 QList<FastqSequenceInfo>& unpaired, bool iteratorContentIsFirst) {
    QList<FastqSequenceInfo> emptyList;
    while (!reads.isEOF()) {
        FastqSequenceInfo seqInfo_1 = reads.getNext(os);
        FastqSequenceInfo seqInfo_2 = tryToFindPair(os, emptyList, seqInfo_1, unpaired); // here we cannot get
        if (!seqInfo_2.isValid()) {
            droppedCounter++;
        } else {
            if (iteratorContentIsFirst) {
                writePair(os, seqInfo_1, seqInfo_2);
            } else {
                writePair(os, seqInfo_2, seqInfo_1);
            }
        }
    }
}

void writeSeq(U2OpStatus& os, FastqFileIterator& iterator, FastqSequenceInfo &info, QFile& file) {
    const QString sequenceData = iterator.getSeqData(info, os);
    CHECK_OP(os, );
    file.write(sequenceData.toLatin1());
}

void PairedFastqComparator::writePair(U2OpStatus &os, FastqSequenceInfo &info_1, FastqSequenceInfo &info_2) {
    writeSeq(os, inputReads_1, info_1, outputFile_1);
    CHECK_OP(os, );

    writeSeq(os, inputReads_2, info_2, outputFile_2);
    CHECK_OP(os, );

    pairedCounter++;
}

} // namespace
