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

#include <U2Core/U2SafePoints.h>

#include "DiamondClassifyLogParser.h"
#include "DiamondClassifyTask.h"
#include "DiamondSupport.h"

namespace U2 {

DiamondClassifyTask::DiamondClassifyTask(const DiamondClassifyTaskSettings &settings)
    : ExternalToolSupportTask(tr("Classify sequences with DIAMOND"), TaskFlags_NR_FOSE_COSC),
      settings(settings)
{
    checkSettings();
    CHECK_OP(stateInfo, );
}

const QString &DiamondClassifyTask::getClassificationUrl() const {
    return settings.classificationUrl;
}

void DiamondClassifyTask::prepare() {
    ExternalToolRunTask *classifyTask = new ExternalToolRunTask(DiamondSupport::TOOL_NAME, getArguments(), new DiamondClassifyLogParser());
    setListenerForTask(classifyTask);
    addSubTask(classifyTask);
}

void DiamondClassifyTask::checkSettings() {
    SAFE_POINT_EXT(!settings.readsUrl.isEmpty(), setError("Reads URL is empty"), );
    SAFE_POINT_EXT(!settings.pairedReads || !settings.readsUrl.isEmpty(), setError("Paired reads URL is empty, but the 'paired reads' option is set"), );
    SAFE_POINT_EXT(!settings.databaseUrl.isEmpty(), setError("DIAMOND database URL is empty"), );
    SAFE_POINT_EXT(!settings.classificationUrl.isEmpty(), setError("DIAMOND classification URL is empty"), );
    SAFE_POINT_EXT(!settings.pairedReads || !settings.pairedClassificationUrl.isEmpty(), setError("URL to paired DIAMOND classification is empty"), );
    SAFE_POINT_EXT(!settings.taxonMapUrl.isEmpty(), setError("Taxon map URL is empty"), );
    SAFE_POINT_EXT(!settings.taxonNodesUrl.isEmpty(), setError("Taxon nodes URL is empty"), );
}

QStringList DiamondClassifyTask::getArguments() const {
    QStringList arguments;
    arguments << "blastx";
    arguments << "-d" << settings.databaseUrl;
    arguments << "-f" << "102";
    arguments << "--taxonmap" << settings.taxonMapUrl;
    arguments << "--taxonnodes" << settings.taxonNodesUrl;
    arguments << "-q" << settings.readsUrl;
    arguments << "-o" << settings.classificationUrl;
    return arguments;
}

}   // namespace U2
