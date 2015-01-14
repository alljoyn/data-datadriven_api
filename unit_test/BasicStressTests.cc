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

#include <gtest/gtest.h>

#include "Common.h"

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
    unique_ptr<VarTestObject> vto;
    shared_ptr<datadriven::ObjectAdvertiser> advertiser = ObjectAdvertiser::Create();

    ASSERT_TRUE(advertiser != nullptr);

    vto = unique_ptr<VarTestObject>(new VarTestObject(advertiser, "VarTestObject"));

    cout << "VarTestObject Initial State is : " << vto->GetState() << endl;
    ASSERT_TRUE(vto->GetState() != vto->ST_ERROR);

    vto->setPropSignature(invSig);
    vto->setPropObjPath("/valid");
    vto->setPropString("valid");

    ASSERT_TRUE(vto->UpdateAll() != ER_OK);
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
    unique_ptr<VarTestObject> vto;
    shared_ptr<datadriven::ObjectAdvertiser> advertiser = ObjectAdvertiser::Create();

    ASSERT_TRUE(advertiser != nullptr);

    vto = unique_ptr<VarTestObject>(new VarTestObject(advertiser, "VarTestObject"));

    cout << "VarTestObject Initial State is : " << vto->GetState() << endl;
    ASSERT_TRUE(vto->GetState() != vto->ST_ERROR);

    vto->setPropSignature("");
    vto->setPropObjPath(invObP);
    vto->setPropString("valid");

    ASSERT_TRUE(vto->UpdateAll() != ER_OK);
    cout << "VarTestObject Final State is : " << vto->GetState() << endl;
}

/**
 * \test The test aims at verifying the ability of properly reporting objects containing oversized data meant to be sent over.
 *       The VarTestObject is used in this scenario.
 *       -# Prepare a bus connection, an observer (with a listener) and one object and verify their readiness.
 *       -# Set the object's string property with an oversized text and the other properties with valid values.
 *       -# Attempt to call UpdateAll on the object and verify that this should fail.
 * */
TEST(BasicStressTests, MassiveObjectSize) {
    // keep objects first, they should survive the advertiser
    unique_ptr<VarTestObject> validTestObject;
    unique_ptr<VarTestObject> vto;
    shared_ptr<ObjectAdvertiser> advertiser = ObjectAdvertiser::Create();

    ASSERT_TRUE(advertiser != nullptr);

    VarTestObjectListener vtestObjectListener;
    std::shared_ptr<Observer<VarTestObjectProxy> > obs = Observer<VarTestObjectProxy>::Create(&vtestObjectListener);

    ASSERT_TRUE(obs->GetStatus() == ER_OK);

    //First make sure the consumer has at least received one valid object. That way we are sure that
    //at least there is one consumer listening.
    //The reasoning behind this is because the UpdateAll method will only fail due to an bad packet
    //length IF it really needs to be sent -> if there are consumers listening.
    validTestObject = unique_ptr<VarTestObject>(new VarTestObject(advertiser, "ValidTestObject"));
    validTestObject->setPropSignature("");
    validTestObject->setPropObjPath("/valid");
    validTestObject->setPropString("valid");
    ASSERT_TRUE(validTestObject->UpdateAll() == ER_OK);
    vtestObjectListener.Wait();

    vto = unique_ptr<VarTestObject>(new VarTestObject(advertiser, "VarTestObject"));

    cout << "VarTestObject Initial State is : " << vto->GetState() << endl;
    ASSERT_TRUE(vto->GetState() != vto->ST_ERROR);

    qcc::String tmp(BIG_TEXT);
    while (tmp.size() < (ajn::ALLJOYN_MAX_PACKET_LEN * 2)) {
        tmp.append(BIG_TEXT);
    }

    vto->setPropSignature("");
    vto->setPropObjPath("/valid");
    vto->setPropString(tmp);

    ASSERT_TRUE(vto->UpdateAll() == ER_BUS_BAD_BODY_LEN);
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
    std::vector<unique_ptr<TestObject> > tos;
    shared_ptr<datadriven::ObjectAdvertiser> advertiser = ObjectAdvertiser::Create();
    qcc::String testObjName;

    ASSERT_TRUE(advertiser != nullptr);

    TestObjectListener testObjectListener;
    std::shared_ptr<Observer<SimpleTestObjectProxy> > obs = Observer<SimpleTestObjectProxy>::Create(
        &testObjectListener);

    ASSERT_TRUE(obs->GetStatus() == ER_OK);

    TestObjectSignalListener tosl;
    ASSERT_TRUE(obs->AddSignalListener<SimpleTestObjectProxy::Test>(tosl) == ER_OK);

    for (int i = 0; i < numPubObjs; i++) {
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "TestObject%d", i);
        testObjName = qcc::String(buffer);
        tos.push_back(unique_ptr<TestObject>(new TestObject(advertiser, testObjName)));

        cout << testObjName.c_str() << " Initial State is : " << tos.back()->GetState() << endl;
        ASSERT_TRUE(tos.back()->GetState() != tos.back()->ST_ERROR);
        ASSERT_TRUE(tos.back()->UpdateAll() == ER_OK);
        testObjectListener.Wait();
    }

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

            EXPECT_EQ(ER_OK, tos[i]->Test(test_str));

            tosl.Wait();

            ++i;
        }
        --stress;
    }

    EXPECT_TRUE(obs->RemoveSignalListener<SimpleTestObjectProxy::Test>(tosl) == ER_OK);
}

