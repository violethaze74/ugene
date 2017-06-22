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

#include <U2Algorithm/BuiltInConsensusAlgorithms.h>
#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>
#include <U2Algorithm/MSAConsensusUtils.h>
#include <U2Algorithm/MsaColorScheme.h>

#include <QApplication>
#include <QClipboard>
#include <QHelpEvent>
#include <QMenu>
#include <QPainter>
#include <QToolTip>

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

// SANGER_TODO: remove relative paths
#include "../McaEditor.h"
#include "../MSAEditor.h"
#include "../MSAEditorConsensusArea.h"
#include "../MSAEditorSequenceArea.h"
#include "../General/MSAGeneralTabFactory.h"

namespace U2 {


#define SETTINGS_ROOT QString("msaeditor/")

MaEditorConsensusAreaSettings::MaEditorConsensusAreaSettings() {
    // SANGER_TODO: currently the ruler cannot be drawn above the text - draw methods should be refactored
    order << MSAEditorConsElement_HISTOGRAM
          << MSAEditorConsElement_CONSENSUS_TEXT
          << MSAEditorConsElement_RULER;
    visibility.insert(MSAEditorConsElement_HISTOGRAM, true);
    visibility.insert(MSAEditorConsElement_CONSENSUS_TEXT, true);
    visibility.insert(MSAEditorConsElement_RULER, true);
    highlightMismatches = false;
}

bool MaEditorConsensusAreaSettings::isVisible(const MaEditorConsElement element) const {
    return visibility.value(element, false);
}

MaEditorConsensusArea::MaEditorConsensusArea(MaEditorWgt *_ui)
    : editor(_ui->getEditor()),
      ui(_ui)
{
    assert(editor->getMaObject());
    completeRedraw = true;
    curPos = -1;
    scribbling = false;
    selecting = false;
    cachedView = new QPixmap();

    QObject *parent=new QObject(this);
    parent->setObjectName("parent");

    connect(ui->getSequenceArea(), SIGNAL(si_startChanged(const QPoint &, const QPoint &)), SLOT(sl_startChanged(const QPoint &, const QPoint &)));
    connect(ui->getSequenceArea(), SIGNAL(si_selectionChanged(const MaEditorSelection &, const MaEditorSelection &)),
        SLOT(sl_selectionChanged(const MaEditorSelection &, const MaEditorSelection &)));
    connect(ui->getEditor(), SIGNAL(si_zoomOperationPerformed(bool)), SLOT(sl_zoomOperationPerformed(bool)));
    connect(ui->getSequenceArea()->getHBar(), SIGNAL(actionTriggered(int)), SLOT(sl_onScrollBarActionTriggered(int)));

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

    setObjectName("consArea");
}

MaEditorConsensusArea::~MaEditorConsensusArea() {
    delete cachedView;
}

QSharedPointer<MSAEditorConsensusCache> MaEditorConsensusArea::getConsensusCache() {
    return consensusCache;
}

void MaEditorConsensusArea::paintFullConsensus(QPixmap &pixmap) {
    pixmap = QPixmap(ui->getSequenceArea()->getXByColumnNum(ui->getEditor()->getAlignmentLen()), getYRange(MSAEditorConsElement_RULER).startPos);
    QPainter p(&pixmap);
    paintFullConsensus(p);
}

void MaEditorConsensusArea::paintFullConsensus(QPainter &p) {
    p.fillRect(QRect(0, 0, ui->getSequenceArea()->getXByColumnNum(ui->getEditor()->getAlignmentLen()), getYRange(MSAEditorConsElement_RULER).startPos), Qt::white);
    drawConsensus(p, 0, ui->getEditor()->getAlignmentLen() - 1, true);
    drawHistogram(p, 0, ui->getEditor()->getAlignmentLen() - 1);
}

void MaEditorConsensusArea::paintConsenusPart(QPixmap &pixmap, const U2Region &region, const QList<qint64> &seqIdx) {
    CHECK(!region.isEmpty(), );
    CHECK(!seqIdx.isEmpty(), );
    CHECK(!ui->getSequenceArea()->isAlignmentEmpty(), );

    CHECK(editor->getColumnWidth() * region.length < 32768, );
    pixmap = QPixmap(editor->getColumnWidth() * region.length, getYRange(MSAEditorConsElement_RULER).startPos);

    QPainter p(&pixmap);
    paintConsenusPart(p, region, seqIdx);
}

void MaEditorConsensusArea::paintConsenusPart(QPainter &p, const U2Region &region, const QList<qint64> &seqIdx) {
    CHECK(!region.isEmpty(), );
    CHECK(!seqIdx.isEmpty(), );
    CHECK(!ui->getSequenceArea()->isAlignmentEmpty(), );

    p.fillRect(QRect(0, 0, editor->getColumnWidth() * region.length, getYRange(MSAEditorConsElement_RULER).startPos), Qt::white);

    //draw consensus
    p.setPen(Qt::black);
    QFont f = ui->getEditor()->getFont();
    f.setWeight(QFont::DemiBold);
    p.setFont(f);

    MSAConsensusAlgorithm *alg = getConsensusAlgorithm();
    SAFE_POINT(alg != NULL, tr("MSA consensus algorothm is NULL"), );
    SAFE_POINT(editor->getMaObject() != NULL, tr("MSA object is NULL"), );
    const MultipleAlignment msa = editor->getMaObject()->getMultipleAlignment();
    for (int pos = 0; pos < region.length; pos++) {
        char c = alg->getConsensusChar(msa, pos + region.startPos, seqIdx.toVector());
        drawConsensusChar(p, pos, 0, c, false, true);
    }

    QColor c("#255060");
    p.setPen(c);

    U2Region yr = getYRange(MSAEditorConsElement_HISTOGRAM);
    yr.startPos++;
    yr.length -= 2; //keep borders

    QBrush brush(c, Qt::Dense4Pattern);
    for (int pos = region.startPos, lastPos = region.endPos() - 1; pos <= lastPos; pos++) {
        U2Region xr = ui->getSequenceArea()->getBaseXRange(pos, region.startPos, true);
        int percent = 0;
        alg->getConsensusCharAndScore(msa, pos, percent, seqIdx.toVector());
        percent = qRound(percent * 100. / seqIdx.size() );
        SAFE_POINT(percent >= 0 && percent <= 100, tr("Percent value is out of [0..100] interval"), );
        int h = qRound(percent * yr.length / 100.0);
        QRect hr(xr.startPos + 1, yr.endPos() - h, xr.length - 2, h);
        p.drawRect(hr);
        p.fillRect(hr, brush);
    }
}

void MaEditorConsensusArea::paintRulerPart(QPixmap &pixmap, const U2Region &region) {
    CHECK( editor->getColumnWidth() * region.length < 32768, );
    CHECK( getYRange(MSAEditorConsElement_RULER).length < 32768, );
    pixmap = QPixmap(editor->getColumnWidth() * region.length, getYRange(MSAEditorConsElement_RULER).length);
    pixmap.fill(Qt::white);
    QPainter p(&pixmap);
    paintRulerPart(p, region);
}

void MaEditorConsensusArea::paintRulerPart(QPainter &p, const U2Region &region) {
    p.fillRect(QRect(0, 0, editor->getColumnWidth() * region.length, getYRange(MSAEditorConsElement_RULER).length), Qt::white);
    p.translate(-ui->getSequenceArea()->getBaseXRange(region.startPos, region.startPos, true).startPos, -getYRange(MSAEditorConsElement_RULER).startPos);
    drawRuler(p, region.startPos, region.endPos(), true);
    // return back to (0, 0)
    p.translate(ui->getSequenceArea()->getBaseXRange(region.startPos, region.startPos, true).startPos, getYRange(MSAEditorConsElement_RULER).startPos);
}

bool MaEditorConsensusArea::event(QEvent* e) {
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

QString MaEditorConsensusArea::createToolTip(QHelpEvent* he) const {
    int  x = he->pos().x();
    int pos = ui->getSequenceArea()->coordToPos(x);
    QString result;
    if (pos >= 0) {
        assert(editor->getMaObject());
        const MultipleAlignment ma = editor->getMaObject()->getMultipleAlignment();
        result = MSAConsensusUtils::getConsensusPercentTip(ma, pos, 0, 4);
    }
    return result;
}

void MaEditorConsensusArea::resizeEvent(QResizeEvent *e) {
    completeRedraw = true;
    QWidget::resizeEvent(e);
}

void MaEditorConsensusArea::paintEvent(QPaintEvent *e) {
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
        drawContent( pCached );
        completeRedraw = false;
    }

    QPainter p(this);
    p.drawPixmap(0, 0, *cachedView);
    drawSelection(p);

    QWidget::paintEvent(e);
}

void MaEditorConsensusArea::drawContent(QPainter& p ) {
    if (drawSettings.isVisible(MSAEditorConsElement_CONSENSUS_TEXT)) {
        drawConsensus(p);
    }
    if (drawSettings.isVisible(MSAEditorConsElement_RULER)) {
        drawRuler(p);
    }
    if (drawSettings.isVisible(MSAEditorConsElement_HISTOGRAM)) {
        drawHistogram(p);
    }
}

void MaEditorConsensusArea::drawSelection(QPainter& p) {
    if (ui->getSequenceArea()->isAlignmentEmpty()) {
        return;
    }

    QFont f = ui->getEditor()->getFont();
    f.setWeight(QFont::DemiBold);
    p.setFont(f);

    MaEditorSelection selection = ui->getSequenceArea()->getSelection();
    int startPos = qMax(selection.x(), ui->getSequenceArea()->getFirstVisibleBase());
    int endPos = qMin(selection.x() + selection.width() - 1,
        ui->getSequenceArea()->getLastVisibleBase(true));
    SAFE_POINT(endPos < ui->getEditor()->getAlignmentLen(), "Incorrect selection width!", );
    for (int pos = startPos; pos <= endPos; ++pos) {
        drawConsensusChar(p, pos, ui->getSequenceArea()->getFirstVisibleBase(), true);
    }
}

void MaEditorConsensusArea::drawConsensus(QPainter& p) {
    if (ui->getSequenceArea()->isAlignmentEmpty()) {
        return;
    }
    int startPos = ui->getSequenceArea()->getFirstVisibleBase();
    int lastPos = ui->getSequenceArea()->getLastVisibleBase(true);
    drawConsensus(p, startPos, lastPos);
}

void MaEditorConsensusArea::drawConsensus(QPainter &p, int startPos, int lastPos, bool useVirtualCoords) {
    if (ui->getSequenceArea()->isAlignmentEmpty()) {
        return;
    }

    //draw consensus
    p.setPen(Qt::black);

    QFont f = ui->getEditor()->getFont();
    f.setWeight(QFont::DemiBold);
    p.setFont(f);

    for (int pos = startPos; pos <= lastPos; pos++) {
        drawConsensusChar(p, pos, startPos, false, useVirtualCoords);
    }
}

void MaEditorConsensusArea::drawConsensusChar(QPainter& p, int pos, int firstVisiblePos, bool selected, bool useVirtualCoords) {
    char c = consensusCache->getConsensusChar(pos);
    drawConsensusChar(p, pos, firstVisiblePos, c, selected, useVirtualCoords);
}

bool isValidConsChar(char ch) {
    return ch != MSAConsensusAlgorithm::INVALID_CONS_CHAR;
}

void MaEditorConsensusArea::drawConsensusChar(QPainter &p, int pos, int firstVisiblePos, char consChar, bool selected, bool useVirtualCoords) {
    U2Region yRange = getYRange(MSAEditorConsElement_CONSENSUS_TEXT);
    U2Region xRange = ui->getSequenceArea()->getBaseXRange(pos, firstVisiblePos, useVirtualCoords);
    QRect cr(xRange.startPos, yRange.startPos, xRange.length + 1, yRange.length);
    if (selected) {
        QColor color(Qt::lightGray);
        color = color.lighter(115);
        p.fillRect(cr, color);
    }
    CHECK(isValidConsChar(consChar), );
    MsaColorScheme* scheme = ui->getSequenceArea()->getCurrentColorScheme();
    if (editor->getResizeMode() == MSAEditor::ResizeMode_FontAndContent) {
        if (highlightConsensusChar(pos)) {
            QColor color = scheme->getColor(0, 0, consChar);
            if (!color.isValid()) {
                color = Qt::red; // default mismatch color (in case of gap-mismatch)
            }
            p.fillRect(cr, color);
        }
        p.drawText(cr, Qt::AlignVCenter | Qt::AlignHCenter, QString(consChar));
    }
}

bool MaEditorConsensusArea::highlightConsensusChar(int /*pos*/) {
    return false;
}

#define RULER_NOTCH_SIZE 3

void MaEditorConsensusArea::drawRuler(QPainter& p, int start, int end, bool drawFull) {
    if (ui->getSequenceArea()->isAlignmentEmpty()) {
        return;
    }

    //draw ruler
    p.setPen(Qt::darkGray);

    int w = (start == -1 && end == -1) ? width() : (end - start)*ui->getEditor()->getColumnWidth();
    int startPos = (start != -1) ? start
                                 : ui->getSequenceArea()->getFirstVisibleBase();
    int lastPos = (end != - 1) ? end - 1
                               : ui->getSequenceArea()->getLastVisibleBase(true);

    QFontMetrics rfm(rulerFont,this);
    U2Region rr = getYRange(MSAEditorConsElement_RULER);
    U2Region rrP = getYRange(MSAEditorConsElement_CONSENSUS_TEXT);
    int dy = rr.startPos - rrP.endPos();
    rr.length += dy;
    rr.startPos -= dy;
    U2Region firstBaseXReg = ui->getSequenceArea()->getBaseXRange(startPos, startPos, drawFull);
    U2Region lastBaseXReg = ui->getSequenceArea()->getBaseXRange(lastPos, startPos, drawFull);
    int firstLastLen = lastBaseXReg.startPos - firstBaseXReg.startPos - 50;
    int firstXCenter = firstBaseXReg.startPos + firstBaseXReg.length / 2;
    QPoint startPoint(firstXCenter, rr.startPos);

    GraphUtils::RulerConfig c;
    c.singleSideNotches = true;
    c.notchSize = RULER_NOTCH_SIZE;
    c.textOffset = (rr.length - rfm.ascent()) /2;
    c.extraAxisLenBefore = startPoint.x();
    c.extraAxisLenAfter = w - (startPoint.x() + firstLastLen);
    c.textBorderStart = -firstBaseXReg.length / 2;
    c.textBorderEnd = -firstBaseXReg.length / 2;

    GraphUtils::drawRuler(p, startPoint, firstLastLen, startPos + 1, lastPos + 1 - 50, rulerFont, c);

    startPoint.setY(rr.endPos());
    c.drawNumbers = false;
    c.textPosition = GraphUtils::LEFT;
    GraphUtils::drawRuler(p, startPoint, firstLastLen, startPos + 1, lastPos + 1 - 50, rulerFont, c);
}

void MaEditorConsensusArea::drawHistogram(QPainter& p) {
    if (ui->getSequenceArea()->isAlignmentEmpty()) {
        return;
    }

    int firstBase = ui->getSequenceArea()->getFirstVisibleBase();
    int lastBase = ui->getSequenceArea()->getLastVisibleBase(true);
    drawHistogram(p, firstBase, lastBase);
}

void MaEditorConsensusArea::drawHistogram(QPainter &p, int firstBase, int lastBase) {
    if (ui->getSequenceArea()->isAlignmentEmpty()) {
        return;
    }

    QColor c("#255060");
    p.setPen(c);
    U2Region yr = getYRange(MSAEditorConsElement_HISTOGRAM);
    yr.startPos++;
    yr.length -= 2; //keep borders

    QBrush brush(c, Qt::Dense4Pattern);
    p.setBrush(brush);
    QVector<QRect> rects;

    for (int pos = firstBase, lastPos = lastBase; pos <= lastPos; pos++) {
        U2Region xr = ui->getSequenceArea()->getBaseXRange(pos, firstBase, true);
        int percent = consensusCache->getConsensusCharPercent(pos);
        assert(percent >= 0 && percent <= 100);
        int h = qRound(percent * yr.length / 100.0);
        QRect hr(xr.startPos + 1, yr.endPos() - h, xr.length - 2, h);
        rects << hr;
    }

    p.drawRects(rects);
}

U2Region MaEditorConsensusArea::getYRange(MaEditorConsElement e) const {
    U2Region res;

    for (QList<MaEditorConsElement>::iterator it = drawSettings.order.begin(); it != drawSettings.order.end(); it++) {
        if (*it == e) {
            res.length = getYRangeLength(e);
            break;
        } else {
            res.startPos += getYRangeLength(*it) * drawSettings.isVisible(*it);
        }
    }
    return res;
}

int MaEditorConsensusArea::getYRangeLength(MaEditorConsElement e) const {
    switch(e) {
        case MSAEditorConsElement_HISTOGRAM:
            return 50;
        case MSAEditorConsElement_CONSENSUS_TEXT:
            return editor->getSequenceRowHeight();
        case MSAEditorConsElement_RULER:
            return  rulerFontHeight + 2 * RULER_NOTCH_SIZE + 4;
    }
    // SANGER_TODO: remove or add FAIL?
    return -1;
}

MSAConsensusAlgorithmFactory* MaEditorConsensusArea::getConsensusAlgorithmFactory() {
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

void MaEditorConsensusArea::updateConsensusAlgorithm() {
    MSAConsensusAlgorithmFactory* newAlgo = getConsensusAlgorithmFactory();
    CHECK(consensusCache != NULL && newAlgo != NULL, );
    ConsensusAlgorithmFlags cacheConsensusFlags = consensusCache->getConsensusAlgorithm()->getFactory()->getFlags();
    ConsensusAlgorithmFlags curFlags = newAlgo->getFlags();
    if ((curFlags & cacheConsensusFlags) != curFlags) {
        consensusCache->setConsensusAlgorithm(newAlgo);
    }
    emit si_consensusAlgorithmChanged(newAlgo->getId());
}

void MaEditorConsensusArea::sl_startChanged(const QPoint& p, const QPoint& prev) {
    if (p.x() == prev.x()) {
        return;
    }
    completeRedraw = true;
    update();
}

void MaEditorConsensusArea::sl_alignmentChanged() {
    updateConsensusAlgorithm();
    completeRedraw = true;
    emit si_mismatchRedrawRequired();
    update();
}

void MaEditorConsensusArea::setupFontAndHeight() {
    rulerFont.setFamily("Arial");
    rulerFont.setPointSize(qMax(8, int(ui->getEditor()->getFont().pointSize() * 0.7)));
    rulerFontHeight = QFontMetrics(rulerFont,this).height();
    setFixedHeight( getYRange(MSAEditorConsElement_RULER).endPos() + 1);
}

void MaEditorConsensusArea::sl_zoomOperationPerformed( bool resizeModeChanged )
{
    if (editor->getResizeMode() == MSAEditor::ResizeMode_OnlyContent && !resizeModeChanged) {
        completeRedraw = true;
        update();
    } else {
        setupFontAndHeight();
    }
}

void MaEditorConsensusArea::sl_selectionChanged(const MaEditorSelection& current, const MaEditorSelection& prev) {
    // TODO: return if only height of selection changes?
    Q_UNUSED(current);
    Q_UNUSED(prev);
    update();
}

void MaEditorConsensusArea::sl_buildStaticMenu(GObjectView* v, QMenu* m) {
    Q_UNUSED(v);
    buildMenu(m);
}

void MaEditorConsensusArea::sl_buildContextMenu(GObjectView* v, QMenu* m) {
    Q_UNUSED(v);
    buildMenu(m);
}

void MaEditorConsensusArea::buildMenu(QMenu* m) {
    QMenu* copyMenu = GUIUtils::findSubMenu(m, MSAE_MENU_COPY);
    SAFE_POINT(copyMenu != NULL, "copyMenu", );
    copyMenu->addAction(copyConsensusAction);
    copyMenu->addAction(copyConsensusWithGapsAction);

//    if (qobject_cast<MSAEditor*>(editor) != NULL) {
//        m->addAction(configureConsensusAction);
//    }

//    CHECK(qobject_cast<McaEditor*>(editor) != NULL, );
//    m->addAction(mismatchController->getNextAction());
//    m->addAction(mismatchController->getPrevAction());
//    m->addSeparator();
}

void MaEditorConsensusArea::sl_copyConsensusSequence() {
    QApplication::clipboard()->setText(consensusCache->getConsensusLine(false));
}

void MaEditorConsensusArea::sl_copyConsensusSequenceWithGaps() {
    QApplication::clipboard()->setText(consensusCache->getConsensusLine(true));
}

void MaEditorConsensusArea::sl_configureConsensusAction() {
    OptionsPanel* optionsPanel = editor->getOptionsPanel();
    SAFE_POINT(NULL != optionsPanel, "Internal error: options panel is NULL"
        " when msageneraltab opening was initiated!",);

    const QString& MSAGeneralTabFactoryId = MSAGeneralTabFactory::getGroupId();
    optionsPanel->openGroupById(MSAGeneralTabFactoryId);
}

void MaEditorConsensusArea::sl_changeConsensusAlgorithm(const QString& algoId) {
    MSAConsensusAlgorithmFactory* algoFactory = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(algoId);
    if (getConsensusAlgorithm()->getFactory() != algoFactory) {
        assert(algoFactory!=NULL);
        setConsensusAlgorithm(algoFactory);
    }
    emit si_consensusAlgorithmChanged(algoId);
}

QString MaEditorConsensusArea::getLastUsedAlgoSettingsKey() const {
    const DNAAlphabet* al = editor->getMaObject()->getAlphabet();
    SAFE_POINT(NULL != al, "Alphabet is NULL", "");
    const char* suffix = al->isAmino() ? "_protein" : al->isNucleic() ? "_nucleic" : "_raw";
    return SETTINGS_ROOT + "_consensus_algorithm_"+ suffix;
}

QString MaEditorConsensusArea::getThresholdSettingsKey(const QString& factoryId) const {
    return getLastUsedAlgoSettingsKey() + "_" + factoryId + "_threshold";
}

U2Region MaEditorConsensusArea::getRullerLineYRange() const {
    return getYRange(MSAEditorConsElement_RULER);
}

void MaEditorConsensusArea::setConsensusAlgorithm(MSAConsensusAlgorithmFactory* algoFactory) {
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

void MaEditorConsensusArea::setConsensusAlgorithmConsensusThreshold(int val) {
    MSAConsensusAlgorithm* algo = getConsensusAlgorithm();
    if (algo->getThreshold() == val) {
        return;
    }
    //store threshold as the last value
    AppContext::getSettings()->setValue(getThresholdSettingsKey(algo->getId()), val);
    algo->setThreshold(val);
}

void MaEditorConsensusArea::sl_onConsensusThresholdChanged(int newValue) {
    Q_UNUSED(newValue);
    completeRedraw = true;
    emit si_mismatchRedrawRequired();
    update();
}

void MaEditorConsensusArea::restoreLastUsedConsensusThreshold() {
    //restore last used threshold for new algorithm type if found
    MSAConsensusAlgorithm* algo = getConsensusAlgorithm();
    int threshold = AppContext::getSettings()->getValue(getThresholdSettingsKey(algo->getId()), algo->getDefaultThreshold()).toInt();
    getConsensusAlgorithm()->setThreshold(threshold);

}

MSAConsensusAlgorithm* MaEditorConsensusArea::getConsensusAlgorithm() const {
    return consensusCache->getConsensusAlgorithm();
}

void MaEditorConsensusArea::sl_changeConsensusThreshold(int val) {
    setConsensusAlgorithmConsensusThreshold(val);
    emit si_consensusThresholdChanged(val);
}

void MaEditorConsensusArea::mousePressEvent(QMouseEvent *e) {
    int x = e->x();
    if (e->buttons() & Qt::LeftButton) {
        selecting = true;
        int lastPos = curPos;
        curPos = ui->getSequenceArea()->getColumnNumByX(x, selecting);
        if (curPos != -1) {
            int height = ui->getSequenceArea()->getNumDisplayedSequences();
            // select current column
            if ((Qt::ShiftModifier == e->modifiers()) && (lastPos != -1)) {
                MaEditorSelection selection(qMin(lastPos, curPos), 0, abs(curPos - lastPos) + 1, height);
                ui->getSequenceArea()->setSelection(selection);
                curPos = lastPos;
            } else {
                MaEditorSelection selection(curPos, 0, 1, height);
                ui->getSequenceArea()->setSelection(selection);
                scribbling = true;
            }
        }
    }
    QWidget::mousePressEvent(e);
}

void MaEditorConsensusArea::mouseMoveEvent(QMouseEvent *e) {
    if ((e->buttons() & Qt::LeftButton) && scribbling) {
        int newPos = ui->getSequenceArea()->getColumnNumByX(e->x(), selecting);
        if ( ui->getSequenceArea()->isPosInRange(newPos)) {
            ui->getSequenceArea()->updateHBarPosition(newPos, true);
        }
        updateSelection(newPos);
    }
    QWidget::mouseMoveEvent(e);
}

void MaEditorConsensusArea::mouseReleaseEvent(QMouseEvent *e) {
    if (ui->getSequenceArea()->isAlignmentEmpty()) {
        QWidget::mouseReleaseEvent(e);
        return;
    }

    if (e->button() == Qt::LeftButton) {
        int newPos = ui->getSequenceArea()->getColumnNumByX(e->x(), selecting);
        updateSelection(newPos);
        scribbling = false;
        selecting = false;
    }

    ui->getSequenceArea()->getHBar()->setupRepeatAction(QAbstractSlider::SliderNoAction);
    QWidget::mouseReleaseEvent(e);
}

void MaEditorConsensusArea::updateSelection(int newPos) {
    CHECK(newPos != curPos, );
    CHECK(newPos != -1, );

    int height = ui->getSequenceArea()->getNumDisplayedSequences();
    int startPos = qMin(curPos,newPos);
    int width = qAbs(newPos - curPos) + 1;
    MaEditorSelection selection(startPos, 0, width, height);
    ui->getSequenceArea()->setSelection(selection);
}

void MaEditorConsensusArea::sl_onScrollBarActionTriggered(int scrollAction) {
    if (scribbling && (scrollAction ==  QAbstractSlider::SliderSingleStepAdd || scrollAction == QAbstractSlider::SliderSingleStepSub)) {
        QPoint coord = mapFromGlobal(QCursor::pos());
        int newPos = ui->getSequenceArea()->getColumnNumByX(coord.x(), selecting);
        updateSelection(newPos);
    }
}

} // namespace
