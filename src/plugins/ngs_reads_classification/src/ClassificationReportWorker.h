/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
 * http://ugene.net
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

#ifndef _U2_CLASSIFICATION_REPORT_H_
#define _U2_CLASSIFICATION_REPORT_H_

#include <U2Core/ExternalToolRunTask.h>
#include <U2Core/GUrl.h>

#include <U2Formats/StreamSequenceReader.h>

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

#include "TaxonomySupport.h"

namespace U2 {
namespace LocalWorkflow {


//////////////////////////////////////////////////
//ClassificationReport
class ClassificationReportPrompter;
typedef PrompterBase<ClassificationReportPrompter> ClassificationReportBase;

class ClassificationReportPrompter : public ClassificationReportBase {
    Q_OBJECT
public:
    ClassificationReportPrompter(Actor* p = 0) : ClassificationReportBase(p) {}
protected:
    QString composeRichDoc();
};

class ClassificationReportWorker: public BaseWorker {
    Q_OBJECT
public:
    ClassificationReportWorker(Actor *a);
protected:
    void init();
//    bool isReady() const;
    Task * tick();
    void cleanup() {}

private:
    IntegralBus *input;
    QMap<TaxID,uint> data;
    uint totalCount;

private slots:
    void sl_taskFinished(Task *task);
};

class ClassificationReportWorkerFactory : public DomainFactory {
    static const QString ACTOR_ID;
public:
    static void init();
    static void cleanup();
    ClassificationReportWorkerFactory() : DomainFactory(ACTOR_ID) {}
    Worker* createWorker(Actor* a) { return new ClassificationReportWorker(a); }
};

class ClassificationReportTask : public Task {
    Q_OBJECT
public:
    ClassificationReportTask(const QMap<TaxID,uint> &data, uint totalCount, const QString &reportUrl, const QString &workingDir);
    QString getUrl() const {return url;}

private:
    void run();

    QMap<TaxID,uint> data;
    const uint totalCount;
    const QString workingDir;
    QString url;
};

} //LocalWorkflow
} //U2

#endif
