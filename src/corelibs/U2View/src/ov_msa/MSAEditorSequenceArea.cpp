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

#include <QApplication>
#include <QClipboard>
#include <QDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QTextStream>

#include <U2Algorithm/CreateSubalignmentTask.h>
#include <U2Algorithm/MsaColorScheme.h>
#include <U2Algorithm/MsaHighlightingScheme.h>

#include <U2Core/AddSequencesToAlignmentTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/ClipboardController.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/MultipleSequenceAlignment.h>
#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/QObjectScopedPointer.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/Settings.h>
#include <U2Core/Settings.h>
#include <U2Core/Task.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/TaskWatchdog.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>

#include <U2Formats/DocumentFormatUtils.h>

#include <U2Gui/AppSettingsGUI.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/Notification.h>
#include <U2Gui/OPWidgetFactory.h>
#include <U2Gui/OptionsPanel.h>
#include <U2Gui/PositionSelector.h>
#include <U2Gui/ProjectTreeController.h>
#include <U2Gui/ProjectTreeItemSelectorDialog.h>

#include "AlignSequencesToAlignment/AlignSequencesToAlignmentTask.h"
#include "Clipboard/SubalignmentToClipboardTask.h"
#include "ColorSchemaSettingsController.h"
#include "CreateSubalignmentDialogController.h"
#include "Highlighting/MSAHighlightingTabFactory.h"
#include "view_rendering/SequenceAreaRenderer.h"
#include "view_rendering/SequenceWithChromatogramAreaRenderer.h" // SANGER_TODO: monitor unneccessary headers!
#include "MSAEditor.h"
#include "MSAEditorNameList.h"
#include "MSAEditorSequenceArea.h"


namespace U2 {

#define SETTINGS_ROOT QString("msaeditor/")
#define SETTINGS_COLOR_NUCL     "color_nucl"
#define SETTINGS_COLOR_AMINO    "color_amino"
#define SETTINGS_COLOR_RAW      "color_raw"
#define SETTINGS_HIGHLIGHT_NUCL     "highlight_nucl"
#define SETTINGS_HIGHLIGHT_AMINO    "highlight_amino"
#define SETTINGS_HIGHLIGHT_RAW      "highlight_raw"
#define SETTINGS_COPY_FORMATTED "copyformatted"

MSAEditorSequenceArea::MSAEditorSequenceArea(MaEditorWgt* _ui, GScrollBar* hb, GScrollBar* vb)
    : MaEditorSequenceArea(_ui, hb, vb),
      useDotsAction(NULL),
      changeTracker(editor->getMaObject()->getEntityRef())
{
    setObjectName("msa_editor_sequence_area");
    setFocusPolicy(Qt::WheelFocus);

    renderer = new SequenceWithChromatogramAreaRenderer(this);

    selectionColor = Qt::black;

    connect(&editModeAnimationTimer, SIGNAL(timeout()), SLOT(sl_changeSelectionColor()));

    connect(ui->getCopySelectionAction(), SIGNAL(triggered()), SLOT(sl_copyCurrentSelection()));
    addAction(ui->getCopySelectionAction());

    connect(ui->getCopyFormattedSelectionAction(), SIGNAL(triggered()), SLOT(sl_copyFormattedSelection()));
    addAction(ui->getCopyFormattedSelectionAction());

    connect(ui->getPasteAction(), SIGNAL(triggered()), SLOT(sl_paste()));
    addAction(ui->getPasteAction());

    delColAction = new QAction(QIcon(":core/images/msaed_remove_columns_with_gaps.png"), tr("Remove columns of gaps..."), this);
    delColAction->setObjectName("remove_columns_of_gaps");
    delColAction->setShortcut(QKeySequence(Qt::SHIFT| Qt::Key_Delete));
    delColAction->setShortcutContext(Qt::WidgetShortcut);
    addAction(delColAction);
    connect(delColAction, SIGNAL(triggered()), SLOT(sl_delCol()));

    insSymAction = new QAction(tr("Fill selection with gaps"), this);
    insSymAction->setObjectName("fill_selection_with_gaps");
    connect(insSymAction, SIGNAL(triggered()), SLOT(sl_fillCurrentSelectionWithGaps()));
    addAction(insSymAction);


    createSubaligniment = new QAction(tr("Save subalignment..."), this);
    createSubaligniment->setObjectName("Save subalignment");
    connect(createSubaligniment, SIGNAL(triggered()), SLOT(sl_createSubaligniment()));

    saveSequence = new QAction(tr("Save sequence..."), this);
    saveSequence->setObjectName("Save sequence");
    connect(saveSequence, SIGNAL(triggered()), SLOT(sl_saveSequence()));

    gotoAction = new QAction(QIcon(":core/images/goto.png"), tr("Go to position..."), this);
    gotoAction->setObjectName("action_go_to_position");
    gotoAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    gotoAction->setShortcutContext(Qt::WindowShortcut);
    gotoAction->setToolTip(QString("%1 (%2)").arg(gotoAction->text()).arg(gotoAction->shortcut().toString()));
    connect(gotoAction, SIGNAL(triggered()), SLOT(sl_goto()));

    removeAllGapsAction = new QAction(QIcon(":core/images/msaed_remove_all_gaps.png"), tr("Remove all gaps"), this);
    removeAllGapsAction->setObjectName("Remove all gaps");
    connect(removeAllGapsAction, SIGNAL(triggered()), SLOT(sl_removeAllGaps()));

    addSeqFromFileAction = new QAction(tr("Sequence from file..."), this);
    addSeqFromFileAction->setObjectName("Sequence from file");
    connect(addSeqFromFileAction, SIGNAL(triggered()), SLOT(sl_addSeqFromFile()));

    addSeqFromProjectAction = new QAction(tr("Sequence from current project..."), this);
    addSeqFromProjectAction->setObjectName("Sequence from current project");
    connect(addSeqFromProjectAction, SIGNAL(triggered()), SLOT(sl_addSeqFromProject()));

    sortByNameAction = new QAction(tr("Sort sequences by name"), this);
    sortByNameAction->setObjectName("action_sort_by_name");
    connect(sortByNameAction, SIGNAL(triggered()), SLOT(sl_sortByName()));

    collapseModeSwitchAction = new QAction(QIcon(":core/images/collapse.png"), tr("Switch on/off collapsing"), this);
    collapseModeSwitchAction->setObjectName("Enable collapsing");
    collapseModeSwitchAction->setCheckable(true);
    connect(collapseModeSwitchAction, SIGNAL(toggled(bool)), SLOT(sl_setCollapsingMode(bool)));

    collapseModeUpdateAction = new QAction(QIcon(":core/images/collapse_update.png"), tr("Update collapsed groups"), this);
    collapseModeUpdateAction->setObjectName("Update collapsed groups");
    collapseModeUpdateAction->setEnabled(false);
    connect(collapseModeUpdateAction, SIGNAL(triggered()), SLOT(sl_updateCollapsingMode()));

    reverseComplementAction = new QAction(tr("Replace selected rows with reverse-complement"), this);
    reverseComplementAction->setObjectName("replace_selected_rows_with_reverse-complement");
    connect(reverseComplementAction, SIGNAL(triggered()), SLOT(sl_reverseComplementCurrentSelection()));

    replaceCharacterAction = new QAction(tr("Replace selected character"), this);
    replaceCharacterAction->setObjectName("replace_selected_character");
    replaceCharacterAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_R));
    replaceCharacterAction->setShortcutContext(Qt::WidgetShortcut);
    addAction(replaceCharacterAction);
    connect(replaceCharacterAction, SIGNAL(triggered()), SLOT(sl_replaceSelectedCharacter()));

    reverseAction = new QAction(tr("Replace selected rows with reverse"), this);
    reverseAction->setObjectName("replace_selected_rows_with_reverse");
    connect(reverseAction, SIGNAL(triggered()), SLOT(sl_reverseCurrentSelection()));

    complementAction = new QAction(tr("Replace selected rows with complement"), this);
    complementAction->setObjectName("replace_selected_rows_with_complement");
    connect(complementAction, SIGNAL(triggered()), SLOT(sl_complementCurrentSelection()));

    connect(editor->getMaObject(), SIGNAL(si_alignmentChanged(const MultipleAlignment&, const MaModificationInfo&)),
        SLOT(sl_alignmentChanged(const MultipleAlignment&, const MaModificationInfo&)));
    connect(editor->getMaObject(), SIGNAL(si_lockedStateChanged()), SLOT(sl_lockedStateChanged()));
    connect(editor->getMaObject(), SIGNAL(si_rowsRemoved(const QList<qint64> &)), SLOT(sl_updateCollapsingMode()));

    connect(this,   SIGNAL(si_startMsaChanging()),
            ui,     SIGNAL(si_startMsaChanging()));
    connect(this,   SIGNAL(si_stopMsaChanging(bool)),
            ui,     SIGNAL(si_stopMsaChanging(bool)));

    connect(editor, SIGNAL(si_buildStaticMenu(GObjectView*, QMenu*)), SLOT(sl_buildStaticMenu(GObjectView*, QMenu*)));
    connect(editor, SIGNAL(si_buildStaticToolbar(GObjectView*, QToolBar*)), SLOT(sl_buildStaticToolbar(GObjectView*, QToolBar*)));
    connect(editor, SIGNAL(si_buildPopupMenu(GObjectView* , QMenu*)), SLOT(sl_buildContextMenu(GObjectView*, QMenu*)));
    connect(editor, SIGNAL(si_zoomOperationPerformed(bool)), SLOT(sl_completeUpdate()));
    connect(editor, SIGNAL(si_fontChanged(QFont)), SLOT(sl_fontChanged(QFont)));
    connect(ui->getCollapseModel(), SIGNAL(toggled()), SLOT(sl_modelChanged()));
    connect(editor, SIGNAL(si_referenceSeqChanged(qint64)), SLOT(sl_completeUpdate()));
    connect(editor, SIGNAL(si_completeUpdate()), SLOT(sl_completeUpdate()));

    QAction* undoAction = ui->getUndoAction();
    QAction* redoAction = ui->getRedoAction();
    addAction(undoAction);
    addAction(redoAction);
    connect(undoAction, SIGNAL(triggered()), SLOT(sl_resetCollapsibleModel()));
    connect(redoAction, SIGNAL(triggered()), SLOT(sl_resetCollapsibleModel()));

    connect(editor->getMaObject(), SIGNAL(si_alphabetChanged(const MaModificationInfo &, const DNAAlphabet*)),
        SLOT(sl_alphabetChanged(const MaModificationInfo &, const DNAAlphabet*)));

    updateColorAndHighlightSchemes();
    updateActions();
}

void MSAEditorSequenceArea::updateColorAndHighlightSchemes() {
    Settings* s = AppContext::getSettings();
    if (!s || !editor){
        return;
    }
    MultipleAlignmentObject* maObj = editor->getMaObject();
    if (!maObj){
        return;
    }

    const DNAAlphabet* al = maObj->getAlphabet();
    if (!al){
        return;
    }

    DNAAlphabetType atype = al->getType();
    DNAAlphabetType currentAlphabet = DNAAlphabet_RAW;
    bool colorSchemesActionsIsEmpty = colorSchemeMenuActions.isEmpty();
    MsaColorSchemeRegistry* csr = AppContext::getMsaColorSchemeRegistry();
    MsaHighlightingSchemeRegistry* hsr = AppContext::getMsaHighlightingSchemeRegistry();
    if (!colorSchemesActionsIsEmpty) {
        QString id = colorSchemeMenuActions.first()->data().toString();
        MsaColorSchemeFactory* f = csr->getMsaColorSchemeFactoryById(id);
        currentAlphabet = f->getAlphabetType();
        if (currentAlphabet == atype) {
            return;
        }
    }

    QString csid;
    QString hsid;
    getColorAndHighlightingIds(csid, hsid, atype, colorSchemesActionsIsEmpty);
    MsaColorSchemeFactory* csf = csr->getMsaColorSchemeFactoryById(csid);
    if (csf == NULL) {
        csf = getDefaultColorSchemeFactory();
    }
    SAFE_POINT(csf != NULL, "Color scheme factory is NULL", );
    MsaHighlightingSchemeFactory* hsf = hsr->getMsaHighlightingSchemeFactoryById(hsid);
    initColorSchemes(csf);
    initHighlightSchemes(hsf, atype);
}

