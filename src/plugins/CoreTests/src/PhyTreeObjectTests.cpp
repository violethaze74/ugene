/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2023 UniPro <ugene@unipro.ru>
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

#include "PhyTreeObjectTests.h"

#include <U2Algorithm/PhyTreeGeneratorRegistry.h>
#include <U2Algorithm/PhyTreeGeneratorTask.h>

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/PhyTreeObject.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

#define ALGORITHM_ATTR "algorithm"
#define INDEX_ATTR "index"
#define OBJ_ATTR "obj"
#define DOC_ATTR "doc"
#define PHYNODE_ATTR "phynode"
#define SIBLING_ATTR "sibling"
#define VALUE_ATTR "value"

#define EPS 0.0001

void GTest_CalculateTreeFromAligment::init(XMLTestFormat*, const QDomElement& el) {
    task = nullptr;
    objContextName = el.attribute(OBJ_ATTR);
    if (objContextName.isEmpty()) {
        failMissingValue(OBJ_ATTR);
        return;
    }

    treeObjContextName = el.attribute(INDEX_ATTR);
    if (treeObjContextName.isEmpty()) {
        failMissingValue(INDEX_ATTR);
        return;
    }

    algId = el.attribute(ALGORITHM_ATTR);
    if (algId.isEmpty()) {
        failMissingValue(ALGORITHM_ATTR);
        return;
    }
}
void GTest_CalculateTreeFromAligment::prepare() {
    auto obj = getContext<GObject>(this, objContextName);
    if (obj == nullptr) {
        stateInfo.setError(QString("wrong value: %1").arg(objContextName));
        return;
    }

    auto maObj = qobject_cast<MultipleSequenceAlignmentObject*>(obj);
    if (maObj == nullptr) {
        stateInfo.setError(QString("can't cast to multiple alignment object from: %1").arg(obj->getGObjectName()));
        return;
    }

    CreatePhyTreeSettings settings;
    settings.algorithm = algId;
    settings.mb_ngen = 1000;
    settings.mrBayesSettingsScript = QString("Begin MrBayes;\n"
                                             "lset Nst=2 rates=gamma ngammacat=4;\n"
                                             "mcmc ngen=1000 samplefreq=100 printfreq=1000 nchains=4 temp=0.4 savebrlens=yes "
                                             "startingtree=random seed=10;\n"
                                             "sumt burnin=10;\n"
                                             "End;\n");

    task = new PhyTreeGeneratorLauncherTask(maObj->getMultipleAlignment(), settings);

    if (task == nullptr) {
        stateInfo.setError(QString("Algorithm %1 not found").arg(algId));
        return;
    }
    addSubTask(task);
}

Task::ReportResult GTest_CalculateTreeFromAligment::report() {
    if (task == nullptr) {
        return ReportResult_Finished;
    }

    if (!task->hasError()) {
        PhyTree tree = task->getResult();
        U2DbiRef dbiRef = AppContext::getDbiRegistry()->getSessionTmpDbiRef(stateInfo);
        CHECK_OP(stateInfo, ReportResult_Finished);
        PhyTreeObject* obj = PhyTreeObject::createInstance(tree, treeObjContextName, dbiRef, stateInfo);
        CHECK_OP(stateInfo, ReportResult_Finished);
        addContext(treeObjContextName, obj);
    }

    return ReportResult_Finished;
}

///////////////////////////////////////////////////////////////////////////////////////////

void GTest_CheckPhyNodeHasSibling::init(XMLTestFormat*, const QDomElement& el) {
    treeContextName = el.attribute(OBJ_ATTR);
    if (treeContextName.isEmpty()) {
        failMissingValue(OBJ_ATTR);
        return;
    }

    nodeName = el.attribute(PHYNODE_ATTR);
    if (nodeName.isEmpty()) {
        failMissingValue(PHYNODE_ATTR);
        return;
    }

    siblingName = el.attribute(SIBLING_ATTR);
    if (siblingName.isEmpty()) {
        failMissingValue(SIBLING_ATTR);
        return;
    }
}

