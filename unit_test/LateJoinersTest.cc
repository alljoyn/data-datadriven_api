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
#include <memory>

#include <gtest/gtest.h>

#include "Common.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_TEST"

/**
 * Testing late joiners.
 */
namespace test_unit_latejoiners {
using namespace test_unit_common;

static void Publish(shared_ptr<datadriven::ObjectAdvertiser> advertiser,
                    std::vector<unique_ptr<TestObject> >& testObjects,
                    int numPubObjs)
{
    qcc::String testObjName;
    for (int i = 0; i < numPubObjs; i++) {
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "TestObjectLateJoiner%d", i);
        testObjName = qcc::String(buffer);
        testObjects.push_back(unique_ptr<TestObject>(new TestObject(advertiser, testObjName)));
        ASSERT_TRUE(testObjects.back()->UpdateAll() == ER_OK);
    }
}

static void Remove(std::vector<unique_ptr<TestObject> >& testObjects)
{
    for (auto& object : testObjects) {
        object->RemoveFromBus();
    }
}

static void WaitOnUpdates(TestObjectListener& testObjectListener,
                          int numPubObjs)
{
    qcc::String testObjName;
    testObjectListener.WaitOnAllUpdates(numPubObjs, 5);
    EXPECT_TRUE((int)testObjectListener.nameToObjects.size() == numPubObjs);
    for (int i = 0; i < numPubObjs; i++) {
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "TestObjectLateJoiner%d", i);
        testObjName = qcc::String(buffer);
        std::map<qcc::String, std::shared_ptr<SimpleTestObjectProxy> >::iterator itr =
            testObjectListener.nameToObjects.find(testObjName);
        EXPECT_TRUE(itr != testObjectListener.nameToObjects.end());
        EXPECT_TRUE(testObjName.compare(itr->first) == 0);
    }
}

/* *
 * \test This test aims at verifying the late joiner functionality, i.e., already published
 *       objects should be able to be seen by late observers.
 *       If objects have been removed by the provider then a late joiner should not see those.
 *       Steps:
 *       -# Publish many test objects after a valid bus connection is set-up
 *       -# Create an observer and add to it a listener and check that this was successful
 *       -# Verify that all published objects can be seen by this observer
 *       -# Create another observer and add to it a new listener and check that this was successful
 *       -# Verify that all published objects can be seen by this new observer
 *       -# Remove all published objects
 *       -# Create a new observer and add to it a new listener and check that this was successful
 *       -# Verify that the last observer cannot see any objects
 * */
TEST(LateJoiners, PublishJoinRemoveJoin) {
    int numPubObjs = 10;
    shared_ptr<datadriven::ObjectAdvertiser> advertiser = ObjectAdvertiser::Create();
    ASSERT_TRUE(advertiser != nullptr);
    std::vector<unique_ptr<TestObject> > testObjects;

    //Publish some test objects knowing that there are no observers yet.
    Publish(advertiser, testObjects, numPubObjs);

    //Check that all published test objects can be seen by a late joiner observer.
    TestObjectListener testObjectListener;
    std::shared_ptr<Observer<SimpleTestObjectProxy> > obs =
        Observer<SimpleTestObjectProxy>::Create(&testObjectListener);

    ASSERT_TRUE(obs->GetStatus() == ER_OK);
    WaitOnUpdates(testObjectListener, numPubObjs);

    //A new late joiner should receive all objects.
    TestObjectListener testObjectListener2;
    std::shared_ptr<Observer<SimpleTestObjectProxy> > obs2 =
        Observer<SimpleTestObjectProxy>::Create(&testObjectListener2);

    ASSERT_TRUE(obs2->GetStatus() == ER_OK);
    WaitOnUpdates(testObjectListener2, numPubObjs);

    //**********************************************************//

    //Remove all test objects
    Remove(testObjects);

    //Check that both observers have seen the objects being removed
    testObjectListener.WaitOnRemove(numPubObjs, 5);
    EXPECT_TRUE((int)testObjectListener.removedObjectNames.size() == numPubObjs);
    testObjectListener2.WaitOnRemove(numPubObjs, 5);
    EXPECT_TRUE((int)testObjectListener2.removedObjectNames.size() == numPubObjs);

    //A new late joiner should NOT receive any objects.
    TestObjectListener testObjectListener3;
    std::shared_ptr<Observer<SimpleTestObjectProxy> > obs3 =
        Observer<SimpleTestObjectProxy>::Create(&testObjectListener3);
    ASSERT_TRUE(obs3->GetStatus() == ER_OK);
    testObjectListener3.WaitFor(5);
    EXPECT_TRUE((int)testObjectListener3.nameToObjects.size() == 0);
    EXPECT_TRUE((int)testObjectListener3.removedObjectNames.size() == 0);
}

/* *
 * \test This test aims at verifying the late joiner functionality, i.e., already published
 *       objects should be able to be seen by late observers.
 *       If objects have been removed by the provider then a late joiner should not see those.
 *       Steps:
 *       -# Create an observer and add to it a listener and check that this was successful
 *       -# Publish many test objects after a valid bus connection is set-up
 *       -# Verify that all published objects can be seen by the observer
 *       -# Create another observer and add to it a new listener and check that this was successful
 *       -# Verify that all published objects can be seen by this new observer
 *       -# Destroy one of the observers
 *       -# Remove all published objects and verify that the leftover observer has seen it
 * */
