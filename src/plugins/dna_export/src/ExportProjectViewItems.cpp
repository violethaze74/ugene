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

#include "ExportProjectViewItems.h"

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNAChromatogramObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DocumentSelection.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectSelection.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/MultiTask.h>
#include <U2Core/MultipleChromatogramAlignmentObject.h>
#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/QObjectScopedPointer.h>
#include <U2Core/SelectionModel.h>
#include <U2Core/SelectionUtils.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Formats/ExportTasks.h>

#include <U2Gui/ExportAnnotations2CSVTask.h>
#include <U2Gui/ExportObjectUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/ProjectView.h>

#include "ExportChromatogramDialog.h"
#include "ExportMSA2MSADialog.h"
#include "ExportMSA2SequencesDialog.h"
#include "ExportQualityScoresTask.h"
#include "ExportSequenceTask.h"
#include "ExportSequences2MSADialog.h"
#include "ExportSequencesDialog.h"
#include "ExportUtils.h"
#include "ImportAnnotationsFromCSVDialog.h"
#include "ImportAnnotationsFromCSVTask.h"

const char* NO_ANNOTATIONS_MESSAGE = "Selected object doesn't have annotations";
const char* MESSAGE_BOX_INFO_TITLE = "Information";

namespace U2 {

ExportProjectViewItemsContoller::ExportProjectViewItemsContoller(QObject* p)
    : QObject(p) {
    exportSequencesToSequenceFormatAction = new QAction(tr("Export sequences..."), this);
    exportSequencesToSequenceFormatAction->setObjectName(ACTION_EXPORT_SEQUENCE);
    connect(exportSequencesToSequenceFormatAction, SIGNAL(triggered()), SLOT(sl_saveSequencesToSequenceFormat()));

    exportCorrespondingSeqsAction = new QAction(tr("Export corresponding sequence..."), this);
    exportCorrespondingSeqsAction->setObjectName(ACTION_EXPORT_CORRESPONDING_SEQ);
    connect(exportCorrespondingSeqsAction, SIGNAL(triggered()), SLOT(sl_saveCorrespondingSequence()));

    exportSequencesAsAlignmentAction = new QAction(tr("Export sequences as alignment..."), this);
    exportSequencesAsAlignmentAction->setObjectName(ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT);
    connect(exportSequencesAsAlignmentAction, SIGNAL(triggered()), SLOT(sl_saveSequencesAsAlignment()));

    exportAlignmentAsSequencesAction = new QAction(tr("Export alignment to sequence format..."), this);
    exportAlignmentAsSequencesAction->setObjectName(ACTION_PROJECT__EXPORT_AS_SEQUENCES_ACTION);
    connect(exportAlignmentAsSequencesAction, SIGNAL(triggered()), SLOT(sl_saveAlignmentAsSequences()));

    exportMcaToMsaAction = new QAction(tr("Export alignment without chromatograms..."), this);
    connect(exportMcaToMsaAction, SIGNAL(triggered()), SLOT(sl_exportMcaToMsa()));

    exportNucleicAlignmentToAminoAction = new QAction(tr("Export amino acid translation..."), this);
    exportNucleicAlignmentToAminoAction->setObjectName(ACTION_PROJECT__EXPORT_TO_AMINO_ACTION);
    connect(exportNucleicAlignmentToAminoAction, SIGNAL(triggered()), SLOT(sl_exportNucleicAlignmentToAmino()));

    importAnnotationsFromCSVAction = new QAction(tr("Import annotations from CSV file..."), this);
    importAnnotationsFromCSVAction->setObjectName("import_annotations_from_CSV_file");
    connect(importAnnotationsFromCSVAction, SIGNAL(triggered()), SLOT(sl_importAnnotationsFromCSV()));

    exportDNAChromatogramAction = new QAction(tr("Export chromatogram to SCF..."), this);
    exportDNAChromatogramAction->setObjectName("action_export_chromatogram");
    connect(exportDNAChromatogramAction, SIGNAL(triggered()), SLOT(sl_exportChromatogramToSCF()));

    exportAnnotations2CSV = new QAction(tr("Export annotations..."), this);
    connect(exportAnnotations2CSV, SIGNAL(triggered()), SLOT(sl_exportAnnotations()));
    exportAnnotations2CSV->setObjectName("ep_exportAnnotations2CSV");

    exportSequenceQuality = new QAction(tr("Export sequence quality..."), this);
    connect(exportSequenceQuality, SIGNAL(triggered()), SLOT(sl_exportSequenceQuality()));

    exportObjectAction = new QAction(tr("Export object..."), this);
    exportObjectAction->setObjectName(ACTION_EXPORT_OBJECT);
    connect(exportObjectAction, SIGNAL(triggered()), SLOT(sl_exportObject()));

    ProjectView* pv = AppContext::getProjectView();
    assert(pv != nullptr);
    connect(pv, SIGNAL(si_onDocTreePopupMenuRequested(QMenu&)), SLOT(sl_addToProjectViewMenu(QMenu&)));
}

void ExportProjectViewItemsContoller::sl_addToProjectViewMenu(QMenu& m) {
    addExportImportMenu(m);
}

#define ACTION_PROJECT__EXPORT_MENU "action_project__export_menu"

void ExportProjectViewItemsContoller::addExportImportMenu(QMenu& m) {
    ProjectView* pv = AppContext::getProjectView();
    assert(pv != nullptr);
    QMenu* sub = nullptr;

    MultiGSelection ms;
    ms.addSelection(pv->getGObjectSelection());
    ms.addSelection(pv->getDocumentSelection());

    QList<GObject*> set = SelectionUtils::findObjects(GObjectTypes::SEQUENCE, &ms, UOF_LoadedOnly);
    if (!set.isEmpty()) {
        sub = new QMenu(tr("Export/Import"));
        sub->addAction(exportSequencesToSequenceFormatAction);
        sub->addAction(exportSequencesAsAlignmentAction);
        foreach (GObject* obj, set) {
            if (obj->getDocument()->getDocumentFormatId() == BaseDocumentFormats::FASTQ) {
                sub->addAction(exportSequenceQuality);
            }
        }
    } else {
        set = SelectionUtils::findObjects(GObjectTypes::MULTIPLE_SEQUENCE_ALIGNMENT, &ms, UOF_LoadedOnly);
        if (set.size() == 1) {
            sub = new QMenu(tr("Export/Import"));
            sub->addAction(exportAlignmentAsSequencesAction);
            GObject* obj = set.first();
            const MultipleSequenceAlignment& ma = qobject_cast<MultipleSequenceAlignmentObject*>(obj)->getMsa();
            if (ma->getAlphabet()->isNucleic()) {
                sub->addAction(exportNucleicAlignmentToAminoAction);
            }
        }
    }

    set = SelectionUtils::findObjects(GObjectTypes::MULTIPLE_CHROMATOGRAM_ALIGNMENT, &ms, UOF_LoadedOnly);
    if (set.size() == 1) {
        if (sub == nullptr) {
            sub = new QMenu(tr("Export/Import"));
        }
        sub->addAction(exportMcaToMsaAction);
    }

    set = SelectionUtils::findObjects(GObjectTypes::ANNOTATION_TABLE, &ms, UOF_LoadedOnly);
    if (set.size() == 1) {
        if (sub == nullptr) {
            sub = new QMenu(tr("Export/Import"));
        }
        sub->addAction(exportAnnotations2CSV);
        sub->addAction(exportCorrespondingSeqsAction);
    }

    set = SelectionUtils::findObjects(GObjectTypes::CHROMATOGRAM, &ms, UOF_LoadedOnly);
    if (set.size() == 1) {
        if (sub == nullptr) {
            sub = new QMenu(tr("Export/Import"));
        }
        sub->addAction(exportDNAChromatogramAction);
    }

    // import part
    set = SelectionUtils::findObjects(GObjectTypes::SEQUENCE, &ms, UOF_LoadedOnly);
    set.append(SelectionUtils::findObjects(GObjectTypes::ANNOTATION_TABLE, &ms, UOF_LoadedOnly));
    if (!set.isEmpty()) {
        if (sub == nullptr) {
            sub = new QMenu(tr("Export/Import"));
        }
        sub->addAction(importAnnotationsFromCSVAction);
    }

    const GSelection* s = ms.findSelectionByType(GSelectionTypes::GOBJECTS);
    const GObjectSelection* os = qobject_cast<const GObjectSelection*>(s);

    const bool exportedObjectsFound = (1 == os->getSelectedObjects().size()) &&
                                      (1 == SelectionUtils::findObjects(GObjectTypes::TEXT, &ms, UOF_LoadedOnly).size() || 1 == SelectionUtils::findObjects(GObjectTypes::VARIANT_TRACK, &ms, UOF_LoadedOnly).size() || 1 == SelectionUtils::findObjects(GObjectTypes::MULTIPLE_SEQUENCE_ALIGNMENT, &ms, UOF_LoadedOnly).size() || 1 == SelectionUtils::findObjects(GObjectTypes::PHYLOGENETIC_TREE, &ms, UOF_LoadedOnly).size() || 1 == SelectionUtils::findObjects(GObjectTypes::ASSEMBLY, &ms, UOF_LoadedOnly).size() || 1 == SelectionUtils::findObjects(GObjectTypes::MULTIPLE_CHROMATOGRAM_ALIGNMENT, &ms, UOF_LoadedOnly).size());
    if (exportedObjectsFound) {
        if (sub == nullptr) {
            sub = new QMenu(tr("Export/Import"));
        }
        sub->addAction(exportObjectAction);
    }

    if (sub != nullptr) {
        sub->setObjectName(ACTION_PROJECT__EXPORT_MENU);
        sub->menuAction()->setObjectName(ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION);
        QAction* beforeAction = GUIUtils::findActionAfter(m.actions(), PROJECT_MENU_SEPARATOR_1);
        m.insertMenu(beforeAction, sub);
    }
}

namespace {

bool hasComplementForAll(const QList<GObject*>& set) {
    foreach (GObject* o, set) {
        U2SequenceObject* so = qobject_cast<U2SequenceObject*>(o);
        if (o == nullptr || GObjectUtils::findComplementTT(so->getAlphabet()) == nullptr) {
            return false;
        }
    }
    return true;
}

bool hasAminoForAll(const QList<GObject*>& set) {
    foreach (GObject* o, set) {
        U2SequenceObject* so = qobject_cast<U2SequenceObject*>(o);
        if (o == nullptr || GObjectUtils::findAminoTT(so, false, nullptr) == nullptr) {
            return false;
        }
    }
    return true;
}

bool hasNucleicForAll(const QList<GObject*>& set) {
    foreach (GObject* o, set) {
        U2SequenceObject* so = qobject_cast<U2SequenceObject*>(o);
        if (o == nullptr || GObjectUtils::findBackTranslationTT(so) == nullptr) {
            return false;
        }
    }
    return true;
}

QList<SharedAnnotationData> getAllRelatedAnnotations(const U2SequenceObject* so, const QList<GObject*>& annotationTables) {
    QList<GObject*> relatedAnnotationTables = GObjectUtils::findObjectsRelatedToObjectByRole(so, GObjectTypes::ANNOTATION_TABLE, ObjectRole_Sequence, annotationTables, UOF_LoadedOnly);
    QList<SharedAnnotationData> anns;
    for (GObject* aObj : qAsConst(relatedAnnotationTables)) {
        auto annObj = qobject_cast<AnnotationTableObject*>(aObj);
        foreach (Annotation* ann, annObj->getAnnotations()) {
            anns.append(ann->getData());
        }
    }
    return anns;
}

void addExportItemsToSettings(ExportSequencesDialog* d, const QList<GObject*> seqObjs, ExportSequenceTaskSettings& s) {
    QList<GObject*> allAnnotationTables;
    if (s.saveAnnotations) {
        allAnnotationTables = GObjectUtils::findAllObjects(UOF_LoadedOnly, GObjectTypes::ANNOTATION_TABLE);
    }
    foreach (GObject* o, seqObjs) {
        U2SequenceObject* so = qobject_cast<U2SequenceObject*>(o);
        SAFE_POINT(so != nullptr, "Invalid sequence object", );
        QList<SharedAnnotationData> anns;
        if (s.saveAnnotations) {
            anns = getAllRelatedAnnotations(so, allAnnotationTables);
        }
        ExportSequenceItem ei;
        ei.setSequenceInfo(so);
        ei.annotations = anns;
        ei.complTT = GObjectUtils::findComplementTT(so->getAlphabet());
        ei.aminoTT = d->translate ? GObjectUtils::findAminoTT(so, false, d->useSpecificTable ? d->translationTable : nullptr) : nullptr;
        ei.backTT = d->backTranslate ? GObjectUtils::findBackTranslationTT(so, d->translationTable) : nullptr;
        s.items.append(ei);
    }
}

}  // namespace

void ExportProjectViewItemsContoller::sl_saveSequencesToSequenceFormat() {
    ProjectView* pv = AppContext::getProjectView();
    SAFE_POINT(pv != nullptr, "Project view is NULL", );

    MultiGSelection ms;
    ms.addSelection(pv->getGObjectSelection());
    ms.addSelection(pv->getDocumentSelection());
    QList<GObject*> set = SelectionUtils::findObjects(GObjectTypes::SEQUENCE, &ms, UOF_LoadedOnly);
    if (set.isEmpty()) {
        QMessageBox::critical(nullptr, tr(MESSAGE_BOX_INFO_TITLE), tr("There are no sequence objects selected."));
        return;
    }

    exportSequences(set);
}

void ExportProjectViewItemsContoller::sl_saveCorrespondingSequence() {
    ProjectView* pv = AppContext::getProjectView();
    SAFE_POINT(pv != nullptr, "Project view is NULL", );

    MultiGSelection ms;
    ms.addSelection(pv->getGObjectSelection());
    ms.addSelection(pv->getDocumentSelection());
    const QList<GObject*> annotTables = SelectionUtils::findObjects(GObjectTypes::ANNOTATION_TABLE, &ms, UOF_LoadedOnly);
    if (annotTables.isEmpty()) {
        QMessageBox::critical(nullptr, tr(MESSAGE_BOX_INFO_TITLE), tr("There is no annotation table selected."));
        return;
    }

    GObject* seqObj = nullptr;
    foreach (const GObjectRelation& relation, annotTables.first()->getObjectRelations()) {
        if (ObjectRole_Sequence == relation.role) {
            seqObj = GObjectUtils::selectObjectByReference(relation.ref, UOF_LoadedOnly);
            break;
        }
    }

    if (seqObj == nullptr) {
        QMessageBox::information(nullptr, tr(MESSAGE_BOX_INFO_TITLE), tr("There is no associated sequence found."));
        return;
    }

    exportSequences(QList<GObject*>() << seqObj);
}

void ExportProjectViewItemsContoller::exportSequences(const QList<GObject*>& seqs) {
    CHECK(!seqs.isEmpty(), );

    bool allowMerge = seqs.size() > 1;
    bool allowComplement = hasComplementForAll(seqs);
    bool allowTranslate = hasAminoForAll(seqs);
    bool allowBackTranslate = hasNucleicForAll(seqs);

    QString defaultFileNameDir;
    QString fileBaseName;
    GUrlUtils::getLocalPathFromUrl((*seqs.constBegin())->getDocument()->getURL(), (*seqs.constBegin())->getGObjectName(), defaultFileNameDir, fileBaseName);

    QString defaultFileName = defaultFileNameDir + QDir::separator() + fileBaseName + "_new.fa";
    QObjectScopedPointer<ExportSequencesDialog> d = new ExportSequencesDialog(allowMerge,
                                                                              allowComplement,
                                                                              allowTranslate,
                                                                              allowBackTranslate,
                                                                              defaultFileName,
                                                                              fileBaseName,
                                                                              BaseDocumentFormats::FASTA,
                                                                              AppContext::getMainWindow()->getQMainWindow());

    const int rc = d->exec();
    CHECK(!d.isNull(), );

    if (rc == QDialog::Rejected) {
        return;
    }
    SAFE_POINT(!d->file.isEmpty(), "Invalid file name detected", );

    ExportSequenceTaskSettings s;
    ExportUtils::loadDNAExportSettingsFromDlg(s, d.data());
    addExportItemsToSettings(d.data(), seqs, s);

    Task* t = ExportUtils::wrapExportTask(new ExportSequenceTask(s), d->addToProject);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void ExportProjectViewItemsContoller::sl_saveSequencesAsAlignment() {
    ProjectView* pv = AppContext::getProjectView();
    assert(pv != nullptr);

    MultiGSelection ms;
    ms.addSelection(pv->getGObjectSelection());
    ms.addSelection(pv->getDocumentSelection());
    QList<GObject*> sequenceObjects = SelectionUtils::findObjectsKeepOrder(GObjectTypes::SEQUENCE, &ms, UOF_LoadedOnly);
    if (sequenceObjects.isEmpty()) {
        QMessageBox::critical(nullptr, L10N::errorTitle(), tr("No sequence objects selected!"));
        return;
    }

    // Check sequence length does not exceed MSA size that UGENE can handle safely.
    // UGENE often loads MSA into memory in full and huge MSA may crash UGENE.
    qint64 totalSequenceSize = 0;
    for (const GObject* obj : sequenceObjects) {
        if (auto sequenceObject = qobject_cast<const U2SequenceObject*>(obj)) {
            totalSequenceSize += sequenceObject->getSequenceLength();
        }
    }
    if (totalSequenceSize >= ExportAlignmentTask::MAX_SAFE_ALIGNMENT_SIZE_TO_EXPORT) {
        QMessageBox::critical(nullptr, L10N::errorTitle(), tr("Sequence is too large to be exported as a multiple alignment"));
        return;
    }

    GUrl seqUrl = sequenceObjects.first()->getDocument()->getURL();
    QString defaultUrl = GUrlUtils::getNewLocalUrlByFormat(seqUrl, sequenceObjects.first()->getGObjectName(), BaseDocumentFormats::CLUSTAL_ALN, "");

    QObjectScopedPointer<ExportSequences2MSADialog> d = new ExportSequences2MSADialog(AppContext::getMainWindow()->getQMainWindow(), defaultUrl);
    const int rc = d->exec();
    CHECK(!d.isNull(), );

    if (rc != QDialog::Accepted) {
        return;
    }

    U2OpStatusImpl os;
    MemoryLocker memoryLocker(os);

    // checking memory consumption
    foreach (GObject* obj, sequenceObjects) {
        U2SequenceObject* dnaObj = qobject_cast<U2SequenceObject*>(obj);
        if (dnaObj == nullptr) {
            continue;
        }
        memoryLocker.tryAcquire(dnaObj->getSequenceLength());
    }
    if (os.hasError()) {
        QMessageBox::critical(nullptr, L10N::errorTitle(), tr("Not enough memory"));
        return;
    }

    MultipleSequenceAlignment ma = MSAUtils::seq2ma(sequenceObjects, os, d->useGenbankHeader);
    if (os.hasError()) {
        QMessageBox::critical(nullptr, L10N::errorTitle(), os.getError());
        return;
    }
    QString objName = GUrl(d->url).baseFileName();
    ma->setName(objName);
    Task* t = ExportUtils::wrapExportTask(new ExportAlignmentTask(ma, d->url, d->format), d->addToProjectFlag);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void ExportProjectViewItemsContoller::sl_saveAlignmentAsSequences() {
    ProjectView* pv = AppContext::getProjectView();
    SAFE_POINT(pv != nullptr, "Project is null", );

    MultiGSelection ms;
    ms.addSelection(pv->getGObjectSelection());
    ms.addSelection(pv->getDocumentSelection());
    QList<GObject*> set = SelectionUtils::findObjects(GObjectTypes::MULTIPLE_SEQUENCE_ALIGNMENT, &ms, UOF_LoadedOnly);
    if (set.size() != 1) {
        QMessageBox::critical(nullptr, L10N::errorTitle(), tr("Select one alignment object to export"));
        return;
    }
    auto msaObject = qobject_cast<MultipleSequenceAlignmentObject*>(set.first());
    SAFE_POINT(msaObject != nullptr, "Not MSA object!", );
    ExportMSA2SequencesDialog::showDialogAndStartExportTask(msaObject);
}

void ExportProjectViewItemsContoller::sl_exportMcaToMsa() {
    ProjectView* projectView = AppContext::getProjectView();
    SAFE_POINT(projectView != nullptr, "Project View is NULL", );

    MultiGSelection ms;
    ms.addSelection(projectView->getGObjectSelection());
    ms.addSelection(projectView->getDocumentSelection());

    QList<GObject*> set = SelectionUtils::findObjects(GObjectTypes::MULTIPLE_CHROMATOGRAM_ALIGNMENT, &ms, UOF_LoadedOnly);
    if (set.size() != 1) {
        QMessageBox::critical(nullptr, L10N::errorTitle(), tr("Select one chromatogram alignment object to export"));
        return;
    }

    MultipleChromatogramAlignmentObject* mcaObject = qobject_cast<MultipleChromatogramAlignmentObject*>(set.first());
    SAFE_POINT(mcaObject != nullptr, "Can't cast the object to MultipleChromatogramAlignmentObject", );
    ExportUtils::launchExportMca2MsaTask(mcaObject);
}

void ExportProjectViewItemsContoller::sl_exportNucleicAlignmentToAmino() {
    ProjectView* pv = AppContext::getProjectView();
    assert(pv != nullptr);

    MultiGSelection ms;
    ms.addSelection(pv->getGObjectSelection());
    ms.addSelection(pv->getDocumentSelection());
    QList<GObject*> msaObjectList = SelectionUtils::findObjects(GObjectTypes::MULTIPLE_SEQUENCE_ALIGNMENT, &ms, UOF_LoadedOnly);
    if (msaObjectList.size() != 1) {
        QMessageBox::critical(nullptr, L10N::errorTitle(), tr("Select one alignment object to export"));
        return;
    }

    auto msaObject = qobject_cast<MultipleSequenceAlignmentObject*>(msaObjectList.first());
    SAFE_POINT(msaObject != nullptr, "Not an MSA object", );

    Document* doc = msaObject->getDocument();
    QString defaultUrl = GUrlUtils::getNewLocalUrlByFormat(doc->getURL(), msaObject->getMsa()->getName(), BaseDocumentFormats::CLUSTAL_ALN, "_transl");

    QObjectScopedPointer<ExportMSA2MSADialog> d = new ExportMSA2MSADialog(defaultUrl, BaseDocumentFormats::CLUSTAL_ALN, true, AppContext::getMainWindow()->getQMainWindow());
    const int rc = d->exec();
    CHECK(!d.isNull() && rc != QDialog::Rejected, );

    const MultipleSequenceAlignment& msa = msaObject->getMsa();
    DNATranslation* translation = AppContext::getDNATranslationRegistry()->lookupTranslation(d->translationTable);
    bool convertUnknowToGaps = d->unknownAmino == ExportMSA2MSADialog::UnknownAmino::Gap;
    bool reverseComplement = d->translationFrame < 0;
    int offset = qAbs(d->translationFrame) - 1;
    auto exportTask = ExportUtils::wrapExportTask(new ExportMSA2MSATask(msa,
                                                                        msa->getRowsIds(),
                                                                        {0, msa->getLength()},
                                                                        d->file,
                                                                        translation,
                                                                        d->formatId,
                                                                        !d->includeGaps,
                                                                        convertUnknowToGaps,
                                                                        reverseComplement,
                                                                        offset),
                                                  d->addToProjectFlag);
    AppContext::getTaskScheduler()->registerTopLevelTask(exportTask);
}

void ExportProjectViewItemsContoller::sl_importAnnotationsFromCSV() {
    QObjectScopedPointer<ImportAnnotationsFromCSVDialog> d = new ImportAnnotationsFromCSVDialog(AppContext::getMainWindow()->getQMainWindow());
    const int rc = d->exec();
    CHECK(!d.isNull(), );

    if (rc != QDialog::Accepted) {
        return;
    }
    ImportAnnotationsFromCSVTaskConfig taskConfig;
    d->toTaskConfig(taskConfig);
    ImportAnnotationsFromCSVTask* task = new ImportAnnotationsFromCSVTask(taskConfig);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
}

void ExportProjectViewItemsContoller::sl_exportChromatogramToSCF() {
    ProjectView* pv = AppContext::getProjectView();
    assert(pv != nullptr);

    MultiGSelection ms;
    ms.addSelection(pv->getGObjectSelection());
    ms.addSelection(pv->getDocumentSelection());
    QList<GObject*> set = SelectionUtils::findObjects(GObjectTypes::CHROMATOGRAM, &ms, UOF_LoadedOnly);
    if (set.size() != 1) {
        QMessageBox::warning(nullptr, L10N::errorTitle(), tr("Select one chromatogram object to export"));
        return;
    }
    GObject* obj = set.first();
    auto chromaObj = qobject_cast<DNAChromatogramObject*>(obj);
    assert(chromaObj != nullptr);

    QObjectScopedPointer<ExportChromatogramDialog> d = new ExportChromatogramDialog(QApplication::activeWindow(), chromaObj->getDocument()->getURL());
    const int rc = d->exec();
    CHECK(!d.isNull(), );

    if (rc == QDialog::Rejected) {
        return;
    }

    ExportChromatogramTaskSettings settings;
    settings.url = d->url;
    settings.complement = d->complemented;
    settings.reverse = d->reversed;
    settings.loadDocument = d->addToProjectFlag;

    Task* task = ExportUtils::wrapExportTask(new ExportDNAChromatogramTask(chromaObj, settings), d->addToProjectFlag);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
}

void ExportProjectViewItemsContoller::sl_exportAnnotations() {
    // find annotations: whole annotation file
    ProjectView* pv = AppContext::getProjectView();
    assert(pv != nullptr);

    MultiGSelection ms;
    ms.addSelection(pv->getGObjectSelection());
    ms.addSelection(pv->getDocumentSelection());

    QList<GObject*> set = SelectionUtils::findObjects(GObjectTypes::ANNOTATION_TABLE, &ms, UOF_LoadedOnly);
    if (set.size() != 1) {
        QMessageBox::warning(QApplication::activeWindow(), exportAnnotations2CSV->text(), tr("Select one annotation object to export"));
        return;
    }

    GObject* obj = set.first();
    auto aObj = qobject_cast<AnnotationTableObject*>(obj);
    SAFE_POINT(aObj != nullptr, "Invalid annotation table detected!", );
    if (!aObj->getAnnotations().isEmpty()) {
        SAFE_POINT(aObj->getDocument() != nullptr, "Invalid document detected!", );
        ExportObjectUtils::exportAnnotations(aObj, aObj->getDocument()->getURL());
        return;
    }
    QMessageBox::warning(QApplication::activeWindow(), exportAnnotations2CSV->text(), tr(NO_ANNOTATIONS_MESSAGE));
}

void ExportProjectViewItemsContoller::sl_exportSequenceQuality() {
    ProjectView* pv = AppContext::getProjectView();
    assert(pv != nullptr);

    MultiGSelection ms;
    ms.addSelection(pv->getGObjectSelection());
    ms.addSelection(pv->getDocumentSelection());
    QList<GObject*> sequenceObjects = SelectionUtils::findObjectsKeepOrder(GObjectTypes::SEQUENCE, &ms, UOF_LoadedOnly);
    if (sequenceObjects.isEmpty()) {
        QMessageBox::critical(nullptr, L10N::errorTitle(), tr("No sequence objects selected!"));
        return;
    }

    LastUsedDirHelper lod;
    lod.url = U2FileDialog::getSaveFileName(QApplication::activeWindow(), tr("Set output quality file"), lod.dir, ".qual");
    if (lod.url.isEmpty()) {
        return;
    }

    QList<Task*> exportTasks;
    foreach (GObject* gObj, sequenceObjects) {
        if (gObj->getDocument()->getDocumentFormatId() != BaseDocumentFormats::FASTQ) {
            continue;
        }
        auto seqObj = qobject_cast<U2SequenceObject*>(gObj);
        ExportQualityScoresConfig cfg;
        cfg.dstFilePath = lod.url;
        Task* exportTask = new ExportPhredQualityScoresTask(seqObj, cfg);
        exportTasks.append(exportTask);
    }

    Task* t = new MultiTask("ExportQualityScoresFromProjectView", exportTasks);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void ExportProjectViewItemsContoller::sl_exportObject() {
    ProjectView* pv = AppContext::getProjectView();
    SAFE_POINT(pv != nullptr, "Invalid project view detected!", );

    const GObjectSelection* selection = pv->getGObjectSelection();
    CHECK(!selection->isEmpty(), );

    LastUsedDirHelper dirHelper;
    const GObject* original = selection->getSelectedObjects().first();
    U2OpStatusImpl os;
    const U2DbiRef dbiRef = AppContext::getDbiRegistry()->getSessionTmpDbiRef(os);
    SAFE_POINT_OP(os, );
    GObject* copied = original->clone(dbiRef, os);
    SAFE_POINT_OP(os, );

    const QString savePath = dirHelper.getLastUsedDir(QString(), QDir::homePath()) + QDir::separator() + copied->getGObjectName();

    ExportObjectUtils::exportObject2Document(copied, savePath);
}

}  // namespace U2
