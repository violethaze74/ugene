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

#include <QApplication>
#include <QClipboard>
#include <QHelpEvent>
#include <QMenu>
#include <QPainter>
#include <QToolTip>

#include <U2Algorithm/BuiltInConsensusAlgorithms.h>
#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>
#include <U2Algorithm/MSAConsensusUtils.h>
#include <U2Algorithm/MsaColorScheme.h>

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/Settings.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/GraphUtils.h>
#include <U2Gui/OPWidgetFactory.h>
#include <U2Gui/OPWidgetFactoryRegistry.h>
#include <U2Gui/OptionsPanel.h>

#include "McaEditor.h"
#include "MSAEditor.h"
#include "MSAEditorConsensusArea.h"
#include "MSAEditorSequenceArea.h"
#include "General/MSAGeneralTabFactory.h"
#include "helpers/BaseWidthController.h"
#include "helpers/ScrollController.h"
#include "view_rendering/MaConsensusAreaRenderer.h"

namespace U2 {

#define SETTINGS_ROOT QString("msaeditor/")

MSAEditorConsensusArea::MSAEditorConsensusArea(MaEditorWgt *_ui)
    : editor(_ui->getEditor()),
      ui(_ui),
      renderer(new MaConsensusAreaRenderer(this))
{
    assert(editor->getMaObject());
    completeRedraw = true;
    curPos = -1;
    scribbling = false;
    selecting = false;
    cachedView = new QPixmap();

    QObject *parent=new QObject(this);
    parent->setObjectName("parent");

    connect(ui->getScrollController(), SIGNAL(si_visibleAreaChanged()), SLOT(sl_visibleAreaChanged()));
    connect(ui->getScrollController(), SIGNAL(si_visibleAreaChanged()), SLOT(sl_completeRedraw()));
    connect(ui->getSequenceArea(), SIGNAL(si_selectionChanged(const MaEditorSelection &, const MaEditorSelection &)),
        SLOT(sl_selectionChanged(const MaEditorSelection &, const MaEditorSelection &)));
    connect(ui->getEditor(), SIGNAL(si_zoomOperationPerformed(bool)), SLOT(sl_zoomOperationPerformed(bool)));
    connect(ui, SIGNAL(si_completeRedraw()), SLOT(sl_completeRedraw()));

    connect(editor->getMaObject(), SIGNAL(si_alignmentChanged(const MultipleAlignment &, const MaModificationInfo &)),
                                    SLOT(sl_alignmentChanged()));

    connect(editor, SIGNAL(si_buildStaticMenu(GObjectView *, QMenu *)), SLOT(sl_buildStaticMenu(GObjectView *, QMenu *)));
    connect(editor, SIGNAL(si_buildStaticToolbar(GObjectView*,QToolBar*)), SLOT(sl_buildStaticToolbar(GObjectView*,QToolBar*)));
    connect(editor, SIGNAL(si_buildPopupMenu(GObjectView * , QMenu *)), SLOT(sl_buildContextMenu(GObjectView *, QMenu *)));

    copyConsensusAction = new QAction(tr("Copy consensus"), this);
    copyConsensusAction->setObjectName("Copy consensus");
    connect(copyConsensusAction, SIGNAL(triggered()), SLOT(sl_copyConsensusSequence()));

    copyConsensusWithGapsAction = new QAction(tr("Copy consensus with gaps"), this);
    copyConsensusWithGapsAction->setObjectName("Copy consensus with gaps");
    connect(copyConsensusWithGapsAction, SIGNAL(triggered()), SLOT(sl_copyConsensusSequenceWithGaps()));

    configureConsensusAction = new QAction(tr("Consensus mode..."), this);
    configureConsensusAction->setObjectName("Consensus mode");
    connect(configureConsensusAction, SIGNAL(triggered()), SLOT(sl_configureConsensusAction()));

    setupFontAndHeight();

    connect(editor, SIGNAL(si_fontChanged(QFont)), SLOT(setupFontAndHeight()));

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    MSAConsensusAlgorithmFactory* algo = getConsensusAlgorithmFactory();
    consensusCache = QSharedPointer<MSAEditorConsensusCache>(new MSAEditorConsensusCache(NULL, editor->getMaObject(), algo));
    connect(consensusCache->getConsensusAlgorithm(), SIGNAL(si_thresholdChanged(int)), SLOT(sl_onConsensusThresholdChanged(int)));
    addAction(ui->getCopySelectionAction());
    addAction(ui->getPasteAction());
    restoreLastUsedConsensusThreshold();

    mismatchController = new MaConsensusMismatchController(this, consensusCache, editor);

    setObjectName("consArea");
}

MSAEditorConsensusArea::~MSAEditorConsensusArea() {
    delete cachedView;
}

QSharedPointer<MSAEditorConsensusCache> MSAEditorConsensusArea::getConsensusCache() {
    return consensusCache;
}

MaEditorWgt *MSAEditorConsensusArea::getEditorWgt() const {
    return ui;
}

QSize MSAEditorConsensusArea::getCanvasSize(const U2Region &region, const MaEditorConsElements &elements) const {
    return QSize(ui->getBaseWidthController()->getBasesWidth(region), renderer->getHeight(elements));
}

bool MSAEditorConsensusArea::event(QEvent* e) {
    switch (e->type()) {
        case QEvent::ToolTip : {
            QHelpEvent* he = static_cast<QHelpEvent *>(e);
            QString tip = createToolTip(he);
            if (!tip.isEmpty()) {
                QToolTip::showText(he->globalPos(), tip);
            }
            return true;
        }
        case QEvent::FocusIn :
            ui->getSequenceArea()->setFocus(static_cast<QFocusEvent *>(e)->reason());
            break;
        case QEvent::Wheel :
            ui->getSequenceArea()->setFocus(Qt::MouseFocusReason);
            break;
        default:
            ; // skip other events
    }

    return QWidget::event(e);
}

QString MSAEditorConsensusArea::createToolTip(QHelpEvent* he) const {
    const int x = he->pos().x();
    const int column = ui->getBaseWidthController()->globalXPositionToColumn(x);
    QString result;
    if (0 <= column && column <= editor->getAlignmentLen()) {
        assert(editor->getMaObject());
        const MultipleAlignment ma = editor->getMaObject()->getMultipleAlignment();
        result = MSAConsensusUtils::getConsensusPercentTip(ma, column, 0, 4);
    }
    return result;
}

void MSAEditorConsensusArea::resizeEvent(QResizeEvent *e) {
    completeRedraw = true;
    QWidget::resizeEvent(e);
}

void MSAEditorConsensusArea::paintEvent(QPaintEvent *e) {
    QSize s = size() * devicePixelRatio();
    QSize sas = ui->getSequenceArea()->size() * devicePixelRatio();

    if (sas.width() != s.width()) { //this can happen due to the manual layouting performed by MSAEditor -> just wait for the next resize+paint
        return;
    }

    assert(s.width() == sas.width());

    if (cachedView->size() != s) {
        delete cachedView;
        cachedView = new QPixmap(s);
        cachedView->setDevicePixelRatio(devicePixelRatio());
        completeRedraw = true;
    }

    if (completeRedraw) {
        QPainter pCached(cachedView);
        pCached.fillRect(cachedView->rect(), Qt::white);
        drawContent(pCached);
        completeRedraw = false;
    }

    QPainter painter(this);
    painter.drawPixmap(0, 0, *cachedView);

    QWidget::paintEvent(e);
}

void MSAEditorConsensusArea::drawContent(QPainter& painter) {
    renderer->drawContent(painter);
}

void MSAEditorConsensusArea::drawContent(QPainter &painter,
                                         const QList<int> &seqIdx,
                                         const U2Region &region,
                                         const MaEditorConsensusAreaSettings &consensusSettings) {
    const ConsensusRenderData consensusRenderData = renderer->getConsensusRenderData(seqIdx, region);
    const ConsensusRenderSettings renderSettings = renderer->getRenderSettigns(region, consensusSettings);
    renderer->drawContent(painter, consensusRenderData, consensusSettings, renderSettings);
}

MSAConsensusAlgorithmFactory* MSAEditorConsensusArea::getConsensusAlgorithmFactory() {
    MSAConsensusAlgorithmRegistry* reg = AppContext::getMSAConsensusAlgorithmRegistry();
    SAFE_POINT(NULL != reg, "Consensus algorithm registry is NULL.", NULL);
    QString lastUsedAlgoKey = getLastUsedAlgoSettingsKey();
    QString lastUsedAlgo = AppContext::getSettings()->getValue(lastUsedAlgoKey).toString();
    MSAConsensusAlgorithmFactory* algo = reg->getAlgorithmFactory(lastUsedAlgo);

    const DNAAlphabet* al = editor->getMaObject()->getAlphabet();
    ConsensusAlgorithmFlags alphaFlags = MSAConsensusAlgorithmFactory::getAphabetFlags(al);
    if (algo == NULL || (algo->getFlags() & alphaFlags) != alphaFlags) {
        algo = reg->getAlgorithmFactory(BuiltInConsensusAlgorithms::DEFAULT_ALGO);
        if ((algo->getFlags() & alphaFlags) != alphaFlags) {
            QList<MSAConsensusAlgorithmFactory*> algorithms = reg->getAlgorithmFactories(MSAConsensusAlgorithmFactory::getAphabetFlags(al));
            SAFE_POINT(algorithms.count() > 0, "There are no consensus algorithms for the current alphabet.", NULL);
            algo = algorithms.first();
        }
        AppContext::getSettings()->setValue(lastUsedAlgoKey, algo->getId());
    }
    return algo;
}

void MSAEditorConsensusArea::updateConsensusAlgorithm() {
    MSAConsensusAlgorithmFactory* newAlgo = getConsensusAlgorithmFactory();
    CHECK(consensusCache != NULL && newAlgo != NULL, );
    ConsensusAlgorithmFlags cacheConsensusFlags = consensusCache->getConsensusAlgorithm()->getFactory()->getFlags();
    ConsensusAlgorithmFlags curFlags = newAlgo->getFlags();
    if ((curFlags & cacheConsensusFlags) != curFlags) {
        consensusCache->setConsensusAlgorithm(newAlgo);
    }
    emit si_consensusAlgorithmChanged(newAlgo->getId());
}

void MSAEditorConsensusArea::sl_alignmentChanged() {
    updateConsensusAlgorithm();
    completeRedraw = true;
    emit si_mismatchRedrawRequired();
    update();
}

void MSAEditorConsensusArea::setupFontAndHeight() {
    consensusSettings.font = ui->getEditor()->getFont();
    consensusSettings.setRulerFont(ui->getEditor()->getFont());
    setFixedHeight(renderer->getHeight());
}

void MSAEditorConsensusArea::sl_zoomOperationPerformed(bool resizeModeChanged) {
    if (!(editor->getResizeMode() == MSAEditor::ResizeMode_OnlyContent && !resizeModeChanged)) {
        setupFontAndHeight();
    }
    sl_completeRedraw();
}

void MSAEditorConsensusArea::sl_completeRedraw() {
    completeRedraw = true;
    update();
}

void MSAEditorConsensusArea::sl_selectionChanged(const MaEditorSelection& current, const MaEditorSelection& prev) {
    if (current.getXRegion() != prev.getXRegion()) {
        sl_completeRedraw();
    }
}

void MSAEditorConsensusArea::sl_buildStaticMenu(GObjectView* v, QMenu* m) {
    Q_UNUSED(v);
    buildMenu(m);
}

void MSAEditorConsensusArea::sl_buildStaticToolbar(GObjectView *, QToolBar *t) {
    CHECK(qobject_cast<McaEditor*>(editor) != NULL, );

    t->addAction(mismatchController->getPrevAction());
    t->addAction(mismatchController->getNextAction());
}

void MSAEditorConsensusArea::sl_buildContextMenu(GObjectView* v, QMenu* m) {
    Q_UNUSED(v);
    buildMenu(m);
}

void MSAEditorConsensusArea::buildMenu(QMenu* m) {
    QMenu* copyMenu = GUIUtils::findSubMenu(m, MSAE_MENU_COPY);
    SAFE_POINT(copyMenu != NULL, "copyMenu", );
    copyMenu->addAction(copyConsensusAction);
    copyMenu->addAction(copyConsensusWithGapsAction);

    if (qobject_cast<MSAEditor*>(editor) != NULL) {
        m->addAction(configureConsensusAction);
    }

    CHECK(qobject_cast<McaEditor*>(editor) != NULL, );
    m->addAction(mismatchController->getNextAction());
    m->addAction(mismatchController->getPrevAction());
    m->addSeparator();
}

void MSAEditorConsensusArea::sl_copyConsensusSequence() {
    QApplication::clipboard()->setText(consensusCache->getConsensusLine(false));
}

void MSAEditorConsensusArea::sl_copyConsensusSequenceWithGaps() {
    QApplication::clipboard()->setText(consensusCache->getConsensusLine(true));
}

void MSAEditorConsensusArea::sl_configureConsensusAction() {
    OptionsPanel* optionsPanel = editor->getOptionsPanel();
    SAFE_POINT(NULL != optionsPanel, "Internal error: options panel is NULL"
        " when msageneraltab opening was initiated!",);

    const QString& MSAGeneralTabFactoryId = MSAGeneralTabFactory::getGroupId();
    optionsPanel->openGroupById(MSAGeneralTabFactoryId);
}

void MSAEditorConsensusArea::sl_changeConsensusAlgorithm(const QString& algoId) {
    MSAConsensusAlgorithmFactory* algoFactory = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(algoId);
    if (getConsensusAlgorithm()->getFactory() != algoFactory) {
        assert(algoFactory!=NULL);
        setConsensusAlgorithm(algoFactory);
    }
    emit si_consensusAlgorithmChanged(algoId);
}

QString MSAEditorConsensusArea::getLastUsedAlgoSettingsKey() const {
    const DNAAlphabet* al = editor->getMaObject()->getAlphabet();
    SAFE_POINT(NULL != al, "Alphabet is NULL", "");
    const char* suffix = al->isAmino() ? "_protein" : al->isNucleic() ? "_nucleic" : "_raw";
    return SETTINGS_ROOT + "_consensus_algorithm_"+ suffix;
}

QString MSAEditorConsensusArea::getThresholdSettingsKey(const QString& factoryId) const {
    return getLastUsedAlgoSettingsKey() + "_" + factoryId + "_threshold";
}

U2Region MSAEditorConsensusArea::getRulerLineYRange() const {
    return renderer->getYRange(MSAEditorConsElement_RULER);
}

void MSAEditorConsensusArea::setConsensusAlgorithm(MSAConsensusAlgorithmFactory* algoFactory) {
    MSAConsensusAlgorithm* oldAlgo = getConsensusAlgorithm();
    if (oldAlgo!=NULL && algoFactory == oldAlgo->getFactory()) {
        return;
    }

    //store threshold for the active algo
    if (oldAlgo!=NULL && oldAlgo->supportsThreshold()) {
        AppContext::getSettings()->setValue(getThresholdSettingsKey(oldAlgo->getId()), oldAlgo->getThreshold());
    }

    //store current algorithm selection
    AppContext::getSettings()->setValue(getLastUsedAlgoSettingsKey(), algoFactory->getId());

    consensusCache->setConsensusAlgorithm(algoFactory);
    connect(consensusCache->getConsensusAlgorithm(), SIGNAL(si_thresholdChanged(int)), SLOT(sl_onConsensusThresholdChanged(int)));
    restoreLastUsedConsensusThreshold();
    completeRedraw = true;
    emit si_mismatchRedrawRequired();
    update();
}

void MSAEditorConsensusArea::setConsensusAlgorithmConsensusThreshold(int val) {
    MSAConsensusAlgorithm* algo = getConsensusAlgorithm();
    if (algo->getThreshold() == val) {
        return;
    }
    //store threshold as the last value
    AppContext::getSettings()->setValue(getThresholdSettingsKey(algo->getId()), val);
    algo->setThreshold(val);
}

const MaEditorConsensusAreaSettings &MSAEditorConsensusArea::getDrawSettings() const {
    return consensusSettings;
}

void MSAEditorConsensusArea::setDrawSettings(const MaEditorConsensusAreaSettings& settings) {
    consensusSettings = settings;
    setFixedHeight(renderer->getHeight());
}

void MSAEditorConsensusArea::sl_onConsensusThresholdChanged(int newValue) {
    Q_UNUSED(newValue);
    completeRedraw = true;
    emit si_mismatchRedrawRequired();
    update();
}

void MSAEditorConsensusArea::restoreLastUsedConsensusThreshold() {
    //restore last used threshold for new algorithm type if found
    MSAConsensusAlgorithm* algo = getConsensusAlgorithm();
    int threshold = AppContext::getSettings()->getValue(getThresholdSettingsKey(algo->getId()), algo->getDefaultThreshold()).toInt();
    getConsensusAlgorithm()->setThreshold(threshold);
}

MSAConsensusAlgorithm* MSAEditorConsensusArea::getConsensusAlgorithm() const {
    return consensusCache->getConsensusAlgorithm();
}

void MSAEditorConsensusArea::sl_changeConsensusThreshold(int val) {
    setConsensusAlgorithmConsensusThreshold(val);
    emit si_consensusThresholdChanged(val);
}

void MSAEditorConsensusArea::sl_visibleAreaChanged() {
    if (scribbling && selecting) {
        const QPoint screenPoint = mapFromGlobal(QCursor::pos());
        const int newPos = ui->getBaseWidthController()->screenXPositionToBase(screenPoint.x());
        updateSelection(newPos);
    }
}

void MSAEditorConsensusArea::mousePressEvent(QMouseEvent *e) {
    if (e->buttons() & Qt::LeftButton) {
        selecting = true;
        int lastPos = curPos;
        curPos = ui->getBaseWidthController()->screenXPositionToBase(e->x());
        if (curPos != -1) {
            const int selectionHeight = ui->getSequenceArea()->getNumDisplayableSequences();
            // select current column
            if ((Qt::ShiftModifier == e->modifiers()) && (lastPos != -1)) {
                MaEditorSelection selection(qMin(lastPos, curPos), 0, abs(curPos - lastPos) + 1, selectionHeight);
                ui->getSequenceArea()->setSelection(selection);
                curPos = lastPos;
            } else {
                MaEditorSelection selection(curPos, 0, 1, selectionHeight);
                ui->getSequenceArea()->setSelection(selection);
                ui->getScrollController()->scrollToBase(curPos, width());
                scribbling = true;
            }
        }
    }
    QWidget::mousePressEvent(e);
}

void MSAEditorConsensusArea::mouseMoveEvent(QMouseEvent *event) {
    if ((event->buttons() & Qt::LeftButton) && scribbling && selecting) {
        const int newPos = ui->getBaseWidthController()->screenXPositionToBase(event->x());
        updateSelection(newPos);
    }
    QWidget::mouseMoveEvent(event);
}

void MSAEditorConsensusArea::mouseReleaseEvent(QMouseEvent *event) {
    if (ui->getSequenceArea()->isAlignmentEmpty()) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    if (event->button() == Qt::LeftButton && selecting) {
        const int newPos = ui->getBaseWidthController()->screenXPositionToBase(event->x());
        updateSelection(newPos);
        scribbling = false;
        selecting = false;
    }

    ui->getScrollController()->stopSmoothScrolling();
    QWidget::mouseReleaseEvent(event);
}

void MSAEditorConsensusArea::updateSelection(int newPos) {
    CHECK(newPos != curPos, );
    CHECK(newPos != -1, );

    int selectionHeight = ui->getSequenceArea()->getNumDisplayableSequences();
    int startPos = qMin(curPos, newPos);
    int selectionWidth = qAbs(newPos - curPos) + 1;
    MaEditorSelection selection(startPos, 0, selectionWidth, selectionHeight);
    ui->getSequenceArea()->setSelection(selection);
    ui->getScrollController()->scrollToBase(newPos, width());
}

} // namespace U2
