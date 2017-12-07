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

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/Counter.h>
#include <U2Core/DataPathRegistry.h>
#include <U2Core/DocumentImport.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/FailTask.h>
#include <U2Core/FileAndDirectoryUtils.h>
#include <U2Core/GObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Formats/BAMUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>

#include "ClarkSupport.h"
#include "ClarkBuildWorker.h"
#include "ClarkClassifyWorker.h"

namespace U2 {
namespace LocalWorkflow {

///////////////////////////////////////////////////////////////
//ClarkBuild
const QString ClarkBuildWorkerFactory::ACTOR_ID("clark-build-db");

static const QString OUTPUT_PORT("out");

static const QString DB_URL("db-url");
static const QString TAXONOMY("taxonomy");
static const QString TAXONOMY_RANK("taxonomy-rank");

static const QString TAX_DIR("taxdir");

/************************************************************************/
/* ClarkBuildPrompter */
/************************************************************************/
QString ClarkBuildPrompter::composeRichDoc() {
    QString doc = tr("Use custom data to build %1 CLARK database.").arg(getHyperlink(DB_URL, getURL(DB_URL)));
    return doc;
}

/************************************************************************/
/* ClarkBuildWorkerFactory */
/************************************************************************/
void ClarkBuildWorkerFactory::init() {

    Descriptor desc( ACTOR_ID, ClarkBuildWorker::tr("Build CLARK Database"),
        ClarkBuildWorker::tr("Build a CLARK database from a set of reference sequences (“targets”)."
                             "NCBI taxonomy data are used to map the accession number found in each reference sequence to its taxonomy ID.") );

    QList<PortDescriptor*> p;
    {
        Descriptor outD(OUTPUT_PORT, ClarkBuildWorker::tr("Output CLARK database"), ClarkBuildWorker::tr("URL to the folder with the CLARK database."));

        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        p << new PortDescriptor(outD, DataTypePtr(new MapDataType("clark.db-url", outM)), false, true);
    }

    QList<Attribute*> a;
    {
        Descriptor dbUrl(DB_URL, ClarkBuildWorker::tr("Database URL"),
            ClarkBuildWorker::tr("A folder that should be used to store the database files."));

        Descriptor taxonomy(TAXONOMY, ClarkBuildWorker::tr("Genomic library"),
            ClarkBuildWorker::tr("Genomes that should be used to build the database (“targets”)."
                                 "<br>The genomes should be specified in FASTA format. There should be one FASTA file per reference sequence. A sequence header must contain an accession number (i.e., &gt;accession.number ... or &gt;gi|number|ref|accession.number| ...)."));

        Descriptor rank(TAXONOMY_RANK, ClarkBuildWorker::tr("Taxonomy rank"),
            ClarkBuildWorker::tr("Set the taxonomy rank for the database."
                                    "<br>CLARK classifies metagenomic samples by using only one taxonomy rank. So as a general rule, "
                                    "consider first the genus or species rank, then if a high proportion of reads "
                                    "cannot be classified, reset your targets definition at a higher taxonomy rank (e.g., family or phylum)."));

        a << new Attribute( dbUrl, BaseTypes::STRING_TYPE(), true);

        a << new Attribute( taxonomy, BaseTypes::STRING_TYPE(), true);
        a << new Attribute( rank, BaseTypes::NUM_TYPE(), false, ClarkClassifySettings::Species);

        Descriptor taxUrl(TAX_DIR, ClarkBuildWorker::tr("Taxdata URL"),
            ClarkBuildWorker::tr("A folder with downloaded NCBI taxonomy info."));

        a << new Attribute( taxUrl, BaseTypes::STRING_TYPE(), true);
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap rankMap;
        rankMap["Species"] = ClarkClassifySettings::Species;
        rankMap["Genus"] = ClarkClassifySettings::Genus;
        rankMap["Family"] = ClarkClassifySettings::Family;
        rankMap["Order"] = ClarkClassifySettings::Order;
        rankMap["Class"] = ClarkClassifySettings::Class;
        rankMap["Phylum"] = ClarkClassifySettings::Phylum;
        delegates[TAXONOMY_RANK] = new ComboBoxDelegate(rankMap);

        DelegateTags tags;
        tags.set(DelegateTags::PLACEHOLDER_TEXT, L10N::required());
        delegates[DB_URL] = new URLDelegate(tags, "clark/database", false, true/*isPath*/);
        delegates[TAXONOMY] = new URLDelegate(tags, "clark/taxonomy", true/*multi*/);
        delegates[TAX_DIR] = new URLDelegate(tags, "clark/taxdata", false, true/*isPath*/);
    }

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new ClarkBuildPrompter());
    proto->addExternalTool(ET_CLARK_getAccssnTaxID);
    proto->addExternalTool(ET_CLARK_getfilesToTaxNodes);
    proto->addExternalTool(ET_CLARK_getTargetsDef);
    proto->addExternalTool(ET_CLARK_buildScript);

