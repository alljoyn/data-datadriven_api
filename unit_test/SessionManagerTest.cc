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

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include <alljoyn/about/AboutServiceApi.h>
#include <alljoyn/about/AboutPropertyStoreImpl.h>

#include <datadriven/Semaphore.h>

#include "SessionManager.h"
#include "BusConnectionImpl.h"

/**
 * Session manager tests.
 */
namespace test_unit_sessionmanager {
using namespace std;
using namespace ajn;
using namespace services;
using namespace datadriven;

#define IFACE "org.allseenalliance.test.SessionTest"
#define PROP "Property"

class Provider :
    public SessionPortListener {
  public:
    Provider(const char* busName,
             SessionPort port) :
        bus("Provider", true),
        store()
    {
        cout << "Provider for " << busName << " on port " << port << endl;
        EXPECT_EQ(ER_OK, bus.Start());
        EXPECT_EQ(ER_OK, bus.Connect());
        QStatus status = bus.RequestName(
            busName,
            DBUS_NAME_FLAG_ALLOW_REPLACEMENT | DBUS_NAME_FLAG_REPLACE_EXISTING |
            DBUS_NAME_FLAG_DO_NOT_QUEUE);
        if (ER_DBUS_REQUEST_NAME_REPLY_EXISTS == status) {
            cout << "Same provider name re-used" << endl;
        } else {
            EXPECT_EQ(ER_OK, status);
        }
        SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_PHYSICAL, TRANSPORT_ANY);
        EXPECT_EQ(ER_OK, bus.BindSessionPort(port, opts, *this));
        // about set up
        about = new AboutService(bus, store);
        EXPECT_TRUE(NULL != about);
        about->Register(port);
    }

    ~Provider()
    {
        bus.Disconnect();
        bus.Stop();
        bus.Join();
        delete about;
    }

    virtual bool AcceptSessionJoiner(SessionPort sessionPort, const char* joiner, const SessionOpts& opts)
    {
        cout << "AcceptSessionJoiner " << joiner << endl;
        return true;
    }

  private:
    BusAttachment bus;
    AboutPropertyStoreImpl store;
    AboutService* about;
};

class SessionManagerTest :
    public::testing::Test {
  protected:
    virtual void SetUp()
    {
        busConn = BusConnectionImpl::GetInstance();
        if (nullptr == busConn) {
            cout << "BusConnection could not be created" << endl;
        }
    }

    virtual void TearDown()
    {
        busConn = nullptr;
    }

    shared_ptr<BusConnectionImpl> busConn;
};

class TestSessionListener :
    public SessionManager::Listener {
  public:
    Semaphore sem;

    virtual void OnSessionEstablished(const SessionManager::Session& session,
                                      const ajn::SessionId& sessionId)
    {
        cout << "Session established for: " << session.GetBusName().c_str() << "/" << session.GetPort() << endl;
        sessions[session] = sessionId;
        sem.Post();
    }

    virtual void OnSessionLost(const SessionManager::Session& session,
                               const ajn::SessionId& sessionId)
    {
        cout << "Session lost for: " << session.GetBusName().c_str() << "/" << session.GetPort() << endl;
        sessions.erase(session);
        sem.Post();
    }

    bool Exists(const char* busName, ajn::SessionPort port)
    {
        SessionManager::Session session(busName, port);
        map<SessionManager::Session, ajn::SessionId>::iterator found = sessions.find(session);
        return (found != sessions.end());
    }

  private:
    map<SessionManager::Session, ajn::SessionId> sessions;
};

/**
 * \test Test trying to release a non-existent session
 *       -# Release a session that was not requested
 *       -# Verify proper situation handling
 * */
TEST_F(SessionManagerTest, ReleaseNonExistingSession) {
    SessionManager sessionMgr(busConn->GetBusAttachment());
    ajn::SessionId id = 1234;
    sessionMgr.ReleaseSessionId(id);
}

/**
 * \test Test checking a non-existent session
 * */
