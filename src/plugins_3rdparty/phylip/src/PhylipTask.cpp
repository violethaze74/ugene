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
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/PhyTreeObject.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2SafePoints.h>
#include "NeighborJoinAdapter.h"

#include "PhylipTask.h"

namespace U2 {

PhylipTask::PhylipTask(const QString &msaUrl, const QString &treeUrl, const CreatePhyTreeSettings &settings)
: CmdlineTask(tr("PHYLIP task"), TaskFlags_NR_FOSE_COSC), msaUrl(msaUrl), treeUrl(treeUrl), settings(settings), loadTask(NULL), treeTask(NULL)
{
}

void PhylipTask::prepare() {
    loadTask = LoadDocumentTask::getDefaultLoadDocTask(msaUrl);
    if (NULL == loadTask) {
        setError(tr("Cannot detect alignment file format."));
        return;
    }
    addSubTask(loadTask);
}

QList<Task*> PhylipTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> result;
    if (loadTask == subTask) {
        MAlignmentObject *msaObject = getLoadedAlignment();
        CHECK_OP(stateInfo, result);
        treeTask = new NeighborJoinCalculateTreeTask(msaObject->getMAlignment(), settings);
        result << treeTask;
    } else if (treeTask == subTask) {
        Task *saveTask = createSaveTreeTask();
        CHECK_OP(stateInfo, result);
        result << saveTask;
    }
    return result;
}

Task::ReportResult PhylipTask::report() {
    CmdlineTask::report();
    return ReportResult_Finished;
}

MAlignmentObject * PhylipTask::getLoadedAlignment() {
    Document *doc = loadTask->getDocument();
    QList<GObject*> objects = doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
    if (objects.isEmpty()) {
        setError(tr("There are no multiple alignments in the file"));
        return NULL;
    }

    return qobject_cast<MAlignmentObject*>(objects.first());
}

Task * PhylipTask::createSaveTreeTask() {
    PhyTree tree = treeTask->getResult();
    U2DbiRef dbiRef = AppContext::getDbiRegistry()->getSessionTmpDbiRef(stateInfo);
    CHECK_OP(stateInfo, NULL);

    QList<GObject*> objects;
    objects << PhyTreeObject::createInstance(tree, "tree", dbiRef, stateInfo);
    CHECK_OP(stateInfo, NULL);

    Document *doc = new Document(BaseDocumentFormats::get(BaseDocumentFormats::NEWICK), IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), treeUrl, U2DbiRef(), objects);
    doc->setParent(this);

    return new SaveDocumentTask(doc);
}

} // U2
