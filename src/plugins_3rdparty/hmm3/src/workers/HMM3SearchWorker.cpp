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

#include "HMM3SearchWorker.h"
#include "HMM3IOWorker.h"
#include "search/uHMM3SearchTask.h"

#include <U2Lang/Datatype.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>

#include <U2Core/AnnotationData.h>
#include <U2Core/DNASequence.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/Log.h>
#include <U2Core/FailTask.h>
#include <U2Core/MultiTask.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/U2OpStatusUtils.h>

//#include <QtGui/QApplication>
/* TRANSLATOR U2::LocalWorkflow::HMM3SearchWorker */

namespace U2 {
namespace LocalWorkflow {

/*******************************
 * HMM3SearchWorkerFactory
 *******************************/
static const QString HMM3_PORT("in-hmm3");

static const QString NAME_ATTR("result-name");
static const QString THRESHOLD_ATTR("threshold-type");
static const QString DOM_E_ATTR("e-val");
static const QString DOM_T_ATTR("score");
static const QString DOM_Z_ATTR("domZ");
static const QString SEED_ATTR("seed");
static const QString BIAS_FILTER_ATTR("nobias");
static const QString SCORE_CORRECTION_ATTR("nonull2");
static const QString HEURISTIC_FILTER_ATTR("max");
static const QString F1_ATTR("F1");
static const QString F2_ATTR("F2");
static const QString F3_ATTR("F3");

const QString HMM3SearchWorker::E_THRESHOLD = "domE";
const QString HMM3SearchWorker::T_THRESHOLD = "domT";
const QString HMM3SearchWorker::CUT_GA_THRESHOLD = "cut_ga";
const QString HMM3SearchWorker::CUT_NC_THRESHOLD = "cut_nc";
const QString HMM3SearchWorker::CUT_TC_THRESHOLD = "cut_tc";

const QString HMM3SearchWorkerFactory::ACTOR("hmm3-search");

void HMM3SearchWorkerFactory::init() {

    QList<PortDescriptor*> p; QList<Attribute*> a;
    {
        Descriptor hd(HMM3_PORT, HMM3SearchWorker::tr("HMM3 profile"), HMM3SearchWorker::tr("HMM3 profile(s) to search with."));
        Descriptor sd(BasePorts::IN_SEQ_PORT_ID(), HMM3SearchWorker::tr("Input sequence"),
            HMM3SearchWorker::tr("An input sequence (nucleotide or protein) to search in."));
        Descriptor od(BasePorts::OUT_ANNOTATIONS_PORT_ID(), HMM3SearchWorker::tr("HMM3 annotations"),
            HMM3SearchWorker::tr("Annotations marking found similar sequence regions."));

        QMap<Descriptor, DataTypePtr> hmmM;
        hmmM[HMM3Lib::HMM3_SLOT] = HMM3Lib::HMM3_PROFILE_TYPE();
        p << new PortDescriptor(hd, DataTypePtr(new MapDataType("hmm.search.hmm", hmmM)), true /*input*/, false, IntegralBusPort::BLIND_INPUT);
        QMap<Descriptor, DataTypePtr> seqM;
        seqM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        p << new PortDescriptor(sd, DataTypePtr(new MapDataType("hmm.search.sequence", seqM)), true /*input*/);
        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();
        p << new PortDescriptor(od, DataTypePtr(new MapDataType("hmm.search.out", outM)), false /*input*/, true);
    }

    {
        Descriptor nd(NAME_ATTR, HMM3SearchWorker::tr("Result annotation"), HMM3SearchWorker::tr("A name of the result annotations."));
        Descriptor nsd(SEED_ATTR, HMM3SearchWorker::tr("Seed"), HMM3SearchWorker::tr("Random generator seed. 0 - means that one-time arbitrary seed will be used."));
        Descriptor td(THRESHOLD_ATTR, HMM3SearchWorker::tr("Threshold type"), HMM3SearchWorker::tr("Controlling reporting and  model-specific thresholds."));
        Descriptor ded(DOM_E_ATTR, HMM3SearchWorker::tr("Filter by high E-value"), HMM3SearchWorker::tr("Report domains with e-value less than."));
        Descriptor dtd(DOM_T_ATTR, HMM3SearchWorker::tr("Filter by low score"), HMM3SearchWorker::tr("Report domains with score greater than."));
        Descriptor dzd(DOM_Z_ATTR, HMM3SearchWorker::tr("Significant sequences"), HMM3SearchWorker::tr("Number of significant sequences, for domain E-value calculation."));
        Descriptor bfd(BIAS_FILTER_ATTR, HMM3SearchWorker::tr("No bias"), HMM3SearchWorker::tr("Turn off composition bias filter."));
        Descriptor scd(SCORE_CORRECTION_ATTR, HMM3SearchWorker::tr("No score corrections"), HMM3SearchWorker::tr("Turn off biased composition score corrections."));
        Descriptor hfd(HEURISTIC_FILTER_ATTR, HMM3SearchWorker::tr("No heuristic filters"), HMM3SearchWorker::tr("Turn all heuristic filters off (less speed, more power)."));
        Descriptor f1d(F1_ATTR, HMM3SearchWorker::tr("MSV threshold"), HMM3SearchWorker::tr("Stage 1 (MSV) threshold: promote hits w/ P <= F1."));
        Descriptor f2d(F2_ATTR, HMM3SearchWorker::tr("Vit threshold"), HMM3SearchWorker::tr("Stage 2 (Vit) threshold: promote hits w/ P <= F2."));
        Descriptor f3d(F3_ATTR, HMM3SearchWorker::tr("Fwd threshold"), HMM3SearchWorker::tr("Stage 3 (Fwd) threshold: promote hits w/ P <= F3."));

        a << new Attribute(nd, BaseTypes::STRING_TYPE(), true, "hmm_signal");
        a << new Attribute(nsd, BaseTypes::NUM_TYPE(), false, 0.0);
        a << new Attribute(td, BaseTypes::STRING_TYPE(), true, HMM3SearchWorker::E_THRESHOLD);
        Attribute *domEAttr = new Attribute(ded, BaseTypes::NUM_TYPE(), false, 1);
        Attribute *domTAttr = new Attribute(dtd, BaseTypes::NUM_TYPE(), false, 0.0);
        a << domEAttr;
        a << domTAttr;
        a << new Attribute(dzd, BaseTypes::NUM_TYPE(), false, 0);
        a << new Attribute(bfd, BaseTypes::BOOL_TYPE(), false, false);
        a << new Attribute(scd, BaseTypes::BOOL_TYPE(), false, false);
        a << new Attribute(hfd, BaseTypes::BOOL_TYPE(), false, false);
        Attribute *f1Attr = new Attribute(f1d, BaseTypes::NUM_TYPE(), false, 0.02);
        Attribute *f2Attr = new Attribute(f2d, BaseTypes::NUM_TYPE(), false, 0.001);
        Attribute *f3Attr = new Attribute(f3d, BaseTypes::NUM_TYPE(), false, 0.00001);
        a << f1Attr;
        a << f2Attr;
        a << f3Attr;

        domEAttr->addRelation(new VisibilityRelation(THRESHOLD_ATTR, HMM3SearchWorker::E_THRESHOLD));
        domTAttr->addRelation(new VisibilityRelation(THRESHOLD_ATTR, HMM3SearchWorker::T_THRESHOLD));
        f1Attr->addRelation(new VisibilityRelation(HEURISTIC_FILTER_ATTR, false));
        f2Attr->addRelation(new VisibilityRelation(HEURISTIC_FILTER_ATTR, false));
        f3Attr->addRelation(new VisibilityRelation(HEURISTIC_FILTER_ATTR, false));
    }

    Descriptor desc(HMM3SearchWorkerFactory::ACTOR, HMM3SearchWorker::tr("HMM3 Search"),
        HMM3SearchWorker::tr("Searches each input sequence for significantly similar sequence matches to all specified HMM profiles."
        " In case several profiles were supplied, searches with all profiles one by one and outputs united set of annotations for each sequence."));
    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap map;
        map[HMM3SearchWorker::tr("<= E-value")] = HMM3SearchWorker::E_THRESHOLD;
        map[HMM3SearchWorker::tr(">= score")] = HMM3SearchWorker::T_THRESHOLD;
        map[HMM3SearchWorker::tr("Use profile's GA gathering cutoffs")] = HMM3SearchWorker::CUT_GA_THRESHOLD;
        map[HMM3SearchWorker::tr("Use profile's NC noise cutoffs")] = HMM3SearchWorker::CUT_NC_THRESHOLD;
        map[HMM3SearchWorker::tr("Use profile's TC trusted cutoffs")] = HMM3SearchWorker::CUT_TC_THRESHOLD;
        delegates[THRESHOLD_ATTR] = new ComboBoxDelegate(map);
    }
    {
        QVariantMap eMap; eMap["prefix"]= ("1e"); eMap["minimum"] = (-99); eMap["maximum"] = (0);
        delegates[DOM_E_ATTR] = new SpinBoxDelegate(eMap);
    }
    {
        QVariantMap nMap; nMap["maximum"] = (INT_MAX); nMap["minimum"] = (0);
        delegates[SEED_ATTR] = new SpinBoxDelegate(nMap);
    }
    {
        QVariantMap tMap; tMap["decimals"]= (2); tMap["minimum"] = (-1e+09); tMap["maximum"] = (1e+09);
        tMap["singleStep"] = (0.1);
        delegates[DOM_T_ATTR] = new DoubleSpinBoxDelegate(tMap);
    }
    {
        QVariantMap map; map["decimals"] = 2; map["minimum"] = 0.0; map["maximum"] = 1000000000.0;
        delegates[DOM_Z_ATTR] = new DoubleSpinBoxDelegate(map);
    }
    {
        QVariantMap map; map["decimals"] = 5; map["minimum"] = -1000000000.0; map["maximum"] = 1000000000.0;
        delegates[F1_ATTR] = new DoubleSpinBoxDelegate(map);
        delegates[F2_ATTR] = new DoubleSpinBoxDelegate(map);
        delegates[F3_ATTR] = new DoubleSpinBoxDelegate(map);
    }

