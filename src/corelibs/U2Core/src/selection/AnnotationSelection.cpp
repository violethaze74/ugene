/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2019 UniPro <ugene@unipro.ru>
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

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/SelectionTypes.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SequenceUtils.h>

#include "AnnotationSelection.h"

namespace U2 {

static QList<Annotation *> emptyAnnotations;

AnnotationSelectionData::AnnotationSelectionData(Annotation *a) : annotation(a) {
}

bool AnnotationSelectionData::operator ==(const AnnotationSelectionData &d) const {
    return d.annotation == annotation;
}

QVector<U2Region> AnnotationSelectionData::getSelectedRegions() const {
    return annotation->getRegions();
}

qint64 AnnotationSelectionData::getSelectedRegionsLen() const {
    qint64 len = 0;
    foreach(const U2Region& region, annotation->getRegions()) {
        len += region.length;
    }
    return len;
}

AnnotationSelection::AnnotationSelection(QObject *p) : GSelection(GSelectionTypes::ANNOTATIONS, p) {
    connect(this, SIGNAL(si_selectionChanged(AnnotationSelection *, const QList<Annotation *> &, const QList<Annotation *> &)), SLOT(sl_selectionChanged()));
}

const QList<AnnotationSelectionData> & AnnotationSelection::getSelection() const {
    return selection;
}

bool AnnotationSelection::isEmpty() const {
    return selection.isEmpty();
}

void AnnotationSelection::clear() {
    if (selection.isEmpty()) {
        return;
    }
    QList<Annotation *> tmpRemoved;
    foreach(const AnnotationSelectionData &asd, selection) {
        tmpRemoved.push_back(asd.annotation);
    }
    selection.clear();
    emit si_selectionChanged(this, emptyAnnotations, tmpRemoved);
}

void AnnotationSelection::removeObjectAnnotations(AnnotationTableObject *obj) {
    QList<Annotation *> removed;
    foreach(Annotation *a, obj->getAnnotations()) {
        for (int i = 0; i < selection.size(); i++) {
            if (selection[i].annotation == a) {
                removed.append(a);
                selection.removeAt(i);
                --i;
            }
        }
    }
    emit si_selectionChanged(this, emptyAnnotations, removed);
}

void AnnotationSelection::addToSelection(Annotation *a) {
    for (int i = 0; i < selection.size(); i++) {
        if (selection[i].annotation == a) {
            return; //nothing changed
        }
    }
    selection.append(AnnotationSelectionData(a));
    emit si_selectionChanged(this, QList<Annotation*>() << a, emptyAnnotations);
}

void AnnotationSelection::removeFromSelection(Annotation *a) {
    bool removed = false;
    for (int i = 0; i < selection.size(); i++) {
        if (selection[i].annotation == a) {
            selection.removeAt(i);
            removed = true;
            break;
        }
    }
    if (removed) {
        emit si_selectionChanged(this, emptyAnnotations, QList<Annotation*>() << a);
    }
}

void AnnotationSelection::getAnnotationSequence(QByteArray &res, const AnnotationSelectionData &ad, char gapSym, const U2EntityRef &seqRef,
    const DNATranslation *complTT, const DNATranslation *aminoTT, U2OpStatus &os) {
    QVector<U2Region> regions = ad.annotation->getRegions();
    QList<QByteArray> parts = U2SequenceUtils::extractRegions(seqRef, regions, complTT, aminoTT, false, os);
    CHECK_OP(os, )
    qint64 resLen = 0;
    foreach(const QByteArray &p, parts) {
        resLen += p.length() + 1; // +1 is for the gap char.
    }
    res.reserve(resLen);
    foreach(const QByteArray &p, parts) {
        if (!res.isEmpty()) {
            res.append(gapSym);
        }
        res.append(p);
    }
}

bool AnnotationSelection::contains(const Annotation *a) const {
    foreach(const AnnotationSelectionData &asd, selection) {
        if (asd.annotation == a) {
            return true;
        }
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////
// Annotation group selection
static QList<AnnotationGroup *> emptyGroups;

AnnotationGroupSelection::AnnotationGroupSelection(QObject *p)
    : GSelection(GSelectionTypes::ANNOTATION_GROUPS, p) {
    connect(this, SIGNAL(si_selectionChanged(AnnotationGroupSelection *, const QList<AnnotationGroup *> &, const QList<AnnotationGroup *> &)),
        SLOT(sl_selectionChanged()));
}

const QList<AnnotationGroup *> & AnnotationGroupSelection::getSelection() const {
    return selection;
}

bool AnnotationGroupSelection::isEmpty() const {
    return selection.isEmpty();
}

bool AnnotationGroupSelection::contains(AnnotationGroup *g) const {
    return selection.contains(g);
}

void AnnotationGroupSelection::clear() {
    if (selection.isEmpty()) {
        return;
    }
    QList<AnnotationGroup *> tmpRemoved = selection;
    selection.clear();
    emit si_selectionChanged(this, emptyGroups, tmpRemoved);
}

void AnnotationGroupSelection::addToSelection(AnnotationGroup *g) {
    if (selection.contains(g)) {
        return;
    }
    selection.append(g);
    QList<AnnotationGroup *> tmpAdded;
    tmpAdded.append(g);
    emit si_selectionChanged(this, tmpAdded, emptyGroups);
}


void AnnotationGroupSelection::removeFromSelection(AnnotationGroup *g) {
    const bool found = selection.removeOne(g);
    if (found) {
        QList<AnnotationGroup *> tmpRemoved;
        tmpRemoved.append(g);
        emit si_selectionChanged(this, emptyGroups, tmpRemoved);
    }
}

}//namespace
