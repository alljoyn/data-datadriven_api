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

#include "validate.h"
#include <cstdio>

/* New value of cnt for the invalidated properties,
 * just to make sure they are correctly picked up on the other side
 */
#define TEST_CNT_VAL 30

namespace test_system_alltypes {
template <> void validate<bool>(const bool& b,
                                size_t nelem,
                                long long& cnt)
{
    assert((++cnt % 2 ? true : false) == b);
}

template <> void validate<qcc::String>(const qcc::String& s,
                                       size_t nelem,
                                       long long& cnt)
{
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "%lld", ++cnt);
    assert(0 == s.compare(qcc::String(buffer)));
}

template <> void validate<datadriven::Signature>(const datadriven::Signature& s,
                                                 size_t nelem,
                                                 long long& cnt)
{
    assert(0 == s.compare(std::string((size_t)++ cnt, 'i').c_str()));
}

template <> void validate<datadriven::ObjectPath>(const datadriven::ObjectPath& o,
                                                  size_t nelem,
                                                  long long& cnt)
{
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "/path/%lld", ++cnt);
    assert(0 == o.compare(qcc::String(buffer)));
}

template <> void validate<ajn::MsgArg>(const ajn::MsgArg& v,
                                       size_t nelem,
                                       long long& cnt)
{
    assert(v.typeId == ajn::ALLJOYN_VARIANT);
    assert(v.v_variant.val->typeId == ajn::ALLJOYN_DOUBLE);
    assert(v.v_variant.val->v_double == ++cnt);
}

template <> void validate<AllTypesTypeDescription::StructNested>(const AllTypesTypeDescription::StructNested& sn,
                                                                 size_t nelem,
                                                                 long long& cnt)
{
    assert(++cnt == sn.member_int32);
}

template <> void validate<AllTypesTypeDescription::StructWithAllTypes>(
    const AllTypesTypeDescription::StructWithAllTypes& swat,
    size_t nelem,
    long long& cnt)
{
    validate(swat.member_boolean, nelem, cnt);
    validate(swat.member_byte, nelem, cnt);
    validate(swat.member_int16, nelem, cnt);
    validate(swat.member_uint16, nelem, cnt);
    validate(swat.member_int32, nelem, cnt);
    validate(swat.member_uint32, nelem, cnt);
    validate(swat.member_int64, nelem, cnt);
    validate(swat.member_uint64, nelem, cnt);
    validate(swat.member_double, nelem, cnt);
    validate(swat.member_string, nelem, cnt);
    validate(swat.member_signature, nelem, cnt);
    validate(swat.member_objpath, nelem, cnt);
    validate(swat.member_variant, nelem, cnt);
    validate(swat.member_struct, nelem, cnt);
    validate(swat.member_array, nelem, cnt);
    validate(swat.member_dict, nelem, cnt);
}

template <> void validate<bool>(const std::vector<bool>& v,
                                size_t nelem,
                                long long& cnt)
{
    bool b = true;

    assert(nelem == v.size());
    for (size_t i = 0; i < nelem; i++) {
        assert(b == v[i]);
        b = !b;
    }
}

template <> void validate<bool, bool>(const std::map<const bool, bool>& m,
                                      size_t nelem,
                                      long long& cnt)
{
    bool b = false; // map is sorted so false comes first

    // maximum two values in map with boolean keys
    nelem = nelem > 2 ? 2 : nelem;
    assert(nelem == m.size());
    for (std::map<const bool, bool>::const_iterator it = m.begin(); it != m.end(); ++it) {
        assert(b == it->first);
        assert(b == it->second);
        b = !b;
    }
}