    proto->setEditor(new DelegateEditor(delegates));
    proto->setIconPath(":/hmm3/images/hmmer_16.png");
    proto->setPrompter(new HMM3SearchPrompter());
    WorkflowEnv::getProtoRegistry()->registerProto(HMM3Lib::HMM3_CATEGORY(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new HMM3SearchWorkerFactory());
}

/*******************************
 * HMM3SearchPrompter
 *******************************/
QString HMM3SearchPrompter::composeRichDoc() {
    Actor* hmmProducer = qobject_cast<IntegralBusPort*>(target->getPort(HMM3_PORT))->getProducer(HMM3_PORT);
    Actor* seqProducer = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()))->getProducer(BasePorts::IN_SEQ_PORT_ID());

    QString seqName = seqProducer ? tr("For each sequence from <u>%1</u>,").arg(seqProducer->getLabel()) : "";
    QString hmmName = hmmProducer ? tr("using all profiles provided by <u>%1</u>,").arg(hmmProducer->getLabel()) : "";

    QString resultName = getHyperlink(NAME_ATTR, getRequiredParam(NAME_ATTR));

    QString doc = tr("%1 search HMM3 signals %2. "
        "<br>Output the list of found regions annotated as <u>%4</u>.")
        .arg(seqName)
        .arg(hmmName)
        .arg(resultName);

