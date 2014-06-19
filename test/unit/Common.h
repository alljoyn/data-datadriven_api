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
#include <SimpleTestObjectInterface.h>
#include <SimpleTestObjectProxy.h>
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
    public ProvidedObject, public SimpleTestObjectInterface {
  public:
    TestObject(BusConnection& conn,
               const qcc::String& name) :
        ProvidedObject(conn), SimpleTestObjectInterface(this)
    {
        this->name = name;
    }

    void Execute(ExecuteReply& reply)
    {
        reply.Send(true);
    }
};

class TestObjectListener :
    public Observer<SimpleTestObjectProxy>::Listener {
  public:
    std::map<qcc::String, std::shared_ptr<SimpleTestObjectProxy> > nameToObjects;
    std::vector<qcc::String> removedObjectNames;

  public:
    virtual void OnUpdate(const std::shared_ptr<SimpleTestObjectProxy>& p)
    {
        SimpleTestObjectProxy::Properties prop = p->GetProperties();
        QCC_DbgPrintf(("RECEIVED OBJ: %s", prop.name.c_str()));
        nameToObjects[prop.name] = p;
    }

    virtual void OnRemove(const std::shared_ptr<SimpleTestObjectProxy>& p)
    {
        SimpleTestObjectProxy::Properties prop = p->GetProperties();
        removedObjectNames.push_back(prop.name);
        QCC_DbgPrintf(("REMOVED OBJ: %s", prop.name.c_str()));
    }
};

class TestObjectSignalListener :
    public datadriven::SignalListener<SimpleTestObjectProxy, SimpleTestObjectProxy::Test>{
    void OnSignal(const SimpleTestObjectProxy::Test& signal)
    {
        assert(qcc::String(DEFAULT_TEST_NAME).compare(signal.test) == 0);
    }
};

//-----------------------------------------------------------------------------------------//

class VarTestObject :
    public ProvidedObject, public VarTestObjectInterface {
  public:
    VarTestObject(BusConnection& conn,
                  const qcc::String& name) :
        ProvidedObject(conn), VarTestObjectInterface(this)
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
    virtual void OnUpdate(const std::shared_ptr<VarTestObjectProxy>& p)
    {
        VarTestObjectProxy::Properties prop = p->GetProperties();
        QCC_DbgPrintf(("RECEIVED VarTestObject: %s", prop.name.c_str()));
        nameToObjects[prop.name] = p;

        QCC_DbgPrintf(("RECEIVED VarTestObject prop_string: %s", prop.prop_string.c_str()));
        QCC_DbgPrintf(("RECEIVED VarTestObject prop_objpath: %s", prop.prop_objpath.c_str()));
        QCC_DbgPrintf(("RECEIVED VarTestObject prop_signature: %s", prop.prop_signature.c_str()));
    }

    virtual void OnRemove(const std::shared_ptr<VarTestObjectProxy>& p)
    {
        VarTestObjectProxy::Properties prop = p->GetProperties();
        removedObjectNames.push_back(prop.name);
        QCC_DbgPrintf(("REMOVED VarTestObject: %s", prop.name.c_str()));
    }
};
}

#endif /* COMMON_H_ */