void MSAEditorSequenceArea::initHighlightSchemes(MsaHighlightingSchemeFactory* hsf, DNAAlphabetType atype) {
    qDeleteAll(highlightingSchemeMenuActions);
    highlightingSchemeMenuActions.clear();
    SAFE_POINT(hsf != NULL, "Highlight scheme factory is NULL", );

    MultipleAlignmentObject* maObj = editor->getMaObject();
    delete highlightingScheme;

    highlightingScheme = hsf->create(this, maObj);

    MsaHighlightingSchemeRegistry* hsr = AppContext::getMsaHighlightingSchemeRegistry();
    QList<MsaHighlightingSchemeFactory*> highFactories = hsr->getMsaHighlightingSchemes(atype);
    foreach (MsaHighlightingSchemeFactory* factory, highFactories) {
        QAction* action = new QAction(factory->getName(), this);
        action->setObjectName(factory->getName());
        action->setCheckable(true);
        action->setChecked(factory == hsf);
        action->setData(factory->getId());
        connect(action, SIGNAL(triggered()), SLOT(sl_changeHighlightScheme()));
        highlightingSchemeMenuActions.append(action);
    }
}

void MSAEditorSequenceArea::initColorSchemes(MsaColorSchemeFactory *defaultColorSchemeFactory) {
    MsaColorSchemeRegistry *msaColorSchemeRegistry = AppContext::getMsaColorSchemeRegistry();
    connect(msaColorSchemeRegistry, SIGNAL(si_customSettingsChanged()), SLOT(sl_registerCustomColorSchemes()));

    registerCommonColorSchemes();
    sl_registerCustomColorSchemes();

    useDotsAction = new QAction(QString(tr("Use dots")), this);
    useDotsAction->setCheckable(true);
    useDotsAction->setChecked(false);
    connect(useDotsAction, SIGNAL(triggered()), SLOT(sl_useDots()));

    applyColorScheme(defaultColorSchemeFactory->getId());
}

void MSAEditorSequenceArea::registerCommonColorSchemes() {
    qDeleteAll(colorSchemeMenuActions);
    colorSchemeMenuActions.clear();

    MsaColorSchemeRegistry *msaColorSchemeRegistry = AppContext::getMsaColorSchemeRegistry();
    QList<MsaColorSchemeFactory*> colorFactories = msaColorSchemeRegistry->getMsaColorSchemes(editor->getMaObject()->getAlphabet()->getType());

    foreach (MsaColorSchemeFactory *factory, colorFactories) {
        QAction *action = new QAction(factory->getName(), this);
        action->setObjectName(factory->getName());
        action->setCheckable(true);
        action->setData(factory->getId());
        connect(action, SIGNAL(triggered()), SLOT(sl_changeColorScheme()));
        colorSchemeMenuActions.append(action);
    }
}

void MSAEditorSequenceArea::getColorAndHighlightingIds(QString &csid, QString &hsid, DNAAlphabetType atype, bool isFirstInitialization) {
    Settings* s = AppContext::getSettings();
    switch (atype) {
    case DNAAlphabet_RAW:
        if (isFirstInitialization) {
            csid = s->getValue(SETTINGS_ROOT + SETTINGS_COLOR_RAW, MsaColorScheme::EMPTY_RAW).toString();
            hsid = s->getValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_NUCL, MsaHighlightingScheme::EMPTY_RAW).toString();
        } else {
            csid = MsaColorScheme::EMPTY_RAW;
            hsid = MsaHighlightingScheme::EMPTY_RAW;
        }
        break;
    case DNAAlphabet_NUCL:
        if (isFirstInitialization) {
            csid = s->getValue(SETTINGS_ROOT + SETTINGS_COLOR_NUCL, MsaColorScheme::UGENE_NUCL).toString();
            hsid = s->getValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_NUCL, MsaHighlightingScheme::EMPTY_NUCL).toString();
        } else {
            csid = MsaColorScheme::UGENE_NUCL;
            hsid = MsaHighlightingScheme::EMPTY_NUCL;
        }
        break;
    case DNAAlphabet_AMINO:
        if (isFirstInitialization) {
            csid = s->getValue(SETTINGS_ROOT + SETTINGS_COLOR_AMINO, MsaColorScheme::UGENE_AMINO).toString();
            hsid = s->getValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_AMINO, MsaHighlightingScheme::EMPTY_AMINO).toString();
        } else {
            csid = MsaColorScheme::UGENE_AMINO;
            hsid = MsaHighlightingScheme::EMPTY_AMINO;
        }
        break;
    default:
        csid = "";
        hsid = "";
        break;
    }
}

void MSAEditorSequenceArea::applyColorScheme(const QString &id) {
    CHECK(NULL != ui->getEditor()->getMaObject(), );

    MsaColorSchemeFactory *factory = AppContext::getMsaColorSchemeRegistry()->getMsaColorSchemeFactoryById(id);
    delete colorScheme;
    colorScheme = factory->create(this, ui->getEditor()->getMaObject());

    connect(factory, SIGNAL(si_factoryChanged()), SLOT(sl_colorSchemeFactoryUpdated()), Qt::UniqueConnection);
    connect(factory, SIGNAL(destroyed(QObject *)), SLOT(sl_setDefaultColorScheme()), Qt::UniqueConnection);

    QList<QAction *> tmpActions = QList<QAction *>() << colorSchemeMenuActions << customColorSchemeMenuActions;
    foreach (QAction *action, tmpActions) {
        action->setChecked(action->data() == id);
    }

    switch (factory->getAlphabetType()) {
    case DNAAlphabet_RAW:
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_COLOR_RAW, id);
        break;
    case DNAAlphabet_NUCL:
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_COLOR_NUCL, id);
        break;
    case DNAAlphabet_AMINO:
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_COLOR_AMINO, id);
        break;
    default:
        FAIL(tr("Unknown alphabet"), );
        break;
    }

    completeRedraw = true;
    update();
    emit si_highlightingChanged();
}

MsaColorSchemeFactory * MSAEditorSequenceArea::getDefaultColorSchemeFactory() {
    MsaColorSchemeRegistry *msaColorSchemeRegistry = AppContext::getMsaColorSchemeRegistry();

    switch (editor->getMaObject()->getAlphabet()->getType()) {
    case DNAAlphabet_RAW:
        return msaColorSchemeRegistry->getMsaColorSchemeFactoryById(MsaColorScheme::EMPTY_RAW);
    case DNAAlphabet_NUCL:
        return msaColorSchemeRegistry->getMsaColorSchemeFactoryById(MsaColorScheme::UGENE_NUCL);
    case DNAAlphabet_AMINO:
        return msaColorSchemeRegistry->getMsaColorSchemeFactoryById(MsaColorScheme::UGENE_AMINO);
    default:
        FAIL(tr("Unknown alphabet"), NULL);
    }
    return NULL;
}

QStringList MSAEditorSequenceArea::getAvailableHighlightingSchemes() const{
    QStringList allSchemas;
    foreach(QAction *a, highlightingSchemeMenuActions){
        allSchemas.append(a->text());
    }
    return allSchemas;
}

QString MSAEditorSequenceArea::getCopyFormatedAlgorithmId() const{
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + SETTINGS_COPY_FORMATTED, BaseDocumentFormats::CLUSTAL_ALN).toString();
}

void MSAEditorSequenceArea::setCopyFormatedAlgorithmId(const QString& algoId){
    AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_COPY_FORMATTED, algoId);
}

bool MSAEditorSequenceArea::hasAminoAlphabet() {
    MultipleAlignmentObject* maObj = editor->getMaObject();
    SAFE_POINT(NULL != maObj, tr("MultipleAlignmentObject is null in MSAEditorSequenceArea::hasAminoAlphabet()"), false);
    const DNAAlphabet* alphabet = maObj->getAlphabet();
    SAFE_POINT(NULL != maObj, tr("DNAAlphabet is null in MSAEditorSequenceArea::hasAminoAlphabet()"), false);
    return DNAAlphabet_AMINO == alphabet->getType();
}

bool MSAEditorSequenceArea::drawContent(QPainter &p) {
    qint64 seqNum = editor->getNumSequences();
    if (ui->isCollapsibleMode()) {
        seqNum = ui->getCollapseModel()->rowToMap(seqNum);
    }
    return drawContent(p, QRect(0, 0, editor->getAlignmentLen(), seqNum));
}

bool MSAEditorSequenceArea::drawContent(QPixmap &pixmap) {
    CHECK(editor->getColumnWidth() * editor->getAlignmentLen() < 32768 &&
           editor->getRowHeight() * editor->getNumSequences() < 32768, false);

    qint64 seqNum = editor->getNumSequences();
    if (ui->isCollapsibleMode()) {
        seqNum = ui->getCollapseModel()->rowToMap(seqNum);
    }
    pixmap = QPixmap(editor->getColumnWidth() * editor->getAlignmentLen(),
                      editor->getRowHeight() * seqNum);
    QPainter p(&pixmap);
    return drawContent(p, QRect(0, 0, editor->getAlignmentLen(), seqNum));
}

bool MSAEditorSequenceArea::drawContent(QPixmap &pixmap,
                                          const U2Region &region,
                                          const QList<qint64> &seqIdx) {
    CHECK(!region.isEmpty(), false);
    CHECK(!seqIdx.isEmpty(), false);

    CHECK(editor->getColumnWidth() * region.length < 32768 &&
           editor->getRowHeight() * seqIdx.size() < 32768, false);
    pixmap = QPixmap(editor->getColumnWidth() * region.length,
                     editor->getRowHeight() * seqIdx.size());
    QPainter p(&pixmap);
    return drawContent(p, region, seqIdx);
}

void MSAEditorSequenceArea::sl_changeColorSchemeOutside(const QString &name) {
    QAction* a = GUIUtils::findAction(QList<QAction*>() << colorSchemeMenuActions << customColorSchemeMenuActions << highlightingSchemeMenuActions, name);
    if (a != NULL) {
        a->trigger();
    }
}

void MSAEditorSequenceArea::sl_doUseDots(){
    useDotsAction->trigger();
}

void MSAEditorSequenceArea::sl_useDots(){
    completeRedraw = true;
    update();
    emit si_highlightingChanged();
}

void MSAEditorSequenceArea::sl_changeCopyFormat(const QString& alg){
    setCopyFormatedAlgorithmId(alg);
}

void MSAEditorSequenceArea::sl_changeColorScheme() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (NULL == action) {
        action = GUIUtils::getCheckedAction(customColorSchemeMenuActions);
    }
    CHECK(NULL != action, );

    applyColorScheme(action->data().toString());
}

void MSAEditorSequenceArea::sl_changeHighlightScheme(){
    QAction* a = qobject_cast<QAction*>(sender());
    if (NULL == a) {
        a = GUIUtils::getCheckedAction(customColorSchemeMenuActions);
    }
    CHECK(NULL != a, );

    editor->saveHighlightingSettings(highlightingScheme->getFactory()->getId(), highlightingScheme->getSettings());

    QString id = a->data().toString();
    MsaHighlightingSchemeFactory* factory = AppContext::getMsaHighlightingSchemeRegistry()->getMsaHighlightingSchemeFactoryById(id);
    SAFE_POINT(NULL != factory, L10N::nullPointerError("highlighting scheme"), );
    if (ui->getEditor()->getMaObject() == NULL) {
        return;
    }

    delete highlightingScheme;
    highlightingScheme = factory->create(this, ui->getEditor()->getMaObject());
    highlightingScheme->applySettings(editor->getHighlightingSettings(id));

    const MultipleAlignment ma = ui->getEditor()->getMaObject()->getMultipleAlignment();

    U2OpStatusImpl os;
    const int refSeq = ma->getRowIndexByRowId(editor->getReferenceRowId(), os);

    MSAHighlightingFactory msaHighlightingFactory;
    QString msaHighlightingId = msaHighlightingFactory.getOPGroupParameters().getGroupId();

    CHECK(ui->getEditor(), );
    CHECK(ui->getEditor()->getOptionsPanel(), );

    if(!factory->isRefFree() && refSeq == -1 && ui->getEditor()->getOptionsPanel()->getActiveGroupId() != msaHighlightingId) {
        QMessageBox::warning(ui, tr("No reference sequence selected"),
            tr("Reference sequence for current highlighting scheme is not selected. Use context menu or Highlighting tab on Options panel to select it"));
    }

    foreach(QAction* action, highlightingSchemeMenuActions) {
        action->setChecked(action == a);
    }
    switch (factory->getAlphabetType()) {
    case DNAAlphabet_RAW:
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_RAW, id);
        break;
    case DNAAlphabet_NUCL:
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_NUCL, id);
        break;
    case DNAAlphabet_AMINO:
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_HIGHLIGHT_AMINO, id);
        break;
    default:
        FAIL(tr("Unknown alphabet"), );
        break;
    }

    completeRedraw = true;
    update();
    emit si_highlightingChanged();
}

