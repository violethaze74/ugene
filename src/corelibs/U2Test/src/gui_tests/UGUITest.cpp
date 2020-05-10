/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2020 UniPro <ugene@unipro.ru>
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

#include <core/GUITestOpStatus.h>
#include <system/GTFile.h>

#include <QApplication>
#include <QDate>
#include <QDesktopWidget>
#include <QDir>
#include <QScreen>

#include <U2Core/GUrlUtils.h>
#include <U2Core/Log.h>

#include "UGUITest.h"

namespace U2 {

static QString getTestDir() {
    QString testDir = qgetenv("UGENE_TESTS_PATH");
    QFileInfo dirInfo(testDir);
    if (testDir.isEmpty()) {
        coreLog.error(QString("UGENE_TESTS_PATH is not set"));
        return "";
    }
    coreLog.info(QString("Using UGENE_TESTS_PATH = '%1'").arg(testDir));
    return testDir;
}

static QString getDataDir() {
    QString dataDir = qgetenv("UGENE_DATA_PATH");
    if (!dataDir.isEmpty()) {
        if (!QFileInfo(dataDir).exists()) {
            coreLog.error(QString("UGENE_DATA_PATH is defined, but doesn't exist: '%1'").arg(dataDir));
        } else {
            return dataDir + (dataDir.endsWith("/") ? "" : "/");
        }
    }

    bool ok = false;
    const int suiteNumber = qgetenv("UGENE_GUI_TEST_SUITE_NUMBER").toInt(&ok);
#ifdef Q_OS_MAC
    if (ok && suiteNumber > 1) {
        dataDir = QString("data%1/").arg(suiteNumber - 1);
    } else {
        dataDir = QString("data/");
    }
#else
    if (ok && suiteNumber > 1) {
        dataDir = QString("../../data%1/").arg(suiteNumber - 1);
    } else {
        dataDir = "../../data/";
    }

    if (!QFileInfo(dataDir).exists()) {
        dataDir = "data/";
    }
#endif

    if (!QFileInfo(dataDir).exists()) {
        coreLog.error(QString("dataDir not found in the default places"));
    }

    return dataDir;
}

static QString getScreenshotDir() {
    QString result;
#ifdef Q_OS_MAC
    result = "../../../../../../screenshotFol/";
#else
    QString guiTestOutputDirectory = qgetenv("GUI_TESTING_OUTPUT");
    if (guiTestOutputDirectory.isEmpty()) {
        result = QDir::homePath() + "/gui_testing_output/" +
                 QDate::currentDate().toString("dd.MM.yyyy") + "/screenshots/";
    } else {
        result = guiTestOutputDirectory + "/gui_testing_output/" +
                 QDate::currentDate().toString("dd.MM.yyyy") + "/screenshots/";
    }
#endif
    return result;
}

const QString UGUITest::testDir = getTestDir();
const QString UGUITest::dataDir = getDataDir();
const QString UGUITest::sandBoxDir = testDir + "_common_data/scenarios/sandbox/";
const QString UGUITest::screenshotDir = getScreenshotDir();

}    // namespace U2
