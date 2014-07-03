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
#include <datadriven/Semaphore.h>
#include "Common.h"

/**
 * Iterating on objects from listener's OnUpdate and OnRemove test.
 */
namespace test_unit_readobjects {
using namespace::test_unit_common;

class TestReadListener :
    public Observer<SimpleTestObjectProxy>::Listener {
  private:
    Semaphore& sem;

  public:
    Observer<SimpleTestObjectProxy>* obs;
    typedef std::set<std::shared_ptr<SimpleTestObjectProxy> > ObjectSet;
    ObjectSet objects;

    ObjectSet iterate() const
    {
        ObjectSet tmp;
        for (datadriven::Observer<SimpleTestObjectProxy>::iterator it = obs->begin(); it != obs->end(); ++it) {
            tmp.insert(*it);
        }

        return tmp; /* RVO */
    }

  public:

    TestReadListener(Semaphore& _sem) :
        sem(_sem), obs(nullptr)
    {
    }

    void setObserver(Observer<SimpleTestObjectProxy>* obs)
    {
        this->obs = obs;
    }

    virtual void OnUpdate(const std::shared_ptr<SimpleTestObjectProxy>& p)
    {
        if (objects.find(p) == objects.end()) {
            objects.insert(p);
            ObjectSet read = iterate();

            ASSERT_EQ(objects, read);
            sem.Post();
        }
    }

    virtual void OnRemove(const std::shared_ptr<SimpleTestObjectProxy>& p)
    {
        objects.erase(p);
        ObjectSet read = iterate();

        ASSERT_EQ(objects, read);
        sem.Post();
    }
};

/**
 * \test The test aims at verifying we can iterate over objects from OnUpdate() and OnRemove()
 *
 * */
TEST(IterateFromCallback, Default) {
    unsigned int numPubObjs = 5; //Increase for stress testing
    shared_ptr<datadriven::ObjectAdvertiser> advertiser = ObjectAdvertiser::Create();
    std::vector<unique_ptr<TestObject> > tos;
    qcc::String testObjName;
    Semaphore sem;

    ASSERT_TRUE(advertiser != nullptr);

    TestReadListener testObjectListener(sem);
    std::shared_ptr<Observer<SimpleTestObjectProxy> > obs = Observer<SimpleTestObjectProxy>::Create(&testObjectListener);
    testObjectListener.setObserver(obs.get());

    ASSERT_EQ(obs->GetStatus(), ER_OK);

    for (unsigned int i = 0; i < numPubObjs; i++) {
        testObjName = ("TestObject" + to_string(i)).c_str();
        tos.push_back(unique_ptr<TestObject>(new TestObject(advertiser, testObjName)));

        cout << testObjName.c_str() << " Initial State is : " << tos.back()->GetState() << endl;
        ASSERT_TRUE(tos.back()->UpdateAll() == ER_OK);
        sem.Wait();
    }

    ASSERT_EQ(testObjectListener.objects.size(), numPubObjs);

    for (auto& i : tos) {
        i->RemoveFromBus();
        sem.Wait();
    }

    ASSERT_EQ(testObjectListener.objects.size(), 0U);
}

class TestIterateRemoveListener :
    public Observer<SimpleTestObjectProxy>::Listener {
  private:
    Semaphore& sem;

  public:

    TestIterateRemoveListener(Semaphore& _sem) :
        sem(_sem)
    {
    }

    virtual void OnUpdate(const std::shared_ptr<SimpleTestObjectProxy>& p)
    {
        sem.Post();
    }

    virtual void OnRemove(const std::shared_ptr<SimpleTestObjectProxy>& p)
    {
        sem.Post();
    }
};

/**
 * \test The test aims at verifying the behaviour of iterating over the observer while the object are being removed
 *
 * */
TEST(IterateWhileRemoved, Default) {
    shared_ptr<datadriven::ObjectAdvertiser> advertiser = ObjectAdvertiser::Create();
    qcc::String testObjName;
    Semaphore sem;

    ASSERT_TRUE(advertiser != nullptr);

    TestIterateRemoveListener testObjectListener(sem);
    std::shared_ptr<Observer<SimpleTestObjectProxy> > obs = Observer<SimpleTestObjectProxy>::Create(&testObjectListener);

    testObjName = "TestObject";
    TestObject to(advertiser, testObjName);

    ASSERT_TRUE(to.UpdateAll() == ER_OK);
    sem.Wait();

    int count = 0;
    auto observer_it = obs->begin();
    auto observer_end = obs->end();
    to.RemoveFromBus();
    sem.Wait();

    /* Assume the object is gone at this moment in time */
    std::shared_ptr<SimpleTestObjectProxy> proxy;
    for (; observer_it != observer_end; ++observer_it) {
        proxy = *observer_it;
        SimpleTestObjectProxy::Properties props = observer_it->GetProperties();
        /* can we still retrieve its state ? */
        ASSERT_EQ(props.name, testObjName);

        /* Calling method should fail (object is dead) */
        std::shared_ptr<datadriven::MethodInvocation<SimpleTestObjectProxy::ExecuteReply> > reply = proxy->Execute();
        ASSERT_EQ(reply->GetReply().GetStatus(), ER_FAIL);

        ++count;
    }
    ASSERT_EQ(count, 1);

    to.UpdateAll();
    sem.Wait();

    /* Calling method should pass (object is alive again) */
    std::shared_ptr<datadriven::MethodInvocation<SimpleTestObjectProxy::ExecuteReply> > reply = proxy->Execute();
    ASSERT_EQ(reply->GetReply().GetStatus(), ER_OK);
}
}
//namespace
