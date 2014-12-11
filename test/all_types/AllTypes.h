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
    AllTypes(std::shared_ptr<datadriven::ObjectAdvertiser> busconn,
             size_t nelem);

    virtual ~AllTypes();

    /**
     * \private blocking call to wait for the consumer to tell the provider to continue
     */
    void WaitForInvalidateAll();

    /**
     * \test Let the provider know to continue with the test.
     */
    virtual void InvalidateAll(int64_t new_cnt,
                               std::shared_ptr<InvalidateAllReply> _reply);

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
                                    ajn::MsgArg arg_in_variant,
                                    const AllTypesInterface::Type::StructWithAllTypes& arg_in_struct,
                                    std::shared_ptr<MethodWithAllTypesInAndOutReply> _reply);

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
                                     std::shared_ptr<MethodWithAllArraysInAndOutReply> reply);

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
                                           std::shared_ptr<MethodWithAllDictionariesInAndOutReply> reply);

    /**
     * \test No output argument method call:
     *       -# reception of a method call with input arguments
     *       -# respond with reply without output arguments
     *       -# fire signal without arguments
     */
    void MethodWithoutOut(bool arg_in_boolean,
                          std::shared_ptr<MethodWithoutOutReply> reply);

    /**
     * \test No input arguments method call:
     *       -# reception of a method call without input arguments
     *       -# respond with reply with output arguments
     */
    void MethodWithoutIn(std::shared_ptr<MethodWithoutInReply> reply);

    // Inval properties getter functions

    /* \brief Getter function for property boolean_inval
     * \param[out] _prop_boolean_inval the value of the boolean_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_boolean_inval(bool& _prop_boolean_inval) const;

    /* \brief Getter function for property byte_inval
     * \param[out] _prop_byte_inval the value of the byte_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_byte_inval(uint8_t& _prop_byte_inval) const;

    /* \brief Getter function for property int16_inval
     * \param[out] _prop_int16_inval the value of the int16_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_int16_inval(int16_t& _prop_int16_inval) const;

    /* \brief Getter function for property uint16_inval
     * \param[out] _prop_uint16_inval the value of the uint16_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_uint16_inval(uint16_t& _prop_uint16_inval) const;

    /* \brief Getter function for property int32_inval
     * \param[out] _prop_int32_inval the value of the int32_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_int32_inval(int32_t& _prop_int32_inval) const;

    /* \brief Getter function for property uint32_inval
     * \param[out] _prop_uint32_inval the value of the uint32_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_uint32_inval(uint32_t& _prop_uint32_inval) const;

    /* \brief Getter function for property int64_inval
     * \param[out] _prop_int64_inval the value of the int64_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_int64_inval(int64_t& _prop_int64_inval) const;

    /* \brief Getter function for property uint64_inval
     * \param[out] _prop_uint64_inval the value of the uint64_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_uint64_inval(uint64_t& _prop_uint64_inval) const;

    /* \brief Getter function for property double_inval
     * \param[out] _prop_double_inval the value of the double_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_double_inval(double& _prop_double_inval) const;

    /* \brief Getter function for property string_inval
     * \param[out] _prop_string_inval the value of the string_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_string_inval(qcc::String& _prop_string_inval) const;

    /* \brief Getter function for property byte_inval
     * \param[out] _prop_byte_inval the value of the byte_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_signature_inval(datadriven::Signature& _prop_signature_inval) const;

    /* \brief Getter function for property objpath_inval
     * \param[out] _prop_objpath_inval the value of the objpath_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_objpath_inval(datadriven::ObjectPath& _prop_objpath_inval) const;

    /* \brief Getter function for property variant_inval
     * \param[out] _prop_variant_inval the value of the variant_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_variant_inval(ajn::MsgArg& _prop_variant_inval) const;

    /* \brief Getter function for property struct_inval
     * \param[out] _prop_struct_inval the value of the struct_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_struct_inval(AllTypesInterface::Type::StructWithAllTypes& _prop_struct_inval) const;

    /* \brief Getter function for property array_inval
     * \param[out] _prop_array_inval the value of the array_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_inval(std::vector<int32_t>& _prop_array_inval) const;

    /* \brief Getter function for property dict_inval
     * \param[out] _prop_dict_inval the value of the dict_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_inval(AllTypesInterface::Type::DictionaryOfInt32& _prop_dict_inval) const;

    // Array inval properties getter functions

    /* \brief Getter function for property array_of_boolean_inval
     * \param[out] _prop_array_of_boolean_inval the value of the array_of_boolean_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_boolean_inval(std::vector<bool>& _prop_array_of_boolean_inval) const;

    /* \brief Getter function for property array_of_byte_inval
     * \param[out] _prop_array_of_byte_inval the value of the array_of_byte_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_byte_inval(std::vector<uint8_t>& _prop_array_of_byte_inval) const;

    /* \brief Getter function for property array_of_int16_inval
     * \param[out] _prop_array_of_int16_inval the value of the array_of_int16_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_int16_inval(std::vector<int16_t>& _prop_array_of_int16_inval) const;

    /* \brief Getter function for property array_of_uint16_inval
     * \param[out] _prop_array_of_uint16_inval the value of the array_of_uint16_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_uint16_inval(std::vector<uint16_t>& _prop_array_of_uint16_inval) const;

    /* \brief Getter function for property array_of_int32_inval
     * \param[out] _prop_array_of_int32_inval the value of the array_of_int32_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_int32_inval(std::vector<int32_t>& _prop_array_of_int32_inval) const;

    /* \brief Getter function for property array_of_uint32_inval
     * \param[out] _prop_array_of_uint32_inval the value of the array_of_uint32_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_uint32_inval(std::vector<uint32_t>& _prop_array_of_uint32_inval) const;

    /* \brief Getter function for property array_of_int64_inval
     * \param[out] _prop_array_of_int64_inval the value of the array_of_uint64_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_int64_inval(std::vector<int64_t>& _prop_array_of_int64_inval) const;

    /* \brief Getter function for property array_of_uint64_inval
     * \param[out] _prop_array_of_uint64_inval the value of the array_of_uint64_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_uint64_inval(std::vector<uint64_t>& _prop_array_of_uint64_inval) const;

    /* \brief Getter function for property array_of_double_inval
     * \param[out] _prop_array_of_double_inval the value of the array_of_double_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_double_inval(std::vector<double>& _prop_array_of_double_inval) const;

    /* \brief Getter function for property array_of_string_inval
     * \param[out] _prop_array_of_string_inval the value of the array_of_string_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_string_inval(std::vector<qcc::String>& _prop_array_of_string_inval) const;

    /* \brief Getter function for property array_of_signature_inval
     * \param[out] _prop_array_of_signature_inval the value of the array_of_signature_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_signature_inval(std::vector<datadriven::Signature>& _prop_array_of_signature_inval) const;

    /* \brief Getter function for property array_of_objpath_inval
     * \param[out] _prop_array_of_objpath_inval the value of the array_of_objpath_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_objpath_inval(std::vector<datadriven::ObjectPath>& _prop_array_of_objpath_inval) const;

    /* \brief Getter function for property array_of_struct_inval
     * \param[out] _prop_array_of_struct_inval the value of the array_of_struct_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_struct_inval(
        std::vector<AllArraysInterface::Type::StructNested>& _prop_array_of_struct_inval)
    const;

    /* \brief Getter function for property array_of_array_inval
     * \param[out] _prop_array_of_array_inval the value of the array_of_array_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_array_inval(std::vector<std::vector<int32_t> >& _prop_array_of_array_inval) const;

    /* \brief Getter function for property array_of_dict_inval
     * \param[out] _prop_array_of_dict_inval the value of the array_of_dict_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_array_of_dict_inval(
        std::vector<AllArraysInterface::Type::DictionaryOfInt32>& _prop_array_of_dict_inval)
    const;

    // Dictionary inval properties getter functions

    /* \brief Getter function for property dict_of_boolean_inval
     * \param[out] _prop_dict_of_boolean_inval the value of the dict_of_boolean_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_boolean_inval(
        AllDictionariesInterface::Type::DictionaryOfBoolean& _prop_dict_of_boolean_inval) const;

    /* \brief Getter function for property dict_of_byte_inval
     * \param[out] _prop_dict_of_byte_inval the value of the dict_of_byte_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_byte_inval(AllDictionariesInterface::Type::DictionaryOfByte& _prop_dict_of_byte_inval)
    const;

    /* \brief Getter function for property dict_of_int16_inval
     * \param[out] _prop_dict_of_int16_inval the value of the dict_of_int16_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_int16_inval(AllDictionariesInterface::Type::DictionaryOfInt16& _prop_dict_of_int16_inval)
    const;

    /* \brief Getter function for property dict_of_uint16_inval
     * \param[out] _prop_dict_of_uint16_inval the value of the dict_of_uint16_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_uint16_inval(
        AllDictionariesInterface::Type::DictionaryOfUnsignedInt16& _prop_dict_of_uint16_inval) const;

    /* \brief Getter function for property dict_of_int32_inval
     * \param[out] _prop_dict_of_int32_inval the value of the dict_of_int32_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_int32_inval(AllDictionariesInterface::Type::DictionaryOfInt32& _prop_dict_of_int32_inval)
    const;

    /* \brief Getter function for property dict_of_uint32_inval
     * \param[out] _prop_dict_of_uint32_inval the value of the dict_of_uint32_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_uint32_inval(
        AllDictionariesInterface::Type::DictionaryOfUnsignedInt32& _prop_dict_of_uint32_inval) const;

    /* \brief Getter function for property dict_of_int64_inval
     * \param[out] _prop_dict_of_int64_inval the value of the dict_of_int64_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_int64_inval(AllDictionariesInterface::Type::DictionaryOfInt64& _prop_dict_of_int64_inval)
    const;

    /* \brief Getter function for property dict_of_uint64_inval
     * \param[out] _prop_dict_of_uint64_inval the value of the dict_of_uint64_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_uint64_inval(
        AllDictionariesInterface::Type::DictionaryOfUnsignedInt64& _prop_dict_of_uint64_inval) const;

    /* \brief Getter function for property dict_of_double_inval
     * \param[out] _prop_dict_of_double_inval the value of the dict_of_double_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_double_inval(AllDictionariesInterface::Type::DictionaryOfDouble& _prop_dict_of_double_inval)
    const;

    /* \brief Getter function for property dict_of_string_inval
     * \param[out] _prop_dict_of_string_inval the value of the dict_of_string_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_string_inval(AllDictionariesInterface::Type::DictionaryOfString& _prop_dict_of_string_inval)
    const;

    /* \brief Getter function for property dict_of_signature_inval
     * \param[out] _prop_dict_of_signature_inval the value of the dict_of_signature_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_signature_inval(
        AllDictionariesInterface::Type::DictionaryOfSignature& _prop_dict_of_signature_inval) const;

    /* \brief Getter function for property dict_of_objpath_inval
     * \param[out] _prop_dict_of_objpath_inval the value of the dict_of_objpath_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_objpath_inval(
        AllDictionariesInterface::Type::DictionaryOfObjPath& _prop_dict_of_objpath_inval) const;

    /* \brief Getter function for property dict_of_struct_inval
     * \param[out] _prop_dict_of_struct_inval the value of the dict_of_struct_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_struct_inval(AllDictionariesInterface::Type::DictionaryOfStruct& _prop_dict_of_struct_inval)
    const;

    /* \brief Getter function for property dict_of_array_inval
     * \param[out] _prop_dict_of_array_inval the value of the dict_of_array_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_array_inval(AllDictionariesInterface::Type::DictionaryOfArray& _prop_dict_of_array_inval)
    const;

    /* \brief Getter function for property dict_of_dict_inval
     * \param[out] _prop_dict_of_dict_inval the value of the dict_of_dict_inval property
     *
     * \retval ER_OK
     */
    QStatus Getprop_dict_of_dict_inval(AllDictionariesInterface::Type::DictionaryOfDictionary& _prop_dict_of_dict_inval)
    const;

  private:
    bool b;
    size_t nelem;

    // Inval properties
    bool prop_boolean_inval;
    uint8_t prop_byte_inval;
    int16_t prop_int16_inval;
    uint16_t prop_uint16_inval;
    int32_t prop_int32_inval;
    uint32_t prop_uint32_inval;
    int64_t prop_int64_inval;
    uint64_t prop_uint64_inval;
    double prop_double_inval;
    qcc::String prop_string_inval;
    datadriven::Signature prop_signature_inval;
    datadriven::ObjectPath prop_objpath_inval;
    ajn::MsgArg prop_variant_inval;
    AllTypesInterface::Type::StructWithAllTypes prop_struct_inval;
    std::vector<int32_t> prop_array_inval;
    AllTypesInterface::Type::DictionaryOfInt32 prop_dict_inval;

    // Array inval properties
    std::vector<bool> prop_array_of_boolean_inval;
    std::vector<uint8_t> prop_array_of_byte_inval;
    std::vector<int16_t> prop_array_of_int16_inval;
    std::vector<uint16_t> prop_array_of_uint16_inval;
    std::vector<int32_t> prop_array_of_int32_inval;
    std::vector<uint32_t> prop_array_of_uint32_inval;
    std::vector<int64_t> prop_array_of_int64_inval;
    std::vector<uint64_t> prop_array_of_uint64_inval;
    std::vector<double> prop_array_of_double_inval;
    std::vector<qcc::String> prop_array_of_string_inval;
    std::vector<datadriven::Signature> prop_array_of_signature_inval;
    std::vector<datadriven::ObjectPath> prop_array_of_objpath_inval;
    std::vector<AllArraysInterface::Type::StructNested> prop_array_of_struct_inval;
    std::vector<std::vector<int32_t> > prop_array_of_array_inval;
    std::vector<AllArraysInterface::Type::DictionaryOfInt32> prop_array_of_dict_inval;

    // Dictionary inval properties
    AllDictionariesInterface::Type::DictionaryOfBoolean prop_dict_of_boolean_inval;
    AllDictionariesInterface::Type::DictionaryOfByte prop_dict_of_byte_inval;
    AllDictionariesInterface::Type::DictionaryOfInt16 prop_dict_of_int16_inval;
    AllDictionariesInterface::Type::DictionaryOfUnsignedInt16 prop_dict_of_uint16_inval;
    AllDictionariesInterface::Type::DictionaryOfInt32 prop_dict_of_int32_inval;
    AllDictionariesInterface::Type::DictionaryOfUnsignedInt32 prop_dict_of_uint32_inval;
    AllDictionariesInterface::Type::DictionaryOfInt64 prop_dict_of_int64_inval;
    AllDictionariesInterface::Type::DictionaryOfUnsignedInt64 prop_dict_of_uint64_inval;
    AllDictionariesInterface::Type::DictionaryOfDouble prop_dict_of_double_inval;
    AllDictionariesInterface::Type::DictionaryOfString prop_dict_of_string_inval;
    AllDictionariesInterface::Type::DictionaryOfSignature prop_dict_of_signature_inval;
    AllDictionariesInterface::Type::DictionaryOfObjPath prop_dict_of_objpath_inval;
    AllDictionariesInterface::Type::DictionaryOfStruct prop_dict_of_struct_inval;
    AllDictionariesInterface::Type::DictionaryOfArray prop_dict_of_array_inval;
    AllDictionariesInterface::Type::DictionaryOfDictionary prop_dict_of_dict_inval;
};
};

#endif /* ALLTYPES_H_ */
