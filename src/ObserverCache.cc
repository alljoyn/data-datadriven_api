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

#include <algorithm>
#include <memory>

#include <datadriven/ObjectAllocator.h>
#include <datadriven/ObserverBase.h>

#include "ObserverCache.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
// PUBLIC //
ObserverCache::ObserverCache(const qcc::String ifName,
                             std::weak_ptr<ObjectAllocator> alloc) :
    ifName(ifName), allocator(alloc)
{
}

ObserverCache::~ObserverCache()
{
    mutex.Lock();
    if (0 != observers.size()) {
        QCC_DbgPrintf(("The observer list was not empty"));
        observers.clear();
    }
    mutex.Unlock();
}

void ObserverCache::AddObserver(std::weak_ptr<ObserverBase> observer)
{
    QCC_DbgPrintf(("Add observer to cache for interface %s", ifName.c_str()));
    mutex.Lock();
    ObserverSet::iterator found = observers.find(observer);
    if (found == observers.end()) {
        observers.insert(observer);
    }
    mutex.Unlock();
}

size_t ObserverCache::RemoveObserver(std::weak_ptr<ObserverBase> observer)
{
    mutex.Lock();
    ObserverSet::iterator found = observers.find(observer);
    if (found != observers.end()) {
        QCC_DbgPrintf(("Remove observer from cache for interface %s", ifName.c_str()));
        observers.erase(found);
    }
    size_t size = observers.size();
    mutex.Unlock();
    return size;
}

void ObserverCache::NotifyObserver(std::weak_ptr<ObserverBase> observer)
{
    QCC_DbgPrintf(("Notify observer about objects in cache for interface %s", ifName.c_str()));
    mutex.Lock();
    std::shared_ptr<ObserverBase> obs = observer.lock();
    if (nullptr != obs) {
        for (ObjectIdToSharedPtrMap::iterator iterator = livingObjects.begin(); iterator != livingObjects.end();
             iterator++) {
            mutex.Unlock();
            obs->UpdateObject(iterator->second);
            mutex.Lock();
            iterator = livingObjects.lower_bound(iterator->first);
        }
    }
    mutex.Unlock();
}

void ObserverCache::NotifyObjectExistence(std::shared_ptr<ProxyInterface> proxyObj, bool add,
                                          ObserverSet notifiedObservers)
{
    if (nullptr != proxyObj) {
        // Notify All observers
        for (ObserverCache::ObserverSet::const_iterator vectorIterator = notifiedObservers.begin();
             vectorIterator != notifiedObservers.end();
             vectorIterator++) {
            std::shared_ptr<ObserverBase> observer = (*vectorIterator).lock();
            if (observer) {
                if (add) {
                    observer->AddObject(proxyObj);
                } else {
                    observer->RemoveObject(proxyObj);
                }
            }
        }
    }
}

ObserverCache::NotificationSet ObserverCache::AddObject(const ObjectId& objId)
{
    std::shared_ptr<ProxyInterface> proxyObj;
    mutex.Lock();
    ObjectIdToSharedPtrMap::iterator aliveIterator = livingObjects.find(objId);
    if (aliveIterator != livingObjects.end()) {
        proxyObj = aliveIterator->second;
        QCC_DbgPrintf(("Update object @%s, path = '%s', session = %lu",
                       objId.GetBusName().c_str(), objId.GetBusObjectPath().c_str(),
                       (unsigned long)objId.GetSessionId()));
    } else {
        ObjectIdToWeakPtrMap::iterator deadIterator = deadObjects.find(objId);
        if (deadIterator != deadObjects.end()) {
            /* We have to potentially resurrect the dead object (insert joke here :)) */
            proxyObj = deadIterator->second.lock();
            if (proxyObj) {
                /* object was still there: promote it */
                QCC_DbgPrintf(("Resurrect object @%s, path = '%s', session = %lu",
                               objId.GetBusName().c_str(), objId.GetBusObjectPath().c_str(),
                               (unsigned long)objId.GetSessionId()));
            } else {
                /* object was already gone */
                QCC_DbgPrintf(("Failed to lock weak ptr, path = '%s', session = %lu",
                               objId.GetBusName().c_str(), objId.GetBusObjectPath().c_str(),
                               (unsigned long)objId.GetSessionId()));
            }

            deadObjects.erase(deadIterator);
        } else {
            QCC_DbgPrintf(("There was no weak ptr @%s, pth = '%s', session = %lu",
                           objId.GetBusName().c_str(), objId.GetBusObjectPath().c_str(),
                           (unsigned long)objId.GetSessionId()));
        }

        if (nullptr == proxyObj) {
            std::shared_ptr<ObjectAllocator> alloc = allocator.lock();
            if (nullptr != alloc) {
                proxyObj = std::shared_ptr<ProxyInterface>(alloc->Alloc(objId));
            }
        }

        if (nullptr != proxyObj) {
            livingObjects.insert(aliveIterator, std::pair<ObjectId, std::shared_ptr<ProxyInterface> >(objId, proxyObj));
            proxyObj->SetAlive(true);
            QCC_DbgPrintf(("(Re-)add object @%s, path = '%s', session = %lu",
                           objId.GetBusName().c_str(), objId.GetBusObjectPath().c_str(),
                           (unsigned long)objId.GetSessionId()));
        }
    }
    NotificationSet snapshot = { proxyObj, GetObservers() };
    mutex.Unlock();

    return snapshot;
}

