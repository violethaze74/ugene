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

#include <QApplication>
#include <QDate>
#include <QDesktopWidget>
#include <QDir>
#include <QScreen>

#include <core/GUITestOpStatus.h>
#include <system/GTFile.h>

#include <U2Core/GUrlUtils.h>
#include <U2Core/Log.h>

#include "UGUITest.h"

namespace U2 {

QString getTestDir(){
    const QString testDir = qgetenv("UGENE_TESTS_PATH");
    if (!testDir.isEmpty()) {
        if (!QFileInfo(testDir).exists()) {
            coreLog.error(QString("UGENE_TESTS_PATH is defined, but doesn't exist: '%1'").arg(testDir));
        } else {
            return testDir + (testDir.endsWith("/") ? "" : "/");
        }
    }

    bool ok;
    int i = qgetenv("UGENE_GUI_TEST_SUITE_NUMBER").toInt(&ok);
#ifdef Q_OS_MAC
    if ( ok && i>1){
        return QString("../../../../../../test%1/").arg(i-1);
    }else{
        return QString("../../../../../../test/");
    }
#else
    if ( ok && i>1){
        return QString("../../test%1/").arg(i-1);
    }else{
        return QString("../../test/");
    }
#endif
}

QString getDataDir(){
    bool ok;
    int i = qgetenv("UGENE_GUI_TEST_SUITE_NUMBER").toInt(&ok);
#ifdef Q_OS_MAC
    if ( ok && i>1 ){
        return QString("data%1/").arg(i-1);
    }else{
        return QString("data/");
    }
#else
    if ( ok && i>1){
        return QString("../../data%1/").arg(i-1);
    }else{
        return QString("../../data/");
    }
#endif
}

QString getScreenshotDir(){
    QString result;
#ifdef Q_OS_MAC
    result = "../../../../../../screenshotFol/";
#else
    QString guiTestOutputDirectory = qgetenv("GUI_TESTING_OUTPUT");
    if(guiTestOutputDirectory.isEmpty()){
        result = QDir::homePath() + "/gui_testing_output/" +
                QDate::currentDate().toString("dd.MM.yyyy") + "/screenshots/";
    }else{
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

}   // namespace U2
