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

#ifndef INIT_DATA_H_
#define INIT_DATA_H_

#include <map>
#include <vector>

// provider
#include "AllTypes.h"
// consumer
#include "AllTypesProxy.h"
#include "AllArraysProxy.h"
#include "AllDictionariesProxy.h"

namespace test_system_alltypes {
using namespace std;
using namespace::gen::org_allseenalliance_test;

// forward
template <typename T> void init_data(std::vector<T>& v,
                                     size_t nelem,
                                     long long& cnt);

template <> void init_data<bool>(std::vector<bool>& v,
                                 size_t nelem,
                                 long long& cnt);

template <typename K, typename V> void init_data(std::map<const K, V>& m,
                                                 size_t nelem,
                                                 long long& cnt);

template <> void init_data<bool>(std::map<const bool, bool>& m,
                                 size_t nelem,
                                 long long& cnt);

template <typename T> void init_data(T& t, size_t nelem, long long& cnt)
{
    t = ++cnt;
}

template <> void init_data<bool>(bool& b,
                                 size_t nelem,
                                 long long& cnt);

template <> void init_data<qcc::String>(qcc::String& s,
                                        size_t nelem,
                                        long long& cnt);

template <> void init_data<datadriven::Signature>(datadriven::Signature& s,
                                                  size_t nelem,
                                                  long long& cnt);

template <> void init_data<datadriven::ObjectPath>(datadriven::ObjectPath& o,
                                                   size_t nelem,
                                                   long long& cnt);

template <> void init_data<AllTypesTypeDescription::StructWithAllTypes>(
    AllTypesTypeDescription::StructWithAllTypes& swat,
    size_t nelem,
    long long& cnt);

template <> void init_data<AllTypesTypeDescription::StructNested>(AllTypesTypeDescription::StructNested& sn,
                                                                  size_t nelem,
                                                                  long long& cnt);

template <> void init_data<AllArraysTypeDescription::StructNested>(AllArraysTypeDescription::StructNested& sn,
                                                                   size_t nelem,
                                                                   long long& cnt);

template <> void init_data<AllDictionariesTypeDescription::StructNested>(
    AllDictionariesTypeDescription::StructNested& sn,
    size_t nelem,
    long long& cnt);

template <typename T> void init_data(std::vector<T>& v, size_t nelem, long long& cnt)
{
    v.resize(nelem);
    for (size_t i = 0; i < nelem; i++) {
        init_data(v[i], nelem, cnt);
    }
}

template <typename K, typename V> void init_data(std::map<const K, V>& m, size_t nelem, long long& cnt)
{
    for (size_t i = 0; i < nelem; i++) {
        long long old_cnt = cnt;
        K k;
        V v;

        init_data(k, nelem, cnt);
        init_data(v, nelem, old_cnt);
        m[k] = v;
    }
}

void init_data(AllTypes& at,
               size_t nelem);
};

#endif /* INIT_DATA_H_ */
