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

#include <datadriven/ObserverImpl.h>

#include <algorithm>

#include <datadriven/RegisteredTypeDescription.h>

#include "ConsumerSessionManagerImpl.h"
#include "BusConnectionImpl.h"
#include "ObserverCache.h"
#include <datadriven/SignalImpl.h>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
class ObserverImpl::RemoveTaskData :
    public BusConnectionImpl::BusConnTaskData {
  private:
    ObserverImpl* observerimpl;
    std::weak_ptr<BusConnectionImpl> busConnectionImpl;
    ObjectId id;

  public:
    RemoveTaskData(ObserverImpl* _observerimpl,
                   std::weak_ptr<BusConnectionImpl> _busConnectionImpl,
                   const ObjectId& objId) :
        observerimpl(_observerimpl), busConnectionImpl(_busConnectionImpl),
        id(objId.busAttachment, objId.busName, objId.busObjectPath, objId.sessionId) { }

    void Execute(const BusConnTaskData* ot) const
    {
        std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
        if (busConn) {
            const RemoveTaskData* rtd  = static_cast<const RemoveTaskData*>(ot);
            QStatus result = busConn->LockForObserver(rtd->observerimpl);

            if (ER_OK == result) {
                rtd->observerimpl->NotifyRemove(rtd->observerimpl->cache->RemoveObject(rtd->id));
                busConn->UnlockObserver();
            }
        }
    }
};

//@TODO this implementation is very very inefficient.
ObserverImpl::iterator::iterator(const ObserverImpl::iterator& i) :
    objects(i.objects), it(i.it)
{
}

ObserverImpl::iterator& ObserverImpl::iterator::operator=(const ObserverImpl::iterator& i)
{
    objects = i.objects;
    it = i.it;
    return *this;
}

ObserverImpl::iterator& ObserverImpl::iterator::operator++()
{
    ++it;
    return *this;
}

ObserverImpl::iterator ObserverImpl::iterator::operator++(int)
{
    iterator tmp = *this;
    ++*this;
    return tmp;
}

bool ObserverImpl::iterator::operator==(const ObserverImpl::iterator& i) const
{
    if (objects.size() == 0) {
        return (i.objects.size() == 0) || (i.objects.end() == i.it);
    }
    if (0 == i.objects.size()) {
        return (objects.end() == it);
    }
    return (it == i.it);
}

bool ObserverImpl::iterator::operator!=(const ObserverImpl::iterator& it) const
{
    return !(*this == it);      /* wouldn't it make sense to first compare the pointers ? */
}

const std::shared_ptr<ProxyInterface>& ObserverImpl::iterator::operator*()
{
    return *it;
}

ObserverImpl::iterator ObserverImpl::iterator::end()
{
    return iterator();
}

ObserverImpl::iterator::iterator(const ObserverImpl& observer) :
    objects(observer.cache->LivingObjects()), it(objects.begin())
{
}

ObserverImpl::iterator::iterator() :
    objects(), it(objects.end())
{
}

void ObserverImpl::Remove(const ObjectId& objectId)
{
    std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
    if (busConn) {
        busConn->ConsumerAsyncEnqueue(new RemoveTaskData(this,
                                                         busConnectionImpl,
                                                         objectId));
    }
}

class ObserverListener :
    public ConsumerInterfaceListener {
  public:
    virtual void OnNew(ConsumerSessionManager::ObserverRef rr, const ObjectId& objId)
    {
        //ILB because we deal with this through the OnUpdate signals now...
    }

    virtual void OnRemove(ConsumerSessionManager::ObserverRef rr, const ObjectId& objId)
    {
        static_cast<ObserverImpl*>(rr)->Remove(objId);
    }
};

