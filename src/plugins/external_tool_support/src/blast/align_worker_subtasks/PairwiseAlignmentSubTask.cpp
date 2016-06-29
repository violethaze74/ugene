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

#include "PairwiseAlignmentSubTask.h"
#include "KAlignSubTask.h"

#include <U2Algorithm/AlignmentAlgorithmsRegistry.h>
#include <U2Algorithm/PairwiseAlignmentTask.h>

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/L10n.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2SequenceUtils.h>

namespace U2 {
namespace Workflow {

namespace {
    qint64 calcMemUsageBytes(DbiDataStorage *storage, const SharedDbiDataHandler &seqId, U2OpStatus &os) {
        QScopedPointer<U2SequenceObject> object(StorageUtils::getSequenceObject(storage, seqId));
        CHECK_EXT(!object.isNull(), os.setError(L10N::nullPointerError("Sequence object")), 0);

        return object->getSequenceLength();
    }

    int toMb(qint64 bytes) {
        return 0.5 + (double(bytes) / (1024 * 1024));
    }
}

/************************************************************************/
/* PairwiseAlignmentTask */
/************************************************************************/
PairwiseAlignmentTask::PairwiseAlignmentTask(const SharedDbiDataHandler &reference,
                                             const SharedDbiDataHandler &read,
                                             DbiDataStorage *storage)
: Task("Pairwise Alignment", TaskFlags_FOSE_COSC), reference(reference), read(read), storage(storage), kalign(NULL), rKalign(NULL), cKalign(NULL), rcKalign(NULL), reverse(false), complement(false), offset(0)
{
    setMaxParallelSubtasks(2);
}

void PairwiseAlignmentTask::prepare() {
    qint64 memUsage = calcMemUsageBytes(storage, reference, stateInfo) + calcMemUsageBytes(storage, read, stateInfo);
    CHECK_OP(stateInfo, );
    if (toMb(memUsage) > 0) {
        addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, toMb(memUsage), false));
    }

    createRcReads();
    CHECK_OP(stateInfo, );

    kalign = new KAlignSubTask(reference, read, storage);
    rKalign = new KAlignSubTask(reference, rRead, storage);
    cKalign = new KAlignSubTask(reference, cRead, storage);
    rcKalign = new KAlignSubTask(reference, rcRead, storage);
    addSubTask(kalign);
    addSubTask(rKalign);
    addSubTask(cKalign);
    addSubTask(rcKalign);
}

QList<Task*> PairwiseAlignmentTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> result;
    CHECK_OP(stateInfo, result);
    // not kalign is filtered for now
    CHECK((kalign == subTask) || (rKalign == subTask) || (cKalign == subTask) || (rcKalign == subTask), result);
    CHECK(kalign->isFinished() && rKalign->isFinished() && cKalign->isFinished() && rcKalign->isFinished(), result);

    createSWAlignment(initRc());

    AbstractAlignmentTaskFactory *factory = KAlignSubTask::getAbstractAlignmentTaskFactory("Smith-Waterman", "SW_classic", stateInfo);
    CHECK_OP(stateInfo, result);

    QScopedPointer<PairwiseAlignmentTaskSettings> settings(KAlignSubTask::createSettings(storage, msa, stateInfo));
    CHECK_OP(stateInfo, result);
    settings->setCustomValue("SW_gapOpen", -10);
    settings->setCustomValue("SW_gapExtd", -1);
    settings->setCustomValue("SW_scoringMatrix", "dna");

    result << factory->getTaskInstance(settings.take());
    return result;
}

void PairwiseAlignmentTask::run() {
    CHECK_OP(stateInfo, );
    QScopedPointer<MAlignmentObject> msaObject(StorageUtils::getMsaObject(storage, msa));
    CHECK_EXT(!msaObject.isNull(), setError(L10N::nullPointerError("MSA object")), );
    int rowCount = msaObject->getNumRows();
    CHECK_EXT(2 == rowCount, setError(L10N::internalError("Wrong rows count: " + QString::number(rowCount))), );

    referenceGaps = msaObject->getRow(0).getGapModel();
    readGaps = msaObject->getRow(1).getGapModel();

    CHECK(offset > 0, );
    shiftGaps(referenceGaps);
    shiftGaps(readGaps);
    readGaps.prepend(U2MsaGap(0, offset));
}

bool PairwiseAlignmentTask::isReverse() const {
    return reverse;
}

bool PairwiseAlignmentTask::isComplement() const {
    return complement;
}

SharedDbiDataHandler PairwiseAlignmentTask::getRead() const {
    if (reverse && complement) {
        return rcKalign->getRead();
    } else if (reverse) {
        return rKalign->getRead();
    } else if (complement) {
        return cKalign->getRead();
    } else {
        return kalign->getRead();
    }
}

