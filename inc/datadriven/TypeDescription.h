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

#ifndef TYPEDESCRIPTION_H_
#define TYPEDESCRIPTION_H_

#include <vector>

#include <qcc/String.h>

#include <alljoyn/Status.h>
#include <alljoyn/InterfaceDescription.h>

namespace datadriven {
/**
 * \private The generated type description will be derived from this class.
 */
class TypeDescription {
  public:
    TypeDescription(const char* name);

    virtual ~TypeDescription();

    /** returns the name of the type */
    const qcc::String& GetName() const;

    /** returns the index of a property */
    int GetPropertyIdx(const char* name) const;

    /** builds ajn interface description */
    QStatus BuildInterface(ajn::InterfaceDescription& iface) const;

    /** resolves ajn interface to array of member pointers (allocates memory !) */
    QStatus ResolveMembers(const ajn::InterfaceDescription& iface,
                           const ajn::InterfaceDescription::Member*** member) const;

    /** returns a vector of all emitted (a.k.a. cached) properties */
    std::vector<const char*> GetEmittablePropertyNames() const;

    /** return true if there are any properties */
    bool HasProperties() const;

  protected:
    const qcc::String name;

    enum class EmitChangesSignal :
        int8_t {
        NEVER,
        ALWAYS,
        INVALIDATES
    };
    void AddProperty(const char* name,
                     const char* signature,
                     uint8_t access,
                     EmitChangesSignal emits);

    void AddMethod(const char* name,
                   const char* inSig,
                   const char* outSig,
                   const char* argNames,
                   uint8_t annotation = 0,
                   const char* accessPerms = 0);

    void AddSignal(const char* name,
                   const char* sig,
                   const char* argNames,
                   uint8_t annotation = 0,
                   const char* accessPerms = 0);

  private:
    struct Property {
        const char* name;
        const char* sig;
        uint8_t access;
        EmitChangesSignal emits;
    };
    struct Method {
        const char* name;
        const char* inSig;
        const char* outSig;
        const char* argNames;
        uint8_t annotation;
        const char* accessPerms;
    };
    struct Signal {
        const char* name;
        const char* sig;
        const char* argNames;
        uint8_t annotation;
        const char* accessPerms;
    };
    std::vector<Property> properties;
    std::vector<Method> methods;
    std::vector<Signal> signals;
};
}

#undef QCC_MODULE
#endif /* TYPEDESCRIPTION_H_ */
