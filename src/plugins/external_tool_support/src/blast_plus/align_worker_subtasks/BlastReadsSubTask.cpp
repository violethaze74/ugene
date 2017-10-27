/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
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

#include "BlastReadsSubTask.h"

#include "blast_plus/BlastNPlusSupportTask.h"

#include <U2Algorithm/AlignmentAlgorithmsRegistry.h>
#include <U2Algorithm/PairwiseAlignmentTask.h>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/MultipleSequenceAlignmentImporter.h>


namespace U2 {
namespace Workflow {

/************************************************************************/
/* BlastReadsSubTask */
/************************************************************************/
BlastReadsSubTask::BlastReadsSubTask(const QString &dbPath,
                                     const QList<SharedDbiDataHandler> &reads,
                                     const SharedDbiDataHandler &reference,
                                     const int minIdentityPercent,
                                     DbiDataStorage *storage)
    : Task(tr("Align reads with BLAST & SW task"), TaskFlags_NR_FOSE_COSC),
      dbPath(dbPath),
      reads(reads),
      reference(reference),
      minIdentityPercent(minIdentityPercent),
      storage(storage)
{
    setMaxParallelSubtasks(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
}

void BlastReadsSubTask::prepare() {
    foreach (const SharedDbiDataHandler &read, reads) {
        BlastAndSwReadTask* subTask = new BlastAndSwReadTask(dbPath, read, reference, minIdentityPercent, storage);
        addSubTask(subTask);

        blastSubTasks << subTask;
    }
}

const QList<BlastAndSwReadTask*>& BlastReadsSubTask::getBlastSubtasks() const {
    return blastSubTasks;
}

/************************************************************************/
/* BlastAndSwReadTask */
/************************************************************************/
BlastAndSwReadTask::BlastAndSwReadTask(const QString &dbPath,
                                       const SharedDbiDataHandler &read,
                                       const SharedDbiDataHandler &reference,
                                       const int minIdentityPercent,
                                       DbiDataStorage *storage)
    : Task(tr("Align one read with BLAST & SW task"), TaskFlags_FOSE_COSC),
      dbPath(dbPath),
      read(read),
      reference(reference),
      minIdentityPercent(minIdentityPercent),
      readIdentity(0),
      offset(0),
      readShift(0),
      storage(storage),
      blastTask(NULL),
      complement(false),
      skipped(false)
{
    blastResultDir = ExternalToolSupportUtils::createTmpDir("blast_reads", stateInfo);

    QScopedPointer<U2SequenceObject> refObject(StorageUtils::getSequenceObject(storage, reference));
    referenceLength = refObject->getSequenceLength();
}

void BlastAndSwReadTask::prepare() {
    BlastTaskSettings settings;

    settings.programName = "blastn";
    settings.databaseNameAndPath = dbPath;
    settings.megablast = true;
    settings.wordSize = 28;
    settings.xDropoffGA = 20;
    settings.xDropoffUnGA = 10;
    settings.xDropoffFGA = 100;
    settings.numberOfProcessors = AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount();
    settings.numberOfHits = 100;
    settings.gapOpenCost = 2;
    settings.gapExtendCost = 2;

    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, read));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("U2SequenceObject")), );

    initialReadName = readObject->getSequenceName();

    settings.querySequence = readObject->getWholeSequenceData(stateInfo);
    CHECK_OP(stateInfo, );
    settings.alphabet = readObject->getAlphabet();
    settings.isNucleotideSeq = settings.alphabet->isNucleic();

    settings.needCreateAnnotations = false;
    settings.groupName = "blast";

    settings.outputResFile = GUrlUtils::prepareTmpFileLocation(blastResultDir, "read_sequence", "gb", stateInfo);
    settings.outputType = 5;

    blastTask = new BlastNPlusSupportTask(settings);
    addSubTask(blastTask);
}

