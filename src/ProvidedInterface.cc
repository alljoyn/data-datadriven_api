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
#include <datadriven/ProvidedObject.h>
#include "RegisteredTypeDescription.h"

#include "BusConnectionImpl.h"
#include "ProvidedObjectImpl.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

namespace datadriven {
ProvidedInterface::ProvidedInterface(const TypeDescription& desc,
                                     std::shared_ptr<ProvidedObjectImpl> providedObject) :
    _status(ER_OK), desc(desc), object(providedObject)
{
    const std::vector<const char*> names = desc.GetEmittablePropertyNames();
    std::vector<const char*>::const_iterator it;
    unsigned int idx;

    for (it = names.begin(), idx = 0; it != names.end(); it++, idx++) {
        marshaledProperties.insert(std::pair<qcc::String, PropertyValue*>(qcc::String(*it),  new PropertyValue(idx)));
    }
}

ProvidedInterface::~ProvidedInterface()
{
    for (std::map<qcc::String, PropertyValue*>::const_iterator it = marshaledProperties.begin();
         it != marshaledProperties.end();
         it++) {
        delete it->second;
    }
    marshaledProperties.clear();
}

std::shared_ptr<ProvidedObjectImpl> ProvidedInterface::GetObjectImpl()
{
    return object;
}

const RegisteredTypeDescription* ProvidedInterface::GetRegisteredTypeDescription() const
{
    return iface.get();
}

QStatus ProvidedInterface::Register(ajn::BusAttachment& ba)
{
    return RegisteredTypeDescription::RegisterInterface(ba, desc, iface);
}

QStatus ProvidedInterface::Update()
{
    if (ProvidedObjectImpl::REGISTERED != object->GetState()) {
        //TODO decide on the proper course of action here.
        //     For now, we just emit a big fat warning message.
        QCC_LogError(ER_BUS_OBJECT_NOT_REGISTERED,
                     ("Cannot signal property updates for an object that is not exposed on the bus"));
        return ER_FAIL;
    }

    // Marshal all properties
    if (ER_OK != (_status = MarshalProperties())) {
        return _status;
    }

    return SignalUpdate();
}

QStatus ProvidedInterface::SignalUpdate()
{
    /* Get list of properties that have to be emitted */
    std::vector<const char*> propertyNames = desc.GetEmittablePropertyNames();
    invalidatedPropertiesMutex.Lock(); /* for safe altering invalidatedProperties list */
    propertyNames.insert(propertyNames.end(), invalidatedProperties.begin(), invalidatedProperties.end());
    invalidatedProperties.clear();
    invalidatedPropertiesMutex.Unlock();
    /* signal change */
    return object->EmitPropChanged(desc.GetName().c_str(), &propertyNames[0],
                                   propertyNames.size(), ajn::SESSION_ID_ALL_HOSTED, 0);
}

QStatus ProvidedInterface::EmitSignal(int signalNumber,
                                      const ajn::MsgArg* args,
                                      size_t numArgs)
{
    return object->EmitSignal(iface->GetMember(signalNumber), args, numArgs);
}

QStatus ProvidedInterface::GetStatus() const
{
    return _status;
}

QStatus ProvidedInterface::GetProperty(const char* propName, ajn::MsgArg& value)
{
    qcc::String annotation;
    const ajn::InterfaceDescription ifDesc = iface->GetInterfaceDescription();
    ifDesc.GetPropertyAnnotation(propName, ajn::org::freedesktop::DBus::AnnotateEmitsChanged, annotation);

    if (annotation == "true") {
        // Get will be done on the cache
        std::map<qcc::String, PropertyValue*>::const_iterator it = marshaledProperties.find(propName);
        if (it != marshaledProperties.end()) {
            value = it->second->msgArg;
            _status = ER_OK;
        }
    } else {
        // Get will be handled by ProvidedInterface derived class
        _status = DispatchGetProperty(propName, value);
    }
    return _status;
}

QStatus ProvidedInterface::SetProperty(const char* propName, ajn::MsgArg& value)
{
    return DispatchSetProperty(propName, value);
}

QStatus ProvidedInterface::InvalidateProperty(const char* name)
{
    QStatus status = ER_OK;
    if (-1 == desc.GetPropertyIdx(name)) {
        status = ER_BUS_NO_SUCH_PROPERTY;
    } else {
        invalidatedPropertiesMutex.Lock();
        invalidatedProperties.insert(name);
        invalidatedPropertiesMutex.Unlock();
    }
    return status;
}
}
