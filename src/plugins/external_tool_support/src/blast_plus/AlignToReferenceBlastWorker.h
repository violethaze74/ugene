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

#ifndef _U2_ALIGN_TO_REFERENCE_BLAST_WORKER_H_
#define _U2_ALIGN_TO_REFERENCE_BLAST_WORKER_H_

#include <U2Lang/BaseDatasetWorker.h>
#include <U2Lang/WorkflowUtils.h>


namespace U2 {

namespace Workflow {
class BlastReadsSubTask;
class FormatDBSubTask;
class ComposeResultSubTask;
}

namespace LocalWorkflow {

/************************************************************************/
/* AlignToReferenceBlastTask */
/************************************************************************/
class AlignToReferenceBlastTask : public Task {
    Q_OBJECT
public:
    AlignToReferenceBlastTask(const QString& refUrl,
                              const QString &resultUrl,
                              const SharedDbiDataHandler &reference,
                              const QList<SharedDbiDataHandler> &reads,
                              int minIdentityPercent,
                              DbiDataStorage *storage);
    QString getResultUrl() const;
    SharedDbiDataHandler getAnnotations() const;

    QMap<QString, bool> getAcceptedReads() const;
    QStringList getDiscardedReads() const;

private:
    void prepare();
    QString generateReport() const;
    QList<Task*> onSubTaskFinished(Task *subTask);
    ReportResult report();

    const QString referenceUrl;
    const QString resultUrl;
    const SharedDbiDataHandler reference;
    const QList<SharedDbiDataHandler> reads;
    const int minIdentityPercent;

    FormatDBSubTask* formatDbSubTask;
    BlastReadsSubTask* blastTask;
    ComposeResultSubTask *composeSubTask;
    SaveDocumentTask *saveTask;

    DbiDataStorage *storage;
};

/************************************************************************/
/* AlignToReferenceBlastPrompter */
/************************************************************************/
class AlignToReferenceBlastPrompter : public PrompterBase<AlignToReferenceBlastPrompter> {
    Q_OBJECT
public:
    AlignToReferenceBlastPrompter(Actor *a);

protected:
    QString composeRichDoc();
};

/************************************************************************/
/* AlignToReferenceBlastWorker */
/************************************************************************/
class AlignToReferenceBlastWorker : public BaseDatasetWorker {
    Q_OBJECT
public:
    AlignToReferenceBlastWorker(Actor *a);

protected:
    Task * createPrepareTask(U2OpStatus &os) const;
    void onPrepared(Task *task, U2OpStatus &os);
    Task * createTask(const QList<Message> &messages) const;
    QVariantMap getResult(Task *task, U2OpStatus &os) const;
    MessageMetadata generateMetadata(const QString &datasetName) const;

private:
    SharedDbiDataHandler reference;
};

/************************************************************************/
/* AlignToReferenceBlastWorkerFactory */
/************************************************************************/
class AlignToReferenceBlastWorkerFactory : public DomainFactory {
public:
    AlignToReferenceBlastWorkerFactory();
    Worker * createWorker(Actor *a);

    static const QString ACTOR_ID;
    static void init();
};

} // LocalWorkflow
} // U2

#endif // _U2_ALIGN_TO_REFERENCE_BLAST_WORKER_H_
