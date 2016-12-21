/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QTimer>

#include "StatisticsWidget.h"

namespace U2 {

inline static QString timeStr(qint64 timeMks) {
    QDateTime t;
    t.setTimeSpec(Qt::UTC);
    t.setMSecsSinceEpoch(timeMks / 1000);
    return t.toString("hh:mm:ss.zzz");
}

#if (QT_VERSION < 0x050400) //Qt 5.7
StatisticsWidget::StatisticsWidget(const QWebElement &container, Dashboard *parent)
#else
StatisticsWidget::StatisticsWidget(const QString &container, Dashboard *parent)
#endif
: TableWidget(container, parent)
{
    useEmptyRows = false;
    createTable();
    fillTable();
    connect(dashboard->monitor(), SIGNAL(si_workerInfoChanged(const QString &, const U2::Workflow::Monitor::WorkerInfo &)),
        SLOT(sl_workerInfoChanged(const QString &, const U2::Workflow::Monitor::WorkerInfo &)));
    connect(dashboard->monitor(), SIGNAL(si_updateProducers()),
        SLOT(sl_update()));
}

void StatisticsWidget::sl_workerInfoChanged(const QString &actor,
    const U2::Workflow::Monitor::WorkerInfo &info)
{
#if (QT_VERSION < 0x050400) //Qt 5.7
    updateRow(actor, createRowByWorker(actor, info));
#else
    //assert(0);
#endif
}

void StatisticsWidget::sl_update() {
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, );

    QMap<QString, WorkerInfo> infos = m->getWorkersInfo();
    foreach (const QString &actor, infos.keys()) {
#if (QT_VERSION < 0x050400) //Qt 5.7
        updateRow(actor, createRowByWorker(actor, infos[actor]));
#else
        dashboard->page()->runJavaScript("updateRow(\"" + container + "\", createRowByWorker(\"" + m->actorName(actor) + "\",\"" + timeStr(infos[actor].timeMks) + "\",\"" + QString::number(m->getDataProduced(actor)) + "\"), " + actor + ");");
#endif
    }
}

QList<int> StatisticsWidget::widths() {
    return QList<int>() << 40 << 30 << 30;
}

QStringList StatisticsWidget::header() {
    return QStringList() << tr("Element") << tr("Elapsed time") << tr("Output messages");
}

QList<QStringList> StatisticsWidget::data() {
    QList<QStringList> result;
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, result);
    QMap<QString, WorkerInfo> infos = m->getWorkersInfo();
    foreach (const QString &actorId, infos.keys()) {
#if (QT_VERSION < 0x050400) //Qt 5.7
        WorkerInfo &info = infos[actorId];
        QStringList row;
        row << actorId; // id
        row << createRowByWorker(actorId, info);
        result << row;
#else
        dashboard->page()->runJavaScript("updateRow(\"" + container + "\", createRowByWorker(\"" + m->actorName(actorId) + "\",\"" + timeStr(infos[actorId].timeMks) + "\",\"" + QString::number(m->getDataProduced(actorId)) + "\"), " + actorId + ");");
#endif
    }
    return result;
}

#if (QT_VERSION < 0x050400) //Qt 5.7
QStringList StatisticsWidget::createRowByWorker(const QString &actor, const WorkerInfo &info) {
    QStringList result;
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, result);

    result << wrapLongText(m->actorName(actor));
    result << timeStr(info.timeMks);
    result << QString::number(m->getDataProduced(actor));
    return result;
}
#endif

} // U2
