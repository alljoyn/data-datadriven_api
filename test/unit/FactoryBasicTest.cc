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

#include <datadriven/BusConnection.h>
#include "BusConnectionImpl.h"
#include <unistd.h>
#include <alljoyn/Status.h>
#include "gtest/gtest.h"

/**
 * Bus connection tests.
 */
namespace test_unit_busconnection {
using namespace datadriven;

/**
 * \test Test the default constructor
 * */
TEST(BusConnectionBasicTest, DefaultConstructor) {
    BusConnectionImpl bc;
    EXPECT_TRUE(bc.GetStatus() == ER_OK) << "Could not initialize BusConnection !";
    EXPECT_TRUE(bc.GetConsumerSessionManager().GetStatus() ==
                ER_OK) << "Could not initialize consumer SessionManager !";
    EXPECT_TRUE(bc.GetProviderSessionManager().GetStatus() ==
                ER_OK) << "Could not initialize provider SessionManager !";
}

/**
 * \test Stress-test BusConnection creation.
 * */
TEST(BusConnectionBasicTest, CreateDeleteRepeat) {
    std::unique_ptr<BusConnection> bc;
    int i = 0;

    while (i < 10) {
        bc = std::unique_ptr<BusConnection>(new BusConnection());
        EXPECT_TRUE(nullptr != bc) << "Could not create BusConnection !";
        bc.reset();
        usleep(30000);
        i++;
    }
}
};
