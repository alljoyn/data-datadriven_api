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

#include "ConsumerSessionManagerImpl.h"
#include <algorithm>
#include <alljoyn/about/AboutPropertyStoreImpl.h>
#include <alljoyn/about/AnnouncementRegistrar.h>
#include <qcc/GUID.h>
#include <cassert>

#include "InterestReceiver.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

using namespace ajn;
using namespace ajn::services;
using namespace datadriven;
using namespace qcc;

ConsumerSessionManagerImpl::ConsumerSessionManagerImpl(const char* _appName) :
    errorStatus(ER_FAIL),
    intflistener(NULL),
    async(this, true),
    clientBusAttachment(_appName, true)
{
    QStatus status = ER_OK;
    SessionOpts opts;
    InterfaceDescription* serverintf;

    do {
        if (NULL == _appName) {
            errorStatus = ER_FAIL;
            QCC_LogError(errorStatus, ("Consumer session manager, provided application name is invalid"));
            break;
        }

        status = prepareBusAttachment(clientBusAttachment);
        if (ER_OK != status) {
            QCC_LogError(status, ("Failed to prepare client bus attachment"));
            errorStatus = status;
            break;
        }

        status = InterestReceiver::CreateInterface(clientBusAttachment, serverintf);
        if (ER_OK != status) {
            QCC_LogError(status, ("Failed to create interest receiver interface"));
            errorStatus = status;
            break;
        }

        async.AsyncTaskQueue::Start();

        clientBusAttachment.RegisterBusListener(*this);
        status = clientBusAttachment.FindAdvertisedName(InterestReceiver::DATADRIVEN_PREFIX);
        if (ER_OK != status) {
            QCC_LogError(status, ("Failed to find advertised name"));
            errorStatus = status;
            break;
        }

        errorStatus = ER_OK; /* if Announce() is already called while still in constructor */
        status = AnnouncementRegistrar::RegisterAnnounceHandler(clientBusAttachment, *this, NULL, 0);
        if (ER_OK != status) {
            QCC_LogError(status, ("Failed to register announce handler"));
            errorStatus = status;
            break;
        }

        errorStatus = ER_OK;
    } while (0);
}

ConsumerSessionManagerImpl::~ConsumerSessionManagerImpl()
{
    AnnouncementRegistrar::UnRegisterAllAnnounceHandlers(clientBusAttachment);
    clientBusAttachment.UnregisterBusListener(*this);
    clientBusAttachment.Disconnect();
    clientBusAttachment.Stop();
    clientBusAttachment.Join();
    async.Stop();
}

QStatus ConsumerSessionManagerImpl::prepareBusAttachment(BusAttachment& busAttachment)
{
    QStatus status = ER_FAIL;

    do {
        status = busAttachment.Start();
        if (status != ER_OK) {
            QCC_LogError(status, ("Failed to start bus attachment"));
            break;
        }

        status = busAttachment.Connect();
        if (status != ER_OK) {
            QCC_LogError(status, ("Failed to connect bus attachment"));
            break;
        }
    } while (0);

    return status;
}

QStatus ConsumerSessionManagerImpl::GetStatus() const
{
    mutex.Lock(__func__, __LINE__);
    QStatus ret = errorStatus;
    mutex.Unlock(__func__, __LINE__);
    return ret;
}

/* This function will return true if we need a session */
bool ConsumerSessionManagerImpl::EvaluateBusLocked(const qcc::String& uniqueBusName) const
{
    if (ER_OK != errorStatus) {
        QCC_LogError(errorStatus, ("Consumer session manager not properly initialized"));
        return false;
    }

    ObjectDescriptionsInBusMap::const_iterator doit = discoveredObjects.find(uniqueBusName);
    if (doit == discoveredObjects.end()) {
        QCC_LogError(ER_BAD_ARG_1, ("Unknown unique bus name '%s'", uniqueBusName.c_str()));
        return false;
    }

    const ObjectDescriptions& od = doit->second;

    std::vector<qcc::String> busInterfaces;
    std::vector<qcc::String> consInterfaces;
    std::vector<qcc::String> intersection;

    for (AboutClient::ObjectDescriptions::const_iterator it = od.begin(); it != od.end(); ++it) {
        qcc::String key = it->first;
        const std::vector<qcc::String>& vector = it->second;
        busInterfaces.insert(busInterfaces.end(), vector.begin(), vector.end());
    }
    std::sort(busInterfaces.begin(), busInterfaces.end());

    /* I don't know a more efficient way to copy all the keys */
    consInterfaces.reserve(consumerInterfaces.size());
    for (ObserverPerIntfMultiMap::const_iterator it = consumerInterfaces.begin(), end = consumerInterfaces.end();
         it != end;
         it = consumerInterfaces.upper_bound(it->first)) {
        consInterfaces.push_back(it->first);
    }

    std::set_intersection(busInterfaces.begin(), busInterfaces.end(), consInterfaces.begin(),
                          consInterfaces.end(), back_inserter(intersection));

    if (intersection.size() > 0) {
        return true;
    } else {
        return false;
    }

    return false;
}