Task::ReportResult GTest_CheckPhyNodeHasSibling::report() {
    auto treeObj = getContext<PhyTreeObject>(this, treeContextName);
    if (treeObj == nullptr) {
        stateInfo.setError(QString("wrong value: %1").arg(treeContextName));
        return ReportResult_Finished;
    }

    const PhyNode* node = treeObj->findPhyNodeByName(nodeName);
    if (node == nullptr) {
        stateInfo.setError(QString("Node %1 not found in tree").arg(nodeName));
        return ReportResult_Finished;
    }

    bool foundSibling = false;

    const PhyBranch* parentBranch = node->getParentBranch();
    SAFE_POINT(parentBranch != nullptr, "Expected node to have a parent branch", ReportResult_Finished);
    const PhyNode* parentNode = parentBranch->parentNode;

    const QList<PhyBranch*> parentBranches = parentNode->getChildBranches();
    for (PhyBranch* branch : qAsConst(parentBranches)) {
        if ((parentNode == branch->parentNode && branch->childNode->name == siblingName) ||
            (branch->parentNode->name == siblingName && node == branch->parentNode)) {
            foundSibling = true;
            break;
        }
    }

    if (!foundSibling) {
        stateInfo.setError(QString("Node %1 doesn't have sibling %2").arg(nodeName).arg(siblingName));
        return ReportResult_Finished;
    }

    return ReportResult_Finished;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GTest_CheckPhyNodeBranchDistance::init(XMLTestFormat*, const QDomElement& el) {
    treeContextName = el.attribute(OBJ_ATTR);
    if (treeContextName.isEmpty()) {
        failMissingValue(OBJ_ATTR);
        return;
    }

    nodeName = el.attribute(PHYNODE_ATTR);
    if (nodeName.isEmpty()) {
        failMissingValue(PHYNODE_ATTR);
        return;
    }

    QString v = el.attribute(VALUE_ATTR);
    if (v.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    }

    bool ok = false;
    distance = v.toDouble(&ok);
    if (!ok) {
        failMissingValue(VALUE_ATTR);
    }
}

Task::ReportResult GTest_CheckPhyNodeBranchDistance::report() {
    if (stateInfo.hasError()) {
        return ReportResult_Finished;
    }

    auto treeObj = getContext<PhyTreeObject>(this, treeContextName);
    if (treeObj == nullptr) {
        stateInfo.setError(QString("wrong value: %1").arg(treeContextName));
        return ReportResult_Finished;
    }

    const PhyNode* node = treeObj->findPhyNodeByName(nodeName);
    if (node == nullptr) {
        stateInfo.setError(QString("Node %1 not found in tree").arg(nodeName));
        return ReportResult_Finished;
    }

    const PhyBranch* parentBranch = node->getParentBranch();
    SAFE_POINT(parentBranch != nullptr, "Expected node to have a parent branch", ReportResult_Finished);
    double chkDistance = parentBranch->distance;
    if (distance - chkDistance > EPS) {
        stateInfo.setError(QString("Distances don't match! Expected %1, real dist is %2").arg(distance).arg(chkDistance));
    }
    return ReportResult_Finished;
}

///////////////////////////////////////////////////////////////////////////////////////////

void GTest_CompareTreesInTwoObjects::init(XMLTestFormat*, const QDomElement& el) {
    docContextName = el.attribute(DOC_ATTR);
    if (docContextName.isEmpty()) {
        failMissingValue(DOC_ATTR);
        return;
    }

    secondDocContextName = el.attribute(VALUE_ATTR);
    if (secondDocContextName.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    }
}

Task::ReportResult GTest_CompareTreesInTwoObjects::report() {
    auto doc = getContext<Document>(this, docContextName);
    if (doc == nullptr) {
        stateInfo.setError(QString("document not found %1").arg(docContextName));
        return ReportResult_Finished;
    }
    auto doc2 = getContext<Document>(this, secondDocContextName);
    if (doc2 == nullptr) {
        stateInfo.setError(QString("document not found %1").arg(secondDocContextName));
        return ReportResult_Finished;
    }

    const QList<GObject*>& objs = doc->getObjects();
    const QList<GObject*>& objs2 = doc2->getObjects();
    for (int i = 0; i < objs.size() && i < objs2.size(); ++i) {
        GObject *obj = objs.at(i), *obj2 = objs2.at(i);
        PhyTreeObject *treeObj = nullptr, *treeObj2 = nullptr;
        if (obj->getGObjectType() == GObjectTypes::PHYLOGENETIC_TREE) {
            treeObj = qobject_cast<PhyTreeObject*>(obj);
        }
        if (obj2->getGObjectType() == GObjectTypes::PHYLOGENETIC_TREE) {
            treeObj2 = qobject_cast<PhyTreeObject*>(obj2);
        }
        if (treeObj == nullptr) {
            stateInfo.setError(QString("can't cast to tree from: %1 in position %2").arg(obj->getGObjectName()).arg(i));
            return ReportResult_Finished;
        }
        if (treeObj2 == nullptr) {
            stateInfo.setError(QString("can't cast to tree from: %1 in position %2").arg(obj2->getGObjectName()).arg(i));
            return ReportResult_Finished;
        }

        if (!PhyTreeObject::treesAreAlike(treeObj->getTree(), treeObj2->getTree())) {
            stateInfo.setError(QString("trees in position %1 are different").arg(i));
            return ReportResult_Finished;
        }
    }

    return ReportResult_Finished;
}

///////////////////////////////////////////////////////////////////////////////////////////

QList<XMLTestFactory*> PhyTreeObjectTests::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(GTest_CalculateTreeFromAligment::createFactory());
    res.append(GTest_CheckPhyNodeHasSibling::createFactory());
    res.append(GTest_CheckPhyNodeBranchDistance::createFactory());
    res.append(GTest_CompareTreesInTwoObjects::createFactory());

    return res;
}

}  // namespace U2
