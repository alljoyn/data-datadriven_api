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

#include <assert.h>
#include <map>

#include <datadriven/datadriven.h>
#include <datadriven/Semaphore.h>

#include "PropertiesProxy.h"
#include "ConsumerBase.h"
#include "data.h"

using namespace std;
using namespace datadriven;
using namespace gen::org_allseenalliance_test;

// C++ FAQ [33.6]
#define CALL_MEMBER_FN(object, ptrToMember)  ((object).*(ptrToMember))

namespace test_system_properties {
class DDAPIPropertiesConsumer :
    public ConsumerBase,
    private Observer<PropertiesProxy>::Listener {
  public:
    DDAPIPropertiesConsumer() :
        proxy(), observer(Observer<PropertiesProxy>::Create(this))
    {
    }

    ~DDAPIPropertiesConsumer()
    {
        proxy.reset();
    }

    virtual void WaitForPeer()
    {
        assert(ER_OK == sync.TimedWait(30 * 1000));
    }

    virtual int32_t GetProperty(const char* name)
    {
        int32_t val;

        if (0 == strcmp(PROP_RO, name)) {
            shared_ptr<datadriven::MethodInvocation<PropertiesProxy::GetPropReadOnlyReply> > inv =
                proxy->GetPropReadOnly();
            const PropertiesProxy::GetPropReadOnlyReply& reply = inv->GetReply();
            assert(ER_OK == reply.GetStatus());
            val = reply.PropReadOnly;
        } else if (0 == strcmp(PROP_RW, name)) {
            shared_ptr<datadriven::MethodInvocation<PropertiesProxy::GetPropReadWriteReply> > inv =
                proxy->GetPropReadWrite();
            const PropertiesProxy::GetPropReadWriteReply& reply = inv->GetReply();
            assert(ER_OK == reply.GetStatus());
            val = reply.PropReadWrite;
        } else if (0 == strcmp(PROP_ET, name)) {
            val = proxy->GetProperties().PropEmitTrue;
        } else if (0 == strcmp(PROP_EI, name)) {
            shared_ptr<datadriven::MethodInvocation<PropertiesProxy::GetPropEmitInvalidatesReply> > inv =
                proxy->GetPropEmitInvalidates();
            const PropertiesProxy::GetPropEmitInvalidatesReply& reply = inv->GetReply();
            assert(ER_OK == reply.GetStatus());
            val = reply.PropEmitInvalidates;
        } else if (0 == strcmp(PROP_EF, name)) {
            shared_ptr<datadriven::MethodInvocation<PropertiesProxy::GetPropEmitFalseReply> > inv =
                proxy->GetPropEmitFalse();
            const PropertiesProxy::GetPropEmitFalseReply& reply = inv->GetReply();
            assert(ER_OK == reply.GetStatus());
            val = reply.PropEmitFalse;
        } else {
            assert(false); // should never happen
        }
        return val;
    }

    virtual void SetProperty(const char* name, int32_t val)
    {
        if (0 == strcmp(PROP_WO, name)) {
            shared_ptr<datadriven::MethodInvocation<PropertiesProxy::SetPropWriteOnlyReply> > inv =
                proxy->SetPropWriteOnly(val);
            const PropertiesProxy::SetPropWriteOnlyReply& reply = inv->GetReply();
            assert(ER_OK == reply.GetStatus());
        } else if (0 == strcmp(PROP_RW, name)) {
            shared_ptr<datadriven::MethodInvocation<PropertiesProxy::SetPropReadWriteReply> > inv =
                proxy->SetPropReadWrite(val);
            const PropertiesProxy::SetPropReadWriteReply& reply = inv->GetReply();
            assert(ER_OK == reply.GetStatus());
        } else if (0 == strcmp(PROP_ET, name)) {
            shared_ptr<datadriven::MethodInvocation<PropertiesProxy::SetPropEmitTrueReply> > inv =
                proxy->SetPropEmitTrue(val);
            const PropertiesProxy::SetPropEmitTrueReply& reply = inv->GetReply();
            assert(ER_OK == reply.GetStatus());
        } else if (0 == strcmp(PROP_EI, name)) {
            shared_ptr<datadriven::MethodInvocation<PropertiesProxy::SetPropEmitInvalidatesReply> > inv =
                proxy->SetPropEmitInvalidates(val);
            const PropertiesProxy::SetPropEmitInvalidatesReply& reply = inv->GetReply();
            assert(ER_OK == reply.GetStatus());
        } else if (0 == strcmp(PROP_EF, name)) {
            shared_ptr<datadriven::MethodInvocation<PropertiesProxy::SetPropEmitFalseReply> > inv =
                proxy->SetPropEmitFalse(val);
            const PropertiesProxy::SetPropEmitFalseReply& reply = inv->GetReply();
            assert(ER_OK == reply.GetStatus());
        } else {
            assert(false); // should never happen
        }
    }

  private:
    shared_ptr<PropertiesProxy> proxy;
    shared_ptr<Observer<PropertiesProxy> > observer;

    struct cmp_str {
        bool operator()(char const* a, char const* b)
        {
            return strcmp(a, b) < 0;
        }
    };

    void OnUpdate(const std::shared_ptr<PropertiesProxy>& pp)
    {
        cout << "Consumer received object update for " << pp->GetObjectId() << endl;
        // verify state
        PropertiesProxy::Properties props = pp->GetProperties();
        assert(signal_offset + INITIAL_ET == props.PropEmitTrue);
        // continue test
        proxy = pp;
        assert(ER_OK == sync.Post());
        // TODO check invalidated properties
        updated++;
    }
};
} /* namespace test_system_properties */

using namespace test_system_properties;

int main(int argc, char** argv)
{
    DDAPIPropertiesConsumer cons;

    cons.Test();
    return 0;
}