class ConsumerSessionManagerImpl::LeaveSessionData :
    public ajn::services::TaskData {
  private:
    ajn::SessionId sessionId;

  public:
    LeaveSessionData(ajn::SessionId _sessionId) :
        sessionId(_sessionId) { }

    friend class ConsumerSessionManagerImpl;
};

void ConsumerSessionManagerImpl::ManageSessionForBusLocked(const qcc::String& uniqueBusName)
{
    if (ER_OK != errorStatus) {
        QCC_LogError(errorStatus, ("Consumer session manager not properly initialized"));
        return;
    }

    QStatus status;
    SessionPerBusMap::iterator it;

    /* session already present ? */
    if ((it = joinedSessionForBus.find(uniqueBusName)) != joinedSessionForBus.end()) {
        if (EvaluateBusLocked(uniqueBusName) == false) {
            QCC_DbgHLPrintf(("Should leave session %lu", (unsigned long)it->second));
            async.Enqueue(new LeaveSessionData(it->second));
            busForJoinedSession.erase(it->second);
            joinedSessionForBus.erase(it);
        }
    } else {
        if (EvaluateBusLocked(uniqueBusName) == true && sessionPending.find(uniqueBusName) == sessionPending.end()) {
            QCC_DbgHLPrintf(("Should establish session with '%s'", uniqueBusName.c_str()));

            SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
            String* copy = new String(uniqueBusName);
            status = clientBusAttachment.JoinSessionAsync(
                uniqueBusName.c_str(), busPorts[uniqueBusName], this, opts, this, (void*)copy);
            if (status != ER_OK) {
                QCC_LogError(status, ("Failed to establish session"));
                delete copy;
            } else {
                sessionPending.insert(uniqueBusName);
            }
        }
    }
}

void ConsumerSessionManagerImpl::ManageSessionsLocked()
{
    if (ER_OK != errorStatus) {
        QCC_LogError(errorStatus, ("Consumer session manager not properly initialized"));
        return;
    }

    for (ObjectDescriptionsInBusMap::const_iterator it = discoveredObjects.begin(); it != discoveredObjects.end();
         ++it) {
        ManageSessionForBusLocked(it->first);
    }
}

void ConsumerSessionManagerImpl::ObjectDescriptionsDifferenceWithFilterLocked(const String& busName,
                                                                              const ObjectDescriptions& odA,
                                                                              const ObjectDescriptions& odB,
                                                                              OnChange onChange)
{
    if (ER_OK != errorStatus) {
        QCC_LogError(errorStatus, ("Consumer session manager not properly initialized"));
        return;
    }

    ObjectDescriptions difference;
#if 0
    QCC_DbgPrintf(("A"));
    for (ObjectDescriptions::const_iterator it = odA.begin(); it != odA.end(); ++it) {
        QCC_DbgPrintf(("\t%s", it->first.c_str()));
    }
    QCC_DbgPrintf(("B"));
    for (ObjectDescriptions::const_iterator it = odB.begin(); it != odB.end(); ++it) {
        QCC_DbgPrintf(("\t%s", it->first.c_str()));
    }
#endif

    if (intflistener == NULL) {
        return;
    }

    SessionPerBusMap::const_iterator sessionit = joinedSessionForBus.find(busName);
    if (sessionit == joinedSessionForBus.end()) {
        return;
    }

    std::set_difference(odA.begin(), odA.end(), odB.begin(), odB.end(), inserter(difference, difference.end()));
    SessionId sessionId = sessionit->second;

    for (ObjectDescriptions::const_iterator it = difference.begin(); it != difference.end(); ++it) {
        const std::vector<qcc::String>& objectInterfaces = it->second;

        for (std::vector<String>::const_iterator intfit = objectInterfaces.begin();
             intfit != objectInterfaces.end();
             ++intfit) {
            const String& interfaceName = *intfit;
            std::pair<ObserverPerIntfMultiMap::const_iterator,
                      ObserverPerIntfMultiMap::const_iterator> ret = consumerInterfaces.equal_range(interfaceName);

            for (ObserverPerIntfMultiMap::const_iterator observerit = ret.first; observerit != ret.second;
                 ++observerit) {
                ObjectId objId(clientBusAttachment, busName, it->first, sessionId);
                ConsumerSessionManager::ObserverRef rr = observerit->second;

                (intflistener->*onChange)(rr, objId);
            }
        }
    }
}

