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

#include <QFile>

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/MultipleChromatogramAlignment.h>
#include <U2Core/MultipleChromatogramAlignmentImporter.h>
#include <U2Core/MultipleChromatogramAlignmentObject.h>
#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/StateLockableDataModel.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2SafePoints.h>

#include "ExportMsa2McaTask.h"

namespace U2 {

const int ExportMsa2McaTask::MAX_TRACE_VALUE = 10000;

ExportMsa2McaTask::ExportMsa2McaTask(MultipleSequenceAlignmentObject *msaObject, const QString &mcaFilePath) :
    DocumentProviderTask(tr("Export MSA to MCA"), TaskFlag_None),
    msaObject(msaObject),
    mcaFilePath(mcaFilePath)
{
    SAFE_POINT_EXT(NULL != msaObject, setError("NULL MSA object"), );
    tpm = Progress_Manual;
}

ExportMsa2McaTask::~ExportMsa2McaTask() {
    delete locker;
}

void ExportMsa2McaTask::prepare() {
    locker = new StateLocker(msaObject);
}

void ExportMsa2McaTask::run() {
    const MultipleSequenceAlignment msa = msaObject->getMsa();
    MultipleChromatogramAlignment mca(msa->getName(), msa->getAlphabet());
    const int rowsCount = msa->getNumRows();
    int rowsCounter = 0;
    foreach (const MultipleSequenceAlignmentRow &msaRow, msa->getMsaRows()) {
        mca->addRow(msaRow->getName(), generateChromatogram(msaRow->getName(), msaRow->getSequence().length()), msaRow->getData());
        stateInfo.setProgress(100 * (rowsCounter++ / rowsCount));
    }

    const U2DbiRef dbiRef(SQLITE_DBI_ID, mcaFilePath);
    QScopedPointer<MultipleChromatogramAlignmentObject> mcaObject(MultipleChromatogramAlignmentImporter::createAlignment(stateInfo, dbiRef, U2ObjectDbi::ROOT_FOLDER, mca));
    CHECK_OP(stateInfo, );

    resultDocument = prepareDocument();
    CHECK_OP(stateInfo, );

    resultDocument->addObject(mcaObject.take());
}

Task::ReportResult ExportMsa2McaTask::report() {
    delete locker;
    locker = NULL;

    if (hasError() || isCanceled()) {
        const bool removed = QFile(mcaFilePath).remove();
        if (!removed) {
            ioLog.trace(QString("Can't remove the file '%1'").arg(mcaFilePath));
        }
    }
    return ReportResult_Finished;
}

DNAChromatogram ExportMsa2McaTask::generateChromatogram(const QString &name, const int length) {
    DNAChromatogram chromatogram;
    chromatogram.name = name;
    chromatogram.traceLength = length;
    chromatogram.seqLength = length;
    for (int i = 0; i < length; i++) {
        chromatogram.A += rand() % 2000;
        chromatogram.C += rand() % 2000;
        chromatogram.G += rand() % 2000;
        chromatogram.T += rand() % 2000;
        chromatogram.prob_A += rand() % 73;
        chromatogram.prob_C += rand() % 73;
        chromatogram.prob_G += rand() % 73;
        chromatogram.prob_T += rand() % 73;
        chromatogram.baseCalls += rand() % 10000;
    }
    return chromatogram;
}

Document * ExportMsa2McaTask::prepareDocument() {
    DocumentFormatRegistry *documentFormatRegistry = AppContext::getDocumentFormatRegistry();
    SAFE_POINT_EXT(NULL != documentFormatRegistry, setError("DocumentFormatRegistry is NULL"), NULL);
    DocumentFormat *format = documentFormatRegistry->getFormatById(BaseDocumentFormats::UGENEDB);
    SAFE_POINT_EXT(NULL != format, setError("Format is NULL"), NULL);

    Document *document = format->createNewLoadedDocument(IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), mcaFilePath, stateInfo);
    CHECK_OP(stateInfo, NULL);

    document->setDocumentOwnsDbiResources(false);
    return document;
}

}
