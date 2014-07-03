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

#include <map>
#include <memory>

#include <gtest/gtest.h>

#include <datadriven/Observer.h>
#include <datadriven/ObserverBase.h>
#include <datadriven/ProxyInterface.h>
#include <datadriven/TypeDescription.h>
#include <datadriven/Semaphore.h>
#include <datadriven/Marshal.h>

#include "BusConnectionImpl.h"
#include "ObserverManager.h"
#include "RegisteredTypeDescription.h"

//#include "SimpleTestObjectProxy.h"

using namespace std;
using namespace ajn;
using namespace datadriven;

#define IFACE_NAME "org.dummy"
#define PROP_EMIT "emit"
#define PROP_INVALIDATE "invalidate"

static Semaphore _sync;

/**
 * Tests involving properties.
 */
namespace test_unit_properties {
class MyTypeDescription :
    public TypeDescription {
  public:
    MyTypeDescription() :
        TypeDescription(IFACE_NAME)
    {
        AddProperty(PROP_EMIT, "i", ajn::PROP_ACCESS_READ, EmitChangesSignal::ALWAYS);
        AddProperty(PROP_INVALIDATE, "i", ajn::PROP_ACCESS_READ, EmitChangesSignal::INVALIDATES);
    }

    ~MyTypeDescription() { }
};

class MyProxyInterface :
    public ProxyInterface {
  public:
    MyProxyInterface(BusAttachment& bus,
                     RegisteredTypeDescription& desc) :
        ProxyInterface(desc, ObjectId(bus, "", "", 0))
    {
    }

    virtual ~MyProxyInterface() { }

    virtual QStatus UnmarshalProperty(const char* name, const ajn::MsgArg& value)
    {
        properties[name] = value;
        return ER_OK;
    }

    void validate(MsgArg& changed,
                  MsgArg& invalidated)
    {
        size_t num = changed.v_array.GetNumElements();
        ASSERT_EQ(properties.size(), num);
        const MsgArg* entries = changed.v_array.GetElements();
        for (size_t i = 0; i < num; i++) {
            const char* name = entries[i].v_dictEntry.key->v_string.str;
            ASSERT_TRUE(properties.find(name) != properties.end());
            ASSERT_TRUE(properties[name] == datadriven::MsgArgDereference(*entries[i].v_dictEntry.val));
        }
        // TODO invalidated stuff??
    }

  private:
    map<qcc::String, MsgArg> properties;
};

class MyObserver :
    public ObserverBase {
  public:
    MyProxyInterface* proxy;

    MyObserver(TypeDescription& type) :
        ObserverBase(type), proxy(nullptr), type(type)
    {
    }

    ~MyObserver()
    {
        observerMgr->UnregisterObserver(weak_this, type.GetName());
        // proxy will be deleted by cache
    }

    QStatus SetRefCountedPtr(weak_ptr<ObserverBase> observer)
    {
        weak_this = observer;
        observerMgr->RegisterObserver(weak_this, type.GetName());
        return ObserverBase::SetRefCountedPtr(observer);
    }

    virtual ProxyInterface* Alloc(const ObjectId& objId)
    {
        proxy = new MyProxyInterface(GetBusConnection()->GetBusAttachment(), *registeredTypeDesc);
        return proxy;
    }

    virtual void AddObject(const shared_ptr<ProxyInterface>& objProxy)
    {
        //cout << "Adding object" << endl;
        _sync.Post();
    }

    virtual void RemoveObject(const shared_ptr<ProxyInterface>& objProxy)
    {
        //cout << "Removing object" << endl;
        _sync.Post();
    }

    virtual void UpdateObject(const shared_ptr<ProxyInterface>& objProxy)
    {
        //cout << "Updating object" << endl;
        _sync.Post();
    }

    using ObserverBase::PropertiesChanged;

  private:
    weak_ptr<ObserverBase> weak_this;
    TypeDescription& type;
};

class PropertiesTests :
    public testing::Test {
  public:
    QStatus status;

#define BUS_NAME "properties_tests"
#define OBJECT_PATH "/my/object"
#define SESSION_ID 12345

    PropertiesTests() :
        status(ER_FAIL), type()
    {
        observer = shared_ptr<MyObserver>(new MyObserver(type));
        observer->SetRefCountedPtr(observer);
        id = new ObjectId(observer->GetBusConnection()->GetBusAttachment(), BUS_NAME, OBJECT_PATH, SESSION_ID);
        proxy = new ProxyBusObject(observer->GetBusConnection()->GetBusAttachment(), id->GetBusName().c_str(),
                                   id->GetBusObjectPath().c_str(), id->GetSessionId());
    }

    virtual ~PropertiesTests()
    {
        delete proxy;
        delete id;
        observer.reset();
    }

  protected:
    void AddObject()
    {
        ObserverManager::GetInstance(observer->GetBusConnection())->GetCache(IFACE_NAME)->AddObject(*id);
        _sync.Wait();
    }

    void RemoveObject()
    {
        ObserverManager::GetInstance(observer->GetBusConnection())->GetCache(IFACE_NAME)->RemoveObject(*id);
        _sync.Wait();
    }

    void PropertiesChanged(MsgArg& changed,
                           MsgArg& invalidated)
    {
        observer->PropertiesChanged(*proxy, IFACE_NAME, changed, invalidated, nullptr);
        _sync.Wait();
        observer->proxy->validate(changed, invalidated);
    }

    virtual void SetUp()
    {
        AddObject();
    }

    virtual void TearDown()
    {
        RemoveObject();
    }

  private:
    MyTypeDescription type;
    shared_ptr<MyObserver> observer;
    ObjectId* id;
    ProxyBusObject* proxy;
};

/**
 * \test Incoming signal without any changes.
 * */
TEST_F(PropertiesTests, NoChanges)
{
    MsgArg changed("a{sv}", 0, nullptr);

    MsgArg invalidated("as", 0, nullptr);

    PropertiesChanged(changed, invalidated);
}

/**
 * \test Incoming signal with only emitted changes.
 * */
TEST_F(PropertiesTests, OnlyEmitted)
{
    MsgArg emitted_value("i", 123);
    MsgArg emitted_entries[1];
    emitted_entries[0].Set("{sv}", PROP_EMIT, &emitted_value);
    MsgArg changed("a{sv}", 1, emitted_entries);

    MsgArg invalidated("as", 0, nullptr);

    PropertiesChanged(changed, invalidated);
}

/**
 * \test Incoming signal with only invalidated changes.
 * */
TEST_F(PropertiesTests, OnlyInvalidated)
{
    MsgArg changed("a{sv}", 0, nullptr);

    const char* invalidated_entries[] = { PROP_INVALIDATE };
    MsgArg invalidated("as", 1, invalidated_entries);

    PropertiesChanged(changed, invalidated);
}

/**
 * \test Incoming signal with both emitted and invalidated changes.
 * */
TEST_F(PropertiesTests, BothEmittedAndInvalidated)
{
    MsgArg emitted_value("i", 123);
    MsgArg emitted_entries[1];
    emitted_entries[0].Set("{sv}", PROP_EMIT, &emitted_value);
    MsgArg changed("a{sv}", 1, emitted_entries);

    const char* invalidated_entries[] = { PROP_INVALIDATE };
    MsgArg invalidated("as", 1, invalidated_entries);

    PropertiesChanged(changed, invalidated);
}
}