void ConsumerSessionManagerImpl::Announce(unsigned short version, SessionPort port,
                                          const char* busName,
                                          const ObjectDescriptions& objectDescs,
                                          const AboutData& aboutData)
{
    QCC_DbgHLPrintf(("Received announcement from '%s'", busName));

    /* The WKN can be found in the AboutData as we cannot completely trust the discovery callbacks
     * (like NameOwnerChanged, FoundAdvertisedName,...)
     * to come in all situations */
    const String& keyName = AboutPropertyStoreImpl::getPropertyStoreName(APP_NAME);
    AboutData::const_iterator it = aboutData.find(keyName);
    if (it == aboutData.end()) {
        QCC_DbgRemoteError(("Received invalid About data, ingoring"));
        return;
    }

    const MsgArg& value = it->second;
    const String& wkn = value.v_string.str;
    if (0 !=
        strncmp(wkn.c_str(), InterestReceiver::DATADRIVEN_PREFIX, sizeof(InterestReceiver::DATADRIVEN_PREFIX) - 1)) {
        QCC_DbgPrintf(("WKN '%s' does not match prefix '%s', ignoring", wkn.c_str(),
                       InterestReceiver::DATADRIVEN_PREFIX));
        return;
    }

    QCC_DbgPrintf(("Remote WKN is %s", wkn.c_str()));

    mutex.Lock(__func__, __LINE__);
    if (ER_OK != errorStatus) {
        QCC_LogError(errorStatus, ("Consumer session manager not properly initialized"));
        mutex.Unlock(__func__, __LINE__);
        return;
    }
    busNameMappingWKN2Unique[wkn] = busName;

    busPorts[busName] = port;

    /* if we already had a session: check for news and/or removals */
    SessionPerBusMap::const_iterator sessionit = joinedSessionForBus.find(busName);
    if (sessionit != joinedSessionForBus.end()) {
        QCC_DbgPrintf(("We already have a session in place with '%s'", busName));
        /* if there was already a session, this is definitely not the first announce */
        assert(discoveredObjects.find(busName) != discoveredObjects.end());
        const ObjectDescriptions& old = discoveredObjects[busName];

        ObjectDescriptionsDifferenceWithFilterLocked(busName,
                                                     old,
                                                     objectDescs,
                                                     &ConsumerInterfaceListener::OnRemove);

        ObjectDescriptionsDifferenceWithFilterLocked(busName,
                                                     objectDescs,
                                                     old,
                                                     &ConsumerInterfaceListener::OnNew);
    } else {
        QCC_DbgPrintf(("We have not yet got a session in place with '%s'", busName));
    }

    discoveredObjects[busName] = objectDescs;

    ManageSessionForBusLocked(busName);
    mutex.Unlock(__func__, __LINE__);

    QCC_DbgPrintf(("END -- Received announcement from '%s'", busName));
}

void ConsumerSessionManagerImpl::FoundAdvertisedName(const char* name,
                                                     ajn::TransportMask transport,
                                                     const char* namePrefix)
{
    /* nothing to do here */
    //QCC_DbgHLPrintf(("Found advertised name '%s'", name));
}

void ConsumerSessionManagerImpl::LostAdvertisedName(const char* name,
                                                    ajn::TransportMask transport,
                                                    const char* namePrefix)
{
    QCC_DbgHLPrintf(("Lost advertised name '%s'", name));

    mutex.Lock(__func__, __LINE__);

    if (ER_OK != errorStatus) {
        QCC_LogError(errorStatus, ("Consumer session manager not properly initialized"));
        mutex.Unlock(__func__, __LINE__);
        return;
    }

    String2StringMap::iterator it = busNameMappingWKN2Unique.find(name);
    if (it != busNameMappingWKN2Unique.end()) {
        busPorts.erase(it->second);
        discoveredObjects.erase(it->second);
        busNameMappingWKN2Unique.erase(it);
    }
    mutex.Unlock(__func__, __LINE__);
}

