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

#include "init_data.h"
#include <cstdio>

namespace test_system_alltypes {
template <> void init_data<bool>(bool& b,
                                 size_t nelem,
                                 long long& cnt)
{
    b = (++cnt % 2 ? true : false);
}

template <> void init_data<qcc::String>(qcc::String& s,
                                        size_t nelem,
                                        long long& cnt)
{
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "%lld", ++cnt);
    s = qcc::String(buffer);
}

template <> void init_data<datadriven::Signature>(datadriven::Signature& s,
                                                  size_t nelem,
                                                  long long& cnt)
{
    s = std::string((size_t)++ cnt, 'i').c_str();
}

template <> void init_data<datadriven::ObjectPath>(datadriven::ObjectPath& o,
                                                   size_t nelem,
                                                   long long& cnt)
{
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "/path/%lld", ++cnt);
    o = qcc::String(buffer);
}

template <> void init_data<ajn::MsgArg>(ajn::MsgArg& v,
                                        size_t nelem,
                                        long long& cnt)
{
    ajn::MsgArg m(ajn::ALLJOYN_DOUBLE);
    m.v_double = ++cnt;

    v.Set("v", &m);
    v.Stabilize();
}

template <> void init_data<AllTypesTypeDescription::StructWithAllTypes>(
    AllTypesTypeDescription::StructWithAllTypes& swat,
    size_t nelem,
    long long& cnt)
{
    init_data(swat.member_boolean, nelem, cnt);
    init_data(swat.member_byte, nelem, cnt);
    init_data(swat.member_int16, nelem, cnt);
    init_data(swat.member_uint16, nelem, cnt);
    init_data(swat.member_int32, nelem, cnt);
    init_data(swat.member_uint32, nelem, cnt);
    init_data(swat.member_int64, nelem, cnt);
    init_data(swat.member_uint64, nelem, cnt);
    init_data(swat.member_double, nelem, cnt);
    init_data(swat.member_string, nelem, cnt);
    init_data(swat.member_signature, nelem, cnt);
    init_data(swat.member_objpath, nelem, cnt);
    init_data(swat.member_variant, nelem, cnt);
    init_data(swat.member_struct, nelem, cnt);
    init_data(swat.member_array, nelem, cnt);
    init_data(swat.member_dict, nelem, cnt);
}

template <> void init_data<AllTypesTypeDescription::StructNested>(AllTypesTypeDescription::StructNested& sn,
                                                                  size_t nelem,
                                                                  long long& cnt)
{
    sn.member_int32 = ++cnt;
}

template <> void init_data<AllArraysTypeDescription::StructNested>(AllArraysTypeDescription::StructNested& sn,
                                                                   size_t nelem,
                                                                   long long& cnt)
{
    sn.member_int32 = ++cnt;
}

template <> void init_data<AllDictionariesTypeDescription::StructNested>(
    AllDictionariesTypeDescription::StructNested& sn,
    size_t nelem,
    long long& cnt)
{
    sn.member_int32 = ++cnt;
}

template <> void init_data<bool>(std::vector<bool>& v,
                                 size_t nelem,
                                 long long& cnt)
{
    bool b = true;

    v.resize(nelem);
    for (size_t i = 0; i < nelem; i++) {
        v[i] = b;
        b = !b;
    }
}

template <> void init_data<bool>(std::map<const bool, bool>& m,
                                 size_t nelem,
                                 long long& cnt)
{
    bool b = true;

    // maximum two values in map with boolean keys
    nelem = nelem > 2 ? 2 : nelem;
    for (size_t i = 0; i < nelem; i++) {
        m[b] = b;
        b = !b;
    }
}
};
