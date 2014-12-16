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

#include <algorithm>

#include <datadriven/ObserverBase.h>
#include <datadriven/SignalListener.h>
#include "RegisteredTypeDescription.h"

#include "BusConnectionImpl.h"
#include "ObserverManager.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
class ObserverBase::ObserverTask :
    public ObserverManager::Task {
  public:
    ObserverTask(std::weak_ptr<ObserverBase> _observer,
                 const ObjectId& objId,
                 const ajn::MsgArg& changedProps,
                 const ajn::MsgArg& invalidatedProps) :
        observerBase(_observer),
        id(objId),
        changedProps(changedProps),
        invalidatedProps(invalidatedProps)
    { }

    virtual ~ObserverTask()
    { }

    void Execute() const
    {
        std::shared_ptr<ObserverBase> obs = observerBase.lock();
        if (obs) {
            obs->UpdateObject(id, &changedProps, &invalidatedProps);
        }
    }

  private:
    std::weak_ptr<ObserverBase> observerBase;
    ObjectId id;
    ajn::MsgArg changedProps;
    ajn::MsgArg invalidatedProps;
};

ObserverBase::ObserverBase(const TypeDescription& typeDesc,
                           ajn::BusAttachment* ba) :
    status(ER_FAIL), busConnectionImpl(BusConnectionImpl::GetInstance(ba))
{
    do {
        if (ER_OK != busConnectionImpl->GetStatus()) {
            QCC_LogError(status, ("BusConnection is not usable"));
            break;
        }

        status = RegisteredTypeDescription::RegisterInterface(
            busConnectionImpl->GetBusAttachment(), typeDesc, registeredTypeDesc);
        if (ER_OK != status) {
            QCC_LogError(status, ("Failed to register interface"));
            break;
        }

        observerMgr = ObserverManager::GetInstance(busConnectionImpl);
    } while (0);
}

ObserverBase::~ObserverBase()
{
    qcc::String ifName = registeredTypeDesc->GetDescription().GetName();
    // Remove the observer from the cache and destroy the cache if this was the last
    // observer for the related proxy interface
    observerMgr->UnregisterObserver(observerBase, ifName);
    // clean out any signal handlers
    signalListenersMutex.Lock();
    for (SignalListenerMap::const_iterator it = signalListeners.begin();
         it != signalListeners.end(); it++) {
        SignalListenerBase* listener = it->second;
        busConnectionImpl->GetBusAttachment().UnregisterSignalHandler(static_cast<ajn::MessageReceiver*>(listener),
                                                                      listener->GetHandler(), listener->GetMember(),
                                                                      NULL);
    }
    signalListenersMutex.Unlock();
}

QStatus ObserverBase::SetRefCountedPtr(std::weak_ptr<ObserverBase> observer)
{
    observerBase = observer;
    qcc::String ifName = registeredTypeDesc->GetDescription().GetName();
    return observerMgr->RegisterObserver(observerBase, ifName);
}

size_t ObserverBase::Size()
{
    std::shared_ptr<ObserverCache> cache = observerMgr->GetCache(registeredTypeDesc->GetDescription().GetName());
    return cache->LivingObjects().size();
}

QStatus ObserverBase::AddSignalListener(SignalListenerBase* listener,
                                        int memberNumber)
{
    signalListenersMutex.Lock();
    const ajn::InterfaceDescription::Member* member = &registeredTypeDesc->GetMember(memberNumber);
    listener->SetObserver(observerBase);
    listener->SetMember(member);
    SignalListenerMap::iterator it = signalListeners.find(member);
    if (it == signalListeners.end()) {
        // Register with Bus Attachment
        ajn::BusAttachment& bus = busConnectionImpl->GetBusAttachment();
        status = bus.RegisterSignalHandler(static_cast<ajn::MessageReceiver*>(listener),
                                           listener->GetHandler(), member, NULL);
        if (ER_OK == status) {
            // Add the new listener to the map
            signalListeners[member] = listener;
        }
    }
    signalListenersMutex.Unlock();
    return status;
}

