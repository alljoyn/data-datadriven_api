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

#include <qcc/Debug.h>

#include "dump.h"

namespace test_system_alltypes {
#undef QCC_MODULE
#define QCC_MODULE "TEST_VERBOSE"
void dump(ostringstream& oss)
{
    QCC_DbgPrintf(("%s", oss.str().c_str()));
}

#undef QCC_MODULE

template <> void dump<uint8_t>(const string& prefix,
                               const uint8_t& t)
{
    ostringstream oss;

    oss << prefix << " = " << (int)t;
    dump(oss);
}

template <> void dump<qcc::String>(const string& prefix,
                                   const qcc::String& s)
{
    ostringstream oss;

    oss << prefix << " = " << s.c_str();
    dump(oss);
}

template <> void dump<datadriven::Signature>(const string& prefix,
                                             const datadriven::Signature& s)
{
    ostringstream oss;

    oss << prefix << " = " << s.c_str();
    dump(oss);
}

template <> void dump<datadriven::ObjectPath>(const string& prefix,
                                              const datadriven::ObjectPath& o)
{
    ostringstream oss;

    oss << prefix << " = " << o.c_str();
    dump(oss);
}

template <> void dump<ajn::MsgArg>(const string& prefix,
                                   const ajn::MsgArg& v)
{
    ostringstream oss;

    assert(v.typeId == ajn::ALLJOYN_VARIANT);
    oss << prefix << " = " << v.v_variant.val;
    dump(oss);
}

template <> void dump<AllTypesTypeDescription::StructNested>(const string& prefix,
                                                             const AllTypesTypeDescription::StructNested& sn)
{
    dump(prefix + ".member_int32", sn.member_int32);
}

template <> void dump<AllTypesTypeDescription::StructWithAllTypes>(const string& prefix,
                                                                   const AllTypesTypeDescription::StructWithAllTypes&
                                                                   swat)
{
    dump(prefix + ".member_boolean", swat.member_boolean);
    dump(prefix + ".member_byte", swat.member_byte);
    dump(prefix + ".member_int16", swat.member_int16);
    dump(prefix + ".member_uint16", swat.member_uint16);
    dump(prefix + ".member_int32", swat.member_int32);
    dump(prefix + ".member_uint32", swat.member_uint32);
    dump(prefix + ".member_int64", swat.member_int64);
    dump(prefix + ".member_uint64", swat.member_uint64);
    dump(prefix + ".member_double", swat.member_double);
    dump(prefix + ".member_string", swat.member_string);
    dump(prefix + ".member_signature", swat.member_signature);
    dump(prefix + ".member_objpath", swat.member_objpath);
    dump(prefix + ".member_variant", swat.member_variant);
    dump(prefix + ".member_struct", swat.member_struct);
    dump(prefix + ".member_array", swat.member_array);
    dump(prefix + ".member_dict", swat.member_dict);
}

template <> void dump<AllTypesProxy::Properties>(const string& prefix,
                                                 const AllTypesProxy::Properties& atpp)
{
    dump(prefix + "prop_boolean", atpp.prop_boolean);
    dump(prefix + "prop_byte", atpp.prop_byte);
    dump(prefix + "prop_int16", atpp.prop_int16);
    dump(prefix + "prop_uint16", atpp.prop_uint16);
    dump(prefix + "prop_int32", atpp.prop_int32);
    dump(prefix + "prop_uint32", atpp.prop_uint32);
    dump(prefix + "prop_int64", atpp.prop_int64);
    dump(prefix + "prop_uint64", atpp.prop_uint64);
    dump(prefix + "prop_double", atpp.prop_double);
    dump(prefix + "prop_string", atpp.prop_string);
    dump(prefix + "prop_signature", atpp.prop_signature);
    dump(prefix + "prop_objpath", atpp.prop_objpath);
    dump(prefix + "prop_variant", atpp.prop_variant);
    dump(prefix + "prop_struct", atpp.prop_struct);
    dump(prefix + "prop_array", atpp.prop_array);
    dump(prefix + "prop_dict", atpp.prop_dict);
}

template <> void dump<AllTypesProxy::SignalWithAllTypes>(const string& prefix,
                                                         const AllTypesProxy::SignalWithAllTypes& swat)
{
    dump(prefix + "arg_boolean", swat.arg_boolean);
    dump(prefix + "arg_byte", swat.arg_byte);
    dump(prefix + "arg_int16", swat.arg_int16);
    dump(prefix + "arg_uint16", swat.arg_uint16);
    dump(prefix + "arg_int32", swat.arg_int32);
    dump(prefix + "arg_uint32", swat.arg_uint32);
    dump(prefix + "arg_int64", swat.arg_int64);
    dump(prefix + "arg_uint64", swat.arg_uint64);
    dump(prefix + "arg_double", swat.arg_double);
    dump(prefix + "arg_string", swat.arg_string);
    dump(prefix + "arg_signature", swat.arg_signature);
    dump(prefix + "arg_objpath", swat.arg_objpath);
    dump(prefix + "arg_variant", swat.arg_variant);
    dump(prefix + "arg_struct", swat.arg_struct);
}

template <> void dump<AllTypesProxy::MethodWithAllTypesInAndOutReply>(const string& prefix,
                                                                      const AllTypesProxy::
                                                                      MethodWithAllTypesInAndOutReply& reply)
{
    dump(prefix + "arg_out_boolean", reply.arg_out_boolean);
    dump(prefix + "arg_out_byte", reply.arg_out_byte);
    dump(prefix + "arg_out_int16", reply.arg_out_int16);
    dump(prefix + "arg_out_uint16", reply.arg_out_uint16);
    dump(prefix + "arg_out_int32", reply.arg_out_int32);
    dump(prefix + "arg_out_uint32", reply.arg_out_uint32);
    dump(prefix + "arg_out_int64", reply.arg_out_int64);
    dump(prefix + "arg_out_uint64", reply.arg_out_uint64);
    dump(prefix + "arg_out_double", reply.arg_out_double);
    dump(prefix + "arg_out_string", reply.arg_out_string);
    dump(prefix + "arg_out_signature", reply.arg_out_signature);
    dump(prefix + "arg_out_objpath", reply.arg_out_objpath);
    dump(prefix + "arg_out_variant", reply.arg_out_variant);
    dump(prefix + "arg_out_struct", reply.arg_out_struct);
}

template <> void dump<AllArraysTypeDescription::StructNested>(const string& prefix,
                                                              const AllArraysTypeDescription::StructNested& sn)
{
    dump(prefix + ".member_int32", sn.member_int32);
}

template <> void dump<AllArraysProxy::Properties>(const string& prefix,
                                                  const AllArraysProxy::Properties& aapp)
{
    dump(prefix + "prop_array_of_boolean", aapp.prop_array_of_boolean);
    dump(prefix + "prop_array_of_byte", aapp.prop_array_of_byte);
    dump(prefix + "prop_array_of_int16", aapp.prop_array_of_int16);
    dump(prefix + "prop_array_of_uint16", aapp.prop_array_of_uint16);
    dump(prefix + "prop_array_of_int32", aapp.prop_array_of_int32);
    dump(prefix + "prop_array_of_uint32", aapp.prop_array_of_uint32);
    dump(prefix + "prop_array_of_int64", aapp.prop_array_of_int64);
    dump(prefix + "prop_array_of_uint64", aapp.prop_array_of_uint64);
    dump(prefix + "prop_array_of_double", aapp.prop_array_of_double);
    dump(prefix + "prop_array_of_string", aapp.prop_array_of_string);
    dump(prefix + "prop_array_of_signature", aapp.prop_array_of_signature);
    dump(prefix + "prop_array_of_objpath", aapp.prop_array_of_objpath);
    dump(prefix + "prop_array_of_struct", aapp.prop_array_of_struct);
    dump(prefix + "prop_array_of_array", aapp.prop_array_of_array);
    dump(prefix + "prop_array_of_dict", aapp.prop_array_of_dict);
}

template <> void dump<AllArraysProxy::SignalWithAllArrays>(const string& prefix,
                                                           const AllArraysProxy::SignalWithAllArrays& swaa)
{
    dump(prefix + "arg_boolean", swaa.arg_boolean);
    dump(prefix + "arg_byte", swaa.arg_byte);
    dump(prefix + "arg_int16", swaa.arg_int16);
    dump(prefix + "arg_uint16", swaa.arg_uint16);
    dump(prefix + "arg_int32", swaa.arg_int32);
    dump(prefix + "arg_uint32", swaa.arg_uint32);
    dump(prefix + "arg_int64", swaa.arg_int64);
    dump(prefix + "arg_uint64", swaa.arg_uint64);
    dump(prefix + "arg_double", swaa.arg_double);
    dump(prefix + "arg_string", swaa.arg_string);
    dump(prefix + "arg_signature", swaa.arg_signature);
    dump(prefix + "arg_objpath", swaa.arg_objpath);
    dump(prefix + "arg_struct", swaa.arg_struct);
    dump(prefix + "arg_array", swaa.arg_array);
    dump(prefix + "arg_dict", swaa.arg_dict);
}

template <> void dump<AllArraysProxy::MethodWithAllArraysInAndOutReply>(const string& prefix,
                                                                        const AllArraysProxy::
                                                                        MethodWithAllArraysInAndOutReply& reply)
{
    dump(prefix + "arg_out_boolean", reply.arg_out_boolean);
    dump(prefix + "arg_out_byte", reply.arg_out_byte);
    dump(prefix + "arg_out_int16", reply.arg_out_int16);
    dump(prefix + "arg_out_uint16", reply.arg_out_uint16);
    dump(prefix + "arg_out_int32", reply.arg_out_int32);
    dump(prefix + "arg_out_uint32", reply.arg_out_uint32);
    dump(prefix + "arg_out_int64", reply.arg_out_int64);
    dump(prefix + "arg_out_uint64", reply.arg_out_uint64);
    dump(prefix + "arg_out_double", reply.arg_out_double);
    dump(prefix + "arg_out_string", reply.arg_out_string);
    dump(prefix + "arg_out_signature", reply.arg_out_signature);
    dump(prefix + "arg_out_objpath", reply.arg_out_objpath);
    dump(prefix + "arg_out_struct", reply.arg_out_struct);
    dump(prefix + "arg_out_array", reply.arg_out_array);
    dump(prefix + "arg_out_dict", reply.arg_out_dict);
}

template <> void dump<AllDictionariesTypeDescription::StructNested>(const string& prefix,
                                                                    const AllDictionariesTypeDescription::StructNested&
                                                                    sn)
{
    dump(prefix + ".member_int32", sn.member_int32);
}

template <> void dump<AllDictionariesProxy::Properties>(const string& prefix,
                                                        const AllDictionariesProxy::Properties& adpp)
{
    dump(prefix + "prop_dict_of_boolean", adpp.prop_dict_of_boolean);
    dump(prefix + "prop_dict_of_byte", adpp.prop_dict_of_byte);
    dump(prefix + "prop_dict_of_int16", adpp.prop_dict_of_int16);
    dump(prefix + "prop_dict_of_uint16", adpp.prop_dict_of_uint16);
    dump(prefix + "prop_dict_of_int32", adpp.prop_dict_of_int32);
    dump(prefix + "prop_dict_of_uint32", adpp.prop_dict_of_uint32);
    dump(prefix + "prop_dict_of_int64", adpp.prop_dict_of_int64);
    dump(prefix + "prop_dict_of_uint64", adpp.prop_dict_of_uint64);
    dump(prefix + "prop_dict_of_double", adpp.prop_dict_of_double);
    dump(prefix + "prop_dict_of_string", adpp.prop_dict_of_string);
    dump(prefix + "prop_dict_of_signature", adpp.prop_dict_of_signature);
    dump(prefix + "prop_dict_of_objpath", adpp.prop_dict_of_objpath);
    dump(prefix + "prop_dict_of_struct", adpp.prop_dict_of_struct);
    dump(prefix + "prop_dict_of_array", adpp.prop_dict_of_array);
    dump(prefix + "prop_dict_of_dict", adpp.prop_dict_of_dict);
}

template <> void dump<AllDictionariesProxy::SignalWithAllDictionaries>(const string& prefix,
                                                                       const AllDictionariesProxy::
                                                                       SignalWithAllDictionaries& swad)
{
    dump(prefix + "arg_boolean", swad.arg_boolean);
    dump(prefix + "arg_byte", swad.arg_byte);
    dump(prefix + "arg_int16", swad.arg_int16);
    dump(prefix + "arg_uint16", swad.arg_uint16);
    dump(prefix + "arg_int32", swad.arg_int32);
    dump(prefix + "arg_uint32", swad.arg_uint32);
    dump(prefix + "arg_int64", swad.arg_int64);
    dump(prefix + "arg_uint64", swad.arg_uint64);
    dump(prefix + "arg_double", swad.arg_double);
    dump(prefix + "arg_string", swad.arg_string);
    dump(prefix + "arg_signature", swad.arg_signature);
    dump(prefix + "arg_objpath", swad.arg_objpath);
    dump(prefix + "arg_struct", swad.arg_struct);
    dump(prefix + "arg_array", swad.arg_array);
    dump(prefix + "arg_dict", swad.arg_dict);
}

template <> void dump<AllDictionariesProxy::MethodWithAllDictionariesInAndOutReply>(
    const string& prefix,
    const AllDictionariesProxy::
    MethodWithAllDictionariesInAndOutReply
    & reply)
{
    dump(prefix + "arg_out_boolean", reply.arg_out_boolean);
    dump(prefix + "arg_out_byte", reply.arg_out_byte);
    dump(prefix + "arg_out_int16", reply.arg_out_int16);
    dump(prefix + "arg_out_uint16", reply.arg_out_uint16);
    dump(prefix + "arg_out_int32", reply.arg_out_int32);
    dump(prefix + "arg_out_uint32", reply.arg_out_uint32);
    dump(prefix + "arg_out_int64", reply.arg_out_int64);
    dump(prefix + "arg_out_uint64", reply.arg_out_uint64);
    dump(prefix + "arg_out_double", reply.arg_out_double);
    dump(prefix + "arg_out_string", reply.arg_out_string);
    dump(prefix + "arg_out_signature", reply.arg_out_signature);
    dump(prefix + "arg_out_objpath", reply.arg_out_objpath);
    dump(prefix + "arg_out_struct", reply.arg_out_struct);
    dump(prefix + "arg_out_array", reply.arg_out_array);
    dump(prefix + "arg_out_dict", reply.arg_out_dict);
}
};
