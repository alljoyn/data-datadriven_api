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

#include <vector>
#include <memory>

#include <alljoyn/InterfaceDescription.h>
#include <alljoyn/MessageReceiver.h>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

namespace datadriven {
class TypeDescription;
class ProvidedObject;
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
     * Describes/defines the type of the provided interface
     * \return Type description
     */
    const TypeDescription* GetTypeDescription() const;

    /** \private
     * Retrieves the registered format of the type description
     * \return Registered type description
     */
    const RegisteredTypeDescription* GetRegisteredTypeDescription() const;

    /** \private
     * Registers the Type description in the underlying communication layer
     * \param busAttachment Alljoyn bus connection
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus RegisterInterface(ajn::BusAttachment& busAttachment);

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

  protected:
    /** \private
     * Initialization of the object
     * \param desc type description
     * \param obj Reference to the Provided Object to which it belongs
     */
    ProvidedInterface(const TypeDescription& desc,
                      ProvidedObject& providedObject);

    /**
     * Object cleanup
     */
    virtual ~ProvidedInterface();

    /** \private
     * Code generator will implement this virtual function to marshal the properties
     */
    virtual std::vector<ajn::MsgArg> MarshalProperties() = 0;

    /** \private
     * Get the object providing this interface.
     */
    ProvidedObject& GetProvidedObject();

    /** \private
     * The current status of this interface.
     * */
    QStatus status;

  private:
    /** Reference to TypeDescription */
    const TypeDescription& desc;

    /** RegisteredTypeDescription */
    std::unique_ptr<RegisteredTypeDescription> iface;

    /** RegisteredTypeDescription */
    ProvidedObject& providedObject;
};
}

#undef QCC_MODULE
#endif /* PROVIDEDINTERFACE_H_ */
