/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
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

#include <QMessageBox>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/CmdlineInOutTaskRunner.h>
#include <U2Core/Counter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/OpenViewTask.h>
#include <U2Gui/SaveDocumentController.h>
#include <U2Gui/U2FileDialog.h>
#include <U2Gui/U2WidgetStateStorage.h>

#include "AlignToReferenceBlastDialog.h"

namespace U2 {

AlignToReferenceBlastCmdlineTask::Settings::Settings()
    : minIdentity(60),
      minLength(0),
      qualityThreshold(30),
      trimBothEnds(true),
      addResultToProject(true)
{

}

const QString AlignToReferenceBlastCmdlineTask::ALIGN_TO_REF_CMDLINE = "align-to-reference";

const QString AlignToReferenceBlastCmdlineTask::TRIM_ARG = "trim-both-ends";
const QString AlignToReferenceBlastCmdlineTask::MIN_LEN_ARG = "min-length";
const QString AlignToReferenceBlastCmdlineTask::THRESHOLD_ARG = "threshold";

const QString AlignToReferenceBlastCmdlineTask::READS_ARG = "reads";

const QString AlignToReferenceBlastCmdlineTask::MIN_IDENTITY_ARG = "min-identity";
const QString AlignToReferenceBlastCmdlineTask::REF_ARG = "reference";
const QString AlignToReferenceBlastCmdlineTask::RESULT_ALIGNMENT_ARG = "result-url";

AlignToReferenceBlastCmdlineTask::AlignToReferenceBlastCmdlineTask(const Settings &settings)
    : Task(tr("Align Sanger reads to reference"), TaskFlags_FOSE_COSC | TaskFlag_MinimizeSubtaskErrorText | TaskFlag_ReportingIsEnabled | TaskFlag_ReportingIsSupported),
      settings(settings),
      cmdlineTask(NULL),
      loadRef(NULL),
      reportFile(AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath() + "/align_to_ref_XXXXXX.txt")
{
    GCOUNTER(cvar, tvar, "AlignToReferenceBlastCmdlineTask");
}

void AlignToReferenceBlastCmdlineTask::prepare() {
    AppContext::getAppSettings()->getUserAppsSettings()->createCurrentProcessTemporarySubDir(stateInfo);
    const bool opened = reportFile.open();
    SAFE_POINT_EXT(opened, setError(L10N::errorOpeningFileWrite(reportFile.fileName())), );
    reportFile.close();

    FormatDetectionConfig config;
    QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(settings.referenceUrl, config);
    CHECK_EXT(!formats.isEmpty() && (NULL != formats.first().format), setError(tr("wrong reference format")), );

    DocumentFormat *format = formats.first().format;
    CHECK_EXT(format->getSupportedObjectTypes().contains(GObjectTypes::SEQUENCE), setError(tr("wrong reference format")), );

    loadRef = new LoadDocumentTask(format->getFormatId(),
        settings.referenceUrl, AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(settings.referenceUrl)));
    addSubTask(loadRef);
}

QString AlignToReferenceBlastCmdlineTask::generateReport() const {
    return reportString;
}