void ConsumerSessionManagerImpl::NameOwnerChanged(const char* busName,
                                                  const char* previousOwner,
                                                  const char* newOwner)
{
    //QCC_DbgHLPrintf(("Name owner changed: WKN '%s', previous owner '%s', new owner '%s'", busName, previousOwner, newOwner));
}

QStatus ConsumerSessionManagerImpl::RegisterObserver(ConsumerSessionManager::ObserverRef observer,
                                                     const String& interfaceName)
{
    QStatus status = ER_FAIL;
    mutex.Lock(__func__, __LINE__);
    bool newIntf = false;
    do {
        if (ER_OK != errorStatus) {
            QCC_LogError(errorStatus, ("Consumer session manager not properly initialized"));
            status = errorStatus;
            break;
        }
        std::pair<ObserverPerIntfMultiMap::const_iterator,
                  ObserverPerIntfMultiMap::const_iterator> ret = consumerInterfaces.equal_range(interfaceName);

        if (ret.first == ret.second) {
            newIntf = true;
        }

        ObserverPerIntfMultiMap::const_iterator it = ret.first;
        for (; it != ret.second; ++it) {
            if (it->second == observer) {
                break;
            }
        }
        if (it != ret.second) {
            status = ER_BAD_ARG_1;
            QCC_LogError(status, ("This observer was already registered"));
            break;
        }

        consumerInterfaces.insert(std::pair<qcc::String, ConsumerSessionManager::ObserverRef>(interfaceName, observer));
        observers.insert(std::pair<ConsumerSessionManager::ObserverRef, qcc::String>(observer, interfaceName));

        /* trigger onNew() here on interesting objects we already have sessions to */
        if (intflistener != NULL) {
            /* AJ Mutexes are recursive so I don't bother with unlocking first.. */
            GetObjectIdsForObserver(observer, InternalIterateCb, (void*)intflistener);
        }

        if (newIntf == true) {
            for (BusPerSessionMap::const_iterator it = busForJoinedSession.begin();
                 it != busForJoinedSession.end();
                 ++it) {
                ModifyInterestLocked(interfaceName, it->first, InterestReceiver::REGISTER_INTEREST_METHOD_CALL);
            }
        }

        /* find all busses that have busobjects that implement this interface and join a session with them if not yet done */
        ManageSessionsLocked();

        status = ER_OK;
    } while (0);

    mutex.Unlock(__func__, __LINE__);

    return status;
}

QStatus ConsumerSessionManagerImpl::UnregisterObserver(ConsumerSessionManager::ObserverRef observer)
{
    QStatus status = ER_FAIL;
    bool lastObserverForIntf = false;
    mutex.Lock(__func__, __LINE__);
    do {
        if (ER_OK != errorStatus) {
            QCC_LogError(errorStatus, ("Consumer session manager not properly initialized"));
            status = errorStatus;
            break;
        }

        IntfPerObserverMap::iterator observerit = observers.find(observer);
        if (observerit == observers.end()) {
            status = ER_BAD_ARG_1;
            QCC_LogError(status, ("Failed to unregister observer as it was not registered"));
            break;
        }

        const String& interfaceName = observerit->second;

        std::pair<ObserverPerIntfMultiMap::iterator,
                  ObserverPerIntfMultiMap::iterator> ret = consumerInterfaces.equal_range(interfaceName);
        assert(ret.first != ret.second);
        //QCC_DbgPrintf(("Before unregistration: %d observers for interface '%s'", std::distance(ret.first, ret.second), interfaceName.c_str()));
        if (std::distance(ret.first, ret.second) == 1) {
            lastObserverForIntf = true;
        }

        ObserverPerIntfMultiMap::iterator it;
        for (it = ret.first; it != ret.second; ++it) {
            if (it->second == observer) {
                break;
            }
        }

        assert(it != ret.second); /* this would mean we did not find the observer which should be there ! */

        /* if this was our last observer for this intf, let everyone know we don't care anymore */
        if (lastObserverForIntf == true) {
            for (BusPerSessionMap::const_iterator it = busForJoinedSession.begin();
                 it != busForJoinedSession.end();
                 ++it) {
                QCC_DbgPrintf(("Last observer for interface '%s' unregistered, remove interest",
                               interfaceName.c_str()));
                ModifyInterestLocked(interfaceName, it->first, InterestReceiver::UNREGISTER_INTEREST_METHOD_CALL);
            }
        }

        observers.erase(observerit);
        consumerInterfaces.erase(it);

        ManageSessionsLocked();

        status = ER_OK;
    } while (0);

    mutex.Unlock(__func__, __LINE__);

    return status;
}

