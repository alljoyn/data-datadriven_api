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

#include "Common.h"
#include "gtest/gtest.h"

/**
 * Testing late joiners.
 */
namespace test_unit_latejoiners {
using namespace::test_unit_common;

/* *
 * \test This test aims at verifying the late joiner functionality, i.e., already published
 *       objects should be able to be seen by late observers.
 *       If objects have been removed by the provider then a late joiner should not see those.
 *       Steps:
 *       -# Publish many test objects after a valid bus connection is set-up
 *       -# After waiting for sometime, create an observer and add to it a listener and check that this was successful
 *       -# Verify that all published objects can be seen by this observer
 *       -# After waiting for sometime, create another observer and add to it a new listener and check that this was successful
 *       -# Verify that all published objects can be seen by this new observer
 *       -# After waiting for sometime, remove all published objects
 *       -# Create a new observer and add to it a new listener and check that this was successful
 *       -# Verify that the last observer cannot see any objects
 * */

TEST(LateJoiners, DISABLED_Default) {
    int numPubObjs = 2;
    BusConnection bconn;
    std::vector<unique_ptr<TestObject> > tos;
    qcc::String testObjName;

    ASSERT_TRUE(bconn.GetStatus() == ER_OK);

    //Publish some test objects knowing that there are no observers yet.
    for (int i = 0; i < numPubObjs; i++) {
        testObjName = ("TestObjectLateJoiner" + to_string(i)).c_str();
        tos.push_back(unique_ptr<TestObject>(new TestObject(bconn, testObjName)));
        ASSERT_TRUE(tos.back()->UpdateAll() == ER_OK);
    }

    usleep(numPubObjs * 3000);

    TestObjectListener testObjectListener = TestObjectListener();
    Observer<SimpleTestObjectProxy> obs(bconn, &testObjectListener);

    ASSERT_TRUE(obs.GetStatus() == ER_OK);

    usleep(numPubObjs * 3000);

    //Check that all published test objects can be seen by this late joiner observer.
    for (int i = 0; i < numPubObjs; i++) {
        testObjName = ("TestObjectLateJoiner" + to_string(i)).c_str();
        std::map<qcc::String, std::shared_ptr<SimpleTestObjectProxy> >::iterator itr =
            testObjectListener.nameToObjects.find(testObjName);
        EXPECT_TRUE(itr != testObjectListener.nameToObjects.end());
        EXPECT_TRUE(testObjName.compare(itr->first) == 0);
    }

    usleep(numPubObjs * 3000);

    //A new late joiner should receiver all objects.

    TestObjectListener testObjectListener2 = TestObjectListener();
    Observer<SimpleTestObjectProxy> obs2(bconn, &testObjectListener2);

    ASSERT_TRUE(obs2.GetStatus() == ER_OK);

    usleep(numPubObjs * 3000);

    EXPECT_TRUE((int)testObjectListener2.nameToObjects.size() == numPubObjs);

    usleep(numPubObjs * 3000);

    //**********************************************************//

    //Remove all test objects
    for (auto& i : tos) {
        i->RemoveFromBus();
    }

    usleep(numPubObjs * 3000);

    //A new late joiner should NOT receive any objects.

    TestObjectListener testObjectListener3 = TestObjectListener();
    Observer<SimpleTestObjectProxy> obs3(bconn, &testObjectListener3);

    ASSERT_TRUE(obs3.GetStatus() == ER_OK);

    usleep(numPubObjs * 3000);

    EXPECT_TRUE((int)testObjectListener3.nameToObjects.size() == 0);

    usleep(numPubObjs * 3000);

    //**********************************************************//
}
}
//namespace
