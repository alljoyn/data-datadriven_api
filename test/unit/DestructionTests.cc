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
 * Testing proper destruction.
 */
namespace test_unit_destruction {
using namespace::test_unit_common;

/* *
 * \test Verify that the order of BusConnection creation (after a certain object on the stack) would not cause a segfault
 *       if the object is not removed explicitly from the bus when exiting.
 *       Steps:
 *       -# Create a unique pointer to a TestObject
 *       -# Create a BusConnection
 *       -# Create an observer with a simple TestObjectlistener
 *       -# Assign a new TestObject to the unique pointer and invoke UpdateAll
 *       -# Verify the state of the test object and end the test
 * */

TEST(Destruction, DISABLED_FirstScenario) {
    unique_ptr<TestObject> sto;
    BusConnection bconn;

    ASSERT_TRUE(bconn.GetStatus() == ER_OK);

    TestObjectListener stestObjectListener = TestObjectListener();
    Observer<SimpleTestObjectProxy> obs(bconn, &stestObjectListener);

    ASSERT_TRUE(obs.GetStatus() == ER_OK);

    sto = unique_ptr<TestObject>(new TestObject(bconn, "SimpleTestObject"));

    sleep(1);
    ASSERT_TRUE(sto->UpdateAll() == ER_OK);
    sleep(1);

    ASSERT_TRUE(sto->GetState() != sto->ERROR);
}

/* *
 * \test Verify that the removal of a signal listener at the end of the scope works fine.
 *       Steps:
 *       -# Create a BusConnection
 *       -# Create a unique pointer to a TestObject
 *       -# Create an observer with a simple TestObjectlistener
 *       -# Attach a TestObjectSignalListener to the observer
 *       -# Assign a new TestObject to the unique pointer and invoke UpdateAll
 *       -# Issue a signal "Test"
 *       -# Verify the state of the test object
 *       -# Remove signal listener
 * */

TEST(Destruction, SecondScenario) {
    BusConnection bconn;
    unique_ptr<TestObject> sto;

    ASSERT_TRUE(bconn.GetStatus() == ER_OK);

    TestObjectListener stestObjectListener = TestObjectListener();
    TestObjectSignalListener tosl = TestObjectSignalListener();

    Observer<SimpleTestObjectProxy> obs(bconn, &stestObjectListener);
    ASSERT_TRUE(obs.AddSignalListener<SimpleTestObjectProxy::Test>(tosl) == ER_OK);

    ASSERT_TRUE(obs.GetStatus() == ER_OK);

    sto = unique_ptr<TestObject>(new TestObject(bconn, "SimpleTestObject"));

    ASSERT_TRUE(sto->UpdateAll() == ER_OK);
    sto->Test(DEFAULT_TEST_NAME);

    ASSERT_TRUE(sto->GetState() != sto->ERROR);
    ASSERT_TRUE(obs.RemoveSignalListener<SimpleTestObjectProxy::Test>(tosl) == ER_OK);
}

/* *
 * \test Verify that the removal of several objects from the bus -regardless whether they were declared before or after the BusConnection- works.
 *       Several observers are involved.
 *       Steps:
 *       -# Create a vector of unique pointers to TestObjects before the BusConnection creation
 *       -# Create a BusConnection
 *       -# Create a vector of unique pointers to TestObjects after the BusConnection creation
 *       -# Create a vector of unique pointers to observers
 *       -# Fill in the vector of unique pointers to observes with new observer objects with TestObjectlistener and TestObjectSignalListener
 *       -# Assign new TestObjects to the the aforementioned vectors of unique pointers to TestObjects and invoke UpdateAll and issue signal "Test" on all
 *       -# Remove all TestObjects from the bus and verify the state of their state
 * */

TEST(Destruction, ThirdScenario) {
    vector<unique_ptr<TestObject> > tosBeforeConn;
    BusConnection bconn;
    vector<unique_ptr<TestObject> > tosAfterConn;

    vector <unique_ptr<Observer<SimpleTestObjectProxy> > > vObservers;

    qcc::String testObjName;
    int numOfObjects = 100;
    int numOfObservers = 100;

    TestObjectListener stestObjectListener = TestObjectListener();
    TestObjectSignalListener tosl = TestObjectSignalListener();

    ASSERT_TRUE(bconn.GetStatus() == ER_OK);

    for (int i = 0; i < numOfObservers; i++) {
        vObservers.push_back(unique_ptr<Observer<SimpleTestObjectProxy> >(new Observer<SimpleTestObjectProxy>(bconn,
                                                                                                              &
                                                                                                              stestObjectListener)));
        ASSERT_TRUE(vObservers.back()->AddSignalListener<SimpleTestObjectProxy::Test>(tosl) == ER_OK);
        ASSERT_TRUE(vObservers.back()->GetStatus() == ER_OK);
    }

    for (int i = 0; i < numOfObjects; i++) {
        testObjName = ("TestObjectBeforeConn" + to_string(i)).c_str();
        tosBeforeConn.push_back(unique_ptr<TestObject>(new TestObject(bconn, testObjName)));

        testObjName = ("TestObjectAfterConn" + to_string(i)).c_str();
        tosAfterConn.push_back(unique_ptr<TestObject>(new TestObject(bconn, testObjName)));

        ASSERT_TRUE(tosBeforeConn.back()->UpdateAll() == ER_OK);
        ASSERT_TRUE(tosAfterConn.back()->UpdateAll() == ER_OK);

        tosBeforeConn.back()->Test(DEFAULT_TEST_NAME);
        tosBeforeConn.back()->Test(DEFAULT_TEST_NAME);
    }

    sleep(1);

    for (int i = 0; i < numOfObjects; i++) {
        ASSERT_TRUE(tosBeforeConn[i]->GetState() != tosBeforeConn[i]->ERROR);
        tosBeforeConn[i]->RemoveFromBus();
        ASSERT_TRUE(tosBeforeConn[i]->GetState() == tosBeforeConn[i]->REMOVED);
        QCC_DbgPrintf(("Removed : %s", tosBeforeConn[i]->GetPath()));

        ASSERT_TRUE(tosAfterConn[i]->GetState() != tosAfterConn[i]->ERROR);
        tosAfterConn[i]->RemoveFromBus();
        ASSERT_TRUE(tosAfterConn[i]->GetState() == tosAfterConn[i]->REMOVED);
        QCC_DbgPrintf(("Removed : %s", tosAfterConn[i]->GetPath()));
    }

    sleep(1);
}
}