void MSAEditorSequenceArea::updateActions() {
    MultipleAlignmentObject* maObj = editor->getMaObject();
    assert(maObj != NULL);
    bool readOnly = maObj->isStateLocked();

    createSubaligniment->setEnabled(!isAlignmentEmpty());
    saveSequence->setEnabled(!isAlignmentEmpty());
    addSeqFromProjectAction->setEnabled(!readOnly);
    addSeqFromFileAction->setEnabled(!readOnly);
    sortByNameAction->setEnabled(!readOnly && !isAlignmentEmpty());
    collapseModeSwitchAction->setEnabled(!readOnly && !isAlignmentEmpty());

//Update actions of "Edit" group
    bool canEditAlignment = !readOnly && !isAlignmentEmpty();
    bool canEditSelectedArea = canEditAlignment && !selection.isNull();
    ui->getDelSelectionAction()->setEnabled(canEditSelectedArea);

    insSymAction->setEnabled(canEditSelectedArea);
    bool oneCharacterIsSelected = selection.width() == 1 && selection.height() == 1;
    replaceCharacterAction->setEnabled(canEditSelectedArea && oneCharacterIsSelected);
    delColAction->setEnabled(canEditAlignment);
    reverseComplementAction->setEnabled(canEditSelectedArea && maObj->getAlphabet()->isNucleic());
    reverseAction->setEnabled(canEditSelectedArea);
    complementAction->setEnabled(canEditSelectedArea && maObj->getAlphabet()->isNucleic());
    removeAllGapsAction->setEnabled(canEditAlignment);

    assert(checkState());
}

void MSAEditorSequenceArea::paintEvent(QPaintEvent *e) {
    drawAll();
    QWidget::paintEvent(e);
}

void MSAEditorSequenceArea::drawAll() {
    QSize s = size();
    if (cachedView->size() != s) {
        assert(completeRedraw);
        delete cachedView;
        cachedView = new QPixmap(s);
    }
    if (completeRedraw) {
        QPainter pCached(cachedView);
        drawVisibleContent(pCached);
        completeRedraw = false;
    }
    QPainter p(this);
    p.drawPixmap(0, 0, *cachedView);
    drawSelection(p);
    drawFocus(p);
}

void MSAEditorSequenceArea::drawVisibleContent(QPainter& p) {
    drawContent(p, QRect(startPos, getFirstVisibleSequence(), getNumVisibleBases(false), getNumVisibleSequences(true)));
}

bool MSAEditorSequenceArea::drawContent(QPainter &p, const QRect &area) {
    QVector<U2Region> range;
    if (ui->isCollapsibleMode()) {
        ui->getCollapseModel()->getVisibleRows(area.y(), area.bottom(), range);
    } else {
        range.append(U2Region(area.y(), area.height()));
    }

    QList <qint64> seqIdx;
    foreach(U2Region region, range) {
        for (qint64 i = region.startPos; i < region.endPos(); i++) {
            seqIdx.append(i);
        }
    }
    p.fillRect(cachedView->rect(), Qt::white);
    bool ok = renderer->drawContent(p, U2Region(area.x(), area.width()), seqIdx);
    emit si_visibleRangeChanged();

    return ok;
}

bool MSAEditorSequenceArea::drawContent(QPainter &p, const U2Region &region, const QList<qint64> &seqIdx) {
    // SANGER_TODO: optimize
    return renderer->drawContent(p, region, seqIdx);
}

// SANGER_TODO: move to renderer
void MSAEditorSequenceArea::drawSelection(QPainter &p) {
    int x = selection.x();
    int y = selection.y();

    U2Region xRange = getBaseXRange(x, true);
    U2Region yRange = getSequenceYRange(y, true);

    QPen pen(highlightSelection || hasFocus()? selectionColor : Qt::gray);
    if (msaMode != EditCharacterMode) {
        pen.setStyle(Qt::DashLine);
    }
    pen.setWidth(highlightSelection ? 2 : 1);
    p.setPen(pen);
    if(yRange.startPos > 0) {
        p.drawRect(xRange.startPos, yRange.startPos, xRange.length*selection.width(), yRange.length*selection.height());
    }
    else {
        qint64 regionHeight = yRange.length*selection.height() + yRange.startPos + 1;
        if(regionHeight <= 0) {
            return;
        }
        p.drawRect(xRange.startPos, -1, xRange.length*selection.width(), regionHeight);
    }
}

void MSAEditorSequenceArea::drawFocus(QPainter& p) {
    if (hasFocus()) {
        p.setPen(QPen(Qt::black, 1, Qt::DotLine));
        p.drawRect(0, 0, width()-1, height()-1);
    }
}

void MSAEditorSequenceArea::resizeEvent(QResizeEvent *e) {
    completeRedraw = true;
    validateRanges();
    QWidget::resizeEvent(e);
}

void MSAEditorSequenceArea::validateRanges() {
    //check x dimension
    int aliLen = editor->getAlignmentLen();
    int visibleBaseCount = countWidthForBases(false);

    if (isAlignmentEmpty()) {
        setFirstVisibleBase(-1);
    } else if (visibleBaseCount > aliLen) {
        setFirstVisibleBase(0);
    } else if (startPos + visibleBaseCount > aliLen) {
        setFirstVisibleBase(aliLen - visibleBaseCount);
    }

    SAFE_POINT(0 <= startPos || isAlignmentEmpty(), tr("Negative startPos with non-empty alignment"), );
    SAFE_POINT(startPos + visibleBaseCount <= aliLen || aliLen < visibleBaseCount, tr("startPos is too big"), );

    updateHScrollBar();

    //check y dimension
    if (ui->isCollapsibleMode()) {
        sl_modelChanged();
        return;
    }

    int nSeqs = editor->getNumSequences();
    int visibleSequenceCount = countHeightForSequences(false);

    if (isAlignmentEmpty()) {
        setFirstVisibleSequence(-1);
    } else if (visibleSequenceCount > nSeqs) {
        setFirstVisibleSequence(0);
    } else if (startSeq + visibleSequenceCount > nSeqs) {
        setFirstVisibleSequence(nSeqs - visibleSequenceCount);
    }

    SAFE_POINT(0 <= startSeq || isAlignmentEmpty(), tr("Negative startSeq with non-empty alignment"), );
    SAFE_POINT(startSeq + visibleSequenceCount <= nSeqs || nSeqs < visibleSequenceCount, tr("startSeq is too big"), );

    updateVScrollBar();
}

void MSAEditorSequenceArea::sl_onHScrollMoved(int pos) {
    if (isAlignmentEmpty()) {
        setFirstVisibleBase(-1);
    } else {
        SAFE_POINT(0 <= pos && pos <= editor->getAlignmentLen() - getNumVisibleBases(false), tr("Position is out of range: %1").arg(QString::number(pos)), );
        setFirstVisibleBase(pos);
    }
}

void MSAEditorSequenceArea::sl_onVScrollMoved(int seq) {
    if (isAlignmentEmpty()) {
        setFirstVisibleSequence(-1);
    } else {
        SAFE_POINT(0 <= seq && seq <= editor->getNumSequences() - getNumVisibleSequences(false), tr("Sequence is out of range: %1").arg(QString::number(seq)), );
        setFirstVisibleSequence(seq);
    }
}

#define SCROLL_STEP 1

void MSAEditorSequenceArea::mouseMoveEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton) {
        QPoint newCurPos = coordToAbsolutePosOutOfRange(e->pos());
        if (isInRange(newCurPos)) {
            updateHBarPosition(newCurPos.x());
            updateVBarPosition(newCurPos.y());
        }

        if (shifting) {
            shiftSelectedRegion(newCurPos.x() - cursorPos.x());
        } else if (selecting) {
            rubberBand->setGeometry(QRect(origin, e->pos()).normalized());
        }
    }

    QWidget::mouseMoveEvent(e);
}

void MSAEditorSequenceArea::mouseReleaseEvent(QMouseEvent *e) {
    rubberBand->hide();
    if (shifting) {
        changeTracker.finishTracking();
        editor->getMaObject()->releaseState();
    }

    QPoint newCurPos = coordToAbsolutePos(e->pos());

    int firstVisibleSeq = getFirstVisibleSequence();
    int visibleRowsNums = getNumDisplayedSequences() - 1;

    int yPosWithValidations = qMax(firstVisibleSeq, newCurPos.y());
    yPosWithValidations = qMin(yPosWithValidations, visibleRowsNums + firstVisibleSeq);

    newCurPos.setY(yPosWithValidations);

    if (shifting) {
        emit si_stopMsaChanging(msaVersionBeforeShifting != editor->getMaObject()->getModificationVersion());
    } else if (Qt::LeftButton == e->button() && Qt::LeftButton == prevPressedButton) {
        updateSelection(newCurPos);
    }
    shifting = false;
    selecting = false;
    msaVersionBeforeShifting = -1;

    shBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
    svBar->setupRepeatAction(QAbstractSlider::SliderNoAction);

    QWidget::mouseReleaseEvent(e);
}

void MSAEditorSequenceArea::mousePressEvent(QMouseEvent *e) {
    prevPressedButton = e->button();

    if (!hasFocus()) {
        setFocus();
    }

    if ((e->button() == Qt::LeftButton)) {
        if (Qt::ShiftModifier == e->modifiers()) {
            QWidget::mousePressEvent(e);
            return;
        }

        origin = e->pos();
        QPoint p = coordToPos(e->pos());
        if(isInRange(p)) {
            setCursorPos(p);

            const MaEditorSelection &s = getSelection();
            if (s.getRect().contains(cursorPos) && !isAlignmentLocked()) {
                shifting = true;
                msaVersionBeforeShifting = editor->getMaObject()->getModificationVersion();
                U2OpStatus2Log os;
                changeTracker.startTracking(os);
                CHECK_OP(os, );
                editor->getMaObject()->saveState();
                emit si_startMsaChanging();
            }
        }

        if (!shifting) {
            selecting = true;
            origin = e->pos();
            QPoint q = coordToAbsolutePos(e->pos());
            if (isInRange(q)) {
                setCursorPos(q);
            }
            rubberBand->setGeometry(QRect(origin, QSize()));
            rubberBand->show();
            cancelSelection();
        }
    }

    QWidget::mousePressEvent(e);
}

