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

#ifndef UNIT_T_COMMON_H_
#define UNIT_T_COMMON_H_

#include <datadriven/datadriven.h>
#include <datadriven/Semaphore.h>
#include <SimpleTestObjectInterface.h>
#include <SimpleTestObjectProxy.h>
#include <ExtensionInterface.h>
#include <ExtensionProxy.h>
#include <VarTestObjectInterface.h>
#include <VarTestObjectProxy.h>
#undef QCC_MODULE
#include <qcc/Debug.h>
#define QCC_MODULE "DD_TEST"

namespace test_unit_common {
#define DEFAULT_TEST_NAME "Test"

#define BIG_TEXT \
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam sollicitudin mi vel lacus fringilla congue.\
     Nulla porta quam non mauris semper vulputate. Donec sed elit a ipsum placerat sodales eget quis enim. Sed magna ligula,\
     varius in lorem a, tincidunt suscipit nisi. Phasellus mi mauris, elementum vel eleifend et, luctus vitae eros. Ut aliquam\
     erat neque, at aliquam massa consectetur gravida. Aliquam ac posuere lorem. Sed at lectus et dolor commodo mattis.\
     Interdum et malesuada fames ac ante ipsum primis in faucibus."

using namespace std;
using namespace datadriven;
using namespace::gen::org_allseenalliance_test;

class TestObject :
    public ProvidedObject, public SimpleTestObjectInterface, public ExtensionInterface {
  public:
    TestObject(shared_ptr<datadriven::ObjectAdvertiser> advertiser,
               const qcc::String& name,
               const qcc::String& othername = "hello world") :
        ProvidedObject(advertiser), SimpleTestObjectInterface(this), ExtensionInterface(this)
    {
        this->name = name;
        this->othername = othername;
    }

    void Execute(std::shared_ptr<ExecuteReply> reply)
    {
        reply->Send(true);
    }

    void ExecuteWithArg(const qcc::String& in, std::shared_ptr<ExecuteWithArgReply> reply)
    {
        reply->Send(true);
    }

    void SetName(const qcc::String& name)
    {
        this->name = name;
    }
};

class TestObjectListener :
    public Observer<SimpleTestObjectProxy>::Listener {
  public:
    std::map<qcc::String, std::shared_ptr<SimpleTestObjectProxy> > nameToObjects;
    std::map<qcc::String, unsigned int> countObjects;
    std::vector<qcc::String> removedObjectNames;
    std::vector<qcc::String> updatedObjectNames; // special cases

  public:

    TestObjectListener() :
        semaphore(), mutex(), numberOfObservers(1)
    {
    }

    TestObjectListener(int numberOfObservers) :
        semaphore(), numberOfObservers(numberOfObservers)
    {
    }

    ~TestObjectListener()
    {
    }

    virtual void OnUpdate(const std::shared_ptr<SimpleTestObjectProxy>& p)
    {
        SimpleTestObjectProxy::Properties prop = p->GetProperties();
        QCC_DbgPrintf(("%p RECEIVED OBJ: %s (path = %s)", this, prop.name.c_str(),
                       p->GetObjectId().GetBusObjectPath().c_str()));
        bool releaseSem = false;
        mutex.Lock();
        std::map<qcc::String, unsigned int>::iterator found = countObjects.find(prop.name);
        if (found == countObjects.end()) {
            nameToObjects[prop.name] = p;
            countObjects[prop.name] = 1;
            updatedObjectNames.push_back(prop.name);
            releaseSem = true;
        } else {
            found->second++;
            if (found->second <= numberOfObservers) {
                updatedObjectNames.push_back(prop.name);
                releaseSem = true;
            }
        }
        mutex.Unlock();
        if (releaseSem) {
            semaphore.Post();
        }
    }

    virtual void OnRemove(const std::shared_ptr<SimpleTestObjectProxy>& p)
    {
        SimpleTestObjectProxy::Properties prop = p->GetProperties();
        mutex.Lock();
        removedObjectNames.push_back(prop.name);
        mutex.Unlock();
        QCC_DbgPrintf(("%p REMOVED OBJ: %s (path = %s)", this, prop.name.c_str(),
                       p->GetObjectId().GetBusObjectPath().c_str()));
        semaphore.Post();
    }

    void Wait(void)
    {
        semaphore.Wait();
    }

    void WaitFor(uint32_t sec)
    {
        semaphore.TimedWait(sec * 1000);
    }

    void WaitOnUpdate(unsigned int numberOfObjects, uint32_t sec = 0)
    {
        while (true) {
            mutex.Lock();
            if (nameToObjects.size() == numberOfObjects) {
                mutex.Unlock();
                break;
            }
            mutex.Unlock();
            if (!sec) {
                semaphore.Wait();
            } else {
                if (ER_TIMEOUT == semaphore.TimedWait(sec * 1000)) {
                    break;
                }
            }
        }
    }

    void WaitOnAllUpdates(unsigned int numberOfObjects, uint32_t sec = 0)
    {
        WaitOnObjects(numberOfObjects, sec, updatedObjectNames);
    }

    void WaitOnRemove(unsigned int numberOfObjects, uint32_t sec = 0)
    {
        WaitOnObjects(numberOfObjects, sec, removedObjectNames);
    }

    void WaitOnObjects(unsigned int numberOfObjects, uint32_t sec, std::vector<qcc::String>& objectNames)
    {
        while (true) {
            mutex.Lock();
            QCC_DbgPrintf(("%p Received %d sem releases", this, objectNames.size()));
            if (objectNames.size() == numberOfObjects) {
                mutex.Unlock();
                break;
            }
            mutex.Unlock();
            if (!sec) {
                semaphore.Wait();
            } else {
                if (ER_TIMEOUT == semaphore.TimedWait(sec * 1000)) {
                    break;
                }
            }
        }
    }

  private:
    Semaphore semaphore;
    datadriven::Mutex mutex;
    unsigned int numberOfObservers;
};

class TestObjectSignalListener :
    public datadriven::SignalListener<SimpleTestObjectProxy, SimpleTestObjectProxy::Test>{
  public:

    TestObjectSignalListener()
    {
    }

    ~TestObjectSignalListener()
    {
    }

    void OnSignal(const SimpleTestObjectProxy::Test& signal)
    {
        assert(qcc::String(DEFAULT_TEST_NAME).compare(signal.test) == 0);
        semaphore.Post();
    }

    void Wait(void)
    {
        semaphore.Wait();
    }

  private:
    Semaphore semaphore;
};

//-----------------------------------------------------------------------------------------//

class VarTestObject :
    public ProvidedObject, public VarTestObjectInterface {
  public:
    VarTestObject(shared_ptr<datadriven::ObjectAdvertiser> advertiser,
                  const qcc::String& name) :
        ProvidedObject(advertiser), VarTestObjectInterface(this)
    {
        this->name = name;
    }

    void setPropObjPath(qcc::String op)
    {
        this->prop_objpath = op;
    }

    void setPropSignature(qcc::String sig)
    {
        this->prop_signature = sig;
    }

    void setPropString(qcc::String s)
    {
        this->prop_string = s;
    }
};

class VarTestObjectListener :
    public Observer<VarTestObjectProxy>::Listener {
  public:
    std::map<qcc::String, std::shared_ptr<VarTestObjectProxy> > nameToObjects;
    std::vector<qcc::String> removedObjectNames;

  public:
    VarTestObjectListener()
    {
    }

    ~VarTestObjectListener()
    {
    }

    virtual void OnUpdate(const std::shared_ptr<VarTestObjectProxy>& p)
    {
        VarTestObjectProxy::Properties prop = p->GetProperties();
        QCC_DbgPrintf(("RECEIVED VarTestObject: %s", prop.name.c_str()));
        nameToObjects[prop.name] = p;

        QCC_DbgPrintf(("RECEIVED VarTestObject prop_string: %s", prop.prop_string.c_str()));
        QCC_DbgPrintf(("RECEIVED VarTestObject prop_objpath: %s", prop.prop_objpath.c_str()));
        QCC_DbgPrintf(("RECEIVED VarTestObject prop_signature: %s", prop.prop_signature.c_str()));
        semaphore.Post();
    }

    virtual void OnRemove(const std::shared_ptr<VarTestObjectProxy>& p)
    {
        VarTestObjectProxy::Properties prop = p->GetProperties();
        removedObjectNames.push_back(prop.name);
        QCC_DbgPrintf(("REMOVED VarTestObject: %s", prop.name.c_str()));
        semaphore.Post();
    }

    void Wait()
    {
        semaphore.Wait();
    }

    void WaitFor(uint32_t sec)
    {
        semaphore.TimedWait(sec * 1000);
    }

  private:
    Semaphore semaphore;
};
}

#endif /* COMMON_H_ */
