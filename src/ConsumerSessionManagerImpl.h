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

#ifndef CONSUMERSESSIONMANAGERIMPL_H_
#define CONSUMERSESSIONMANAGERIMPL_H_

#include <qcc/String.h>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/version.h>
#include <alljoyn/Session.h>
#include <alljoyn/AllJoynStd.h>
#include <alljoyn/about/AnnounceHandler.h>
#include <alljoyn/about/AboutClient.h>
#include <alljoyn/services_common/AsyncTaskQueue.h>
#include <qcc/Mutex.h>
#include <map>
#include <set>
#include <forward_list>
#include <vector>

#include <datadriven/ObjectId.h>
#include "ConsumerSessionManager.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
class ConsumerSessionManagerImpl :
    private ajn::services::AnnounceHandler,
    private ajn::SessionListener,
    private ajn::BusAttachment::JoinSessionAsyncCB,
    private ajn::BusListener,
    private ajn::services::AsyncTask {
  public:
    ConsumerSessionManagerImpl(const char* appName);
    virtual ~ConsumerSessionManagerImpl();

    QStatus GetStatus() const;

    /* CONSUMER */
    QStatus RegisterObserver(ConsumerSessionManager::ObserverRef rr,
                             const qcc::String& interfaceName);

    QStatus UnregisterObserver(ConsumerSessionManager::ObserverRef rr);

    /* to be used by consumers when they want to consume
     * Warning: interfaceName must have been passed to RegisterObserver() prior to using this function !
     *
     *  */
    QStatus GetObjectIdsForObserver(ConsumerSessionManager::ObserverRef rr,
                                    ConsumerSessionManager::iterateObjectIdCb iterate,
                                    void* ctx);

    /* Higher layers will be notified through a listener when objects have been created/deleted (that match an interface we are interested in) */
    void RegisterInterfaceListener(ConsumerInterfaceListener* intfListener);

    /* To be used by higher layers to create ProxyBusObject (although you also get this in ObjectId..) */
    ajn::BusAttachment& GetBusAttachment()
    {
        return clientBusAttachment;
    }

  private:
    typedef std::map<qcc::String, ajn::SessionId> SessionPerBusMap;
    typedef std::map<qcc::String, ObjectDescriptions> ObjectDescriptionsInBusMap;       /* key = busUniqueName */
    typedef std::map<qcc::String, unsigned int> StringCountMap;
    typedef std::multimap<qcc::String, ConsumerSessionManager::ObserverRef> ObserverPerIntfMultiMap;
    typedef std::map<ConsumerSessionManager::ObserverRef, qcc::String> IntfPerObserverMap;
    typedef std::map<ajn::SessionId, qcc::String> BusPerSessionMap;
    typedef std::map<qcc::String, qcc::String> String2StringMap;
    typedef std::map<qcc::String, ajn::SessionPort> StringPortMap;
    typedef void (ConsumerInterfaceListener::* OnChange)(ConsumerSessionManager::ObserverRef rr,
                                                         const ObjectId& objId);

    class LeaveSessionData;

    static const int DATADRIVEN_SERVICE_PORT = 5001;
    static const char* DATADRIVEN_PREFIX;
    mutable qcc::Mutex mutex;       /* internal mutex to protect internal datastructures from concurrent access */
    QStatus errorStatus;

    ConsumerInterfaceListener* intflistener;
    ObserverPerIntfMultiMap consumerInterfaces;        /* key=intfname, value=observer (this is a multimap because multiple observers might be interested in the same interface) */
    IntfPerObserverMap observers;
    ObjectDescriptionsInBusMap discoveredObjects;
    SessionPerBusMap joinedSessionForBus;       /* key = busUniqueName, value = session */
    BusPerSessionMap busForJoinedSession;
    /* Caveat: We do not take into account a unique name might have multiple WKN's ... */
    String2StringMap busNameMappingWKN2Unique;       /* key = WKN, value = uniquename */
    StringPortMap busPorts;
    std::set<qcc::String> sessionPending;

    ajn::services::AsyncTaskQueue async;
    ajn::BusAttachment clientBusAttachment;

    QStatus prepareBusAttachment(ajn::BusAttachment& busAttachment);

  #if 0
    void CopyInterfaceNames(const std::vector<const ajn::InterfaceDescription*>& in,
                            std::vector<qcc::String>& out) const;

  #endif

    QStatus ModifyInterestLocked(const qcc::String& intfName,
                                 ajn::SessionId sessionId,
                                 const char* methodCall);

    /* Substract B from A (so A-B) and call onChange for all interesting objects */
    void ObjectDescriptionsDifferenceWithFilterLocked(const qcc::String& busName,
                                                      const ObjectDescriptions& odA,
                                                      const ObjectDescriptions& odB,
                                                      OnChange onChange);

    static void InternalIterateCb(ConsumerSessionManager::ObserverRef observer,
                                  const ObjectId& objId,
                                  void* ctx);

    /* returns true/false if a session should/shouldn't be established */
    bool EvaluateBusLocked(const qcc::String& uniqueBusName) const;

    /* returns true/false if a session is/isn't established */
    void ManageSessionForBusLocked(const qcc::String& uniqueBusName);

    void ManageSessionsLocked();

    /* ajn::services::AnnounceHandler*/
    virtual void Announce(unsigned short version,
                          ajn::SessionPort port,
                          const char* busName,
                          const ObjectDescriptions& objectDescs,
                          const AboutData& aboutData);

    /* ajn::BusAttachment::JoinSessionAsyncCB */
    virtual void JoinSessionCB(QStatus status,
                               ajn::SessionId id,
                               const ajn::SessionOpts& opts,
                               void* context);

    /* ajn::SessionListener */
    virtual void SessionLost(ajn::SessionId sessionId,
                             SessionLostReason reason);

    /* ajn::Buslistener */
    virtual void FoundAdvertisedName(const char* name,
                                     ajn::TransportMask transport,
                                     const char* namePrefix);

    virtual void LostAdvertisedName(const char* name,
                                    ajn::TransportMask transport,
                                    const char* namePrefix);

    virtual void NameOwnerChanged(const char* busName,
                                  const char* previousOwner,
                                  const char* newOwner);

    /* ajn::services::AsyncTask */
    virtual void OnEmptyQueue();

    virtual void OnTask(ajn::services::TaskData const* taskdata);
};
} /* namespace datadriven */
#undef QCC_MODULE
#endif /* CONSUMERSESSIONMANAGERIMPL_H_ */
