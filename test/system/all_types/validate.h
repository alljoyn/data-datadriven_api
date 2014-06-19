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

#ifndef VALIDATE_H_
#define VALIDATE_H_

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
template <typename T> void validate(const std::vector<T>& v,
                                    size_t nelem,
                                    long long& cnt);

template <> void validate<bool>(const std::vector<bool>& v,
                                size_t nelem,
                                long long& cnt);

template <typename K, typename V> void validate(const std::map<const K, V>& m,
                                                size_t nelem,
                                                long long& cnt);

template <> void validate<bool, bool>(const std::map<const bool, bool>& m,
                                      size_t nelem,
                                      long long& cnt);

template <typename T> void validate(const T& t, size_t nelem, long long& cnt)
{
    assert(++cnt == (long long)t);
}

template <> void validate<bool>(const bool& b,
                                size_t nelem,
                                long long& cnt);

template <> void validate<qcc::String>(const qcc::String& s,
                                       size_t nelem,
                                       long long& cnt);

template <> void validate<datadriven::Signature>(const datadriven::Signature& s,
                                                 size_t nelem,
                                                 long long& cnt);

template <> void validate<datadriven::ObjectPath>(const datadriven::ObjectPath& o,
                                                  size_t nelem,
                                                  long long& cnt);

template <> void validate<AllTypesTypeDescription::StructWithAllTypes>(
    const AllTypesTypeDescription::StructWithAllTypes& swat,
    size_t nelem,
    long long& cnt);

template <> void validate<AllTypesTypeDescription::StructNested>(const AllTypesTypeDescription::StructNested& sn,
                                                                 size_t nelem,
                                                                 long long& cnt);

template <> void validate<AllArraysTypeDescription::StructNested>(const AllArraysTypeDescription::StructNested& sn,
                                                                  size_t nelem,
                                                                  long long& cnt);

template <> void validate<AllDictionariesTypeDescription::StructNested>(
    const AllDictionariesTypeDescription::StructNested& sn,
    size_t nelem,
    long long& cnt);

template <typename T> void validate(const std::vector<T>& v, size_t nelem, long long& cnt)
{
    assert(nelem == v.size());
    for (size_t i = 0; i < nelem; i++) {
        validate(v[i], nelem, cnt);
    }
}

template <typename K, typename V> void validate(const std::map<const K, V>& m, size_t nelem, long long& cnt)
{
    assert(nelem == m.size());
    for (typename std::map<const K, V>::const_iterator it = m.begin(); it != m.end(); ++it) {
        long long old_cnt = cnt;

        validate(it->first, nelem, cnt);
        validate(it->second, nelem, old_cnt);
    }
}

void validate(const AllTypesProxy::Properties& atpp,
              size_t nelem);

void validate(const AllTypesProxy::SignalWithAllTypes& signal,
              size_t nelem);

void validate(const AllTypesProxy::MethodWithAllTypesInAndOutReply& reply,
              size_t nelem);

void validate(const AllArraysProxy::Properties& atpp,
              size_t nelem);

void validate(const AllArraysProxy::SignalWithAllArrays& signal,
              size_t nelem);

void validate(const AllArraysProxy::MethodWithAllArraysInAndOutReply& reply,
              size_t nelem,
              long long& cnt);

void validate(const AllDictionariesProxy::Properties& atpp,
              size_t nelem);

void validate(const AllDictionariesProxy::SignalWithAllDictionaries& signal,
              size_t nelem);

void validate(const AllDictionariesProxy::MethodWithAllDictionariesInAndOutReply& reply,
              size_t nelem,
              long long& cnt);
};

#endif /* VALIDATE_H_ */