    return doc;
}

/*******************************
 * HMM3SearchWorker
 *******************************/
HMM3SearchWorker::HMM3SearchWorker(Actor* a) : BaseWorker(a, false), hmmPort(NULL), seqPort(NULL), output(NULL) {
}

void HMM3SearchWorker::init() {
    setDefaultUHMM3SearchSettings(&cfg);

    hmmPort = ports.value(HMM3_PORT);
    seqPort = ports.value(BasePorts::IN_SEQ_PORT_ID());
    output = ports.value(BasePorts::OUT_ANNOTATIONS_PORT_ID());
    seqPort->addComplement(output);
    output->addComplement(seqPort);

    initConfig();
    resultName = actor->getParameter(NAME_ATTR)->getAttributeValue<QString>(context);
    if(resultName.isEmpty()){
        algoLog.details(tr("Value for attribute name is empty, default name used"));
        resultName = "hmm_signal";
    }
}

bool HMM3SearchWorker::isReady() const {
    if (isDone()) {
        return false;
    }
    bool seqEnded = seqPort->isEnded();
    bool hmmEnded = hmmPort->isEnded();
    int seqHasMes = seqPort->hasMessage();
    int hmmHasMes = hmmPort->hasMessage();
    return hmmHasMes || (hmmEnded && (seqHasMes || seqEnded));
}

