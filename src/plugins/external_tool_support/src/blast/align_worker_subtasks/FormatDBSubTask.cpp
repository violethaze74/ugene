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

#include "FormatDBSubTask.h"
#include "blast/FormatDBSupport.h"
#include "blast/FormatDBSupportTask.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/ExternalToolRunTask.h>
#include <U2Core/L10n.h>
#include <U2Core/U2SafePoints.h>

#include <U2Lang/DbiDataStorage.h>

#include <QDir>

namespace U2 {
namespace Workflow {

FormatDBSubTask::FormatDBSubTask(const QString &referenceUrl,
                                 const SharedDbiDataHandler &referenceDbHandler,
                                 DbiDataStorage *storage)
    : Task(tr("Format DB task wrapper"), TaskFlags_NR_FOSE_COSC),
      referenceUrl(referenceUrl),
      referenceDbHandler(referenceDbHandler),
      storage(storage)
{

}

void FormatDBSubTask::prepare() {
    FormatDBSupportTaskSettings settings;
    settings.inputFilesPath << referenceUrl;

    QScopedPointer<U2SequenceObject> refObject(StorageUtils::getSequenceObject(storage, referenceDbHandler));
    CHECK_EXT(!refObject.isNull(), setError(L10N::nullPointerError("U2SequenceObject")), );

    CHECK_EXT(refObject->getAlphabet() != NULL, setError(L10N::nullPointerError("DNAAlphabet")), );
    settings.isInputAmino = refObject->getAlphabet()->isAmino();
    settings.databaseTitle = refObject->getSequenceName();

    settings.outputPath = ExternalToolSupportUtils::createTmpDir("align_to_ref", stateInfo);
    CHECK_OP(stateInfo, );

    FormatDBSupportTask* formatTask = new FormatDBSupportTask(ET_FORMATDB, settings);
    addSubTask(formatTask);

    databaseNameAndPath = settings.outputPath;
}

const QString& FormatDBSubTask::getResultPath() const {
    return databaseNameAndPath;
}

} // namespace Workflow
} // namespace U2
