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

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/U2SafePoints.h>

#include "PrepareReferenceSequenceTask.h"
#include "RemoveGapsFromSequenceTask.h"

namespace U2 {

PrepareReferenceSequenceTask::PrepareReferenceSequenceTask(const QString &referenceUrl, const U2DbiRef &dstDbiRef)
    : DocumentProviderTask(tr("Prepare reference sequence"), TaskFlags_NR_FOSE_COSC),
      referenceUrl(referenceUrl),
      dstDbiRef(dstDbiRef),
      loadTask(NULL),
      removeGapsTask(NULL)
{
    SAFE_POINT_EXT(!referenceUrl.isEmpty(), setError("Reference URL is empty"), );
    SAFE_POINT_EXT(dstDbiRef.isValid(), setError("Destination DBI reference is not valid"), );
}

const U2EntityRef &PrepareReferenceSequenceTask::getReferenceEntityRef() const {
    return referenceEntityRef;
}

void PrepareReferenceSequenceTask::prepare() {
    QVariantMap hints;
    hints[DocumentFormat::DBI_REF_HINT] = QVariant::fromValue<U2DbiRef>(dstDbiRef);
    loadTask = LoadDocumentTask::getDefaultLoadDocTask(stateInfo, referenceUrl, hints);
    CHECK_OP(stateInfo, );
    addSubTask(loadTask);
}

QList<Task *> PrepareReferenceSequenceTask::onSubTaskFinished(Task *subTask) {
    QList<Task *> newSubTasks;
    CHECK_OP(stateInfo, newSubTasks);

    if (loadTask == subTask) {
        Document * const document = loadTask->getDocument(false);
        SAFE_POINT(NULL != document, "Document is NULL", newSubTasks);

        document->setDocumentOwnsDbiResources(false);

        QList<GObject *> objects = document->findGObjectByType(GObjectTypes::SEQUENCE);
        CHECK_EXT(!objects.isEmpty(), setError(tr("No reference sequence in the file: ") + referenceUrl), newSubTasks);
        CHECK_EXT(1 == objects.size(), setError(tr("More than one sequence in the reference file: ") + referenceUrl), newSubTasks);

        U2SequenceObject *referenceObject = qobject_cast<U2SequenceObject *>(objects.first());
        SAFE_POINT_EXT(NULL != referenceObject, setError(tr("Unable to cast gobject to sequence object")), newSubTasks);
        CHECK_EXT(referenceObject->getAlphabet()->isDNA(), setError(tr("The input reference sequence '%1' contains characters that don't belong to DNA alphabet.").arg(referenceObject->getSequenceName())), newSubTasks);

        referenceEntityRef = referenceObject->getEntityRef();

        newSubTasks << new RemoveGapsFromSequenceTask(referenceObject);
    }

    return newSubTasks;
}

}   // namespace U2