QList<Task*> AlignToReferenceBlastCmdlineTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> result;
    CHECK(subTask != NULL, result);
    CHECK(!subTask->isCanceled() && !subTask->hasError(), result);
    if (loadRef == subTask) {
        CHECK_EXT(loadRef->getDocument(false) != NULL, setError(tr("Loaded reference document is NULL")), result);
        CHECK_EXT(!loadRef->getDocument(false)->findGObjectByType(GObjectTypes::SEQUENCE).isEmpty(), setError(tr("No sequence objects in reference document")), result);
        CHECK_EXT(loadRef->getDocument(false)->findGObjectByType(GObjectTypes::SEQUENCE).size() == 1, setError(tr("'%1' has invalid data. Input a file with a single reference sequence.").arg(settings.referenceUrl)), result);
        GObject *firtsSequenceObject = loadRef->getDocument(false)->findGObjectByType(GObjectTypes::SEQUENCE).first();
        U2SequenceObject* so = qobject_cast<U2SequenceObject*>(firtsSequenceObject);
        CHECK_EXT(so != NULL, setError(tr("Unable to cast gobject to sequence object")), result);
        CHECK_EXT(so->getAlphabet()->isDNA(), setError(tr("The input reference sequence '%1' contains characters that don't belong to DNA alphabet.").arg(so->getSequenceName())), result);

        CmdlineInOutTaskConfig config;

        config.command = "--task=" + ALIGN_TO_REF_CMDLINE;
        QString argString = "--%1=\"%2\"";
        config.arguments << argString.arg(REF_ARG).arg(settings.referenceUrl);
        config.arguments << argString.arg(READS_ARG).arg(settings.readUrls.join(";"));
        config.arguments << argString.arg(MIN_IDENTITY_ARG).arg(settings.minIdentity);
        config.arguments << argString.arg(MIN_LEN_ARG).arg(settings.minLength);
        config.arguments << argString.arg(THRESHOLD_ARG).arg(settings.qualityThreshold);
        config.arguments << argString.arg(TRIM_ARG).arg(settings.trimBothEnds);
        config.arguments << argString.arg(RESULT_ALIGNMENT_ARG).arg(settings.outAlignment);

        config.reportFile = reportFile.fileName();
        config.emptyOutputPossible = true;

        cmdlineTask = new CmdlineInOutTaskRunner(config);
        result.append(cmdlineTask);
    } else if (subTask == cmdlineTask && settings.addResultToProject) {
        // add load document task
        FormatDetectionConfig config;
        QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(settings.outAlignment, config);
        CHECK_EXT(!formats.isEmpty() && (NULL != formats.first().format), setError(tr("wrong output format")), result);

        DocumentFormat *format = formats.first().format;
        CHECK_EXT(format->getSupportedObjectTypes().contains(GObjectTypes::MULTIPLE_CHROMATOGRAM_ALIGNMENT), setError(tr("wrong output format")), result);

        LoadDocumentTask *loadTask= new LoadDocumentTask(format->getFormatId(),
                                                         settings.outAlignment, AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(settings.outAlignment)));
        AddDocumentAndOpenViewTask *openTask = new AddDocumentAndOpenViewTask(loadTask);
        AppContext::getTaskScheduler()->registerTopLevelTask(openTask);
    }

    return result;
}

void AlignToReferenceBlastCmdlineTask::run() {
    reportFile.open();
    reportString = reportFile.readAll();
}

Task::ReportResult AlignToReferenceBlastCmdlineTask::report() {
    if (loadRef != NULL) {
        loadRef->cleanup();
    }
    return ReportResult_Finished;
}

QStringList AlignToReferenceBlastDialog::lastUsedReadsUrls;
const QString AlignToReferenceBlastDialog::defaultOutputName("sanger_reads_alignment.ugenedb");

AlignToReferenceBlastDialog::AlignToReferenceBlastDialog(QWidget *parent)
    : QDialog(parent),
      saveController(NULL),
      savableWidget(this)
{
    setupUi(this);

    new HelpButton(this, buttonBox, "18220587"); //! TODO: help link

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Align"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    connectSlots();
    initSaveController();

    U2WidgetStateStorage::restoreWidgetState(savableWidget);
    foreach (const QString& read, lastUsedReadsUrls) {
        readsListWidget->addItem(read);
    }
}

void AlignToReferenceBlastDialog::initSaveController() {
    SaveDocumentControllerConfig conf;
    conf.defaultFormatId = BaseDocumentFormats::UGENEDB;
    conf.fileDialogButton = setOutputButton;
    conf.fileNameEdit = outputLineEdit;
    conf.formatCombo = NULL;
    conf.parentWidget = this;
    conf.saveTitle = tr("Select Output File...");
    conf.defaultFileName = GUrlUtils::getDefaultDataPath() + "/" + defaultOutputName;

    const QList<DocumentFormatId> formats = QList<DocumentFormatId>() << BaseDocumentFormats::UGENEDB;
    saveController = new SaveDocumentController(conf, formats, this);
}