QStatus ObserverBase::RemoveSignalListener(SignalListenerBase* listener)
{
    signalListenersMutex.Lock();
    SignalListenerMap::iterator it = signalListeners.find(listener->GetMember());
    if (it != signalListeners.end()) {
        if (listener == it->second) {
            // Unregister from Bus Attachment
            ajn::BusAttachment& bus = busConnectionImpl->GetBusAttachment();
            status = bus.UnregisterSignalHandler(static_cast<ajn::MessageReceiver*>(listener),
                                                 listener->GetHandler(), listener->GetMember(), NULL);
            // Remove it from the map
            signalListeners.erase(it);
        }
    }
    signalListenersMutex.Unlock();
    return status;
}

void ObserverBase::HandleSignal(SignalListenerBase* listener,
                                const ajn::Message& message)
{
    signalListenersMutex.Lock();
    SignalListenerMap::iterator it = signalListeners.find(listener->GetMember());
    if ((it != signalListeners.end()) && (listener == it->second)) {
        listener->SignalHandler(message);
    }
    signalListenersMutex.Unlock();
}

/** Return observer construction status */
QStatus ObserverBase::GetStatus() const
{
    return status;
}

const RegisteredTypeDescription& ObserverBase::GetRegisteredTypeDescription() const
{
    return *registeredTypeDesc;
}

void ObserverBase::PropertiesChanged(ajn::ProxyBusObject& obj,
                                     const char* ifaceName,
                                     const ajn::MsgArg& changed,
                                     const ajn::MsgArg& invalidated,
                                     void* context)
{
    QCC_DbgPrintf(("ObserverBase: got PropertyChanged signal from '%s' in '%s' (changed: %d, invalidated: %d)",
                   obj.GetPath().c_str(), obj.GetServiceName().c_str()));

    ObjectId objectId(busConnectionImpl->GetBusAttachment(), obj.GetServiceName(), obj.GetPath(), obj.GetSessionId());
    ObserverTask* task = new ObserverTask(observerBase, objectId, changed, invalidated);
    observerMgr->Enqueue(task);
}

std::shared_ptr<BusConnectionImpl> ObserverBase::GetBusConnection() const
{
    return busConnectionImpl;
}

void ObserverBase::UpdateObject(const ObjectId& objId,
                                const ajn::MsgArg* changedProps,
                                const ajn::MsgArg* invalidatedProps)
{
    std::shared_ptr<ProxyInterface> objProxy = nullptr;
    std::shared_ptr<ObserverCache> cache = observerMgr->GetCache(registeredTypeDesc->GetDescription().GetName());

    if (cache != nullptr) {
        QCC_DbgPrintf(("Update object (%s, %s)", objId.GetBusName().c_str(), objId.GetBusObjectPath().c_str()));
        objProxy = cache->UpdateObject(objId, changedProps);
        if (nullptr == objProxy) {
            QCC_LogError(ER_FAIL, ("Observer => UpdateObject: Failed to get proxy object"));
        }
    }
    // TODO handle invalidated props
}

ObjectId* ObserverBase::GetObjectId(ajn::Message message)
{
    return new ObjectId(busConnectionImpl->GetBusAttachment(), message);
}

std::shared_ptr<ProxyInterface> ObserverBase::GetObject(const ObjectId& objId)
{
    std::shared_ptr<ObserverCache> cache = observerMgr->GetCache(registeredTypeDesc->GetDescription().GetName());
    std::shared_ptr<ProxyInterface> proxyObj = nullptr;
    if (nullptr != cache) {
        proxyObj = cache->GetObject(objId);
    }
    return proxyObj;
}

std::vector<std::shared_ptr<ProxyInterface> > ObserverBase::GetObjects() const
{
    std::shared_ptr<ObserverCache> cache = observerMgr->GetCache(registeredTypeDesc->GetDescription().GetName());
    return cache->LivingObjects();
}
}
