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

#include <datadriven/datadriven.h>
#include <datadriven/Semaphore.h>

#include "MethodsInterface.h"
#include "MethodsProxy.h"

using namespace std;
using namespace gen::org_allseenalliance_test;

/**
 * Methods tests.
 */
namespace test_system_methods {
#define TIMEOUT 1 * 1000
#define NUMBER 12345

/***[ provider code ]**********************************************************/

class Methods :
    public datadriven::ProvidedObject,
    public MethodsInterface {
  public:
    std::shared_ptr<ReplyAsyncReply> replyAsync;
    uint32_t timeout;

    Methods(shared_ptr<datadriven::ObjectAdvertiser> advertiser) :
        datadriven::ProvidedObject(advertiser),
        MethodsInterface(this), replyAsync(nullptr)
    {
    }

  protected:
    void Sleep(uint32_t timeout, std::shared_ptr<SleepReply> _reply)
    {
        cout << "Provider sleeping" << endl;
        sleep(timeout / 1000);
        _reply->Send();
    }

    void ReplyViaSignal(int32_t i)
    {
        cout << "Provider sending signal" << endl;
        assert(ER_OK == SignalForReply(i));
    }

    void MethodWithCallback(std::shared_ptr<MethodWithCallbackReply> _reply)
    {
        cout << "Provider sending methodreply" << endl;
        _reply->Send();
    }

    // This method does not return a reply
    void ReplyAsync(uint32_t timeout, std::shared_ptr<ReplyAsyncReply> _reply)
    {
        cout << "Provider sending Reply asynchronously" << endl;
        this->timeout = timeout;
        replyAsync = _reply;
    }
};

static void be_provider(void)
{
    shared_ptr<datadriven::ObjectAdvertiser> advertiser = datadriven::ObjectAdvertiser::Create();
    assert(nullptr != advertiser);

    Methods m(advertiser);

    /* signal existence of object */
    cout << "Provider announcing object" << endl;
    assert(ER_OK == m.UpdateAll());
    assert(ER_OK == m.Methods::GetStatus());
    cout << "Provider sleeping" << endl;
    while (true) {
        sleep(1);
        if (m.replyAsync) {
            sleep(m.timeout / 1000);
            m.replyAsync->Send();
            m.replyAsync = nullptr;
            m.timeout = 0;
        }
    }
}

/***[ consumer code ]**********************************************************/

static datadriven::Semaphore _sync;

class MethodsListener :
    public datadriven::Observer<MethodsProxy>::Listener {
  public:
    void OnUpdate(const std::shared_ptr<MethodsProxy>& mp)
    {
        cout << "Consumer received object update for " << mp->GetObjectId() << endl;
        assert(ER_OK == _sync.Post());
    }
};

class MethodsSignalListener :
    public datadriven::SignalListener<MethodsProxy, MethodsProxy::SignalForReply>{
    void OnSignal(const MethodsProxy::SignalForReply& signal)
    {
        assert(NUMBER == signal.i);
        _sync.Post();
    }
};

/**
 * \test Method time out mechanism.
 *       -# call sleep method
 *       -# wait for reply
 *       -# reply status should be ER_TIMEOUT
 */
static void test_method_timeout(const MethodsProxy& mp)
{
    std::shared_ptr<datadriven::MethodInvocation<MethodsProxy::SleepReply> > inv = mp.Sleep(TIMEOUT * 2,
                                                                                            TIMEOUT);
    cout << "Consumer waiting for reply" << endl;
#ifndef NDEBUG
    const MethodsProxy::SleepReply& reply = inv->GetReply();
#endif
    cout << "Consumer reply ready" << endl;
    assert(inv->READY == inv->GetState());
    assert(ER_TIMEOUT == reply.GetStatus());
}

/**
 * \test Method without reply.
 *       -# call ReplyViaSignal method
 *       -# ensure method call was processed by waiting for the SignalForReply signal
 */
static void test_method_noreply(const MethodsProxy& mp)
{
    assert(ER_OK == mp.ReplyViaSignal(NUMBER));
    cout << "Consumer waiting for signal" << endl;
    _sync.Wait();
}

class MyMethodReplyListener :
    public datadriven::MethodReplyListener<MethodsProxy::MethodWithCallbackReply> {
    void OnReply(const MethodsProxy::MethodWithCallbackReply& reply)
    {
        cout << "Consumer method reply callback ready" << endl;
        assert(ER_OK == reply.GetStatus());
        _sync.Post();
    }
};

/**
 * \test Method reply Async callback mechanism
 *       -# call MethodwithCallback method
 *       -# Wait for reply in callback
 */
static void test_method_reply_callback(const MethodsProxy& mp)
{
    MyMethodReplyListener listener = MyMethodReplyListener();
    std::shared_ptr<datadriven::MethodInvocation<MethodsProxy::MethodWithCallbackReply> > inv = mp.MethodWithCallback(
        TIMEOUT);
    inv->SetListener(listener);

    cout << "Consumer waiting for method with callback" << endl;
    _sync.Wait();
}

class MyTimeoutMethodReplyListener :
    public datadriven::MethodReplyListener<MethodsProxy::SleepReply> {
    void OnReply(const MethodsProxy::SleepReply& reply)
    {
        cout << "Consumer method timeout reply callback ready" << endl;
        assert(ER_TIMEOUT == reply.GetStatus());
        _sync.Post();
    }
};

/**
 * \test Method reply Async timeout callback mechanism
 *       -# call Sleep method
 *       -# force that the callback is set only after the timeout has happened
 *       -# Wait for timeout reply in callback
 */
static void test_method_reply_callback_timeout(const MethodsProxy& mp)
{
    MyTimeoutMethodReplyListener listener = MyTimeoutMethodReplyListener();
    std::shared_ptr<datadriven::MethodInvocation<MethodsProxy::SleepReply> > inv =
        mp.Sleep(TIMEOUT * 2, 1);

    /* Wait with adding the listener till after the timeout has occurred */
    sleep(3);
    inv->SetListener(listener);

    cout << "Consumer waiting for method with callback timeout" << endl;
    _sync.Wait();
}

class MyCancelMethodReplyListener :
    public datadriven::MethodReplyListener<MethodsProxy::SleepReply> {
    void OnReply(const MethodsProxy::SleepReply& reply)
    {
        cout << "We must never get here!" << endl;
        /* Always fail */
        assert(0);
    }
};

/**
 * \test Method reply Async callback mechanism
 *       -# call Sleep method
 *       -# Add a method reply listener
 *       -# Cancel the method
 */
static void test_method_reply_callback_cancel(const MethodsProxy& mp)
{
    MyCancelMethodReplyListener listener = MyCancelMethodReplyListener();
    std::shared_ptr<datadriven::MethodInvocation<MethodsProxy::SleepReply> > inv =
        mp.Sleep(TIMEOUT, TIMEOUT);
    inv->SetListener(listener);
    inv->Cancel();

    cout << "Consumer test the cancel method on the MethodInvocation" << endl;

    sleep(TIMEOUT / 500);
}

class MyReplyAsyncMethodReplyListener :
    public datadriven::MethodReplyListener<MethodsProxy::ReplyAsyncReply> {
    void OnReply(const MethodsProxy::ReplyAsyncReply& reply)
    {
        cout << "Received async reply from provider" << endl;
        assert(ER_OK == reply.GetStatus());
        _sync.Post();
    }
};

/**
 * \test Method reply Async callback mechanism
 *       -# Call ReplyAsync method
 *       -# Add a method reply listener
 *       -# Wait for reply
 */
static void test_method_reply_async(const MethodsProxy& mp)
{
    MyReplyAsyncMethodReplyListener listener = MyReplyAsyncMethodReplyListener();
    std::shared_ptr<datadriven::MethodInvocation<MethodsProxy::ReplyAsyncReply> > inv =
        mp.ReplyAsync(TIMEOUT);
    inv->SetListener(listener);

    cout << "Consumer test the provider side" << endl;

    _sync.Wait();
}

static void be_consumer(void)
{
    MethodsListener ml = MethodsListener();
    MethodsSignalListener msl = MethodsSignalListener();

    std::shared_ptr<datadriven::Observer<MethodsProxy> > observer = datadriven::Observer<MethodsProxy>::Create(&ml);
    assert(ER_OK == observer->AddSignalListener(msl));
    // wait for object
    cout << "Consumer waiting for object" << endl;
    assert(ER_OK == _sync.Wait());
    // iterate objects
    cout << "Consumer iterating objects" << endl;
    for (datadriven::Observer<MethodsProxy>::iterator it = observer->begin();
         it != observer->end();
         ++it) {
        assert(ER_OK == it->GetStatus());
        cout << "Consumer in iterator for " << it->GetObjectId() << endl;
        test_method_timeout(**it);
        test_method_noreply(**it);
        test_method_reply_callback(**it);
        test_method_reply_callback_timeout(**it);
        test_method_reply_async(**it);
        test_method_reply_callback_cancel(**it);
    }
    cout << "Consumer done" << endl;
}
};

/***[ main code ]**************************************************************/

using namespace test_system_methods;

int main(int argc, char** argv)
{
    // only play provider if first command-line argument starts with a 'p'
    if (argc <= 1) {
        cout << "Usage: " << argv[0] << " <consumer|provider>" << endl;
        return 1;
    }
    if ('p' == *argv[1]) {
        be_provider();
    } else {
        be_consumer();
    }
    return 0;
}