void validate(const AllTypesProxy::Properties& atpp,
              size_t nelem)
{
    long long cnt = 0;

    validate(atpp.prop_boolean, nelem, cnt);
    validate(atpp.prop_byte, nelem, cnt);
    validate(atpp.prop_int16, nelem, cnt);
    validate(atpp.prop_uint16, nelem, cnt);
    validate(atpp.prop_int32, nelem, cnt);
    validate(atpp.prop_uint32, nelem, cnt);
    validate(atpp.prop_int64, nelem, cnt);
    validate(atpp.prop_uint64, nelem, cnt);
    validate(atpp.prop_double, nelem, cnt);
    validate(atpp.prop_string, nelem, cnt);
    validate(atpp.prop_signature, nelem, cnt);
    validate(atpp.prop_objpath, nelem, cnt);
    validate(atpp.prop_variant, nelem, cnt);
    validate(atpp.prop_struct, nelem, cnt);
    validate(atpp.prop_array, nelem, cnt);
    validate(atpp.prop_dict, nelem, cnt);
}

void validateInvalidate(const std::shared_ptr<AllTypesProxy>& atp, size_t nelem, long long cnt_val)
{
    long long cnt = cnt_val;

    validate(atp->Getprop_boolean_inval()->GetReply().prop_boolean_inval, nelem, cnt);
    validate(atp->Getprop_byte_inval()->GetReply().prop_byte_inval, nelem, cnt);
    validate(atp->Getprop_int16_inval()->GetReply().prop_int16_inval, nelem, cnt);
    validate(atp->Getprop_uint16_inval()->GetReply().prop_uint16_inval, nelem, cnt);
    validate(atp->Getprop_int32_inval()->GetReply().prop_int32_inval, nelem, cnt);
    validate(atp->Getprop_uint32_inval()->GetReply().prop_uint32_inval, nelem, cnt);
    validate(atp->Getprop_int64_inval()->GetReply().prop_int64_inval, nelem, cnt);
    validate(atp->Getprop_uint64_inval()->GetReply().prop_uint64_inval, nelem, cnt);
    validate(atp->Getprop_double_inval()->GetReply().prop_double_inval, nelem, cnt);
    validate(atp->Getprop_string_inval()->GetReply().prop_string_inval, nelem, cnt);
    validate(atp->Getprop_signature_inval()->GetReply().prop_signature_inval, nelem, cnt);
    validate(atp->Getprop_objpath_inval()->GetReply().prop_objpath_inval, nelem, cnt);
    validate(atp->Getprop_variant_inval()->GetReply().prop_variant_inval, nelem, cnt);
    validate(atp->Getprop_struct_inval()->GetReply().prop_struct_inval, nelem, cnt);
    validate(atp->Getprop_array_inval()->GetReply().prop_array_inval, nelem, cnt);
    validate(atp->Getprop_dict_inval()->GetReply().prop_dict_inval, nelem, cnt);
}

void validate(const AllTypesProxy::SignalWithAllTypes& signal,
              size_t nelem)
{
    long long cnt = 0;

    validate(signal.arg_boolean, nelem, cnt);
    validate(signal.arg_byte, nelem, cnt);
    validate(signal.arg_int16, nelem, cnt);
    validate(signal.arg_uint16, nelem, cnt);
    validate(signal.arg_int32, nelem, cnt);
    validate(signal.arg_uint32, nelem, cnt);
    validate(signal.arg_int64, nelem, cnt);
    validate(signal.arg_uint64, nelem, cnt);
    validate(signal.arg_double, nelem, cnt);
    validate(signal.arg_string, nelem, cnt);
    validate(signal.arg_signature, nelem, cnt);
    validate(signal.arg_objpath, nelem, cnt);
    validate(signal.arg_variant, nelem, cnt);
    validate(signal.arg_struct, nelem, cnt);
}

