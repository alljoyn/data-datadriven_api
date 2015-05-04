/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
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

#include <memory>
#include <unistd.h>

#include <gtest/gtest.h>

#include <alljoyn/Status.h>
#include "BusConnectionImpl.h"

/**
 * Bus connection tests.
 */
namespace test_unit_busconnection {
using namespace datadriven;

/**
 * \test Test the default constructor
 * */
TEST(BusConnectionBasicTest, DefaultConstructor) {
    std::shared_ptr<BusConnectionImpl> bc = BusConnectionImpl::GetInstance();
    EXPECT_TRUE(bc->GetStatus() == ER_OK) << "Could not initialize BusConnection !";
}

/**
 * \test Stress-test BusConnection creation.
 * */
TEST(BusConnectionBasicTest, CreateDeleteRepeat) {
    std::shared_ptr<BusConnectionImpl> bc;
    int i = 0;

    while (i < 10) {
        bc = BusConnectionImpl::GetInstance();
        EXPECT_TRUE(nullptr != bc) << "Could not create BusConnection !";
        bc.reset();
        usleep(30000);
        i++;
    }
}
};
