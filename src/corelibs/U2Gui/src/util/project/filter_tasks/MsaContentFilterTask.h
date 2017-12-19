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

#ifndef _U2_MSA_CONTENT_FILTER_TASK_H_
#define _U2_MSA_CONTENT_FILTER_TASK_H_

#include <U2Core/AbstractProjectFilterTask.h>

namespace U2 {

class MultipleAlignmentObject;

//////////////////////////////////////////////////////////////////////////
/// MaContentFilterTask
//////////////////////////////////////////////////////////////////////////

class MaContentFilterTask : public AbstractProjectFilterTask {
public:
    MaContentFilterTask(const ProjectTreeControllerModeSettings &settings, const QList<QPointer<Document> > &docs);

protected:
    bool filterAcceptsObject(GObject *obj);

private:
    bool msaContainsPattern(MultipleAlignmentObject *maObject, const QString &pattern);

    static bool patternFitsMsaAlphabet(MultipleAlignmentObject *maObject, const QString &pattern);
};

//////////////////////////////////////////////////////////////////////////
/// MaContentFilterTaskFactory
//////////////////////////////////////////////////////////////////////////

class U2GUI_EXPORT MaContentFilterTaskFactory : public ProjectFilterTaskFactory {
protected:
    AbstractProjectFilterTask * createNewTask(const ProjectTreeControllerModeSettings &settings, const QList<QPointer<Document> > &docs) const;
};

} // namespace U2

#endif // _U2_OBJ_NAME_FILTER_TASK_H_
