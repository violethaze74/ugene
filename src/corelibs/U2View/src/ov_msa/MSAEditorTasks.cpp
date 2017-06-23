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

#include "MSAEditorTasks.h"
#include "MSAEditor.h"
#include "MSAEditorState.h"
#include "MSAEditorConsensusArea.h"

#include "MaEditorFactory.h"
#include "McaEditor.h" // SANGER_TODO: deal with includes

#include <U2Algorithm/MSAConsensusAlgorithm.h>

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/MultipleChromatogramAlignmentObject.h>
#include <U2Core/TextObject.h>
#include <U2Core/UnloadedObject.h>

#include <U2Gui/OpenViewTask.h>

#include <U2Formats/DocumentFormatUtils.h>

#include <QSet>

namespace U2 {

/* TRANSLATOR U2::MSAEditor */
/* TRANSLATOR U2::ObjectViewTask */

//////////////////////////////////////////////////////////////////////////
/// open new view

OpenMaEditorTask::OpenMaEditorTask(MultipleAlignmentObject* _obj, GObjectViewFactoryId fid, GObjectType type)
    : ObjectViewTask(fid),
      type(type),
      maObject(_obj)
{
    assert(!maObject.isNull());
}

OpenMaEditorTask::OpenMaEditorTask(UnloadedObject* _obj, GObjectViewFactoryId fid, GObjectType type)
    : ObjectViewTask(fid),
      type(type),
      unloadedReference(_obj)
{
    assert(_obj->getLoadedObjectType() == type);
    documentsToLoad.append(_obj->getDocument());
}

OpenMaEditorTask::OpenMaEditorTask(Document* doc, GObjectViewFactoryId fid, GObjectType type)
    : ObjectViewTask(fid),
      type(type),
      maObject(NULL)
{
    assert(!doc->isLoaded());
    documentsToLoad.append(doc);
}

void OpenMaEditorTask::open() {
    if (stateInfo.hasError() || (maObject.isNull() && documentsToLoad.isEmpty())) {
        return;
    }
    if (maObject.isNull()) {
        Document* doc = documentsToLoad.first();
        if(!doc){
            stateInfo.setError(tr("Documet removed from project"));
            return;
        }
        if (unloadedReference.isValid()) {
            GObject* obj = doc->findGObjectByName(unloadedReference.objName);
            if (obj!=NULL && obj->getGObjectType() == type) {
                maObject = qobject_cast<MultipleAlignmentObject*>(obj);
            }
        } else {
            QList<GObject*> objects = doc->findGObjectByType(type, UOF_LoadedAndUnloaded);
            maObject = objects.isEmpty() ? NULL : qobject_cast<MultipleAlignmentObject*>(objects.first());
        }
        if (maObject.isNull()) {
            stateInfo.setError(tr("Multiple alignment object not found"));
            return;
        }
    }
    viewName = GObjectViewUtils::genUniqueViewName(maObject->getDocument(), maObject);
    uiLog.details(tr("Opening MSA editor for object: %1").arg(maObject->getGObjectName()));

    MaEditor* v = getEditor(viewName, maObject);
    GObjectViewWindow* w = new GObjectViewWindow(v, viewName, false);
    MWMDIManager* mdiManager = AppContext::getMainWindow()->getMDIManager();
    mdiManager->addMDIWindow(w);

}

void OpenMaEditorTask::updateTitle(MSAEditor* msaEd) {
    const QString& oldViewName = msaEd->getName();
    GObjectViewWindow* w = GObjectViewUtils::findViewByName(oldViewName);
    if (w != NULL) {
        MultipleAlignmentObject* msaObject = msaEd->getMaObject();
        QString newViewName = GObjectViewUtils::genUniqueViewName(msaObject->getDocument(), msaObject);
        msaEd->setName(newViewName);
        w->setWindowTitle(newViewName);
    }
}

OpenMsaEditorTask::OpenMsaEditorTask(MultipleAlignmentObject* obj)
    : OpenMaEditorTask(obj, MsaEditorFactory::ID, GObjectTypes::MULTIPLE_SEQUENCE_ALIGNMENT)
{
}

OpenMsaEditorTask::OpenMsaEditorTask(UnloadedObject* obj)
    : OpenMaEditorTask(obj, MsaEditorFactory::ID, GObjectTypes::MULTIPLE_SEQUENCE_ALIGNMENT)
{
}

OpenMsaEditorTask::OpenMsaEditorTask(Document* doc)
    : OpenMaEditorTask(doc, MsaEditorFactory::ID, GObjectTypes::MULTIPLE_SEQUENCE_ALIGNMENT)
{
}

MaEditor* OpenMsaEditorTask::getEditor(const QString& viewName, GObject* obj) {
    return MsaEditorFactory::getEditor(viewName, obj);
}

OpenMcaEditorTask::OpenMcaEditorTask(MultipleAlignmentObject* obj)
    : OpenMaEditorTask(obj, McaEditorFactory::ID, GObjectTypes::MULTIPLE_CHROMATOGRAM_ALIGNMENT)
{
}

OpenMcaEditorTask::OpenMcaEditorTask(UnloadedObject* obj)
    : OpenMaEditorTask(obj, McaEditorFactory::ID, GObjectTypes::MULTIPLE_CHROMATOGRAM_ALIGNMENT)
{
}

OpenMcaEditorTask::OpenMcaEditorTask(Document* doc)
    : OpenMaEditorTask(doc, McaEditorFactory::ID, GObjectTypes::MULTIPLE_CHROMATOGRAM_ALIGNMENT)
{
}

MaEditor* OpenMcaEditorTask::getEditor(const QString& viewName, GObject* obj) {
    QList<GObjectRelation> relations = obj->findRelatedObjectsByRole(ObjectRole_ReferenceSequence);
    SAFE_POINT(relations.size() <= 1, "Wrong amount of reference sequences", NULL);
    if (!relations.isEmpty()) {
        GObjectRelation r = relations.first();
        GObject* ref = GObjectUtils::selectObjectByReference(r.ref, UOF_LoadedAndUnloaded);
        if (ref != NULL) {
            return McaEditorFactory::getEditor(viewName, obj, ref);
        }
    }
    return McaEditorFactory::getEditor(viewName, obj);
}

//////////////////////////////////////////////////////////////////////////
// open view from state

OpenSavedMSAEditorTask::OpenSavedMSAEditorTask(GObjectType type, MaEditorFactory* factory,
                                               const QString& viewName, const QVariantMap& stateData)
    : ObjectViewTask(factory->getId(), viewName, stateData),
      type(type),
      factory(factory)
{
    MSAEditorState state(stateData);
    GObjectReference ref = state.getMSAObjectRef();
    Document* doc = AppContext::getProject()->findDocumentByURL(ref.docUrl);
    if (doc == NULL) {
        doc = createDocumentAndAddToProject(ref.docUrl, AppContext::getProject(), stateInfo);
        CHECK_OP_EXT(stateInfo, stateIsIllegal = true ,);
    }
    if (!doc->isLoaded()) {
        documentsToLoad.append(doc);
    }

}

void OpenSavedMSAEditorTask::open() {
    CHECK_OP(stateInfo, );

    MSAEditorState state(stateData);
    GObjectReference ref = state.getMSAObjectRef();
    Document* doc = AppContext::getProject()->findDocumentByURL(ref.docUrl);
    if (doc == NULL) {
        stateIsIllegal = true;
        stateInfo.setError(L10N::errorDocumentNotFound(ref.docUrl));
        return;
    }
    GObject* obj = NULL;
    if (doc->isDatabaseConnection() && ref.entityRef.isValid()) {
        obj = doc->getObjectById(ref.entityRef.entityId);
    } else {
        obj = doc->findGObjectByName(ref.objName);
    }
    if (obj == NULL || obj->getGObjectType() != type) {
        stateIsIllegal = true;
        stateInfo.setError(tr("Alignment object not found: %1").arg(ref.objName));
        return;
    }
    MultipleSequenceAlignmentObject* msaObject = qobject_cast<MultipleSequenceAlignmentObject*>(obj);
    assert(msaObject!=NULL);

    MaEditor* v = factory->getEditor(viewName, msaObject); // SANGER_TODO: rename to maObject
    GObjectViewWindow* w = new GObjectViewWindow(v, viewName, true);
    MWMDIManager* mdiManager =     AppContext::getMainWindow()->getMDIManager();
    mdiManager->addMDIWindow(w);

    updateRanges(stateData, v);
}

void OpenSavedMSAEditorTask::updateRanges(const QVariantMap& stateData, MaEditor* ctx) {
    Q_UNUSED(ctx);
    MSAEditorState state(stateData);

    QFont f = state.getFont();
    if (!f.isCopyOf(QFont())) {
        ctx->setFont(f);
    }

    int firstPos = state.getFirstPos();
    ctx->setFirstVisibleBase(firstPos); // SCROLLING_TODO: replace base number with offset

    float zoomFactor = state.getZoomFactor();
    ctx->setZoomFactor(zoomFactor);
}


//////////////////////////////////////////////////////////////////////////
// update
UpdateMSAEditorTask::UpdateMSAEditorTask(GObjectView* v, const QString& stateName, const QVariantMap& stateData)
: ObjectViewTask(v, stateName, stateData)
{
}

void UpdateMSAEditorTask::update() {
    // SANGER_TODO: if this valid?
    if (view.isNull() || (view->getFactoryId() != MsaEditorFactory::ID && view->getFactoryId() != McaEditorFactory::ID)) {
        return; //view was closed;
    }

    MSAEditor* msaView = qobject_cast<MSAEditor*>(view.data());
    assert(msaView!=NULL);

    OpenSavedMSAEditorTask::updateRanges(stateData, msaView);
}


ExportMSAConsensusTask::ExportMSAConsensusTask(const ExportMSAConsensusTaskSettings& s )
: DocumentProviderTask(tr("Export consensus to MSA")
, (TaskFlags(TaskFlag_NoRun) | TaskFlag_FailOnSubtaskError | TaskFlag_CancelOnSubtaskCancel))
, settings(s), extractConsensus(NULL) {
    setVerboseLogMode(true);
    SAFE_POINT_EXT(s.msa != NULL, setError("Given msa pointer is NULL"), );
}

void ExportMSAConsensusTask::prepare(){
    extractConsensus = new ExtractConsensusTask(settings.keepGaps, settings.msa);
    addSubTask(extractConsensus);
}

QList<Task*> ExportMSAConsensusTask::onSubTaskFinished( Task* subTask ){
    QList<Task*> result;
    if(subTask == extractConsensus && !isCanceled() && !hasError()) {
        Document *takenDoc = createDocument();
        CHECK_OP(stateInfo, result);
        SaveDocumentTask *saveTask = new SaveDocumentTask(takenDoc, takenDoc->getIOAdapterFactory(), takenDoc->getURL());
        saveTask->addFlag(SaveDoc_Overwrite);
        Project *proj = AppContext::getProject();
        if(proj != NULL){
            if(proj->findDocumentByURL(takenDoc->getURL()) != NULL){
                result.append(saveTask);
                return result;
            }
        }
        saveTask->addFlag(SaveDoc_OpenAfter);
        result.append(saveTask);
    }
    return result;
}

Document *ExportMSAConsensusTask::createDocument(){
    filteredConsensus = extractConsensus->getExtractedConsensus();
    CHECK_EXT(!filteredConsensus.isEmpty(), setError("Consensus is empty!"), NULL);
    QString fullPath = GUrlUtils::prepareFileLocation(settings.url, stateInfo);
    CHECK_OP(stateInfo, NULL);
    GUrl url(fullPath);

    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(settings.url));
    DocumentFormat *df = AppContext::getDocumentFormatRegistry()->getFormatById(settings.format);
    CHECK_EXT(df, setError("Document format is NULL!"), NULL);
    GObject *obj = NULL;
    QScopedPointer<Document> doc(df->createNewLoadedDocument(iof, fullPath, stateInfo));
    CHECK_OP(stateInfo, NULL);
    if (df->getFormatId() == BaseDocumentFormats::PLAIN_TEXT){
        obj = TextObject::createInstance(filteredConsensus, settings.name, doc->getDbiRef(), stateInfo);
    }else{
        obj = DocumentFormatUtils::addSequenceObject(doc->getDbiRef(), settings.name, filteredConsensus, false, QVariantMap(), stateInfo);
    }
    CHECK_OP(stateInfo, NULL);
    doc->addObject(obj);
    return doc.take();
}

