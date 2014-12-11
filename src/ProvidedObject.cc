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

#include <datadriven/ProvidedObject.h>
#include <datadriven/ProvidedInterface.h>

#include "ObjectAdvertiserImpl.h"
#include "ProvidedObjectImpl.h"
#include "RegisteredTypeDescription.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

namespace datadriven {
ProvidedObject::ProvidedObject(std::shared_ptr<ObjectAdvertiser> objectAdvertiser,
                               const qcc::String& path) :
    objectAdvertiserImpl(objectAdvertiser->GetImpl()),
    providedObjectImpl(new ProvidedObjectImpl(objectAdvertiser->GetImpl(), path, *this))
{
    providedObjectImpl->SetRefCountedPtr(providedObjectImpl);
}

ProvidedObject::ProvidedObject(std::shared_ptr<ObjectAdvertiser> objectAdvertiser) :
    objectAdvertiserImpl(objectAdvertiser->GetImpl()),
    providedObjectImpl(new ProvidedObjectImpl(objectAdvertiser->GetImpl(), *this))
{
    providedObjectImpl->SetRefCountedPtr(providedObjectImpl);
}

ProvidedObject::~ProvidedObject()
{
    RemoveFromBus();
}

std::shared_ptr<ProvidedObjectImpl> ProvidedObject::GetImpl()
{
    return providedObjectImpl;
}

void ProvidedObject::RemoveFromBus()
{
    // Remove BusObject from Alljoyn bus
    providedObjectImpl->RemoveFromBus();
}

ProvidedObject::State ProvidedObject::GetState()
{
    return (ProvidedObject::State)providedObjectImpl->GetState();
}

QStatus ProvidedObject::AddProvidedInterface(ProvidedInterface* providedInterface,
                                             MethodCallbacks callbacks[],
                                             size_t numCallbacks)
{
    QStatus status = ER_OK;

    if (NULL == providedInterface) {
        status = ER_BAD_ARG_1;
        QCC_LogError(status, ("Invalid arguments"));
        return status;
    }

    std::shared_ptr<ObjectAdvertiserImpl> advertiser = objectAdvertiserImpl.lock();
    if (advertiser) {
        if (ER_OK != advertiser->GetStatus()) {
            status = ER_FAIL;
            QCC_LogError(status, ("Object advertiser not properly initialized"));
            return status;
        }

        status = providedInterface->Register(advertiser->GetBusAttachment());
        const RegisteredTypeDescription* reg = providedInterface->GetRegisteredTypeDescription();
        if (status == ER_OK) {
            status = providedObjectImpl->AddInterfaceToBus(reg->GetInterfaceDescription());
            if (status == ER_OK) {
                qcc::String ifName = reg->GetDescription().GetName();

                interfaces[ifName] = providedInterface;
                providedObjectImpl->AddInterfaceName(ifName);
            }
        }

        if (ER_OK == status) {
            for (size_t i = 0; (ER_OK == status) && (i < numCallbacks); ++i) {
                MethodCallbacks& mh = callbacks[i];
                status = providedObjectImpl->AddMethodHandlerToBus(&reg->GetMember(
                                                                       mh.memberId), mh.handler, providedInterface);
            }
        }
    }
    if (ER_OK != status) {
        QCC_LogError(status, ("Failed to add published interface"));
    }
    return status;
}

void ProvidedObject::RemoveProvidedInterface(ProvidedInterface* providedInterface)
{
    if (NULL != providedInterface) {
        const RegisteredTypeDescription* reg = providedInterface->GetRegisteredTypeDescription();
        qcc::String ifName = reg->GetDescription().GetName();

        interfaces.erase(ifName);
        providedObjectImpl->RemoveInterfaceName(ifName);
    }
}

const char* ProvidedObject::GetPath() const
{
    return providedObjectImpl->GetPath();
}

QStatus ProvidedObject::UpdateAll()
{
    QStatus status;

    /**
     * The correct sequence to ensure data validity at all times is the following
     * - MarshalAllProperties
     * - Register the object to the bus
     * - Send the PropertiesChanged signal
     *
     * The announcement of the object will be seen by consumer, which will then GetallProperties.
     * Therefore we must make sure that the data is already marshaled.
     */

    // Marshal all properties of all provided interfaces of the object
    std::map<qcc::String, const ProvidedInterface*>::iterator it = interfaces.begin();
    std::map<qcc::String, const ProvidedInterface*>::iterator endit = interfaces.end();
    for (; it != endit; ++it) {
        ProvidedInterface* intf = const_cast<ProvidedInterface*>(it->second);
        if (ER_OK != (status = intf->MarshalProperties())) {
            return status;
        }
    }

    // Register the object on the bus
    status = providedObjectImpl->Register();
    if (ER_OK != status) {
        QCC_LogError(status, ("Failed as the object is in error/unregistered state"));
        return status;
    }

    // Call the propertiesChanged signal for all provided interfaces of the object
    for (it = interfaces.begin(); it != endit; ++it) {
        ProvidedInterface* intf = const_cast<ProvidedInterface*>(it->second);
        status = intf->Update();
        if (ER_OK != status) {
            QCC_LogError(status, ("Failed to update for a given interface"));
            return status;
        }
    }
    return status;
}

QStatus ProvidedObject::PutOnBus()
{
    return UpdateAll();
}

const ProvidedInterface* ProvidedObject::GetInterfaceByName(const char* name)
{
    std::map<qcc::String, const ProvidedInterface*>::const_iterator it = interfaces.find(qcc::String(name));

    if (it != interfaces.end()) {
        return it->second;
    }
    return NULL;
}

QStatus ProvidedObject::Get(const char* ifcName, const char* propName, ajn::MsgArg& val)
{
    QStatus status = ER_FAIL;
    ProvidedInterface* intf = const_cast<ProvidedInterface*>(GetInterfaceByName(ifcName));
    if (NULL != intf) {
        status = intf->GetProperty(propName, val);
    }
    return status;
}

QStatus ProvidedObject::Set(const char* ifcName, const char* propName, ajn::MsgArg& val)
{
    QStatus status = ER_FAIL;
    ProvidedInterface* intf = const_cast<ProvidedInterface*>(GetInterfaceByName(ifcName));
    if (NULL != intf) {
        status = intf->SetProperty(propName, val);
    }
    return status;
}
}