void ConsumerSessionManagerImpl::JoinSessionCB(QStatus status, SessionId id,
                                               const SessionOpts& opts,
                                               void* context)
{
    qcc::String* busName = static_cast<qcc::String*>(context);

    mutex.Lock(__func__, __LINE__);

    do {
        if (ER_OK != errorStatus) {
            QCC_LogError(errorStatus, ("Consumer session manager not properly initialized"));
            break;
        }

        sessionPending.erase(*busName);
        if (status == ER_OK) {
            QCC_DbgPrintf(("Successfully joined session %lu", (unsigned long)id));
        } else {
            QCC_LogError(status, ("Failed to join session %lu", (unsigned long)id));
            break;
        }
        joinedSessionForBus[*busName] = id;
        busForJoinedSession[id] = *busName;

        ObjectDescriptions empty;
        ObjectDescriptionsInBusMap::const_iterator it = discoveredObjects.find(*busName);
        assert(it != discoveredObjects.end());

        ObjectDescriptionsDifferenceWithFilterLocked(*busName,
                                                     it->second,
                                                     empty,
                                                     &ConsumerInterfaceListener::OnNew);

        /* inform remote end of all our interests... */
        for (ObserverPerIntfMultiMap::const_iterator it = consumerInterfaces.begin(), end = consumerInterfaces.end();
             it != end;
             it = consumerInterfaces.upper_bound(it->first)) {
            ModifyInterestLocked(it->first, id, InterestReceiver::REGISTER_INTEREST_METHOD_CALL);
        }
    } while (0);
    mutex.Unlock(__func__, __LINE__);

    delete busName;
}

QStatus ConsumerSessionManagerImpl::ModifyInterestLocked(const qcc::String& intfName,
                                                         SessionId sessionId,
                                                         const char* methodCall)
{
    if (ER_OK != errorStatus) {
        QCC_LogError(errorStatus, ("Consumer session manager not properly initialized"));
        return errorStatus;
    }

    QStatus status;
    assert(busForJoinedSession.find(sessionId) != busForJoinedSession.end()); /* should never fail */
    BusPerSessionMap::const_iterator it = busForJoinedSession.find(sessionId);
    if (it == busForJoinedSession.end()) {
        QCC_LogError(ER_BAD_ARG_2, ("Unknown session id %lu", (unsigned long)sessionId));
        return ER_FAIL;
    }

    const String& busName = it->second;

    ProxyBusObject remoteObj(clientBusAttachment,
                             busName.c_str(), InterestReceiver::DATADRIVEN_INTEREST_PATH, sessionId);
    const InterfaceDescription* irintf = clientBusAttachment.GetInterface(InterestReceiver::DATADRIVEN_INTEREST_INTF);
    if (irintf == NULL) {
        QCC_LogError(ER_FAIL,
                     ("Interface '%s' is not registered to the underlying BusAttachment",
                      InterestReceiver::DATADRIVEN_INTEREST_INTF));
        return ER_FAIL;
    }

    status = remoteObj.AddInterface(*irintf);
    if (status != ER_OK) {
        QCC_LogError(status,
                     ("Failed to add interface '%s' to ProxyBusObject", InterestReceiver::DATADRIVEN_INTEREST_INTF));
        return status;
    }

    Message reply(clientBusAttachment);
    MsgArg inputs[1];

    inputs[0].Set("s", intfName.c_str());

    /* Fire-and-forget */
    status = remoteObj.MethodCall(InterestReceiver::DATADRIVEN_INTEREST_INTF, methodCall, inputs, 1, 0);

    if (ER_OK != status) {
        QCC_LogError(status, ("Failed to modify interests"));
    }

    return status;
}

