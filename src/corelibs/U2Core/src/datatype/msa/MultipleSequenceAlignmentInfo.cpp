/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include "MultipleSequenceAlignmentInfo.h"

namespace U2 {

const QString MultipleSequenceAlignmentInfo::NAME                  = "NAME";
const QString MultipleSequenceAlignmentInfo::ACCESSION             = "ACCESSION";
const QString MultipleSequenceAlignmentInfo::DESCRIPTION           = "DESCRIPTION";
const QString MultipleSequenceAlignmentInfo::SS_CONSENSUS          = "SS_CONSENSUS";
const QString MultipleSequenceAlignmentInfo::REFERENCE_LINE        = "REFERENCE_LINE";
const QString MultipleSequenceAlignmentInfo::CUTOFFS               = "CUTOFFS";

bool MultipleSequenceAlignmentInfo::isValid( const QVariantMap& map ) {
    return hasName( map );
}

static QVariant getValue( const QString& tag, const QVariantMap& map ) {
    return map.value( tag );
}

static void setValue( QVariantMap& map, const QString& tag, const QVariant& val ) {
    assert( !tag.isEmpty() );
    if( !val.isNull() ) {
        map.insert( tag, val );
    }
}

QString MultipleSequenceAlignmentInfo::getName( const QVariantMap& map ) {
    return getValue( NAME, map ).toString();
}

void MultipleSequenceAlignmentInfo::setName( QVariantMap& map, const QString& name ) {
    setValue( map, NAME, name );
}

bool MultipleSequenceAlignmentInfo::hasName( const QVariantMap& map ) {
    return !getName( map ).isEmpty();
}

QString MultipleSequenceAlignmentInfo::getAccession( const QVariantMap& map ) {
    return getValue( ACCESSION, map ).toString();
}

void MultipleSequenceAlignmentInfo::setAccession( QVariantMap& map, const QString& acc ) {
    setValue( map, ACCESSION, acc );
}

bool MultipleSequenceAlignmentInfo::hasAccession( const QVariantMap& map ) {
    return !getAccession( map ).isEmpty();
}

QString MultipleSequenceAlignmentInfo::getDescription( const QVariantMap& map ) {
    return getValue( DESCRIPTION, map ).toString();
}

void MultipleSequenceAlignmentInfo::setDescription( QVariantMap& map, const QString& desc ) {
    setValue( map, DESCRIPTION, desc );
}

bool MultipleSequenceAlignmentInfo::hasDescription( const QVariantMap& map ) {
    return !getDescription( map ).isEmpty();
}

QString MultipleSequenceAlignmentInfo::getSSConsensus( const QVariantMap& map ) {
    return getValue( SS_CONSENSUS, map ).toString();
}

void MultipleSequenceAlignmentInfo::setSSConsensus( QVariantMap& map, const QString& cs ) {
    setValue( map, SS_CONSENSUS, cs );
}

bool MultipleSequenceAlignmentInfo::hasSSConsensus( const QVariantMap& map ) {
    return !getSSConsensus( map ).isEmpty();
}

QString MultipleSequenceAlignmentInfo::getReferenceLine( const QVariantMap& map ) {
    return getValue( REFERENCE_LINE, map ).toString();
}

void MultipleSequenceAlignmentInfo::setReferenceLine( QVariantMap& map ,const QString& rf ) {
    setValue( map, REFERENCE_LINE, rf );
}

bool MultipleSequenceAlignmentInfo::hasReferenceLine( const QVariantMap& map ) {
    return !getReferenceLine( map ).isEmpty();
}

void MultipleSequenceAlignmentInfo::setCutoff( QVariantMap& map, Cutoffs coff, float val ) {
    setValue( map, CUTOFFS + QString::number( static_cast< int >( coff ) ), val );
}

float MultipleSequenceAlignmentInfo::getCutoff( const QVariantMap& map, Cutoffs coff ) {
    return static_cast< float >( getValue( CUTOFFS + QString::number( static_cast< int >( coff ) ), map ).toDouble() );
}

bool MultipleSequenceAlignmentInfo::hasCutoff( const QVariantMap& map, Cutoffs coff ) {
    bool ok = false;
    getValue( CUTOFFS + QString::number( static_cast< int >( coff ) ), map ).toDouble( &ok );
    return ok;
}

}   // namespace U2