QList<U2MsaGap> PairwiseAlignmentTask::getReferenceGaps() const {
    return referenceGaps;
}

QList<U2MsaGap> PairwiseAlignmentTask::getReadGaps() const {
    return readGaps;
}

QString PairwiseAlignmentTask::getInitialReadName() const {
    return initialReadName;
}

QByteArray PairwiseAlignmentTask::getComplement(const QByteArray &sequence, const DNAAlphabet *alphabet) {
    DNATranslation *translator = AppContext::getDNATranslationRegistry()->lookupComplementTranslation(alphabet);
    CHECK_EXT(NULL != translator, setError(tr("Can't translate read sequence to reverse complement")), "");

    QByteArray translation(sequence.length(), 0);
    translator->translate(sequence.constData(), sequence.length(), translation.data(), translation.length());
    return translation;
}

QByteArray PairwiseAlignmentTask::getReverse(const QByteArray &sequence) const {
    QByteArray result = sequence;
    TextUtils::reverse(result.data(), result.length());
    return result;
}

QByteArray PairwiseAlignmentTask::getReverseComplement(const QByteArray &sequence, const DNAAlphabet *alphabet) {
    return getReverse(getComplement(sequence, alphabet));
}

void PairwiseAlignmentTask::createRcReads() {
    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, read));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), );

    DNASequence seq = readObject->getWholeSequence(stateInfo);
    CHECK_OP(stateInfo, );
    QByteArray sequence = seq.seq;
    initialReadName = seq.getName();

    seq.seq = getReverse(sequence);
    seq.setName(initialReadName + "_rev");
    U2EntityRef rRef = U2SequenceUtils::import(storage->getDbiRef(), seq, stateInfo);
    CHECK_OP(stateInfo, );
    rRead = storage->getDataHandler(rRef);

    seq.seq = getComplement(sequence, readObject->getAlphabet());
    seq.setName(initialReadName + "_compl");
    U2EntityRef cRef = U2SequenceUtils::import(storage->getDbiRef(), seq, stateInfo);
    CHECK_OP(stateInfo, );
    cRead = storage->getDataHandler(cRef);

    seq.seq = getReverseComplement(sequence, readObject->getAlphabet());
    seq.setName(initialReadName + "_rev_compl");
    U2EntityRef rcRef = U2SequenceUtils::import(storage->getDbiRef(), seq, stateInfo);
    CHECK_OP(stateInfo, );
    rcRead = storage->getDataHandler(rcRef);
}

KAlignSubTask * PairwiseAlignmentTask::initRc() {
    QList<qint64> values;
    values << kalign->getMaxRegionSize();
    values << rKalign->getMaxRegionSize();
    values << cKalign->getMaxRegionSize();
    values << rcKalign->getMaxRegionSize();
    qSort(values);
    qint64 max = values.last();

    if (kalign->getMaxRegionSize() == max) {
        reverse = false;
        complement = false;
    } else if (rKalign->getMaxRegionSize() == max) {
        reverse = true;
        complement = false;
    } else if (cKalign->getMaxRegionSize() == max) {
        reverse = false;
        complement = true;
    } else {
        assert(rcKalign->getMaxRegionSize() == max);
        reverse = true;
        complement = true;
    }

    if (reverse && complement) {
        return rcKalign;
    } else if (reverse) {
        return rKalign;
    } else if (complement) {
        return cKalign;
    } else {
        return kalign;
    }
}

void PairwiseAlignmentTask::createSWAlignment(KAlignSubTask *task) {
    QScopedPointer<U2SequenceObject> refObject(StorageUtils::getSequenceObject(storage, reference));
    CHECK_EXT(!refObject.isNull(), setError(L10N::nullPointerError("Reference sequence")), );
    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, task->getRead()));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), );

    QByteArray referenceData = refObject->getSequenceData(task->getCoreRegion(), stateInfo);
    CHECK_OP(stateInfo, );

    MAlignment alignment("msa", refObject->getAlphabet());
    alignment.addRow(refObject->getSequenceName(), referenceData, stateInfo);
    CHECK_OP(stateInfo, );
    QByteArray readData = readObject->getWholeSequenceData(stateInfo);
    CHECK_OP(stateInfo, );
    alignment.addRow(readObject->getSequenceName(), readData, stateInfo);
    CHECK_OP(stateInfo, );

    QScopedPointer<MAlignmentObject> msaObj(MAlignmentImporter::createAlignment(storage->getDbiRef(), alignment, stateInfo));
    CHECK_OP(stateInfo, );
    msa = storage->getDataHandler(msaObj->getEntityRef());
    offset = task->getCoreRegion().startPos;
}

void PairwiseAlignmentTask::shiftGaps(QList<U2MsaGap> &gaps) const {
    for (int i=0; i<gaps.size(); i++) {
        gaps[i].offset += offset;
    }
}

} // namespace Workflow
} // namespace U2