QList<Task*> BlastAndSwReadTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> result;
    CHECK(subTask != NULL, result);
    CHECK(!subTask->hasError() && !subTask->isCanceled(), result);

    if (subTask == blastTask) {
        U2Region referenceRegion = getReferenceRegion(blastTask->getResultedAnnotations());
        if (referenceRegion.isEmpty()) {
            skipped = true;
            taskLog.info(tr("%1 was skipped. Low identity.").arg(getReadName()));
            return result;
        }
        createAlignment(referenceRegion);

        AbstractAlignmentTaskFactory *factory = getAbstractAlignmentTaskFactory("Smith-Waterman", "SW_classic", stateInfo);
        CHECK_OP(stateInfo, result);

        QScopedPointer<PairwiseAlignmentTaskSettings> settings(createSettings(storage, msa, stateInfo));
        CHECK_OP(stateInfo, result);
        settings->setCustomValue("SW_gapOpen", -10);
        settings->setCustomValue("SW_gapExtd", -1);
        settings->setCustomValue("SW_scoringMatrix", "dna");

        result << factory->getTaskInstance(settings.take());
    }
    return result;
}

void BlastAndSwReadTask::run() {
    CHECK_OP(stateInfo, );
    CHECK(!skipped, );

    QScopedPointer<MultipleSequenceAlignmentObject> msaObject(StorageUtils::getMsaObject(storage, msa));
    CHECK_EXT(!msaObject.isNull(), setError(L10N::nullPointerError("MSA object for %1").arg(getReadName())), );
    int rowCount = msaObject->getNumRows();
    CHECK_EXT(2 == rowCount, setError(L10N::internalError("Wrong rows count: " + QString::number(rowCount))), );

    referenceGaps = msaObject->getMsaRow(0)->getGapModel();
    readGaps = msaObject->getMsaRow(1)->getGapModel();

    CHECK(offset > 0, );
    shiftGaps(referenceGaps);
    MsaRowUtils::addOffsetToGapModel(readGaps, offset);
}

bool BlastAndSwReadTask::isComplement() const {
    return complement;
}

const SharedDbiDataHandler& BlastAndSwReadTask::getRead() const {
    return read;
}

const U2MsaRowGapModel& BlastAndSwReadTask::getReferenceGaps() const {
    return referenceGaps;
}

const U2MsaRowGapModel& BlastAndSwReadTask::getReadGaps() const {
    return readGaps;
}

bool BlastAndSwReadTask::isReadAligned() const {
    return !skipped;
}

QString BlastAndSwReadTask::getReadName() const {
    return initialReadName;
}

MultipleSequenceAlignment BlastAndSwReadTask::getMAlignment() {
    QScopedPointer<MultipleSequenceAlignmentObject> msaObj(StorageUtils::getMsaObject(storage, msa));
    CHECK(msaObj != NULL, MultipleSequenceAlignment());

    return msaObj->getMultipleAlignment();
}

qint64 BlastAndSwReadTask::getOffset() const {
    return offset;
}

int BlastAndSwReadTask::getReadIdentity() const {
    return readIdentity;
}

U2Region BlastAndSwReadTask::getReferenceRegion(const QList<SharedAnnotationData> &blastAnnotations) {
    U2Region refRegion;
    U2Region blastReadRegion;
    int maxIdentity = 0;
    foreach (const SharedAnnotationData& ann, blastAnnotations) {
        QString percentQualifier = ann->findFirstQualifierValue("identities");
        int annIdentity = percentQualifier.left(percentQualifier.indexOf('/')).toInt();
        if (annIdentity  > maxIdentity ) {
            // identity
            maxIdentity = annIdentity;

            // annotation region on read
            blastReadRegion = ann->getRegions().first();

            // region on reference
            qint64 hitFrom = ann->findFirstQualifierValue("hit-from").toInt();
            qint64 hitTo = ann->findFirstQualifierValue("hit-to").toInt();
            qint64 leftMost = qMin(hitFrom, hitTo);
            qint64 rightMost = qMax(hitFrom, hitTo);
            refRegion = U2Region(leftMost - 1, rightMost - leftMost);

            // frame
            QString frame = ann->findFirstQualifierValue("source_frame");
            complement = (frame == "complement");
        }
    }
    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, read));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), U2Region());
    qint64 readLen = readObject->getSequenceLength();

    readIdentity = 100 * maxIdentity / readLen;
    CHECK(readIdentity >= minIdentityPercent, U2Region());

    qint64 undefinedLen = readLen - maxIdentity;
    readShift = undefinedLen - blastReadRegion.startPos;

    // extend ref region to the read
    refRegion.startPos = qMax((qint64)0, (qint64)(refRegion.startPos - undefinedLen));
    refRegion.length = qMin(referenceLength - refRegion.startPos, (qint64)(blastReadRegion.length + 2 * undefinedLen));

    return refRegion;
}

