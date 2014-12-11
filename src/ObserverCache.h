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

#ifndef OBSERVERCACHE_H_
#define OBSERVERCACHE_H_

#include <map>
#include <memory>
#include <set>

#include <datadriven/ObjectId.h>
#include <datadriven/ProxyInterface.h>

namespace datadriven {
class ObserverBase;
class ObjectAllocator;

class ObserverCache {
  public:
    ObserverCache(const qcc::String ifName);

    ~ObserverCache();

    /**
     * Add a new \a observer to the cache
     *
     * \param observer the new observer
     */
    void AddObserver(std::weak_ptr<ObserverBase> observer);

    /**
     * Remove \a observer from the cache and return the new vector size
     *
     * \param observer the observer to be removed
     * \return the size of the vector after removing the observer
     */
    size_t RemoveObserver(std::weak_ptr<ObserverBase> observer);

    /**
     * Notify the \a observer with the current status of all proxy interface objects in the cache
     *
     * \param observer the observer to be notified
     */
    void NotifyObserver(std::weak_ptr<ObserverBase> observer);

    /**
     * Install the allocator for objects of this cache
     *
     * \param allocator the allocator to be installed
     */
    void SetAllocator(std::weak_ptr<ObjectAllocator> allocator);

    /**
     * Add a new object identified by \a objId to the cache using \a allocator
     * This does not notify the application, only updates the cache.
     * Use the NotifyObjectExistence method to trigger the application.
     *
     * \param objId the object identifier
     * \return the new proxy interface object
     */
    std::shared_ptr<ProxyInterface> AddObject(const ObjectId& objId);

    /**
     * Remove an object identified by \a objId
     * This does not notify the application, only updates the cache.
     * Use the NotifyObjectExistence method to trigger the application.
     *
     * \param objId the object identifier
     * \return the removed proxy interface object
     */
    std::shared_ptr<ProxyInterface> RemoveObject(const ObjectId& objId);

    /**
     * Update an object identified by \a objId to the cache using \a dict
     * This does notify the application.
     *
     * \param objId the object identifier
     * \param dict the updated properties
     * \return the updated proxy interface object
     */
    std::shared_ptr<ProxyInterface> UpdateObject(const ObjectId& objId,
                                                 const ajn::MsgArg* dict);

    /**
     * Trigger the application when an object was added or removed.
     *
     * \param obj proxy interface
     * \param add added/removed
     */
    void NotifyObjectExistence(std::shared_ptr<ProxyInterface> obj,
                               bool add);

    /**
     * This will return, by definition, a living object
     *
     * \param objId the object identifier
     * \return the proxy interface object
     * */
    std::shared_ptr<ProxyInterface> GetObject(const ObjectId& objId);

    /* TODO: REPLACE THIS NAIVE APPROACH (by reusing the iterator on livingobjects*/
    std::vector<std::shared_ptr<ProxyInterface> > LivingObjects() const;

  private:
    /**
     * Set of observers for a specific proxy interface.
     */
    typedef std::set<std::weak_ptr<ObserverBase>, std::owner_less <std::weak_ptr<ObserverBase> > > ObserverSet;
    ObserverSet observers;

    /* ignore busAttachment here (not relevant and probably will go out anyway) */
    struct ObjectIdComp {
        bool operator()(const ObjectId& o1,
                        const ObjectId& o2) const;
    };
    typedef std::map<ObjectId, std::shared_ptr<ProxyInterface>, ObjectIdComp> ObjectIdToSharedPtrMap;
    typedef std::map<ObjectId, std::weak_ptr<ProxyInterface>, ObjectIdComp> ObjectIdToWeakPtrMap;

    ObjectIdToSharedPtrMap livingObjects;
    ObjectIdToWeakPtrMap deadObjects;         /* aka the graveyard */
    mutable datadriven::Mutex mutex;         /* is recursive */

    /**
     * The name of the interface for which this cache is created
     */
    qcc::String ifName;

    /**
     * The allocator to allocate objects of the type T held in this cache
     * This is implicitly derived from the Observer<T>
     */
    std::weak_ptr<ObjectAllocator> allocator;

    void GarbageCollect();
};
}

#endif /* OBSERVERCACHE_H_ */
