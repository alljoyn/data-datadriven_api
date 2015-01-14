/******************************************************************************
 * Copyright (c) 2014-2015, AllSeen Alliance. All rights reserved.
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

#include <datadriven/Mutex.h>

#include <alljoyn/about/AnnouncementRegistrar.h>

#include <datadriven/ObserverBase.h>

#include "ObserverCache.h"
#include "BusConnectionImpl.h"
#include "SessionManager.h"
#include "ObserverManager.h"

#include <algorithm>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
class ObserverManagerTask :
    public ObserverManager::Task {
  public:
    ObserverManagerTask(const std::vector<std::weak_ptr<ObserverCache> >& wcaches,
                        const ObjectId& objId,
                        ObserverManager::Action action) :
        wcaches(wcaches), id(objId), action(action)
    { }

    virtual ~ObserverManagerTask()
    { }

    void Execute() const
    {
        typedef std::pair<std::shared_ptr<ObserverCache>, ObserverCache::NotificationSet> CacheProxyEntry;
        typedef std::multimap<std::shared_ptr<ObserverCache>, ObserverCache::NotificationSet> CacheProxyMultiMap;
        CacheProxyMultiMap usedcaches;
        for (const std::weak_ptr<ObserverCache>& wcache : wcaches) {
            std::shared_ptr<ObserverCache> cache = wcache.lock();
            if (cache) {
                switch (action) {
                case ObserverManager::Action::ADD:
                    {
                        QCC_DbgPrintf(("Add object (%s, %s)", id.GetBusName().c_str(),
                                       id.GetBusObjectPath().c_str()));
                        CacheProxyEntry entry(cache, cache->AddObject(id));
                        usedcaches.insert(entry);
                        break;
                    }

                case ObserverManager::Action::REMOVE:
                    {
                        QCC_DbgPrintf(("Remove object (%s, %s)", id.GetBusName().c_str(),
                                       id.GetBusObjectPath().c_str()));
                        CacheProxyEntry entry(cache, cache->RemoveObject(id));
                        usedcaches.insert(entry);
                        break;
                    }

                default:
                    break;
                }
            }
        }

        for (const CacheProxyEntry& entry : usedcaches) {
            switch (action) {
            case ObserverManager::Action::ADD :
                entry.first->NotifyObjectExistence(entry.second.interface, true, entry.second.observers);
                break;

            case ObserverManager::Action::REMOVE:
                entry.first->NotifyObjectExistence(entry.second.interface, false, entry.second.observers);
                break;

            default:
                break;
            }
        }
    }

  private:
    std::vector<std::weak_ptr<ObserverCache> > wcaches;
    ObjectId id;
    ObserverManager::Action action;
};

/* datadriven::AsyncTask */
void ObserverManager::OnEmptyQueue()
{
}

/* datadriven::AsyncTask */
void ObserverManager::OnTask(TaskData const* taskdata)
{
    const ObserverManagerTask* data  = static_cast<const ObserverManagerTask*>(taskdata);
    data->Execute();
}

void ObserverManager::Enqueue(const Task* task)
{
    asyncTaskQueue.Enqueue(task);
}

std::shared_ptr<ObserverManager> ObserverManager::GetInstance(std::shared_ptr<
                                                                  BusConnectionImpl>
                                                              busConnection)
{
    static datadriven::Mutex mutex;
    static std::weak_ptr<ObserverManager> instance;
    std::shared_ptr<ObserverManager> sharedInstance = nullptr;

    mutex.Lock();
    sharedInstance = instance.lock();
    if (!sharedInstance && busConnection) {
        sharedInstance = std::shared_ptr<ObserverManager>(new ObserverManager(busConnection));
        if (ER_OK != sharedInstance->GetStatus()) {
            sharedInstance = nullptr;
        }
        instance = sharedInstance;
    }
    mutex.Unlock();
    return sharedInstance;
}

