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

#ifndef OBJECTID_H_
#define OBJECTID_H_

#include <ostream>

#include <qcc/String.h>

#include <alljoyn/Session.h>
#include <alljoyn/Message.h>
#include <alljoyn/ProxyBusObject.h>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
/**
 * \class ObjectId
 * \brief Unique identity for each object that is discovered on the AllJoyn
 * bus.
 *
 * In AllJoyn terms, a bus object is uniquely identified by its object path in
 * combination with the unique bus name of the bus attachment hosting the bus
 * object. In the Data-driven API, you should not really care about these
 * concepts. Instead, you can treat the ObjectId as an opaque identifier.
 */
class ObjectId {
  public:
    /** \private
     * Initializes all members of the object.
     * \param[in] busAttachment Communication layer connection
     * \param[in] busName Name on the communication layer
     * \param[in] busObjectPath Path identifier on the communication layer
     * \param[in] sessionId Session from the communication layer
     */
    ObjectId(ajn::BusAttachment& busAttachment,
             const qcc::String& busName,
             const qcc::String& busObjectPath,
             ajn::SessionId sessionId);

    /** \private
     * Initializes object members from a communication message.
     * \param[in] busAttachment Communication layer connection
     * \param[in] message Underlying communication message
     */
    ObjectId(ajn::BusAttachment& busAttachment,
             const ajn::Message& message);

    /**
     * Get the unique bus name of the ObjectId.
     * \return Bus name.
     */
    const qcc::String& GetBusName() const;

    /**
     * Get the object path of the ObjectId.
     * \return Bus object path
     */
    const qcc::String& GetBusObjectPath() const;

    /**
     * \private
     * Get session identifier.
     * \return Session identifier of the ObjectId.
     */
    const ajn::SessionId& GetSessionId() const;

    /** \private
     * Create a ProxyBusObject corresponding to the ObjectId content
     * \return The created ProxyBusObject
     */
    ajn::ProxyBusObject MakeProxyBusObject() const;

  private:
    ajn::BusAttachment& busAttachment;
    qcc::String busName;
    qcc::String busObjectPath;
    ajn::SessionId sessionId;

    friend class ObserverBase;
};

/** Creates a printable stream of the ObjectId contents */
std::ostream& operator<<(std::ostream& strm,
                         const ObjectId& id);
}

#undef QCC_MODULE
#endif /* OBJECTID_H_ */
