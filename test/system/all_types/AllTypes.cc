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

#include "AllTypes.h"
#include "init_data.h"

namespace test_system_alltypes {
using namespace std;
using namespace::gen::org_allseenalliance_test;

AllTypes::AllTypes(datadriven::BusConnection& busconn,
                   size_t nelem) :
    datadriven::ProvidedObject(busconn),
    AllTypesInterface(this),
    AllArraysInterface(this),
    AllDictionariesInterface(this),
    b(false)
{
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
    init_data(prop_struct, nelem, cnt);
    init_data(prop_array, nelem, cnt);
    init_data(prop_dict, nelem, cnt);
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
                                          const AllTypesInterface::Type::StructWithAllTypes& arg_in_struct,
                                          MethodWithAllTypesInAndOutReply& reply)
{
    cout << "Provider received method call for " << __FUNCTION__ << endl;
    cout << "Provider sending reply" << endl;
    reply.Send(arg_in_boolean,
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
                                           MethodWithAllArraysInAndOutReply& reply)
{
    cout << "Provider received method call for " << __FUNCTION__ << endl;
    cout << "Provider sending reply" << endl;
    reply.Send(arg_in_boolean,
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
    MethodWithAllDictionariesInAndOutReply& reply)
{
    cout << "Provider received method call for " << __FUNCTION__ << endl;
    cout << "Provider sending reply" << endl;
    reply.Send(arg_in_boolean,
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

void AllTypes::MethodWithoutOut(bool arg_in_boolean, MethodWithoutOutReply& reply)
{
    cout << "Provider received method call for " << __FUNCTION__ << endl;
    this->b = arg_in_boolean;
    cout << "Provider sending reply" << endl;
    reply.Send();
    // also send signal without arguments
    cout << "Provider sending signal" << endl;
    SignalWithoutArgs();
}

void AllTypes::MethodWithoutIn(MethodWithoutInReply& reply)
{
    cout << "Provider received method call for " << __FUNCTION__ << endl;
    cout << "Provider sending reply" << endl;
    reply.Send(this->b);
}
};
