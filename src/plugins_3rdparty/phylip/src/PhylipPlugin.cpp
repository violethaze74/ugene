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

#include "PhylipPlugin.h"

#include <U2Core/AppContext.h>
#include <U2Core/CMDLineRegistry.h>
#include <U2Core/Task.h>
#include <U2Core/TaskStarter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/GObjectTypes.h>

#include <U2Core/GAutoDeleteList.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/DialogUtils.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#include <QtGui/QMenu>
#else
#include <QtWidgets/QDialog>
#include <QtWidgets/QMenu>
#endif

#include "PhylipPluginTests.h"
#include "PhylipCmdlineTask.h"
#include "PhylipTask.h"
#include "NeighborJoinAdapter.h"
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
	PhylipPlugin * plug = new PhylipPlugin();
	return plug;
}

const QString PhylipPlugin::PHYLIP_NEIGHBOUR_JOIN("PHYLIP Neighbor Joining");

PhylipPlugin::PhylipPlugin() 
: Plugin(tr("PHYLIP"), tr("PHYLIP (the PHYLogeny Inference Package) is a package of programs for inferring phylogenies (evolutionary trees)."
         " Original version at: http://evolution.genetics.washington.edu/phylip.html"), false)
{

    PhyTreeGeneratorRegistry* registry = AppContext::getPhyTreeGeneratorRegistry();
    registry->registerPhyTreeGenerator(new NeighborJoinAdapter(), PHYLIP_NEIGHBOUR_JOIN);

    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = PhylipPluginTests::createTestFactories();

    foreach(XMLTestFactory* f, l->qlist) { 
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }

    processCmdlineOptions();
}

PhylipPlugin::~PhylipPlugin() {
    //nothing to do
}

namespace {
    CreatePhyTreeSettings fetchSettings() {
        CreatePhyTreeSettings settings;
        CMDLineRegistry *cmdLineRegistry = AppContext::getCMDLineRegistry();
        if (cmdLineRegistry->hasParameter(PhylipCmdlineTask::MATRIX_ARG)) {
            settings.matrixId = cmdLineRegistry->getParameterValue(PhylipCmdlineTask::MATRIX_ARG);
        }
        if (cmdLineRegistry->hasParameter(PhylipCmdlineTask::GAMMA_ARG)) {
            settings.useGammaDistributionRates = cmdLineRegistry->getParameterValue(PhylipCmdlineTask::GAMMA_ARG).toInt();
        }
        if (cmdLineRegistry->hasParameter(PhylipCmdlineTask::ALPHA_ARG)) {
            settings.alphaFactor = cmdLineRegistry->getParameterValue(PhylipCmdlineTask::ALPHA_ARG).toDouble();
        }
        if (cmdLineRegistry->hasParameter(PhylipCmdlineTask::TT_RATIO_ARG)) {
            settings.ttRatio = cmdLineRegistry->getParameterValue(PhylipCmdlineTask::TT_RATIO_ARG).toDouble();
        }
        if (cmdLineRegistry->hasParameter(PhylipCmdlineTask::BOOTSTRAP_ARG)) {
            settings.bootstrap = cmdLineRegistry->getParameterValue(PhylipCmdlineTask::BOOTSTRAP_ARG).toInt();
        }
        if (cmdLineRegistry->hasParameter(PhylipCmdlineTask::REPLICATES_ARG)) {
            settings.replicates = cmdLineRegistry->getParameterValue(PhylipCmdlineTask::REPLICATES_ARG).toInt();
        }
        if (cmdLineRegistry->hasParameter(PhylipCmdlineTask::SEED_ARG)) {
            settings.seed = cmdLineRegistry->getParameterValue(PhylipCmdlineTask::SEED_ARG).toInt();
        }
        if (cmdLineRegistry->hasParameter(PhylipCmdlineTask::FRACTION_ARG)) {
            settings.fraction = cmdLineRegistry->getParameterValue(PhylipCmdlineTask::FRACTION_ARG).toDouble();
        }
        if (cmdLineRegistry->hasParameter(PhylipCmdlineTask::CONSENSUS_ARG)) {
            settings.consensusID = cmdLineRegistry->getParameterValue(PhylipCmdlineTask::CONSENSUS_ARG);
        }
        return settings;
    }
}

void PhylipPlugin::processCmdlineOptions() {
    CMDLineRegistry *cmdLineRegistry = AppContext::getCMDLineRegistry();
    CHECK(cmdLineRegistry->hasParameter(PhylipCmdlineTask::PHYLIP_CMDLINE), );
    CHECK(cmdLineRegistry->hasParameter(PhylipCmdlineTask::MSA_ARG), );
    CHECK(cmdLineRegistry->hasParameter(PhylipCmdlineTask::TREE_ARG), );

    QString msaUrl = cmdLineRegistry->getParameterValue(PhylipCmdlineTask::MSA_ARG);
    QString treeUrl = cmdLineRegistry->getParameterValue(PhylipCmdlineTask::TREE_ARG);
    CreatePhyTreeSettings settings = fetchSettings();

    Task *t = new PhylipTask(msaUrl, treeUrl, settings);
    connect(AppContext::getPluginSupport(), SIGNAL(si_allStartUpPluginsLoaded()), new TaskStarter(t), SLOT(registerTask()));
}

}//namespace
