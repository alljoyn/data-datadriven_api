/******************************************************************************
 * Copyright (c) 2014, AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#include <datadriven/TypeDescription.h>

#include <qcc/Debug.h>
#include <alljoyn/DBusStd.h>
#define QCC_MODULE "DD_COMMON"

using namespace datadriven;

TypeDescription::TypeDescription(const char* name) :
    name(name)
{
}

TypeDescription::~TypeDescription()
{
};

const qcc::String& TypeDescription::GetName() const
{
    return name;
}

int TypeDescription::GetPropertyIdx(const char* name) const
{
    size_t i;

    for (i = 0; i < properties.size(); i++) {
        if (0 == strcmp(name, properties[i].name)) {
            break;
        }
    }
    return (i == properties.size() ? -1 : i);
}

void TypeDescription::AddProperty(const char* name,
                                  const char* signature,
                                  uint8_t access,
                                  EmitChangesSignal emits)
{
    Property p = { name, signature, access, emits };
    properties.push_back(p);
}

void TypeDescription::AddMethod(const char* name,
                                const char* inSig,
                                const char* outSig,
                                const char* argNames,
                                uint8_t annotation,
                                const char* accessPerms)
{
    Method m = { name, inSig, outSig, argNames, annotation, accessPerms };
    methods.push_back(m);
}

void TypeDescription::AddSignal(const char* name,
                                const char* sig,
                                const char* argNames,
                                uint8_t annotation,
                                const char* accessPerms)
{
    Signal s = { name, sig, argNames, annotation, accessPerms };
    signals.push_back(s);
}

QStatus TypeDescription::BuildInterface(ajn::InterfaceDescription& iface) const
{
    QStatus status = ER_OK;

    do {
        qcc::String updateSig;
        qcc::String updateArgs;

        // add properties
        for (size_t i = 0; (ER_OK == status) && (i < properties.size()); i++) {
            Property p = properties[i];
            status = iface.AddProperty(p.name, p.sig, p.access);
            if (ER_OK == status) {
                qcc::String emits;
                switch (p.emits) {
                case EmitChangesSignal::NEVER:
                    emits = "false";
                    break;

                case EmitChangesSignal::ALWAYS:
                    emits = "true";
                    updateSig += p.sig;
                    if (updateArgs.size() > 0) {
                        updateArgs.push_back(',');
                    }
                    updateArgs += p.name;
                    break;

                case EmitChangesSignal::INVALIDATES:
                    emits = "invalidates";
                    break;
                }
                status = iface.AddPropertyAnnotation(p.name, ajn::org::freedesktop::DBus::AnnotateEmitsChanged, emits);
            }
        }
        // add defined methods
        for (size_t i = 0; (ER_OK == status) && (i < methods.size()); i++) {
            Method m = methods[i];
            status = iface.AddMethod(m.name, m.inSig, m.outSig, m.argNames, m.annotation, m.accessPerms);
        }
        // add defined signals
        for (size_t i = 0; (ER_OK == status) && (i < signals.size()); i++) {
            Signal s = signals[i];
            status = iface.AddSignal(s.name, s.sig, s.argNames, s.annotation, s.accessPerms);
        }
    } while (0);
    return status;
}

/** resolves ajn interface to array of member pointers (allocates memory !) */
QStatus TypeDescription::ResolveMembers(const ajn::InterfaceDescription& iface,
                                        const ajn::InterfaceDescription::Member*** member) const
{
    QStatus status = ER_OK;
    size_t cnt = 0;

    cnt += methods.size();
    cnt += signals.size();
    // keep references to actual members for later usage
    *member = NULL;
    *member = new const ajn::InterfaceDescription::Member *[cnt];
    if (NULL != *member) {
        int idx = 0;

        for (size_t i = 0; i < methods.size(); i++) {
            (*member)[idx++] = iface.GetMethod(methods[i].name);
        }
        for (size_t i = 0; i < signals.size(); i++) {
            (*member)[idx++] = iface.GetSignal(signals[i].name);
        }
    } else {
        status = ER_OUT_OF_MEMORY;
    }
    return status;
}

std::vector<const char*> TypeDescription::GetEmittablePropertyNames() const
{
    std::vector<const char*> names;
    std::vector<Property>::const_iterator it;

    for (it = properties.begin(); it != properties.end(); it++) {
        if ((*it).emits == EmitChangesSignal::ALWAYS) {
            names.push_back((*it).name);
        }
    }
    return names;
}

bool TypeDescription::HasProperties() const
{
    return (0 != properties.size());
}
