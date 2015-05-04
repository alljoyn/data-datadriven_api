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

#ifndef DATADRIVEN_MARSHAL_H_
#define DATADRIVEN_MARSHAL_H_

#include <map>
#include <vector>
#include <memory>

#include <qcc/String.h>

#include <alljoyn/MsgArg.h>
#include <alljoyn/Status.h>

#include <datadriven/ObjectPath.h>
#include <datadriven/Signature.h>

namespace datadriven {
/**
 * Dereferences the provided MsgArg if it is a variant otherwise returns the
 * argument.
 */
const ajn::MsgArg& MsgArgDereference(const ajn::MsgArg& msgarg);

QStatus MarshalArray(ajn::MsgArg& msgarg,
                     ajn::MsgArg* elements,
                     size_t numElements);

QStatus MarshalStruct(ajn::MsgArg& msgarg,
                      ajn::MsgArg* members,
                      size_t numMembers);

QStatus MarshalDictEntry(ajn::MsgArg& msgarg,
                         ajn::MsgArg* key,
                         ajn::MsgArg* val);

QStatus Marshal(ajn::MsgArg& msgarg,
                bool data);

QStatus Marshal(ajn::MsgArg& msgarg,
                uint8_t data);

QStatus Marshal(ajn::MsgArg& msgarg,
                int16_t data);

QStatus Marshal(ajn::MsgArg& msgarg,
                uint16_t data);

QStatus Marshal(ajn::MsgArg& msgarg,
                int32_t data);

QStatus Marshal(ajn::MsgArg& msgarg,
                uint32_t data);

QStatus Marshal(ajn::MsgArg& msgarg,
                int64_t data);

QStatus Marshal(ajn::MsgArg& msgarg,
                uint64_t data);

QStatus Marshal(ajn::MsgArg& msgarg,
                double data);

QStatus Marshal(ajn::MsgArg& msgarg,
                const qcc::String& data);

QStatus Marshal(ajn::MsgArg& msgarg,
                const Signature& data);

QStatus Marshal(ajn::MsgArg& msgarg,
                const ObjectPath& data);

QStatus Marshal(ajn::MsgArg& msgarg,
                const ajn::MsgArg& data);

template <typename K, typename V> QStatus Marshal(ajn::MsgArg& msgarg,
                                                  const std::map<const K, V>& data);

template <typename T> QStatus Marshal(ajn::MsgArg& msgarg, const std::vector<T>& data)
{
    QStatus status = ER_OK;
    size_t numElements = data.size();
    std::unique_ptr<ajn::MsgArg[]> elements;

    if (numElements > 0) {
        size_t i = 0;

        elements = std::unique_ptr<ajn::MsgArg[]>(new ajn::MsgArg[numElements]);
        for (typename std::vector<T>::const_iterator it = data.begin(); it != data.end(); ++it) {
            if (ER_OK != (status = Marshal(elements[i++], *it))) {
                break;
            }
        }
    } else {
        // determine signature of element
        elements = std::unique_ptr<ajn::MsgArg[]>(new ajn::MsgArg[1]);
        status = Marshal(elements[0], T());
    }
    if (ER_OK == status) {
        status = MarshalArray(msgarg, elements.release(), numElements);
    }
    return status;
}

template <typename K, typename V> QStatus Marshal(ajn::MsgArg& msgarg, const std::map<const K, V>& data)
{
    QStatus status = ER_OK;
    size_t numElements = data.size();
    std::unique_ptr<ajn::MsgArg[]> elements;
    std::unique_ptr<ajn::MsgArg> key;
    std::unique_ptr<ajn::MsgArg> val;

    if (numElements > 0) {
        size_t i = 0;

        elements = std::unique_ptr<ajn::MsgArg[]>(new ajn::MsgArg[numElements]);
        for (typename std::map<K, V>::const_iterator it = data.begin(); it != data.end(); ++it) {
            key = std::unique_ptr<ajn::MsgArg>(new ajn::MsgArg);
            if (ER_OK != (status = Marshal(*key, it->first))) {
                break;
            }
            val = std::unique_ptr<ajn::MsgArg>(new ajn::MsgArg);
            if (ER_OK != (status = Marshal(*val, it->second))) {
                break;
            }
            if (ER_OK != (status = MarshalDictEntry(elements[i], key.release(), val.release()))) {
                break;
            }
            i++;
        }
    } else {
        // determine signature of dictionary
        do {
            elements = std::unique_ptr<ajn::MsgArg[]>(new ajn::MsgArg[1]);
            key = std::unique_ptr<ajn::MsgArg>(new ajn::MsgArg);
            if (ER_OK != (status = Marshal(*key, K()))) {
                break;
            }
            val = std::unique_ptr<ajn::MsgArg>(new ajn::MsgArg);
            if (ER_OK != (status = Marshal(*val, V()))) {
                break;
            }
            if (ER_OK != (status = MarshalDictEntry(elements[0], key.release(), val.release()))) {
                break;
            }
        } while (0);
    }
    if (ER_OK == status) {
        status = MarshalArray(msgarg, elements.release(), numElements);
    }
    return status;
}

QStatus Unmarshal(bool& data,
                  const ajn::MsgArg& msgarg);

QStatus Unmarshal(uint8_t& data,
                  const ajn::MsgArg& msgarg);

QStatus Unmarshal(int16_t& data,
                  const ajn::MsgArg& msgarg);

QStatus Unmarshal(uint16_t& data,
                  const ajn::MsgArg& msgarg);

QStatus Unmarshal(int32_t& data,
                  const ajn::MsgArg& msgarg);

QStatus Unmarshal(uint32_t& data,
                  const ajn::MsgArg& msgarg);

QStatus Unmarshal(int64_t& data,
                  const ajn::MsgArg& msgarg);

QStatus Unmarshal(uint64_t& data,
                  const ajn::MsgArg& msgarg);

QStatus Unmarshal(double& data,
                  const ajn::MsgArg& msgarg);

QStatus Unmarshal(qcc::String& data,
                  const ajn::MsgArg& msgarg);

QStatus Unmarshal(Signature& data,
                  const ajn::MsgArg& msgarg);

QStatus Unmarshal(ObjectPath& data,
                  const ajn::MsgArg& msgarg);

QStatus Unmarshal(ajn::MsgArg& data,
                  const ajn::MsgArg& msgarg);

template <typename K, typename V> QStatus Unmarshal(std::map<const K, V>& data,
                                                    const ajn::MsgArg& msgarg);

template <typename T> QStatus Unmarshal(std::vector<T>& data, const ajn::MsgArg& msgarg)
{
    QStatus status = ER_FAIL;
    const ajn::MsgArg& m = MsgArgDereference(msgarg);

    if (ajn::ALLJOYN_ARRAY == m.typeId) {
        size_t numElements = m.v_array.GetNumElements();
        data.clear();

        if (numElements > 0) {
            const ajn::MsgArg* elements = m.v_array.GetElements();
            std::vector<T> tmp;

            tmp.reserve(numElements);
            for (size_t i = 0; i < numElements; ++i) {
                T t;
                if (ER_OK != (status = Unmarshal(t, elements[i]))) {
                    break;
                }
                tmp.push_back(t);
            }
            if (ER_OK == status) {
                data.swap(tmp);
            }
        } else {
            status = ER_OK;
        }
    }
    return status;
}

template <typename K, typename V> QStatus Unmarshal(std::map<const K, V>& data, const ajn::MsgArg& msgarg)
{
    QStatus status = ER_FAIL;
    const ajn::MsgArg& m = MsgArgDereference(msgarg);

    if (ajn::ALLJOYN_ARRAY == m.typeId) {
        size_t numElements = m.v_array.GetNumElements();
        data.clear();

        if (numElements > 0) {
            const ajn::MsgArg* elements = m.v_array.GetElements();
            std::map<const K, V> tmp;

            for (size_t i = 0; i < numElements; ++i) {
                K k;
                if (ER_OK != (status = Unmarshal(k, *elements[i].v_dictEntry.key))) {
                    break;
                }
                V v;
                if (ER_OK != (status = Unmarshal(v, *elements[i].v_dictEntry.val))) {
                    break;
                }
                tmp[k] = v;
            }
            if (ER_OK == status) {
                data.swap(tmp);
            }
        } else {
            status = ER_OK;
        }
    }
    return status;
}

template <> QStatus Unmarshal<bool>(std::vector<bool>& data,
                                    const ajn::MsgArg& msgarg);

template <> QStatus Unmarshal<uint8_t>(std::vector<uint8_t>& data,
                                       const ajn::MsgArg& msgarg);

template <> QStatus Unmarshal<int16_t>(std::vector<int16_t>& data,
                                       const ajn::MsgArg& msgarg);

template <> QStatus Unmarshal<uint16_t>(std::vector<uint16_t>& data,
                                        const ajn::MsgArg& msgarg);

template <> QStatus Unmarshal<int32_t>(std::vector<int32_t>& data,
                                       const ajn::MsgArg& msgarg);

template <> QStatus Unmarshal<uint32_t>(std::vector<uint32_t>& data,
                                        const ajn::MsgArg& msgarg);

template <> QStatus Unmarshal<int64_t>(std::vector<int64_t>& data,
                                       const ajn::MsgArg& msgarg);

template <> QStatus Unmarshal<uint64_t>(std::vector<uint64_t>& data,
                                        const ajn::MsgArg& msgarg);

template <> QStatus Unmarshal<double>(std::vector<double>& data,
                                      const ajn::MsgArg& msgarg);
}

#endif /* DATADRIVEN_MARSHAL_H_ */
