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
#include <U2Core/U2SafePoints.h>

#include "Hmmer3SearchWorkflowTask.h"
#include "workers/HMM3SearchWorker.h"

namespace U2 {

using namespace LocalWorkflow;

Hmmer3SearchWorfklowTask::Hmmer3SearchWorfklowTask(const QString &profileUrl, U2SequenceObject *sequenceObject, AnnotationTableObject *annotationsObject,
    const QString &group, const QString &description, U2FeatureType type, const QString &name, const UHMM3SearchTaskSettings &settings)
: Task("HMMER 3 search workflow task", TaskFlags_NR_FOSE_COSC | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled),
profileUrl(profileUrl), sequenceObject(sequenceObject), annotationsObject(annotationsObject),
group(group), description(description), type(type), name(name), settings(settings), workflowTask(NULL), resultCount(0)
{

}

void Hmmer3SearchWorfklowTask::prepare() {
    SimpleInOutWorkflowTaskConfig config = getConfig();
    CHECK_OP(stateInfo, );

    workflowTask = new SimpleInOutWorkflowTask(config);
    addSubTask(workflowTask);
}

Task::ReportResult Hmmer3SearchWorfklowTask::report() {
    CHECK_OP(stateInfo, ReportResult_Finished);
    Document *doc = workflowTask->getDocument();
    CHECK(NULL != doc, ReportResult_Finished);
    QList<GObject*> objects = doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
    if (objects.isEmpty()) {
        setError(tr("No annotations objects found"));
        return ReportResult_Finished;
    }
    AnnotationTableObject *hmmerObject = qobject_cast<AnnotationTableObject*>(objects.first());
    QList<SharedAnnotationData> data;
    foreach (Annotation *annotation, hmmerObject->getAnnotations()) {
        SharedAnnotationData annData = annotation->getData();
        annData->name = name;
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

SimpleInOutWorkflowTaskConfig Hmmer3SearchWorfklowTask::getConfig() {
    SimpleInOutWorkflowTaskConfig config;
    U2DbiRef dbiRef = AppContext::getDbiRegistry()->getSessionTmpDbiRef(stateInfo);
    CHECK_OP(stateInfo, config);
    config.objects << sequenceObject->clone(dbiRef, stateInfo);
    CHECK_OP(stateInfo, config);
    config.inFormat = BaseDocumentFormats::PLAIN_GENBANK;
    config.outFormat = BaseDocumentFormats::PLAIN_GENBANK;
    config.schemaName = "hmm3-search";
    config.emptyResultPossible = true;

    config.extraArgs << "--hmm=" + profileUrl;
    config.extraArgs << "--seed=" + QString::number(settings.inner.seed);

    if (p7H_GA == settings.inner.useBitCutoffs) {
        config.extraArgs << "--threshold-type=" + HMM3SearchWorker::CUT_GA_THRESHOLD;
    } else if (p7H_NC == settings.inner.useBitCutoffs) {
        config.extraArgs << "--threshold-type=" + HMM3SearchWorker::CUT_NC_THRESHOLD;
    } else if (p7H_TC == settings.inner.useBitCutoffs) {
        config.extraArgs << "--threshold-type=" + HMM3SearchWorker::CUT_TC_THRESHOLD;
    } else if (OPTION_NOT_SET == settings.inner.domT) {
        config.extraArgs << "--domE=" + QString::number(log10(settings.inner.domE));
    } else {
        config.extraArgs << "--domT=" + QString::number(settings.inner.domT);
    }
    if (settings.inner.domZ > 0) {
        config.extraArgs << "--domZ=" + QString::number(settings.inner.domZ);
    }

    config.extraArgs << "--nobias=" + QString::number(settings.inner.noBiasFilter);
    config.extraArgs << "--nonull2=" + QString::number(settings.inner.noNull2);
    config.extraArgs << "--max=" + QString::number(settings.inner.doMax);

    if (!settings.inner.doMax) {
        config.extraArgs << "--F1=" + QString::number(settings.inner.f1);
        config.extraArgs << "--F2=" + QString::number(settings.inner.f2);
        config.extraArgs << "--F3=" + QString::number(settings.inner.f3);
    }
    return config;
}

} // U2