ObserverManager::ObserverManager(std::shared_ptr<BusConnectionImpl> busConnection) :
    status(ER_OK),
    busConnection(busConnection),
    sessionMgr(new SessionManager(busConnection->GetBusAttachment())),
    asyncTaskQueue(this, true)
{
    status = sessionMgr->GetStatus();
    if (ER_OK != status) {
        QCC_LogError(status, ("Failed to start session manager"));
    }
    this->sessionMgr->RegisterListener(this);
    asyncTaskQueue.Start();
}

ObserverManager::~ObserverManager()
{
    Stop();
    asyncTaskQueue.Stop();
    delete sessionMgr;
}

QStatus ObserverManager::GetStatus() const
{
    return status;
}

void ObserverManager::Stop()
{
    cachesMutex.Lock();
    QCC_DbgPrintf(("Stop observer manager"));
    sessionMgr->UnregisterListener(this);
    for (ObserverCacheMap::iterator iterator = caches.begin();
         iterator != caches.end();
         iterator++) {
        RemoveAnnounceHandler(iterator->first);
    }
    cachesMutex.Unlock();
}

QStatus ObserverManager::RegisterObserver(std::weak_ptr<ObserverBase> observer,
                                          qcc::String ifName)
{
    QStatus status = ER_OK;
    bool newCache = false;
    std::shared_ptr<ObserverCache> cache = nullptr;

    cachesMutex.Lock();
    do {
        ObserverCacheMap::iterator iterator = caches.find(ifName);
        if (caches.end() == iterator) {
            QCC_DbgPrintf(("Create observer cache for %s", ifName.c_str()));
            // add MatchRule for signals on this interface
            qcc::String rule("type='signal',interface='");
            rule.append(ifName);
            rule.append("'");
            status = busConnection->GetBusAttachment().AddMatch(rule.c_str());
            QCC_DbgPrintf(("Add match rule for %s", ifName.c_str()));
            if (ER_OK != status) {
                QCC_LogError(status, ("Failed to add signal match rule"));
                break;
            }
            status = AddAnnounceHandler(ifName);
            if (ER_OK != status) {
                QCC_LogError(status, ("Failed to register announce handler"));
                break;
            }
            cache = std::shared_ptr<ObserverCache>(new ObserverCache(ifName, observer));
            caches[ifName] = cache;
            newCache = true;
        } else {
            cache = iterator->second;
        }
        if (nullptr != cache) {
            cache->AddObserver(observer);
            cache->NotifyObserver(observer);
        }
    } while (0);
    cachesMutex.Unlock();
    if (newCache) {
        PopulateCache(cache, ifName);
    }
    return status;
}

void ObserverManager::UnregisterObserver(std::weak_ptr<ObserverBase> observer,
                                         qcc::String ifName)
{
    cachesMutex.Lock();
    ObserverCacheMap::iterator iterator = caches.find(ifName);
    if (caches.end() != iterator) {
        std::shared_ptr<ObserverCache> cache = iterator->second;
        if (0 == cache->RemoveObserver(observer)) {
            QCC_DbgPrintf(("Destroy observer cache for %s", ifName.c_str()));
            QStatus status = RemoveAnnounceHandler(ifName);
            if (ER_OK != status) {
                QCC_LogError(status, ("Failed to unregister announce handler"));
            }
            // remove MatchRule for signals on this interface
            qcc::String rule("type='signal',interface='");
            rule.append(ifName);
            rule.append("'");
            status = busConnection->GetBusAttachment().RemoveMatch(rule.c_str());
            QCC_DbgPrintf(("Remove match rule for %s", ifName.c_str()));
            caches.erase(ifName);
        }
    } else {
        QCC_LogError(ER_FAIL, ("Cache not found !"));
    }
    cachesMutex.Unlock();
}

const std::shared_ptr<ObserverCache> ObserverManager::GetCache(qcc::String ifName)
{
    cachesMutex.Lock();
    ObserverCacheMap::iterator iterator = caches.find(ifName);
    std::shared_ptr<ObserverCache> cache = nullptr;
    if (iterator != caches.end()) {
        cache = iterator->second;
    }
    cachesMutex.Unlock();
    return cache;
}

