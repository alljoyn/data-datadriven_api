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

#ifndef PROVIDEDINTERFACE_H_
#define PROVIDEDINTERFACE_H_

#include <memory>
#include <map>
#include <set>

#include <datadriven/Mutex.h>

#include <alljoyn/InterfaceDescription.h>
#include <alljoyn/MessageReceiver.h>

namespace datadriven {
class TypeDescription;
class ProvidedObjectImpl;
class RegisteredTypeDescription;

/**
 * \class ProvidedInterface
 * \brief Abstract base class for all generated provider-side AllJoyn interface
 *        classes.
 *
 * You should never interact with an object of this class directly, always
 * use the subclasses created by the code generator. The only method in this
 * class that is relevant to application developers is ProvidedInterface::Update.
 *
 * For an AllJoyn interface Foo, the code generator would create an abstract class
 * FooInterface that is derived from ProvidedInterface. Application developers
 * must then inherit from both FooInterface and ProvidedObject to create an
 * object that can be exposed on the AllJoyn bus.
 */
class ProvidedInterface :
    public ajn::MessageReceiver {
  public:
    /** \private
     * Retrieves the registered format of the type description
     * \return Registered type description
     */
    const RegisteredTypeDescription* GetRegisteredTypeDescription() const;

    /** \private
     * Registers this interface in the underlying communication layer
     * \param busAttachment Alljoyn bus connection
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus Register(ajn::BusAttachment& busAttachment);

    /**
     * \brief Alert all consumers that the observable properties of this object
     *        have changed.
     *
     * Changes in an object's properties should be communicated to consumers in
     * a consistent way. For example, suppose interface Foo declares two
     * integer properties X and Y, with the invariant X + Y == 100. If X
     * changes for some reason, Y should change alongside X to preserve the
     * invariant. As the Data-driven API framework itself does not know about
     * those invariants, it cannot decide for you when to alert consumers that
     * an object's properties have changed. Therefore, the application code
     * must tell the framework that the object's properties are now in a
     * consistent state, and it is time to send the update. This is the purpose
     * of the Update method.
     *
     * Typically, the granularity of consistency for objects is not the full
     * object, but rather the interface, which is why the Update method is tied
     * to a specific AllJoyn interface, and not to the provided object as a
     * whole. There is a clear exception to this rule however, which is when
     * the object is first exposed on the AllJoyn bus. At this point, it is
     * important that the properties defined in all interfaces implemented by
     * the object are exposed on the bus simultaneously. For this reason, the
     * ProvidedObject::UpdateAll method is defined as a convenience method that
     * calls the Update method for all interfaces implemented by the object.
     *
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus Update();

    /**
     * Return the status of the last bus interaction
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus GetStatus() const;

    /** \private
     * Code generator will implement this virtual function to marshal the properties
     */
    virtual QStatus MarshalProperties() = 0;

    /** \private
     * Get the value of a property with name \a name
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus GetProperty(const char* name,
                        ajn::MsgArg& value);

    /** \private
     * Set the value of a property with name \a name
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus SetProperty(const char* name,
                        ajn::MsgArg& value);

  protected:
    /**
     * \class PropertyValue
     * \brief Class holds the property value. Both the index in the message arguments array
     *        as the MsgArg itself are stored in this class
     */
    class PropertyValue {
      public:
        /**
         * The marshaled value of the property
         */
        ajn::MsgArg msgArg;
        /**
         * The index of this property in the message arguments array
         */
        unsigned int idx;

        /**
         * Constucts a PropertyValue object with a given index
         *
         * \param[in] idx index of the property
         */
        PropertyValue(unsigned int idx) :
            idx(idx) { };
    };
    /** \private
     * A map of the marshaled properties (name-value pairs)
     */
    std::map<qcc::String, PropertyValue*> marshaledProperties;

    /** \private
     * Initialization of the object
     * \param desc type description
     * \param obj Reference to the Provided Object to which it belongs
     */
    ProvidedInterface(const TypeDescription& desc,
                      std::shared_ptr<ProvidedObjectImpl> providedObject);

    /**
     * Object cleanup
     */
    virtual ~ProvidedInterface();

    /** \private
     * Get the implementation of the provided object. This is used by the
     * generated code.
     *
     * \return the ProvidedObjectImpl shared pointer
     */
    std::shared_ptr<ProvidedObjectImpl> GetObjectImpl();

    /** \private
     * The current status of this interface.
     * */
    QStatus _status;

    /** \private
     * Code generator will implement this virtual function to set one of the properties
     */
    virtual QStatus DispatchSetProperty(const char* propName,
                                        ajn::MsgArg& propValue) = 0;

    /** \private
     * Code generator will implement this virtual function to get one of the properties
     */
    virtual QStatus DispatchGetProperty(const char* propName,
                                        ajn::MsgArg& propValue) const = 0;

    /** \private
     * Invalidate a property so it will be sent out in the PropertiesChanged signal.
     *
     * \retval ER_OK on success
     * \retval ER_BUS_NO_SUCH_PROPERTY if the property name is invalid
     */
    QStatus InvalidateProperty(const char* name);

    /** \private
     * Emits a signal (broadcast) across the communication layer
     * \param[in] signalNumber The number of the signal to be emitted.
     * \param[in] args List of input arguments to be taken by the Signal invocation.
     * \param[in] numArgs Number of input arguments.
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus EmitSignal(int signalNumber,
                       const ajn::MsgArg* args = NULL,
                       size_t numArgs = 0);

  private:
    /** Reference to TypeDescription */
    const TypeDescription& desc;

    /** RegisteredTypeDescription */
    std::unique_ptr<RegisteredTypeDescription> iface;

    /** ProvidedObject */
    std::shared_ptr<ProvidedObjectImpl> object;

    /** Set of properties that are invalidated and will be sent out in a PropertiesChanged signal.*/
    std::set<const char*> invalidatedProperties;

    /** Protects the invalidatedProperties set. */
    mutable datadriven::Mutex invalidatedPropertiesMutex;

    /**
     * \private
     * Send the update signal
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus SignalUpdate();
};
}

#undef QCC_MODULE
#endif /* PROVIDEDINTERFACE_H_ */
