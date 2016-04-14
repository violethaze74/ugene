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

#ifndef _U2_HMMER3_SEARCH_WORKFLOW_TASK_H_
#define _U2_HMMER3_SEARCH_WORKFLOW_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/U2FeatureType.h>
#include <search/uHMM3SearchTask.h>

namespace U2 {

class AnnotationTableObject;
class SimpleInOutWorkflowTask;
class U2SequenceObject;

class Hmmer3SearchWorfklowTask : public Task {
    Q_OBJECT
public:
    Hmmer3SearchWorfklowTask(const QString &profileUrl, U2SequenceObject *sequenceObject, AnnotationTableObject *annotationsObject,
        const QString &group, const QString &description, U2FeatureType type, const QString &name, const UHMM3SearchTaskSettings &settings);

    void prepare();
    ReportResult report();
    QString generateReport() const;

private:
    QString profileUrl;
    U2SequenceObject *sequenceObject;
    AnnotationTableObject *annotationsObject;
    QString group;
    QString description;
    U2FeatureType type;
    QString name;
    UHMM3SearchTaskSettings settings;
    SimpleInOutWorkflowTask *workflowTask;
    int resultCount;
};

} // U2

#endif // _U2_HMMER3_SEARCH_WORKFLOW_TASK_H_
