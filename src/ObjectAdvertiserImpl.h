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

#ifndef OBJECTADVERTISERIMPL_H_
#define OBJECTADVERTISERIMPL_H_

#include <memory>
#include <map>
#include <set>
#include <vector>

#include <qcc/String.h>
#include <datadriven/Mutex.h>

#include <alljoyn/version.h>
#include <alljoyn/Session.h>
#include <alljoyn/AllJoynStd.h>
#include <alljoyn/about/AboutPropertyStoreImpl.h>

#include "BusConnectionImpl.h"
#include "common/AsyncTaskQueue.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

namespace datadriven {
class ProvidedObjectImpl;

/* The responsibility of this class is to manage the availability of BusObjects
 * on the Bus.  It will
 * - register them on the Bus and announce them through About
 * - unregister them from the Bus and hide them again from About.
 */
class ObjectAdvertiserImpl :
    private AsyncTask, ajn::SessionPortListener {
  public:
    ObjectAdvertiserImpl(ajn::BusAttachment* bus,
                         ajn::services::AboutPropertyStoreImpl* aboutPropertyStore = NULL);
    virtual ~ObjectAdvertiserImpl();

    ajn::BusAttachment& GetBusAttachment() const;

    QStatus AddProvidedObject(std::weak_ptr<ProvidedObjectImpl> object);

    void RemoveProvidedObject(std::weak_ptr<ProvidedObjectImpl> object);

    void CallMethodHandler(std::weak_ptr<ProvidedObjectImpl> object,
                           ajn::MessageReceiver* ctx,
                           ajn::MessageReceiver::MethodHandler handler,
                           const ajn::InterfaceDescription::Member* member,
                           ajn::Message& message);

    class Task :
        public TaskData {
      public:
        virtual void Execute() const = 0;
    };

    void ProviderAsyncEnqueue(const Task* bctd);

    QStatus GetStatus() const;

  private:
    /** Asynchronous task queue used by ProvidedObject to schedule a task */
    mutable AsyncTaskQueue providerAsync;

    /** Mutex that protects the providers set */
    mutable datadriven::Mutex providersMutex;
    /** Set of provided objects advertised by this advertiser */
    std::set<std::weak_ptr<ProvidedObjectImpl>, std::owner_less<std::weak_ptr<ProvidedObjectImpl> > > providers;

    /* Mutex to protect AboutService internals */
    mutable datadriven::Mutex aboutMutex;

    std::shared_ptr<BusConnectionImpl> busConnection;
    QStatus errorStatus;

    ajn::services::AboutPropertyStoreImpl* aboutPropertyStore;
    bool ownAboutPropertyStore;

    QStatus AboutUpdate(const std::shared_ptr<ajn::BusObject> busObject,
                        const std::vector<qcc::String>& interfaceNames,
                        bool add);

    QStatus AdvertiseBusObject(std::shared_ptr<ajn::BusObject> busObject,
                               const std::vector<qcc::String>& interfaceNames);

    QStatus UnadvertiseBusObject(std::shared_ptr<ajn::BusObject> busObject,
                                 const std::vector<qcc::String>& interfaceNames);

    /* ajn::SessionPortListener */
    virtual bool AcceptSessionJoiner(ajn::SessionPort sessionPort,
                                     const char* joiner,
                                     const ajn::SessionOpts& opts);

    virtual void SessionJoined(ajn::SessionPort sessionPort,
                               ajn::SessionId id,
                               const char* joiner);

    /* datadriven::AsyncTask */
    virtual void OnEmptyQueue();

    virtual void OnTask(TaskData const* taskdata);

    ObjectAdvertiserImpl(const ObjectAdvertiserImpl&);
    ObjectAdvertiserImpl& operator=(const ObjectAdvertiserImpl&);
};
} /* namespace datadriven */

#undef QCC_MODULE
#endif /* OBJECTADVERTISERIMPL_H_ */