TEST_F(SessionManagerTest, CheckNonExistingSession) {
    SessionManager sessionMgr(busConn->GetBusAttachment());
    EXPECT_FALSE(sessionMgr.IsSessionEstablished("Test", 135));
}

/**
 * \test Test setting up a session to a non-existent peer
 * */
TEST_F(SessionManagerTest, SetupSessionNoPeer) {
    SessionManager sessionMgr(busConn->GetBusAttachment());
    const char* busName = "test.Provider";
    ajn::SessionPort port = 5050;
    ajn::SessionId sessionId = 0;
    EXPECT_FALSE(sessionMgr.GetSessionId(busName, port, sessionId));
    EXPECT_EQ(0u, sessionId);
}

/**
 * \test Test checking an existent session
 *       -# Start session manager, register listener
 *       -# Check if a certain session is set up (false)
 *       -# Start provider
 *       -# Wait until listener callback is called
 *       -# Check if session is established
 *       -# Check if session id is not 0
 *       -# Check if the listener was called for the correct session
 *       -# Try setting up a second session to the same peer with different port
 * */
TEST_F(SessionManagerTest, SetupSession) {
    SessionManager sessionMgr(busConn->GetBusAttachment());
    TestSessionListener listener;
    const char* busName = "test.Provider";
    ajn::SessionPort port = 5050;
    ajn::SessionId sessionId = 0;
    sessionMgr.RegisterListener(&listener);
    EXPECT_FALSE(sessionMgr.GetSessionId(busName, port, sessionId));
    EXPECT_EQ(0u, sessionId);
    Provider provider(busName, port);

    listener.sem.Wait();
    EXPECT_TRUE(sessionMgr.IsSessionEstablished(busName, port));
    EXPECT_TRUE(sessionMgr.GetSessionId(busName, port, sessionId));
    EXPECT_NE(0u, sessionId);
    EXPECT_TRUE(listener.Exists(busName, port));
    EXPECT_FALSE(sessionMgr.IsSessionEstablished(busName, port + 1));
    EXPECT_FALSE(sessionMgr.GetSessionId(busName, port + 1, sessionId));
}

/**
 * \test Test session refcounting
 *       -# Start session manager, register listener
 *       -# Check if a certain session is set up (false)
 *       -# Start provider
 *       -# Wait until listener callback is called
 *       -# Check if session is established
 *       -# Check if session id is not 0
 *       -# Check if the listener was called for the correct session
 * */
TEST_F(SessionManagerTest, SessionRefCounting) {
    SessionManager sessionMgr(busConn->GetBusAttachment());
    TestSessionListener listener;
    const char* busName = "test.Provider";
    ajn::SessionPort port = 5050;
    ajn::SessionId sessionId = 0;
    sessionMgr.RegisterListener(&listener);
    EXPECT_FALSE(sessionMgr.GetSessionId(busName, port, sessionId)); // refcount = 1
    EXPECT_FALSE(sessionMgr.GetSessionId(busName, port, sessionId)); // refcount = 2
    EXPECT_EQ(0u, sessionId);
    Provider provider(busName, port);

    listener.sem.Wait();
    EXPECT_TRUE(sessionMgr.IsSessionEstablished(busName, port));
    EXPECT_TRUE(sessionMgr.GetSessionId(busName, port, sessionId)); // refcount = 3
    EXPECT_NE(0u, sessionId);
    EXPECT_TRUE(listener.Exists(busName, port));
    sessionMgr.ReleaseSessionId(sessionId); // refcount = 2
    EXPECT_TRUE(sessionMgr.IsSessionEstablished(busName, port));
    sessionMgr.ReleaseSessionId(sessionId); // refcount = 1
    EXPECT_TRUE(sessionMgr.IsSessionEstablished(busName, port));
    sessionMgr.ReleaseSessionId(sessionId); // refcount = 0
    EXPECT_FALSE(sessionMgr.IsSessionEstablished(busName, port));
}

/**
 * \test Test checking a non-existent session
 *       -# Start session manager, register listener
 *       -# Start several peers with the same busName
 *       -# Setup a session towards each one of them
 *       -# Wait until listener callback is called for all of the peers
 *       -# Check if sessions are established
 *       -# Check if session ids are not 0
 *       -# Check if the listener was called for the correct sessions
 * */