void validate(const AllTypesProxy::MethodWithAllTypesInAndOutReply& reply,
              size_t nelem)
{
    long long cnt = 0;

    validate(reply.arg_out_boolean, nelem, cnt);
    validate(reply.arg_out_byte, nelem, cnt);
    validate(reply.arg_out_int16, nelem, cnt);
    validate(reply.arg_out_uint16, nelem, cnt);
    validate(reply.arg_out_int32, nelem, cnt);
    validate(reply.arg_out_uint32, nelem, cnt);
    validate(reply.arg_out_int64, nelem, cnt);
    validate(reply.arg_out_uint64, nelem, cnt);
    validate(reply.arg_out_double, nelem, cnt);
    validate(reply.arg_out_string, nelem, cnt);
    validate(reply.arg_out_signature, nelem, cnt);
    validate(reply.arg_out_objpath, nelem, cnt);
    validate(reply.arg_out_variant, nelem, cnt);
    validate(reply.arg_out_struct, nelem, cnt);
}

template <> void validate<AllArraysTypeDescription::StructNested>(const AllArraysTypeDescription::StructNested& sn,
                                                                  size_t nelem,
                                                                  long long& cnt)
{
    assert(++cnt == sn.member_int32);
}

void validate(const AllArraysProxy::Properties& aapp,
              size_t nelem)
{
    long long cnt = 0;

    validate(aapp.prop_array_of_boolean, nelem, cnt);
    validate(aapp.prop_array_of_byte, nelem, cnt);
    validate(aapp.prop_array_of_int16, nelem, cnt);
    validate(aapp.prop_array_of_uint16, nelem, cnt);
    validate(aapp.prop_array_of_int32, nelem, cnt);
    validate(aapp.prop_array_of_uint32, nelem, cnt);
    validate(aapp.prop_array_of_int64, nelem, cnt);
    validate(aapp.prop_array_of_uint64, nelem, cnt);
    validate(aapp.prop_array_of_double, nelem, cnt);
    validate(aapp.prop_array_of_string, nelem, cnt);
    validate(aapp.prop_array_of_signature, nelem, cnt);
    validate(aapp.prop_array_of_objpath, nelem, cnt);
    validate(aapp.prop_array_of_struct, nelem, cnt);
    validate(aapp.prop_array_of_array, nelem, cnt);
    validate(aapp.prop_array_of_dict, nelem, cnt);
}

void validateInvalidate(const std::shared_ptr<AllArraysProxy>& aap, size_t nelem, long long cnt_val)
{
    long long cnt = cnt_val;

    validate(aap->Getprop_array_of_boolean_inval()->GetReply().prop_array_of_boolean_inval, nelem, cnt);
    validate(aap->Getprop_array_of_byte_inval()->GetReply().prop_array_of_byte_inval, nelem, cnt);
    validate(aap->Getprop_array_of_int16_inval()->GetReply().prop_array_of_int16_inval, nelem, cnt);
    validate(aap->Getprop_array_of_uint16_inval()->GetReply().prop_array_of_uint16_inval, nelem, cnt);
    validate(aap->Getprop_array_of_int32_inval()->GetReply().prop_array_of_int32_inval, nelem, cnt);
    validate(aap->Getprop_array_of_uint32_inval()->GetReply().prop_array_of_uint32_inval, nelem, cnt);
    validate(aap->Getprop_array_of_int64_inval()->GetReply().prop_array_of_int64_inval, nelem, cnt);
    validate(aap->Getprop_array_of_uint64_inval()->GetReply().prop_array_of_uint64_inval, nelem, cnt);
    validate(aap->Getprop_array_of_double_inval()->GetReply().prop_array_of_double_inval, nelem, cnt);
    validate(aap->Getprop_array_of_string_inval()->GetReply().prop_array_of_string_inval, nelem, cnt);
    validate(aap->Getprop_array_of_signature_inval()->GetReply().prop_array_of_signature_inval, nelem, cnt);
    validate(aap->Getprop_array_of_objpath_inval()->GetReply().prop_array_of_objpath_inval, nelem, cnt);
    validate(aap->Getprop_array_of_struct_inval()->GetReply().prop_array_of_struct_inval, nelem, cnt);
    validate(aap->Getprop_array_of_array_inval()->GetReply().prop_array_of_array_inval, nelem, cnt);
    validate(aap->Getprop_array_of_dict_inval()->GetReply().prop_array_of_dict_inval, nelem, cnt);
}

