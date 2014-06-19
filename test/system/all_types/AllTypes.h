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

#ifndef ALLTYPES_H_
#define ALLTYPES_H_

#include "AllTypesInterface.h"
#include "AllArraysInterface.h"
#include "AllDictionariesInterface.h"

namespace test_system_alltypes {
/**
 * \class
 * Provided object with all supported types.
 */
class AllTypes :
    public datadriven::ProvidedObject,
    public gen::org_allseenalliance_test::AllTypesInterface,
    public gen::org_allseenalliance_test::AllArraysInterface,
    public gen::org_allseenalliance_test::AllDictionariesInterface {
  public:
    AllTypes(datadriven::BusConnection& busconn,
             size_t nelem);

    /**
     * \test All types method call:
     *       -# reception of a method call with input arguments of all types
     *       -# respond with reply with output arguments of all types
     *       -# fire signal with arguments of all types
     */
    void MethodWithAllTypesInAndOut(bool arg_in_boolean,
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
                                    MethodWithAllTypesInAndOutReply& _reply);

    /**
     * \test Arrays of all types method call:
     *       -# reception of a method call with input arguments of all arrays
     *       -# respond with reply with output arguments of all arrays
     *       -# fire signal with arguments of all arrays
     */
    void MethodWithAllArraysInAndOut(const std::vector<bool>& arg_in_boolean,
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
                                     MethodWithAllArraysInAndOutReply& reply);

    /**
     * \test Dictionaries with all types of keys and values method call:
     *       -# reception of a method call with input arguments of all dictionaries
     *       -# respond with reply with output arguments of all dictionaries
     *       -# fire signal with arguments of all dictionaries
     */
    void MethodWithAllDictionariesInAndOut(const AllDictionariesInterface::Type::DictionaryOfBoolean& arg_in_boolean,
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
                                           const AllDictionariesInterface::Type::DictionaryOfDouble& arg_in_double,
                                           const AllDictionariesInterface::Type::DictionaryOfString& arg_in_string,
                                           const AllDictionariesInterface::Type::DictionaryOfSignature&
                                           arg_in_signature,
                                           const AllDictionariesInterface::Type::DictionaryOfObjPath& arg_in_objpath,
                                           const AllDictionariesInterface::Type::DictionaryOfStruct& arg_in_struct,
                                           const AllDictionariesInterface::Type::DictionaryOfArray& arg_in_array,
                                           const AllDictionariesInterface::Type::DictionaryOfDictionary& arg_in_dict,
                                           MethodWithAllDictionariesInAndOutReply& reply);

    /**
     * \test No output argument method call:
     *       -# reception of a method call with input arguments
     *       -# respond with reply without output arguments
     *       -# fire signal without arguments
     */
    void MethodWithoutOut(bool arg_in_boolean,
                          MethodWithoutOutReply& reply);

    /**
     * \test No input arguments method call:
     *       -# reception of a method call without input arguments
     *       -# respond with reply with output arguments
     */
    void MethodWithoutIn(MethodWithoutInReply& reply);

  private:
    bool b;
};
};

#endif /* ALLTYPES_H_ */
