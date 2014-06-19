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

#ifndef consumerSESSIONMANAGER_H_
#define consumerSESSIONMANAGER_H_

#include <memory>

#include <datadriven/ObjectId.h>
#include <alljoyn/BusAttachment.h>

namespace datadriven {
class ConsumerSessionManagerImpl;
class ObserverImpl;
class ConsumerInterfaceListener;

/* The responsibility of this class is to establish and teardown session to providers that advertise
 * BusObjects which implement interfaces we are interested in */
class ConsumerSessionManager {
  public:
    typedef void* ObserverRef;    /* Replace this later with actual Observer type */

    /* object iteration callback */
    typedef void (iterateObjectIdCb)(ObserverRef observer, const ObjectId& objId, void* ctx);

    /* The only argument is passed directly to BusAttachment (AFAIK it has little or no use) */
    ConsumerSessionManager(const char* appName);
    virtual ~ConsumerSessionManager();

    QStatus GetStatus() const;

    /* CONSUMER */
    QStatus RegisterObserver(ObserverRef rr,
                             const qcc::String& interfaceName);

    QStatus UnregisterObserver(ObserverRef rr);

    /* to be used by consumers when they want to consume
     * Warning: interfaceName must have been passed to RegisterObserver() prior to using this function !
     *
     *  */
    QStatus GetObjectIdsForObserver(ObserverRef rr,
                                    iterateObjectIdCb iterate,
                                    void* ctx);

    /* Higher layers will be notified through a listener when objects have been created/deleted (that match an interface we are interested in) */
    void RegisterInterfaceListener(ConsumerInterfaceListener* intfListener);

    /* To be used by higher layers to create ProxyBusObject (although you also get this in ObjectId..) */
    ajn::BusAttachment& GetBusAttachment();

  private:
    std::unique_ptr<ConsumerSessionManagerImpl> consumerSessionManagerImpl;

    ConsumerSessionManager(const ConsumerSessionManager&);
    ConsumerSessionManager& operator=(const ConsumerSessionManager&);
};

class ConsumerInterfaceListener {
  private:

  public:
    ConsumerInterfaceListener() { }

    virtual ~ConsumerInterfaceListener() { }

    /* This function will be called objects are added/removed (or interfaces on those objects) implementing interfaces are interested in */
    virtual void OnNew(ConsumerSessionManager::ObserverRef observer,
                       const ObjectId& objId) = 0;

    virtual void OnRemove(ConsumerSessionManager::ObserverRef observer,
                          const ObjectId& objId) = 0;
};
}
#endif