void validate(const AllArraysProxy::SignalWithAllArrays& signal,
              size_t nelem)
{
    long long cnt = 0;

    validate(signal.arg_boolean, nelem, cnt);
    validate(signal.arg_byte, nelem, cnt);
    validate(signal.arg_int16, nelem, cnt);
    validate(signal.arg_uint16, nelem, cnt);
    validate(signal.arg_int32, nelem, cnt);
    validate(signal.arg_uint32, nelem, cnt);
    validate(signal.arg_int64, nelem, cnt);
    validate(signal.arg_uint64, nelem, cnt);
    validate(signal.arg_double, nelem, cnt);
    validate(signal.arg_string, nelem, cnt);
    validate(signal.arg_signature, nelem, cnt);
    validate(signal.arg_objpath, nelem, cnt);
    validate(signal.arg_struct, nelem, cnt);
    validate(signal.arg_array, nelem, cnt);
    validate(signal.arg_dict, nelem, cnt);
}

void validate(const AllArraysProxy::MethodWithAllArraysInAndOutReply& reply,
              size_t nelem,
              long long& cnt)
{
    validate(reply.arg_out_boolean, nelem, cnt);
    validate(reply.arg_out_byte, nelem, cnt);
    validate(reply.arg_out_int16, nelem, cnt);
    validate(reply.arg_out_uint16, nelem, cnt);
    validate(reply.arg_out_int32, nelem, cnt);
    validate(reply.arg_out_uint32, nelem, cnt);
    validate(reply.arg_out_int64, nelem, cnt);
    validate(reply.arg_out_uint64, nelem, cnt);
    validate(reply.arg_out_double, nelem, cnt);
    validate(reply.arg_out_string, nelem, cnt);
    validate(reply.arg_out_signature, nelem, cnt);
    validate(reply.arg_out_objpath, nelem, cnt);
    validate(reply.arg_out_struct, nelem, cnt);
    validate(reply.arg_out_array, nelem, cnt);
    validate(reply.arg_out_dict, nelem, cnt);
}

template <> void validate<AllDictionariesTypeDescription::StructNested>(
    const AllDictionariesTypeDescription::StructNested& sn,
    size_t nelem,
    long long& cnt)
{
    assert(++cnt == sn.member_int32);
}

void validate(const AllDictionariesProxy::Properties& adpp,
              size_t nelem)
{
    long long cnt = 0;

    validate(adpp.prop_dict_of_boolean, nelem, cnt);
    validate(adpp.prop_dict_of_byte, nelem, cnt);
    validate(adpp.prop_dict_of_int16, nelem, cnt);
    validate(adpp.prop_dict_of_uint16, nelem, cnt);
    validate(adpp.prop_dict_of_int32, nelem, cnt);
    validate(adpp.prop_dict_of_uint32, nelem, cnt);
    validate(adpp.prop_dict_of_int64, nelem, cnt);
    validate(adpp.prop_dict_of_uint64, nelem, cnt);
    validate(adpp.prop_dict_of_double, nelem, cnt);
    validate(adpp.prop_dict_of_string, nelem, cnt);
    validate(adpp.prop_dict_of_signature, nelem, cnt);
    validate(adpp.prop_dict_of_objpath, nelem, cnt);
    validate(adpp.prop_dict_of_struct, nelem, cnt);
    validate(adpp.prop_dict_of_array, nelem, cnt);
    validate(adpp.prop_dict_of_dict, nelem, cnt);
}

