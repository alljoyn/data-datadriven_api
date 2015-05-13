/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
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

#include <algorithm>
#include <cassert>

#include <alljoyn/AutoPinger.h>

#include "SessionManager.h"

#include <qcc/Debug.h>
#include <qcc/Thread.h>
#define QCC_MODULE "DD_CONSUMER"

#define PING_GROUP "DDAPI"
#define PING_TIME 15

using namespace ajn;
using namespace datadriven;
using namespace qcc;

SessionManager::SessionManager(BusAttachment& ba) :
    errorStatus(ER_INIT_FAILED),
    shuttingDown(false),
    async(this, true),
    clientBusAttachment(ba),
    pingListener(new AutoPingListener(this))
{
    do {
        async.AsyncTaskQueue::Start();

        pingManager = std::unique_ptr<ajn::AutoPinger>(new ajn::AutoPinger(ba));
        if (NULL == pingManager) {
            QCC_LogError(ER_FAIL, ("Failed to get AutoPinger"));
            break;
        }
        pingManager->AddPingGroup(PING_GROUP, *pingListener, PING_TIME);

        errorStatus = ER_OK;
    } while (0);
}

SessionManager::~SessionManager()
{
    mutex.Lock(__FUNCTION__, __LINE__);
    // Set flag to indicate "shutting down" to prevent handling of new incomming session events (new/lost)
    shuttingDown = true;

    for (SessionVector::const_iterator it = sessions.begin(); it != sessions.end(); ++it) {
        // Cleanup sessions listeners
        clientBusAttachment.SetSessionListener((*it).GetId(), NULL);
    }
    mutex.Unlock(__FUNCTION__, __LINE__);
    async.Stop();
}

QStatus SessionManager::GetStatus() const
{
    mutex.Lock(__FUNCTION__, __LINE__);
    QStatus ret = errorStatus;
    if (shuttingDown) {
        ret = ER_FAIL;
    }
    mutex.Unlock(__FUNCTION__, __LINE__);
    return ret;
}

bool SessionManager::IsSessionEstablished(const qcc::String& uniqueBusName,
                                          const ajn::SessionPort port) const
{
    mutex.Lock(__FUNCTION__, __LINE__);
    SessionVector::const_iterator it = std::search_n(sessions.begin(), sessions.end(), 1,
                                                     Session(uniqueBusName, port), SessionManager::SessionComp);
    bool sessionEstablished = (it != sessions.end()) && (*it).IsEstablished();
    mutex.Unlock(__FUNCTION__, __LINE__);
    return sessionEstablished;
}

bool SessionManager::GetSessionId(const qcc::String& uniqueBusName,
                                  const ajn::SessionPort port,
                                  ajn::SessionId& sessionId)
{
    bool sessionEstablished = false;

    mutex.Lock(__FUNCTION__, __LINE__);
    do {
        if (ER_OK != errorStatus) {
            QCC_LogError(errorStatus, ("Session manager not properly initialized"));
            break;
        }
        if (shuttingDown) {
            QCC_LogError(ER_FAIL, ("Session manager is shutting down"));
            break;
        }

        Session session(uniqueBusName, port);
        SessionVector::iterator sessionIt = std::search_n(sessions.begin(), sessions.end(), 1,
                                                          session, SessionManager::SessionComp);
        if (sessionIt != sessions.end()) {
            // Increment refcount
            (*sessionIt).Increment();
            QCC_DbgPrintf(("Session (%sestablished) found for %s / %d",
                           (*sessionIt).IsEstablished() ? "" : "not yet", uniqueBusName.c_str(), port));
            sessionEstablished = (*sessionIt).IsEstablished();
            if (sessionEstablished) {
                // Retrieve sessionId
                sessionId = (*sessionIt).GetId();
            }
        } else {
            QCC_DbgPrintf(("Session (not yet established) added for %s / %d", uniqueBusName.c_str(), port));
            SessionVector::iterator it = std::search_n(sessions.begin(), sessions.end(), 1,
                                                       session, SessionManager::BusNameComp);

            // Add the new session, refcount = 1
            if (it == sessions.end()) {
                sessions.push_back(session);
                //Add sleep to make sure the first ping can succeed
                //See ASACORE-1995
                qcc::Sleep(500);
                pingManager->AddDestination(PING_GROUP, uniqueBusName);
            } else {
                Session* newSession = new Session(session);
                SessionVector::iterator newIt = sessions.insert(sessions.begin(), session);
                if (ER_OK != JoinSession(newSession)) {
                    sessions.erase(newIt);
                    delete newSession;
                }
            }
        }
    } while (0);
    mutex.Unlock(__FUNCTION__, __LINE__);

    return sessionEstablished;
}

