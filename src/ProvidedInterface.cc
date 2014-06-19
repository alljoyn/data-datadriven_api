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

#include <datadriven/ProvidedInterface.h>
#include <datadriven/RegisteredTypeDescription.h>
#include <datadriven/ProvidedObject.h>
#include "WriterCache.h"
#include "BusConnectionImpl.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

namespace datadriven {
ProvidedInterface::ProvidedInterface(const TypeDescription& desc,
                                     ProvidedObject& providedObject) :
    desc(desc), providedObject(providedObject)
{
}

ProvidedInterface::~ProvidedInterface()
{
}

const TypeDescription* ProvidedInterface::GetTypeDescription() const
{
    return &desc;
}

const RegisteredTypeDescription* ProvidedInterface::GetRegisteredTypeDescription() const
{
    return iface.get();
}

QStatus ProvidedInterface::RegisterInterface(ajn::BusAttachment& ba)
{
    return RegisteredTypeDescription::RegisterInterface(ba, desc, iface);
}

QStatus ProvidedInterface::Update()
{
    if (ProvidedObject::REGISTERED != providedObject.GetState()) {
        //TODO decide on the proper course of action here.
        //     For now, we just emit a big fat warning message.
        QCC_LogError(ER_BUS_OBJECT_NOT_REGISTERED,
                     ("Cannot signal property updates for an object that is not exposed on the bus"));
        return ER_FAIL;
    }
    WriterCache::Properties args = MarshalProperties();

    if (ER_OK != status) {
        return status;
    }

    std::shared_ptr<BusConnectionImpl> busConn = providedObject.GetBusConnection().lock();

    if (busConn) {
        WriterCache* cache = busConn->GetWriterCache(desc.GetName());

        if (NULL != cache) {
            status = cache->Update(&providedObject, args);
            if (status != ER_OK) {
                QCC_LogError(status, ("Failed to update cache"));
            }
        } else {
            status = ER_FAIL;
            QCC_LogError(status, ("Invalid cache"));
        }
    } else {
        status = ER_FAIL;
        QCC_LogError(status, ("Invalid bus connection"));
    }
    return status;
}

ProvidedObject& ProvidedInterface::GetProvidedObject()
{
    return providedObject;
}

QStatus ProvidedInterface::GetStatus() const
{
    return status;
}
}