/**
 * \test This tests aims to verify whether the reply object of a method call
 * will have an error state if the method failed due to marshalling reasons
 */
TEST(BasicStressTests, MethodCallInvalidArgument) {
    unique_ptr<TestObject> vto;
    shared_ptr<datadriven::ObjectAdvertiser> advertiser = ObjectAdvertiser::Create();

    ASSERT_TRUE(advertiser != nullptr);

    vto = unique_ptr<TestObject>(new TestObject(advertiser, "TestObject"));

    ASSERT_NE(vto->ST_ERROR, vto->GetState());
    ASSERT_EQ(ER_OK, vto->UpdateAll());

    TestObjectListener testObjectListener;
    std::shared_ptr<Observer<SimpleTestObjectProxy> > obs = Observer<SimpleTestObjectProxy>::Create(
        &testObjectListener);
    TestObjectSignalListener tosl;
    ASSERT_EQ(ER_OK, obs->AddSignalListener<SimpleTestObjectProxy::Test>(tosl));
    testObjectListener.Wait();

    /* Make a very long string */
    qcc::String tmp(BIG_TEXT);
    while (tmp.size() < (ajn::ALLJOYN_MAX_PACKET_LEN * 2)) {
        tmp.append(BIG_TEXT);
    }

    for (auto& it : testObjectListener.nameToObjects) {
        /* Normal case */
        std::shared_ptr<datadriven::MethodInvocation<SimpleTestObjectProxy::ExecuteReply> > invocation(
            it.second->Execute());
        SimpleTestObjectProxy::ExecuteReply reply = invocation->GetReply();
        EXPECT_EQ(ER_OK, reply.GetStatus());
        EXPECT_TRUE(reply.success);

        /* Fail case */
        std::shared_ptr<datadriven::MethodInvocation<SimpleTestObjectProxy::ExecuteWithArgReply> > invocation2(
            it.second->ExecuteWithArg(tmp));
        SimpleTestObjectProxy::ExecuteWithArgReply reply2 = invocation2->GetReply();
        EXPECT_EQ(ER_BUS_BAD_BODY_LEN, reply2.GetStatus());
    }
}

/**
 * \test This tests aims to verify sending out a signal will return an error
 *       if marshalling an argument fails.
 *
 *       Remark: With ajn::SESSION_ID_ALL_HOSTED we have to make sure
 *       we create an observer otherwise there will be no session to send the signal on
 *       and no marshaling of the data will be done, causing the test to fail.
 */
TEST(BasicStressTests, SignalInvalidArgument) {
    unique_ptr<TestObject> vto;
    shared_ptr<datadriven::ObjectAdvertiser> advertiser = ObjectAdvertiser::Create();

    ASSERT_TRUE(advertiser != nullptr);

    TestObjectListener testObjectListener;
    std::shared_ptr<Observer<SimpleTestObjectProxy> > obs = Observer<SimpleTestObjectProxy>::Create(
        &testObjectListener);

    ASSERT_TRUE(obs->GetStatus() == ER_OK);

    TestObjectSignalListener tosl;
    ASSERT_TRUE(obs->AddSignalListener<SimpleTestObjectProxy::Test>(tosl) == ER_OK);

    vto = unique_ptr<TestObject>(new TestObject(advertiser, "TestObject"));

    ASSERT_NE(vto->ST_ERROR, vto->GetState());
    ASSERT_EQ(ER_OK, vto->UpdateAll());

    testObjectListener.Wait();

    /* Make a very long string */
    qcc::String tmp(BIG_TEXT);
    while (tmp.size() < (ajn::ALLJOYN_MAX_PACKET_LEN * 2)) {
        tmp.append(BIG_TEXT);
    }

    ASSERT_EQ(ER_BUS_BAD_BODY_LEN, vto->Test(tmp));
}
}
//namespace