void BlastAndSwReadTask::createAlignment(const U2Region& refRegion) {
    QScopedPointer<U2SequenceObject> refObject(StorageUtils::getSequenceObject(storage, reference));
    CHECK_EXT(!refObject.isNull(), setError(L10N::nullPointerError("Reference sequence")), );
    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, read));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), );

    QByteArray referenceData = refObject->getSequenceData(refRegion, stateInfo);
    CHECK_OP(stateInfo, );

    MultipleSequenceAlignment alignment("msa", refObject->getAlphabet());
    alignment->addRow(refObject->getSequenceName(), referenceData);
    CHECK_OP(stateInfo, );
    QByteArray readData = readObject->getWholeSequenceData(stateInfo);
    CHECK_OP(stateInfo, );

    if (readShift != 0) {
        alignment->addRow(readObject->getSequenceName(),
                         complement ? DNASequenceUtils::reverseComplement(readData) : readData, U2MsaRowGapModel() << U2MsaGap(0, readShift), stateInfo);
    } else {
        alignment->addRow(readObject->getSequenceName(), complement ? DNASequenceUtils::reverseComplement(readData) : readData);
    }

    CHECK_OP(stateInfo, );

    QScopedPointer<MultipleSequenceAlignmentObject> msaObj(MultipleSequenceAlignmentImporter::createAlignment(storage->getDbiRef(), alignment, stateInfo));
    CHECK_OP(stateInfo, );
    msa = storage->getDataHandler(msaObj->getEntityRef());
    offset = refRegion.startPos;
}

void BlastAndSwReadTask::shiftGaps(U2MsaRowGapModel &gaps) const {
    for (int i = 0; i < gaps.size(); i++) {
        gaps[i].offset += offset;
    }
}

AbstractAlignmentTaskFactory* BlastAndSwReadTask::getAbstractAlignmentTaskFactory(const QString &algoId, const QString &implId, U2OpStatus &os) {
    AlignmentAlgorithm *algo = AppContext::getAlignmentAlgorithmsRegistry()->getAlgorithm(algoId);
    CHECK_EXT(NULL != algo, os.setError(BlastAndSwReadTask::tr("The %1 algorithm is not found. Add the %1 plugin.").arg(algoId)), NULL);

    AlgorithmRealization *algoImpl = algo->getAlgorithmRealization(implId);
    CHECK_EXT(NULL != algoImpl, os.setError(BlastAndSwReadTask::tr("The %1 algorithm is not found. Check that the %1 plugin is up to date.").arg(algoId)), NULL);

    return algoImpl->getTaskFactory();
}

PairwiseAlignmentTaskSettings* BlastAndSwReadTask::createSettings(DbiDataStorage *storage, const SharedDbiDataHandler &msa, U2OpStatus &os) {
    QScopedPointer<MultipleSequenceAlignmentObject> msaObject(StorageUtils::getMsaObject(storage, msa));
    CHECK_EXT(!msaObject.isNull(), os.setError(L10N::nullPointerError("MSA object")), NULL);

    U2DataId referenceId = msaObject->getMsaRow(0)->getRowDbInfo().sequenceId;
    U2DataId readId = msaObject->getMsaRow(1)->getRowDbInfo().sequenceId;

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