Task* HMM3SearchWorker::tick() {
    while (hmmPort->hasMessage()) {
        hmms << hmmPort->get().getData().toMap().value(HMM3Lib::HMM3_SLOT.getId()).value<const P7_HMM*>();
    }
    if (!hmmPort->isEnded()) { //  || hmms.isEmpty() || !seqPort->hasMessage()
        return NULL;
    }

    if (seqPort->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(seqPort);
        if (inputMessage.isEmpty() || hmms.isEmpty()) {
            output->transit();
            return NULL;
        }
        SharedDbiDataHandler seqId = inputMessage.getData().toMap().value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<SharedDbiDataHandler>();
        QScopedPointer<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(context->getDataStorage(), seqId));
        if (seqObj.isNull()) {
            return NULL;
        }
        U2OpStatusImpl os;
        DNASequence dnaSequence = seqObj->getWholeSequence(os);
        CHECK_OP(os, new FailTask(os.getError()));

        if (dnaSequence.alphabet->getType() != DNAAlphabet_RAW) {
            QList<Task*> subtasks;
            UHMM3SearchTaskSettings settings;
            settings.inner = cfg;
            foreach(const P7_HMM* hmm, hmms) {
                subtasks << new UHMM3SWSearchTask (hmm, dnaSequence, settings);
            }
            Task* searchTask = new MultiTask(tr("Find HMM3 signals in %1").arg(dnaSequence.getName()), subtasks);
            connect(new TaskSignalMapper(searchTask), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));
            return searchTask;
        }
        QString err = tr("Bad sequence supplied to input: %1").arg(dnaSequence.getName());
        return new FailTask(err);
    } if (seqPort->isEnded()) {
        setDone();
        output->setEnded();
    }
    return NULL;
}

void HMM3SearchWorker::sl_taskFinished(Task *t) {
    SAFE_POINT(NULL != t, "Invalid task is encountered",);
    if (t->isCanceled()) {
        return;
    }
    if (NULL != output) {
        QList<SharedAnnotationData> list;

        foreach(Task *sub, t->getSubtasks()) {
            UHMM3SWSearchTask *hst = qobject_cast<UHMM3SWSearchTask *>(sub);
            if (hst == NULL){
                continue;
            }
            list.append(hst->getResultsAsAnnotations(U2FeatureTypes::MiscSignal, resultName));
        }

        CHECK(!list.isEmpty(), );

        const SharedDbiDataHandler tableId = context->getDataStorage()->putAnnotationTable(list);
        output->put(Message(BaseTypes::ANNOTATION_TABLE_TYPE(), qVariantFromValue<SharedDbiDataHandler>(tableId)));
        algoLog.info(tr("Found %1 HMM3 signals").arg(list.size()));
    }
}

void HMM3SearchWorker::cleanup() {
}

void HMM3SearchWorker::initConfig() {
    const QString thresholdType = getValue<QString>(THRESHOLD_ATTR);
    if (E_THRESHOLD == thresholdType) {
        int domENum = actor->getParameter(DOM_E_ATTR)->getAttributeValue<int>(context);
        if (domENum > 1) {
            algoLog.details(tr("Power of e-value must be less or equal to zero. Using default value: 1e+1"));
            domENum = 1;
        }
        cfg.domE = pow(10, domENum);
    } else if (T_THRESHOLD == thresholdType) {
        cfg.domT = (float)actor->getParameter(DOM_T_ATTR)->getAttributeValue<double>(context);
        if (cfg.domT <= 0) {
            algoLog.details(tr("Score must be greater than zero. Using default value: 0.01"));
            cfg.domT = 0.01;
        }
    } else if (CUT_GA_THRESHOLD == thresholdType) {
        cfg.useBitCutoffs = p7H_GA;
    } else if (CUT_NC_THRESHOLD == thresholdType) {
        cfg.useBitCutoffs = p7H_NC;
    } else if (CUT_TC_THRESHOLD == thresholdType) {
        cfg.useBitCutoffs = p7H_TC;
    }

    double domZValue = getValue<double>(DOM_Z_ATTR);
    if (domZValue > 0) {
        cfg.domZ = domZValue;
    }

    cfg.noBiasFilter = getValue<bool>(BIAS_FILTER_ATTR);
    cfg.noNull2 = getValue<bool>(SCORE_CORRECTION_ATTR);
    cfg.doMax = getValue<bool>(HEURISTIC_FILTER_ATTR);

    if (!cfg.doMax) {
        cfg.f1 = getValue<double>(F1_ATTR);
        cfg.f2 = getValue<double>(F1_ATTR);
        cfg.f3 = getValue<double>(F1_ATTR);
    }

    cfg.seed = actor->getParameter(SEED_ATTR)->getAttributeValue<int>(context);
}

} //namespace LocalWorkflow
} //namespace U2
