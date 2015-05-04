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

#include <assert.h>

#include <datadriven/datadriven.h>
#include <datadriven/Semaphore.h>
#include <alljoyn/Init.h>

#include "PropertiesInterface.h"
#include "data.h"

using namespace std;
using namespace gen::org_allseenalliance_test;

/**
 * Properties tests.
 */
namespace test_system_properties {
class Properties :
    public datadriven::ProvidedObject,
    public PropertiesInterface {
  public:

    datadriven::Semaphore sync;

    Properties(shared_ptr<datadriven::ObjectAdvertiser> advertiser) :
        datadriven::ProvidedObject(advertiser),
        PropertiesInterface(this),
        PropReadOnly(INITIAL_RO),
        PropReadWrite(INITIAL_RW),
        PropEmitInvalidates(INITIAL_EI),
        PropEmitFalse(INITIAL_EF)
    {
        PropEmitTrue = INITIAL_ET;
    }

    ~Properties()
    {
    }

  protected:
    virtual QStatus GetPropReadOnly(int32_t& _PropReadOnly) const
    {
        cout << "Provider GetPropReadOnly()" << endl;
        _PropReadOnly = this->PropReadOnly;
        return ER_OK;
    }

    virtual QStatus SetPropWriteOnly(int32_t _PropWriteOnly)
    {
        cout << "Provider SetPropWriteOnly()" << endl;
        // to make this observable by a consumer we will update the
        // read-only property
        this->PropReadOnly = _PropWriteOnly;
        return ER_OK;
    }

    virtual QStatus SetPropReadWrite(int32_t _PropReadWrite)
    {
        cout << "Provider SetPropReadWrite()" << endl;
        this->PropReadWrite = _PropReadWrite;
        return ER_OK;
    }

    virtual QStatus GetPropReadWrite(int32_t& _PropReadWrite) const
    {
        cout << "Provider GetPropReadWrite()" << endl;
        _PropReadWrite = this->PropReadWrite;
        return ER_OK;
    }

    virtual QStatus SetPropEmitTrue(int32_t _PropEmitTrue)
    {
        cout << "Provider SetPropEmitTrue()" << endl;
        this->PropEmitTrue = _PropEmitTrue;
        Update();
        return ER_OK;
    }

    virtual QStatus SetPropEmitInvalidates(int32_t _PropEmitInvalidates)
    {
        cout << "Provider SetPropEmitInvalidates()" << endl;
        this->PropEmitInvalidates = _PropEmitInvalidates;
        QStatus status = InvalidatePropEmitInvalidates();
        assert(ER_OK == status);
        Update();
        return status;
    }

    virtual QStatus GetPropEmitInvalidates(int32_t& _PropEmitInvalidates) const
    {
        cout << "Provider GetPropEmitInvalidates()" << endl;
        _PropEmitInvalidates = this->PropEmitInvalidates;
        return ER_OK;
    }

    virtual QStatus SetPropEmitFalse(int32_t _PropEmitFalse)
    {
        cout << "Provider SetPropEmitFalse()" << endl;
        this->PropEmitFalse = _PropEmitFalse;
        return ER_OK;
    }

    virtual QStatus GetPropEmitFalse(int32_t& _PropEmitFalse) const
    {
        cout << "Provider GetPropEmitFalse()" << endl;
        _PropEmitFalse = this->PropEmitFalse;
        return ER_OK;
    }

  private:
    int32_t PropReadOnly;
    int32_t PropReadWrite;
    int32_t PropEmitInvalidates;
    int32_t PropEmitFalse;
};
};

using namespace test_system_properties;

int main(int argc, char** argv)
{
    if (AllJoynInit() != ER_OK) {
        return EXIT_FAILURE;
    }
#ifdef ROUTER
    if (AllJoynRouterInit() != ER_OK) {
        AllJoynShutdown();
        return EXIT_FAILURE;
    }
#endif
    {
        shared_ptr<datadriven::ObjectAdvertiser> advertiser = datadriven::ObjectAdvertiser::Create();
        assert(nullptr != advertiser);

        Properties p(advertiser);

        /* signal existence of object */
        cout << "Provider announcing object" << endl;
        assert(ER_OK == p.UpdateAll());
        assert(ER_OK == p.Properties::GetStatus());

        cout << "Provider sleeping" << endl;
        while (true) {
            sleep(60);
        }

        advertiser.reset();
        advertiser = nullptr;
    }
#ifdef ROUTER
    AllJoynRouterShutdown();
#endif
    AllJoynShutdown();
    return 0;
}