void MSAEditorSequenceArea::keyPressEvent(QKeyEvent *e) {
    if (!hasFocus()) {
        return;
    }

    int key = e->key();
    if (msaMode == EditCharacterMode) {
        processCharacterInEditMode(e);
        return;
    }

    bool shift = e->modifiers().testFlag(Qt::ShiftModifier);
    const bool ctrl = e->modifiers().testFlag(Qt::ControlModifier);
#ifdef Q_OS_MAC
    // In one case it is better to use a Command key as modifier,
    // in another - a Control key. genuineCtrl - Control key on Mac OS X.
    const bool genuineCtrl = e->modifiers().testFlag(Qt::MetaModifier);
#else
    const bool genuineCtrl = ctrl;
#endif
    static QPoint selectionStart(0, 0);
    static QPoint selectionEnd(0, 0);

    if (ctrl && (key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up || key == Qt::Key_Down)) {
        //remap to page_up/page_down
        shift = key == Qt::Key_Up || key == Qt::Key_Down;
        key =  (key == Qt::Key_Up || key == Qt::Key_Left) ? Qt::Key_PageUp : Qt::Key_PageDown;
    }
    //part of these keys are assigned to actions -> so them never passed to keyPressEvent (action handling has higher priority)
    int endX, endY;
    switch(key) {
        case Qt::Key_Escape:
             cancelSelection();
             break;
        case Qt::Key_Left:
            if(!(Qt::ShiftModifier & e->modifiers())) {
                moveSelection(-1,0);
                break;
            }
            if (selectionEnd.x() < 1) {
                break;
            }
            selectionEnd.setX(selectionEnd.x() - 1);
            endX = selectionEnd.x();
            if (isPosInRange(endX)) {
                if (endX != -1) {
                    int firstColumn = qMin(selectionStart.x(),endX);
                    int width = qAbs(endX - selectionStart.x()) + 1;
                    int startSeq = selection.y();
                    int height = selection.height();
                    if (selection.isNull()) {
                        startSeq = cursorPos.y();
                        height = 1;
                    }
                    MaEditorSelection _selection(firstColumn, startSeq, width, height);
                    setSelection(_selection);
                }
            }
            break;
        case Qt::Key_Right:
            if(!(Qt::ShiftModifier & e->modifiers())) {
                moveSelection(1,0);
                break;
            }
            if (selectionEnd.x() >= (editor->getAlignmentLen() - 1)) {
                break;
            }
            selectionEnd.setX(selectionEnd.x() +  1);
            endX = selectionEnd.x();
            if (isPosInRange(endX)) {
                if (endX != -1) {
                    int firstColumn = qMin(selectionStart.x(),endX);
                    int width = qAbs(endX - selectionStart.x()) + 1;
                    int startSeq = selection.y();
                    int height = selection.height();
                    if (selection.isNull()) {
                        startSeq = cursorPos.y();
                        height = 1;
                    }
                    MaEditorSelection _selection(firstColumn, startSeq, width, height);
                    setSelection(_selection);
                }
            }
            break;
        case Qt::Key_Up:
            if(!(Qt::ShiftModifier & e->modifiers())) {
                moveSelection(0,-1);
                break;
            }
            if(selectionEnd.y() < 1) {
                break;
            }
            selectionEnd.setY(selectionEnd.y() - 1);
            endY = selectionEnd.y();
            if (isSeqInRange(endY)) {
                if (endY != -1) {
                    int startSeq = qMin(selectionStart.y(),endY);
                    int height = qAbs(endY - selectionStart.y()) + 1;
                    int firstColumn = selection.x();
                    int width = selection.width();
                    if (selection.isNull()) {
                        firstColumn = cursorPos.x();
                        width = 1;
                    }
                    MaEditorSelection _selection(firstColumn, startSeq, width, height);
                    setSelection(_selection);
                }
            }
            break;
        case Qt::Key_Down:
            if(!(Qt::ShiftModifier & e->modifiers())) {
                moveSelection(0,1);
                break;
            }
            if (selectionEnd.y() >= (ui->getCollapseModel()->displayedRowsCount() - 1)) {
                break;
            }
            selectionEnd.setY(selectionEnd.y() + 1);
            endY = selectionEnd.y();
            if (isSeqInRange(endY)) {
                if (endY != -1) {
                    int startSeq = qMin(selectionStart.y(),endY);
                    int height = qAbs(endY - selectionStart.y()) + 1;
                    int firstColumn = selection.x();
                    int width = selection.width();
                    if (selection.isNull()) {
                        firstColumn = cursorPos.x();
                        width = 1;
                    }
                    MaEditorSelection _selection(firstColumn, startSeq, width, height);
                    setSelection(_selection);
                }
            }
            break;
        case Qt::Key_Delete:
            if (!isAlignmentLocked() && !shift) {
                emit si_startMsaChanging();
                deleteCurrentSelection();
            }
            break;
        case Qt::Key_Home:
            cancelSelection();
            if (shift) { //scroll namelist
                setFirstVisibleSequence(0);
                setCursorPos(QPoint(cursorPos.x(), 0));
            } else { //scroll sequence
                cancelSelection();
                setFirstVisibleBase(0);
                setCursorPos(QPoint(0, cursorPos.y()));
            }
            break;
        case Qt::Key_End:
            cancelSelection();
            if (shift) { //scroll namelist
                int n = getNumDisplayedSequences() - 1;
                setFirstVisibleSequence(n);
                setCursorPos(QPoint(cursorPos.x(), n));
            } else { //scroll sequence
                int n = editor->getAlignmentLen() - 1;
                setFirstVisibleBase(n);
                setCursorPos(QPoint(n, cursorPos.y()));
            }
            break;
        case Qt::Key_PageUp:
            cancelSelection();
            if (shift) { //scroll namelist
                int nVis = getNumVisibleSequences(false);
                int fp = qMax(0, getFirstVisibleSequence() - nVis);
                int cp = qMax(0, cursorPos.y() - nVis);
                setFirstVisibleSequence(fp);
                setCursorPos(QPoint(cursorPos.x(), cp));
            } else { //scroll sequence
                int nVis = getNumVisibleBases(false);
                int fp = qMax(0, getFirstVisibleBase() - nVis);
                int cp = qMax(0, cursorPos.x() - nVis);
                setFirstVisibleBase(fp);
                setCursorPos(QPoint(cp, cursorPos.y()));
            }
            break;
        case Qt::Key_PageDown:
            cancelSelection();
            if (shift) { //scroll namelist
                int nVis = getNumVisibleSequences(false);
                int nSeq = getNumDisplayedSequences();
                int fp = qMin(nSeq-1, getFirstVisibleSequence() + nVis);
                int cp = qMin(nSeq-1, cursorPos.y() + nVis);
                setFirstVisibleSequence(fp);
                setCursorPos(QPoint(cursorPos.x(), cp));
            } else { //scroll sequence
                int nVis = getNumVisibleBases(false);
                int len = editor->getAlignmentLen();
                int fp  = qMin(len-1, getFirstVisibleBase() + nVis);
                int cp  = qMin(len-1, cursorPos.x() + nVis);
                setFirstVisibleBase(fp);
                setCursorPos(QPoint(cp, cursorPos.y()));
            }
            break;
        case Qt::Key_Backspace:
            removeGapsPrecedingSelection(genuineCtrl ? 1 : -1);
            break;
        case Qt::Key_Insert:
        case Qt::Key_Space:
            // We can't use Command+Space on Mac OS X - it is reserved
            if(!isAlignmentLocked()) {
                emit si_startMsaChanging();
                insertGapsBeforeSelection(genuineCtrl ? 1 : -1);
            }
            break;
        case Qt::Key_Shift:
            if (!selection.isNull()) {
                selectionStart = selection.topLeft();
                selectionEnd = selection.getRect().bottomRight();
            } else {
                selectionStart = cursorPos;
                selectionEnd = cursorPos;
            }
            break;
    }
    QWidget::keyPressEvent(e);
}

void MSAEditorSequenceArea::keyReleaseEvent(QKeyEvent *ke) {
    if ((ke->key() == Qt::Key_Space || ke->key() == Qt::Key_Delete) && !isAlignmentLocked() && !ke->isAutoRepeat()) {
        emit si_stopMsaChanging(true);
    }

    QWidget::keyReleaseEvent(ke);
}

void MSAEditorSequenceArea::focusInEvent(QFocusEvent* fe) {
    QWidget::focusInEvent(fe);
    update();
}

void MSAEditorSequenceArea::focusOutEvent(QFocusEvent* fe) {
    QWidget::focusOutEvent(fe);
    exitFromEditCharacterMode();
    update();
}

void MSAEditorSequenceArea::moveCursor(int dx, int dy) {
    QPoint p = cursorPos + QPoint(dx, dy);
    if (!isInRange(p)) {
        return;
    }

    // Move only one cell selection?
    // TODO: consider selection movement
    int sz = selection.width()*selection.height();
    if (sz != 1) {
        return;
    }

    if (!isVisible(p, false)) {
        if (isVisible(cursorPos, true)) {
            if (dx != 0) {
                setFirstVisibleBase(startPos + dx);
            }
            if (dy!=0) {
                setFirstVisibleSequence(getFirstVisibleSequence()+dy);
            }
        } else {
            setFirstVisibleBase(p.x());
            setFirstVisibleSequence(p.y());
        }
    }
    setCursorPos(p);
    //setSelection(MSAEditorSelection(p, 1,1));
}

void MSAEditorSequenceArea::setCursorPos(const QPoint& p) {
    SAFE_POINT(isInRange(p), tr("Cursor position is out of range"), );
    if (p == cursorPos) {
        return;
    }

    cursorPos = p;

    highlightSelection = false;
    updateActions();
}

void MSAEditorSequenceArea::setCursorPos(int x, int y) {
    setCursorPos(QPoint(x, y));
}

void MSAEditorSequenceArea::setCursorPos(int pos) {
    setCursorPos(QPoint(pos, cursorPos.y()));
}

void MSAEditorSequenceArea::highlightCurrentSelection()  {
    highlightSelection = true;
    update();
}

void MSAEditorSequenceArea::removeGapsPrecedingSelection(int countOfGaps) {
    const MaEditorSelection selectionBackup = selection;
    // check if selection exists
    if (selectionBackup.isNull()) {
        return;
    }

    const QPoint selectionTopLeftCorner(selectionBackup.topLeft());
    // don't perform the deletion if the selection is at the alignment start
    if (0 == selectionTopLeftCorner.x() || -1 > countOfGaps || 0 == countOfGaps) {
        return;
    }

    int removedRegionWidth = (-1 == countOfGaps) ? selectionBackup.width() : countOfGaps;
    QPoint topLeftCornerOfRemovedRegion(selectionTopLeftCorner.x() - removedRegionWidth,
        selectionTopLeftCorner.y());
    if (0 > topLeftCornerOfRemovedRegion.x()) {
        removedRegionWidth -= qAbs(topLeftCornerOfRemovedRegion.x());
        topLeftCornerOfRemovedRegion.setX(0);
    }

    // SANGER_TODO: dealing with gaps is not determined for MCA -- let i tb only in MSA for now
    MultipleSequenceAlignmentObject *maObj = getEditor()->getMaObject();
    if (NULL == maObj || maObj->isStateLocked()) {
        return;
    }

    // if this method was invoked during a region shifting
    // then shifting should be canceled
    cancelShiftTracking();

    const U2Region rowsContainingRemovedGaps(getSelectedRows());
    U2OpStatus2Log os;
    U2UseCommonUserModStep userModStep(maObj->getEntityRef(), os);
    Q_UNUSED(userModStep);

    const int countOfDeletedSymbols = maObj->deleteGap(os, rowsContainingRemovedGaps,
        topLeftCornerOfRemovedRegion.x(), removedRegionWidth);

    // if some symbols were actually removed and the selection is not located
    // at the alignment end, then it's needed to move the selection
    // to the place of the removed symbols
    if (0 < countOfDeletedSymbols) {
        const MaEditorSelection newSelection(selectionBackup.x() - countOfDeletedSymbols,
            topLeftCornerOfRemovedRegion.y(), selectionBackup.width(),
            selectionBackup.height());
        setSelection(newSelection);
    }
}

void MSAEditorSequenceArea::sl_alignmentChanged(const MultipleAlignment&, const MaModificationInfo& modInfo) {
    exitFromEditCharacterMode();
    int nSeq = editor->getNumSequences();
    int aliLen = editor->getAlignmentLen();
    if (ui->isCollapsibleMode()) {
        nSeq = getNumDisplayedSequences();
        updateCollapsedGroups(modInfo);
    }

    editor->updateReference();

    //todo: set in one method!
    setFirstVisibleBase(qBound(0, startPos, aliLen-countWidthForBases(false)));
    setFirstVisibleSequence(qBound(0, startSeq, nSeq - countHeightForSequences(false)));

    if ((selection.x() > aliLen - 1) || (selection.y() > nSeq - 1)) {
        cancelSelection();
    } else {
        const QPoint selTopLeft(qMin(selection.x(), aliLen - 1),
            qMin(selection.y(), nSeq - 1));
        const QPoint selBottomRight(qMin(selection.x() + selection.width() - 1, aliLen - 1),
            qMin(selection.y() + selection.height() - 1, nSeq -1));

        MaEditorSelection newSelection(selTopLeft, selBottomRight);
        // we don't emit "selection changed" signal to avoid redrawing
        setSelection(newSelection);
    }

    updateHScrollBar();
    updateVScrollBar();

    completeRedraw = true;
    updateActions();
    update();
}

