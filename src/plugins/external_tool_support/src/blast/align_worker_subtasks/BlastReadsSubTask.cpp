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

#include "BlastReadsSubTask.h"

#include "blast/BlastAllSupportTask.h"

#include <U2Algorithm/AlignmentAlgorithmsRegistry.h>
#include <U2Algorithm/PairwiseAlignmentTask.h>

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/MAlignmentImporter.h>


namespace U2 {
namespace Workflow {

/************************************************************************/
/* BlastReadsSubTask */
/************************************************************************/
BlastReadsSubTask::BlastReadsSubTask(const QString &dbPath,
                                     const QList<SharedDbiDataHandler> &reads,
                                     const SharedDbiDataHandler &reference,
                                     DbiDataStorage *storage)
    : Task("Blast reads task", TaskFlags_NR_FOSE_COSC),
      dbPath(dbPath),
      reads(reads),
      reference(reference),
      storage(storage)
{

}

void BlastReadsSubTask::prepare() {
    foreach (const SharedDbiDataHandler &read, reads) {
        BlastAndSwReadTask* subTask = new BlastAndSwReadTask(dbPath, read, reference, storage);
        addSubTask(subTask);

        blastSubTasks << subTask;
    }
}

QList<BlastAndSwReadTask*> BlastReadsSubTask::getBlastSubtasks() const {
    return blastSubTasks;
}

/************************************************************************/
/* BlastAndSwReadTask */
/************************************************************************/
BlastAndSwReadTask::BlastAndSwReadTask(const QString &dbPath,
                                       const SharedDbiDataHandler &read,
                                       const SharedDbiDataHandler &reference,
                                       DbiDataStorage *storage)
    : Task("Blast and SW one read", TaskFlags_FOSE_COSC),
      dbPath(dbPath),
      read(read),
      reference(reference),
      offset(0),
      storage(storage),
      blastTask(NULL)
{
    blastResultDir = ExternalToolSupportUtils::createTmpDir("blast_reads", stateInfo);
}

void BlastAndSwReadTask::prepare() {
    BlastTaskSettings settings;

    settings.programName = "blastn";// >? check the alphabet
    settings.databaseNameAndPath = dbPath;
    settings.megablast = true;
    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, read));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("U2SequenceObject")), );

    initialReadName = readObject->getSequenceName();

    settings.querySequence = readObject->getWholeSequenceData(stateInfo);
    CHECK_OP(stateInfo, );
    settings.alphabet = readObject->getAlphabet();
    settings.isNucleotideSeq = settings.alphabet->isNucleic();

    settings.needCreateAnnotations = false;
    settings.groupName = "blast";

    settings.outputResFile = GUrlUtils::prepareTmpFileLocation(blastResultDir, "read_sequnece", "gb", stateInfo);
    settings.outputType = 8;

    blastTask = new BlastAllSupportTask(settings);
    addSubTask(blastTask);
}

QList<Task*> BlastAndSwReadTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> result;

    // check with ext?
    CHECK(subTask != NULL, result);
    CHECK(!subTask->hasError() && !subTask->isCanceled(), result);

    if (subTask == blastTask) {
        U2Region referenceRegion = getReferenceRegion(blastTask->getResultedAnnotations());
        CHECK(!referenceRegion.isEmpty(), result);
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

bool BlastAndSwReadTask::isComplement() const {
    //! TODO
    return false;
}

SharedDbiDataHandler BlastAndSwReadTask::getRead() const {
    return read;
}

QList<U2MsaGap> BlastAndSwReadTask::getReferenceGaps() const {
    return referenceGaps;
}

QList<U2MsaGap> BlastAndSwReadTask::getReadGaps() const {
    return readGaps;
}

QString BlastAndSwReadTask::getInitialReadName() const {
    return initialReadName;
}

U2Region BlastAndSwReadTask::getReferenceRegion(const QList<SharedAnnotationData> &blastAnnotations) {
    U2Region r;
    int maxIdentity = 0;
    foreach (const SharedAnnotationData& ann, blastAnnotations) {
        complement = ann->findFirstQualifierValue("hit_frame") != "direct";

        QString percentQualifier = ann->findFirstQualifierValue("identities");
        int annIdentity = percentQualifier.left(percentQualifier.indexOf('/')).toInt();
        if (annIdentity  > maxIdentity ) {
            maxIdentity = annIdentity;
            r = ann->getRegions().first(); // extra check the result choul
            if (ann->getRegions().size() != 1) { // should be impossible
                r = U2Region::containingRegion(ann->getRegions());
            }
        }
    }
    //TODO: extend the region on the sides

    return r;
}

void BlastAndSwReadTask::createAlignment(const U2Region& refRegion) {
    QScopedPointer<U2SequenceObject> refObject(StorageUtils::getSequenceObject(storage, reference));
    CHECK_EXT(!refObject.isNull(), setError(L10N::nullPointerError("Reference sequence")), );
    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, read));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), );

    QByteArray referenceData = refObject->getSequenceData(refRegion, stateInfo);
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
    offset = refRegion.startPos;
}

void BlastAndSwReadTask::shiftGaps(QList<U2MsaGap> &gaps) const {
    for (int i=0; i<gaps.size(); i++) {
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