ObserverImpl::ObserverImpl(BusConnection& busConnection,
                           const TypeDescription& typeDesc) :
    status(ER_FAIL), busConnectionImpl(busConnection.busConnectionImpl), cache(new ObserverCache()),
    observerListener(new ObserverListener)
{
    std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
    if (busConn) {
        ConsumerSessionManager& Mgr = busConn->GetConsumerSessionManager();
        do {
            status = RegisteredTypeDescription::RegisterInterface(Mgr.GetBusAttachment(), typeDesc, iface);

            if (ER_OK != status) {
                QCC_LogError(status, ("Failed to register interface"));
                break;
            }

            status = Mgr.RegisterObserver(this, typeDesc.GetName());

            if (ER_OK != status) {
                QCC_LogError(status, ("Failed to register observer"));
                break;
            }

            Mgr.RegisterInterfaceListener(&*observerListener);
            busConn->AddObserver(this);
        } while (0);
    } else {
        QCC_LogError(status, ("Bus connection invalid !"));
    }
}

ObserverImpl::~ObserverImpl()
{
    std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
    if (busConn) {
        busConn->RemoveObserver(this);

        ConsumerSessionManager& consumerSessionManager = busConn->GetConsumerSessionManager();
        consumerSessionManager.RegisterInterfaceListener(NULL);

        QStatus resp = consumerSessionManager.UnregisterObserver(this);
        if (ER_OK != resp) {
            QCC_LogError(resp, ("Failed to unregister observer"));
        }
    }
}

QStatus ObserverImpl::AddSignalListener(SignalListenerImpl* l)
{
    QStatus result = ER_FAIL;
    std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
    if (busConn) {
        result = busConn->AddSignalListener(l);
    }
    return result;
}

QStatus ObserverImpl::RemoveSignalListener(SignalListenerImpl* l)
{
    QStatus result = ER_FAIL;
    std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
    if (busConn) {
        result = busConn->RemoveSignalListener(l);
    }
    return result;
}

/** Return observer construction status */
QStatus ObserverImpl::GetStatus() const
{
    return status;
}

std::shared_ptr<ProxyInterface> ObserverImpl::Get(const ObjectId& id)
{
    return cache->Get(id);
}

struct ObserverAllocator :
    public ObserverCache::ObjectAllocator {
    virtual datadriven::ProxyInterface* Alloc(const ObjectId& objId)
    {
        return observer.Alloc(objId);
    }

    ObserverAllocator(const ObserverImpl& observer) :
        observer(observer) { };
    const ObserverImpl& observer;
};

std::shared_ptr<ProxyInterface> ObserverImpl::Get(const::ajn::Message& message)
{
    std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
    if (busConn) {
        ajn::BusAttachment& busAttachment = busConn->GetConsumerSessionManager().GetBusAttachment();
        ObserverAllocator allocator(*this);
        return cache->SetObject(ObjectId(busAttachment, message), allocator);
    }
    return std::shared_ptr<ProxyInterface>();
}

const RegisteredTypeDescription& ObserverImpl::GetRegisteredTypeDescription() const
{
    return *iface;
}

void ObserverImpl::Notify(const std::shared_ptr<ProxyInterface>& obj, Listener::CallBack cb) const
{
    if (!interfaceListeners.empty()) {
        std::vector<Listener*>::const_iterator it = interfaceListeners.begin();
        std::vector<Listener*>::const_iterator end = interfaceListeners.end();
        for (; it != end; ++it) {
            cb(**it, obj);
        }
    }
}

QStatus ObserverImpl::AddListener(ObserverImpl::Listener& l)
{
    QStatus result = ER_FAIL;

    listenersMutex.Lock();
    do {
        if (std::find(interfaceListeners.begin(), interfaceListeners.end(), &l) != interfaceListeners.end()) {
            // Listener is already present: prevent double entries
            break;
        }
        interfaceListeners.push_back(&l);
        result = ER_OK;
    } while (0);
    listenersMutex.Unlock();
    return result;
}

QStatus ObserverImpl::RemoveListener(ObserverImpl::Listener& l)
{
    std::vector<Listener*>::iterator it;

    listenersMutex.Lock();
    it = std::find(interfaceListeners.begin(), interfaceListeners.end(), &l);
    if (it != interfaceListeners.end()) {
        interfaceListeners.erase(it);
    }
    listenersMutex.Unlock();
    return ER_OK;
}
}
