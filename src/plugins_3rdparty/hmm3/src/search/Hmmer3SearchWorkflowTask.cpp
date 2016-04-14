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

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/U1AnnotationUtils.h>
#include <U2Lang/SimpleWorkflowTask.h>

#include "Hmmer3SearchWorkflowTask.h"

namespace U2 {

Hmmer3SearchWorfklowTask::Hmmer3SearchWorfklowTask(const QString &profileUrl, U2SequenceObject *sequenceObject, AnnotationTableObject *annotationsObject,
    const QString &group, const QString &description, U2FeatureType type, const QString &name, const UHMM3SearchTaskSettings &settings)
: Task("HMMER 3 search workflow task", TaskFlags_NR_FOSE_COSC | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled),
profileUrl(profileUrl), sequenceObject(sequenceObject), annotationsObject(annotationsObject),
group(group), description(description), type(type), name(name), settings(settings), workflowTask(NULL), resultCount(0)
{

}

void Hmmer3SearchWorfklowTask::prepare() {
    SimpleInOutWorkflowTaskConfig config;
    U2DbiRef dbiRef = AppContext::getDbiRegistry()->getSessionTmpDbiRef(stateInfo);
    CHECK_OP(stateInfo, );
    config.objects << sequenceObject->clone(dbiRef, stateInfo);
    CHECK_OP(stateInfo, );
    config.inFormat = BaseDocumentFormats::PLAIN_GENBANK;
    config.outFormat = BaseDocumentFormats::PLAIN_GENBANK;
    config.schemaName = "hmm3-search";

    config.extraArgs << QString("--hmm=%1").arg(profileUrl);
    config.extraArgs << QString("--seed=%1").arg(settings.inner.seed);
    config.extraArgs << QString("--annotation-name=%1").arg(name);

    workflowTask = new SimpleInOutWorkflowTask(config);
    addSubTask(workflowTask);
}

Task::ReportResult Hmmer3SearchWorfklowTask::report() {
    Document *doc = workflowTask->getDocument();
    QList<GObject*> objects = doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
    if (objects.isEmpty()) {
        setError(tr("No annotations objects found"));
        return ReportResult_Finished;
    }
    AnnotationTableObject *hmmerObject = qobject_cast<AnnotationTableObject*>(objects.first());
    QList<SharedAnnotationData> data;
    foreach (Annotation *annotation, hmmerObject->getAnnotations()) {
        SharedAnnotationData annData = annotation->getData();
        annData->type = type;
        data << annData;
    }
    U1AnnotationUtils::addDescriptionQualifier(data, description);
    resultCount += data.size();
    annotationsObject->addAnnotations(data, group);
    return ReportResult_Finished;
}

QString Hmmer3SearchWorfklowTask::generateReport() const {
    QString res;
    res += "<table>";
    res += "<tr><td width=200><b>" + tr("HMM profile used") + "</b></td><td>" + QFileInfo(profileUrl).absoluteFilePath() + "</td></tr>";

    if (hasError() || isCanceled()) {
        res += "<tr><td width=200><b>" + tr("Task was not finished") + "</b></td><td></td></tr>";
        res += "</table>";
        return res;
    }

    res += "<tr><td><b>" + tr("Result annotation table") + "</b></td><td>" + annotationsObject->getDocument()->getName() + "</td></tr>";
    res += "<tr><td><b>" + tr("Result annotation group") + "</b></td><td>" + group + "</td></tr>";
    res += "<tr><td><b>" + tr("Result annotation name") + "</b></td><td>" + name + "</td></tr>";

    res += "<tr><td><b>" + tr("Results count") + "</b></td><td>" + QString::number(resultCount) + "</td></tr>";
    res += "</table>";
    return res;
}

} // U2
