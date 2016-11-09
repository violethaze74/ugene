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

#ifndef _U2_TABLEWIDGET_H_
#define _U2_TABLEWIDGET_H_

#include <U2Designer/Dashboard.h>

namespace U2 {

class TableWidget : public DashboardWidget {
    Q_OBJECT
public:
#if (QT_VERSION < 0x050400) //Qt 5.7
    TableWidget(const QWebElement &container, Dashboard *parent);
#else
    TableWidget(const QString &container, Dashboard *parent);
#endif

    /** The list of % */
    virtual QList<int> widths() = 0;
    virtual QStringList header() = 0;
    /** The first item of data is the id, which has not the column */
    virtual QList<QStringList> data() = 0;

    static QString wrapLongText(const QString &text);

protected slots:
    void createTable();
    void fillTable();

protected:
    bool useEmptyRows;
#if (QT_VERSION < 0x050400) //Qt 5.7
    QMap<QString, QWebElement> rows;
#else
    QMap<QString, QString> rows;
#endif

protected:
    void addRow(const QString &dataId, const QStringList &d);
    void updateRow(const QString &dataId, const QStringList &d);
    virtual QString createRow(const QStringList &d);

private:
    void addEmptyRows();
};

} // U2

#endif // _U2_TABLEWIDGET_H_