void validateInvalidate(const std::shared_ptr<AllDictionariesProxy>& aap, size_t nelem, long long cnt_val)
{
    long long cnt = cnt_val;

    validate(aap->Getprop_dict_of_boolean_inval()->GetReply().prop_dict_of_boolean_inval, nelem, cnt);
    validate(aap->Getprop_dict_of_byte_inval()->GetReply().prop_dict_of_byte_inval, nelem, cnt);
    validate(aap->Getprop_dict_of_int16_inval()->GetReply().prop_dict_of_int16_inval, nelem, cnt);
    validate(aap->Getprop_dict_of_uint16_inval()->GetReply().prop_dict_of_uint16_inval, nelem, cnt);
    validate(aap->Getprop_dict_of_int32_inval()->GetReply().prop_dict_of_int32_inval, nelem, cnt);
    validate(aap->Getprop_dict_of_uint32_inval()->GetReply().prop_dict_of_uint32_inval, nelem, cnt);
    validate(aap->Getprop_dict_of_int64_inval()->GetReply().prop_dict_of_int64_inval, nelem, cnt);
    validate(aap->Getprop_dict_of_uint64_inval()->GetReply().prop_dict_of_uint64_inval, nelem, cnt);
    validate(aap->Getprop_dict_of_double_inval()->GetReply().prop_dict_of_double_inval, nelem, cnt);
    validate(aap->Getprop_dict_of_string_inval()->GetReply().prop_dict_of_string_inval, nelem, cnt);
    validate(aap->Getprop_dict_of_signature_inval()->GetReply().prop_dict_of_signature_inval, nelem, cnt);
    validate(aap->Getprop_dict_of_objpath_inval()->GetReply().prop_dict_of_objpath_inval, nelem, cnt);
    validate(aap->Getprop_dict_of_struct_inval()->GetReply().prop_dict_of_struct_inval, nelem, cnt);
    validate(aap->Getprop_dict_of_array_inval()->GetReply().prop_dict_of_array_inval, nelem, cnt);
    validate(aap->Getprop_dict_of_dict_inval()->GetReply().prop_dict_of_dict_inval, nelem, cnt);
}

void validate(const AllDictionariesProxy::SignalWithAllDictionaries& signal,
              size_t nelem)
{
    long long cnt = 0;

    validate(signal.arg_boolean, nelem, cnt);
    validate(signal.arg_byte, nelem, cnt);
    validate(signal.arg_int16, nelem, cnt);
    validate(signal.arg_uint16, nelem, cnt);
    validate(signal.arg_int32, nelem, cnt);
    validate(signal.arg_uint32, nelem, cnt);
    validate(signal.arg_int64, nelem, cnt);
    validate(signal.arg_uint64, nelem, cnt);
    validate(signal.arg_double, nelem, cnt);
    validate(signal.arg_string, nelem, cnt);
    validate(signal.arg_signature, nelem, cnt);
    validate(signal.arg_objpath, nelem, cnt);
    validate(signal.arg_struct, nelem, cnt);
    validate(signal.arg_array, nelem, cnt);
    validate(signal.arg_dict, nelem, cnt);
}

void validate(const AllDictionariesProxy::MethodWithAllDictionariesInAndOutReply& reply,
              size_t nelem,
              long long& cnt)
{
    validate(reply.arg_out_boolean, nelem, cnt);
    validate(reply.arg_out_byte, nelem, cnt);
    validate(reply.arg_out_int16, nelem, cnt);
    validate(reply.arg_out_uint16, nelem, cnt);
    validate(reply.arg_out_int32, nelem, cnt);
    validate(reply.arg_out_uint32, nelem, cnt);
    validate(reply.arg_out_int64, nelem, cnt);
    validate(reply.arg_out_uint64, nelem, cnt);
    validate(reply.arg_out_double, nelem, cnt);
    validate(reply.arg_out_string, nelem, cnt);
    validate(reply.arg_out_signature, nelem, cnt);
    validate(reply.arg_out_objpath, nelem, cnt);
    validate(reply.arg_out_struct, nelem, cnt);
    validate(reply.arg_out_array, nelem, cnt);
    validate(reply.arg_out_dict, nelem, cnt);
}
};
