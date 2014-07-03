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

#include <vector>

#include <qcc/String.h>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/ProxyBusObject.h>

#include <alljoyn/about/AnnounceHandler.h>
#include <alljoyn/about/AnnouncementRegistrar.h>

#include "ConsumerBase.h"
#include "data.h"

using namespace std;
using namespace ajn;
using namespace services;

namespace test_system_properties {
class AJNPropertiesConsumer :
    public ConsumerBase,
    private AnnounceHandler,
    private BusAttachment::JoinSessionAsyncCB,
    private ProxyBusObject::PropertiesChangedListener {
  public:
    AJNPropertiesConsumer() :
        bus("AJNPropertiesConsumer", true), pbo(NULL)
    {
        assert(ER_OK == bus.Start());
        assert(ER_OK == bus.Connect());

        const char* interfaces[] = { IFACE };
        assert(ER_OK == AnnouncementRegistrar::RegisterAnnounceHandler(bus, *this, interfaces,
                                                                       sizeof(interfaces) / sizeof(interfaces[0])));
        RegisterInterface();
    }

    ~AJNPropertiesConsumer()
    {
        delete pbo;
    }

    // TODO need ASACORE_47 fix for signal testing
    // TODO check updated properties in signal
    // TODO check invalidated properties in signal

    virtual void WaitForPeer()
    {
        // TODO wait for properties signal
        assert(ER_OK == sync.TimedWait(30 * 1000));
    }

    virtual int32_t GetProperty(const char* name)
    {
        MsgArg val;

        cout << "Consumer get property " << name << endl;
        assert(ER_OK == pbo->GetProperty(IFACE, name, val));

        return MsgArgDereference(val);
    }

    virtual void SetProperty(const char* name, int32_t val)
    {
        MsgArg msgarg_val("i", val);

        cout << "Consumer set property " << name << endl;

        assert(ER_OK == pbo->SetProperty(IFACE, name, msgarg_val));
    }

    virtual void PropertiesChanged(ProxyBusObject& obj,
                                   const char* ifaceName,
                                   const MsgArg& changed,
                                   const MsgArg& invalidated,
                                   void* context)
    {
        updated++;
        cout << "AJN consumer got changed property " << endl;
        assert(ER_OK == sync.Post());
    }

  private:
    BusAttachment bus;
    ProxyBusObject* pbo;
    qcc::String remoteBusName;
    qcc::String objectPath;
    const InterfaceDescription* iface;

    virtual void Announce(unsigned short version,
                          unsigned short port,
                          const char* busName,
                          const ObjectDescriptions& objectDescs,
                          const AboutData& aboutData)
    {
        // ensure at least one object
        assert(objectDescs.size() > 0);
        remoteBusName = busName;
        // get path of object matching our interface
        for (AboutClient::ObjectDescriptions::const_iterator object = objectDescs.begin();
             (0 == objectPath.length()) && (object != objectDescs.end());
             ++object) {
            vector<qcc::String> interfaces = object->second;
            for (vector<qcc::String>::const_iterator iface = interfaces.begin(); iface != interfaces.end(); ++iface) {
                if (*iface == IFACE) {
                    objectPath = object->first;
                    break;
                }
            }
        }
        // join session with peer
        SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_PHYSICAL, TRANSPORT_ANY);
        assert(ER_OK == bus.JoinSessionAsync(busName, port, NULL, opts, this, NULL));
    }

    void JoinSessionCB(QStatus status,
                       ajn::SessionId id,
                       const ajn::SessionOpts& opts,
                       void* context)
    {
        assert(ER_OK == status);
        pbo = new ProxyBusObject(bus, remoteBusName.c_str(), objectPath.c_str(), id);
        assert(NULL != pbo);

        cout << "Consumer add interface" << endl;

        /* Add interface and register the properties changed handler */
        assert(ER_OK == pbo->AddInterface(*iface));
        assert(ER_OK == pbo->RegisterPropertiesChangedListener(IFACE, NULL, 0, *this, NULL));

        updated++;
        assert(ER_OK == sync.Post());
    };

    int32_t MsgArgDereference(const MsgArg& value)
    {
        assert(ALLJOYN_VARIANT == value.typeId);
        assert(ALLJOYN_INT32 == value.v_variant.val->typeId);
        return value.v_variant.val->v_int32;
    }

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
        assert(ER_OK == tmp->AddPropertyAnnotation(PROP_EI, org::freedesktop::DBus::AnnotateEmitsChanged, "invalidates"));
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
    AJNPropertiesConsumer cons;

    cons.Test();
    return 0;
}
