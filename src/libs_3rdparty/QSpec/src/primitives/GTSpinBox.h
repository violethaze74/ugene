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

#pragma once
#include <QSpinBox>

#include "GTGlobals.h"

namespace HI {
/*!
 * \brief The class for working with QSpinBox primitive
 */
class HI_EXPORT GTSpinBox {
public:
    static int getValue(QSpinBox* spinBox);
    static int getValue(const QString& spinBoxName, QWidget* parent = nullptr);

    static void setValue(QSpinBox* spinBox, int v, GTGlobals::UseMethod useMethod = GTGlobals::UseKeyBoard);
    static void setValue(const QString& spinBoxName, int v, GTGlobals::UseMethod useMethod = GTGlobals::UseKeyBoard, QWidget* parent = nullptr);
    static void setValue(const QString& spinBoxName, int v, QWidget* parent = NULL);

    static void checkLimits(QSpinBox* spinBox, int min, int max);
    static void checkLimits(const QString& spinBoxName, int min, int max, QWidget* parent = nullptr);
};

}  // namespace HI
