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

FastqFileIterator::FastqFileIterator(const QString &url, U2OpStatus& os)
    : file(url)
{
    bool ok = file.open(QIODevice::ReadOnly);
    if (!ok) {
        os.setError(PairedFastqComparator::tr("Cannot open the file '%1'.").arg(url));
    }
}

FastqFileIterator::~FastqFileIterator() {
    file.close();
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

QByteArray FastqFileIterator::getSeqData(const FastqSequenceInfo &info, U2OpStatus& os) {
    qint64 currentPos = file.pos();
    QByteArray seqData("@");

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
    : input_1(inputFile_1),
      input_2(inputFile_2),
      output_1(outputFile_1),
      output_2(outputFile_2),
      pairedCounter(0),
      droppedCounter(0) {

}

void PairedFastqComparator::compare(U2OpStatus &os) {

    FastqFileIterator reads_1(input_1, os);
    CHECK_OP(os, );
    FastqFileIterator reads_2(input_2, os);
    CHECK_OP(os, );

    QList<FastqSequenceInfo> unpaired_1;
    QList<FastqSequenceInfo> unpaired_2;

    while (!reads_1.isEOF() && !reads_2.isEOF() && os.isCoR()) {

        FastqSequenceInfo seqInfo_1 = reads_1.getNext(os);
        FastqSequenceInfo seqInfo_2 = reads_2.getNext(os);

        if (seqInfo_1 == seqInfo_2) {
            pairedCounter++;

            droppedCounter += unpaired_1.size();
            droppedCounter += unpaired_2.size();

            unpaired_1.clear();
            unpaired_2.clear();
            continue;
        }

        if (tryToFindPair(os, unpaired_1, seqInfo_1, unpaired_2)) {
            pairedCounter++;
            unpaired_2 << seqInfo_2;
            continue;
        }
        CHECK_OP(os, );

        if (tryToFindPair(os, unpaired_2, seqInfo_2, unpaired_1)) {
            pairedCounter++;
            unpaired_1 << seqInfo_1;
            continue;
        }
        CHECK_OP(os, );

        unpaired_1 << seqInfo_1;
        unpaired_2 << seqInfo_2;
    }

    // for correct counters info
    tryToFindPairIInTail(os, reads_1, unpaired_2);
    CHECK_OP(os, );
    tryToFindPairIInTail(os, reads_2, unpaired_1);
    CHECK_OP(os, );
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

bool PairedFastqComparator::tryToFindPair(U2OpStatus& os, QList<FastqSequenceInfo>& initializer, const FastqSequenceInfo& info,
                   QList<FastqSequenceInfo>& searchIn) {
    if (searchIn.contains(info)) {
        droppedCounter += initializer.size();
        initializer.clear();

        dropUntilItem(os, searchIn, info);
        return true;
    }
    return false;
}

void PairedFastqComparator::tryToFindPairIInTail(U2OpStatus& os, FastqFileIterator& reads, QList<FastqSequenceInfo>& unpaired) {
    QList<FastqSequenceInfo> emptyList;
    while (!reads.isEOF()) {
        FastqSequenceInfo seqInfo_1 = reads.getNext(os);
        if (!tryToFindPair(os, emptyList, seqInfo_1, unpaired)) {
            droppedCounter++;
        } else {
            pairedCounter++;
        }
    }
}

void PairedFastqComparator::writeSeqInfo(U2OpStatus& os, FastqFileIterator& iterator, FastqSequenceInfo& info, QFile& outputFile) {
    const QByteArray sequenceData = iterator.getSeqData(info, os);
    CHECK_OP(os, );
    outputFile.write(sequenceData);
}

void PairedFastqComparator::writePair(U2OpStatus &os,
                                      FastqFileIterator &iterator_1, FastqSequenceInfo &info_1, QFile &output_1,
                                      FastqFileIterator &iterator_2, FastqSequenceInfo &info_2, QFile &output_2) {
    writeSeqInfo(os, iterator_1, info_1, output_1);
    writeSeqInfo(os, iterator_2, info_2, output_2);
    pairedCounter++;
}

} // namespace