QStatus ObserverManager::AddAnnounceHandler(const qcc::String ifName)
{
    QStatus status = ER_OK;
    // Register the announce handler for the interface
    QCC_DbgPrintf(("Register announcement handler for interface '%s'", ifName.c_str()));
    const char* interfaceName = ifName.c_str();
    status = ajn::services::AnnouncementRegistrar::RegisterAnnounceHandler(
        busConnection->GetBusAttachment(), *this, &interfaceName, 1);
    return status;
}

QStatus ObserverManager::RemoveAnnounceHandler(const qcc::String ifName)
{
    QStatus status = ER_OK;
    // Unregister the announce handler for the interface
    QCC_DbgPrintf(("Unregister announcement handler for interface '%s'", ifName.c_str()));
    const char* interfaceName = ifName.c_str();
    status = ajn::services::AnnouncementRegistrar::UnRegisterAnnounceHandler(
        busConnection->GetBusAttachment(), *this, &interfaceName, 1);
    return status;
}

std::vector<std::weak_ptr<ObserverCache> > ObserverManager::GetObserverCaches(const std::vector<qcc::String>& ifNames)
const
{
    std::vector<std::weak_ptr<ObserverCache> > wcaches;
    for (qcc::String ifName : ifNames) {
        std::map<const qcc::String, std::shared_ptr<ObserverCache> >::const_iterator it = caches.find(ifName);
        if (it != caches.end()) {
            wcaches.push_back(it->second);
        }
    }

    return wcaches;
}

void ObserverManager::AddObject(const std::vector<qcc::String>& ifNames,
                                const ObjectId& objectId)
{
    QCC_DbgPrintf(("Start async task to add object (%s, %s)", objectId.GetBusName().c_str(),
                   objectId.GetBusObjectPath().c_str()));
    ObserverManagerTask* taskData =
        new ObserverManagerTask(GetObserverCaches(ifNames), objectId, Action::ADD);
    asyncTaskQueue.Enqueue(taskData);
}

void ObserverManager::RemoveObject(const std::vector<qcc::String>& ifNames,
                                   const ObjectId& objectId)
{
    QCC_DbgPrintf(("Start async task to remove object (%s, %s)", objectId.GetBusName().c_str(),
                   objectId.GetBusObjectPath().c_str()));
    ObserverManagerTask* taskData =
        new ObserverManagerTask(GetObserverCaches(ifNames), objectId, Action::REMOVE);
    asyncTaskQueue.Enqueue(taskData);
}

void ObserverManager::PopulateCache(std::shared_ptr<ObserverCache> cache,
                                    const qcc::String& ifName)
{
    objectsMutex.Lock();
    ObjectDescriptionsMap::const_iterator it = discoveredObjects.begin();
    while (it != discoveredObjects.end()) {
        ajn::SessionPort port = it->first.GetPort();
        qcc::String busName = it->first.GetBusName();
        ObjectDescriptions descriptions = it->second;
        ObjectDescriptions::const_iterator itDescription = descriptions.begin();
        while (itDescription != descriptions.end()) {
            const std::vector<qcc::String>& interfaces = itDescription->second;
            std::vector<qcc::String>::const_iterator itInterfaces =
                std::find(interfaces.begin(), interfaces.end(), ifName);
            if (interfaces.end() != itInterfaces) {
                if (sessionMgr->IsSessionEstablished(busName, port)) {
                    ajn::SessionId sessionId;
                    if (sessionMgr->GetSessionId(busName, port, sessionId)) {
                        ObjectId objId(busConnection->GetBusAttachment(), busName,
                                       itDescription->first, sessionId);
                        QCC_DbgPrintf(("Add object (%s, %s)", objId.GetBusName().c_str(),
                                       objId.GetBusObjectPath().c_str()));
                        cache->AddObject(objId);
                        sessionMgr->ReleaseSessionId(sessionId);
                    }
                }
            }
            itDescription++;
        }
        it++;
    }
    objectsMutex.Unlock();
}

