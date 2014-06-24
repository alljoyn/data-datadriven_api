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

#include "BusConnectionImpl.h"
#include "ProviderCache.h"

#include <qcc/Debug.h>
#include <algorithm>
#define QCC_MODULE "DD_COMMON"

namespace datadriven {
using namespace std;

BusConnectionImpl::BusConnectionImpl() :
    status(ER_FAIL),
    // \todo provide sensible data or remove name argument
    consumerSessionManager("test"),
    providerSessionManager("test", this),
    providerAsync(this, true),
    consumerAsync(this, true)
{
    do {
        consumerAsync.AsyncTaskQueue::Start();
        status = consumerSessionManager.GetStatus();
        if (status != ER_OK) {
            QCC_LogError(status, ("Failed to start consumer session manager"));
            break;
        }

        providerAsync.AsyncTaskQueue::Start();
        status = providerSessionManager.GetStatus();
        if (status != ER_OK) {
            QCC_LogError(status, ("Failed to start provider session manager"));
            break;
        }
    } while (0);
}

BusConnectionImpl::~BusConnectionImpl()
{
    providerAsync.Stop();
    consumerAsync.Stop();

    // We use lock here in case an Async stask was started before we stopped the queue
    providersMutex.Lock();

    // Remove providers and related busObjects and cached data
    std::vector<const ProvidedObject*>::const_iterator objIt = providers.begin();
    std::vector<const ProvidedObject*>::const_iterator objIt_end = providers.end();

    for (; objIt != objIt_end; ++objIt) {
        ProvidedObject* providedObject = const_cast<ProvidedObject*>(*objIt);

        // Remove associated busObject from bus
        providerSessionManager.RemoveBusObject(*providedObject);
    }
    providers.clear();
    providersMutex.Unlock();

    observersMutex.Lock();
    // Remove observers and signal listeners
    map<const ObserverImpl*, SignalListenerPerMemberMap>::const_iterator it = observers.begin();
    map<const ObserverImpl*, SignalListenerPerMemberMap>::const_iterator it_end = observers.end();

    for (; it != it_end; ++it) {
        const SignalListenerPerMemberMap& members = it->second;
        SignalListenerPerMemberMap::const_iterator membersIt = members.begin();
        SignalListenerPerMemberMap::const_iterator end = members.end();
        for (; membersIt != end; ++membersIt) {
            SignalListenerImpl* signalListener = membersIt->second;
            ajn::BusAttachment& busAttachment = consumerSessionManager.GetBusAttachment();
            busAttachment.UnregisterSignalHandler(static_cast<ajn::MessageReceiver*>(signalListener),
                                                  signalListener->GetHandler(),
                                                  signalListener->GetMember(),
                                                  NULL);
        }
    }
    observers.clear();
    observersMutex.Unlock();
}

QStatus BusConnectionImpl::GetStatus() const
{
    return status;
}

ConsumerSessionManager& BusConnectionImpl::GetConsumerSessionManager()
{
    return consumerSessionManager;
}

ProviderSessionManager& BusConnectionImpl::GetProviderSessionManager()
{
    return providerSessionManager;
}

void BusConnectionImpl::RegisterTypeDescription(const RegisteredTypeDescription* typedesc)
{
    if (ER_OK != status || typedesc == NULL) {
        QCC_LogError(status, ("Failed to register type description"));
        return;
    }

    qcc::String intfname = typedesc->GetDescription().GetName();
    map<qcc::String, const RegisteredTypeDescription*>::iterator it = typedescs.find(intfname);
    if (it == typedescs.end()) {
        typedescs[intfname] = typedesc;
    }
}

ProviderCache* BusConnectionImpl::GetProviderCache(const qcc::String& intfname)
{
    map<qcc::String, std::unique_ptr<ProviderCache> >::iterator it = caches.find(intfname);
    if (it == caches.end()) {
        caches[intfname] =
            std::unique_ptr<ProviderCache>(new ProviderCache(*typedescs[intfname], providerSessionManager));
    }
    return caches[intfname].get();
}

void BusConnectionImpl::ProviderAsyncEnqueue(const BusConnTaskData* bctd)
{
    providerAsync.Enqueue(bctd);
}

void BusConnectionImpl::ConsumerAsyncEnqueue(const BusConnTaskData* bctd)
{
    consumerAsync.Enqueue(bctd);
}

/* ajn::services::AsyncTask */
void BusConnectionImpl::OnEmptyQueue()
{
}

void BusConnectionImpl::OnTask(ajn::services::TaskData const* taskdata)
{
    const BusConnTaskData* ot  = static_cast<const BusConnTaskData*>(taskdata);
    ot->Execute(ot);
}

void BusConnectionImpl::AddObserver(const ObserverImpl* observer)
{
    observersMutex.Lock();
    map<const ObserverImpl*, SignalListenerPerMemberMap>::iterator it = observers.find(observer);
    if (it == observers.end()) {
        observers[observer] = SignalListenerPerMemberMap();
    }
    observersMutex.Unlock();
}

void BusConnectionImpl::RemoveObserver(const ObserverImpl* observer)
{
    observersMutex.Lock();
    map<const ObserverImpl*, SignalListenerPerMemberMap>::iterator it = observers.find(observer);
    if (it != observers.end()) {
        SignalListenerPerMemberMap& members = observers[observer];
        SignalListenerPerMemberMap::const_iterator membersIt = members.begin();
        SignalListenerPerMemberMap::const_iterator end = members.end();
        for (; membersIt != end; ++membersIt) {
            SignalListenerImpl* signalListener = membersIt->second;

            ajn::BusAttachment& busAttachment = consumerSessionManager.GetBusAttachment();
            busAttachment.UnregisterSignalHandler(static_cast<ajn::MessageReceiver*>(signalListener),
                                                  signalListener->GetHandler(),
                                                  signalListener->GetMember(),
                                                  NULL);
        }

        // Remove observer
        observers.erase(it);
    }
    observersMutex.Unlock();
}

QStatus BusConnectionImpl::AddSignalListener(SignalListenerImpl* signalListener)
{
    QStatus result = ER_FAIL;

    observersMutex.Lock();
    map<const ObserverImpl*, SignalListenerPerMemberMap>::iterator it = observers.find(signalListener->GetObserver());
    if (it != observers.end()) {
        SignalListenerPerMemberMap::iterator membersIt = it->second.find(signalListener->GetMember());

        if (membersIt == it->second.end()) {
            // Register with Bus Attachement
            result = consumerSessionManager.GetBusAttachment().RegisterSignalHandler(
                static_cast<ajn::MessageReceiver*>(signalListener),
                signalListener->GetHandler(),
                signalListener->GetMember(),
                NULL);
            if (ER_OK == result) {
                // Add the new listener to the map
                it->second[signalListener->GetMember()] = signalListener;
            }
        }
    }
    observersMutex.Unlock();

    return result;
}

QStatus BusConnectionImpl::RemoveSignalListener(SignalListenerImpl* signalListener)
{
    observersMutex.Lock();
    map<const ObserverImpl*, SignalListenerPerMemberMap>::iterator it = observers.find(signalListener->GetObserver());
    if (it != observers.end()) {
        SignalListenerPerMemberMap::iterator membersIt = it->second.find(signalListener->GetMember());

        if (membersIt != it->second.end()) {
            if (signalListener == membersIt->second) {
                // Unregister from Bus Attachement
                consumerSessionManager.GetBusAttachment().UnregisterSignalHandler(
                    static_cast<ajn::MessageReceiver*>(signalListener),
                    signalListener->GetHandler(),
                    signalListener->GetMember(),
                    NULL);
                // Remove it from the map
                it->second.erase(membersIt);
            }
        }
    }
    observersMutex.Unlock();

    return ER_OK;
}

QStatus BusConnectionImpl::LockForSignalListener(SignalListenerImpl* signalListener)
{
    QStatus result = ER_FAIL;

    observersMutex.Lock();
    map<const ObserverImpl*, SignalListenerPerMemberMap>::iterator it = observers.find(signalListener->GetObserver());
    if (it != observers.end()) {
        SignalListenerPerMemberMap::iterator membersIt = it->second.find(signalListener->GetMember());

        if (membersIt != it->second.end()) {
            if (signalListener == membersIt->second) {
                result = ER_OK;
            }
        }
    }

    if (ER_OK != result) {
        // Provided listener not present, remove lock
        observersMutex.Unlock();
    }

    return result;
}

QStatus BusConnectionImpl::LockForObserver(ObserverImpl* observer)
{
    QStatus result = ER_FAIL;

    observersMutex.Lock();
    map<const ObserverImpl*, SignalListenerPerMemberMap>::iterator it = observers.find(observer);
    if (it != observers.end()) {
        result = ER_OK;
    } else {
        // Observer not present, remove lock
        observersMutex.Unlock();
    }

    return result;
}

void BusConnectionImpl::UnlockObserver()
{
    observersMutex.Unlock();
}

QStatus BusConnectionImpl::AddProvidedObject(ProvidedObject* object)
{
    QStatus result = ER_FAIL;
    providersMutex.Lock();

    std::vector<const ProvidedObject*>::iterator objIt = std::find(providers.begin(), providers.end(), object);
    if (objIt == providers.end()) {
        result = providerSessionManager.AdvertiseBusObject(*object);
        if (ER_OK == result) {
            providers.push_back(object);
        }
    }
    providersMutex.Unlock();

    return result;
}

void BusConnectionImpl::RemoveProvidedObject(ProvidedObject* object)
{
    providersMutex.Lock();

    std::vector<const ProvidedObject*>::iterator objIt = std::find(providers.begin(), providers.end(), object);
    if (objIt != providers.end()) {
        QStatus result = providerSessionManager.RemoveBusObject(*object);

        if (ER_OK != result) {
            QCC_LogError(ER_FAIL, ("Remove of object from bus failed!"));
        }

        std::vector<const ProvidedInterface*>::iterator it = object->interfaces.begin();
        std::vector<const ProvidedInterface*>::iterator it_end = object->interfaces.end();
        for (; it != it_end; ++it) {
            ProvidedInterface* intf = const_cast<ProvidedInterface*>(*it);
            ProviderCache* cache = GetProviderCache(intf->GetTypeDescription()->GetName());
            if (NULL != cache) {
                cache->Remove(object);
            } else {
                QCC_LogError(ER_FAIL, ("Invalid cache!"));
            }
        }
        providers.erase(objIt);
    }
    providersMutex.Unlock();
}

QStatus BusConnectionImpl::LockForProvidedObject(ProvidedObject* object)
{
    QStatus result = ER_FAIL;
    providersMutex.Lock();

    std::vector<const ProvidedObject*>::iterator objIt = std::find(providers.begin(), providers.end(), object);
    if (objIt != providers.end()) {
        result = ER_OK;
    }

    if (ER_OK != result) {
        // Provided object not present, remove lock
        providersMutex.Unlock();
    }

    return result;
}

void BusConnectionImpl::UnlockProvidedObject()
{
    providersMutex.Unlock();
}
}
