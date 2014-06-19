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

#include "ObserverCache.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
// PUBLIC //
ObserverCache::ObserverCache()
{
}

std::shared_ptr<ProxyInterface> ObserverCache::SetObject(const ObjectId& objId, ObjectAllocator& allocator)
{
    std::shared_ptr<ProxyInterface> livingObj;
    mutex.Lock();
    ObjectIdToSharedPtrMap::iterator aliveit = livingObjects.find(objId);
    if (aliveit != livingObjects.end()) {
        livingObj = aliveit->second;
        QCC_DbgPrintf(("Update object @%s, path = '%s', session = %lu",
                       objId.GetBusName().c_str(), objId.GetBusObjectPath().c_str(), (unsigned long)objId.GetSessionId()));
    } else {
        ObjectIdToWeakPtrMap::iterator deadit = deadObjects.find(objId);
        if (deadit != deadObjects.end()) {
            /* We have to potentially resurrect the dead object (insert joke here :)) */
            livingObj = deadit->second.lock();
            if (livingObj) {
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

            deadObjects.erase(deadit);
        } else {
            QCC_DbgPrintf(("There was no weak ptr @%s, pth = '%s', session = %lu",
                           objId.GetBusName().c_str(), objId.GetBusObjectPath().c_str(),
                           (unsigned long)objId.GetSessionId()));
        }

        if (!livingObj) {
            livingObj = std::shared_ptr<ProxyInterface>(allocator.Alloc(objId));
        }

        livingObjects.insert(aliveit, std::pair<ObjectId, std::shared_ptr<ProxyInterface> >(objId, livingObj));
        livingObj->SetAlive(true);
        QCC_DbgPrintf(("(Re-)add object @%s, path = '%s', session = %lu",
                       objId.GetBusName().c_str(), objId.GetBusObjectPath().c_str(), (unsigned long)objId.GetSessionId()));
    }

    mutex.Unlock();
    return livingObj;
}

std::shared_ptr<ProxyInterface> ObserverCache::RemoveObject(const ObjectId& objId)
{
    /* We agreed we would not remove the object, but rather convert the strong reference to a weak reference. */

    mutex.Lock();
    ObjectIdToSharedPtrMap::iterator it = livingObjects.find(objId);
    assert(it != livingObjects.end());

    std::weak_ptr<ProxyInterface> weak(it->second);
    std::shared_ptr<ProxyInterface> obj = it->second;
    it->second->SetAlive(false);

    livingObjects.erase(it);
    deadObjects.insert(std::pair<ObjectId, std::weak_ptr<ProxyInterface> >(objId, weak));

    mutex.Unlock();

    QCC_DbgPrintf(("Remove object @%s, path = '%s', session = %lu",
                   objId.GetBusName().c_str(), objId.GetBusObjectPath().c_str(), (unsigned long)objId.GetSessionId()));
    GarbageCollect();         /* TODO: not always trigger this */
    return obj;
}

std::shared_ptr<ProxyInterface> ObserverCache::Get(const ObjectId& objId)
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
