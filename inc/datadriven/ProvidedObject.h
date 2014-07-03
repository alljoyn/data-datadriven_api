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

#include <alljoyn/InterfaceDescription.h>
#include <alljoyn/MessageReceiver.h>

#include <datadriven/ObjectAdvertiser.h>

namespace test_unit_sessionmanager { class BasicTestObject; }

namespace datadriven {
class ProvidedObjectImpl;
class ProvidedInterface;

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
 *     MyFooBar(std::shared_ptr<ObjectAdvertiser> advertiser) :
 *         ProvidedObject(advertiser),
 *         FooInterface(this),
 *         BarInterface(this)
 *     { ... }
 * }
 * \endcode
 *
 * \note The order of the base classes is significant! ProvidedObject must be
 *       the first base class specified, followed by the xyzInterface classes.
 */

class ProvidedObject  {
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

    /** \private
     * Get the implementation of the provided object.  Internal use only.
     */
    std::shared_ptr<ProvidedObjectImpl> GetImpl();

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

    /** \private Callback handler structure */
    struct MethodCallbacks {
        int memberId;
        ajn::MessageReceiver::MethodHandler handler;
        MethodCallbacks(int memberId,
                        ajn::MessageReceiver::MethodHandler handler) :
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
     * Handle a bus request to read a property from this object.
     *
     * \param ifcName   Identifies the interface that the property is defined on
     * \param propName  Identifies the the property to get
     * \param[out] val  Returns the property value. The type of this value is the actual value
     *                  type.
     * \return the status of the action. ER_OK means everything went well.
     */
    QStatus Get(const char* ifcName,
                const char* propName,
                ajn::MsgArg& val);

    /** \private
     * Handle a bus attempt to write a property value to this object.
     *
     * \param ifcName   Identifies the interface that the property is defined on
     * \param propName  Identifies the the property to set
     * \param val       The property value to set. The type of this value is the actual value
     *                  type.
     * \return the status of the action. ER_OK means everything went well.
     */
    QStatus Set(const char* ifcName,
                const char* propName,
                ajn::MsgArg& val);

  protected:
    /**
     * \brief Constructor
     *
     * This variant of the constructor allows for the explicit specification of
     * an object path. In many cases, you should not care about the object
     * path, and let the Data-driven API generate one for you by using the
     * ProvidedObject::ProvidedObject(std::shared_ptr<ObjectAdvertiser>) constructor.
     *
     * \see ProvidedObject::GetPath
     *
     * \param[in] advertiser The advertiser to be used for advertising the object.
     * \param[in] path The object path.
     */
    ProvidedObject(std::shared_ptr<ObjectAdvertiser> advertiser,
                   const qcc::String& path);

    /**
     * \brief Constructor
     *
     * This variant of the constructor will auto-generate an object path. This
     * is the recommended way of working. If you want control over the object
     * path, use the
     * ProvidedObject::ProvidedObject(std::shared_ptr<ObjectAdvertiser>, const qcc::String&)
     * constructor.
     *
     * \param[in] advertiser The advertiser to be used for advertising the object.
     */
    ProvidedObject(std::shared_ptr<ObjectAdvertiser> advertiser);

  private:
    /* unfortunately BusObject does not expose its interfaces.
     * Maybe in the future we can change this at BusObject level and then we don't need to store it ourselves.. */
    std::map<qcc::String, const ProvidedInterface*> interfaces;
    std::weak_ptr<ObjectAdvertiserImpl> objectAdvertiserImpl;
    std::shared_ptr<ProvidedObjectImpl> providedObjectImpl;

    /**
     * \private
     * Check if a certain interface is provided by this object.
     * \return the ProvidedInterface with a given \a name or NULL otherwise
     */
    const ProvidedInterface* GetInterfaceByName(const char* name);
};
}

#undef QCC_MODULE
#endif /* PROVIDEDOBJECT_H_ */
