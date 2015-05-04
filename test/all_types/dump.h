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

#ifndef DUMP_H_
#define DUMP_H_

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <datadriven/Signature.h>
#include <datadriven/ObjectPath.h>

// provider
#include "AllTypes.h"
// consumer
#include "AllTypesProxy.h"
#include "AllArraysProxy.h"
#include "AllDictionariesProxy.h"

namespace test_system_alltypes {
using namespace std;
using namespace::gen::org_allseenalliance_test;

void dump(ostringstream& oss);

// T
template <typename T> void dump(const string& prefix,
                                const T& t);

template <> void dump<uint8_t>(const string& prefix,
                               const uint8_t& t);

template <> void dump<qcc::String>(const string& prefix,
                                   const qcc::String& s);

template <> void dump<datadriven::Signature>(const string& prefix,
                                             const datadriven::Signature& s);

template <> void dump<datadriven::ObjectPath>(const string& prefix,
                                              const datadriven::ObjectPath& o);

template <> void dump<AllTypesTypeDescription::StructNested>(const string& prefix,
                                                             const AllTypesTypeDescription::StructNested& sn);

template <> void dump<AllTypesTypeDescription::StructWithAllTypes>(const string& prefix,
                                                                   const AllTypesTypeDescription::StructWithAllTypes&
                                                                   swat);

template <> void dump<AllTypesProxy::Properties>(const string& prefix,
                                                 const AllTypesProxy::Properties& atpp);

template <> void dump<AllTypesProxy::SignalWithAllTypes>(const string& prefix,
                                                         const AllTypesProxy::SignalWithAllTypes& swat);

template <> void dump<AllTypesProxy::MethodWithAllTypesInAndOutReply>(const string& prefix,
                                                                      const AllTypesProxy::
                                                                      MethodWithAllTypesInAndOutReply& reply);

template <> void dump<AllArraysProxy::Properties>(const string& prefix,
                                                  const AllArraysProxy::Properties& atpp);

template <> void dump<AllArraysProxy::SignalWithAllArrays>(const string& prefix,
                                                           const AllArraysProxy::SignalWithAllArrays& swaa);

template <> void dump<AllArraysProxy::MethodWithAllArraysInAndOutReply>(const string& prefix,
                                                                        const AllArraysProxy::
                                                                        MethodWithAllArraysInAndOutReply& reply);

template <> void dump<AllArraysTypeDescription::StructNested>(const string& prefix,
                                                              const AllArraysTypeDescription::StructNested& sn);

template <> void dump<AllDictionariesProxy::Properties>(const string& prefix,
                                                        const AllDictionariesProxy::Properties& atpp);

template <> void dump<AllDictionariesProxy::SignalWithAllDictionaries>(const string& prefix,
                                                                       const AllDictionariesProxy::
                                                                       SignalWithAllDictionaries& swad);

template <> void dump<AllDictionariesProxy::MethodWithAllDictionariesInAndOutReply>(const string& prefix,
                                                                                    const AllDictionariesProxy::
                                                                                    MethodWithAllDictionariesInAndOutReply
                                                                                    & reply);

template <> void dump<AllDictionariesTypeDescription::StructNested>(const string& prefix,
                                                                    const AllDictionariesTypeDescription::StructNested&
                                                                    sn);

// K and V
template <typename K, typename V> void dump(const string& prefix,
                                            const K& k,
                                            const V& v);

template <typename V> void dump(const string& prefix,
                                const uint8_t& k,
                                const V& v);

template <typename V> void dump(const string& prefix,
                                const qcc::String& k,
                                const V& v);

template <typename V> void dump(const string& prefix,
                                const datadriven::Signature& k,
                                const V& v);

template <typename V> void dump(const string& prefix,
                                const datadriven::ObjectPath& k,
                                const V& v);

// std::vector<T>
template <typename T> void dump(const string& prefix,
                                const std::vector<T>& v);

// std::map<const K, V>
template <typename K, typename V> void dump(const string& prefix,
                                            const std::map<const K, V>& m);

// implementations

template <typename T> void dump(const string& prefix, const T& t)
{
    ostringstream oss;

    oss << prefix << " = " << boolalpha << t;
    dump(oss);
}

template <typename K, typename V> void dump(const string& prefix, const K& k, const V& v)
{
    ostringstream oss;

    oss << prefix << boolalpha << "[" << k << "]";
    dump(oss.str(), v);
}

template <typename V> void dump(const string& prefix, const uint8_t& k, const V& v)
{
    ostringstream oss;

    oss << prefix << "[" << (int)k << "]";
    dump(oss.str(), v);
}

template <typename V> void dump(const string& prefix, const qcc::String& k, const V& v)
{
    ostringstream oss;

    oss << prefix << "[" << k.c_str() << "]";
    dump(oss.str(), v);
}

template <typename V> void dump(const string& prefix, const datadriven::Signature& k, const V& v)
{
    ostringstream oss;

    oss << prefix << "[" << k.c_str() << "]";
    dump(oss.str(), v);
}

template <typename V> void dump(const string& prefix, const datadriven::ObjectPath& k, const V& v)
{
    ostringstream oss;

    oss << prefix << "[" << k.c_str() << "]";
    dump(oss.str(), v);
}

template <typename T> void dump(const string& prefix, const std::vector<T>& v)
{
    for (size_t i = 0; i < v.size(); i++) {
        dump(prefix, i, v[i]);
    }
}

template <typename K, typename V> void dump(const string& prefix, const std::map<const K, V>& m)
{
    for (typename std::map<const K, V>::const_iterator it = m.begin(); it != m.end(); ++it) {
        dump(prefix, it->first, it->second);
    }
}
};

#endif /* DUMP_H_ */
