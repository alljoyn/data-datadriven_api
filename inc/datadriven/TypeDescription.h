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

#include <qcc/String.h>
#include <alljoyn/Status.h>
#include <alljoyn/InterfaceDescription.h>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_COMMON"

namespace datadriven {
/**
 * \private The generated type description will be derived from this class.
 */
class TypeDescription {
  public:
    /** returns the name of the type */
    virtual const qcc::String GetName() const = 0;

    /** builds ajn interface description */
    virtual QStatus BuildInterface(ajn::InterfaceDescription& iface) const = 0;

    /** resolves ajn interface to array of member pointers (allocates memory !) */
    virtual QStatus ResolveMembers(const ajn::InterfaceDescription& iface,
                                   const ajn::InterfaceDescription::Member*** member) const = 0;

    virtual ~TypeDescription();
};
}

#undef QCC_MODULE
#endif /* TYPEDESCRIPTION_H_ */
