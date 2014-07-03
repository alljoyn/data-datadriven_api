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

#include <qcc/Thread.h>
#include <datadriven/Semaphore.h>

using namespace datadriven;

/**
 * Tests involving properties.
 */
namespace test_unit_semaphore {
class MyThreadData {
  public:
    MyThreadData(Semaphore* sem,
                 bool wait,
                 bool timed = false) :
        status(ER_OK), sem(sem), wait(wait), timed(timed)
    { }

    QStatus status;
    Semaphore* sem;
    bool wait;
    bool timed;
};

class MyThread :
    public qcc::Thread {
  public:
    MyThread() { }

    virtual ~MyThread() { }

    virtual qcc::ThreadReturn STDCALL Run(void* arg)
    {
        MyThreadData* data = (MyThreadData*)arg;

        if (data->wait) {
            if (data->timed) {
                data->status = data->sem->TimedWait(5000);
            } else {
                data->status = data->sem->Wait();
            }
        } else {
            data->status = data->sem->Post();
        }
        return data;
    }
};

class SemaphoreTests :
    public testing::Test {
  public:
    SemaphoreTests() { }

    virtual ~SemaphoreTests() { }

    MyThread t1;
    MyThread t2;
};

/**
 * \test Wait before post.
 */
TEST_F(SemaphoreTests, WaitBeforePost)
{
    Semaphore sem;
    MyThreadData wait_data(&sem, true);
    MyThreadData post_data(&sem, false);

    t1.Start(&wait_data);
    qcc::Sleep(1000); // ensure thread 1 is started and waiting
    t2.Start(&post_data);
    t1.Join();
    t2.Join();
    ASSERT_EQ(ER_OK, wait_data.status);
    ASSERT_EQ(ER_OK, post_data.status);
}

/**
 * \test Post before wait.
 */
TEST_F(SemaphoreTests, PostBeforeWait)
{
    Semaphore sem;
    MyThreadData wait_data(&sem, true);
    MyThreadData post_data(&sem, false);

    t1.Start(&post_data);
    qcc::Sleep(1000); // ensure thread 1 is started and has posted
    t2.Start(&wait_data);
    t1.Join();
    t2.Join();
    ASSERT_EQ(ER_OK, wait_data.status);
    ASSERT_EQ(ER_OK, post_data.status);
}

/**
 * \test Initial value.
 */
TEST_F(SemaphoreTests, InitialValue)
{
    Semaphore sem(2);
    ASSERT_EQ(ER_OK, sem.Wait());
    ASSERT_EQ(ER_OK, sem.TimedWait(500));
}

/**
 * \test Time-out.
 */
TEST_F(SemaphoreTests, TimeOut)
{
    Semaphore sem(0);
    ASSERT_EQ(ER_TIMEOUT, sem.TimedWait(500));
    ASSERT_EQ(ER_TIMEOUT, sem.TimedWait(0));
    // ensure no time-out if already posted
    ASSERT_EQ(ER_OK, sem.Post());
    ASSERT_EQ(ER_OK, sem.TimedWait(500));
}

/**
 * \test Timed wait before post.
 */
TEST_F(SemaphoreTests, TimedWaitBeforePost)
{
    Semaphore sem;
    MyThreadData wait_data(&sem, true, true);
    MyThreadData post_data(&sem, false);

    t1.Start(&wait_data);
    qcc::Sleep(1000); // ensure thread 1 is started and waiting
    t2.Start(&post_data);
    t1.Join();
    t2.Join();
    ASSERT_EQ(ER_OK, wait_data.status);
    ASSERT_EQ(ER_OK, post_data.status);
}
}
