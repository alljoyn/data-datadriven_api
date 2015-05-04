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

#include <alljoyn/BusAttachment.h>

#include <datadriven/ObjectId.h>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
using namespace ajn;

ObjectId::ObjectId(BusAttachment& _busAttachment,
                   const qcc::String& _busName,
                   const qcc::String& _busObjectPath,
                   ajn::SessionId _sessionId) :
    busAttachment(_busAttachment), busName(_busName), busObjectPath(_busObjectPath), sessionId(_sessionId)
{
}

ObjectId::ObjectId(BusAttachment& _busAttachment,
                   const ajn::Message& message) :
    busAttachment(_busAttachment), busName(message->GetSender()), busObjectPath(message->GetObjectPath()),
    sessionId(message->GetSessionId())
{
}

const qcc::String& ObjectId::GetBusName() const
{
    return busName;
}

/** Get busobject path */
const qcc::String& ObjectId::GetBusObjectPath() const
{
    return busObjectPath;
}

/** Get session id */
const ajn::SessionId& ObjectId::GetSessionId() const
{
    return sessionId;
}

ajn::ProxyBusObject ObjectId::MakeProxyBusObject() const
{
    return ajn::ProxyBusObject(busAttachment, busName.c_str(), busObjectPath.c_str(), sessionId);
}

std::ostream& operator<<(std::ostream& out, const ObjectId& objId)
{
    out << "ObjectId(" <<
        "bus name:" << objId.GetBusName().c_str() <<
        ", object path:" << objId.GetBusObjectPath().c_str() <<
        ")";
    return out;
}
}
