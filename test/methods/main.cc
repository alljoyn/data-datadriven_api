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
    Methods(shared_ptr<datadriven::ObjectAdvertiser> advertiser) :
        datadriven::ProvidedObject(advertiser),
        MethodsInterface(this)
    {
    }

  protected:
    void Sleep(uint32_t timeout, SleepReply& _reply)
    {
        cout << "Provider sleeping" << endl;
        sleep(timeout / 1000);
        _reply.Send();
    }

    void ReplyViaSignal(int32_t i)
    {
        cout << "Provider sending signal" << endl;
        assert(ER_OK == SignalForReply(i));
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
        sleep(60);
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
    std::shared_ptr<datadriven::MethodInvocation<MethodsProxy::SleepReply> > inv = mp.Sleep(TIMEOUT * 2, TIMEOUT);
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
