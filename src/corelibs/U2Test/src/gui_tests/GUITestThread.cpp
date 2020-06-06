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

#include <core/GUITest.h>
#include <core/GUITestOpStatus.h>
#include <core/MainThreadRunnable.h>
#include <drivers/GTMouseDriver.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QGuiApplication>
#include <QScreen>

#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>

#include "GUITestService.h"
#include "GUITestTeamcityLogger.h"
#include "GUITestThread.h"
#include "UGUITest.h"
#include "UGUITestBase.h"

namespace U2 {

GUITestThread::GUITestThread(HI::GUITest *test, bool isRunPostActionsAndCleanup)
    : test(test),
      isRunPostActionsAndCleanup(isRunPostActionsAndCleanup),
      testResult("Not run") {
    SAFE_POINT(test != nullptr, "GUITest is NULL", );
}

void GUITestThread::run() {
    SAFE_POINT(test != nullptr, "GUITest is NULL", );

    GUITests tests;
    tests << preChecks();
    tests << test;
    tests << postChecks();

    clearSandbox();

    QTimer timer;
    connect(&timer, SIGNAL(timeout()), this, SLOT(sl_getMemory()), Qt::DirectConnection);
    timer.start(1000);

    QString error = launchTest(tests);
    if (isRunPostActionsAndCleanup) {
        cleanup();
    }

    timer.stop();

    testResult = error.isEmpty() ? GUITestTeamcityLogger::successResult : error;
    writeTestResult();

    exit();
}

void GUITestThread::sl_testTimeOut() {
    saveScreenshot();
    cleanup();
    testResult = QString("test timed out");
    writeTestResult();
    exit();
}

QString GUITestThread::launchTest(const GUITests &tests) {
    QTimer::singleShot(test->getTimeout(), this, SLOT(sl_testTimeOut()));

    // Start all tests with some common mouse position.
    GTMouseDriver::moveTo(QPoint(400, 300));

    HI::GUITestOpStatus os;
    try {
        foreach (HI::GUITest *t, tests) {
            qDebug("launchTest: %s", t->getFullName().toLocal8Bit().constData());
            t->run(os);
        }
    } catch (HI::GUITestOpStatus *) {
    }
    //Run post checks if has error
    QString error = os.getError();
    if (!error.isEmpty()) {
        try {
            foreach (HI::GUITest *t, postChecks()) {
                t->run(os);
            }
        } catch (HI::GUITestOpStatus *) {
        }
    }
    return error;
}

GUITests GUITestThread::preChecks() {
    UGUITestBase *tb = AppContext::getGUITestBase();
    SAFE_POINT(NULL != tb, "GUITestBase is NULL", GUITests());

    //    GUITests additionalChecks = tb->takeTests(GUITestBase::PreAdditional);
    GUITests additionalChecks = tb->getTests(UGUITestBase::PreAdditional);
    SAFE_POINT(!additionalChecks.isEmpty(), "additionalChecks is empty", GUITests());

    return additionalChecks;
}

GUITests GUITestThread::postChecks() {
    UGUITestBase *tb = AppContext::getGUITestBase();
    GUITests additionalChecks = tb->getTests(UGUITestBase::PostAdditionalChecks);
    SAFE_POINT(!additionalChecks.isEmpty(), "additionalChecks is empty", GUITests());
    return additionalChecks;
}

GUITests GUITestThread::postActions() {
    UGUITestBase *tb = AppContext::getGUITestBase();
    SAFE_POINT(NULL != tb, "GUITestBase is NULL", GUITests());

    //    GUITests additionalChecks = tb->takeTests(GUITestBase::PostAdditionalActions);
    GUITests additionalChecks = tb->getTests(UGUITestBase::PostAdditionalActions);
    SAFE_POINT(!additionalChecks.isEmpty(), "additionalChecks is empty", GUITests());

    return additionalChecks;
}

void GUITestThread::clearSandbox() {
    const QString pathToSandbox = UGUITest::testDir + "_common_data/scenarios/sandbox/";
    QDir sandbox(pathToSandbox);

    foreach (const QString &fileName, sandbox.entryList()) {
        if (fileName != "." && fileName != "..") {
            if (QFile::remove(pathToSandbox + fileName)) {
                continue;
            } else {
                QDir dir(pathToSandbox + fileName);
                removeDir(dir.absolutePath());
            }
        }
    }
}

void GUITestThread::removeDir(const QString &dirName) {
    QDir dir(dirName);

    foreach (const QFileInfo &fileInfo, dir.entryInfoList()) {
        const QString fileName = fileInfo.fileName();
        const QString filePath = fileInfo.filePath();
        if (fileName != "." && fileName != "..") {
            if (QFile::remove(filePath)) {
                continue;
            } else {
                QDir subDir(filePath);
                if (subDir.rmdir(filePath)) {
                    continue;
                } else {
                    removeDir(filePath);
                }
            }
        }
    }
    dir.rmdir(dir.absoluteFilePath(dirName));
}

void GUITestThread::saveScreenshot() {
    class Scenario : public HI::CustomScenario {
    public:
        Scenario(HI::GUITest *test)
            : test(test) {
        }

        void run(HI::GUITestOpStatus &) {
            const QPixmap originalPixmap = QGuiApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId());
            originalPixmap.save(HI::GUITest::screenshotDir + test->getFullName() + ".jpg");
        }

    private:
        HI::GUITest *test;
    };

    HI::GUITestOpStatus os;
    HI::MainThreadRunnable::runInMainThread(os, new Scenario(test));
}

void GUITestThread::cleanup() {
    test->cleanup();
    foreach (HI::GUITest *postAction, postActions()) {
        HI::GUITestOpStatus os;
        try {
            postAction->run(os);
        } catch (HI::GUITestOpStatus *opStatus) {
            coreLog.error(opStatus->getError());
        }
    }
}

void GUITestThread::writeTestResult() {
    printf("%s\n", (GUITestService::GUITESTING_REPORT_PREFIX + ": " + testResult).toUtf8().data());
}

}    // namespace U2
