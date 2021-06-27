/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2021 UniPro <ugene@unipro.ru>
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

#include "DisassemblyAndCloningPlugin.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QIcon>
#include <QHBoxLayout>
#include <QWebEngineView>

#include <U2Core/AppContext.h>
#include <U2Core/L10n.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/MainWindow.h>
#include <U2Gui/ToolsMenu.h>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin * U2_PLUGIN_INIT_FUNC() {
    return new DisassemblyAndCloningPlugin();
}

DisassemblyAndCloningPlugin::DisassemblyAndCloningPlugin()
    : Plugin(tr("Is silico disassembly and cloning"),
             tr("In silico experiments for gene assembly and molecular cloning")) {
    if (AppContext::getMainWindow() != nullptr) {
        QAction* a = new QAction(QIcon(":/core/images/todo.png"), tr("Gene assembly and molecular cloning..."), this);
        a->setObjectName(ToolsMenu::GENE_ASSEMBLY_AND_MOLECULAR_CLONING);
        connect(a, SIGNAL(triggered()), SLOT(sl_showWindow()));
        ToolsMenu::addAction(ToolsMenu::TOOLS, a);
    }
}

void DisassemblyAndCloningPlugin::sl_showWindow() {

    QDialog dlg(QApplication::activeWindow());
    dlg.setWindowTitle(tr("Gene assembly and molecular cloning"));
    dlg.setWindowIcon(QIcon(":/core/images/todo.png"));
    dlg.resize(1366, 768);


    QWebEngineView* view = new QWebEngineView(&dlg);
    view->load(QUrl("qrc:disassembly_and_cloning/html/index.html"));
    view->show();

    auto l = new QHBoxLayout;
    l->addWidget(view);
    dlg.setLayout(l);

    dlg.exec();
}


}