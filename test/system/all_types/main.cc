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

#include <iostream>
#include <semaphore.h>

#include <datadriven/datadriven.h>

// provider
#include "AllTypes.h"
// consumer
#include "AllTypesProxy.h"
#include "AllArraysProxy.h"
#include "AllDictionariesProxy.h"

#include "init_data.h"
#include "dump.h"
#include "validate.h"

using namespace std;
using namespace gen::org_allseenalliance_test;

/**
 * Supported types tests.
 */
namespace test_system_alltypes {
// number of elements in arrays/vectors
#define NUM_ELEMENTS 2
static size_t _num_elements = NUM_ELEMENTS;

/***[ provider code ]**********************************************************/

static int be_provider(void)
{
    int rc = 0;
    datadriven::BusConnection busConnection;
    assert(ER_OK == busConnection.GetStatus());
    AllTypes at(busConnection, NUM_ELEMENTS);

    /* signal existence of object */
    cout << "Provider announcing object" << endl;
    assert(ER_OK == at.UpdateAll());
    assert(ER_OK == at.AllTypesInterface::GetStatus());
    assert(ER_OK == at.AllArraysInterface::GetStatus());
    assert(ER_OK == at.AllDictionariesInterface::GetStatus());
    cout << "Provider sleeping" << endl;
    while (true) {
        sleep(60);
    }
    return rc;
}

/***[ consumer code ]**********************************************************/

static sem_t _sem;

class AllTypesListener :
    public datadriven::Observer<AllTypesProxy>::Listener,
    public datadriven::Observer<AllArraysProxy>::Listener,
    public datadriven::Observer<AllDictionariesProxy>::Listener {
  public:
    /**
     * \test Reception of an update of properties with all types.
     *       -# validate object properties
     */
    void OnUpdate(const std::shared_ptr<AllTypesProxy>& atp)
    {
        cout << "Consumer received object update for " << atp->GetObjectId() << endl;
        dump("Consumer OnUpdate properties ", atp->GetProperties());
        validate(atp->GetProperties(), _num_elements);
        sem_post(&_sem);
        cout << "Consumer validated object update" << endl;
    }

    void OnRemove(const std::shared_ptr<AllTypesProxy>& atp)
    {
        cout << "Consumer received object remove for " << atp->GetObjectId() << endl;
        dump("Consumer OnRemove properties ", atp->GetProperties());
        validate(atp->GetProperties(), _num_elements);
        cout << "Consumer validated object remove" << endl;
    }

    /**
     * \test Reception of an update of array properties with all types.
     *       -# validate object properties
     */
    void OnUpdate(const std::shared_ptr<AllArraysProxy>& aap)
    {
        cout << "Consumer received object update for " << aap->GetObjectId() << endl;
        dump("Consumer OnUpdate properties ", aap->GetProperties());
        validate(aap->GetProperties(), _num_elements);
        sem_post(&_sem);
        cout << "Consumer validated object update" << endl;
    }

    void OnRemove(const std::shared_ptr<AllArraysProxy>& aap)
    {
        cout << "Consumer received object remove for " << aap->GetObjectId() << endl;
        dump("Consumer OnRemove properties ", aap->GetProperties());
        validate(aap->GetProperties(), _num_elements);
        cout << "Consumer validated object remove" << endl;
    }

    /**
     * \test Reception of an update of dictionary properties with all types of
     *       keys and values.
     *       -# validate object properties
     */
    void OnUpdate(const std::shared_ptr<AllDictionariesProxy>& adp)
    {
        cout << "Consumer received object update for " << adp->GetObjectId() << endl;
        dump("Consumer OnUpdate properties ", adp->GetProperties());
        validate(adp->GetProperties(), _num_elements);
        sem_post(&_sem);
        cout << "Consumer validated object update" << endl;
    }

    void OnRemove(const std::shared_ptr<AllDictionariesProxy>& adp)
    {
        cout << "Consumer received object remove for " << adp->GetObjectId() << endl;
        dump("Consumer OnRemove properties ", adp->GetProperties());
        validate(adp->GetProperties(), _num_elements);
        cout << "Consumer validated object remove" << endl;
    }
};

/**
 * \test Reception of a signal with all types.
 *       -# validate object properties
 *       -# validate signal arguments
 */
class AllTypesSignalListener :
    public datadriven::SignalListener<AllTypesProxy, AllTypesProxy::SignalWithAllTypes> {
    void OnSignal(const AllTypesProxy::SignalWithAllTypes& signal)
    {
        const std::shared_ptr<AllTypesProxy> atp = signal.GetEmitter();

        cout << "Consumer received signal from " << atp->GetObjectId() << endl;
        dump("Consumer OnAllTypesSignal properties ", atp->GetProperties());
        validate(atp->GetProperties(), _num_elements);
        dump("Consumer OnAllTypesSignal signal ", signal);
        validate(signal, _num_elements);
        sem_post(&_sem);
        cout << "Consumer validated signal" << endl;
    }
};

/**
 * \test Reception of a signal with arrays of all types.
 *       -# validate object properties
 *       -# validate signal arguments
 */
class AllArraysSignalListener :
    public datadriven::SignalListener<AllArraysProxy, AllArraysProxy::SignalWithAllArrays> {
    void OnSignal(const AllArraysProxy::SignalWithAllArrays& signal)
    {
        const std::shared_ptr<AllArraysProxy> aap = signal.GetEmitter();

        cout << "Consumer received signal from " << aap->GetObjectId() << endl;
        dump("Consumer OnAllArraysSignal properties ", aap->GetProperties());
        validate(aap->GetProperties(), _num_elements);
        dump("Consumer OnAllArraysSignal signal ", signal);
        validate(signal, _num_elements);
        sem_post(&_sem);
        cout << "Consumer validated signal" << endl;
    }
};

/**
 * \test Reception of a signal with dictionaries with all types of keys and
 *       values:
 *       -# validate object properties
 *       -# validate signal arguments
 */
class AllDictionariesSignalListener :
    public datadriven::SignalListener<AllDictionariesProxy, AllDictionariesProxy::SignalWithAllDictionaries> {
    void OnSignal(const AllDictionariesProxy::SignalWithAllDictionaries& signal)
    {
        const std::shared_ptr<AllDictionariesProxy> atp = signal.GetEmitter();

        cout << "Consumer received signal from " << atp->GetObjectId() << endl;
        dump("Consumer OnAllDictionariesSignal properties ", atp->GetProperties());
        validate(atp->GetProperties(), _num_elements);
        dump("Consumer OnAllDictionariesSignal signal ", signal);
        validate(signal, _num_elements);
        sem_post(&_sem);
        cout << "Consumer validated signal" << endl;
    }
};

/**
 * \test Reception of a signal without arguments.
 *       -# validate object properties
 */
class NoArgsSignalListener :
    public datadriven::SignalListener<AllTypesProxy, AllTypesProxy::SignalWithoutArgs> {
    void OnSignal(const AllTypesProxy::SignalWithoutArgs& signal)
    {
        const std::shared_ptr<AllTypesProxy> atp = signal.GetEmitter();

        cout << "Consumer received no-args signal from " << atp->GetObjectId() << endl;
        dump("Consumer OnNoArgsSignal properties ", atp->GetProperties());
        validate(atp->GetProperties(), _num_elements);
        sem_post(&_sem);
        cout << "Consumer validated signal" << endl;
    }
};

/**
 * \test All types method call:
 *       -# call method
 *       -# wait for reply
 *       -# validate reply status
 *       -# validate output arguments
 */
static void call_method_all_types(const shared_ptr<AllTypesProxy>& proxy)
{
    AllTypesTypeDescription::StructWithAllTypes swat;
    long long cnt = 12;

    init_data(swat, _num_elements, cnt);
    cout << "Consumer calling all types method on " << proxy->GetObjectId() << endl;
    std::shared_ptr<datadriven::MethodInvocation<AllTypesProxy::MethodWithAllTypesInAndOutReply> > inv =
        proxy->MethodWithAllTypesInAndOut(true, 2, 3, 4, 5, 6, 7, 8, 9, "10", "iiiiiiiiiii", "/path/12", swat);
    cout << "Consumer waiting for reply" << endl;
    const AllTypesProxy::MethodWithAllTypesInAndOutReply& reply = inv->GetReply();
    assert(datadriven::MethodInvocation<AllTypesProxy::MethodWithAllTypesInAndOutReply>::READY == inv->GetState());
    assert(ER_OK == reply.GetStatus());
    dump("Consumer AllTypesMethod reply ", reply);
    validate(reply, _num_elements);
    cout << "Consumer validated reply" << endl;
    cout << "Consumer waiting for signal" << endl;
    sem_wait(&_sem);
    cout << "Consumer done waiting for signal" << endl;
}

/**
 * \test Arrays of all types method call:
 *       -# call method
 *       -# wait for reply
 *       -# validate reply status
 *       -# validate output arguments
 */
static void call_method_all_arrays(const shared_ptr<AllArraysProxy>& proxy)
{
    long long cnt = 0;
    std::vector<bool> array_of_boolean;
    std::vector<uint8_t> array_of_byte;
    std::vector<int16_t> array_of_int16;
    std::vector<uint16_t> array_of_uint16;
    std::vector<int32_t> array_of_int32;
    std::vector<uint32_t> array_of_uint32;
    std::vector<int64_t> array_of_int64;
    std::vector<uint64_t> array_of_uint64;
    std::vector<double> array_of_double;
    std::vector<qcc::String> array_of_string;
    std::vector<datadriven::Signature> array_of_signature;
    std::vector<datadriven::ObjectPath> array_of_objpath;
    std::vector<AllArraysTypeDescription::StructNested> array_of_struct;
    std::vector<std::vector<int32_t> > array_of_array;
    std::vector<AllArraysTypeDescription::DictionaryOfInt32> array_of_dict;

    init_data(array_of_boolean, _num_elements, cnt);
    init_data(array_of_byte, _num_elements, cnt);
    init_data(array_of_int16, _num_elements, cnt);
    init_data(array_of_uint16, _num_elements, cnt);
    init_data(array_of_int32, _num_elements, cnt);
    init_data(array_of_uint32, _num_elements, cnt);
    init_data(array_of_int64, _num_elements, cnt);
    init_data(array_of_uint64, _num_elements, cnt);
    init_data(array_of_double, _num_elements, cnt);
    init_data(array_of_string, _num_elements, cnt);
    init_data(array_of_signature, _num_elements, cnt);
    init_data(array_of_objpath, _num_elements, cnt);
    init_data(array_of_struct, _num_elements, cnt);
    init_data(array_of_array, _num_elements, cnt);
    init_data(array_of_dict, _num_elements, cnt);
    cout << "Consumer calling all arrays method on " << proxy->GetObjectId() << endl;
    std::shared_ptr<datadriven::MethodInvocation<AllArraysProxy::MethodWithAllArraysInAndOutReply> > inv =
        proxy->MethodWithAllArraysInAndOut(array_of_boolean,
                                           array_of_byte,
                                           array_of_int16,
                                           array_of_uint16,
                                           array_of_int32,
                                           array_of_uint32,
                                           array_of_int64,
                                           array_of_uint64,
                                           array_of_double,
                                           array_of_string,
                                           array_of_signature,
                                           array_of_objpath,
                                           array_of_struct,
                                           array_of_array,
                                           array_of_dict);
    cout << "Consumer waiting for reply" << endl;
    const AllArraysProxy::MethodWithAllArraysInAndOutReply& reply = inv->GetReply();
    assert(datadriven::MethodInvocation<AllArraysProxy::MethodWithAllArraysInAndOutReply>::READY == inv->GetState());
    assert(ER_OK == reply.GetStatus());
    dump("Consumer AllArraysMethod reply ", reply);
    cnt = 0;
    validate(reply, _num_elements, cnt);
    cout << "Consumer validated reply" << endl;
    cout << "Consumer waiting for signal" << endl;
    sem_wait(&_sem);
    cout << "Consumer done waiting for signal" << endl;
}

/**
 * \test Dictionaries with all types of keys and values method call:
 *       -# call method
 *       -# wait for reply
 *       -# validate reply status
 *       -# validate output arguments
 */
static void call_method_all_dictionaries(const shared_ptr<AllDictionariesProxy>& proxy)
{
    long long cnt = 0;
    AllDictionariesTypeDescription::DictionaryOfBoolean dict_of_boolean;
    AllDictionariesTypeDescription::DictionaryOfByte dict_of_byte;
    AllDictionariesTypeDescription::DictionaryOfInt16 dict_of_int16;
    AllDictionariesTypeDescription::DictionaryOfUnsignedInt16 dict_of_uint16;
    AllDictionariesTypeDescription::DictionaryOfInt32 dict_of_int32;
    AllDictionariesTypeDescription::DictionaryOfUnsignedInt32 dict_of_uint32;
    AllDictionariesTypeDescription::DictionaryOfInt64 dict_of_int64;
    AllDictionariesTypeDescription::DictionaryOfUnsignedInt64 dict_of_uint64;
    AllDictionariesTypeDescription::DictionaryOfDouble dict_of_double;
    AllDictionariesTypeDescription::DictionaryOfString dict_of_string;
    AllDictionariesTypeDescription::DictionaryOfSignature dict_of_signature;
    AllDictionariesTypeDescription::DictionaryOfObjPath dict_of_objpath;
    AllDictionariesTypeDescription::DictionaryOfStruct dict_of_struct;
    AllDictionariesTypeDescription::DictionaryOfArray dict_of_array;
    AllDictionariesTypeDescription::DictionaryOfDictionary dict_of_dict;

    init_data(dict_of_boolean, _num_elements, cnt);
    init_data(dict_of_byte, _num_elements, cnt);
    init_data(dict_of_int16, _num_elements, cnt);
    init_data(dict_of_uint16, _num_elements, cnt);
    init_data(dict_of_int32, _num_elements, cnt);
    init_data(dict_of_uint32, _num_elements, cnt);
    init_data(dict_of_int64, _num_elements, cnt);
    init_data(dict_of_uint64, _num_elements, cnt);
    init_data(dict_of_double, _num_elements, cnt);
    init_data(dict_of_string, _num_elements, cnt);
    init_data(dict_of_signature, _num_elements, cnt);
    init_data(dict_of_objpath, _num_elements, cnt);
    init_data(dict_of_struct, _num_elements, cnt);
    init_data(dict_of_array, _num_elements, cnt);
    init_data(dict_of_dict, _num_elements, cnt);
    cout << "Consumer calling all dictionaries method on " << proxy->GetObjectId() << endl;
    std::shared_ptr<datadriven::MethodInvocation<AllDictionariesProxy::MethodWithAllDictionariesInAndOutReply> > inv =
        proxy->MethodWithAllDictionariesInAndOut(dict_of_boolean,
                                                 dict_of_byte,
                                                 dict_of_int16,
                                                 dict_of_uint16,
                                                 dict_of_int32,
                                                 dict_of_uint32,
                                                 dict_of_int64,
                                                 dict_of_uint64,
                                                 dict_of_double,
                                                 dict_of_string,
                                                 dict_of_signature,
                                                 dict_of_objpath,
                                                 dict_of_struct,
                                                 dict_of_array,
                                                 dict_of_dict);
    cout << "Consumer waiting for reply" << endl;
    const AllDictionariesProxy::MethodWithAllDictionariesInAndOutReply& reply = inv->GetReply();
    assert(
        datadriven::MethodInvocation<AllDictionariesProxy::MethodWithAllDictionariesInAndOutReply>::READY ==
        inv->GetState());
    assert(ER_OK == reply.GetStatus());
    dump("Consumer AllDictionariesMethod reply ", reply);
    cnt = 0;
    validate(reply, _num_elements, cnt);
    cout << "Consumer validated reply" << endl;
    cout << "Consumer waiting for signal" << endl;
    sem_wait(&_sem);
    cout << "Consumer done waiting for signal" << endl;
}

/**
 * \test No output argument method call:
 *       -# call method
 *       -# wait for reply
 *       -# validate reply status
 */
static void call_method_no_out(const shared_ptr<AllTypesProxy>& proxy)
{
    cout << "Consumer calling no out args method on " << proxy->GetObjectId() << endl;
    std::shared_ptr<datadriven::MethodInvocation<AllTypesProxy::MethodWithoutOutReply> > inv = proxy->MethodWithoutOut(
        true);
    cout << "Consumer waiting for no out args reply" << endl;
#ifndef NDEBUG
    const AllTypesProxy::MethodWithoutOutReply& reply = inv->GetReply();
#endif
    assert(datadriven::MethodInvocation<AllTypesProxy::MethodWithoutOutReply>::READY == inv->GetState());
    assert(ER_OK == reply.GetStatus());
    cout << "Consumer validated no out args reply" << endl;
    cout << "Consumer waiting for signal" << endl;
    sem_wait(&_sem);
    cout << "Consumer done waiting for signal" << endl;
}

/**
 * \test No input arguments method call:
 *       -# call method
 *       -# wait for reply
 *       -# validate reply status
 *       -# validate output arguments
 */
static void call_method_no_in(const shared_ptr<AllTypesProxy>& proxy)
{
    cout << "Consumer calling no in args method on " << proxy->GetObjectId() << endl;
    std::shared_ptr<datadriven::MethodInvocation<AllTypesProxy::MethodWithoutInReply> > inv = proxy->MethodWithoutIn();
    cout << "Consumer waiting for no in args reply" << endl;
#ifndef NDEBUG
    const AllTypesProxy::MethodWithoutInReply& reply = inv->GetReply();
#endif
    assert(datadriven::MethodInvocation<AllTypesProxy::MethodWithoutInReply>::READY == inv->GetState());
    assert(ER_OK == reply.GetStatus());
    assert(true == reply.arg_out_boolean);
    cout << "Consumer validated no in args reply" << endl;
}

static int be_consumer(void)
{
    int rc = 0;
    long long cnt = 0;
    datadriven::BusConnection busConnection;
    assert(ER_OK == busConnection.GetStatus());

    AllTypesListener atl = AllTypesListener();
    datadriven::Observer<AllTypesProxy> observer_at(busConnection, &atl);
    datadriven::Observer<AllArraysProxy> observer_aa(busConnection, &atl);
    datadriven::Observer<AllDictionariesProxy> observer_ad(busConnection, &atl);

    AllTypesSignalListener atsl = AllTypesSignalListener();
    AllArraysSignalListener aasl = AllArraysSignalListener();
    AllDictionariesSignalListener adsl = AllDictionariesSignalListener();
    NoArgsSignalListener nasl = NoArgsSignalListener();

    cout << "Consumer registering listeners" << endl;

    observer_at.AddSignalListener(atsl);
    observer_at.AddSignalListener(nasl);
    observer_aa.AddSignalListener(aasl);
    observer_ad.AddSignalListener(adsl);
    // wait for object update (3 interfaces)
    cout << "Consumer waiting for object" << endl;
    sem_wait(&_sem);
    sem_wait(&_sem);
    sem_wait(&_sem);
    // iterate objects
    cout << "Consumer iterating objects/interfaces" << endl;
    for (datadriven::Observer<AllTypesProxy>::iterator it = observer_at.begin();
         it != observer_at.end();
         ++it) {
        assert(ER_OK == it->GetStatus());
        cout << "Consumer in iterator for " << it->GetObjectId() << endl;
        validate(it->GetProperties(), _num_elements);
        call_method_all_types(*it);
        call_method_no_out(*it);
        call_method_no_in(*it);
        cnt++;
    }
    for (datadriven::Observer<AllArraysProxy>::iterator it = observer_aa.begin();
         it != observer_aa.end();
         ++it) {
        assert(ER_OK == it->GetStatus());
        cout << "Consumer in iterator for " << it->GetObjectId() << endl;
        validate(it->GetProperties(), _num_elements);
        call_method_all_arrays(*it);
        cnt++;
    }
    for (datadriven::Observer<AllDictionariesProxy>::iterator it = observer_ad.begin();
         it != observer_ad.end();
         ++it) {
        assert(ER_OK == it->GetStatus());
        cout << "Consumer in iterator for " << it->GetObjectId() << endl;
        validate(it->GetProperties(), _num_elements);
        call_method_all_dictionaries(*it);
        cnt++;
    }
    assert(3 == cnt);
    cout << "Consumer done" << endl;
    return rc;
}
};

/***[ main code ]**************************************************************/

using namespace test_system_alltypes;

int main(int argc, char** argv)
{
    bool consumer = true;
    int rc = 0;

    // only play provider if first command-line argument starts with a 'p'
    if (argc <= 1) {
        cout << "Usage: " << argv[0] << " <consumer|provider> [array-size]" << endl;
        return 1;
    }
    if ('p' == *argv[1]) {
        consumer = false;
    }
    if (argc > 2) {
        _num_elements = atoi(argv[2]);
    }
    cout << "Using arrays/dictionaries of size " << _num_elements << endl;
    sem_init(&_sem, 0, 0);
    rc = consumer ? be_consumer() : be_provider();
    sem_destroy(&_sem);
    return rc;
}
