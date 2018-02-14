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

#ifndef _U2_CLASSIFICATION_FILTER_H_
#define _U2_CLASSIFICATION_FILTER_H_

#include <U2Core/ExternalToolRunTask.h>
#include <U2Core/GUrl.h>

#include <U2Formats/StreamSequenceReader.h>

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

#include "TaxonomySupport.h"

namespace U2 {
namespace LocalWorkflow {


class ClassificationFilterSettings {
public:
    static const QString RESOLUTION_SEPARATE;
//    static const QString RESOLUTION_DISTRIBUTE;
    static const QString RESOLUTION_DISCARD;

//    static const QString SPECIES;
//    static const QString GENUS;
//    static const QString FAMILY;
//    static const QString ORDER;
//    static const QString CLASS;
//    static const QString PHYLUM;

    ClassificationFilterSettings();

//    QString rank;
    QString resolution;
    QSet<TaxID> taxons;
    QString workingDir;
    bool paired;
};

//////////////////////////////////////////////////
//ClassificationFilter
class ClassificationFilterPrompter;
typedef PrompterBase<ClassificationFilterPrompter> ClassificationFilterBase;

class ClassificationFilterPrompter : public ClassificationFilterBase {
    Q_OBJECT
public:
    ClassificationFilterPrompter(Actor* p = 0) : ClassificationFilterBase(p) {}
protected:
    QString composeRichDoc();
};

class ClassificationFilterWorker: public BaseWorker {
    Q_OBJECT
public:
    ClassificationFilterWorker(Actor *a);
protected:
    void init();
//    bool isReady() const;
    Task * tick();
    void cleanup() {}

private:

private slots:
    void sl_taskFinished(Task *task);

protected:
    IntegralBus *input;
    //IntegralBus *pairedOutput;
    IntegralBus *output;
    ClassificationFilterSettings cfg;
};

class ClassificationFilterWorkerFactory : public DomainFactory {
    static const QString ACTOR_ID;
public:
    static void init();
    static void cleanup();
    ClassificationFilterWorkerFactory() : DomainFactory(ACTOR_ID) {}
    Worker* createWorker(Actor* a) { return new ClassificationFilterWorker(a); }
};

class ClassificationFilterTask : public Task {
    Q_OBJECT
public:
    ClassificationFilterTask(const ClassificationFilterSettings &cfg, const QString &readsUrl,
                             const QString &pairedReadsUrl, const TaxonomyClassificationResult &report);

    const QStringList &getSeUrls() const {return seUrls;}
    const QStringList &getPeUrls() const {return peUrls;}
    const QMap<QString, TaxID> &getFoundIDs() const {return foundIDs;}
    bool hasMissed() const {return missed;}

private:
    void prepare();
    void run();
    QString filter(DNASequence *seq, QString inputName);
    bool write(DNASequence *seq, QString fileName, const StreamSequenceReader &original);

    const ClassificationFilterSettings cfg;
    const QString readsUrl;
    const QString pairedReadsUrl;
    const TaxonomyClassificationResult report;
    QString dir;
    QStringList seUrls;
    QStringList peUrls;
    QMultiMap<QString, TaxID> foundIDs;
    bool missed;
};

} //LocalWorkflow
} //U2

#endif
