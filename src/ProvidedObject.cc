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
#include <datadriven/BusConnection.h>
#include "BusConnectionImpl.h"
#include <qcc/GUID.h>
#include "WriterCache.h"
#include "ProviderSessionManager.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

namespace datadriven {
class ProvidedObject::MethodHandlerTaskData :
    public BusConnectionImpl::BusConnTaskData {
  private:
    std::weak_ptr<BusConnectionImpl> busConnectionImpl;
    ProvidedObject* provider;
    ajn::MessageReceiver* ctx;
    ajn::MessageReceiver::MethodHandler handler;
    const ajn::InterfaceDescription::Member* member;
    ajn::Message message;

  public:
    MethodHandlerTaskData(std::weak_ptr<BusConnectionImpl> _busConnectionImpl,
                          ProvidedObject* _provider,
                          ajn::MessageReceiver* _ctx,
                          ajn::MessageReceiver::MethodHandler _handler,
                          const ajn::InterfaceDescription::Member* _member,
                          ajn::Message& _message) :
        busConnectionImpl(_busConnectionImpl), provider(_provider), ctx(_ctx), handler(_handler), member(_member),
        message(_message) { }

    virtual void Execute(const BusConnTaskData* td) const
    {
        std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
        if (busConn) {
            const MethodHandlerTaskData* mhtd = static_cast<const MethodHandlerTaskData*>(td);
            QStatus result = busConn->LockForProvidedObject(mhtd->provider);

            if (ER_OK == result) {
                (mhtd->ctx->*mhtd->handler)(mhtd->member, const_cast<ajn::Message&>(mhtd->message));
                busConn->UnlockProvidedObject();
            }
        }
    }
};

ProvidedObject::ProvidedObject(BusConnection& busConnection,
                               const qcc::String& path) :
    BusObject(path.c_str()), busConnectionImpl(busConnection.busConnectionImpl), state(ProvidedObject::CONSTRUCTED)
{
}

ProvidedObject::ProvidedObject(BusConnection& busConnection) :
    BusObject(GeneratePath().c_str()), busConnectionImpl(busConnection.busConnectionImpl),
    state(ProvidedObject::CONSTRUCTED)
{
}

ProvidedObject::~ProvidedObject()
{
    if (state == ProvidedObject::REGISTERED) {
        std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
        if (busConn) {
            busConn->RemoveProvidedObject(this);
        }
        state = ProvidedObject::REMOVED;
    }
}

qcc::String ProvidedObject::GeneratePath()
{
    return "/O" + qcc::GUID128().ToString();
}

void ProvidedObject::Register()
{
    if (ProvidedObject::REGISTERED != state) {
        std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
        if (busConn) {
            QStatus result = busConn->AddProvidedObject(this);

            if (ER_OK == result) {
                state = ProvidedObject::REGISTERED;
            } else {
                state = ProvidedObject::ERROR;
            }
        }
    }
}

void ProvidedObject::CallMethodHandler(ajn::MessageReceiver::MethodHandler handler,
                                       const ajn::InterfaceDescription::Member* member,
                                       ajn::Message& message,
                                       void* context)
{
    ajn::MessageReceiver* ctxObject = static_cast<ajn::MessageReceiver*>(context);
    std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
    if (busConn) {
        busConn->ProviderAsyncEnqueue(new MethodHandlerTaskData(busConnectionImpl,
                                                                this,
                                                                ctxObject,
                                                                handler,
                                                                member,
                                                                message));
    }
}

void ProvidedObject::RemoveFromBus()
{
    if (state == ProvidedObject::REGISTERED) {
        std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
        if (busConn) {
            busConn->RemoveProvidedObject(this);
        }
        state = ProvidedObject::REMOVED;
    }
}

ProvidedObject::State ProvidedObject::GetState()
{
    return state;
}

QStatus ProvidedObject::EmitSignal(const ajn::InterfaceDescription::Member& signal,
                                   const ajn::MsgArg* args,
                                   size_t numArgs)
{
    QStatus status = ER_FAIL;
    if (ProvidedObject::REGISTERED != state) {
        /* Actually BusObject::Signal() should return a proper error message, but it does not.
         * After two failed attempts to fix this at BusObject level
         * (1st attempt: by setting isRegistered as a check in BusObject::Signal() --> lead to race condition
         *  2nd attempt: by setting bus = NULL in BusObject::ObjectUnregistered() --> lead to assert in AJN unit test )
         * we decided to fix it here for now */
        QCC_LogError(ER_BUS_OBJECT_NOT_REGISTERED, ("Cannot signal on an object that is not exposed on the bus"));
        return ER_BUS_OBJECT_NOT_REGISTERED;
    }

    std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
    if (busConn) {
        ProviderSessionManager& providerSessionManager = busConn->GetProviderSessionManager();
        if (ER_OK != providerSessionManager.GetStatus()) {
            QCC_LogError(ER_FAIL, ("Trying to emit signal with an improperly initialized provider session manager"));
            return ER_FAIL;
        }
        status = providerSessionManager.BusObjectSignal(*this, signal.iface->GetName(), &signal, args, numArgs);
        if (ER_OK != status) {
            QCC_LogError(status, ("Failed to emit signal"));
        }
    }
    return status;
}

QStatus ProvidedObject::MethodReply(const ajn::Message& msg,
                                    const ajn::MsgArg* args,
                                    size_t numArgs)
{
    return BusObject::MethodReply(msg, args, numArgs);
}

QStatus ProvidedObject::AddProvidedInterface(ProvidedInterface* providedInterface,
                                             MethodCallbacks handlers[],
                                             size_t numHandlers)
{
    QStatus status = ER_OK;

    if (NULL == providedInterface) {
        status = ER_BAD_ARG_1;
        QCC_LogError(status, ("Invalid arguments"));
        return status;
    }

    std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
    if (busConn) {
        ProviderSessionManager& providerSessionManager = busConn->GetProviderSessionManager();
        if (ER_OK != providerSessionManager.GetStatus()) {
            status = ER_FAIL;
            QCC_LogError(status, ("Provider session manager not properly initialized"));
            return status;
        }

        status = providedInterface->RegisterInterface(providerSessionManager.GetBusAttachment());
        const RegisteredTypeDescription* reg = providedInterface->GetRegisteredTypeDescription();
        if (status == ER_OK) {
            status = BusObject::AddInterface(reg->GetInterfaceDescription());
            if (status == ER_OK) {
                interfaces.push_back(providedInterface);
            }
            busConn->RegisterTypeDescription(reg);
        }

        if (ER_OK == status) {
            for (size_t i = 0; (ER_OK == status) && (i < numHandlers); ++i) {
                MethodCallbacks& mh = handlers[i];
                status = BusObject::AddMethodHandler(&reg->GetMember(mh.memberId), mh.handler, providedInterface);
            }
        }
    }
    if (ER_OK != status) {
        QCC_LogError(status, ("Failed to add published interface"));
    }
    return status;
}

void ProvidedObject::GetInterfaceNames(std::vector<qcc::String>& out) const
{
    for (std::vector<const ProvidedInterface*>::const_iterator it = interfaces.begin(); it != interfaces.end(); ++it) {
        out.push_back((*it)->GetTypeDescription()->GetName());
    }
}

const char* ProvidedObject::GetPath() const
{
    ProvidedObject* po = const_cast<ProvidedObject*>(this);       /* We have to const_cast because BusObject::GetPath() does not have const qualifier */
    return po->BusObject::GetPath();
}

std::weak_ptr<BusConnectionImpl> ProvidedObject::GetBusConnection()
{
    return busConnectionImpl;
}

QStatus ProvidedObject::UpdateAll()
{
    QStatus status = ER_OK;
    Register();
    if (ProvidedObject::ERROR == state) {
        QCC_LogError(ER_FAIL, ("Failed as the object is in error/unregistered state"));
        return ER_FAIL;
    }

    std::vector<const ProvidedInterface*>::iterator it = interfaces.begin();
    std::vector<const ProvidedInterface*>::iterator endit = interfaces.end();
    for (; it != endit; ++it) {
        ProvidedInterface* intf = const_cast<ProvidedInterface*>(*it);
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
}