void SessionManager::ReleaseSessionId(ajn::SessionId& sessionId)
{
    mutex.Lock(__FUNCTION__, __LINE__);
    SessionVector::iterator sessionIt = std::search_n(sessions.begin(), sessions.end(), 1,
                                                      Session(sessionId), SessionIdComp);
    if (sessions.end() != sessionIt) {
        // Decrement refcount
        if (0 == (*sessionIt).Decrement()) {
            async.Enqueue(new LeaveSessionData((*sessionIt).GetId()));
            qcc::String busName = (*sessionIt).GetBusName();
            pingManager->RemoveDestination(PING_GROUP, busName);
            sessions.erase(sessionIt);
        }
    }
    mutex.Unlock(__FUNCTION__, __LINE__);
}

void SessionManager::RegisterListener(SessionManager::Listener* listener)
{
    mutex.Lock(__FUNCTION__, __LINE__);
    if (ER_OK != errorStatus) {
        QCC_LogError(errorStatus, ("Session manager not properly initialized"));
    } else if (shuttingDown) {
        QCC_LogError(ER_FAIL, ("Session manager is shutting down"));
    } else {
        sessionListeners.insert(listener);
    }
    mutex.Unlock(__FUNCTION__, __LINE__);
}

void SessionManager::UnregisterListener(SessionManager::Listener* listener)
{
    mutex.Lock(__FUNCTION__, __LINE__);
    if (ER_OK != errorStatus) {
        QCC_LogError(errorStatus, ("Session manager not properly initialized"));
    } else if (shuttingDown) {
        QCC_LogError(ER_FAIL, ("Session manager is shutting down"));
    } else {
        sessionListeners.erase(listener);
    }
    mutex.Unlock(__FUNCTION__, __LINE__);
}

void SessionManager::JoinSessionCB(QStatus status, SessionId id,
                                   const SessionOpts& opts,
                                   void* context)
{
    Session* session = static_cast<Session*>(context);

    mutex.Lock(__FUNCTION__, __LINE__);
    do {
        if (ER_OK != errorStatus) {
            QCC_LogError(errorStatus, ("Session manager not properly initialized"));
            break;
        }
        if (shuttingDown) {
            QCC_LogError(ER_FAIL, ("Session manager is shutting down"));
            break;
        }
        if (ER_OK != status) {
            QCC_LogError(status, ("Session could not be joined"));
            break;
        }

        QCC_DbgPrintf(("Session %lu established with %s", (unsigned long)id, session->GetBusName().c_str()));
        SessionVector::iterator sessionIt = std::search_n(sessions.begin(), sessions.end(), 1,
                                                          *session, SessionManager::SessionComp);
        (*sessionIt).Established(true);
        (*sessionIt).SetId(id);

        // Inform about new session
        for (SessionListenerSet::iterator it = sessionListeners.begin(); it != sessionListeners.end(); it++) {
            (*it)->OnSessionEstablished(*session, id);
        }
    } while (0);
    mutex.Unlock(__FUNCTION__, __LINE__);
    delete session;
}

void SessionManager::SessionLost(ajn::SessionId sessionId, SessionLostReason reason)
{
    mutex.Lock(__FUNCTION__, __LINE__);
    do {
        QCC_DbgPrintf(("Lost session %lu", (unsigned long)sessionId));

        if (ER_OK != errorStatus) {
            QCC_LogError(errorStatus, ("Session manager not properly initialized"));
            break;
        }
        if (shuttingDown) {
            QCC_LogError(ER_FAIL, ("Session manager is shutting down"));
            break;
        }

        SessionVector::iterator sessionIt = std::search_n(sessions.begin(), sessions.end(), 1,
                                                          Session(sessionId), SessionIdComp);
        if (sessionIt == sessions.end()) {
            /* If we get here, we have probably scheduled a LeaveSessionData an instant ago to leave this session ourselves
             * No need to do here anything... */
            break;
        }

        // Inform about our loss
        for (SessionListenerSet::iterator it = sessionListeners.begin(); it != sessionListeners.end(); it++) {
            (*it)->OnSessionLost(*sessionIt, sessionId);
        }
        (*sessionIt).Established(false);
        (*sessionIt).SetId(0);
    } while (0);
    mutex.Unlock(__FUNCTION__, __LINE__);
}