void MSAEditorSequenceArea::updateCollapsedGroups(const MaModificationInfo& modInfo) {
    U2OpStatus2Log os;
    if(modInfo.rowContentChanged) {
        QList<qint64> updatedRows;
        bool isModelChanged = false;
        QMap<qint64, QList<U2MsaGap> > curGapModel = getEditor()->getMaObject()->getGapModel();
        QList<U2Region> updatedRegions;
        foreach (qint64 modifiedSeqId, modInfo.modifiedRowIds) {
            int modifiedRowPos = editor->getMaObject()->getRowPosById(modifiedSeqId);
            const MultipleSequenceAlignmentRow &modifiedRowRef = editor->getMaObject()->getRow(modifiedRowPos);
            modifiedRowPos = ui->getCollapseModel()->rowToMap(modifiedRowPos);
            const U2Region rowsCollapsibleGroup = ui->getCollapseModel()->mapSelectionRegionToRows(U2Region(modifiedRowPos, 1));
            if (updatedRegions.contains(rowsCollapsibleGroup)) {
                continue;
            }
            for(int i = rowsCollapsibleGroup.startPos; i < rowsCollapsibleGroup.endPos(); i++) {
                qint64 identicalRowId = editor->getMaObject()->getRow(i)->getRowId();
                if(!updatedRows.contains(identicalRowId) && !modInfo.modifiedRowIds.contains(identicalRowId)) {
                    isModelChanged = isModelChanged || modifiedRowRef->getGapModel() != curGapModel[identicalRowId];
                    curGapModel[identicalRowId] = modifiedRowRef->getGapModel();
                    updatedRows.append(identicalRowId);
                }
            }
            updatedRegions.append(rowsCollapsibleGroup);
        }
        if(isModelChanged) {
            getEditor()->getMaObject()->updateGapModel(os, curGapModel);
            return;
        }
    }
}

void MSAEditorSequenceArea::sl_buildStaticToolbar(GObjectView* v, QToolBar* t) {
    t->addAction(ui->getUndoAction());
    t->addAction(ui->getRedoAction());
    t->addAction(gotoAction);
    t->addAction(removeAllGapsAction);
    t->addSeparator();
    // SANGER_TODO: depending on the type of editor - the action should be unavailable
    CHECK(qobject_cast<MSAEditor*>(v) != NULL, );
    t->addAction(collapseModeSwitchAction);
    t->addAction(collapseModeUpdateAction);
    t->addSeparator();
}

void MSAEditorSequenceArea::sl_buildStaticMenu(GObjectView*, QMenu* m) {
    buildMenu(m);
}

void MSAEditorSequenceArea::sl_buildContextMenu(GObjectView*, QMenu* m) {
    buildMenu(m);

    QMenu* editMenu = GUIUtils::findSubMenu(m, MSAE_MENU_EDIT);
    SAFE_POINT(editMenu != NULL, "editMenu", );

    QList<QAction*> actions;
    actions << insSymAction << replaceCharacterAction << reverseComplementAction << reverseAction << complementAction << delColAction << removeAllGapsAction;

    QMenu* copyMenu = GUIUtils::findSubMenu(m, MSAE_MENU_COPY);
    SAFE_POINT(copyMenu != NULL, "copyMenu", );
    editMenu->insertAction(editMenu->actions().first(), ui->getDelSelectionAction());
    if (rect().contains(mapFromGlobal(QCursor::pos()))) {
        editMenu->addActions(actions);
        copyMenu->addAction(ui->getCopySelectionAction());
        copyMenu->addAction(ui->getCopyFormattedSelectionAction());
    }

    m->setObjectName("msa sequence area context menu");
}

void MSAEditorSequenceArea::sl_showCustomSettings(){
    AppContext::getAppSettingsGUI()->showSettingsDialog(ColorSchemaSettingsPageId);
}

void MSAEditorSequenceArea::buildMenu(QMenu* m) {
    QAction* copyMenuAction = GUIUtils::findAction(m->actions(), MSAE_MENU_LOAD);
    m->insertAction(copyMenuAction, gotoAction);

    QMenu* loadSeqMenu = GUIUtils::findSubMenu(m, MSAE_MENU_LOAD);
    SAFE_POINT(loadSeqMenu != NULL, "loadSeqMenu", );
    loadSeqMenu->addAction(addSeqFromProjectAction);
    loadSeqMenu->addAction(addSeqFromFileAction);

    QMenu* editMenu = GUIUtils::findSubMenu(m, MSAE_MENU_EDIT);
    SAFE_POINT(editMenu != NULL, "editMenu", );
    QList<QAction*> actions;
    actions << replaceCharacterAction << reverseComplementAction <<  reverseAction << complementAction << removeAllGapsAction;
    editMenu->insertActions(editMenu->isEmpty() ? NULL : editMenu->actions().first(), actions);

    QMenu * exportMenu = GUIUtils::findSubMenu(m, MSAE_MENU_EXPORT);
    SAFE_POINT(exportMenu != NULL, "exportMenu", );
    exportMenu->addAction(createSubaligniment);
    exportMenu->addAction(saveSequence);

    QMenu* copyMenu = GUIUtils::findSubMenu(m, MSAE_MENU_COPY);
    SAFE_POINT(copyMenu != NULL, "copyMenu", );
    ui->getCopySelectionAction()->setDisabled(selection.isNull());
    emit si_copyFormattedChanging(!selection.isNull());
    copyMenu->addAction(ui->getCopySelectionAction());
    ui->getCopyFormattedSelectionAction()->setDisabled(selection.isNull());
    copyMenu->addAction(ui->getCopyFormattedSelectionAction());
    copyMenu->addAction(ui->getPasteAction());

    QMenu* viewMenu = GUIUtils::findSubMenu(m, MSAE_MENU_VIEW);
    SAFE_POINT(viewMenu != NULL, "viewMenu", );
    viewMenu->addAction(sortByNameAction);

    QMenu* colorsSchemeMenu = new QMenu(tr("Colors"), NULL);
    colorsSchemeMenu->menuAction()->setObjectName("Colors");
    colorsSchemeMenu->setIcon(QIcon(":core/images/color_wheel.png"));
    foreach(QAction* a, colorSchemeMenuActions) {
        colorsSchemeMenu->addAction(a);
    }

    QMenu* customColorSchemaMenu = new QMenu(tr("Custom schemes"), colorsSchemeMenu);
    customColorSchemaMenu->menuAction()->setObjectName("Custom schemes");

    foreach(QAction* a, customColorSchemeMenuActions) {
        customColorSchemaMenu->addAction(a);
    }

    if (!customColorSchemeMenuActions.isEmpty()){
        customColorSchemaMenu->addSeparator();
    }

    lookMSASchemesSettingsAction = new QAction(tr("Create new color scheme"), this);
    lookMSASchemesSettingsAction->setObjectName("Create new color scheme");
    connect(lookMSASchemesSettingsAction, SIGNAL(triggered()), SLOT(sl_showCustomSettings()));
    customColorSchemaMenu->addAction(lookMSASchemesSettingsAction);

    colorsSchemeMenu->addMenu(customColorSchemaMenu);
    m->insertMenu(GUIUtils::findAction(m->actions(), MSAE_MENU_EDIT), colorsSchemeMenu);

    QMenu* highlightSchemeMenu = new QMenu(tr("Highlighting"), NULL);

    highlightSchemeMenu->menuAction()->setObjectName("Highlighting");

    foreach(QAction* a, highlightingSchemeMenuActions) {
        highlightSchemeMenu->addAction(a);
    }
    highlightSchemeMenu->addSeparator();
    highlightSchemeMenu->addAction(useDotsAction);
    m->insertMenu(GUIUtils::findAction(m->actions(), MSAE_MENU_EDIT), highlightSchemeMenu);
}

void MSAEditorSequenceArea::sl_fontChanged(QFont font) {
    Q_UNUSED(font);
    completeRedraw = true;
    repaint();
}

void MSAEditorSequenceArea::sl_alphabetChanged(const MaModificationInfo &mi, const DNAAlphabet *prevAlphabet) {
    updateColorAndHighlightSchemes();

    QString message;
    if (mi.alphabetChanged || mi.type != MaModificationType_Undo) {
        message = tr("The alignment has been modified, so that its alphabet has been switched from \"%1\" to \"%2\". Use \"Undo\", if you'd like to restore the original alignment.")
            .arg(prevAlphabet->getName()).arg(editor->getMaObject()->getAlphabet()->getName());
    }

    if (message.isEmpty()) {
        return;
    }
    const NotificationStack *notificationStack = AppContext::getMainWindow()->getNotificationStack();
    CHECK(notificationStack != NULL, );
    notificationStack->addNotification(message, Info_Not);
}

void MSAEditorSequenceArea::sl_delCol() {
    QObjectScopedPointer<DeleteGapsDialog> dlg = new DeleteGapsDialog(this, editor->getMaObject()->getNumRows());
    dlg->exec();
    CHECK(!dlg.isNull(), );

    if (dlg->result() == QDialog::Accepted) {
        MSACollapsibleItemModel *collapsibleModel = ui->getCollapseModel();
        SAFE_POINT(NULL != collapsibleModel, tr("NULL collapsible model!"), );
        collapsibleModel->reset();

        DeleteMode deleteMode = dlg->getDeleteMode();
        int value = dlg->getValue();

        // if this method was invoked during a region shifting
        // then shifting should be canceled
        cancelShiftTracking();

        MultipleSequenceAlignmentObject* msaObj = getEditor()->getMaObject();
        int gapCount = GAP_COLUMN_ONLY;
        switch(deleteMode) {
        case DeleteByAbsoluteVal:
            gapCount = value;
            break;
        case DeleteByRelativeVal: {
            int absoluteValue = qRound((msaObj->getNumRows() * value) / 100.0);
            if (absoluteValue < 1) {
                absoluteValue = 1;
            }
            gapCount = absoluteValue;
            break;
        }
        case DeleteAll:
            gapCount = GAP_COLUMN_ONLY;
            break;
        default:
            FAIL("Unknown delete mode", );
        }

        QList<qint64> columnsToDelete = msaObj->getColumnsWithGaps(gapCount);
        if (columnsToDelete.isEmpty()) {
            return;
        }
        U2OpStatus2Log os;
        U2UseCommonUserModStep userModStep(msaObj->getEntityRef(), os);
        Q_UNUSED(userModStep);
        SAFE_POINT_OP(os, );
        msaObj->deleteColumnWithGaps(gapCount);
    }
}

void MSAEditorSequenceArea::sl_fillCurrentSelectionWithGaps() {
    if(!isAlignmentLocked()) {
        emit si_startMsaChanging();
        insertGapsBeforeSelection();
        emit si_stopMsaChanging(true);
    }
}

void MSAEditorSequenceArea::sl_goto() {
    QObjectScopedPointer<QDialog> dlg = new QDialog(this);
    dlg->setModal(true);
    dlg->setWindowTitle(tr("Go To"));
    int aliLen = editor->getAlignmentLen();
    PositionSelector *ps = new PositionSelector(dlg.data(), 1, aliLen, true);
    connect(ps, SIGNAL(si_positionChanged(int)), SLOT(sl_onPosChangeRequest(int)));
    dlg->exec();
}

void MSAEditorSequenceArea::sl_onPosChangeRequest(int pos) {
    centerPos(pos-1);
    setSelection(MaEditorSelection(pos-1,selection.y(),1,1));
}

void MSAEditorSequenceArea::sl_lockedStateChanged() {
    updateActions();
}

void MSAEditorSequenceArea::centerPos(const QPoint& pos) {
    assert(isInRange(pos));
    int newStartPos = qMax(0, pos.x() - getNumVisibleBases(false)/2);
    setFirstVisibleBase(newStartPos);

    int newStartSeq = qMax(0, pos.y() - getNumVisibleSequences(false)/2);
    setFirstVisibleSequence(newStartSeq);
}