TEST_F(SessionManagerTest, SetupMultipleSessionSamePeer) {
    SessionManager sessionMgr(busConn->GetBusAttachment());
    TestSessionListener listener;
    int beginPort = 5050;
    int endPort = 5055;
    Provider* provider[endPort - beginPort];
    const char* busName = "test.Provider";
    ajn::SessionId sessionId = 0;
    sessionMgr.RegisterListener(&listener);

    for (int port = beginPort, i = 0; port < endPort; port++, i++) {
        provider[i] = new Provider(busName, port);
        EXPECT_FALSE(sessionMgr.GetSessionId(busName, port, sessionId));
        listener.sem.Wait();
    }

    for (int port = beginPort; port < endPort; port++) {
        EXPECT_TRUE(sessionMgr.IsSessionEstablished(busName, port));
        EXPECT_TRUE(sessionMgr.GetSessionId(busName, port, sessionId));
        EXPECT_NE(0u, sessionId);
        EXPECT_TRUE(listener.Exists(busName, port));
    }

    for (int port = beginPort, i = 0; port < endPort; port++, i++) {
        delete provider[i];
        listener.sem.Wait();
        sessionId = 0u;
        EXPECT_FALSE(sessionMgr.IsSessionEstablished(busName, port));
        EXPECT_FALSE(sessionMgr.GetSessionId(busName, port, sessionId));
        EXPECT_EQ(0u, sessionId);
        EXPECT_FALSE(listener.Exists(busName, port));
    }
}

/**
 * \test Test checking a non-existent session
 *       -# Start session manager, register listener
 *       -# Start several providers
 *       -# Wait until listener callback is called for all of the peers
 *       -# Check if sessions are established
 *       -# Check if session ids are not 0
 *       -# Check if the listener was called for the correct sessions
 * */
TEST_F(SessionManagerTest, SetupMultipleSessionDifferentPeers) {
    SessionManager sessionMgr(busConn->GetBusAttachment());
    TestSessionListener listener;
    int beginPort = 5050;
    int endPort = 5055;
    Provider* provider[endPort - beginPort];
    ajn::SessionId sessionId = 0;
    sessionMgr.RegisterListener(&listener);

    for (int port = beginPort, i = 0; port < endPort; port++, i++) {
        qcc::String busName("test.Provider.");
        busName.append('a' + i);
        provider[i] = new Provider(busName.c_str(), port);
        EXPECT_FALSE(sessionMgr.GetSessionId(busName, port, sessionId));
        listener.sem.Wait();
    }

    for (int port = beginPort, i = 0; port < endPort; port++, i++) {
        qcc::String busName("test.Provider.");
        busName.append('a' + i);
        EXPECT_TRUE(sessionMgr.IsSessionEstablished(busName.c_str(), port));
        EXPECT_TRUE(sessionMgr.GetSessionId(busName, port, sessionId));
        EXPECT_NE(0u, sessionId);
        EXPECT_TRUE(listener.Exists(busName.c_str(), port));
    }

    for (int port = beginPort, i = 0; port < endPort; port++, i++) {
        qcc::String busName("test.Provider.");
        busName.append('a' + i);
        delete provider[i];
        listener.sem.Wait();
        sessionId = 0u;
        EXPECT_FALSE(sessionMgr.IsSessionEstablished(busName.c_str(), port));
        EXPECT_FALSE(sessionMgr.GetSessionId(busName, port, sessionId));
        EXPECT_EQ(0u, sessionId);
        EXPECT_FALSE(listener.Exists(busName.c_str(), port));
    }
}

/**
 * \test Test trying to unregister a non-existent listener
 *       -# Unregister a listener that was not requested
 *       -# Verify proper situation handling
 * */
TEST_F(SessionManagerTest, ReleaseNonExistingListener) {
    SessionManager sessionMgr(busConn->GetBusAttachment());
    TestSessionListener listener;
    sessionMgr.UnregisterListener(&listener);
}
}
//namespace
