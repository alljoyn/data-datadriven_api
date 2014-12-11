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

#ifndef SESSIONMANAGER_H_
#define SESSIONMANAGER_H_

#include <map>
#include <set>
#include <memory>

#include <qcc/String.h>
#include <datadriven/Mutex.h>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/Session.h>
#include <alljoyn/about/AboutClient.h>
#include <alljoyn/services_common/AsyncTaskQueue.h>
#include <alljoyn/PingListener.h>
#include <alljoyn/AutoPinger.h>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
class SessionManager :
    private ajn::SessionListener,
    private ajn::BusAttachment::JoinSessionAsyncCB,
    private ajn::services::AsyncTask {
  public:
    /**
     * Constructs the session manager
     *
     * \param[in] ba The bus attachment
     */
    SessionManager(ajn::BusAttachment& ba);

    /**
     * Virtual destructor.
     */
    virtual ~SessionManager();

    /**
     * Return the status of this class. The status can be checked at all times.
     * This method can typically be called after construction of an object of this class
     * in order to check if construction went well.
     *
     * \return the status of the session manager
     */
    QStatus GetStatus() const;

    /**
     * Check if a session has been setup.
     *
     * \param[in] uniqueBusName the bus name
     * \param[in] port the session port
     * \retval true the session was already setup
     * \retval false the session was not yet setup
     */
    bool IsSessionEstablished(const qcc::String& uniqueBusName,
                              const ajn::SessionPort port) const;

    /**
     * Check if a session has been setup. If it has been setup, return true. If not,
     * start session setup asynchronous and return false.
     * The \a sessionId for the given \a uniqueBusName is only valid if the return value is true.
     *
     * \param[in] uniqueBusName the bus name
     * \param[in] port the session port
     * \param[out] sessionId the corresponding session id
     * \retval true the session was already setup
     * \retval false the session was not yet setup
     */
    bool GetSessionId(const qcc::String& uniqueBusName,
                      const ajn::SessionPort port,
                      ajn::SessionId& sessionId);

    /**
     * Inform the session manager that a certain user is not interested anymore in \a sessionId
     * If all users have released the \a sessionId, the session can be closed.
     *
     * \param[in] sessionId the session id to be released
     */
    void ReleaseSessionId(ajn::SessionId& sessionId);

    /**
     * \class SessionManager::Session
     * \brief This class contains all necessary information about a session
     */
    class Session {
      public:
        Session() :
            port(0), id(0), sessionEstablished(false), refCount(1) { }

        Session(ajn::SessionId id) :
            port(0), id(id), sessionEstablished(false), refCount(1) { }

        Session(qcc::String busName,
                ajn::SessionPort port) :
            busName(busName), port(port), id(0), sessionEstablished(false), refCount(1) { }

        /**
         * Overloading the operator<
         * \param[in] other the other Session object
         * \return true if this object precedes \a other
         */
        bool operator<(Session other) const
        {
            if (busName == other.busName) {
                return (port < other.port);
            }
            return (busName < other.busName);
        }

        /**
         * \return the session bus name
         */
        const qcc::String& GetBusName() const { return busName; }

        /**
         * \return the session port
         */
        ajn::SessionPort GetPort() const { return port; }

        /**
         * \return the session id
         */
        ajn::SessionId GetId() const { return id; }

        /**
         * Check if the session was established
         * \return true if the session is established
         */
        bool IsEstablished() const { return sessionEstablished; }

        /**
         * Change the sessionEstablished state
         * \param[in] established the new state
         */
        void Established(bool established) { sessionEstablished = established; }

        /**
         * Sets the session Id
         * \param the session id
         */
        void SetId(ajn::SessionId id) { this->id = id; }

        /**
         * Increment ref count
         * \return the new ref count
         */
        unsigned int Increment() { return ++refCount; }

        /**
         * Decrement ref count
         * \return the new ref count
         */
        unsigned int Decrement() { return --refCount; }

      private:
        qcc::String busName;
        ajn::SessionPort port;
        ajn::SessionId id;
        bool sessionEstablished;
        unsigned int refCount;
    };

    /**
     * \class SessionManager::Listener
     * \brief Used to register to the session manager in order to be notified when sessions
     *        are being established or lost.
     */
    class Listener {
      public:
        /**
         * Virtual destructor.
         */
        virtual ~Listener() { }

        /**
         * Called when a new session has been established. The session and session id
         * that identify the session are passed to the implementer of the listener object.
         *
         * \param[in] session the session
         * \param[in] sessionId the sessionId of the session
         */
        virtual void OnSessionEstablished(const Session& session,
                                          const ajn::SessionId& sessionId) = 0;

        /**
         * Called when a session has been lost. The session and session id
         * that identify the session are passed to the implementer of the listener object.
         *
         * \param[in] session the session
         * \param[in] sessionId the sessionId of the session
         */
        virtual void OnSessionLost(const Session& session,
                                   const ajn::SessionId& sessionId) = 0;
    };

    /**
     * Register \a listener to the session manager. The caller of this method will now
     * be notified when new sessions have been established or lost.
     *
     * \param[in] listener the new listener to be registered
     */
    void RegisterListener(SessionManager::Listener* listener);

    /**
     * Unregister \a listener to the session manager. The caller of this method will no longer
     * be notified when sessions are established or lost.
     *
     * \param[in] listener the new listener to be registered
     */
    void UnregisterListener(SessionManager::Listener* listener);

  private:
    class LeaveSessionData;

    /**
     * SessionListenerSet simply keeps the list of registered ::SessionManager::Listener
     */
    typedef std::set<SessionManager::Listener*> SessionListenerSet;
    SessionListenerSet sessionListeners;

    /**
     * SessionVector keeps track of the sessions.
     */
    typedef std::vector<Session> SessionVector;
    SessionVector sessions;

    /**
     * Internal mutex to protect internal data structures from concurrent access
     */
    mutable datadriven::Mutex mutex;

    /**
     * The status of the session manager.
     */
    QStatus errorStatus;

    /**
     * An asynchronous task queue that runs tasks on behalf of the session manager or one of its users.
     */
    ajn::services::AsyncTaskQueue async;

    /**
     * The corresponding bus attachment this session manager uses to set up sessions to others.
     */
    ajn::BusAttachment& clientBusAttachment;

    /**
     * \class AutoPingListener
     * \brief This class implements the PingListener. It will be notified when
     *        destinations have been found or lost.
     */
    class AutoPingListener :
        public ajn::PingListener {
      private:
        SessionManager* sessionMgr;

      public:
        /**
         * Constructor of the AutoPingListener
         *
         * \param[in] sessionManager a pointer to the related session manager
         */
        AutoPingListener(SessionManager* sesionManager) :
            sessionMgr(sesionManager) { }

        /**
         * Virtual destructor
         */
        virtual ~AutoPingListener() { }

        /**
         * Called when the AutoPinger finds a new \a destination for a certain \a group
         *
         * \param[in] group the ping group
         * \param[in] destination the new discovered destination
         */
        virtual void DestinationFound(const qcc::String& group,
                                      const qcc::String& destination);

        /**
         * Called when the AutoPinger looses connection with \a destination for a certain \a group
         *
         * \param[in] group the ping group
         * \param[in] destination the lost destination
         */
        virtual void DestinationLost(const qcc::String& group,
                                     const qcc::String& destination);
    };
    /**
     * Let the AutoPingListener call private methods of this class.
     */
    friend class AutoPingListener;

    /**
     * The ping listener for this session manager.
     */
    std::unique_ptr<AutoPingListener> pingListener;

    /**
     * The ping manager used by the session manager. The pingListener will be registered to it.
     */
    std::unique_ptr<ajn::AutoPinger> pingManager;

    /**
     * Called when JoinSessionAsync() completes.
     *
     * \param status ER_OK if successful
     * \param sessionId Unique identifier for session.
     * \param opts Session options.
     * \param context User defined context which will be passed as-is to callback.
     */
    virtual void JoinSessionCB(QStatus status,
                               ajn::SessionId id,
                               const ajn::SessionOpts& opts,
                               void* context);

    /**
     * Called by the bus when an existing session becomes disconnected.
     *
     * \param sessionId Id of session that was lost.
     * \param reason The reason for the session being lost
     */
    virtual void SessionLost(ajn::SessionId sessionId,
                             SessionLostReason reason);

    /**
     * Join a session characterized by \a session
     *
     * \param[in] session the session to be set up
     * \return ER_OK when successful
     */
    QStatus JoinSession(Session* session);

    /**
     * Compare sessions based on the busName
     */
    static bool BusNameComp(Session left,
                            Session right);

    /**
     * Compare sessions based on the session Id
     */
    static bool SessionIdComp(Session left,
                              Session right);

    /**
     * Compare sessions based on busName and port
     */
    static bool SessionComp(Session left,
                            Session right);

    class LeaveSessionData :
        public ajn::services::TaskData {
      private:
        ajn::SessionId sessionId;

      public:
        LeaveSessionData(ajn::SessionId _sessionId) :
            sessionId(_sessionId) { }

        ajn::SessionId GetSessionId() const { return sessionId; }
    };

    /**
     * Called when the asynchronous task queue becomes empty.
     * Nothing to be done by the session manager at the moment.
     */
    virtual void OnEmptyQueue();

    /**
     * Called when a new task on the asynchronous task queue needs to be handled.
     */
    virtual void OnTask(ajn::services::TaskData const* taskdata);
};
} /* namespace datadriven */
#undef QCC_MODULE
#endif /* SESSIONMANAGER_H_ */