void MSAEditorSequenceArea::centerPos(int pos) {
    centerPos(QPoint(pos, cursorPos.y()));
}

void MSAEditorSequenceArea::wheelEvent (QWheelEvent * we) {
    bool toMin = we->delta() > 0;
    if (we->modifiers() == 0) {
        shBar->triggerAction(toMin ? QAbstractSlider::SliderSingleStepSub : QAbstractSlider::SliderSingleStepAdd);
    }  else if (we->modifiers() & Qt::SHIFT) {
        svBar->triggerAction(toMin ? QAbstractSlider::SliderSingleStepSub : QAbstractSlider::SliderSingleStepAdd);
    }
    QWidget::wheelEvent(we);
}

void MSAEditorSequenceArea::sl_removeAllGaps() {
    MultipleSequenceAlignmentObject* msa = getEditor()->getMaObject();
    SAFE_POINT(NULL != msa, tr("NULL msa object!"), );
    assert(!msa->isStateLocked());

    MSACollapsibleItemModel *collapsibleModel = ui->getCollapseModel();
    SAFE_POINT(NULL != collapsibleModel, tr("NULL collapsible model!"), );
    collapsibleModel->reset();

    // if this method was invoked during a region shifting
    // then shifting should be canceled
    cancelShiftTracking();

    U2OpStatus2Log os;
    U2UseCommonUserModStep userModStep(msa->getEntityRef(), os);
    Q_UNUSED(userModStep);
    SAFE_POINT_OP(os, );

    QMap<qint64, QList<U2MsaGap> > noGapModel;
    foreach (qint64 rowId, msa->getMultipleAlignment()->getRowsIds()) {
        noGapModel[rowId] = QList<U2MsaGap>();
    }

    msa->updateGapModel(os, noGapModel);

    MsaDbiUtils::trim(msa->getEntityRef(), os);
    msa->updateCachedMultipleAlignment();

    SAFE_POINT_OP(os, );

    setFirstVisibleBase(0);
    setFirstVisibleSequence(0);
    SAFE_POINT_OP(os, );
}

void MSAEditorSequenceArea::sl_modelChanged() {
    MSACollapsibleItemModel *collapsibleModel = ui->getCollapseModel();
    SAFE_POINT(NULL != collapsibleModel, tr("NULL collapsible model!"), );

    if (collapsibleModel->isEmpty()) {
        collapseModeSwitchAction->setChecked(false);
        collapseModeUpdateAction->setEnabled(false);
    }

    int startToLast = getNumDisplayedSequences() - getFirstVisibleSequence();
    int availableNum = countHeightForSequences(false);
    if (startToLast < availableNum) {
        int newStartSeq = qMax(0, startSeq - availableNum + startToLast);
        if (startSeq != newStartSeq) {
            setFirstVisibleSequence(newStartSeq);
            return;
        }
    }
    updateSelection();

    completeRedraw = true;
    updateVScrollBar();
    update();
}

void MSAEditorSequenceArea::sl_completeUpdate(){
    completeRedraw = true;
    validateRanges();
    updateActions();
    update();
    onVisibleRangeChanged();
}

void MSAEditorSequenceArea::sl_createSubaligniment(){
    CHECK(getEditor() != NULL, );
    QObjectScopedPointer<CreateSubalignmentDialogController> dialog = new CreateSubalignmentDialogController(getEditor()->getMaObject(), selection.getRect(), this);
    dialog->exec();
    CHECK(!dialog.isNull(), );

    if (dialog->result() == QDialog::Accepted){
        U2Region window = dialog->getRegion();
        bool addToProject = dialog->getAddToProjFlag();
        QString path = dialog->getSavePath();
        QStringList seqNames = dialog->getSelectedSeqNames();
        Task* csTask = new CreateSubalignmentAndOpenViewTask(getEditor()->getMaObject(),
            CreateSubalignmentSettings(window, seqNames, path, true, addToProject, dialog->getFormatId()));
        AppContext::getTaskScheduler()->registerTopLevelTask(csTask);
    }
}

void MSAEditorSequenceArea::sl_saveSequence(){
    CHECK(getEditor() != NULL, );
    int seqIndex = selection.y();

    if(selection.height() > 1){
        QMessageBox::critical(NULL, tr("Warning!"), tr("You must select only one sequence for export."));
        return;
    }

    QString seqName = editor->getMaObject()->getMultipleAlignment()->getRow(seqIndex)->getName();
    QObjectScopedPointer<SaveSelectedSequenceFromMSADialogController> d = new SaveSelectedSequenceFromMSADialogController((QWidget*)AppContext::getMainWindow()->getQMainWindow());
    const int rc = d->exec();
    CHECK(!d.isNull(), );

    if (rc == QDialog::Rejected) {
        return;
    }
    //TODO: OPTIMIZATION code below can be wrapped to task
    DNASequence seq;
    foreach(const DNASequence &s,  MSAUtils::ma2seq(getEditor()->getMaObject()->getMsa(), d->trimGapsFlag)){
        if (s.getName() == seqName){
            seq = s;
            break;
        }
    }

    U2OpStatus2Log  os;
    QString fullPath = GUrlUtils::prepareFileLocation(d->url, os);
    CHECK_OP(os, );
    GUrl url(fullPath);

    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(d->url));
    DocumentFormat *df = AppContext::getDocumentFormatRegistry()->getFormatById(d->format);
    Document *doc;
    QList<GObject*> objs;
    doc = df->createNewLoadedDocument(iof, fullPath, os);
    CHECK_OP_EXT(os, delete doc, );
    U2SequenceObject* seqObj = DocumentFormatUtils::addSequenceObjectDeprecated(doc->getDbiRef(), U2ObjectDbi::ROOT_FOLDER, seq.getName(), objs, seq, os);
    CHECK_OP_EXT(os, delete doc, );
    doc->addObject(seqObj);
    SaveDocumentTask *t = new SaveDocumentTask(doc, doc->getIOAdapterFactory(), doc->getURL());

    if (d->addToProjectFlag){
        Project *p = AppContext::getProject();
        Document *loadedDoc=p->findDocumentByURL(url);
        if (loadedDoc) {
            coreLog.details("The document already in the project");
            QMessageBox::warning(ui, tr("warning"), tr("The document already in the project"));
            return;
        }
        p->addDocument(doc);

        // Open view for created document
        DocumentSelection ds;
        ds.setSelection(QList<Document*>() <<doc);
        MultiGSelection ms;
        ms.addSelection(&ds);
        foreach (GObjectViewFactory *f, AppContext::getObjectViewFactoryRegistry()->getAllFactories()) {
            if (f->canCreateView(ms)) {
                Task *tt = f->createViewTask(ms);
                AppContext::getTaskScheduler()->registerTopLevelTask(tt);
                break;
            }
        }
    }else{
        t->addFlag(SaveDoc_DestroyAfter);
    }
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void MSAEditorSequenceArea::sl_registerCustomColorSchemes() {
    deleteOldCustomSchemes();

    MsaColorSchemeRegistry *msaColorSchemeRegistry = AppContext::getMsaColorSchemeRegistry();
    QList<MsaColorSchemeFactory *> customFactories = msaColorSchemeRegistry->getMsaCustomColorSchemes(editor->getMaObject()->getAlphabet()->getType());

    foreach (MsaColorSchemeFactory *factory, customFactories) {
        QAction *action = new QAction(factory->getName(), this);
        action->setObjectName(factory->getName());
        action->setCheckable(true);
        action->setData(factory->getId());
        connect(action, SIGNAL(triggered()), SLOT(sl_changeColorScheme()));
        customColorSchemeMenuActions.append(action);
    }
}

void MSAEditorSequenceArea::sl_colorSchemeFactoryUpdated() {
    applyColorScheme(colorScheme->getFactory()->getId());
}

void MSAEditorSequenceArea::sl_setDefaultColorScheme() {
    MsaColorSchemeFactory *defaultFactory = getDefaultColorSchemeFactory();
    SAFE_POINT(NULL != defaultFactory, L10N::nullPointerError("default color scheme factory"), );
    applyColorScheme(defaultFactory->getId());
}

void MSAEditorSequenceArea::updateHBarPosition(int base) {
    if (isAlignmentEmpty()) {
        shBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
        return;
    }

    if (base <= getFirstVisibleBase()) {
        shBar->setupRepeatAction(QAbstractSlider::SliderSingleStepSub, 50, 10);
    } else  if (base >= getLastVisibleBase(true)) {
        shBar->setupRepeatAction(QAbstractSlider::SliderSingleStepAdd, 50, 10);
    } else {
        shBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
    }
}

void MSAEditorSequenceArea::updateVBarPosition(int seq) {
    if (isAlignmentEmpty()) {
        svBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
        return;
    }

    if (seq <= getFirstVisibleSequence()) {
        svBar->setupRepeatAction(QAbstractSlider::SliderSingleStepSub, 50, 10);
    } else if (seq >= getLastVisibleSequence(true)) {
        svBar->setupRepeatAction(QAbstractSlider::SliderSingleStepAdd, 50, 10);
    } else {
        svBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
    }
}

void MSAEditorSequenceArea::sl_replaceSelectedCharacter() {
    msaMode = EditCharacterMode;
    editModeAnimationTimer.start(500);
    highlightCurrentSelection();
}

void MSAEditorSequenceArea::sl_delCurrentSelection()
{
    emit si_startMsaChanging();
    deleteCurrentSelection();
    emit si_stopMsaChanging(true);
}

U2Region MSAEditorSequenceArea::getRowsAt(int pos) const {
    if (!ui->isCollapsibleMode()) {
        return U2Region(pos, 1);
    }

    MSACollapsibleItemModel* m = ui->getCollapseModel();
    int itemIdx = m->itemAt(pos);
    if (itemIdx >= 0) {
        const MSACollapsableItem& item = m->getItem(itemIdx);
        return U2Region(item.row, item.numRows);
    }
    return U2Region(m->mapToRow(pos), 1);
}

void MSAEditorSequenceArea::sl_copyCurrentSelection()
{
    CHECK(getEditor() != NULL, );
    // TODO: probably better solution would be to export selection???

    assert(isInRange(selection.topLeft()));
    assert(isInRange(QPoint(selection.x() + selection.width() - 1, selection.y() + selection.height() - 1)));

    MultipleSequenceAlignmentObject* maObj = getEditor()->getMaObject();
    if (selection.isNull()) {
        return;
    }

    MSACollapsibleItemModel* m = ui->getCollapseModel();
    U2Region sel(m->mapToRow(selection.y()), m->mapToRow(selection.y() + selection.height()) - m->mapToRow(selection.y()));

    QString selText;
    U2OpStatus2Log os;
    for (int i = sel.startPos; i < sel.endPos(); ++i) {
        if (ui->getCollapseModel()->rowToMap(i, true) < 0) {
            continue;
        }
        int len = selection.width();
        QByteArray seqPart = maObj->getMsaRow(i)->mid(selection.x(), len, os)->toByteArray(len, os);
        selText.append(seqPart);
        if (i + 1 != sel.endPos()) { // do not add line break into the last line
            selText.append("\n");
        }
    }
    QApplication::clipboard()->setText(selText);
}

void MSAEditorSequenceArea::sl_copyFormattedSelection(){
    const DocumentFormatId& formatId = getCopyFormatedAlgorithmId();
    Task* clipboardTask = new SubalignmentToClipboardTask(editor, selection.getRect(), formatId);
    AppContext::getTaskScheduler()->registerTopLevelTask(clipboardTask);
}

void MSAEditorSequenceArea::sl_paste(){
    MultipleAlignmentObject* msaObject = editor->getMaObject();
    if (msaObject->isStateLocked()) {
        return;
    }
    PasteFactory* pasteFactory = AppContext::getPasteFactory();
    SAFE_POINT(pasteFactory != NULL, "PasteFactory is null", );

    bool focus = ui->isAncestorOf(QApplication::focusWidget());
    PasteTask* task = pasteFactory->pasteTask(!focus);
    if (focus) {
        connect(new TaskSignalMapper(task), SIGNAL(si_taskFinished(Task *)), SLOT(sl_pasteFinished(Task*)));
    }
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
}

