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

#include <U2Core/MultipleSequenceAlignmentObject.h>
#include <U2Core/MultipleChromatogramAlignmentObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2SafePoints.h>

#include "../ProjectFilterNames.h"

#include "MaSeqNameFilterTask.h"

namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// MaSeqNameFilterTask
//////////////////////////////////////////////////////////////////////////

MaSeqNameFilterTask::MaSeqNameFilterTask(const ProjectTreeControllerModeSettings &settings, const QList<QPointer<Document> > &docs)
    : AbstractProjectFilterTask(settings, ProjectFilterNames::MA_SEQ_NAME_FILTER_NAME, docs)
{

}

bool MaSeqNameFilterTask::filterAcceptsObject(GObject *obj) {
    MultipleAlignmentObject *maObj = qobject_cast<MultipleAlignmentObject *>(obj);
    CHECK(NULL != maObj, false);

    for (int i = 0, n = maObj->getNumRows(); i < n; ++i) {
        if (settings.nameFilterAcceptsString(maObj->getRow(i)->getName())) {
            return true;
        }
    }

    // if this is MCA -> check reference sequence name too.
    MultipleChromatogramAlignmentObject* mcaObj = qobject_cast<MultipleChromatogramAlignmentObject*>(maObj);
    if (mcaObj != NULL) {
        U2SequenceObject* refObj = mcaObj->getReferenceObj();
        if (refObj != NULL && settings.nameFilterAcceptsString(refObj->getSequenceName())) {
            return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////
/// MaSeqNameFilterTaskFactory
//////////////////////////////////////////////////////////////////////////

AbstractProjectFilterTask * MaSeqNameFilterTaskFactory::createNewTask(const ProjectTreeControllerModeSettings &settings,
    const QList<QPointer<Document> > &docs) const
{
    return new MaSeqNameFilterTask(settings, docs);
}


} // namespace U2
