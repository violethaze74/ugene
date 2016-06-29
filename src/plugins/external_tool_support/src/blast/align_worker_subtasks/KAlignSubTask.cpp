#include "KAlignSubTask.h"

#include <U2Algorithm/AlignmentAlgorithmsRegistry.h>
#include <U2Algorithm/PairwiseAlignmentTask.h>

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/L10n.h>
#include <U2Core/MAlignmentImporter.h>

namespace U2 {
namespace Workflow {

//! DOUBLE
//! but this file will be remove anyway so whatever
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
/* KAlignSubTask */
/************************************************************************/
const qint64 KAlignSubTask::MAX_GAP_SIZE = 10;
const qint64 KAlignSubTask::EXTENSION_COEF = 2;

KAlignSubTask::KAlignSubTask(const SharedDbiDataHandler &reference, const SharedDbiDataHandler &read, DbiDataStorage *storage)
: Task("KAlign task wrapper", TaskFlags_FOSE_COSC), reference(reference), read(read), storage(storage), maxRegionSize(0)
{

}

const SharedDbiDataHandler KAlignSubTask::getRead() const {
    return read;
}

qint64 KAlignSubTask::getMaxRegionSize() const {
    return maxRegionSize;
}

U2Region KAlignSubTask::getCoreRegion() const {
    return coreRegion;
}

void KAlignSubTask::prepare() {
    qint64 memUsage = calcMemUsageBytes(storage, reference, stateInfo) + calcMemUsageBytes(storage, read, stateInfo);
    CHECK_OP(stateInfo, );
    if (toMb(memUsage) > 0) {
        addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, toMb(memUsage), false));
    }

    createAlignment();
    CHECK_OP(stateInfo, );

    AbstractAlignmentTaskFactory *factory = getAbstractAlignmentTaskFactory("Hirschberg (KAlign)", "KAlign", stateInfo);
    CHECK_OP(stateInfo, );

    QScopedPointer<PairwiseAlignmentTaskSettings> settings(createSettings(storage, msa, stateInfo));
    CHECK_OP(stateInfo, );
    addSubTask(factory->getTaskInstance(settings.take()));
}

void KAlignSubTask::run() {
    QScopedPointer<MAlignmentObject> msaObject(StorageUtils::getMsaObject(storage, msa));
    CHECK_EXT(!msaObject.isNull(), setError(L10N::nullPointerError("MSA object")), );
    int rowCount = msaObject->getNumRows();
    CHECK_EXT(2 == rowCount, setError(L10N::internalError("Wrong rows count: " + QString::number(rowCount))), );

    MAlignmentRow readRow = msaObject->getRow(1);
    QList<U2Region> regions = getRegions(readRow.getGapModel(), readRow.getRowLengthWithoutTrailing());
    calculateCoreRegion(regions);
    extendCoreRegion(regions);

    algoLog.details(tr("Max region size: %1").arg(maxRegionSize));
    algoLog.details(tr("Core region: %1-%2").arg(coreRegion.startPos).arg(coreRegion.endPos()-1));
}

QList<U2Region> KAlignSubTask::getRegions(const QList<U2MsaGap> &gaps, qint64 rowLength) const {
    QList<U2Region> regions;
    qint64 startPos = 0;
    foreach (const U2MsaGap &gap, gaps) {
        qint64 length = gap.offset - startPos;
        if (length > 0) {
            regions << U2Region(startPos, length);
        }
        startPos = gap.offset + gap.gap;
    }
    if (startPos < rowLength) {
        qint64 length = rowLength - startPos;
        regions << U2Region(startPos, length);
    }
    return regions;
}