void MSAEditorSequenceArea::sl_pasteFinished(Task* _pasteTask){
    CHECK(getEditor() != NULL, );
    MultipleSequenceAlignmentObject* msaObject = getEditor()->getMaObject();
    if (msaObject->isStateLocked()) {
        return;
    }

    PasteTask* pasteTask = qobject_cast<PasteTask*>(_pasteTask);
    if(NULL == pasteTask || pasteTask->isCanceled()) {
        return;
    }
    const QList<Document*>& docs = pasteTask->getDocuments();

    AddSequencesFromDocumentsToAlignmentTask *task = new AddSequencesFromDocumentsToAlignmentTask(msaObject, docs);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
}

bool MSAEditorSequenceArea::shiftSelectedRegion(int shift) {
    CHECK(getEditor() != NULL, false);
    if (0 == shift) {
        return true;
    }

    MultipleSequenceAlignmentObject *maObj = getEditor()->getMaObject();
    if (!maObj->isStateLocked()) {
        const U2Region rows = getSelectedRows();
        const int x = selection.x();
        const int y = rows.startPos;
        const int width = selection.width();
        const int height = rows.length;
        if (maObj->isRegionEmpty(x, y, width, height)) {
            return true;
        }
        // backup current selection for the case when selection might disappear
        const MaEditorSelection selectionBackup = selection;

        const int resultShift = maObj->shiftRegion(x, y, width, height, shift);
        if (0 != resultShift) {
            int newCursorPosX = (cursorPos.x() + resultShift >= 0) ? cursorPos.x() + resultShift : 0;
            setCursorPos(newCursorPosX);

            const MaEditorSelection newSelection(selectionBackup.x() + resultShift, selectionBackup.y(),
                selectionBackup.width(), selectionBackup.height());
            setSelection(newSelection);
            if ((selectionBackup.getRect().right() == getLastVisibleBase(false) && resultShift > 0)
                || (selectionBackup.x() == getFirstVisibleBase() && 0 > resultShift))
            {
                setFirstVisibleBase(startPos + resultShift);
            }
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void MSAEditorSequenceArea::deleteCurrentSelection() {
    CHECK(getEditor() != NULL, );

    if (selection.isNull()) {
        return;
    }
    assert(isInRange(selection.topLeft()));
    assert(isInRange(QPoint(selection.x() + selection.width() - 1, selection.y() + selection.height() - 1)));
    MultipleSequenceAlignmentObject* maObj = getEditor()->getMaObject();
    if (maObj == NULL || maObj->isStateLocked()) {
        return;
    }

    const QRect areaBeforeSelection(0, 0, selection.x(), selection.height());
    const QRect areaAfterSelection(selection.x() + selection.width(), selection.y(),
        maObj->getLength() - selection.x() - selection.width(), selection.height());
    if (maObj->isRegionEmpty(areaBeforeSelection.x(), areaBeforeSelection.y(), areaBeforeSelection.width(), areaBeforeSelection.height())
        && maObj->isRegionEmpty(areaAfterSelection.x(), areaAfterSelection.y(), areaAfterSelection.width(), areaAfterSelection.height())
        && selection.height() == maObj->getNumRows())
    {
        return;
    }

    // if this method was invoked during a region shifting
    // then shifting should be canceled
    cancelShiftTracking();

    U2OpStatusImpl os;
    U2UseCommonUserModStep userModStep(maObj->getEntityRef(), os);
    Q_UNUSED(userModStep);
    SAFE_POINT_OP(os, );

    const U2Region& sel = getSelectedRows();
    maObj->removeRegion(selection.x(), sel.startPos, selection.width(), sel.length, true);

    if (selection.height() == 1 && selection.width() == 1) {
        if (isInRange(selection.topLeft())) {
            return;
        }
    }
    cancelSelection();
}

void MSAEditorSequenceArea::processCharacterInEditMode(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape) {
        exitFromEditCharacterMode();
        return;
    }

    QString text = e->text().toUpper();
    if (1 == text.length()) {
        QChar emDash(0x2015);
        QRegExp latinCharacterOrGap(QString("([A-Z]| |-|%1)").arg(emDash));
        if (latinCharacterOrGap.exactMatch(text)) {
            QChar newChar = text.at(0);
            newChar = (newChar == '-' || newChar == emDash || newChar == ' ') ? U2Msa::GAP_CHAR : newChar;
            replaceSelectedCharacter(newChar.toLatin1());
        }
        else {
            MainWindow *mainWindow = AppContext::getMainWindow();
            const QString message = tr("It is not possible to insert the character into the alignment."
                                       "Please use a character from set A-Z (upper-case or lower-case) or the gap character ('Space', '-' or '%1').").arg(emDash);
            mainWindow->addNotification(message, Error_Not);
            exitFromEditCharacterMode();
        }
    }
}

void MSAEditorSequenceArea::replaceSelectedCharacter(char newCharacter) {
    CHECK(getEditor() != NULL, );
    if (selection.isNull()) {
        return;
    }
    SAFE_POINT(isInRange(selection.topLeft()), "Incorrect selection is detected!", );
    MultipleSequenceAlignmentObject* maObj = getEditor()->getMaObject();
    if (maObj == NULL || maObj->isStateLocked()) {
        return;
    }

    U2OpStatusImpl os;
    U2UseCommonUserModStep userModStep(maObj->getEntityRef(), os);
    Q_UNUSED(userModStep);
    SAFE_POINT_OP(os, );

    const U2Region& sel = getSelectedRows();
    for (qint64 rowIndex = sel.startPos; rowIndex < sel.endPos(); rowIndex++) {
        maObj->replaceCharacter(selection.x(), rowIndex, newCharacter);
    }

    exitFromEditCharacterMode();
}

void MSAEditorSequenceArea::addRowToSelection(int rowNumber) {
    selectedRows.append(rowNumber);
}

void MSAEditorSequenceArea::deleteRowFromSelection(int rowNumber) {
    selectedRows.removeAll(rowNumber);
}

void MSAEditorSequenceArea::clearSelection() {
    selectedRows.clear();
}

void MSAEditorSequenceArea::sl_addSeqFromFile()
{
    CHECK(getEditor() != NULL, );
    MultipleSequenceAlignmentObject* msaObject = getEditor()->getMaObject();
    if (msaObject->isStateLocked()) {
        return;
    }

    QString filter = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::SEQUENCE, true);

    LastUsedDirHelper lod;
    QStringList urls;
#ifdef Q_OS_MAC
    if (qgetenv(ENV_GUI_TEST).toInt() == 1 && qgetenv(ENV_USE_NATIVE_DIALOGS).toInt() == 0) {
        urls = U2FileDialog::getOpenFileNames(this, tr("Open file with sequences"), lod.dir, filter, 0, QFileDialog::DontUseNativeDialog);
    } else
#endif
    urls = U2FileDialog::getOpenFileNames(this, tr("Open file with sequences"), lod.dir, filter);

    if (!urls.isEmpty()) {
        lod.url = urls.first();
        cancelSelection();
        AddSequencesFromFilesToAlignmentTask *task = new AddSequencesFromFilesToAlignmentTask(msaObject, urls);
        TaskWatchdog::trackResourceExistence(msaObject, task, tr("A problem occurred during adding sequences. The multiple alignment is no more available."));
        AppContext::getTaskScheduler()->registerTopLevelTask(task);
    }

}

void MSAEditorSequenceArea::sl_addSeqFromProject()
{
    CHECK(getEditor() != NULL, );
    MultipleSequenceAlignmentObject* msaObject = getEditor()->getMaObject();
    if (msaObject->isStateLocked()) {
        return;
    }

    ProjectTreeControllerModeSettings settings;
    settings.objectTypesToShow.insert(GObjectTypes::SEQUENCE);

    QList<GObject*> objects = ProjectTreeItemSelectorDialog::selectObjects(settings,this);
    QList<DNASequence> objectsToAdd;
    U2OpStatus2Log os;
    foreach(GObject* obj, objects) {
        U2SequenceObject *seqObj = qobject_cast<U2SequenceObject*>(obj);
        if (seqObj) {
            objectsToAdd.append(seqObj->getWholeSequence(os));
            SAFE_POINT_OP(os, );
        }
    }
    if (objectsToAdd.size() > 0) {
        AddSequenceObjectsToAlignmentTask *addSeqObjTask = new AddSequenceObjectsToAlignmentTask(getEditor()->getMaObject(), objectsToAdd);
        AppContext::getTaskScheduler()->registerTopLevelTask(addSeqObjTask);
        cancelSelection();
    }
}

void MSAEditorSequenceArea::sl_sortByName() {
    CHECK(getEditor() != NULL, );
    MultipleSequenceAlignmentObject* msaObject = getEditor()->getMaObject();
    if (msaObject->isStateLocked()) {
        return;
    }
    MultipleSequenceAlignment msa = msaObject->getMultipleAlignmentCopy();
    msa->sortRowsByName();
    QStringList rowNames = msa->getRowNames();
    if (rowNames != msaObject->getMultipleAlignment()->getRowNames()) {
        U2OpStatusImpl os;
        msaObject->updateRowsOrder(os, msa->getRowsIds());
        SAFE_POINT_OP(os, );
    }
    if (ui->isCollapsibleMode()) {
        sl_updateCollapsingMode();
    }
}

void MSAEditorSequenceArea::sl_setCollapsingMode(bool enabled) {
    CHECK(getEditor() != NULL, );
    GCOUNTER(cvar, tvar, "Switch collapsing mode");

    MultipleSequenceAlignmentObject* msaObject = getEditor()->getMaObject();
    int prevNumVisibleSequences = getNumVisibleSequences(false);
    if (msaObject == NULL  || msaObject->isStateLocked()) {
        if (collapseModeSwitchAction->isChecked()) {
            collapseModeSwitchAction->setChecked(false);
            collapseModeUpdateAction->setEnabled(false);
        }
        return;
    }

    ui->setCollapsibleMode(enabled);
    if (enabled) {
        collapseModeUpdateAction->setEnabled(true);
        sl_updateCollapsingMode();
    }
    else {
        collapseModeUpdateAction->setEnabled(false);

        MSACollapsibleItemModel *collapsibleModel = ui->getCollapseModel();
        SAFE_POINT(NULL != collapsibleModel, tr("NULL collapsible model!"), );
        collapsibleModel->reset();
    }

    updateSelection();
    updateVScrollBar();
    int emptyRowsCount = prevNumVisibleSequences - getNumVisibleSequences(false);
    if(emptyRowsCount > 0) {
        setFirstVisibleSequence(qMax(getFirstVisibleSequence() - emptyRowsCount, 0));
    }
}

void MSAEditorSequenceArea::sl_updateCollapsingMode() {
    CHECK(getEditor() != NULL, );
    GCOUNTER(cvar, tvar, "Update collapsing mode");

    CHECK(ui->isCollapsibleMode(), );
    MultipleSequenceAlignmentObject *msaObject = getEditor()->getMaObject();
    SAFE_POINT(NULL != msaObject, tr("NULL Msa Object!"), );

    MSACollapsibleItemModel *collapsibleModel = ui->getCollapseModel();

    Document *doc = msaObject->getDocument();
    SAFE_POINT(NULL != doc, tr("NULL document!"), );

    MultipleSequenceAlignment msa = msaObject->getMultipleAlignmentCopy();
    QVector<U2Region> unitedRows;
    bool sorted = msa->sortRowsBySimilarity(unitedRows);
    collapsibleModel->reset(unitedRows);

    U2OpStatusImpl os;
    if (sorted) {
        msaObject->updateRowsOrder(os, msa->getRowsIds());
        SAFE_POINT_OP(os, );
    }

    MaModificationInfo mi;
    mi.alignmentLengthChanged = false;
    msaObject->updateCachedMultipleAlignment(mi);
}

void MSAEditorSequenceArea::insertGapsBeforeSelection(int countOfGaps) {
    CHECK(getEditor() != NULL, );
    if (selection.isNull() || 0 == countOfGaps || -1 > countOfGaps) {
        return;
    }
    SAFE_POINT(isInRange(selection.topLeft()), tr("Top left corner of the selection has incorrect coords"), );
    SAFE_POINT(isInRange(QPoint(selection.x() + selection.width() - 1, selection.y() + selection.height() - 1)),
        tr("Bottom right corner of the selection has incorrect coords"), );

    // if this method was invoked during a region shifting
    // then shifting should be canceled
    cancelShiftTracking();

    MultipleSequenceAlignmentObject *maObj = getEditor()->getMaObject();
    if (NULL == maObj || maObj->isStateLocked()) {
        return;
    }
    U2OpStatus2Log os;
    U2UseCommonUserModStep userModStep(maObj->getEntityRef(), os);
    Q_UNUSED(userModStep);
    SAFE_POINT_OP(os,);

    const MultipleSequenceAlignment ma = maObj->getMultipleAlignment();
    if (selection.width() == ma->getLength() && selection.height() == ma->getNumRows()) {
        return;
    }

    const int removedRegionWidth = (-1 == countOfGaps) ? selection.width() : countOfGaps;
    const U2Region& sequences = getSelectedRows();
    maObj->insertGap(sequences,  selection.x() , removedRegionWidth);
    moveSelection(removedRegionWidth, 0, true);
}

void MSAEditorSequenceArea::reverseComplementModification(ModificationType& type) {
    CHECK(getEditor() != NULL, );
    if (type == ModificationType::NoType)
        return;
    MultipleSequenceAlignmentObject* maObj = getEditor()->getMaObject();
    if (maObj == NULL || maObj->isStateLocked()) {
        return;
    }
    if (!maObj->getAlphabet()->isNucleic()) {
        return;
    }
    if (selection.height() == 0) {
        return;
    }
    assert(isInRange(selection.topLeft()));
    assert(isInRange(QPoint(selection.x() + selection.width() - 1, selection.y() + selection.height() - 1)));
    if (!selection.isNull()) {
        // if this method was invoked during a region shifting
        // then shifting should be canceled
        cancelShiftTracking();

        const MultipleSequenceAlignment ma = maObj->getMultipleAlignment();
        DNATranslation* trans = AppContext::getDNATranslationRegistry()->lookupComplementTranslation(ma->getAlphabet());
        if (trans == NULL || !trans->isOne2One()) {
            return;
        }

        U2OpStatus2Log os;
        U2UseCommonUserModStep userModStep(maObj->getEntityRef(), os);
        Q_UNUSED(userModStep);
        SAFE_POINT_OP(os, );

        const U2Region& sel = getSelectedRows();

        QList<qint64> modifiedRowIds;
        modifiedRowIds.reserve(sel.length);
        for (int i = sel.startPos; i < sel.endPos(); i++) {
            const MultipleSequenceAlignmentRow currentRow = ma->getMsaRow(i);
            QByteArray currentRowContent = currentRow->toByteArray(ma->getLength(), os);
            switch (type.getType())
            {
            case ModificationType::Reverse:
                TextUtils::reverse(currentRowContent.data(), currentRowContent.length());
                break;
            case ModificationType::Complement:
                trans->translate(currentRowContent.data(), currentRowContent.length());
                break;
            case ModificationType::ReverseComplement:
                TextUtils::reverse(currentRowContent.data(), currentRowContent.length());
                trans->translate(currentRowContent.data(), currentRowContent.length());
                break;
            }
            QString name = currentRow->getName();
            ModificationType oldType(ModificationType::NoType);
            if (name.endsWith("|revcompl")) {
                name.resize(name.length() - QString("|revcompl").length());
                oldType = ModificationType::ReverseComplement;
            } else if (name.endsWith("|compl")) {
                name.resize(name.length() - QString("|compl").length());
                oldType = ModificationType::Complement;
            } else if (name.endsWith("|rev")) {
                name.resize(name.length() - QString("|rev").length());
                oldType = ModificationType::Reverse;
            }
            ModificationType newType = type + oldType;
            switch (newType.getType())
            {
            case ModificationType::NoType:
                break;
            case ModificationType::Reverse:
                name.append("|rev");
                break;
            case ModificationType::Complement:
                name.append("|compl");
                break;
            case ModificationType::ReverseComplement:
                name.append("|revcompl");
                break;
            }

            // Split the sequence into gaps and chars
            QByteArray seqBytes;
            QList<U2MsaGap> gapModel;
            MsaDbiUtils::splitBytesToCharsAndGaps(currentRowContent, seqBytes, gapModel);

            maObj->updateRow(os, i, name, seqBytes, gapModel);
            modifiedRowIds << currentRow->getRowId();
        }

        MaModificationInfo modInfo;
        modInfo.modifiedRowIds = modifiedRowIds;
        modInfo.alignmentLengthChanged = false;
        maObj->updateCachedMultipleAlignment(modInfo);
    }
}

void MSAEditorSequenceArea::sl_reverseComplementCurrentSelection() {
    ModificationType type(ModificationType::ReverseComplement);
    reverseComplementModification(type);
}

void MSAEditorSequenceArea::sl_reverseCurrentSelection() {
    ModificationType type(ModificationType::Reverse);
    reverseComplementModification(type);
}

void MSAEditorSequenceArea::sl_complementCurrentSelection() {
    ModificationType type(ModificationType::Complement);
    reverseComplementModification(type);
}

QPair<QString, int> MSAEditorSequenceArea::getGappedColumnInfo() const{
    QPair<QString, int> p; // SANGER_TODO: ',' is embarrases CHECK method(?)
    CHECK(getEditor() != NULL, p);
    if (isAlignmentEmpty()) {
        return QPair<QString, int>(QString::number(0), 0);
    }

    const MultipleSequenceAlignmentRow row = getEditor()->getMaObject()->getMsaRow(getSelectedRows().startPos);
    int len = row->getUngappedLength();
    QChar current = row->charAt(selection.topLeft().x());
    if(current == U2Msa::GAP_CHAR){
        return QPair<QString, int>(QString("gap"),len);
    }else{
        int pos = row->getUngappedPosition(selection.topLeft().x());
        return QPair<QString, int>(QString::number(pos + 1),len);
    }
}

void MSAEditorSequenceArea::sl_resetCollapsibleModel() {
    editor->resetCollapsibleModel();
}

void MSAEditorSequenceArea::sl_setCollapsingRegions(const QList<QStringList>& collapsedGroups) {
    CHECK(getEditor() != NULL, );
    MSACollapsibleItemModel* m = ui->getCollapseModel();
    SAFE_POINT(NULL != m, tr("Incorrect pointer to MSACollapsibleItemModel"),);
    m->reset();

    MultipleSequenceAlignmentObject* msaObject = getEditor()->getMaObject();
    QStringList rowNames = msaObject->getMultipleAlignment()->getRowNames();
    QVector<U2Region> collapsedRegions;

    //Calculate regions of the groups
    foreach(const QStringList& seqsGroup, collapsedGroups) {
        int regionStart = rowNames.size(), regionEnd = 0;
        foreach(const QString& seqName, seqsGroup) {
            int seqPos = rowNames.indexOf(seqName);
            regionStart = qMin(seqPos, regionStart);
            regionEnd = qMax(seqPos, regionEnd);
        }
        if(regionStart > 0 && regionEnd <= rowNames.size() && regionEnd > regionStart) {
            collapsedRegions.append(U2Region(regionStart, regionEnd - regionStart + 1));
        }
    }
    //Function 'reset' in 'MSACollapsibleItemModel' work only with sorted regions
    qSort(collapsedRegions);

    if (msaObject == NULL || msaObject->isStateLocked()) {
        if (collapseModeSwitchAction->isChecked()) {
            collapseModeSwitchAction->setChecked(false);
        }
        return;
    }

    ui->setCollapsibleMode(true);

    m->reset(collapsedRegions);

    MaModificationInfo mi;
    msaObject->updateCachedMultipleAlignment(mi);

    updateVScrollBar();
}

void MSAEditorSequenceArea::sl_changeSelectionColor() {
    QColor black(Qt::black);
    selectionColor = (black == selectionColor) ? Qt::darkGray : Qt::black;
    update();
}

QString MSAEditorSequenceArea::exportHighligtning(int startPos, int endPos, int startingIndex, bool keepGaps, bool dots, bool transpose) {
    CHECK(getEditor() != NULL, QString());
    QStringList result;

    MultipleSequenceAlignmentObject* maObj = getEditor()->getMaObject();
    assert(maObj!=NULL);

    const MultipleSequenceAlignment msa = maObj->getMultipleAlignment();

    U2OpStatusImpl os;
    const int refSeq = getEditor()->getMaObject()->getMsa()->getRowIndexByRowId(editor->getReferenceRowId(), os);
    MultipleSequenceAlignmentRow row;
    if (U2MsaRow::INVALID_ROW_ID != refSeq) {
        row = msa->getMsaRow(refSeq);
    }

    QString header;
    header.append("Position\t");
    QString refSeqName = editor->getReferenceRowName();
    header.append(refSeqName);
    header.append("\t");
    foreach(QString name, maObj->getMultipleAlignment()->getRowNames()){
        if(name != refSeqName){
            header.append(name);
            header.append("\t");
        }
    }
    header.remove(header.length()-1,1);
    result.append(header);


    int posInResult = startingIndex;

    for (int pos = startPos-1; pos < endPos; pos++) {
        QString rowStr;
        rowStr.append(QString("%1").arg(posInResult));
        rowStr.append(QString("\t") + QString(msa->charAt(refSeq, pos)) + QString("\t"));
        bool informative = false;
        for (int seq = 0; seq < msa->getNumRows(); seq++) {  //FIXME possible problems when sequences have moved in view
            if (seq == refSeq) continue;
            char c = msa->charAt(seq, pos);

            SAFE_POINT(NULL != row, "MSA row is NULL", "");
            const char refChar = row->charAt(pos);
            if (refChar == '-' && !keepGaps) {
                continue;
            }

            QColor unused;
            bool highlight = false;
            highlightingScheme->setUseDots(useDotsAction->isChecked());
            highlightingScheme->process(refChar, c, unused, highlight, pos, seq);

            if (highlight) {
                rowStr.append(c);
                informative = true;
            } else {
                if (dots) {
                    rowStr.append(".");
                } else {
                    rowStr.append(" ");
                }
            }
            rowStr.append("\t");
        }
        if(informative){
            header.remove(rowStr.length() - 1, 1);
            result.append(rowStr);
        }
        posInResult++;
    }

    if (!transpose){
        QStringList transposedRows = TextUtils::transposeCSVRows(result, "\t");
        return transposedRows.join("\n");
    }

    return result.join("\n");
}

MsaColorScheme * MSAEditorSequenceArea::getCurrentColorScheme() const {
    return colorScheme;
}

MsaHighlightingScheme * MSAEditorSequenceArea::getCurrentHighlightingScheme() const {
    return highlightingScheme;
}

bool MSAEditorSequenceArea::getUseDotsCheckedState() const {
    return useDotsAction->isChecked();
}

void MSAEditorSequenceArea::cancelShiftTracking() {
    shifting = false;
    selecting = false;
    changeTracker.finishTracking();
    editor->getMaObject()->releaseState();
}

ExportHighligtningTask::ExportHighligtningTask(ExportHighligtingDialogController *dialog, MSAEditorSequenceArea *msaese_)
    : Task(tr("Export highlighting"), TaskFlags_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled)
{
    msaese = msaese_;
    startPos = dialog->startPos;
    endPos = dialog->endPos;
    startingIndex = dialog->startingIndex;
    keepGaps = dialog->keepGaps;
    dots = dialog->dots;
    transpose = dialog->transpose;
    url = dialog->url;
}

void ExportHighligtningTask::run(){
    QString exportedData = msaese->exportHighligtning(startPos, endPos, startingIndex, keepGaps, dots, transpose);

    QFile resultFile(url.getURLString());
    CHECK_EXT(resultFile.open(QFile::WriteOnly | QFile::Truncate), url.getURLString(),);
    QTextStream contentWriter(&resultFile);
    contentWriter << exportedData;
}

Task::ReportResult ExportHighligtningTask::report() {
    return ReportResult_Finished;
}

QString ExportHighligtningTask::generateReport() const {
    QString res;
    if(!isCanceled() && !hasError()){
        res += "<b>" + tr("Export highligtning finished successfully") + "</b><br><b>" + tr("Result file:") + "</b> " + url.getURLString();
    }
    return res;
}

}//namespace