TEST(LateJoiners, JoinPublishJoinDestroyRemove) {
    int numPubObjs = 20;
    shared_ptr<datadriven::ObjectAdvertiser> advertiser = ObjectAdvertiser::Create();
    ASSERT_TRUE(advertiser != nullptr);
    std::vector<unique_ptr<TestObject> > testObjects;

    //Create a new observer.
    TestObjectListener testObjectListener;
    std::shared_ptr<Observer<SimpleTestObjectProxy> > obs =
        Observer<SimpleTestObjectProxy>::Create(&testObjectListener);
    ASSERT_TRUE(obs->GetStatus() == ER_OK);

    //Publish some test objects.
    Publish(advertiser, testObjects, numPubObjs);

    //Wait until all objects have been received by the observer
    WaitOnUpdates(testObjectListener, numPubObjs);

    //A new late joiner should receive all objects.
    TestObjectListener testObjectListener2;
    std::shared_ptr<Observer<SimpleTestObjectProxy> > obs2 =
        Observer<SimpleTestObjectProxy>::Create(&testObjectListener2);
    ASSERT_TRUE(obs2->GetStatus() == ER_OK);
    WaitOnUpdates(testObjectListener2, numPubObjs);

    //Destroy one of the observers
    obs2.reset();

    //Remove all test objects
    Remove(testObjects);

    //Wait until all objects are removed
    testObjectListener.WaitOnRemove(numPubObjs, 5);
    EXPECT_TRUE((int)testObjectListener.removedObjectNames.size() == numPubObjs);

    //Check that the already destroyed observer does not receive anything
    testObjectListener2.WaitOnRemove(numPubObjs, 5);
    EXPECT_TRUE((int)testObjectListener2.removedObjectNames.size() == 0);
}

/* *
 * \test This test aims at verifying the late joiner functionality, i.e., already published
 *       objects should be able to be seen by late observers.
 *       If objects have been removed by the provider then a late joiner should not see those.
 *       Steps:
 *       -# Create an observer and add to it a listener and check that this was successful
 *       -# Publish many test objects after a valid bus connection is set-up
 *       -# Verify that all published objects can be seen by the observer
 *       -# Create another observer and add to it a new listener and check that this was successful
 *       -# Verify that all published objects can be seen by this new observer
 *       -# Destroy both observers
 *       -# Create another observer and add to it a new listener and check that this was successful
 *       -# Verify that all published objects can be seen by this new observer
 *       -# Remove all published objects and verify that the leftover observer has seen it
 * */
TEST(LateJoiners, JoinPublishJoinDestroyJoinRemove) {
    int numPubObjs = 20;
    shared_ptr<datadriven::ObjectAdvertiser> advertiser = ObjectAdvertiser::Create();
    ASSERT_TRUE(advertiser != nullptr);
    std::vector<unique_ptr<TestObject> > testObjects;

    //Create a new observer.
    TestObjectListener testObjectListener;
    std::shared_ptr<Observer<SimpleTestObjectProxy> > obs =
        Observer<SimpleTestObjectProxy>::Create(&testObjectListener);
    ASSERT_TRUE(obs->GetStatus() == ER_OK);

    //Publish some test objects
    Publish(advertiser, testObjects, numPubObjs);

    //Wait until all objects have been received by the observer
    WaitOnUpdates(testObjectListener, numPubObjs);

    //A new late joiner should receive all objects.
    TestObjectListener testObjectListener2;
    std::shared_ptr<Observer<SimpleTestObjectProxy> > obs2 =
        Observer<SimpleTestObjectProxy>::Create(&testObjectListener2);
    ASSERT_TRUE(obs2->GetStatus() == ER_OK);
    WaitOnUpdates(testObjectListener2, numPubObjs);

    //Destroy both observers
    obs.reset();
    obs2.reset();

    //A new late joiner should receive all objects.
    TestObjectListener testObjectListener3;
    std::shared_ptr<Observer<SimpleTestObjectProxy> > obs3 =
        Observer<SimpleTestObjectProxy>::Create(&testObjectListener3);
    ASSERT_TRUE(obs3->GetStatus() == ER_OK);
    WaitOnUpdates(testObjectListener3, numPubObjs);

    //Remove all test objects
    Remove(testObjects);

    //Wait until all objects are removed
    testObjectListener3.WaitOnRemove(numPubObjs, 10);
    EXPECT_TRUE((int)testObjectListener3.removedObjectNames.size() == numPubObjs);

    //Check that the already destroyed observers do not receive anything
    testObjectListener.WaitOnRemove(numPubObjs, 5);
    EXPECT_TRUE((int)testObjectListener.removedObjectNames.size() == 0);
    testObjectListener2.WaitOnRemove(numPubObjs, 5);
    EXPECT_TRUE((int)testObjectListener2.removedObjectNames.size() == 0);
}
}
//namespace