void ObserverManager::ObjectDescriptionsDifference(const qcc::String& busName,
                                                   const ajn::SessionId& sessionId,
                                                   const ObjectDescriptions& odA,
                                                   const ObjectDescriptions& odB,
                                                   Action action)
{
    ObjectDescriptions difference;

    std::set_difference(odA.begin(), odA.end(), odB.begin(), odB.end(),
                        inserter(difference, difference.end()));

    for (ObjectDescriptions::const_iterator it = difference.begin();
         it != difference.end();
         ++it) {
        ObjectId objId(busConnection->GetBusAttachment(), busName, it->first, sessionId);
        const std::vector<qcc::String>& objectInterfaces = it->second;
        switch (action) {
        case Action::ADD:
            AddObject(objectInterfaces, objId);
            break;

        case Action::REMOVE:
            RemoveObject(objectInterfaces, objId);
            break;
        }
    }
}

void ObserverManager::Announce(unsigned short version,
                               ajn::SessionPort port,
                               const char* busName,
                               const ObjectDescriptions& objectDescs,
                               const AboutData& aboutData)
{
    QCC_DbgPrintf(("Received announcement from '%s'", busName));
    objectsMutex.Lock();

#if 0
    for (ObjectDescriptions::const_iterator it = objectDescs.begin();
         it != objectDescs.end();
         it++) {
        QCC_DbgPrintf(("-- for obj '%s'", it->first.c_str()));
        for (std::vector<qcc::String>::const_iterator itObj = it->second.begin();
             itObj != it->second.end();
             itObj++) {
            QCC_DbgPrintf(("---- interface '%s'", (*itObj).c_str()));
        }
    }
#endif

    // Ask session manager if a session already exists
    ajn::SessionId sessionId;
    SessionManager::Session session(busName, port);
    ObjectDescriptionsMap::iterator it = discoveredObjects.find(session);
    bool established = sessionMgr->IsSessionEstablished(busName, port);
    bool discovered = discoveredObjects.end() != it;
    if (!discovered || established) {
        if (sessionMgr->GetSessionId(busName, port, sessionId)) {
            QCC_DbgPrintf(("We already have a session in place with '%s'", busName));

            /* if there was already a session, this is definitely not the first announce */
            assert(discoveredObjects.find(session) != discoveredObjects.end());
            const ObjectDescriptions& old = discoveredObjects[session];

            ObjectDescriptionsDifference(busName, sessionId, old, objectDescs,
                                         Action::REMOVE);
            ObjectDescriptionsDifference(busName, sessionId, objectDescs, old, Action::ADD);
        }
        if (discovered) {
            sessionMgr->ReleaseSessionId(sessionId);
        }
    } else {
        QCC_DbgPrintf(("We have not yet got a session in place with '%s'", busName));
    }
    discoveredObjects[session] = objectDescs;

    objectsMutex.Unlock();
}

void ObserverManager::OnSessionEstablished(const SessionManager::Session& session,
                                           const ajn::SessionId& sessionId)
{
    QCC_DbgPrintf(("Session established for '%s'", session.GetBusName().c_str()));
    objectsMutex.Lock();
    ObjectDescriptions empty;
    ObjectDescriptionsMap::const_iterator it = discoveredObjects.find(session);
    assert(it != discoveredObjects.end());

    ObjectDescriptionsDifference(session.GetBusName(), sessionId, it->second, empty,
                                 Action::ADD);
    objectsMutex.Unlock();
}

void ObserverManager::OnSessionLost(const SessionManager::Session& session,
                                    const ajn::SessionId& sessionId)
{
    QCC_DbgPrintf(("Session lost for '%s'", session.GetBusName().c_str()));
    objectsMutex.Lock();
    ObjectDescriptions empty;
    ObjectDescriptionsMap::const_iterator it = discoveredObjects.find(session);
    assert(it != discoveredObjects.end());

    ObjectDescriptionsDifference(session.GetBusName(), sessionId, it->second, empty,
                                 Action::REMOVE);
    objectsMutex.Unlock();
}
}
