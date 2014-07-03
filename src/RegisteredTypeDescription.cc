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

#include <iostream>

#include "RegisteredTypeDescription.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_COMMON"

namespace datadriven {
QStatus RegisteredTypeDescription::RegisterInterface(ajn::BusAttachment& ba,
                                                     const TypeDescription& desc,
                                                     std::unique_ptr<RegisteredTypeDescription>& inst)
{
    QStatus status = ER_OUT_OF_MEMORY;
    ajn::InterfaceDescription* tmp;

    inst = std::unique_ptr<RegisteredTypeDescription>(new RegisteredTypeDescription(desc));
    if (nullptr != inst) {
        status = ba.CreateInterface(desc.GetName().c_str(), tmp);
        if (ER_BUS_IFACE_ALREADY_EXISTS == status) {
            inst->iface = ba.GetInterface(desc.GetName().c_str());
            status = ER_OK;
        } else if (ER_OK == status) {
            status = desc.BuildInterface(*tmp);
            if (ER_OK == status) {
                tmp->Activate();
                inst->iface = tmp;
            } else {
                QCC_LogError(status, ("Failed to build interface '%s'", desc.GetName().c_str()));
            }
        } else {
            QCC_LogError(status, ("Failed to create interface '%s'", desc.GetName().c_str()));
        }
        if (ER_OK == status) {
            status = desc.ResolveMembers(*inst->iface, &inst->member);
        }
    }

    if (status != ER_OK) {
        inst.reset();
    }

    return status;
}

RegisteredTypeDescription::~RegisteredTypeDescription()
{
    delete[] member;
}

RegisteredTypeDescription::RegisteredTypeDescription(const TypeDescription& desc) :
    desc(desc), iface(NULL), member(NULL)
{
}

const TypeDescription& RegisteredTypeDescription::GetDescription() const
{
    return desc;
}

const ajn::InterfaceDescription& RegisteredTypeDescription::GetInterfaceDescription() const
{
    return *iface;
}

const ajn::InterfaceDescription::Member& RegisteredTypeDescription::GetMember(int memberNumber) const
{
    return *member[memberNumber];
}
};