void ConsumerSessionManagerImpl::SessionLost(ajn::SessionId sessionId, SessionLostReason reason)
{
    mutex.Lock(__func__, __LINE__);
    do {
        QCC_DbgPrintf(("Lost session %lu", (unsigned long)sessionId));

        if (ER_OK != errorStatus) {
            QCC_LogError(errorStatus, ("Consumer session manager not properly initialized"));
            break;
        }

        BusPerSessionMap::iterator it = busForJoinedSession.find(sessionId);
        if (it == busForJoinedSession.end()) {
            /* If we get here, we have probably scheduled a LeaveSessionData an instant ago to leave this session ourselves
             * No need to do here anything... */
            break;
        }

        ObjectDescriptions empty;

        ObjectDescriptionsInBusMap::const_iterator odit = discoveredObjects.find(it->second);
        if (odit == discoveredObjects.end()) {
            // OnRemove() will not be triggered, should not happen
            QCC_LogError(ER_FAIL,
                         ("Failed to find any discovered objects for lost session %lu", (unsigned long)sessionId));
            break;
        }

        ObjectDescriptionsDifferenceWithFilterLocked(it->second,
                                                     odit->second,
                                                     empty,
                                                     &ConsumerInterfaceListener::OnRemove);
        joinedSessionForBus.erase(it->second);
        busForJoinedSession.erase(it);
    } while (0);
    mutex.Unlock(__func__, __LINE__);
}

QStatus ConsumerSessionManagerImpl::GetObjectIdsForObserver(ConsumerSessionManager::ObserverRef rr,
                                                            ConsumerSessionManager::iterateObjectIdCb iterate,
                                                            void* ctx)
{
    QStatus status = ER_FAIL;
    if (iterate == NULL) {
        return ER_BAD_ARG_1;
    }

    mutex.Lock(__func__, __LINE__);
    do {
        if (ER_OK != errorStatus) {
            QCC_LogError(errorStatus, ("Consumer session manager not properly initialized"));
            status = errorStatus;
            break;
        }

        IntfPerObserverMap::const_iterator intfobserver_it = observers.find(rr);
        if (intfobserver_it == observers.end()) {
            status = ER_FAIL;
            QCC_LogError(ER_BAD_ARG_1, ("Failed to find observer"));
            break;
        }
        const String& interfaceName = intfobserver_it->second;

        for (ObjectDescriptionsInBusMap::const_iterator doit = discoveredObjects.begin();
             doit != discoveredObjects.end();
             ++doit) {
            SessionPerBusMap::const_iterator sessionit;

            if ((sessionit = joinedSessionForBus.find(doit->first)) != joinedSessionForBus.end()) {
                const ObjectDescriptions& od = doit->second;

                for (AboutClient::ObjectDescriptions::const_iterator odit = od.begin(); odit != od.end(); ++odit) {
                    const std::vector<qcc::String>& intfvector = odit->second;

                    /* not really clear why I can just use find without namespace std qualifier */
                    std::vector<qcc::String>::const_iterator intfit = find(intfvector.begin(),
                                                                           intfvector.end(), interfaceName);
                    if (intfit != intfvector.end()) {
                        /* we have found something */
                        ObjectId objId(clientBusAttachment, doit->first, odit->first,  sessionit->second);
                        iterate(rr, objId, ctx);
                    }
                }
            }
        }

        status = ER_OK;
    } while (0);
    mutex.Unlock(__func__, __LINE__);

    return status;
}

void ConsumerSessionManagerImpl::InternalIterateCb(ConsumerSessionManager::ObserverRef observer,
                                                   const ObjectId& objId,
                                                   void* ctx)
{
    ConsumerInterfaceListener* il = static_cast<ConsumerInterfaceListener*>(ctx);
    il->OnNew(observer, objId);
}

void ConsumerSessionManagerImpl::RegisterInterfaceListener(ConsumerInterfaceListener* intfListener)
{
    /* TBD */
    mutex.Lock(__func__, __LINE__);
    if (ER_OK != errorStatus) {
        QCC_LogError(errorStatus, ("Consumer session manager not properly initialized"));
        mutex.Unlock(__func__, __LINE__);
        return;
    }
    intflistener = intfListener;
    mutex.Unlock(__func__, __LINE__);
}

void ConsumerSessionManagerImpl::OnEmptyQueue()
{
}

void ConsumerSessionManagerImpl::OnTask(TaskData const* taskdata)
{
    const LeaveSessionData* lsd = static_cast<const LeaveSessionData*>(taskdata);

    errorStatus = clientBusAttachment.LeaveSession(lsd->sessionId);
    if (errorStatus != ER_OK) {
        QCC_LogError(errorStatus, ("Failed to leave session"));
        return;
    }
}
