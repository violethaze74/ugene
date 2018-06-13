/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2018 UniPro <ugene@unipro.ru>
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

#include "KrakenBuildPrompter.h"
#include "KrakenBuildWorkerFactory.h"
#include "MinimizerLengthValidator.h"

namespace U2 {
namespace Workflow {

bool MinimizerLengthValidator::validate(const Actor *actor, ProblemList &problemList, const QMap<QString, QString> &) const {
    const int minimizerLength = actor->getParameter(LocalWorkflow::KrakenBuildWorkerFactory::MINIMIZER_LENGTH_ATTR_ID)->getAttributeValueWithoutScript<int>();
    const int kMerLength = actor->getParameter(LocalWorkflow::KrakenBuildWorkerFactory::K_MER_LENGTH_ATTR_ID)->getAttributeValueWithoutScript<int>();
    if (minimizerLength >= kMerLength) {
        problemList << Problem(LocalWorkflow::KrakenBuildPrompter::tr("Minimizer length has to be less than K-mer length"), actor->getId());
        return false;
    }

    return true;
}

}   // namespace Workflow
}   // namespace U2
