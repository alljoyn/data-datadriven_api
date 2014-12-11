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

#ifndef OBSERVERMANAGER_H_
#define OBSERVERMANAGER_H_

#include <map>
#include <memory>

#include <alljoyn/about/AnnounceHandler.h>
#include <alljoyn/services_common/AsyncTaskQueue.h>

#include <qcc/String.h>
#include <datadriven/Mutex.h>

#include "ObserverCache.h"
#include "SessionManager.h"

namespace datadriven {
class BusConnectionImpl;

class ObserverManager :
    private ajn::services::AnnounceHandler,
    private ajn::services::AsyncTask,
    private SessionManager::Listener {
  public:
    enum class Action :
        int8_t {
        ADD, REMOVE
    };

    static std::shared_ptr<ObserverManager> GetInstance(std::shared_ptr<BusConnectionImpl> busConn = nullptr);

    ~ObserverManager();

    QStatus GetStatus() const;

    /**
     * Stop the observer manager by unregistering all announce handlers and
     * unregistering to the session manager
     */
    void Stop();

    /**
     * Return a shared pointer to the cache for interface named \a ifName
     *
     * \param ifName the interface name
     */
    const std::shared_ptr<ObserverCache> GetCache(qcc::String ifName);

    /**
     * Adds a new cache for interface with name \a ifName if it does not already
     * exist.
     *
     * \param ifName the interface name
     * \return the status code
     */
    QStatus RegisterObserver(std::weak_ptr<ObserverBase> observer,
                             qcc::String ifName);

    /**
     * Remove the \a observer linked to the cache for interface with name \a ifName.
     * If there are no more observers linked to the cache, remove it.
     *
     * \param observer the observer that must be detached from the cache
     * \param ifName the interface name
     */
    void UnregisterObserver(std::weak_ptr<ObserverBase> observer,
                            qcc::String ifName);

    // Asynchronous task handling

    class Task :
        public ajn::services::TaskData {
      public:
        virtual void Execute() const = 0;
    };

    void Enqueue(const Task* task);

  private:
    /**
     * Private constructor since this is a singleton.
     */
    ObserverManager(std::shared_ptr<BusConnectionImpl> busConnection);

    /**
     * Observer manager's last status.
     */
    QStatus status;

    /**
     * The map of discovered objects with Session class (contains busName and port) as the key and
     * the object descriptions as value
     */
    typedef std::map<SessionManager::Session, ObjectDescriptions> ObjectDescriptionsMap;
    ObjectDescriptionsMap discoveredObjects;

    /**
     * Mutex that protects access to the discovered objects map
     */
    datadriven::Mutex objectsMutex;

    /**
     * The observer cache per interface name keeping track of all proxy interface instances
     * and all observers for that interface
     */
    typedef std::map<const qcc::String, std::shared_ptr<ObserverCache> > ObserverCacheMap;
    ObserverCacheMap caches;

    /**
     * Mutex that protects access to the observer cache map
     */
    datadriven::Mutex cachesMutex;

    /**
     * Reference to busConnection
     */
    std::shared_ptr<BusConnectionImpl> busConnection;

    /**
     * The session manager
     */
    SessionManager* sessionMgr;

    /**
     * Asynchronous task queue for consumer related actions.
     */
    mutable ajn::services::AsyncTaskQueue asyncTaskQueue;

    /**
     * Register an new about announce handler for a given \a ifName
     *
     * \param ifName the interface name
     * \return the status of the registration
     */
    QStatus AddAnnounceHandler(const qcc::String ifName);

    /**
     * Unregister an about announce handler for a given \a ifName
     *
     * \param ifName the interface name
     * \return the status of the unregistration
     */
    QStatus RemoveAnnounceHandler(const qcc::String ifName);

    /**
     * Add a new object (this method will enqueue a new async task object)
     *
     * \param ifName the interfaces of the object
     * \param objId the object id
     */
    void AddObject(const std::vector<qcc::String>& ifName,
                   const ObjectId& objId);

    /**
     * Remove object (this method will enqueue a new async task object)
     *
     * \param ifName the interface name
     * \param objId the object id
     */
    void RemoveObject(const std::vector<qcc::String>& ifName,
                      const ObjectId& objId);

    /**
     * Populate a new created cache with possibly already discovered objects
     * This is part of late joining functionality
     */
    void PopulateCache(std::shared_ptr<ObserverCache> cache,
                       const qcc::String& ifName);

    /* Subtract B from A (so A-B) and call onChange for all interesting objects */
    void ObjectDescriptionsDifference(const qcc::String& busName,
                                      const ajn::SessionId& sessionId,
                                      const ObjectDescriptions& odA,
                                      const ObjectDescriptions& odB,
                                      Action action);

    /**
     * Derived from ajn::services::AnnounceHandler
     * Called when a new peer is announced on about
     */
    virtual void Announce(unsigned short version,
                          ajn::SessionPort port,
                          const char* busName,
                          const ObjectDescriptions& objectDescs,
                          const AboutData& aboutData);

    /**
     * Derived from the SessionManagerListener
     * Called when a new session has been established with \a busName
     */
    void OnSessionEstablished(const SessionManager::Session& session,
                              const ajn::SessionId& sessionId);

    /**
     * Derived from the SessionManagerListener
     * Called when a session with \a busName has been lost
     */
    void OnSessionLost(const SessionManager::Session& session,
                       const ajn::SessionId& sessionId);

    /* ajn::services::AsyncTask */
    virtual void OnEmptyQueue();

    virtual void OnTask(ajn::services::TaskData const* taskdata);

    std::vector<std::weak_ptr<ObserverCache> > GetObserverCaches(const std::vector<qcc::String>& ifNames) const;
};
}

#endif /* OBSERVERMANAGER_H_ */
