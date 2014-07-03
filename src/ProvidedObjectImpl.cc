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

#include <qcc/GUID.h>

#include <datadriven/ProvidedInterface.h>
#include <datadriven/ProvidedObject.h>

#include "BusConnectionImpl.h"
#include "ObjectAdvertiserImpl.h"
#include "ProvidedObjectImpl.h"
#include "RegisteredTypeDescription.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

namespace datadriven {
class ProvidedObjectImpl::MethodHandlerTask :
    public ObjectAdvertiserImpl::Task {
  private:
    std::weak_ptr<ObjectAdvertiserImpl> objectAdvertiserImpl;
    std::weak_ptr<ProvidedObjectImpl> object;
    ajn::MessageReceiver* ctx;
    ajn::MessageReceiver::MethodHandler handler;
    const ajn::InterfaceDescription::Member* member;
    ajn::Message message;

  public:
    virtual ~MethodHandlerTask() { }

    MethodHandlerTask(std::weak_ptr<ObjectAdvertiserImpl> objectAdvertiserImpl,
                      std::weak_ptr<ProvidedObjectImpl> object,
                      ajn::MessageReceiver* ctx,
                      ajn::MessageReceiver::MethodHandler handler,
                      const ajn::InterfaceDescription::Member* member,
                      ajn::Message& message) :
        objectAdvertiserImpl(objectAdvertiserImpl), object(object), ctx(ctx), handler(handler), member(member),
        message(message) { }

    virtual void Execute() const
    {
        std::shared_ptr<ObjectAdvertiserImpl> advertiser = objectAdvertiserImpl.lock();
        if (advertiser) {
            advertiser->CallMethodHandler(object, ctx, handler, member, const_cast<ajn::Message&>(message));
        }
    }
};

ProvidedObjectImpl::ProvidedObjectImpl(std::weak_ptr<ObjectAdvertiserImpl> objectAdvertiserImpl,
                                       const qcc::String& path,
                                       ProvidedObject& obj) :
    BusObject(path.c_str()), objectAdvertiserImpl(objectAdvertiserImpl), state(ProvidedObjectImpl::CONSTRUCTED),
    providedObject(obj)
{
}

ProvidedObjectImpl::ProvidedObjectImpl(std::weak_ptr<ObjectAdvertiserImpl> objectAdvertiserImpl,
                                       ProvidedObject& obj) :
    BusObject(GeneratePath().c_str()), objectAdvertiserImpl(objectAdvertiserImpl),
    state(ProvidedObjectImpl::CONSTRUCTED), providedObject(obj)
{
}

ProvidedObjectImpl::~ProvidedObjectImpl()
{
}

qcc::String ProvidedObjectImpl::GeneratePath()
{
    return "/O" + qcc::GUID128().ToString();
}

QStatus ProvidedObjectImpl::Register()
{
    QStatus result = ER_FAIL;
    if (ProvidedObjectImpl::REGISTERED != state) {
        std::shared_ptr<ObjectAdvertiserImpl> advertiser = objectAdvertiserImpl.lock();
        if (advertiser) {
            result = advertiser->AddProvidedObject(self);

            if (ER_OK == result) {
                state = ProvidedObjectImpl::REGISTERED;
                result = ER_OK;
            } else {
                state = ProvidedObjectImpl::ERROR;
                QCC_LogError(ER_FAIL, ("Could not register object on the bus"));
            }
        }
    } else {
        result = ER_OK;
    }
    return result;
}

void ProvidedObjectImpl::CallMethodHandler(ajn::MessageReceiver::MethodHandler handler,
                                           const ajn::InterfaceDescription::Member* member,
                                           ajn::Message& message,
                                           void* context)
{
    if (NULL == context) {
        // When registering method handlers the DDAPI will provide an extra
        // context.  If no such context is present then it was not a handler
        // of the DDAPI and we use the mechanism of BusObject.h
        (this->*handler)(member, message);
    } else {
        ajn::MessageReceiver* ctxObject = static_cast<ajn::MessageReceiver*>(context);
        std::shared_ptr<ObjectAdvertiserImpl> advertiser = objectAdvertiserImpl.lock();
        if (advertiser) {
            advertiser->ProviderAsyncEnqueue(new MethodHandlerTask(objectAdvertiserImpl,
                                                                   self,
                                                                   ctxObject,
                                                                   handler,
                                                                   member,
                                                                   message));
        }
    }
}

void ProvidedObjectImpl::RemoveFromBus()
{
    if (state == ProvidedObjectImpl::REGISTERED) {
        std::shared_ptr<ObjectAdvertiserImpl> advertiser = objectAdvertiserImpl.lock();
        if (advertiser) {
            advertiser->RemoveProvidedObject(self);
        }
        state = ProvidedObjectImpl::REMOVED;
    }
}

ProvidedObjectImpl::State ProvidedObjectImpl::GetState()
{
    return state;
}

QStatus ProvidedObjectImpl::EmitSignal(const ajn::InterfaceDescription::Member& signal,
                                       const ajn::MsgArg* args,
                                       size_t numArgs)
{
    QStatus status = ER_FAIL;
    if (ProvidedObjectImpl::REGISTERED != state) {
        /* Actually BusObject::Signal() should return a proper error message, but it does not.
         * After two failed attempts to fix this at BusObject level
         * (1st attempt: by setting isRegistered as a check in BusObject::Signal() --> lead to race condition
         *  2nd attempt: by setting bus = NULL in BusObject::ObjectUnregistered() --> lead to assert in AJN unit test )
         * we decided to fix it here for now */
        QCC_LogError(ER_BUS_OBJECT_NOT_REGISTERED, ("Cannot signal on an object that is not exposed on the bus"));
        return ER_BUS_OBJECT_NOT_REGISTERED;
    }

    std::shared_ptr<ObjectAdvertiserImpl> advertiser = objectAdvertiserImpl.lock();
    if (advertiser) {
        QCC_DbgPrintf(("Emitting signal '%s' in interface '%s' for bus object at path '%s'",
                       signal.name.c_str(), signal.iface->GetName(), GetPath()));
        status = Signal(NULL, 0, signal, args, numArgs, 0, ajn::ALLJOYN_FLAG_GLOBAL_BROADCAST);
        if (status != ER_OK) {
            QCC_LogError(status, ("Failed to emit signal '%s' in interface '%s' for bus object at path '%s'",
                                  signal.name.c_str(), signal.iface->GetName(), GetPath()));
        }
    }
    return status;
}

QStatus ProvidedObjectImpl::MethodReply(const ajn::Message& msg,
                                        const ajn::MsgArg* args,
                                        size_t numArgs)
{
    return BusObject::MethodReply(msg, args, numArgs);
}

QStatus ProvidedObjectImpl::MethodReplyError(const ajn::Message& msg,
                                             const char* error,
                                             const char* errorMessage)
{
    return BusObject::MethodReply(msg, error, errorMessage);
}

QStatus ProvidedObjectImpl::MethodReplyErrorCode(const ajn::Message& msg,
                                                 QStatus status)
{
    return BusObject::MethodReply(msg, status);
}

const std::vector<qcc::String>& ProvidedObjectImpl::GetInterfaceNames() const
{
    return interfaceNames;
}

void ProvidedObjectImpl::AddInterfaceName(const qcc::String& name)
{
    interfaceNames.push_back(name);
}

const char* ProvidedObjectImpl::GetPath() const
{
    ProvidedObjectImpl* po = const_cast<ProvidedObjectImpl*>(this);       /* We have to const_cast because BusObject::GetPath() does not have const qualifier */
    return po->BusObject::GetPath();
}

QStatus ProvidedObjectImpl::Get(const char* ifcName, const char* propName, ajn::MsgArg& val)
{
    return providedObject.Get(ifcName, propName, val);
}

QStatus ProvidedObjectImpl::Set(const char* ifcName, const char* propName, ajn::MsgArg& val)
{
    return providedObject.Set(ifcName, propName, val);
}

QStatus ProvidedObjectImpl::AddInterfaceToBus(const ajn::InterfaceDescription& iface)
{
    return BusObject::AddInterface(iface);
}

QStatus ProvidedObjectImpl::AddMethodHandlerToBus(const ajn::InterfaceDescription::Member* member,
                                                  ajn::MessageReceiver::MethodHandler handler,
                                                  void* context)
{
    return BusObject::AddMethodHandler(member, handler, context);
}

void ProvidedObjectImpl::SetRefCountedPtr(std::weak_ptr<ProvidedObjectImpl> obj)
{
    self = obj;
}
}