bool SessionManager::BusNameComp(Session left, Session right)
{
    return (left.GetBusName() == right.GetBusName());
}

bool SessionManager::SessionIdComp(Session left, Session right)
{
    return (left.GetId() == right.GetId());
}

bool SessionManager::SessionComp(Session left, Session right)
{
    return (left.GetBusName() == right.GetBusName()) && (left.GetPort() == right.GetPort());
}

void SessionManager::OnEmptyQueue()
{
}

QStatus SessionManager::JoinSession(Session* session)
{
    SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);

    return clientBusAttachment.JoinSessionAsync(session->GetBusName().c_str(), session->GetPort(),
                                                this, opts, this, (void*)session);
}

void SessionManager::OnTask(TaskData const* taskdata)
{
    const LeaveSessionData* lsd = static_cast<const LeaveSessionData*>(taskdata);

    errorStatus = clientBusAttachment.LeaveJoinedSession(lsd->GetSessionId());
    if (errorStatus != ER_OK) {
        QCC_LogError(errorStatus, ("Failed to leave session"));
        return;
    }
}

void SessionManager::AutoPingListener::DestinationFound(const qcc::String& group,
                                                        const qcc::String& destination)
{
    QCC_DbgHLPrintf(("Destination found '%s'", destination.c_str()));

    if ((NULL == sessionMgr) || ER_OK != (sessionMgr->GetStatus())) {
        QCC_LogError(ER_FAIL, ("Session manager not properly initialized or is shutting down"));
        return;
    }

    sessionMgr->mutex.Lock(__FUNCTION__, __LINE__);

    // Set session parameters and join it
    for (SessionManager::SessionVector::iterator itSession = sessionMgr->sessions.begin();
         itSession != sessionMgr->sessions.end(); itSession++) {
        if (((*itSession).GetBusName() == destination) && !(*itSession).IsEstablished()) {
            QCC_DbgHLPrintf(("Should establish session with '%s'", destination.c_str()));
            Session* session = new Session(*itSession);
            QStatus status = sessionMgr->JoinSession(session);
            if (status != ER_OK) {
                QCC_LogError(status, ("Failed to establish session"));
                delete session;
            }
        }
    }

    sessionMgr->mutex.Unlock(__FUNCTION__, __LINE__);
}

void SessionManager::AutoPingListener::DestinationLost(const qcc::String& group,
                                                       const qcc::String& destination)
{
    QCC_DbgHLPrintf(("Destination lost '%s'", destination.c_str()));

    if ((NULL == sessionMgr) || ER_OK != (sessionMgr->GetStatus())) {
        QCC_LogError(ER_FAIL, ("Session manager not properly initialized or is shutting down"));
        return;
    }

    sessionMgr->mutex.Lock(__FUNCTION__, __LINE__);

    for (SessionManager::SessionVector::iterator itSession = sessionMgr->sessions.begin();
         itSession != sessionMgr->sessions.end(); itSession++) {
        if (((*itSession).GetBusName() == destination) && (*itSession).IsEstablished()) {
            // Inform about our loss
            for (SessionManager::SessionListenerSet::iterator it = sessionMgr->sessionListeners.begin();
                 it != sessionMgr->sessionListeners.end();
                 it++) {
                (*it)->OnSessionLost(*itSession, (*itSession).GetId());
            }

            // Close and Cleanup session
            sessionMgr->async.Enqueue(new LeaveSessionData((*itSession).GetId()));
            (*itSession).Established(false);
            (*itSession).SetId(0);
        }
    }

    sessionMgr->mutex.Unlock(__FUNCTION__, __LINE__);
}
