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

#ifndef BUSCONNECTION_H_
#define BUSCONNECTION_H_

#include <memory>

#include <alljoyn/Status.h>

namespace test_unit_sessionmanager { class BasicTestObject; }

namespace datadriven {
class BusConnectionImpl;

/**
 * \class BusConnection
 * \brief Connection to the AllJoyn communication bus.
 *
 * This class maintains the connection with the AllJoyn bus. It serves as the
 * main entry point to the Data-driven API. You need to create an object of
 * this class before you can create a ProvidedObject or an Observer.
 */
class BusConnection {
  public:
    /**
     * Initializes the object and sets up a connection to the AllJoyn bus.
     *
     * \note After creation, the application developer should check whether the
     *       connection to the AllJoyn bus was successfully set up by calling
     *       BusConnection::GetStatus.
     */
    BusConnection();

    /**
     * Disconnects from the AllJoyn bus and performs cleanup.
     *
     * \note Any Observer or ProvidedObject instances that were created for
     *       this BusConnection instance are obviously no longer usable after
     *       the BusConnection is destroyed.
     */
    virtual ~BusConnection();

    /**
     * \brief Retrieves the BusConnection's <em>integrity state</em>.
     *
     * The integrity state of the BusConnection indicates whether the
     * connection to the AllJoyn bus has succeeded.
     *
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus GetStatus() const;

  private:
    BusConnection(const BusConnection&);
    void operator=(const BusConnection&);

  private:
    friend class ObserverImpl;
    friend class SignalListenerImpl;
    friend class ProvidedInterface;
    friend class ProvidedObject;
    friend class test_unit_sessionmanager::BasicTestObject;
    std::shared_ptr<BusConnectionImpl> busConnectionImpl;
};
}

#endif /* BUSCONNECTION_H_ */
