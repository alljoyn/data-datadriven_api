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

#include <datadriven/ProxyInterface.h>
#include <datadriven/RegisteredTypeDescription.h>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
using namespace ajn;

const ObjectId& ProxyInterface::GetObjectId() const
{
    return objId;
}

ProxyInterface::ProxyInterface(const RegisteredTypeDescription& desc,
                               const ObjectId& objId) :
    status(ER_FAIL), desc(desc), objId(objId), alive(false)
{
    proxyBusObject = objId.MakeProxyBusObject();
    status = proxyBusObject.AddInterface(desc.GetInterfaceDescription());
    if (ER_OK != status) {
        QCC_LogError(status, ("Failed to add interface"));
    }
}

ProxyInterface::~ProxyInterface()
{
}

QStatus ProxyInterface::GetStatus() const
{
    return status;
}

const RegisteredTypeDescription& ProxyInterface::GetTypeDescription() const
{
    return desc;
}

bool ProxyInterface::IsAlive() const
{
    bool retval;
    mutex.Lock();
    retval = alive;
    mutex.Unlock();
    return retval;
}

void ProxyInterface::SetAlive(bool _alive)
{
    mutex.Lock();
    alive = _alive;
    mutex.Unlock();
}

const ajn::ProxyBusObject& ProxyInterface::GetProxyBusObject() const
{
    return proxyBusObject;
}
}
