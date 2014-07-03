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

#include <datadriven/Marshal.h>

namespace datadriven {
const ajn::MsgArg& MsgArgDereference(const ajn::MsgArg& msgarg)
{
    return (ajn::ALLJOYN_VARIANT == msgarg.typeId ? *msgarg.v_variant.val : msgarg);
}

QStatus MarshalArray(ajn::MsgArg& msgarg,
                     ajn::MsgArg* elements,
                     size_t numElements)
{
    QStatus status = ER_OK;

    msgarg.typeId = ajn::ALLJOYN_ARRAY;
    status = msgarg.v_array.SetElements(elements[0].Signature().c_str(), numElements,
                                        numElements ? elements : NULL);
    if (ER_OK == status) {
        msgarg.SetOwnershipFlags(ajn::MsgArg::OwnsArgs);
        msgarg.Stabilize();
    }
    return status;
}

QStatus MarshalStruct(ajn::MsgArg& msgarg,
                      ajn::MsgArg* members,
                      size_t numMembers)
{
    QStatus status = ER_OK;

    msgarg.typeId = ajn::ALLJOYN_STRUCT;
    msgarg.v_struct.numMembers = numMembers;
    msgarg.v_struct.members = members;
    msgarg.SetOwnershipFlags(ajn::MsgArg::OwnsArgs);
    msgarg.Stabilize();
    return status;
}

QStatus MarshalDictEntry(ajn::MsgArg& msgarg,
                         ajn::MsgArg* key,
                         ajn::MsgArg* val)
{
    QStatus status = ER_OK;

    msgarg.typeId = ajn::ALLJOYN_DICT_ENTRY;
    msgarg.v_dictEntry.key = key;
    msgarg.v_dictEntry.val = val;
    msgarg.SetOwnershipFlags(ajn::MsgArg::OwnsArgs);
    msgarg.Stabilize();
    return status;
}

QStatus Marshal(ajn::MsgArg& msgarg, bool data)
{
    msgarg = ajn::ALLJOYN_BOOLEAN;
    msgarg.v_bool = data;
    return ER_OK;
}

QStatus Marshal(ajn::MsgArg& msgarg, uint8_t data)
{
    msgarg = ajn::ALLJOYN_BYTE;
    msgarg.v_byte = data;
    return ER_OK;
}

QStatus Marshal(ajn::MsgArg& msgarg, int16_t data)
{
    msgarg = ajn::ALLJOYN_INT16;
    msgarg.v_int16 = data;
    return ER_OK;
}

QStatus Marshal(ajn::MsgArg& msgarg, uint16_t data)
{
    msgarg = ajn::ALLJOYN_UINT16;
    msgarg.v_uint16 = data;
    return ER_OK;
}

QStatus Marshal(ajn::MsgArg& msgarg, int32_t data)
{
    msgarg = ajn::ALLJOYN_INT32;
    msgarg.v_int32 = data;
    return ER_OK;
}

QStatus Marshal(ajn::MsgArg& msgarg, uint32_t data)
{
    msgarg = ajn::ALLJOYN_UINT32;
    msgarg.v_uint32 = data;
    return ER_OK;
}

QStatus Marshal(ajn::MsgArg& msgarg, int64_t data)
{
    msgarg = ajn::ALLJOYN_INT64;
    msgarg.v_int64 = data;
    return ER_OK;
}

QStatus Marshal(ajn::MsgArg& msgarg, uint64_t data)
{
    msgarg = ajn::ALLJOYN_UINT64;
    msgarg.v_uint64 = data;
    return ER_OK;
}

QStatus Marshal(ajn::MsgArg& msgarg, double data)
{
    msgarg = ajn::ALLJOYN_DOUBLE;
    msgarg.v_double = data;
    return ER_OK;
}

QStatus Marshal(ajn::MsgArg& msgarg, const qcc::String& data)
{
    msgarg.typeId = ajn::ALLJOYN_STRING;
    msgarg.v_string.str = data.c_str();
    msgarg.v_string.len = msgarg.v_string.str ? strlen(msgarg.v_string.str) : 0;
    return ER_OK;
}

QStatus Marshal(ajn::MsgArg& msgarg, const Signature& data)
{
    return msgarg.Set("g", data.c_str());
}

QStatus Marshal(ajn::MsgArg& msgarg, const ObjectPath& data)
{
    return msgarg.Set("o", data.c_str());
}

QStatus Marshal(ajn::MsgArg& msgarg, const ajn::MsgArg& data)
{
    QStatus status = msgarg.Set("v", data.v_variant.val);
    if (ER_OK == status) {
        msgarg.Stabilize();
    }
    return status;
}

QStatus Unmarshal(bool& data, const ajn::MsgArg& msgarg)
{
    if (ajn::ALLJOYN_BOOLEAN != msgarg.typeId) {
        return ER_FAIL;
    } else {
        data = msgarg.v_bool;
        return ER_OK;
    }
}

QStatus Unmarshal(uint8_t& data, const ajn::MsgArg& msgarg)
{
    if (ajn::ALLJOYN_BYTE != msgarg.typeId) {
        return ER_FAIL;
    } else {
        data = msgarg.v_byte;
        return ER_OK;
    }
}

QStatus Unmarshal(int16_t& data, const ajn::MsgArg& msgarg)
{
    if (ajn::ALLJOYN_INT16 != msgarg.typeId) {
        return ER_FAIL;
    } else {
        data = msgarg.v_int16;
        return ER_OK;
    }
}

QStatus Unmarshal(uint16_t& data, const ajn::MsgArg& msgarg)
{
    if (ajn::ALLJOYN_UINT16 != msgarg.typeId) {
        return ER_FAIL;
    } else {
        data = msgarg.v_uint16;
        return ER_OK;
    }
}

QStatus Unmarshal(int32_t& data, const ajn::MsgArg& msgarg)
{
    if (ajn::ALLJOYN_INT32 != msgarg.typeId) {
        return ER_FAIL;
    } else {
        data = msgarg.v_int32;
        return ER_OK;
    }
}

QStatus Unmarshal(uint32_t& data, const ajn::MsgArg& msgarg)
{
    if (ajn::ALLJOYN_UINT32 != msgarg.typeId) {
        return ER_FAIL;
    } else {
        data = msgarg.v_uint32;
        return ER_OK;
    }
}

QStatus Unmarshal(int64_t& data, const ajn::MsgArg& msgarg)
{
    if (ajn::ALLJOYN_INT64 != msgarg.typeId) {
        return ER_FAIL;
    } else {
        data = msgarg.v_int64;
        return ER_OK;
    }
}

QStatus Unmarshal(uint64_t& data, const ajn::MsgArg& msgarg)
{
    if (ajn::ALLJOYN_UINT64 != msgarg.typeId) {
        return ER_FAIL;
    } else {
        data = msgarg.v_uint64;
        return ER_OK;
    }
}

QStatus Unmarshal(double& data, const ajn::MsgArg& msgarg)
{
    if (ajn::ALLJOYN_DOUBLE != msgarg.typeId) {
        return ER_FAIL;
    } else {
        data = msgarg.v_double;
        return ER_OK;
    }
}

QStatus Unmarshal(qcc::String& data, const ajn::MsgArg& msgarg)
{
    if (ajn::ALLJOYN_STRING != msgarg.typeId) {
        return ER_FAIL;
    } else {
        data.assign(msgarg.v_string.str, msgarg.v_string.len);
        return ER_OK;
    }
}

QStatus Unmarshal(Signature& data, const ajn::MsgArg& msgarg)
{
    if (ajn::ALLJOYN_SIGNATURE != msgarg.typeId) {
        return ER_FAIL;
    } else {
        data.assign(msgarg.v_signature.sig, msgarg.v_signature.len);
        return ER_OK;
    }
}

QStatus Unmarshal(ObjectPath& data, const ajn::MsgArg& msgarg)
{
    if (ajn::ALLJOYN_OBJECT_PATH != msgarg.typeId) {
        return ER_FAIL;
    } else {
        data.assign(msgarg.v_objPath.str, msgarg.v_objPath.len);
        return ER_OK;
    }
}

QStatus Unmarshal(ajn::MsgArg& data, const ajn::MsgArg& msgarg)
{
    if (ajn::ALLJOYN_VARIANT != msgarg.typeId) {
        return ER_FAIL;
    } else {
        data = msgarg;
        return ER_OK;
    }
}

template <> QStatus Unmarshal<bool>(std::vector<bool>& data, const ajn::MsgArg& msgarg)
{
    QStatus status = ER_FAIL;

    if (ajn::ALLJOYN_BOOLEAN_ARRAY == msgarg.typeId) {
        data.clear();
        data.insert(data.begin(), msgarg.v_scalarArray.v_bool,
                    msgarg.v_scalarArray.v_bool + msgarg.v_scalarArray.numElements);
        status = ER_OK;
    }
    return status;
}

template <> QStatus Unmarshal<uint8_t>(std::vector<uint8_t>& data, const ajn::MsgArg& msgarg)
{
    QStatus status = ER_FAIL;

    if (ajn::ALLJOYN_BYTE_ARRAY == msgarg.typeId) {
        data.clear();
        data.insert(data.begin(), msgarg.v_scalarArray.v_byte,
                    msgarg.v_scalarArray.v_byte + msgarg.v_scalarArray.numElements);
        status = ER_OK;
    }
    return status;
}

template <> QStatus Unmarshal<int16_t>(std::vector<int16_t>& data, const ajn::MsgArg& msgarg)
{
    QStatus status = ER_FAIL;

    if (ajn::ALLJOYN_INT16_ARRAY == msgarg.typeId) {
        data.clear();
        data.insert(data.begin(), msgarg.v_scalarArray.v_int16,
                    msgarg.v_scalarArray.v_int16 + msgarg.v_scalarArray.numElements);
        status = ER_OK;
    }
    return status;
}

template <> QStatus Unmarshal<uint16_t>(std::vector<uint16_t>& data, const ajn::MsgArg& msgarg)
{
    QStatus status = ER_FAIL;

    if (ajn::ALLJOYN_UINT16_ARRAY == msgarg.typeId) {
        data.clear();
        data.insert(data.begin(), msgarg.v_scalarArray.v_uint16,
                    msgarg.v_scalarArray.v_uint16 + msgarg.v_scalarArray.numElements);
        status = ER_OK;
    }
    return status;
}

template <> QStatus Unmarshal<int32_t>(std::vector<int32_t>& data, const ajn::MsgArg& msgarg)
{
    QStatus status = ER_FAIL;

    if (ajn::ALLJOYN_INT32_ARRAY == msgarg.typeId) {
        data.clear();
        data.insert(data.begin(), msgarg.v_scalarArray.v_int32,
                    msgarg.v_scalarArray.v_int32 + msgarg.v_scalarArray.numElements);
        status = ER_OK;
    }
    return status;
}

template <> QStatus Unmarshal<uint32_t>(std::vector<uint32_t>& data, const ajn::MsgArg& msgarg)
{
    QStatus status = ER_FAIL;

    if (ajn::ALLJOYN_UINT32_ARRAY == msgarg.typeId) {
        data.clear();
        data.insert(data.begin(), msgarg.v_scalarArray.v_uint32,
                    msgarg.v_scalarArray.v_uint32 + msgarg.v_scalarArray.numElements);
        status = ER_OK;
    }
    return status;
}

template <> QStatus Unmarshal<int64_t>(std::vector<int64_t>& data, const ajn::MsgArg& msgarg)
{
    QStatus status = ER_FAIL;

    if (ajn::ALLJOYN_INT64_ARRAY == msgarg.typeId) {
        data.clear();
        data.insert(data.begin(), msgarg.v_scalarArray.v_int64,
                    msgarg.v_scalarArray.v_int64 + msgarg.v_scalarArray.numElements);
        status = ER_OK;
    }
    return status;
}

template <> QStatus Unmarshal<uint64_t>(std::vector<uint64_t>& data, const ajn::MsgArg& msgarg)
{
    QStatus status = ER_FAIL;

    if (ajn::ALLJOYN_UINT64_ARRAY == msgarg.typeId) {
        data.clear();
        data.insert(data.begin(), msgarg.v_scalarArray.v_uint64,
                    msgarg.v_scalarArray.v_uint64 + msgarg.v_scalarArray.numElements);
        status = ER_OK;
    }
    return status;
}

template <> QStatus Unmarshal<double>(std::vector<double>& data, const ajn::MsgArg& msgarg)
{
    QStatus status = ER_FAIL;

    if (ajn::ALLJOYN_DOUBLE_ARRAY == msgarg.typeId) {
        data.clear();
        data.insert(data.begin(), msgarg.v_scalarArray.v_double,
                    msgarg.v_scalarArray.v_double + msgarg.v_scalarArray.numElements);
        status = ER_OK;
    }
    return status;
}
}