    // TODO: replace the category name with a constant after extracting to a separate plugin
    WorkflowEnv::getProtoRegistry()->registerProto(ClarkBuildPrompter::tr("NGS: Reads Classification"), proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new ClarkBuildWorkerFactory());
}

void ClarkBuildWorkerFactory::cleanup() {
    delete WorkflowEnv::getProtoRegistry()->unregisterProto(ACTOR_ID);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    delete localDomain->unregisterEntry(ACTOR_ID);
}


/************************************************************************/
/* ClarkBuildWorker */
/************************************************************************/
ClarkBuildWorker::ClarkBuildWorker(Actor *a)
:BaseWorker(a), output(NULL)
{

}

void ClarkBuildWorker::init() {
    output = ports.value(OUTPUT_PORT);
    SAFE_POINT(NULL != output, QString("Port with id '%1' is NULL").arg(OUTPUT_PORT), );
}

Task * ClarkBuildWorker::tick() {
    if (!isDone()) {
        QString databaseUrl = getValue<QString>(DB_URL);
        int rank = getValue<int>(TAXONOMY_RANK);
        QString genUrls = getValue<QString>(TAXONOMY);
        QString taxdataUrl = getValue<QString>(TAX_DIR);

        ClarkBuildTask *task = new ClarkBuildTask(databaseUrl, genUrls.split(';'), rank, taxdataUrl);
        task->addListeners(createLogListeners(2)); //fixme WTF???
        connect(new TaskSignalMapper(task), SIGNAL(si_taskFinished(Task *)), SLOT(sl_taskFinished(Task *)));
        setDone();
        return task;
    }
    return NULL;
}

void ClarkBuildWorker::sl_taskFinished(Task* t) {
    ClarkBuildTask *task = qobject_cast<ClarkBuildTask *>(t);
    SAFE_POINT(NULL != task, "Invalid task is encountered", );
    if (!task->isFinished() || task->hasError() || task->isCanceled()) {
        return;
    }

    const QString dbUrl = task->getDbUrl();

    QVariantMap data;
    data[BaseSlots::URL_SLOT().getId()] = dbUrl;
    output->put(Message(output->getBusType(), data));
    context->getMonitor()->addOutputFile(dbUrl, getActor()->getId());

    algoLog.trace(tr("Built Clark database"));
}

ClarkBuildTask::ClarkBuildTask(const QString &dbUrl, const QStringList &genomeUrls, int rank, const QString &taxdataUrl)
    : ExternalToolSupportTask(tr("Build Clark database"), TaskFlags_NR_FOSE_COSC),
    dbUrl(dbUrl), genomeUrls(genomeUrls), taxdataUrl(taxdataUrl), rank(rank)
{
  GCOUNTER(cvar, tvar, "ClarkBuildTask");

  SAFE_POINT_EXT(!dbUrl.isEmpty(), setError("Db URL is empty"), );
  SAFE_POINT_EXT(!taxdataUrl.isEmpty(), setError("Taxdata URL is empty"), );
  SAFE_POINT_EXT(!genomeUrls.isEmpty(), setError("genomic URLs is empty"), );
}

void ClarkBuildTask::prepare() {
    algoLog.info("ClarkBuildTask " + genomeUrls.join(";"));
    const QString db("custom");// = QString("custom_%1").arg(rank);
    const QString reflist = dbUrl + "/.custom";
    QDir dir(dbUrl);
    if (!dir.mkpath(db)){
        setError(tr("Failed to create database path: %1/%2").arg(dbUrl).arg(db));
        return;
    }
    {
        QFile refdata(reflist);
        refdata.open(QIODevice::WriteOnly);
        refdata.write(genomeUrls.join("\n").toLocal8Bit());
        refdata.close();
    }

  QString toolName = ET_CLARK_buildScript;
  QScopedPointer<ExternalToolRunTask> task(new ExternalToolRunTask(toolName, getArguments(), new ExternalToolLogParser()));
  CHECK_OP(stateInfo, );
  setListenerForTask(task.data());
  addSubTask(task.take());
}

QStringList ClarkBuildTask::getArguments() {
  // TODO: taxonomy is not processed

//    QVariantMap rankMap;
//    rankMap[ClarkClassifySettings::Species] = "--species";
//    rankMap[ClarkClassifySettings::Genus] = "--genus";
//    rankMap[ClarkClassifySettings::Family] = "--family";
//    rankMap[ClarkClassifySettings::Order] = "--order";
//    rankMap[ClarkClassifySettings::Class] = "--class";
//    rankMap[ClarkClassifySettings::Phylum] = "--phylum";


  QStringList arguments;

  arguments << dbUrl << taxdataUrl << "custom" << QString::number(rank);

  return arguments;
}

} //LocalWorkflow
} //U2
