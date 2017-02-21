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

#ifndef _U2_EXPORT_MSA_2_MCA_TASK_H_
#define _U2_EXPORT_MSA_2_MCA_TASK_H_

#include <U2Core/DNAChromatogram.h>
#include <U2Core/DocumentProviderTask.h>

namespace U2 {

class StateLocker;
class MultipleSequenceAlignmentObject;

class ExportMsa2McaTask : public DocumentProviderTask {
public:
    ExportMsa2McaTask(MultipleSequenceAlignmentObject *msaObject, const QString &mcaFilePath);
    ~ExportMsa2McaTask();

private:
    void prepare();
    void run();
    ReportResult report();

    static DNAChromatogram generateChromatogram(const QString &name, const int length);
    Document * prepareDocument();

    MultipleSequenceAlignmentObject *msaObject;
    const QString mcaFilePath;
    StateLocker *locker;

    static const int MAX_TRACE_VALUE;
};

}   // namespace U2

#endif // _U2_EXPORT_MSA_2_MCA_TASK_H_
