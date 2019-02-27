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

#include <QDir>
#include <QSettings>

#include <U2Core/U2SafePoints.h>

#include <U2Lang/WorkflowSettings.h>

#include "Dashboard.h"
#include "ScanDashboardsDirTask.h"

namespace U2 {

ScanDashboardsDirTask::ScanDashboardsDirTask()
    : Task(tr("Scan dashboards folder"), TaskFlag_None)
{

}

const QStringList &ScanDashboardsDirTask::getOpenedDashboards() const {
    return openedDashboards;
}

const QList<DashboardInfo> &ScanDashboardsDirTask::getResult() const {
    return result;
}

void ScanDashboardsDirTask::run() {
    QDir outDir(WorkflowSettings::getWorkflowOutputDirectory());
    CHECK(outDir.exists(), );

    QFileInfoList dirs = outDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    foreach (const QFileInfo &info, dirs) {
        QString dirPath = info.absoluteFilePath() + "/";
        DashboardInfo dbi(dirPath);
        if (isDashboardDir(dirPath, dbi)) {
            result << dbi;
            if (dbi.opened) {
                openedDashboards << dirPath;
            }
        }
    }
}

bool ScanDashboardsDirTask::isDashboardDir(const QString &dirPath, DashboardInfo &info) {
    QDir dir(dirPath + Dashboard::REPORT_SUB_DIR);
    CHECK(dir.exists(), false);
    CHECK(dir.exists(Dashboard::DB_FILE_NAME), false);
    CHECK(dir.exists(Dashboard::SETTINGS_FILE_NAME), false);

    QSettings settings(dirPath + Dashboard::REPORT_SUB_DIR + Dashboard::SETTINGS_FILE_NAME, QSettings::IniFormat);
    info.opened = settings.value(Dashboard::OPENED_SETTING).toBool();
    info.name = settings.value(Dashboard::NAME_SETTING).toString();
    return true;
}

}   // namespace U2
