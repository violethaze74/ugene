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

#include "MSASchemesMenuBuilder.h"

#include <QLabel>
#include <QWidgetAction>
#include <QMenu>

#include <U2Algorithm/MsaColorScheme.h>
#include <U2Algorithm/MsaHighlightingScheme.h>

namespace U2 {

void MsaSchemesMenuBuilder::createAndFillColorSchemeMenuActions(QList<QAction*> &actions, const QList<MsaColorSchemeFactory*> &colorFactories, 
    DNAAlphabetType alphabet, QObject *actionsParent) {
    if (alphabet == DNAAlphabet_RAW) {
        QList<MsaColorSchemeFactory *> rawColorSchemesFactories;
        QList<MsaColorSchemeFactory *> aminoColorSchemesFactories;
        QList<MsaColorSchemeFactory *> nucleotideColorSchemesFactories;
        foreach(MsaColorSchemeFactory *factory, colorFactories) {
            if (factory->isAlphabetFit(DNAAlphabet_RAW)) {
                rawColorSchemesFactories.append(factory);
            } else if (factory->isAlphabetFit(DNAAlphabet_AMINO)) {
                aminoColorSchemesFactories.append(factory);
            } else if (factory->isAlphabetFit(DNAAlphabet_NUCL)) {
                nucleotideColorSchemesFactories.append(factory);
            }
        }
        if (!rawColorSchemesFactories.isEmpty()) {
            actions.append(new QAction(SECTION_TOKEN + tr("RAW alphabet"), actionsParent));
            fillColorSchemeMenuActions(actions, rawColorSchemesFactories, actionsParent);
        }
        if (!aminoColorSchemesFactories.isEmpty()) {
            actions.append(new QAction(SECTION_TOKEN + tr("Amino alphabet"), actionsParent));
            fillColorSchemeMenuActions(actions, aminoColorSchemesFactories, actionsParent);
        }
        if (!nucleotideColorSchemesFactories.isEmpty()) {
            actions.append(new QAction(SECTION_TOKEN + tr("Nucleotide alphabet"), actionsParent));
            fillColorSchemeMenuActions(actions, nucleotideColorSchemesFactories, actionsParent);
        }
    } else {
        fillColorSchemeMenuActions(actions, colorFactories, actionsParent);
    }
}

void MsaSchemesMenuBuilder::createAndFillHighlightingMenuActions(QList<QAction*> &actions, QList<MsaHighlightingSchemeFactory *> highlightingSchemesFactories,
    DNAAlphabetType alphabet, QObject *actionsParent) {
    if (alphabet == DNAAlphabet_RAW) {
        QList<MsaHighlightingSchemeFactory *> commonHighlightSchemesFactories;
        QList<MsaHighlightingSchemeFactory *> rawHighlightSchemesFactories;
        QList<MsaHighlightingSchemeFactory *> aminoHighlightSchemesFactories;
        QList<MsaHighlightingSchemeFactory *> nucleotideHighlightSchemesFactories;
        foreach(MsaHighlightingSchemeFactory *factory, highlightingSchemesFactories) {
            if (factory->isAlphabetFit(DNAAlphabet_AMINO) &&
                factory->isAlphabetFit(DNAAlphabet_NUCL) &&
                factory->isAlphabetFit(DNAAlphabet_RAW)) {
                commonHighlightSchemesFactories.append(factory);
            } else if (factory->isAlphabetFit(DNAAlphabet_RAW)) {
                rawHighlightSchemesFactories.append(factory);
            } else if (factory->isAlphabetFit(DNAAlphabet_AMINO)) {
                aminoHighlightSchemesFactories.append(factory);
            } else if (factory->isAlphabetFit(DNAAlphabet_NUCL)) {
                nucleotideHighlightSchemesFactories.append(factory);
            }
        }

        fillHighlightingSchemeMenuActions(actions, commonHighlightSchemesFactories, actionsParent);

        if (!rawHighlightSchemesFactories.isEmpty()) {
            actions.append(new QAction(SECTION_TOKEN + tr("RAW alphabet"), actionsParent));
            fillHighlightingSchemeMenuActions(actions, rawHighlightSchemesFactories, actionsParent);
        }

        if (!aminoHighlightSchemesFactories.isEmpty()) {
            actions.append(new QAction(SECTION_TOKEN + tr("Amino acid alphabet"), actionsParent));
            fillHighlightingSchemeMenuActions(actions, aminoHighlightSchemesFactories, actionsParent);
        }

        if (!nucleotideHighlightSchemesFactories.isEmpty()) {
            actions.append(new QAction(SECTION_TOKEN + tr("Nucleotide alphabet"), actionsParent));
            fillHighlightingSchemeMenuActions(actions, nucleotideHighlightSchemesFactories, actionsParent);
        }
    } else {
        fillHighlightingSchemeMenuActions(actions, highlightingSchemesFactories, actionsParent);
    }
}

void MsaSchemesMenuBuilder::addActionOrTextSeparatorToMenu(QAction* a, QMenu* colorsSchemeMenu) {
    if (a->text().contains(SECTION_TOKEN)) {
        QString text = a->text().replace(SECTION_TOKEN, QString());
        QLabel *pLabel = new QLabel(text);
        pLabel->setAlignment(Qt::AlignCenter);
        pLabel->setStyleSheet("font: bold;");
        QWidgetAction *separator = new QWidgetAction(a);
        separator->setDefaultWidget(pLabel);
        colorsSchemeMenu->addAction(separator);
    } else {
        colorsSchemeMenu->addAction(a);
    }
}

void MsaSchemesMenuBuilder::fillColorSchemeMenuActions(QList<QAction*> &actions, QList<MsaColorSchemeFactory*> colorFactories, QObject *actionsParent) {
    foreach(MsaColorSchemeFactory *factory, colorFactories) {
        QString name = factory->getName();
        QAction *action = new QAction(name, actionsParent);
        action->setObjectName(name);
        action->setCheckable(true);
        action->setData(factory->getId());
        connect(action, SIGNAL(triggered()), actionsParent, SLOT(sl_changeColorScheme()));
        actions.append(action);
    }
}

void MsaSchemesMenuBuilder::fillHighlightingSchemeMenuActions(QList<QAction*> &actions, const QList<MsaHighlightingSchemeFactory*> &highlightingSchemeFactories, QObject *actionsParent) {
    foreach(MsaHighlightingSchemeFactory *factory, highlightingSchemeFactories) {
        QString name = factory->getName();
        QAction *action = new QAction(name, actionsParent);
        action->setObjectName(name);
        action->setCheckable(true);
        action->setData(factory->getId());
        connect(action, SIGNAL(triggered()), actionsParent, SLOT(sl_changeHighlightScheme()));
        actions.append(action);
    }
}

}