ExtractConsensusTask::ExtractConsensusTask( bool keepGaps_, MSAEditor* msa_ )
: Task(tr("Export consensus to MSA"), TaskFlags(TaskFlag_None)),
keepGaps(keepGaps_), msa(msa_){
    setVerboseLogMode(true);
    SAFE_POINT_EXT(msa != NULL, setError("Given msa pointer is NULL"), );
}

void ExtractConsensusTask::run() {
    CHECK(msa->getUI(), );
    CHECK(msa->getUI()->getConsensusArea(), );
    CHECK(msa->getUI()->getConsensusArea()->getConsensusCache(),);

    MSAConsensusAlgorithm *algorithm = msa->getUI()->getConsensusArea()->getConsensusAlgorithm();
    const MultipleAlignment ma = msa->getMaObject()->getMultipleAlignmentCopy();
    for (int i = 0, n = ma->getLength(); i < n; i++) {
        if (stateInfo.isCoR()) {
            return;
        }
        int count = 0;
        int nSeq = ma->getNumRows();
        SAFE_POINT(0 != nSeq, tr("No sequences in alignment"), );

        QChar c = algorithm->getConsensusCharAndScore(ma, i, count);
        if (c != U2Msa::GAP_CHAR || keepGaps) {
            filteredConsensus.append(c);
        }
    }
}

const QByteArray& ExtractConsensusTask::getExtractedConsensus() const {
    return filteredConsensus;
}


ExportMSAConsensusTaskSettings::ExportMSAConsensusTaskSettings(): keepGaps(true), msa(NULL),
format(BaseDocumentFormats::PLAIN_TEXT)
{}

} // namespace
