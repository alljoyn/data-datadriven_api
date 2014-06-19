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

#ifndef PROXYINTERFACE_H_
#define PROXYINTERFACE_H_

#include <datadriven/ObjectId.h>
#include <qcc/Debug.h>
#include <alljoyn/Status.h>
#include <alljoyn/ProxyBusObject.h>
#include <qcc/Mutex.h>
#define QCC_MODULE "DD_CONSUMER"

namespace ajn {
class ProxyBusObject;
}

namespace datadriven {
class TypeDescription;
class RegisteredTypeDescription;

/**
 * \class ProxyInterface
 * \brief Abstract base class for all generated consumer-side AllJoyn interface
 *        classes.
 *
 * You should never interact with an object of this class directly, always use
 * the subclasses created by the code generator. The only methods in this class
 * that are relevant for application developers are ProxyInterface::GetObjectId
 * and ProxyInterface::IsAlive.
 *
 * For an AllJoyn interface Foo, the code generator will create a class
 * FooProxy that is derived from ProxyInterface. Instances of FooProxy are
 * created and managed by an instance of Observer<FooProxy>. All interactions
 * with remote objects implementing interface Foo should go through FooProxy
 * and Observer<FooProxy>.
 */
class ProxyInterface {
  public:
    /**
     * Get the unique identifier (ObjectId) for this object.
     *
     * \see ObjectId
     * \see Observer
     *
     * \return the object id
     */
    const ObjectId& GetObjectId() const;

    /** \private
     * \brief Get status of the proxy object. This refers to whether the object was
     *        successfully constructed.
     *
     * This method should not be used by application code: you can only get
     * proxy objects from some Observer, and that Observer will make sure you
     * only get objects for which status == ER_OK.
     *
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus GetStatus() const;

    /**
     * \brief Check whether the remote object is still alive.
     *
     * While you are holding a reference to a proxy object, its remote
     * counterpart may already have been removed from the AllJoyn bus by the
     * provider. This method will tell you whether, to the best of the
     * Observer's knowledge, the remote object is still exposed on the bus.
     *
     * \note As we are operating in a distributed system, there is always a
     * small time window in which the remote object has already been removed
     * from the bus by its provider, but our consumer-side Observer has not
     * been notified of this. As such, you should be aware that method calls to
     * a remote object can fail even if IsAlive() returns true.
     *
     * \retval true if still in use
     * \retval false if already removed by the provider
     */
    bool IsAlive() const;

    /**
     * \brief Destructor
     */
    virtual ~ProxyInterface();

  protected:
    /** \private
     * Constructor
     */
    ProxyInterface(const RegisteredTypeDescription& desc,
                   const ObjectId& objId);
    /** \private */
    const RegisteredTypeDescription& GetTypeDescription() const;

  private:
    QStatus status;
    const RegisteredTypeDescription& desc;
    ObjectId objId;
    ajn::ProxyBusObject proxyBusObject;
    bool alive;   /* alternatively we could retrieve this from the readercache */
    mutable qcc::Mutex mutex;

    template <typename> friend class MethodInvocation;
    friend class ObserverCache;

    /** \internal Observer will call this function to mark the object as alive/dead */
    void SetAlive(bool _alive);

    /** \internal Needed by MethodInvocation to to perform methodcall at AJN level */
    const ajn::ProxyBusObject& GetProxyBusObject() const;

    /** A proxy interface should not be copyable **/
    ProxyInterface(const ProxyInterface&);
    void operator=(const ProxyInterface&);
};
}
#undef QCC_MODULE
#endif /* PROXYINTERFACE_H_ */
