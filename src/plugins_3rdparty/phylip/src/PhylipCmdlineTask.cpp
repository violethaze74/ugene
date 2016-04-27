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
#include <U2Core/CmdlineInOutTaskRunner.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/PhyTreeObject.h>
#include <U2Core/U2DbiRegistry.h>

#include "PhylipCmdlineTask.h"

namespace U2 {

const QString PhylipCmdlineTask::PHYLIP_CMDLINE = "phylip";
const QString PhylipCmdlineTask::MSA_ARG = "in";
const QString PhylipCmdlineTask::TREE_ARG = "out";
const QString PhylipCmdlineTask::MATRIX_ARG = "matrix";
const QString PhylipCmdlineTask::GAMMA_ARG = "gamma";
const QString PhylipCmdlineTask::ALPHA_ARG = "alpha-factor";
const QString PhylipCmdlineTask::TT_RATIO_ARG = "tt-ratio";
const QString PhylipCmdlineTask::BOOTSTRAP_ARG = "bootstrap";
const QString PhylipCmdlineTask::REPLICATES_ARG = "replicates";
const QString PhylipCmdlineTask::SEED_ARG = "seed";
const QString PhylipCmdlineTask::FRACTION_ARG = "fraction";
const QString PhylipCmdlineTask::CONSENSUS_ARG = "consensus";

PhylipCmdlineTask::PhylipCmdlineTask(const MAlignment &msa, const CreatePhyTreeSettings &settings)
: PhyTreeGeneratorTask(msa, settings), cmdlineTask(NULL), msaObject(NULL)
{
    setTaskName(tr("PHYLIP command line wrapper task"));
    tpm = Progress_SubTasksBased;
}

void PhylipCmdlineTask::prepare() {
    createCmdlineTask();
    CHECK_OP(stateInfo, );
    addSubTask(cmdlineTask);
}

Task::ReportResult PhylipCmdlineTask::report() {
    Document *doc = cmdlineTask->getDocument();
    CHECK(NULL != doc, ReportResult_Finished);
    QList<GObject*> objects = doc->findGObjectByType(GObjectTypes::PHYLOGENETIC_TREE);
    if (objects.isEmpty()) {
        setError(tr("No tree objects found"));
        return ReportResult_Finished;
    }
    PhyTreeObject *treeObject = qobject_cast<PhyTreeObject*>(objects.first());
    result = treeObject->getTree();
    return ReportResult_Finished;
}

void PhylipCmdlineTask::createCmdlineTask() {
    CmdlineInOutTaskConfig config;
    U2DbiRef dbiRef = AppContext::getDbiRegistry()->getSessionTmpDbiRef(stateInfo);
    CHECK_OP(stateInfo, );
    msaObject = MAlignmentImporter::createAlignment(dbiRef, const_cast<MAlignment&>(inputMA), stateInfo);
    CHECK_OP(stateInfo, );
    config.inputObjects << msaObject;
    config.inputFormat = BaseDocumentFormats::CLUSTAL_ALN;
    config.outputFormat = BaseDocumentFormats::NEWICK;
    config.command = "--" + PHYLIP_CMDLINE;
    QString argString = "--%1=\"%2\"";
    config.arguments << argString.arg(MATRIX_ARG).arg(settings.matrixId);
    config.arguments << argString.arg(GAMMA_ARG).arg(settings.useGammaDistributionRates);
    config.arguments << argString.arg(ALPHA_ARG).arg(settings.alphaFactor);
    config.arguments << argString.arg(TT_RATIO_ARG).arg(settings.ttRatio);
    config.arguments << argString.arg(BOOTSTRAP_ARG).arg(settings.bootstrap);
    config.arguments << argString.arg(REPLICATES_ARG).arg(settings.replicates);
    config.arguments << argString.arg(SEED_ARG).arg(settings.seed);
    config.arguments << argString.arg(FRACTION_ARG).arg(settings.fraction);
    config.arguments << argString.arg(CONSENSUS_ARG).arg(settings.consensusID);

    cmdlineTask = new CmdlineInOutTaskRunner(config);
}

} // U2
