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

#include "MethodReplyErrorInterface.h"
#include "MethodReplyErrorProxy.h"

using namespace std;
using namespace gen::org_allseenalliance_test;

/**
 * Methods tests.
 */
namespace test_system_methods {
/***[ provider code ]**********************************************************/

class Methods :
    public datadriven::ProvidedObject,
    public MethodReplyErrorInterface {
  public:
    Methods(shared_ptr<datadriven::ObjectAdvertiser> advertiser) :
        datadriven::ProvidedObject(advertiser),
        MethodReplyErrorInterface(this)
    {
    }

  protected:
    void Normal(NormalReply& _reply)
    {
        cout << "Provider sending Normal reply" << endl;
        _reply.Send();
    }

    void Error(ErrorReply& _reply)
    {
        cout << "Provider sending Error reply" << endl;
        _reply.SendError("org.allseenalliance.test.MethodReplyError.Error",
                         "This is an error string with description");
    }

    void ErrorCode(ErrorCodeReply& _reply)
    {
        cout << "Provider sending ErrorCode reply" << endl;
        _reply.SendErrorCode(ER_WARNING);
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

class MethodReplyErrorListener :
    public datadriven::Observer<MethodReplyErrorProxy>::Listener {
  public:
    void OnUpdate(const std::shared_ptr<MethodReplyErrorProxy>& mp)
    {
        cout << "Consumer received object update for " << mp->GetObjectId() << endl;
        assert(ER_OK == _sync.Post());
    }
};

/**
 * \test Method time out mechanism.
 *       -# Call a method with a normal reply
 *       -# Call a method with an error string reply
 *       -# Call a method with an error Code reply
 */
static void test_method_reply_error(const MethodReplyErrorProxy& mp)
{
    /* Test a normal reply */
    std::shared_ptr<datadriven::MethodInvocation<MethodReplyErrorProxy::NormalReply> > invNormal = mp.Normal();
    cout << "Consumer waiting for a Normal reply" << endl;
    MethodReplyErrorProxy::NormalReply replyNormal = invNormal->GetReply();
    cout << "Consumer Normal reply ready" << endl;
    assert(invNormal->READY == invNormal->GetState());
    assert(ER_OK == replyNormal.GetStatus());

    /* Test if error string on MethodReplyError comes through */
    std::shared_ptr<datadriven::MethodInvocation<MethodReplyErrorProxy::ErrorReply> > invError = mp.Error();
    cout << "Consumer waiting for Error reply" << endl;
    MethodReplyErrorProxy::ErrorReply replyError = invError->GetReply();
    qcc::String error = replyError.GetErrorName();
    qcc::String errorMessage = replyError.GetErrorDescription();
    cout << "Consumer Error reply ready" << endl;
    assert(ER_BUS_REPLY_IS_ERROR_MESSAGE == replyError.GetStatus());
    assert(error.size() && errorMessage.size());
    cout << "Error method call returned and error \"" << error.c_str()  << "\" with error message \"" <<
    errorMessage.c_str() <<
    "\"" << endl;

    /* Test if error code on MethodReplyErrorCode comes through */
    std::shared_ptr<datadriven::MethodInvocation<MethodReplyErrorProxy::ErrorCodeReply> > invErrorCode = mp.ErrorCode();
    cout << "Consumer waiting for ErrorCode reply" << endl;
    MethodReplyErrorProxy::ErrorCodeReply replyErrorCode = invErrorCode->GetReply();
    error = replyErrorCode.GetErrorName();
    errorMessage = replyErrorCode.GetErrorDescription();
    cout << "Consumer ErrorCode reply ready" << endl;
    assert(ER_WARNING == replyErrorCode.GetStatus());
    cout << "ErrorCode method call returned an error \"" << QCC_StatusText(replyErrorCode.GetStatus()) << "\"" << endl;
}

static void be_consumer(void)
{
    MethodReplyErrorListener ml = MethodReplyErrorListener();

    std::shared_ptr<datadriven::Observer<MethodReplyErrorProxy> > observer =
        datadriven::Observer<MethodReplyErrorProxy>::Create(&ml);
    // wait for object
    cout << "Consumer waiting for object" << endl;
    assert(ER_OK == _sync.Wait());
    // iterate objects
    cout << "Consumer iterating objects" << endl;
    for (datadriven::Observer<MethodReplyErrorProxy>::iterator it = observer->begin();
         it != observer->end();
         ++it) {
        assert(ER_OK == it->GetStatus());
        cout << "Consumer in iterator for " << it->GetObjectId() << endl;
        test_method_reply_error(**it);
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
