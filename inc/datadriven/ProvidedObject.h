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

#ifndef PROVIDEDOBJECT_H_
#define PROVIDEDOBJECT_H_

#include <memory>
#include <vector>
#include <alljoyn/BusObject.h>
#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

namespace datadriven {
class BusConnectionImpl;
class BusConnection;
class ProvidedInterface;   // forward

/**
 * \class ProvidedObject
 * \brief Base class for all objects a provider wants to expose on the AllJoyn
 *        bus.
 *
 * You should never directly create an instance of this class. This class is
 * intended to be used as a base class that provides common functionality for
 * all objects a provided exposes on the bus.
 *
 * For example, if a provider wishes to expose an object on the AllJoyn bus
 * that implements two interfaces Foo and Bar, it should create a class
 * MyFooBar that inherits from ProvidedObject, FooInterface (created by the
 * code generator) and BarInterface (created by the code generator). In
 * MyFooBar, the developer must provide implementations for all methods defined
 * in interfaces Foo and Bar.
 *
 * In actual code, you'd get something along these lines:
 * \code
 * class MyFooBar : public ProvidedObject, public FooInterface, public BarInterface {
 *   public:
 *     MyFooBar(BusConnection& conn) :
 *         ProvidedObject(conn),
 *         FooInterface(this),
 *         BarInterface(this)
 *     { ... }
 * }
 * \endcode
 *
 * \note The order of the base classes is significant! ProvidedObject must be
 *       the first base class specified, followed by the xyzInterface classes.
 */

class ProvidedObject :
    public ajn::BusObject {
  public:
    /*
     * The life cycle of a ProvidedObject
     *
     *   CONSTRUCTED ---> REGISTERED <--> REMOVED
     *         \               |            /
     *          \              v           /
     *           -->         ERROR      <--
     */

    /**
     * \enum State
     * Set of states that the ProvidedObject can take during its life cycle.
     */
    enum State {
        CONSTRUCTED, /**< Initial state of the object life cycle, the object has never been exposed on the AllJoyn bus. */
        REGISTERED, /**< The object is exposed on the AllJoyn bus. */
        REMOVED, /**< The object is removed from the AllJoyn bus. */
        ERROR /**< The object is in an error state and cannot be exposed on the AllJoyn bus. */
    };

  public:

    /** Destructor */
    virtual ~ProvidedObject();

    /**
     * \brief Trigger simultaneous update notifications on all the interfaces implemented by this
     *        object.
     * \retval ER_OK on success
     * \retval others on failure
     * \see ProvidedInterface::Update for a more detailed explanation.
     */
    QStatus UpdateAll();

    /**
     * \brief Expose the object on the AllJoyn bus. Wrapper on UpdateAll.
     *
     * \retval ER_OK on success
     * \retval others on failure
     * \see ProvidedObject::UpdateAll for a more detailed explanation.
     */
    QStatus PutOnBus();

    /**
     * \brief Removes the object from the AllJoyn bus.
     */
    void RemoveFromBus();

    /**
     * \brief Get the current life cycle state of the object.
     * \return One of ProvidedObject::State
     */
    ProvidedObject::State GetState();

    /** \private
     * Emits a signal (broadcast) across the communication layer
     * \param[in] signal Member from an interface description on which the signal will be emitted
     * \param[in] args List of input arguments to be taken by the Signal invocation.
     * \param[in] numArgs Number of input arguments.
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus EmitSignal(const ajn::InterfaceDescription::Member& signal,
                       const ajn::MsgArg* args = NULL,
                       size_t numArgs = 0);

    /** \private
     * Used when a reply needs to be send back as a response on an invoked method call
     * \param[in] message Marshaled <em>response content</em> to be send back
     * \param[in] args List of input arguments to be considered when sending the response
     * \param[in] numArgs Number of input arguments.
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus MethodReply(const ajn::Message& message,
                        const ajn::MsgArg* args = NULL,
                        size_t numArgs = 0);

    /** \private Callback handler structure */
    struct MethodCallbacks {
        int memberId;
        MessageReceiver::MethodHandler handler;
        MethodCallbacks(int memberId,
                        MessageReceiver::MethodHandler handler) :
            memberId(memberId), handler(handler) { };
    };

    /** \private
     * Add an interface with its callback listeners to the ProvidedObject
     * \param[in] iface Interface to be added
     * \param[in] callbacks List of method callbacks for the interface required by the underlying communication layer
     * \param[in] numCallbacks Number of callback methods to consider
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus AddProvidedInterface(ProvidedInterface * iface, MethodCallbacks callbacks[], size_t numCallbacks);

    /** \private
     * Retrieves a list of interface names currently present in the ProvidedObject
     * \param[in] out Vector containing the names of the current available interface names
     */
    void GetInterfaceNames(std::vector<qcc::String>& out) const;

    /**
     * \brief Get the object's <em>object path</em>.
     *
     * The <em>object path</em> is an AllJoyn Core concept that you should not
     * really care about when you use the Data-driven API. The object path is
     * basically the unique identity of your object within its hosting
     * BusConnection.
     *
     * \return the object path
     */
    const char* GetPath() const;

    /** \private
     * Retrieves a reference to the connection object with the underlying communication layer
     * \return Connection object with the underlying communication layer
     */
    std::weak_ptr<BusConnectionImpl> GetBusConnection();

  protected:
    /**
     * \brief Constructor
     *
     * This variant of the constructor allows for the explicit specification of
     * an object path. In many cases, you should not care about the object
     * path, and let the Data-driven API generate one for you by using the
     * ProvidedObject::ProvidedObject(BusConnection&) constructor.
     *
     * \see ProvidedObject::GetPath
     *
     * \param[in] busConnection the BusConnection that will host the object.
     * \param[in] path the object path
     */
    ProvidedObject(BusConnection& busConnection,
                   const qcc::String& path);

    /**
     * \brief Constructor
     *
     * This variant of the constructor will auto-generate an object path. This
     * is the recommended way of working. If you want control over the object
     * path, use the
     * ProvidedObject::ProvidedObject(BusConnection&, const qcc::String&)
     * constructor.
     *
     * \param[in] busConnection the BusConnection that will host the object.
     */
    ProvidedObject(BusConnection& busConnection);

  private:
    /* unfortunately BusObject does not expose its interfaces.
     * Maybe in the future we can change this at BusObject level and then we don't need to store it ourselves.. */
    friend class BusConnectionImpl;
    std::vector<const ProvidedInterface*> interfaces;
    std::weak_ptr<BusConnectionImpl> busConnectionImpl;
    ProvidedObject::State state;
    void Register();

    void CallMethodHandler(ajn::MessageReceiver::MethodHandler handler,
                           const ajn::InterfaceDescription::Member* member,
                           ajn::Message& message,
                           void* context);

    static qcc::String GeneratePath();

    class MethodHandlerTaskData;
};
}

#undef QCC_MODULE
#endif /* PROVIDEDOBJECT_H_ */
