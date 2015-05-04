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

#include <iostream>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/BusListener.h>
#include <alljoyn/BusObject.h>
#include <alljoyn/DBusStd.h>
#include <alljoyn/InterfaceDescription.h>
#include <alljoyn/Session.h>
#include <alljoyn/AboutObj.h>
#include <alljoyn/Init.h>
#include <qcc/Thread.h>

#include "data.h"

using namespace std;
using namespace ajn;

#define PATH "/AJNPropertiesProvider/1"
#define SESSION_PORT 12345

namespace test_system_properties {
class AJNPropertiesProvider :
    public SessionPortListener,
    public BusObject {
  public:
    AJNPropertiesProvider() :
        BusObject(PATH),
        bus("AJNPropertiesProvider", true),
        about(bus),
        iface(NULL),
        propReadOnly(INITIAL_RO),
        propReadWrite(INITIAL_RW),
        propEmitTrue(INITIAL_ET),
        propEmitInvalidates(INITIAL_EI),
        propEmitFalse(INITIAL_EF)
    {
        // bus set up
        assert(ER_OK == bus.Start());
        RegisterInterface();
        assert(ER_OK == bus.Connect());
        assert(ER_OK == bus.RequestName("test.system.properties.AJNPropertiesProvider",
                                        DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE));
        SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
        SessionPort port = SESSION_PORT;
        assert(ER_OK == bus.BindSessionPort(port, opts, *this));
        assert(ER_OK == bus.AdvertiseName(IFACE, TRANSPORT_ANY));
        // about set up
        AboutData aboutData("en");

        uint8_t appId[] = { 0x53, 0x61, 0x79, 0x20,
                            0x68, 0x65, 0x6c, 0x6c,
                            0x6f, 0x20, 0x74, 0x6f,
                            0x20, 0x51, 0x65, 0x6f };
        aboutData.SetAppId(appId, 16);
        aboutData.SetDeviceName("AJN Provider");
        //DeviceId is a string encoded 128bit UUID
        aboutData.SetDeviceId("2fd25710-ee7b-11e4-90ec-1681e6b88ec1");
        aboutData.SetAppName("AJN Provider");
        aboutData.SetManufacturer("QEO LLC");
        aboutData.SetModelNumber("181180");
        aboutData.SetDescription("AJN Provider");
        aboutData.SetDateOfManufacture("2014-03-24");
        aboutData.SetSoftwareVersion("1.0.0");
        aboutData.SetHardwareVersion("1.0.0");
        aboutData.SetSupportUrl("https://allseenalliance.org/developers/learn/ddapi");

        assert(aboutData.IsValid());
        // object set up
        assert(ER_OK == BusObject::AddInterface(*iface, ANNOUNCED));
        assert(ER_OK == bus.RegisterBusObject(*this));
        qcc::Sleep(200); //Give the consumer the chance to start up before sending the About signal.
        assert(ER_OK == about.Announce(SESSION_PORT, aboutData));
    }

    virtual bool AcceptSessionJoiner(SessionPort sessionPort, const char* joiner, const SessionOpts& opts)
    {
        cout << "AcceptSessionJoiner " << joiner << endl;
        return true;
    }

    // TODO need ASACORE_47 fix for signal testing
    // TODO send updated properties in signal
    // TODO send invalidated properties in signal

  protected:
    virtual QStatus Get(const char* ifcName, const char* propName, MsgArg& val)
    {
        QStatus status = ER_OK;

        cout << "Provider Get " << propName << endl;
        do {
            if (0 != strcmp(ifcName, IFACE)) {
                status = ER_BUS_OBJECT_NO_SUCH_INTERFACE;
                break;
            }
            if (0 == strcmp(propName, PROP_RO)) {
                val.Set("i", propReadOnly);
                break;
            }
            if (0 == strcmp(propName, PROP_WO)) {
                status = ER_FAIL;
                break;
            }
            if (0 == strcmp(propName, PROP_RW)) {
                val.Set("i", propReadWrite);
                break;
            }
            if (0 == strcmp(propName, PROP_ET)) {
                val.Set("i", propEmitTrue);
                break;
            }
            if (0 == strcmp(propName, PROP_EI)) {
                val.Set("i", propEmitInvalidates);
                break;
            }
            if (0 == strcmp(propName, PROP_EF)) {
                val.Set("i", propEmitFalse);
                break;
            }
            status = ER_BUS_NO_SUCH_PROPERTY;
        } while (0);
        return status;
    }

    virtual QStatus Set(const char* ifcName, const char* propName, MsgArg& val)
    {
        QStatus status = ER_OK;

        cout << "Provider Set " << propName << endl;
        do {
            if (0 != strcmp(ifcName, IFACE)) {
                status = ER_BUS_OBJECT_NO_SUCH_INTERFACE;
                break;
            }
            if (ALLJOYN_INT32 != val.typeId) {
                status = ER_FAIL;
                break;
            }
            if (0 == strcmp(propName, PROP_RO)) {
                status = ER_FAIL;
                break;
            }
            if (0 == strcmp(propName, PROP_WO)) {
                // to make this observable by a consumer we will update the
                // read-only property
                propReadOnly = val.v_int32;
                break;
            }
            if (0 == strcmp(propName, PROP_RW)) {
                propReadWrite = val.v_int32;
                break;
            }
            if (0 == strcmp(propName, PROP_ET)) {
                propEmitTrue = val.v_int32;
                EmitPropChanged(IFACE, propName, val, SESSION_ID_ALL_HOSTED, 0);
                break;
            }
            if (0 == strcmp(propName, PROP_EI)) {
                propEmitInvalidates = val.v_int32;
                EmitPropChanged(IFACE, propName, val, SESSION_ID_ALL_HOSTED, 0);
                break;
            }
            if (0 == strcmp(propName, PROP_EF)) {
                propEmitFalse = val.v_int32;
                break;
            }
            status = ER_BUS_NO_SUCH_PROPERTY;
        } while (0);
        return status;
    }

  private:
    BusAttachment bus;
    AboutObj about;
    const InterfaceDescription* iface;

    int32_t propReadOnly;
    int32_t propReadWrite;
    int32_t propEmitTrue;
    int32_t propEmitInvalidates;
    int32_t propEmitFalse;

    void RegisterInterface()
    {
        InterfaceDescription* tmp;

        assert(ER_OK == bus.CreateInterface(IFACE, tmp));
        assert(ER_OK == tmp->AddProperty(PROP_RO, "i", PROP_ACCESS_READ));
        assert(ER_OK == tmp->AddProperty(PROP_WO, "i", PROP_ACCESS_WRITE));
        assert(ER_OK == tmp->AddProperty(PROP_RW, "i", PROP_ACCESS_RW));
        assert(ER_OK == tmp->AddProperty(PROP_ET, "i", PROP_ACCESS_RW));
        assert(ER_OK == tmp->AddPropertyAnnotation(PROP_ET, org::freedesktop::DBus::AnnotateEmitsChanged, "true"));
        assert(ER_OK == tmp->AddProperty(PROP_EI, "i", PROP_ACCESS_RW));
        assert(ER_OK ==
               tmp->AddPropertyAnnotation(PROP_EI, org::freedesktop::DBus::AnnotateEmitsChanged, "invalidates"));
        assert(ER_OK == tmp->AddProperty(PROP_EF, "i", PROP_ACCESS_RW));
        assert(ER_OK == tmp->AddPropertyAnnotation(PROP_EF, org::freedesktop::DBus::AnnotateEmitsChanged, "false"));
        tmp->Activate();
        iface = tmp;
    }
};
} /* namespace test_system_properties */

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
        AJNPropertiesProvider prov;

        cout << "Provider sleeping" << endl;
        while (true) {
            sleep(60);
        }
    }
#ifdef ROUTER
    AllJoynRouterShutdown();
#endif
    AllJoynShutdown();

    return 0;
}
