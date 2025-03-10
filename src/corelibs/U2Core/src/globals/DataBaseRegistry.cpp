/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2023 UniPro <ugene@unipro.ru>
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

#include "DataBaseRegistry.h"

namespace U2 {

DataBaseRegistry::DataBaseRegistry(QObject* o)
    : QObject(o) {
}

DataBaseRegistry::~DataBaseRegistry() {
    foreach (const DataBaseFactory* dbf, factories) {
        delete dbf;
    }
}

bool DataBaseRegistry::registerDataBase(DataBaseFactory* f, const QString& id) {
    if (!isRegistered(id)) {
        factories[id] = f;
        return true;
    } else {
        return false;
    }
}

bool DataBaseRegistry::isRegistered(const QString& id) {
    if (factories.contains(id)) {
        return true;
    } else {
        return false;
    }
}

DataBaseFactory* DataBaseRegistry::getFactoryById(const QString& id) {
    if (isRegistered(id)) {
        return factories[id];
    } else {
        return nullptr;
    }
}

}  // namespace U2
