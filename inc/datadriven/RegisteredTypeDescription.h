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

#ifndef REGISTEREDTYPEDESCRIPTION_H_
#define REGISTEREDTYPEDESCRIPTION_H_

#include <memory>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/InterfaceDescription.h>
#include <datadriven/TypeDescription.h>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_COMMON"

namespace datadriven {
/**
 * \private represents a registered type to a particular busattachment.
 * The amount of TypeDescription is fixed at compile type.
 * However, you can register this multiple times, hence creating multiple
 * RegisteredTypeDescriptions.
 */
class RegisteredTypeDescription {
  public:
    /** Internal method
     * \internal Register particular typedescription with a busattachment to get a new RegisterTypeDescription */
    static QStatus RegisterInterface(ajn::BusAttachment& busAttachment,
                                     const TypeDescription& desc,
                                     std::unique_ptr<RegisteredTypeDescription>& inst);

    /** Internal method
     * \internal Get the TypeDescription belonging to this RegisteredTypeDescription */
    const TypeDescription& GetDescription() const;

    /** Internal method
     * \internal Get AJN-level interface description */
    const ajn::InterfaceDescription& GetInterfaceDescription() const;

    /** Get specific member of AJN-level interface description */
    const ajn::InterfaceDescription::Member& GetMember(int memberNumber) const;

    ~RegisteredTypeDescription();

  protected:
    /** Reference to type description */
    const TypeDescription& desc;

    /** Pointer to alljoyn interface description */
    const ajn::InterfaceDescription* iface;

    /** array of pointer to ajn members */
    const ajn::InterfaceDescription::Member** member;

  private:
    RegisteredTypeDescription(const TypeDescription& desc);
};
}

#undef QCC_MODULE
#endif /* REGISTEREDTYPEDESCRIPTION_H_ */
