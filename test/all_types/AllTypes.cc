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

#include <iostream>

#include <datadriven/Semaphore.h>

#include "AllTypes.h"
#include "init_data.h"

namespace test_system_alltypes {
using namespace std;
using namespace::gen::org_allseenalliance_test;

static datadriven::Semaphore _sem;

AllTypes::AllTypes(shared_ptr<datadriven::ObjectAdvertiser> advertiser,
                   size_t nbelem) :
    datadriven::ProvidedObject(advertiser),
    AllTypesInterface(this),
    AllArraysInterface(this),
    AllDictionariesInterface(this),
    b(false)
{
    nelem = nbelem;

    // initialize unnamed properties
    prop_unnamed_struct.member0 = false;

    // initial standard properties
    long long cnt = 0;
    init_data(prop_boolean, nelem, cnt);
    init_data(prop_byte, nelem, cnt);
    init_data(prop_int16, nelem, cnt);
    init_data(prop_uint16, nelem, cnt);
    init_data(prop_int32, nelem, cnt);
    init_data(prop_uint32, nelem, cnt);
    init_data(prop_int64, nelem, cnt);
    init_data(prop_uint64, nelem, cnt);
    init_data(prop_double, nelem, cnt);
    init_data(prop_string, nelem, cnt);
    init_data(prop_signature, nelem, cnt);
    init_data(prop_objpath, nelem, cnt);
    init_data(prop_variant, nelem, cnt);
    init_data(prop_struct, nelem, cnt);
    init_data(prop_array, nelem, cnt);
    init_data(prop_dict, nelem, cnt);
    cnt = 0; // inval properties
    init_data(prop_boolean_inval, nelem, cnt);
    init_data(prop_byte_inval, nelem, cnt);
    init_data(prop_int16_inval, nelem, cnt);
    init_data(prop_uint16_inval, nelem, cnt);
    init_data(prop_int32_inval, nelem, cnt);
    init_data(prop_uint32_inval, nelem, cnt);
    init_data(prop_int64_inval, nelem, cnt);
    init_data(prop_uint64_inval, nelem, cnt);
    init_data(prop_double_inval, nelem, cnt);
    init_data(prop_string_inval, nelem, cnt);
    init_data(prop_signature_inval, nelem, cnt);
    init_data(prop_objpath_inval, nelem, cnt);
    init_data(prop_variant_inval, nelem, cnt);
    init_data(prop_struct_inval, nelem, cnt);
    init_data(prop_array_inval, nelem, cnt);
    init_data(prop_dict_inval, nelem, cnt);

    cnt = 0; // new interface, start counting from zero again
    init_data(prop_array_of_boolean, nelem, cnt);
    init_data(prop_array_of_byte, nelem, cnt);
    init_data(prop_array_of_int16, nelem, cnt);
    init_data(prop_array_of_uint16, nelem, cnt);
    init_data(prop_array_of_int32, nelem, cnt);
    init_data(prop_array_of_uint32, nelem, cnt);
    init_data(prop_array_of_int64, nelem, cnt);
    init_data(prop_array_of_uint64, nelem, cnt);
    init_data(prop_array_of_double, nelem, cnt);
    init_data(prop_array_of_string, nelem, cnt);
    init_data(prop_array_of_signature, nelem, cnt);
    init_data(prop_array_of_objpath, nelem, cnt);
    init_data(prop_array_of_struct, nelem, cnt);
    init_data(prop_array_of_array, nelem, cnt);
    init_data(prop_array_of_dict, nelem, cnt);
    cnt = 0; // Array inval properties
    init_data(prop_array_of_boolean_inval, nelem, cnt);
    init_data(prop_array_of_byte_inval, nelem, cnt);
    init_data(prop_array_of_int16_inval, nelem, cnt);
    init_data(prop_array_of_uint16_inval, nelem, cnt);
    init_data(prop_array_of_int32_inval, nelem, cnt);
    init_data(prop_array_of_uint32_inval, nelem, cnt);
    init_data(prop_array_of_int64_inval, nelem, cnt);
    init_data(prop_array_of_uint64_inval, nelem, cnt);
    init_data(prop_array_of_double_inval, nelem, cnt);
    init_data(prop_array_of_string_inval, nelem, cnt);
    init_data(prop_array_of_signature_inval, nelem, cnt);
    init_data(prop_array_of_objpath_inval, nelem, cnt);
    init_data(prop_array_of_struct_inval, nelem, cnt);
    init_data(prop_array_of_array_inval, nelem, cnt);
    init_data(prop_array_of_dict_inval, nelem, cnt);

    cnt = 0; // new interface, start counting from zero again
    init_data(prop_dict_of_boolean, nelem, cnt);
    init_data(prop_dict_of_byte, nelem, cnt);
    init_data(prop_dict_of_int16, nelem, cnt);
    init_data(prop_dict_of_uint16, nelem, cnt);
    init_data(prop_dict_of_int32, nelem, cnt);
    init_data(prop_dict_of_uint32, nelem, cnt);
    init_data(prop_dict_of_int64, nelem, cnt);
    init_data(prop_dict_of_uint64, nelem, cnt);
    init_data(prop_dict_of_double, nelem, cnt);
    init_data(prop_dict_of_string, nelem, cnt);
    init_data(prop_dict_of_signature, nelem, cnt);
    init_data(prop_dict_of_objpath, nelem, cnt);
    init_data(prop_dict_of_struct, nelem, cnt);
    init_data(prop_dict_of_array, nelem, cnt);
    init_data(prop_dict_of_dict, nelem, cnt);
    cnt = 0; // Dictionary inval properties
    init_data(prop_dict_of_boolean_inval, nelem, cnt);
    init_data(prop_dict_of_byte_inval, nelem, cnt);
    init_data(prop_dict_of_int16_inval, nelem, cnt);
    init_data(prop_dict_of_uint16_inval, nelem, cnt);
    init_data(prop_dict_of_int32_inval, nelem, cnt);
    init_data(prop_dict_of_uint32_inval, nelem, cnt);
    init_data(prop_dict_of_int64_inval, nelem, cnt);
    init_data(prop_dict_of_uint64_inval, nelem, cnt);
    init_data(prop_dict_of_double_inval, nelem, cnt);
    init_data(prop_dict_of_string_inval, nelem, cnt);
    init_data(prop_dict_of_signature_inval, nelem, cnt);
    init_data(prop_dict_of_objpath_inval, nelem, cnt);
    init_data(prop_dict_of_struct_inval, nelem, cnt);
    init_data(prop_dict_of_array_inval, nelem, cnt);
    init_data(prop_dict_of_dict_inval, nelem, cnt);
}

AllTypes::~AllTypes()
{
}

void AllTypes::InvalidateAll(int64_t new_cnt, std::shared_ptr<InvalidateAllReply> _reply)
{
    /* Reset all the values and start from TEST_CNT_VAL
     * This is to test if the other side does pick up the invalidated properties */
    long long cnt = new_cnt;

    init_data(prop_boolean_inval, nelem, cnt);
    init_data(prop_byte_inval, nelem, cnt);
    init_data(prop_int16_inval, nelem, cnt);
    init_data(prop_uint16_inval, nelem, cnt);
    init_data(prop_int32_inval, nelem, cnt);
    init_data(prop_uint32_inval, nelem, cnt);
    init_data(prop_int64_inval, nelem, cnt);
    init_data(prop_uint64_inval, nelem, cnt);
    init_data(prop_double_inval, nelem, cnt);
    init_data(prop_string_inval, nelem, cnt);
    init_data(prop_signature_inval, nelem, cnt);
    init_data(prop_objpath_inval, nelem, cnt);
    init_data(prop_variant_inval, nelem, cnt);
    init_data(prop_struct_inval, nelem, cnt);
    init_data(prop_array_inval, nelem, cnt);
    init_data(prop_dict_inval, nelem, cnt);

    QStatus status;
    if (ER_OK != (status = Invalidateprop_boolean_inval()) ||
        ER_OK != (status = Invalidateprop_byte_inval()) ||
        ER_OK != (status = Invalidateprop_int16_inval()) ||
        ER_OK != (status = Invalidateprop_uint16_inval()) ||
        ER_OK != (status = Invalidateprop_int32_inval()) ||
        ER_OK != (status = Invalidateprop_uint32_inval()) ||
        ER_OK != (status = Invalidateprop_int64_inval()) ||
        ER_OK != (status = Invalidateprop_uint64_inval()) ||
        ER_OK != (status = Invalidateprop_double_inval()) ||
        ER_OK != (status = Invalidateprop_string_inval()) ||
        ER_OK != (status = Invalidateprop_signature_inval()) ||
        ER_OK != (status = Invalidateprop_objpath_inval()) ||
        ER_OK != (status = Invalidateprop_variant_inval()) ||
        ER_OK != (status = Invalidateprop_struct_inval()) ||
        ER_OK != (status = Invalidateprop_array_inval()) ||
        ER_OK != (status = Invalidateprop_dict_inval())) {
        cout << "Could not invalidate some properties" << endl;

        _reply->SendErrorCode(ER_FAIL);
        assert(ER_OK == _sem.Post());
        return;
    }
    cout << "All properties have been invalidated" << endl;

    // Array inval properties
    cnt = new_cnt;

    init_data(prop_array_of_boolean_inval, nelem, cnt);
    init_data(prop_array_of_byte_inval, nelem, cnt);
    init_data(prop_array_of_int16_inval, nelem, cnt);
    init_data(prop_array_of_uint16_inval, nelem, cnt);
    init_data(prop_array_of_int32_inval, nelem, cnt);
    init_data(prop_array_of_uint32_inval, nelem, cnt);
    init_data(prop_array_of_int64_inval, nelem, cnt);
    init_data(prop_array_of_uint64_inval, nelem, cnt);
    init_data(prop_array_of_double_inval, nelem, cnt);
    init_data(prop_array_of_string_inval, nelem, cnt);
    init_data(prop_array_of_signature_inval, nelem, cnt);
    init_data(prop_array_of_objpath_inval, nelem, cnt);
    init_data(prop_array_of_struct_inval, nelem, cnt);
    init_data(prop_array_of_array_inval, nelem, cnt);
    init_data(prop_array_of_dict_inval, nelem, cnt);

    if (ER_OK != (status = Invalidateprop_array_of_boolean_inval()) ||
        ER_OK != (status = Invalidateprop_array_of_byte_inval()) ||
        ER_OK != (status = Invalidateprop_array_of_int16_inval()) ||
        ER_OK != (status = Invalidateprop_array_of_uint16_inval()) ||
        ER_OK != (status = Invalidateprop_array_of_int32_inval()) ||
        ER_OK != (status = Invalidateprop_array_of_uint32_inval()) ||
        ER_OK != (status = Invalidateprop_array_of_int64_inval()) ||
        ER_OK != (status = Invalidateprop_array_of_uint64_inval()) ||
        ER_OK != (status = Invalidateprop_array_of_double_inval()) ||
        ER_OK != (status = Invalidateprop_array_of_string_inval()) ||
        ER_OK != (status = Invalidateprop_array_of_signature_inval()) ||
        ER_OK != (status = Invalidateprop_array_of_objpath_inval()) ||
        ER_OK != (status = Invalidateprop_array_of_struct_inval()) ||
        ER_OK != (status = Invalidateprop_array_of_array_inval()) ||
        ER_OK != (status = Invalidateprop_array_of_dict_inval())) {
        cout << "Could not invalidate some array properties" << endl;

        _reply->SendErrorCode(ER_FAIL);
        assert(ER_OK == _sem.Post());
        return;
    }
    cout << "All array properties have been invalidated" << endl;

    // Dictionary inval properties
    cnt = new_cnt;

    init_data(prop_dict_of_boolean_inval, nelem, cnt);
    init_data(prop_dict_of_byte_inval, nelem, cnt);
    init_data(prop_dict_of_int16_inval, nelem, cnt);
    init_data(prop_dict_of_uint16_inval, nelem, cnt);
    init_data(prop_dict_of_int32_inval, nelem, cnt);
    init_data(prop_dict_of_uint32_inval, nelem, cnt);
    init_data(prop_dict_of_int64_inval, nelem, cnt);
    init_data(prop_dict_of_uint64_inval, nelem, cnt);
    init_data(prop_dict_of_double_inval, nelem, cnt);
    init_data(prop_dict_of_string_inval, nelem, cnt);
    init_data(prop_dict_of_signature_inval, nelem, cnt);
    init_data(prop_dict_of_objpath_inval, nelem, cnt);
    init_data(prop_dict_of_struct_inval, nelem, cnt);
    init_data(prop_dict_of_array_inval, nelem, cnt);
    init_data(prop_dict_of_dict_inval, nelem, cnt);

    if (ER_OK != (status = Invalidateprop_dict_of_boolean_inval()) ||
        ER_OK != (status = Invalidateprop_dict_of_byte_inval()) ||
        ER_OK != (status = Invalidateprop_dict_of_int16_inval()) ||
        ER_OK != (status = Invalidateprop_dict_of_uint16_inval()) ||
        ER_OK != (status = Invalidateprop_dict_of_int32_inval()) ||
        ER_OK != (status = Invalidateprop_dict_of_uint32_inval()) ||
        ER_OK != (status = Invalidateprop_dict_of_int64_inval()) ||
        ER_OK != (status = Invalidateprop_dict_of_uint64_inval()) ||
        ER_OK != (status = Invalidateprop_dict_of_double_inval()) ||
        ER_OK != (status = Invalidateprop_dict_of_string_inval()) ||
        ER_OK != (status = Invalidateprop_dict_of_signature_inval()) ||
        ER_OK != (status = Invalidateprop_dict_of_objpath_inval()) ||
        ER_OK != (status = Invalidateprop_dict_of_struct_inval()) ||
        ER_OK != (status = Invalidateprop_dict_of_array_inval()) ||
        ER_OK != (status = Invalidateprop_dict_of_dict_inval())) {
        cout << "Could not invalidate some dictionary properties" << endl;

        _reply->SendErrorCode(ER_FAIL);
        assert(ER_OK == _sem.Post());
        return;
    }

    _reply->Send();
    assert(ER_OK == _sem.Post());
    return;
}

void AllTypes::WaitForInvalidateAll()
{
    assert(ER_OK == _sem.Wait());
}

void AllTypes::MethodWithAllTypesInAndOut(bool arg_in_boolean,
                                          uint8_t arg_in_byte,
                                          int16_t arg_in_int16,
                                          uint16_t arg_in_uint16,
                                          int32_t arg_in_int32,
                                          uint32_t arg_in_uint32,
                                          int64_t arg_in_int64,
                                          uint64_t arg_in_uint64,
                                          double arg_in_double,
                                          const qcc::String& arg_in_string,
                                          datadriven::Signature arg_in_signature,
                                          datadriven::ObjectPath arg_in_objpath,
                                          ajn::MsgArg arg_in_variant,
                                          const AllTypesInterface::Type::StructWithAllTypes& arg_in_struct,
                                          std::shared_ptr<MethodWithAllTypesInAndOutReply> reply)
{
    cout << "Provider received method call for " << __FUNCTION__ << endl;
    cout << "Provider sending reply" << endl;
    reply->Send(arg_in_boolean,
                arg_in_byte,
                arg_in_int16,
                arg_in_uint16,
                arg_in_int32,
                arg_in_uint32,
                arg_in_int64,
                arg_in_uint64,
                arg_in_double,
                arg_in_string,
                arg_in_signature,
                arg_in_objpath,
                arg_in_variant,
                arg_in_struct);
    // also send signal with same arguments
    cout << "Provider sending signal" << endl;
    SignalWithAllTypes(arg_in_boolean,
                       arg_in_byte,
                       arg_in_int16,
                       arg_in_uint16,
                       arg_in_int32,
                       arg_in_uint32,
                       arg_in_int64,
                       arg_in_uint64,
                       arg_in_double,
                       arg_in_string,
                       arg_in_signature,
                       arg_in_objpath,
                       arg_in_variant,
                       arg_in_struct);
}

void AllTypes::MethodWithAllArraysInAndOut(const std::vector<bool>& arg_in_boolean,
                                           const std::vector<uint8_t>& arg_in_byte,
                                           const std::vector<int16_t>& arg_in_int16,
                                           const std::vector<uint16_t>& arg_in_uint16,
                                           const std::vector<int32_t>& arg_in_int32,
                                           const std::vector<uint32_t>& arg_in_uint32,
                                           const std::vector<int64_t>& arg_in_int64,
                                           const std::vector<uint64_t>& arg_in_uint64,
                                           const std::vector<double>& arg_in_double,
                                           const std::vector<qcc::String>& arg_in_string,
                                           const std::vector<datadriven::Signature>& arg_in_signature,
                                           const std::vector<datadriven::ObjectPath>& arg_in_objpath,
                                           const std::vector<AllArraysInterface::Type::StructNested>& arg_in_struct,
                                           const std::vector<std::vector<int32_t> >& arg_in_array,
                                           const std::vector<AllArraysInterface::Type::DictionaryOfInt32>& arg_in_dict,
                                           std::shared_ptr<MethodWithAllArraysInAndOutReply> reply)
{
    cout << "Provider received method call for " << __FUNCTION__ << endl;
    cout << "Provider sending reply" << endl;
    reply->Send(arg_in_boolean,
                arg_in_byte,
                arg_in_int16,
                arg_in_uint16,
                arg_in_int32,
                arg_in_uint32,
                arg_in_int64,
                arg_in_uint64,
                arg_in_double,
                arg_in_string,
                arg_in_signature,
                arg_in_objpath,
                arg_in_struct,
                arg_in_array,
                arg_in_dict);
    // also send signal with same arguments
    cout << "Provider sending signal" << endl;
    SignalWithAllArrays(arg_in_boolean,
                        arg_in_byte,
                        arg_in_int16,
                        arg_in_uint16,
                        arg_in_int32,
                        arg_in_uint32,
                        arg_in_int64,
                        arg_in_uint64,
                        arg_in_double,
                        arg_in_string,
                        arg_in_signature,
                        arg_in_objpath,
                        arg_in_struct,
                        arg_in_array,
                        arg_in_dict);
}

void AllTypes::MethodWithAllDictionariesInAndOut(
    const AllDictionariesInterface::Type::DictionaryOfBoolean& arg_in_boolean,
    const AllDictionariesInterface::Type::DictionaryOfByte& arg_in_byte,
    const AllDictionariesInterface::Type::DictionaryOfInt16& arg_in_int16,
    const AllDictionariesInterface::Type::DictionaryOfUnsignedInt16&
    arg_in_uint16,
    const AllDictionariesInterface::Type::DictionaryOfInt32& arg_in_int32,
    const AllDictionariesInterface::Type::DictionaryOfUnsignedInt32&
    arg_in_uint32,
    const AllDictionariesInterface::Type::DictionaryOfInt64& arg_in_int64,
    const AllDictionariesInterface::Type::DictionaryOfUnsignedInt64&
    arg_in_uint64,
    const AllDictionariesInterface::Type::DictionaryOfDouble&
    arg_in_double,
    const AllDictionariesInterface::Type::DictionaryOfString&
    arg_in_string,
    const AllDictionariesInterface::Type::DictionaryOfSignature&
    arg_in_signature,
    const AllDictionariesInterface::Type::DictionaryOfObjPath&
    arg_in_objpath,
    const AllDictionariesInterface::Type::DictionaryOfStruct&
    arg_in_struct,
    const AllDictionariesInterface::Type::DictionaryOfArray& arg_in_array,
    const AllDictionariesInterface::Type::DictionaryOfDictionary&
    arg_in_dict,
    std::shared_ptr<MethodWithAllDictionariesInAndOutReply> reply)
{
    cout << "Provider received method call for " << __FUNCTION__ << endl;
    cout << "Provider sending reply" << endl;
    reply->Send(arg_in_boolean,
                arg_in_byte,
                arg_in_int16,
                arg_in_uint16,
                arg_in_int32,
                arg_in_uint32,
                arg_in_int64,
                arg_in_uint64,
                arg_in_double,
                arg_in_string,
                arg_in_signature,
                arg_in_objpath,
                arg_in_struct,
                arg_in_array,
                arg_in_dict);
    // also send signal with same arguments
    cout << "Provider sending signal" << endl;
    SignalWithAllDictionaries(arg_in_boolean,
                              arg_in_byte,
                              arg_in_int16,
                              arg_in_uint16,
                              arg_in_int32,
                              arg_in_uint32,
                              arg_in_int64,
                              arg_in_uint64,
                              arg_in_double,
                              arg_in_string,
                              arg_in_signature,
                              arg_in_objpath,
                              arg_in_struct,
                              arg_in_array,
                              arg_in_dict);
}

void AllTypes::MethodWithoutOut(bool arg_in_boolean, std::shared_ptr<MethodWithoutOutReply> reply)
{
    cout << "Provider received method call for " << __FUNCTION__ << endl;
    this->b = arg_in_boolean;
    cout << "Provider sending reply" << endl;
    reply->Send();
    // also send signal without arguments
    cout << "Provider sending signal" << endl;
    SignalWithoutArgs();
    SignalMultipleInheritance();
}

void AllTypes::MethodWithoutIn(std::shared_ptr<MethodWithoutInReply> reply)
{
    cout << "Provider received method call for " << __FUNCTION__ << endl;
    cout << "Provider sending reply" << endl;
    reply->Send(this->b);
}

/* Invalidate properties Getter functions */

QStatus AllTypes::Getprop_boolean_inval(bool& _prop_boolean_inval) const
{
    _prop_boolean_inval = prop_boolean_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_byte_inval(uint8_t& _prop_byte_inval) const
{
    _prop_byte_inval = prop_byte_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_int16_inval(int16_t& _prop_int16_inval) const
{
    _prop_int16_inval = prop_int16_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_uint16_inval(uint16_t& _prop_uint16_inval) const
{
    _prop_uint16_inval = prop_uint16_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_int32_inval(int32_t& _prop_int32_inval) const
{
    _prop_int32_inval = prop_int32_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_uint32_inval(uint32_t& _prop_uint32_inval) const
{
    _prop_uint32_inval = prop_uint32_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_int64_inval(int64_t& _prop_int64_inval) const
{
    _prop_int64_inval = prop_int64_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_uint64_inval(uint64_t& _prop_uint64_inval) const
{
    _prop_uint64_inval = prop_uint64_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_double_inval(double& _prop_double_inval) const
{
    _prop_double_inval = prop_double_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_string_inval(qcc::String& _prop_string_inval) const
{
    _prop_string_inval = prop_string_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_signature_inval(datadriven::Signature& _prop_signature_inval) const
{
    _prop_signature_inval = prop_signature_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_objpath_inval(datadriven::ObjectPath& _prop_objpath_inval) const
{
    _prop_objpath_inval = prop_objpath_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_variant_inval(ajn::MsgArg& _prop_variant_inval) const
{
    _prop_variant_inval = prop_variant_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_struct_inval(AllTypesInterface::Type::StructWithAllTypes& _prop_struct_inval) const
{
    _prop_struct_inval = prop_struct_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_inval(std::vector<int32_t>& _prop_array_inval) const
{
    _prop_array_inval = prop_array_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_inval(AllTypesInterface::Type::DictionaryOfInt32& _prop_dict_inval) const
{
    _prop_dict_inval = prop_dict_inval;
    return ER_OK;
}

// Array inval properties getter functions

QStatus AllTypes::Getprop_array_of_boolean_inval(std::vector<bool>& _prop_array_of_boolean_inval) const
{
    _prop_array_of_boolean_inval = prop_array_of_boolean_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_of_byte_inval(std::vector<uint8_t>& _prop_array_of_byte_inval) const
{
    _prop_array_of_byte_inval = prop_array_of_byte_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_of_int16_inval(std::vector<int16_t>& _prop_array_of_int16_inval) const
{
    _prop_array_of_int16_inval = prop_array_of_int16_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_of_uint16_inval(std::vector<uint16_t>& _prop_array_of_uint16_inval) const
{
    _prop_array_of_uint16_inval = prop_array_of_uint16_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_of_int32_inval(std::vector<int32_t>& _prop_array_of_int32_inval) const
{
    _prop_array_of_int32_inval = prop_array_of_int32_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_of_uint32_inval(std::vector<uint32_t>& _prop_array_of_uint32_inval) const
{
    _prop_array_of_uint32_inval = prop_array_of_uint32_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_of_int64_inval(std::vector<int64_t>& _prop_array_of_int64_inval) const
{
    _prop_array_of_int64_inval = prop_array_of_int64_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_of_uint64_inval(std::vector<uint64_t>& _prop_array_of_uint64_inval) const
{
    _prop_array_of_uint64_inval = prop_array_of_uint64_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_of_double_inval(std::vector<double>& _prop_array_of_double_inval) const
{
    _prop_array_of_double_inval = prop_array_of_double_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_of_string_inval(std::vector<qcc::String>& _prop_array_of_string_inval) const
{
    _prop_array_of_string_inval = prop_array_of_string_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_of_signature_inval(std::vector<datadriven::Signature>& _prop_array_of_signature_inval)
const
{
    _prop_array_of_signature_inval = prop_array_of_signature_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_of_objpath_inval(std::vector<datadriven::ObjectPath>& _prop_array_of_objpath_inval)
const
{
    _prop_array_of_objpath_inval = prop_array_of_objpath_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_of_struct_inval(
    std::vector<AllArraysInterface::Type::StructNested>& _prop_array_of_struct_inval) const
{
    _prop_array_of_struct_inval = prop_array_of_struct_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_of_array_inval(std::vector<std::vector<int32_t> >& _prop_array_of_array_inval) const
{
    _prop_array_of_array_inval = prop_array_of_array_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_array_of_dict_inval(
    std::vector<AllArraysInterface::Type::DictionaryOfInt32>& _prop_array_of_dict_inval) const
{
    _prop_array_of_dict_inval = prop_array_of_dict_inval;
    return ER_OK;
}

// Dictionary inval properties getter functions

QStatus AllTypes::Getprop_dict_of_boolean_inval(
    AllDictionariesInterface::Type::DictionaryOfBoolean& _prop_dict_of_boolean_inval) const
{
    _prop_dict_of_boolean_inval = prop_dict_of_boolean_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_of_byte_inval(AllDictionariesInterface::Type::DictionaryOfByte& _prop_dict_of_byte_inval)
const
{
    _prop_dict_of_byte_inval = prop_dict_of_byte_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_of_int16_inval(
    AllDictionariesInterface::Type::DictionaryOfInt16& _prop_dict_of_int16_inval) const
{
    _prop_dict_of_int16_inval = prop_dict_of_int16_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_of_uint16_inval(
    AllDictionariesInterface::Type::DictionaryOfUnsignedInt16& _prop_dict_of_uint16_inval) const
{
    _prop_dict_of_uint16_inval = prop_dict_of_uint16_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_of_int32_inval(
    AllDictionariesInterface::Type::DictionaryOfInt32& _prop_dict_of_int32_inval) const
{
    _prop_dict_of_int32_inval = prop_dict_of_int32_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_of_uint32_inval(
    AllDictionariesInterface::Type::DictionaryOfUnsignedInt32& _prop_dict_of_uint32_inval) const
{
    _prop_dict_of_uint32_inval = prop_dict_of_uint32_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_of_int64_inval(
    AllDictionariesInterface::Type::DictionaryOfInt64& _prop_dict_of_int64_inval) const
{
    _prop_dict_of_int64_inval = prop_dict_of_int64_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_of_uint64_inval(
    AllDictionariesInterface::Type::DictionaryOfUnsignedInt64& _prop_dict_of_uint64_inval) const
{
    _prop_dict_of_uint64_inval = prop_dict_of_uint64_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_of_double_inval(
    AllDictionariesInterface::Type::DictionaryOfDouble& _prop_dict_of_double_inval) const
{
    _prop_dict_of_double_inval = prop_dict_of_double_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_of_string_inval(
    AllDictionariesInterface::Type::DictionaryOfString& _prop_dict_of_string_inval) const
{
    _prop_dict_of_string_inval = prop_dict_of_string_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_of_signature_inval(
    AllDictionariesInterface::Type::DictionaryOfSignature& _prop_dict_of_signature_inval) const
{
    _prop_dict_of_signature_inval = prop_dict_of_signature_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_of_objpath_inval(
    AllDictionariesInterface::Type::DictionaryOfObjPath& _prop_dict_of_objpath_inval) const
{
    _prop_dict_of_objpath_inval = prop_dict_of_objpath_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_of_struct_inval(
    AllDictionariesInterface::Type::DictionaryOfStruct& _prop_dict_of_struct_inval) const
{
    _prop_dict_of_struct_inval = prop_dict_of_struct_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_of_array_inval(
    AllDictionariesInterface::Type::DictionaryOfArray& _prop_dict_of_array_inval) const
{
    _prop_dict_of_array_inval = prop_dict_of_array_inval;
    return ER_OK;
}

QStatus AllTypes::Getprop_dict_of_dict_inval(
    AllDictionariesInterface::Type::DictionaryOfDictionary& _prop_dict_of_dict_inval) const
{
    _prop_dict_of_dict_inval = prop_dict_of_dict_inval;
    return ER_OK;
}
};
