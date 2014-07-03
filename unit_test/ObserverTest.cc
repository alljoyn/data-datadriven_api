/******************************************************************************
 * Copyright (c) 2009-2013, AllSeen Alliance. All rights reserved.
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
 * Observer tests.
 */
namespace test_unit_observer {
using namespace::test_unit_common;

#define TRIES 30

/**
 * \test Correct behaviour of an observer without a listener.
 *       -# validate listener code for add, update and remove of objects
 *       -# validate iterator code for add, update and remove of objects
 * */
TEST(ObserverTests, NoListener)
{
    int i;

    shared_ptr<datadriven::ObjectAdvertiser> advertiser = ObjectAdvertiser::Create();
    ASSERT_TRUE(advertiser != nullptr);

    TestObject obj(advertiser, "xyz");

    std::shared_ptr<Observer<SimpleTestObjectProxy> > obs = Observer<SimpleTestObjectProxy>::Create(nullptr);
    ASSERT_TRUE(obs != nullptr);

    /* 1) publish an object */
    ASSERT_EQ(ER_OK, obj.PutOnBus());
    /* wait for object by polling iterator */
    i = 0;
    while ((obs->begin() == obs->end()) && (i < TRIES)) {
        sleep(1);
        i++;
    }
    ASSERT_NE(TRIES, i);
    /* only one object should be present */
    ASSERT_TRUE(obs->end() == ++obs->begin());
    /* and it should contain valid data */
    ASSERT_TRUE(0 == obs->begin()->GetProperties().name.compare("xyz"));

    /* 2) update object */
    obj.SetName("abc");
    obj.UpdateAll();
    /* wait for update by polling */
    i = 0;
    while ((0 != obs->begin()->GetProperties().name.compare("abc")) && (i < TRIES)) {
        sleep(1);
        i++;
    }
    ASSERT_NE(TRIES, i);
    /* still only one object should be present */
    ASSERT_TRUE(obs->end() == ++obs->begin());

    /* 3) remove object */
    obj.RemoveFromBus();
    /* wait for object removal by polling iterator */
    while (obs->begin() != obs->end()) {
        sleep(1);
    }
}
}
//namespace
