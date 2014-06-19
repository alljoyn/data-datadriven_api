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

#ifndef PROVIDERSESSIONMANAGERIMPL_H_
#define PROVIDERSESSIONMANAGERIMPL_H_

#include <qcc/String.h>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/version.h>
#include <alljoyn/Session.h>
#include <alljoyn/AllJoynStd.h>
#include <alljoyn/about/AboutPropertyStoreImpl.h>

#include <qcc/Mutex.h>
#include <map>
#include <set>
#include <vector>

#include "../inc/datadriven/datadriven.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

namespace datadriven {
class InterestReceiver;

class ProviderSessionManagerImpl :
    private ajn::SessionListener,
    private ajn::SessionPortListener {
  public:
    ProviderSessionManagerImpl(const char* appName,
                               BusConnectionImpl* fac);
    virtual ~ProviderSessionManagerImpl();

    QStatus AdvertiseBusObject(ProvidedObject& busObject);

    QStatus RemoveBusObject(ProvidedObject& busObject);

    QStatus BusObjectSignal(ProvidedObject& busObject,
                            const qcc::String& interface,
                            const ajn::InterfaceDescription::Member* member,
                            const ajn::MsgArg* msgArg,
                            size_t len);

    /* To be used by higher layers to create BusObject */
    ajn::BusAttachment& GetBusAttachment();

    QStatus GetStatus() const;

    static const int DATADRIVEN_SERVICE_PORT = 5001;

  private:
    friend class InterestReceiver;

    typedef std::map<qcc::String, ajn::SessionId> SessionPerBusMap;
    typedef std::map<ajn::SessionId, std::set<qcc::String> > InterfacesPerSessionMap;

    mutable qcc::Mutex mutex;     /* internal mutex to protect internal datastructures from concurrent access */
    mutable qcc::Mutex aboutMutex; /* internal mutex to protect AboutService internals */

    BusConnectionImpl* busConnectionImpl;
    std::unique_ptr<InterestReceiver> ir;
    QStatus errorStatus;
    InterfacesPerSessionMap registeredInterfaces;
    ajn::BusAttachment serverBusAttachment;

    QStatus _GetStatus(bool lock = true,
                       bool trace = true) const;

    /* I don't have the time to implement a propertystore that exactly fits our needs... */
    ajn::services::AboutPropertyStoreImpl aboutPropertyStore;
    QStatus AboutUpdate(const ProvidedObject& busObject,
                        bool add);

    /* ajn::SessionListener */
    virtual void SessionLost(ajn::SessionId sessionId,
                             SessionLostReason reason);

    /* ajn::SessionPortListener */
    virtual bool AcceptSessionJoiner(ajn::SessionPort sessionPort,
                                     const char* joiner,
                                     const ajn::SessionOpts& opts);

    virtual void SessionJoined(ajn::SessionPort sessionPort,
                               ajn::SessionId id,
                               const char* joiner);

    ProviderSessionManagerImpl(const ProviderSessionManagerImpl&);
    ProviderSessionManagerImpl& operator=(const ProviderSessionManagerImpl&);
};
} /* namespace datadriven */

#undef QCC_MODULE
#endif /* PROVIDERSESSIONMANAGERIMPL_H_ */
