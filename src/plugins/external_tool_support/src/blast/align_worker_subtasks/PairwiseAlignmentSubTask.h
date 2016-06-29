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

#ifndef _U2_PAIRWISE_ALIGNMENT_SUBTASK_H_
#define _U2_PAIRWISE_ALIGNMENT_SUBTASK_H_

#include <U2Core/Task.h>

#include <U2Lang/DbiDataHandler.h>
#include <U2Lang/DbiDataStorage.h>

namespace U2 {
namespace Workflow {

class KAlignSubTask;

class PairwiseAlignmentTask : public Task {
    Q_OBJECT
public:
    PairwiseAlignmentTask(const SharedDbiDataHandler &reference,
                          const SharedDbiDataHandler &read,
                          DbiDataStorage *storage);
    void prepare();
    QList<Task*> onSubTaskFinished(Task *subTask);
    void run();

    bool isReverse() const;
    bool isComplement() const;
    SharedDbiDataHandler getRead() const;
    QList<U2MsaGap> getReferenceGaps() const;
    QList<U2MsaGap> getReadGaps() const;
    QString getInitialReadName() const;

private:
    QByteArray getComplement(const QByteArray &sequence, const DNAAlphabet *alphabet);
    QByteArray getReverse(const QByteArray &sequence) const;
    QByteArray getReverseComplement(const QByteArray &sequence, const DNAAlphabet *alphabet);
    void createRcReads();
    KAlignSubTask * initRc();
    void createSWAlignment(KAlignSubTask *task);
    void shiftGaps(QList<U2MsaGap> &gaps) const;

private:
    const SharedDbiDataHandler reference;
    const SharedDbiDataHandler read;
    SharedDbiDataHandler rRead;
    SharedDbiDataHandler cRead;
    SharedDbiDataHandler rcRead;
    DbiDataStorage *storage;

    KAlignSubTask *kalign;
    KAlignSubTask *rKalign;
    KAlignSubTask *cKalign;
    KAlignSubTask *rcKalign;
    bool reverse;
    bool complement;
    qint64 offset;

    SharedDbiDataHandler msa;
    qint64 maxChunkSize;
    QList<U2MsaGap> referenceGaps;
    QList<U2MsaGap> readGaps;
    QString initialReadName;
};

} // namespace Workflow
} // namespace U2

#endif // _U2_PAIRWISE_ALIGNMENT_SUBTASK_H_
