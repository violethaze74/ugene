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

#include <U2Core/DNAAlphabet.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>


namespace U2 {
namespace Workflow {

BlastReadsSubTask::BlastReadsSubTask(const QString &dbPath,
                                     const QList<SharedDbiDataHandler> &reads,
                                     DbiDataStorage *storage)
    : Task("Blast reads task", TaskFlags_NR_FOSE_COSC),
      dbPath(dbPath),
      reads(reads),
      storage(storage)
{

}

void BlastReadsSubTask::prepare() {
    foreach (const SharedDbiDataHandler &read, reads) {
        BlastAndSwReadTask* subTask = new BlastAndSwReadTask(dbPath, read, storage);
        addSubTask(subTask);
    }
}

BlastAndSwReadTask::BlastAndSwReadTask(const QString &dbPath,
                                       const SharedDbiDataHandler &read,
                                       DbiDataStorage *storage)
    : Task("Blast and SW one read", TaskFlags_FOSE_COSC),
      dbPath(dbPath),
      read(read),
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
        QList<SharedAnnotationData> blastAnnotations = blastTask->getResultedAnnotations();
        //! create an alignment for SmithWaterman
    }

    return result;
}

void BlastAndSwReadTask::run() {

}

QByteArray BlastAndSwReadTask::getRead() {
    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, read));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("U2SequenceObject")), QByteArray());

    return readObject->getWholeSequenceData(stateInfo);
}

} // namespace Workflow
} // namespace U2