void KAlignSubTask::calculateCoreRegion(const QList<U2Region> &regions) {
    coreRegion = U2Region(0, 0);
    maxRegionSize = 0;
    U2Region currentRegion = coreRegion;
    foreach (const U2Region &region, regions) {
        if (maxRegionSize < region.length) {
            maxRegionSize = region.length;
        }
        if (0 == currentRegion.length) {
            currentRegion = region;
            continue;
        }
        if (region.startPos - currentRegion.endPos() < MAX_GAP_SIZE) {
            currentRegion.length = region.endPos() - currentRegion.startPos;
        } else {
            if (coreRegion.length < currentRegion.length) {
                coreRegion = currentRegion;
            }
            currentRegion = region;
        }
    }
    if (coreRegion.length < currentRegion.length) {
        coreRegion = currentRegion;
    }
}

void KAlignSubTask::extendCoreRegion(const QList<U2Region> &regions) {
    qint64 leftSize = 0;
    qint64 rightSize = 0;
    foreach (const U2Region &region, regions) {
        if (region.endPos() - 1 < coreRegion.startPos) {
            leftSize += region.length;
        }
        if (region.startPos > coreRegion.endPos() - 1) {
            rightSize += region.length;
        }
    }
    coreRegion.startPos -= leftSize * EXTENSION_COEF;
    coreRegion.length += (leftSize + rightSize) * EXTENSION_COEF;
}

AbstractAlignmentTaskFactory * KAlignSubTask::getAbstractAlignmentTaskFactory(const QString &algoId, const QString &implId, U2OpStatus &os) {
    AlignmentAlgorithm *algo = AppContext::getAlignmentAlgorithmsRegistry()->getAlgorithm(algoId);
    CHECK_EXT(NULL != algo, os.setError(tr("The %1 algorithm is not found. Add the %1 plugin.").arg(algoId)), NULL);

    AlgorithmRealization *algoImpl = algo->getAlgorithmRealization(implId);
    CHECK_EXT(NULL != algoImpl, os.setError(tr("The %1 algorithm is not found. Check that the %1 plugin is up to date.").arg(algoId)), NULL);

    return algoImpl->getTaskFactory();
}

void KAlignSubTask::createAlignment() {
    QScopedPointer<U2SequenceObject> refObject(StorageUtils::getSequenceObject(storage, reference));
    CHECK_EXT(!refObject.isNull(), setError(L10N::nullPointerError("Reference sequence")), );
    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, read));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), );

    MAlignment alignment("msa", refObject->getAlphabet());
    QByteArray refData = refObject->getWholeSequenceData(stateInfo);
    CHECK_OP(stateInfo, );
    alignment.addRow(refObject->getSequenceName(), refData, stateInfo);
    CHECK_OP(stateInfo, );
    QByteArray readData = readObject->getWholeSequenceData(stateInfo);
    CHECK_OP(stateInfo, );
    alignment.addRow(readObject->getSequenceName(), readData, stateInfo);
    CHECK_OP(stateInfo, );

    QScopedPointer<MAlignmentObject> msaObj(MAlignmentImporter::createAlignment(storage->getDbiRef(), alignment, stateInfo));
    CHECK_OP(stateInfo, );
    msa = storage->getDataHandler(msaObj->getEntityRef());
}

PairwiseAlignmentTaskSettings * KAlignSubTask::createSettings(DbiDataStorage *storage, const SharedDbiDataHandler &msa, U2OpStatus &os) {
    QScopedPointer<MAlignmentObject> msaObject(StorageUtils::getMsaObject(storage, msa));
    CHECK_EXT(!msaObject.isNull(), os.setError(L10N::nullPointerError("MSA object")), NULL);

    U2DataId referenceId = msaObject->getRow(0).getRowDBInfo().sequenceId;
    U2DataId readId = msaObject->getRow(1).getRowDBInfo().sequenceId;

    PairwiseAlignmentTaskSettings *settings = new PairwiseAlignmentTaskSettings();
    settings->alphabet = msaObject->getAlphabet()->getId();
    settings->inNewWindow = false;
    settings->msaRef = msaObject->getEntityRef();
    settings->firstSequenceRef = U2EntityRef(msaObject->getEntityRef().dbiRef, referenceId);
    settings->secondSequenceRef = U2EntityRef(msaObject->getEntityRef().dbiRef, readId);
    return settings;
}

} // namespace Workflow
} // namespace U2