AlignToReferenceBlastCmdlineTask::Settings AlignToReferenceBlastDialog::getSettings() const {
    return settings;
}

void AlignToReferenceBlastDialog::accept() {
    if (referenceLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Reference sequence is not set."));
        return;
    }
    settings.referenceUrl = referenceLineEdit->text();

    if (readsListWidget->count() == 0) {
        QMessageBox::warning(this, tr("Error"),
                             tr("No reads provided."));
        return;
    }
    QStringList readUrls;
    for (int i = 0; i < readsListWidget->count(); i++) {
        QListWidgetItem* item = readsListWidget->item(i);
        SAFE_POINT(item != NULL, "Item is NULL", );
        QString s = item->text();
        readUrls.append(s);
    }
    settings.readUrls = readUrls;
    lastUsedReadsUrls = readUrls;

    settings.minIdentity = minIdentitySpinBox->value();
    settings.minLength = minLenSpinBox->value();
    settings.qualityThreshold = qualitySpinBox->value();
    settings.trimBothEnds = trimCheckBox->isChecked();

    if (outputLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Output file is not set."));
        return;
    }
    settings.outAlignment = outputLineEdit->text();
    settings.addResultToProject = addToProjectCheckbox->isChecked();

    QDialog::accept();
}

void AlignToReferenceBlastDialog::sl_setReference() {
    LastUsedDirHelper lod;
    QString filter = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::SEQUENCE, true);

    lod.url = U2FileDialog::getOpenFileName(this, tr("Open Reference Sequence"), lod.dir, filter);
    if (lod.url.isEmpty()) {
        return;
    }
    referenceLineEdit->setText(lod.url);
}

void AlignToReferenceBlastDialog::sl_addRead() {
    LastUsedDirHelper lod;
    QString filter = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::SEQUENCE, true);

    QStringList readFiles = U2FileDialog::getOpenFileNames(this, tr("Select File(s) with Read(s)"), lod.dir, filter);
    if (readFiles.isEmpty()) {
        return;
    }

    foreach (const QString& read, readFiles) {
        if (readsListWidget->findItems(read, Qt::MatchExactly).isEmpty()) {
            readsListWidget->addItem(read);
        }
    }
}

void AlignToReferenceBlastDialog::sl_removeRead() {
    QList<QListWidgetItem*> selection = readsListWidget->selectedItems();
    CHECK(!selection.isEmpty(), );

    foreach (QListWidgetItem* item, selection) {
        readsListWidget->takeItem(readsListWidget->row(item));
    }
    qDeleteAll(selection);
}


void AlignToReferenceBlastDialog::sl_referenceChanged(const QString &newRef) {
    QFileInfo outFileFi(outputLineEdit->text());
    if (!fitsDefaultPattern(outFileFi.fileName())) {
        return;
    }
    
    QFileInfo referenceFileInfo(newRef);
    QString newOutFileName = referenceFileInfo.baseName() + "_" + defaultOutputName;
    outputLineEdit->setText(outFileFi.dir().absolutePath() + "/" + newOutFileName);
}

void AlignToReferenceBlastDialog::connectSlots() {
    connect(setReferenceButton, SIGNAL(clicked(bool)), SLOT(sl_setReference()));
    connect(addReadButton, SIGNAL(clicked(bool)), SLOT(sl_addRead()));
    connect(removeReadButton, SIGNAL(clicked(bool)), SLOT(sl_removeRead()));
    connect(referenceLineEdit, SIGNAL(textChanged(const QString &)), SLOT(sl_referenceChanged(const QString &)));
}

bool AlignToReferenceBlastDialog::fitsDefaultPattern(const QString &name) const {
    if (name.endsWith(defaultOutputName)) {
        return true;
    }
    return false;
}

} // namespace
