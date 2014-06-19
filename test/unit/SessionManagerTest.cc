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
#include "ConsumerSessionManager.h"
#include "ProviderSessionManager.h"
#include "gtest/gtest.h"
#include "BusConnectionImpl.h"

/**
 * Session manager tests.
 */
namespace test_unit_sessionmanager {
using namespace ajn;
using namespace datadriven;

class SessionManagerTest :
    public::testing::Test {
  protected:

    virtual void SetUp()
    {
        //Per Test if needed
    }

    virtual void TearDown()
    {
        //Per Test if needed
    }
};

class TestInterfaceListener :
    public ConsumerInterfaceListener {
    std::vector<qcc::String> objects;

    virtual void OnNew(ConsumerSessionManager::ObserverRef rr, const ObjectId& objId)
    {
        std::cout << "New Object : " << objId.GetBusObjectPath().c_str() << "\n";
        objects.push_back(objId.GetBusObjectPath());
    }

    virtual void OnRemove(ConsumerSessionManager::ObserverRef rr, const ObjectId& objId)
    {
        std::cout << "Removed Object : " << objId.GetBusObjectPath().c_str() << "\n";

        std::vector<qcc::String>::iterator itr = std::find(objects.begin(), objects.end(), (objId.GetBusObjectPath()));

        ASSERT_TRUE(itr != objects.end())
        << "Trying to remove an object that was not tracked from before !! Obj Path : "
        << objId.GetBusObjectPath().c_str();

        objects.erase(itr);
    }

  public:

    bool Exists(const qcc::String& objBusName)
    {
        std::vector<qcc::String>::iterator found = objects.end();
        found = std::find(objects.begin(), objects.end(), objBusName);
        return (found != objects.end());
    }
};
class BasicIntfDesc :
    public TypeDescription {
  public:
    BasicIntfDesc(qcc::String name) :
        name(name)
    {
    }

    virtual const qcc::String GetName() const
    {
        return name;
    }

    virtual QStatus BuildInterface(ajn::InterfaceDescription& iface) const
    {
        return ER_OK;
    }

    virtual QStatus ResolveMembers(const ajn::InterfaceDescription& iface,
                                   const ajn::InterfaceDescription::Member*** member) const
    {
        return ER_OK;
    }

  private:
    qcc::String name;
};

class BasicTestObject :
    public ProvidedObject {
  public:
    BasicTestObject(BusConnection& fact,
                    qcc::String& path) :
        ProvidedObject(fact, path), busConnectionImpl(NULL)
    {
        busConnectionImpl = fact.busConnectionImpl.get();
    }

    BusConnectionImpl* busConnectionImpl;
};

class BasicIntf :
    public ProvidedInterface {
  public:
    BasicIntf(ajn::BusAttachment& ba,
              BasicTestObject& bo,
              BasicIntfDesc& intfDesc) :
        ProvidedInterface(intfDesc, bo)
    {
        QStatus status = this->RegisterInterface(ba);
        if (status != ER_OK) {
            std::cerr << "Could not create interface " << std::endl;
            abort();
        }
    }

  protected:
    std::vector<ajn::MsgArg> MarshalProperties()
    {
        std::vector<ajn::MsgArg> args;
        return args;
    }
};

/**
 * \test Test the basic publishing functionality and notification using
 *       a registered listener:
 *       -# Publish an object on a given interface
 *       -# Verify successful publishing through a registered listener
 *       -# Remove the object from that interface
 *       -# Verify successful removal through a registered listener
 * */
TEST_F(SessionManagerTest, BasicPublishRemoveObjectWithListener)
{
    qcc::String testBusPath = "/ThisIsaValidBusPath";
    qcc::String intfName = "org.sessionmanager.Test";

    BusConnection fact;
    TestInterfaceListener mTestInterfaceListener;
    ConsumerSessionManager::ObserverRef mObserver = NULL;

    std::unique_ptr<BasicTestObject> mBasicTestObject =
        std::unique_ptr<BasicTestObject>(new BasicTestObject(fact, testBusPath));
    ProviderSessionManager& providerSessionManager = mBasicTestObject->busConnectionImpl->GetProviderSessionManager();
    ConsumerSessionManager& consumerSessionManager =
        mBasicTestObject->busConnectionImpl->GetConsumerSessionManager();

    BasicIntfDesc basicIntfDesc(intfName);
    std::unique_ptr<BasicIntf> intf =
        std::unique_ptr<BasicIntf>(new BasicIntf(providerSessionManager.GetBusAttachment(), *mBasicTestObject,
                                                 basicIntfDesc));

    consumerSessionManager.RegisterInterfaceListener(&mTestInterfaceListener);

    EXPECT_TRUE(ER_OK == (consumerSessionManager.RegisterObserver(mObserver, intfName)));
    EXPECT_EQ(ER_OK, mBasicTestObject->AddProvidedInterface(intf.get(), NULL, 0));
    EXPECT_EQ(ER_OK, providerSessionManager.AdvertiseBusObject(*mBasicTestObject));

    usleep(30000);

    //****************************************************************************************//

    EXPECT_TRUE(mTestInterfaceListener.Exists(testBusPath)) << "Published object path does not exist ! : "
                                                            << testBusPath.c_str();
    EXPECT_EQ(ER_OK, providerSessionManager.RemoveBusObject(*mBasicTestObject));

    usleep(30000);

    EXPECT_FALSE(mTestInterfaceListener.Exists(testBusPath)) << "Published object still exists ! : "
                                                             << testBusPath.c_str();
    EXPECT_EQ(ER_OK, consumerSessionManager.UnregisterObserver(mObserver));

    //****************************************************************************************//

    mBasicTestObject.reset();
    intf.reset();

    usleep(30000);
}

static void MatchIterateObjectId(ConsumerSessionManager::ObserverRef observer,
                                 const ObjectId& objId,
                                 void* ctx)
{
    EXPECT_TRUE(objId.GetBusObjectPath().compare(((BasicTestObject*)ctx)->GetPath()) == 0);
}

static void UnMatchIterateObjectId(ConsumerSessionManager::ObserverRef observer,
                                   const ObjectId& objId,
                                   void* ctx)
{
    EXPECT_TRUE(objId.GetBusObjectPath().compare(((BasicTestObject*)ctx)->GetPath()) != 0);
}

/**
 * \test Test the basic Publishing functionality and object (in)existence using
 *       GetObjectIdsForObserver:
 *       -# Publish an object on a given interface
 *       -# Verify successful publishing using GetObjectIdsForObserver
 *       -# Remove the object from that interface
 *       -# Verify successful removal using GetObjectIdsForObserver
 * */
TEST_F(SessionManagerTest, BasicPublishRemoveObjectWithGetObjectIdsForObserver) {
    qcc::String testBusPath = "/ThisIsaValidBusPath";
    qcc::String intfName = "org.sessionmanager.Test";

    BusConnection fact;
    ConsumerSessionManager::ObserverRef mObserver = NULL;

    std::unique_ptr<BasicTestObject> mBasicOTestbject =
        std::unique_ptr<BasicTestObject>(new BasicTestObject(fact, testBusPath));
    ProviderSessionManager& providerSessionManager = mBasicOTestbject->busConnectionImpl->GetProviderSessionManager();
    ConsumerSessionManager& consumerSessionManager =
        mBasicOTestbject->busConnectionImpl->GetConsumerSessionManager();

    BasicIntfDesc basicIntfDesc(intfName);
    std::unique_ptr<BasicIntf> intf =
        std::unique_ptr<BasicIntf>(new BasicIntf(providerSessionManager.GetBusAttachment(), *mBasicOTestbject,
                                                 basicIntfDesc));

    EXPECT_TRUE(ER_OK == (consumerSessionManager.RegisterObserver(mObserver, intfName)));
    EXPECT_EQ(ER_OK, mBasicOTestbject->AddProvidedInterface(intf.get(), NULL, 0));
    EXPECT_EQ(ER_OK, providerSessionManager.AdvertiseBusObject(*mBasicOTestbject));

    usleep(30000);

    //****************************************************************************************//

    EXPECT_EQ(ER_OK,
              consumerSessionManager.GetObjectIdsForObserver(mObserver, MatchIterateObjectId, mBasicOTestbject.get()));
    EXPECT_EQ(ER_OK, providerSessionManager.RemoveBusObject(*mBasicOTestbject));

    usleep(30000);

    EXPECT_EQ(ER_OK,
              consumerSessionManager.GetObjectIdsForObserver(mObserver, UnMatchIterateObjectId, mBasicOTestbject.get()));
    EXPECT_EQ(ER_OK, consumerSessionManager.UnregisterObserver(mObserver));

    //****************************************************************************************//

    mBasicOTestbject.reset();
    intf.reset();

    usleep(30000);
}

/**
 * \test Test the Publishing functionality of many objects and their removal
 *       -# Publish a number of objects on a given interface
 *       -# Verify successful publishing
 *       -# Remove the objects from that interface
 *       -# Verify successful removal
 * */
TEST_F(SessionManagerTest, PublishRemoveManyObjects)
{
    qcc::String testBusPath = "/ThisIsaValidBusPath";
    qcc::String intfName = "org.sessionmanager.Test";

    int numOfBasicObjects = 100;

    BusConnection fact;

    ProviderSessionManager* providerSessionManager = NULL;
    ConsumerSessionManager* consumerSessionManager = NULL;
    BasicIntfDesc basicIntfDesc(intfName);
    TestInterfaceListener mTestInterfaceListener;
    ConsumerSessionManager::ObserverRef mObserver = NULL;

    std::vector<std::unique_ptr<BasicTestObject> > manyBasicObjects;
    std::vector<std::unique_ptr<BasicIntf> > manyBasicIntfs;

    for (int i = 0; i < numOfBasicObjects; i++) {
        testBusPath = "/ThisIsaValidBusPath";
        testBusPath += static_cast<std::ostringstream*>(&(std::ostringstream() << i))->str().c_str();

        std::unique_ptr<BasicTestObject> mBasicTESTObject =
            std::unique_ptr<BasicTestObject>(new BasicTestObject(fact, testBusPath));
        if ((NULL == providerSessionManager) && (NULL == consumerSessionManager)) {
            providerSessionManager = &mBasicTESTObject->busConnectionImpl->GetProviderSessionManager();
            consumerSessionManager = &mBasicTESTObject->busConnectionImpl->GetConsumerSessionManager();
        }
        manyBasicObjects.push_back(std::move(mBasicTESTObject));
        manyBasicIntfs.push_back(std::unique_ptr<BasicIntf>(new BasicIntf(providerSessionManager->GetBusAttachment(),
                                                                          *(manyBasicObjects.back()), basicIntfDesc)));
        testBusPath.clear();
    }

    consumerSessionManager->RegisterInterfaceListener(&mTestInterfaceListener);

    EXPECT_TRUE(ER_OK == (consumerSessionManager->RegisterObserver(mObserver, intfName)));

    for (int i = 0; i < numOfBasicObjects; i++) {
        EXPECT_EQ(ER_OK, manyBasicObjects[i]->AddProvidedInterface(manyBasicIntfs[i].get(), NULL, 0));
        EXPECT_EQ(ER_OK, providerSessionManager->AdvertiseBusObject(*(manyBasicObjects[i])));
    }

    usleep(numOfBasicObjects * 300);

    //****************************************************************************************//

    for (int i = 0; i < numOfBasicObjects; i++) {
        testBusPath = "/ThisIsaValidBusPath";
        testBusPath += static_cast<std::ostringstream*>(&(std::ostringstream() << i))->str().c_str();

        EXPECT_TRUE(mTestInterfaceListener.Exists(testBusPath)) << "Published object path does not exist ! : "
                                                                << testBusPath.c_str();
        EXPECT_EQ(ER_OK, providerSessionManager->RemoveBusObject(*(manyBasicObjects[i])));
    }
    usleep(numOfBasicObjects * 2000);

    for (int i = 0; i < numOfBasicObjects; i++) {
        testBusPath = "/ThisIsaValidBusPath";
        testBusPath += static_cast<std::ostringstream*>(&(std::ostringstream() << i))->str().c_str();
        EXPECT_FALSE(mTestInterfaceListener.Exists(testBusPath)) << "Published object still exists ! : "
                                                                 << testBusPath.c_str();
    }

    EXPECT_EQ(ER_OK, consumerSessionManager->UnregisterObserver(mObserver));

    //****************************************************************************************//

    manyBasicObjects.clear();
    manyBasicIntfs.clear();

    usleep(30000);
}

/**
 * \test Test trying to remove an inexistent object
 *       -# Remove an object that was not previously published
 *       -# Verify proper situation handling
 * */
TEST_F(SessionManagerTest, RemoveInexistentObject) {
    BusConnection fact;
    qcc::String testBusPath = "/UnPublishedBasicObjectPath";

    BasicTestObject* mBasicTestObject = new BasicTestObject(fact, testBusPath);
    ProviderSessionManager& providerSessionManager = mBasicTestObject->busConnectionImpl->GetProviderSessionManager();

    /*As odd as it may seem but the provider does not get a non ER_OK if it
     * removes an object that was not published beforehand hence the expected ER_OK
     * */
    EXPECT_EQ(ER_OK, providerSessionManager.RemoveBusObject(*mBasicTestObject));
}

//More tests to come concerning many observers and stress testing in general
}
//namespace
