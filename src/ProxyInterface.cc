/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
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

#include <vector>

#include <datadriven/Marshal.h>
#include <datadriven/ProxyInterface.h>

#include "RegisteredTypeDescription.h"

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
    status(ER_FAIL), desc(desc), objId(objId), alive(false), propChangedListener(nullptr)
{
    proxyBusObject = objId.MakeProxyBusObject();
    status = proxyBusObject.AddInterface(desc.GetInterfaceDescription());
    if (ER_OK != status) {
        QCC_LogError(status, ("Failed to add interface"));
    }
}

ProxyInterface::~ProxyInterface()
{
    status = proxyBusObject.UnregisterPropertiesChangedListener(ifaceName.c_str(), *propChangedListener);
    if (ER_OK != status) {
        QCC_LogError(status, ("Failed to unregister property changed handler: %s", ifaceName.c_str()));
    }
}

QStatus ProxyInterface::RegisterPropertiesChangedHandler(ajn::ProxyBusObject::PropertiesChangedListener* listener)
{
    do {
        propChangedListener = listener;
        ifaceName = desc.GetDescription().GetName();
        status = proxyBusObject.RegisterPropertiesChangedListener(
            ifaceName.c_str(), NULL, 0, *propChangedListener, NULL);
        if (ER_OK != status) {
            QCC_LogError(status, ("Failed to register property changed handler"));
        }
    } while (0);
    return status;
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

QStatus ProxyInterface::Get(const char* propName, ajn::MsgArg& value) const
{
    const char* ifName = desc.GetDescription().GetName().c_str();
    return proxyBusObject.GetProperty(ifName, propName, value);
}

QStatus ProxyInterface::GetAll(ajn::MsgArg& values) const
{
    const char* ifName = desc.GetDescription().GetName().c_str();
    return proxyBusObject.GetAllProperties(ifName, values);
}

QStatus ProxyInterface::Set(const char* propName, const ajn::MsgArg& value) const
{
    const char* ifName = desc.GetDescription().GetName().c_str();
    return proxyBusObject.SetProperty(ifName, propName, const_cast<ajn::MsgArg&>(value));
}

void ProxyInterface::UpdateProperties(const ajn::MsgArg* values)
{
    status = ER_OK;

    if (desc.GetDescription().HasProperties()) {
        ajn::MsgArg tmp;

        if (nullptr == values) {
            // if no values are provided then get them from the remote object
            status = GetAll(tmp);
            if (ER_OK == status) {
                values = &tmp;
            } else {
                QCC_LogError(status, ("ProxyInterface: Failed to GetAll properties"));
            }
        }
        if (ER_OK == status) {
            if (ajn::ALLJOYN_ARRAY != values->typeId) {
                status = ER_FAIL;
                QCC_LogError(status, ("ProxyInterface: Invalid data"));
            } else {
                const ajn::MsgArg* elem = values->v_array.GetElements();
                size_t numElem = values->v_array.GetNumElements();
                for (size_t i = 0; i < numElem; i++) {
                    const ajn::MsgArg* key = elem[i].v_dictEntry.key;
                    const ajn::MsgArg* val = elem[i].v_dictEntry.val;
                    if (ajn::ALLJOYN_STRING != key->typeId) {
                        status = ER_FAIL;
                        break;
                    }

                    const ajn::MsgArg& msgarg = datadriven::MsgArgDereference(*val);
                    status = UnmarshalProperty(key->v_string.str, msgarg);
                    if (ER_OK != status) {
                        QCC_LogError(status, ("ProxyInterface: Failed to unmarshal property: %s", key->v_string.str));
                        break;
                    }
                }
            }
        }
    }
}
}
