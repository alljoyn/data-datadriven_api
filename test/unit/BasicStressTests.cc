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
 * Several basic stress tests.
 */
namespace test_unit_basicstresstests {
using namespace::test_unit_common;

/**
 * \test The test aims at verifying the ability of properly reporting objects containing an invalid signature property.
 *       The VarTestObject is used in this scenario.
 *       -# Prepare a bus connection, an observer (with a listener) and one object and verify their readiness.
 *       -# Set the object's signature property to an invalid value and the other properties with valid values.
 *       -# Attempt to call UpdateAll on the object and verify that this should fail.
 * */
TEST(BasicStressTests, InvalidPropSig) {
    qcc::String invSig("-123xyzA///fjjj#$#$%^^&*&**");
    BusConnection bconn;
    unique_ptr<VarTestObject> vto;

    ASSERT_TRUE(bconn.GetStatus() == ER_OK);

    VarTestObjectListener vtestObjectListener = VarTestObjectListener();
    Observer<VarTestObjectProxy> obs(bconn, &vtestObjectListener);

    ASSERT_TRUE(obs.GetStatus() == ER_OK);

    vto = unique_ptr<VarTestObject>(new VarTestObject(bconn, "VarTestObject"));

    cout << "VarTestObject Initial State is : " << vto->GetState() << endl;
    ASSERT_TRUE(vto->GetState() != vto->ERROR);

    vto->setPropSignature(invSig);
    vto->setPropObjPath("/valid");
    vto->setPropString("valid");

    sleep(1);
    ASSERT_TRUE(vto->UpdateAll() != ER_OK);
    sleep(1);
    cout << "VarTestObject Final State is : " << vto->GetState() << endl;
}

/**
 * \test The test aims at verifying the ability of properly reporting objects containing an invalid object path property.
 *       The VarTestObject is used in this scenario.
 *       -# Prepare a bus connection, an observer (with a listener) and one object and verify their readiness.
 *       -# Set the object's object-path property to an invalid value and the other properties with valid values.
 *       -# Attempt to call UpdateAll on the object and verify that this should fail.
 * */
TEST(BasicStressTests, InvalidObjectPath) {
    qcc::String invObP("!!!");
    BusConnection bconn;
    unique_ptr<VarTestObject> vto;

    ASSERT_TRUE(bconn.GetStatus() == ER_OK);

    VarTestObjectListener vtestObjectListener = VarTestObjectListener();
    Observer<VarTestObjectProxy> obs(bconn, &vtestObjectListener);

    ASSERT_TRUE(obs.GetStatus() == ER_OK);

    vto = unique_ptr<VarTestObject>(new VarTestObject(bconn, "VarTestObject"));

    cout << "VarTestObject Initial State is : " << vto->GetState() << endl;
    ASSERT_TRUE(vto->GetState() != vto->ERROR);

    vto->setPropSignature("");
    vto->setPropObjPath(invObP);
    vto->setPropString("valid");

    sleep(1);
    ASSERT_TRUE(vto->UpdateAll() != ER_OK);
    sleep(1);
    cout << "VarTestObject Final State is : " << vto->GetState() << endl;
}

/**
 * \test The test aims at verifying the ability of properly reporting objects containing oversized data meant to be sent over.
 *       The VarTestObject is used in this scenario.
 *       -# Prepare a bus connection, an observer (with a listener) and one object and verify their readiness.
 *       -# Set the object's string property with an oversized text and the other properties with valid values.
 *       -# Attempt to call UpdateAll on the object and verify that this should fail.
 * */
TEST(BasicStressTests, DISABLED_MassiveObjectSize) {
    BusConnection bconn;
    unique_ptr<VarTestObject> vto;

    ASSERT_TRUE(bconn.GetStatus() == ER_OK);

    VarTestObjectListener vtestObjectListener = VarTestObjectListener();
    Observer<VarTestObjectProxy> obs(bconn, &vtestObjectListener);

    ASSERT_TRUE(obs.GetStatus() == ER_OK);

    vto = unique_ptr<VarTestObject>(new VarTestObject(bconn, "VarTestObject"));

    cout << "VarTestObject Initial State is : " << vto->GetState() << endl;
    ASSERT_TRUE(vto->GetState() != vto->ERROR);

    qcc::String tmp(BIG_TEXT);

    while (tmp.size() <= ajn::ALLJOYN_MAX_PACKET_LEN * 2) {
        tmp.append(BIG_TEXT);
    }

    vto->setPropSignature("");
    vto->setPropObjPath("/valid");
    vto->setPropString(tmp);

    sleep(1);
    ASSERT_TRUE(vto->UpdateAll() != ER_OK);
    sleep(1);
    cout << "VarTestObject final State is : " << vto->GetState() << endl;
}

/**
 * \test The test aims at verifying the ability to call methods and emit signals are a high frequency.
 *       The SimpleTestObject is used in this scenario.
 *       -# Publish many test objects after a valid bus connection, observer and (Signal)listener are set-up
 *       -# Verify that method calls on all published objects (as a consumer) and signaling (as a provider) at a high frequency is viable
 *       -# Verify successful listeners unregistration
 * */
TEST(BasicStressTests, FrequentMethodCallsAndSignals) {
    int numPubObjs = 10;
    int stress = 1000;
    BusConnection bconn;
    std::vector<unique_ptr<TestObject> > tos;
    qcc::String testObjName;

    ASSERT_TRUE(bconn.GetStatus() == ER_OK);

    TestObjectListener testObjectListener = TestObjectListener();
    Observer<SimpleTestObjectProxy> obs(bconn, &testObjectListener);

    ASSERT_TRUE(obs.GetStatus() == ER_OK);

    TestObjectSignalListener tosl = TestObjectSignalListener();
    ASSERT_TRUE(obs.AddSignalListener<SimpleTestObjectProxy::Test>(tosl) == ER_OK);

    for (int i = 0; i < numPubObjs; i++) {
        testObjName = ("TestObject" + to_string(i)).c_str();
        tos.push_back(unique_ptr<TestObject>(new TestObject(bconn, testObjName)));

        cout << testObjName.c_str() << " Initial State is : " << tos.back()->GetState() << endl;
        ASSERT_TRUE(tos.back()->GetState() != tos.back()->ERROR);
        ASSERT_TRUE(tos.back()->UpdateAll() == ER_OK);
    }

    usleep(numPubObjs * 3000);

    std::map<qcc::String, std::shared_ptr<SimpleTestObjectProxy> >::iterator it;
    SimpleTestObjectProxy::ExecuteReply reply;
    const qcc::String test_str(DEFAULT_TEST_NAME);
    while (stress > 0) {
        it = testObjectListener.nameToObjects.begin();
        for (int i = 0; it != testObjectListener.nameToObjects.end(); ++it) {
            std::shared_ptr<datadriven::MethodInvocation<SimpleTestObjectProxy::ExecuteReply> > invocation(
                it->second->Execute());
            reply = invocation->GetReply();
            EXPECT_TRUE(reply.GetStatus() == ER_OK);
            EXPECT_TRUE(reply.success);

            tos[i]->Test(test_str);
            ++i;
        }
        --stress;
    }

    usleep(numPubObjs * 3000);

    EXPECT_TRUE(obs.RemoveSignalListener<SimpleTestObjectProxy::Test>(tosl) == ER_OK);
}
}
//namespace
