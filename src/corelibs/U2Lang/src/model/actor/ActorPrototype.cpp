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

#include "ActorPrototype.h"

#include <U2Lang/Actor.h>

#include "ActorConfigurationEditor.h"
#include "ActorDocument.h"

namespace U2 {
namespace Workflow {

QList<PortDescriptor*> ActorPrototype::getPortDesciptors() const {
    return ports;
}

QList<Attribute*> ActorPrototype::getAttributes() const {
    return attrs;
}

void ActorPrototype::addAttribute(Attribute* a) {
    assert(a != nullptr);
    attrs << a;
}

int ActorPrototype::removeAttribute(Attribute* attr) {
    assert(attr != nullptr);
    return attrs.removeAll(attr);
}

void ActorPrototype::setEditor(ConfigurationEditor* e) {
    assert(e != nullptr);
    ed = e;
}

ConfigurationEditor* ActorPrototype::getEditor() const {
    return ed;
}

void ActorPrototype::setValidator(ConfigurationValidator* v) {
    assert(v != nullptr);
    val = v;
}

void ActorPrototype::setPrompter(Prompter* p) {
    assert(p != nullptr);
    prompter = p;
}

void ActorPrototype::setPortValidator(const QString& id, ConfigurationValidator* v) {
    assert(v != nullptr);
    portValidators[id] = v;
}

bool ActorPrototype::isAcceptableDrop(const QMimeData*, QVariantMap*) const {
    return false;
}

Port* ActorPrototype::createPort(const PortDescriptor& d, Actor* p) {
    return new Port(d, p);
}

Actor* ActorPrototype::createInstance(const ActorId& actorId, AttributeScript* script, const QVariantMap& params) {
    Actor* proc = new Actor(actorId, this, script);
    if (ed) {
        ed->updateDelegates();
    }

    foreach (PortDescriptor* pd, getPortDesciptors()) {
        Port* p = createPort(*pd, proc);
        QString pid = pd->getId();
        if (portValidators.contains(pid)) {
            p->setValidator(portValidators.value(pid));
        }
        proc->ports[pid] = p;
    }
    foreach (Attribute* a, getAttributes()) {
        proc->addParameter(a->getId(), a->clone());
    }
    if (val) {
        proc->setValidator(val);
    }
    if (prompter) {
        proc->setDescription(prompter->createDescription(proc));
    }

    QMapIterator<QString, QVariant> i(params);
    while (i.hasNext()) {
        i.next();
        proc->setParameter(i.key(), i.value());
    }
    if (ed) {
        auto actorEd = dynamic_cast<ActorConfigurationEditor*>(ed);
        if (actorEd != nullptr) {
            auto editor = dynamic_cast<ActorConfigurationEditor*>(ed->clone());
            editor->setConfiguration(proc);
            proc->setEditor(editor);
        } else {
            proc->setEditor(ed->clone());
        }
    }
    proc->updateItemsAvailability();

    usageCounter++;
    connect(proc, SIGNAL(destroyed()), SLOT(sl_onActorDestruction()));

    return proc;
}

void ActorPrototype::setDisplayName(const QString& n) {
    VisualDescriptor::setDisplayName(n);
    emit si_nameChanged();
}

void ActorPrototype::setDocumentation(const QString& d) {
    VisualDescriptor::setDocumentation(d);
    emit si_descriptionChanged();
}

Attribute* ActorPrototype::getAttribute(const QString& id) const {
    Attribute* res = nullptr;
    foreach (Attribute* a, attrs) {
        if (a->getId() == id) {
            res = a;
            break;
        }
    }
    return res;
}

void ActorPrototype::setScriptFlag(bool flag) {
    isScript = flag;
}

void ActorPrototype::setNonStandard(const QString& path) {
    isStandard = false;
    actorFilePath = path;
}

void ActorPrototype::setSchema(const QString& path) {
    isStandard = false;
    isSchema = true;
    actorFilePath = path;
}

void ActorPrototype::addExternalTool(const QString& toolId, const QString& paramId) {
    externalTools[toolId] = paramId;
}

const StrStrMap& ActorPrototype::getExternalTools() const {
    return externalTools;
}

void ActorPrototype::clearExternalTools() {
    externalTools.clear();
}

ActorPrototype::ActorPrototype(const Descriptor& d,
                               const QList<PortDescriptor*>& ports,
                               const QList<Attribute*>& attrs)
    : QObject(nullptr), VisualDescriptor(d), attrs(attrs), ports(ports), ed(nullptr), val(nullptr), prompter(nullptr),
      isScript(false), isStandard(true), isSchema(false), allowsEmptyPorts(false), influenceOnPathFlag(false), usageCounter(0) {
}

ActorPrototype::~ActorPrototype() {
    qDeleteAll(attrs);
    qDeleteAll(ports);
    delete ed;
    delete val;
    delete prompter;
    qDeleteAll(portValidators);
}

void ActorPrototype::sl_onActorDestruction() {
    usageCounter--;
}

int ActorPrototype::getUsageCounter() const {
    return usageCounter;
}

}  // namespace Workflow
}  // namespace U2
