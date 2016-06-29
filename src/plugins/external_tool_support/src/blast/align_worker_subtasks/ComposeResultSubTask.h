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

#ifndef _U2_COMPOSE_RESULT_SUBTASK_H_
#define _U2_COMPOSE_RESULT_SUBTASK_H_

#include <U2Core/Task.h>

#include <U2Lang/DbiDataHandler.h>
#include <U2Lang/DbiDataStorage.h>

namespace U2 {
namespace Workflow {

class PairwiseAlignmentTask;

class ComposeResultSubTask : public Task {
    Q_OBJECT
public:
    ComposeResultSubTask(const SharedDbiDataHandler &reference, const QList<SharedDbiDataHandler> &reads, const QList<PairwiseAlignmentTask*> subTasks, DbiDataStorage *storage);
    void prepare();
    void run();
    SharedDbiDataHandler getAlignment() const;
    SharedDbiDataHandler getAnnotations() const;

private:
    PairwiseAlignmentTask * getPATask(int readNum);
    DNASequence getReadSequence(int readNum);
    DNASequence getReferenceSequence();
    QList<U2MsaGap> getReferenceGaps();
    QList<U2MsaGap> getShiftedGaps(int rowNum);
    void insertShiftedGapsIntoReference(MAlignment &alignment, const QList<U2MsaGap> &gaps);
    void insertShiftedGapsIntoRead(MAlignment &alignment, int readNum, const QList<U2MsaGap> &gaps);
    MAlignment createAlignment();
    void createAnnotations(const MAlignment &alignment);
    U2Region getReadRegion(const MAlignmentRow &readRow, const MAlignmentRow &referenceRow) const;
    U2Location getLocation(const U2Region &region, bool isComplement);

private:
    const SharedDbiDataHandler reference;
    const QList<SharedDbiDataHandler> reads;
    const QList<PairwiseAlignmentTask*> subTasks;
    DbiDataStorage *storage;
    SharedDbiDataHandler msa;
    SharedDbiDataHandler annotations;
};

} // namespace Workflow
} // namespace U2

#endif // _U2_COMPOSE_RESULT_SUBTASK_H_
