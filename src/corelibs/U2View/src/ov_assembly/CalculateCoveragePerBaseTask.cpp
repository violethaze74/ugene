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

#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2AttributeUtils.h>
#include <U2Core/U2CoreAttributes.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2SafePoints.h>

#include "CalculateCoveragePerBaseTask.h"

namespace U2 {

CalculateCoveragePerBaseOnRegionTask::CalculateCoveragePerBaseOnRegionTask(const U2DbiRef &dbiRef, const U2DataId &assemblyId, const U2Region &region) :
    Task(tr("Calculate coverage per base for assembly %1 on region (%2, %3)"), TaskFlag_None),
    dbiRef(dbiRef),
    assemblyId(assemblyId),
    region(region),
    results(new QVector<CoveragePerBaseInfo>)
{
    SAFE_POINT_EXT(dbiRef.isValid(), setError(tr("Invalid database reference")), );
    SAFE_POINT_EXT(!assemblyId.isEmpty(), setError(tr("Invalid assembly ID")), );

    DbiConnection con(dbiRef, stateInfo);
    CHECK_OP(stateInfo, );
    U2AssemblyDbi *assemblyDbi = con.dbi->getAssemblyDbi();
    SAFE_POINT_EXT(NULL != assemblyDbi, setError(tr("Assembly DBI is NULL")), );

    const U2Assembly assembly = assemblyDbi->getAssemblyObject(assemblyId, stateInfo);
    CHECK_OP(stateInfo, );
    setTaskName(getTaskName().arg(assembly.visualName).arg(region.startPos).arg(region.endPos()));
}

CalculateCoveragePerBaseOnRegionTask::~CalculateCoveragePerBaseOnRegionTask() {
    delete results;
}

void CalculateCoveragePerBaseOnRegionTask::run() {
    U2AssemblyUtils::calculateCoveragePerBase(dbiRef, assemblyId, region, results, stateInfo);
}

const U2Region &CalculateCoveragePerBaseOnRegionTask::getRegion() const {
    return region;
}

QVector<CoveragePerBaseInfo> *CalculateCoveragePerBaseOnRegionTask::takeResult() {
    QVector<CoveragePerBaseInfo> *result = results;
    results = NULL;
    return result;
}

CalculateCoveragePerBaseTask::CalculateCoveragePerBaseTask(const U2DbiRef &dbiRef, const U2DataId &assemblyId) :
    Task(tr("Calculate coverage per base for assembly %1"), TaskFlags_NR_FOSE_COSC),
    dbiRef(dbiRef),
    assemblyId(assemblyId)
{
    SAFE_POINT_EXT(dbiRef.isValid(), setError(tr("Invalid database reference")), );
    SAFE_POINT_EXT(!assemblyId.isEmpty(), setError(tr("Invalid assembly ID")), );

    DbiConnection con(dbiRef, stateInfo);
    CHECK_OP(stateInfo, );
    U2AssemblyDbi *assemblyDbi = con.dbi->getAssemblyDbi();
    SAFE_POINT_EXT(NULL != assemblyDbi, setError(tr("Assembly DBI is NULL")), );

    const U2Assembly assembly = assemblyDbi->getAssemblyObject(assemblyId, stateInfo);
    CHECK_OP(stateInfo, );
    setTaskName(getTaskName().arg(assembly.visualName));
}

CalculateCoveragePerBaseTask::~CalculateCoveragePerBaseTask() {
    qDeleteAll(results.values());
}

void CalculateCoveragePerBaseTask::prepare() {
    DbiConnection con(dbiRef, stateInfo);
    CHECK_OP(stateInfo, );
    U2AttributeDbi *attributeDbi = con.dbi->getAttributeDbi();
    SAFE_POINT_EXT(NULL != attributeDbi, setError(tr("Attribute DBI is NULL")), );

    const U2IntegerAttribute lengthAttribute = U2AttributeUtils::findIntegerAttribute(attributeDbi, assemblyId, U2BaseAttributeName::reference_length, stateInfo);
    CHECK_OP(stateInfo, );
    CHECK_EXT(lengthAttribute.hasValidId(), setError(tr("Can't get the assembly length: attribute is missing")), );

    const qint64 length = lengthAttribute.value;
    SAFE_POINT_EXT(0 < length, setError(tr("Assembly has zero length")), );

    qint64 tasksCount = length / MAX_REGION_LENGTH + (length % MAX_REGION_LENGTH > 0 ? 1 : 0);
    for (qint64 i = 0; i < tasksCount; i++) {
        const U2Region region(i * MAX_REGION_LENGTH, (i == tasksCount - 1 ? length % MAX_REGION_LENGTH : MAX_REGION_LENGTH));
        addSubTask(new CalculateCoveragePerBaseOnRegionTask(dbiRef, assemblyId, region));
    }
}

QList<Task *> CalculateCoveragePerBaseTask::onSubTaskFinished(Task *subTask) {
    QList<Task *> res;
    CHECK_OP(stateInfo, res);

    CalculateCoveragePerBaseOnRegionTask *calculateTask = qobject_cast<CalculateCoveragePerBaseOnRegionTask *>(subTask);
    SAFE_POINT_EXT(NULL != calculateTask, setError(tr("An unexpected subtask")), res);

    results.insert(calculateTask->getRegion().startPos, calculateTask->takeResult());
    emit si_regionIsProcessed(calculateTask->getRegion().startPos);

    return res;
}

bool CalculateCoveragePerBaseTask::isResultReady(qint64 startPos) const {
    return results.contains(startPos);
}

bool CalculateCoveragePerBaseTask::areThereUnprocessedResults() const {
    return !results.isEmpty();
}

QVector<CoveragePerBaseInfo> *CalculateCoveragePerBaseTask::takeResult(qint64 startPos) {
    QVector<CoveragePerBaseInfo> *result = results.value(startPos, NULL);
    results.remove(startPos);
    return result;
}

}   // namespace U2