ObserverCache::NotificationSet ObserverCache::RemoveObject(const ObjectId& objId)
{
    /* We agreed we would not remove the object, but rather convert the strong reference to a weak reference. */

    mutex.Lock();
    ObjectIdToSharedPtrMap::iterator it = livingObjects.find(objId);
    NotificationSet snapshot = { nullptr, GetObservers() };
    if (it != livingObjects.end()) {
        std::weak_ptr<ProxyInterface> weak(it->second);
        std::shared_ptr<ProxyInterface> proxyObj = it->second;
        it->second->SetAlive(false);

        livingObjects.erase(it);
        deadObjects.insert(std::pair<ObjectId, std::weak_ptr<ProxyInterface> >(objId, weak));
        snapshot.interface = proxyObj;

        QCC_DbgPrintf(("Remove object @%s, path = '%s', session = %lu",
                       objId.GetBusName().c_str(), objId.GetBusObjectPath().c_str(),
                       (unsigned long)objId.GetSessionId()));
    }
    mutex.Unlock();
    GarbageCollect();         /* TODO: not always trigger this */
    return snapshot;
}

std::shared_ptr<ProxyInterface> ObserverCache::UpdateObject(const ObjectId& objId, const ajn::MsgArg* dict)
{
    mutex.Lock();
    std::shared_ptr<ProxyInterface> proxyObj = nullptr;
    QStatus status = ER_OK;
    ObjectIdToSharedPtrMap::iterator it = livingObjects.find(objId);
    if (it != livingObjects.end()) {
        proxyObj = it->second;
        proxyObj->UpdateProperties(dict);
        status = proxyObj->GetStatus();
        if (ER_OK != status) {
            QCC_LogError(ER_FAIL, ("UpdateObject: Failed to unmarshal properties"));
        }
    }
    mutex.Unlock();

    // Notify all observers about the change in proxy interface objects
    if (nullptr != proxyObj) {
        // Notify All observers
        for (ObserverCache::ObserverSet::const_iterator vectorIterator = observers.begin();
             vectorIterator != observers.end();
             vectorIterator++) {
            std::shared_ptr<ObserverBase> observer = (*vectorIterator).lock();
            if (observer) {
                if (ER_OK == status) {
                    observer->UpdateObject(proxyObj);
                } else {
                    /* Let the observer know something went wrong with the proxyObj */
                    observer->RemoveObject(proxyObj);
                }
            }
        }
        if (ER_OK != status) {
            /* Reset the proxyObj */
            proxyObj.reset();
        }
    } else {
        QCC_DbgPrintf(("Failed to find object @%s, path = '%s', session = %lu",
                       objId.GetBusName().c_str(), objId.GetBusObjectPath().c_str(),
                       (unsigned long)objId.GetSessionId()));
    }

    return proxyObj;
}

std::shared_ptr<ProxyInterface> ObserverCache::GetObject(const ObjectId& objId)
{
    mutex.Lock();
    ObjectIdToSharedPtrMap::iterator it = livingObjects.find(objId);
    if (it != livingObjects.end()) {
        mutex.Unlock();
        return it->second;
    }
    mutex.Unlock();

    QCC_DbgPrintf(("Failed to find object @%s, path = '%s', session = %lu",
                   objId.GetBusName().c_str(), objId.GetBusObjectPath().c_str(), (unsigned long)objId.GetSessionId()));

    return std::shared_ptr<ProxyInterface>();
}

std::vector<std::shared_ptr<ProxyInterface> > ObserverCache::LivingObjects() const
{
    std::vector<std::shared_ptr<ProxyInterface> > objects;
    objects.reserve(livingObjects.size());
    for (ObjectIdToSharedPtrMap::const_iterator objit = livingObjects.begin(); objit != livingObjects.end(); ++objit) {
        objects.push_back(objit->second);
    }

    return objects;
}

ObserverCache::ObserverSet ObserverCache::GetObservers() const
{
    return observers;
}

// PRIVATE //

bool ObserverCache::ObjectIdComp::operator()(const ObjectId& o1, const ObjectId& o2) const
{
    if (o1.GetBusName() != o2.GetBusName()) {
        return o1.GetBusName() < o2.GetBusName();
    }

    if (o1.GetBusObjectPath() != o2.GetBusObjectPath()) {
        return o1.GetBusObjectPath() < o2.GetBusObjectPath();
    }

    return false;
}

void ObserverCache::GarbageCollect()
{
    mutex.Lock();
    for (ObjectIdToWeakPtrMap::iterator deadit = deadObjects.begin(); deadit != deadObjects.end();) {
        if (deadit->second.expired()) {
            deadObjects.erase(deadit++);
        } else {
            ++deadit;
        }
    }
    mutex.Unlock();
}
}
