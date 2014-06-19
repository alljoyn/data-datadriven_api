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
#include <vector>

#include <datadriven/ObjectId.h>
#include <datadriven/ProxyInterface.h>

namespace datadriven {
class ObserverCache {
  public:
    class ObjectAllocator {
      public:
        virtual ProxyInterface* Alloc(const ObjectId& objId) = 0;
    };

    ObserverCache();

    std::shared_ptr<ProxyInterface> SetObject(const ObjectId& objId,
                                              ObjectAllocator& allocator);

    std::shared_ptr<ProxyInterface> RemoveObject(const ObjectId& objId);

    /* This will return, by definition, a living object */
    std::shared_ptr<ProxyInterface> Get(const ObjectId& objId);

    /* TODO: REPLACE THIS NAIVE APPROACH (by reusing the iterator on livingobjects*/
    std::vector<std::shared_ptr<ProxyInterface> > LivingObjects() const;

  private:
    /* ignore busAttachment here (not relevant and probably will go out anyway) */
    struct ObjectIdComp {
        bool operator()(const ObjectId& o1,
                        const ObjectId& o2) const;
    };
    typedef std::map<ObjectId, std::shared_ptr<ProxyInterface>, ObjectIdComp> ObjectIdToSharedPtrMap;
    typedef std::map<ObjectId, std::weak_ptr<ProxyInterface>, ObjectIdComp> ObjectIdToWeakPtrMap;

    ObjectIdToSharedPtrMap livingObjects;
    ObjectIdToWeakPtrMap deadObjects;         /* aka the graveyard */
    mutable qcc::Mutex mutex;         /* is recursive */

    void GarbageCollect();
};
}

#endif /* OBSERVERCACHE_H_ */
