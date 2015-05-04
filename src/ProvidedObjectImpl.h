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

#ifndef PROVIDEDOBJECTIMPL_H_
#define PROVIDEDOBJECTIMPL_H_

#include <memory>
#include <vector>

#include <alljoyn/BusObject.h>
#include <datadriven/Mutex.h>
#include <datadriven/ProvidedObject.h>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

namespace datadriven {
class ObjectAdvertiserImpl;

class ProvidedObjectImpl :
    public ajn::BusObject {
  public:

    /** Destructor */
    virtual ~ProvidedObjectImpl();

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

    /**
     * \brief  Used to send back an error message on an invoked method call
     *
     * \param[in] message Marshaled <em>response content</em> to be send back
     * \param[in] error The name of the error
     * \param[in] errorMessage A description of the error
     *
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus MethodReplyError(const ajn::Message& message,
                             const char* error,
                             const char* errorMessage = NULL);

    /**
     * \brief  Used to send back an error status code on an invoked method call
     *
     * \param[in] message Marshaled <em>response content</em> to be send back
     * \param[in] status The status code for the error
     *
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus MethodReplyErrorCode(const ajn::Message& message,
                                 QStatus status);

    /** \private
     * Retrieves a list of interface names currently present in the ProvidedObject
     */
    const std::vector<qcc::String>& GetInterfaceNames() const;

    /**
     * Adds the interface name to the list of interfaces provided by this object.
     * \param name the interface name
     */
    void AddInterfaceName(const qcc::String& name);

    /**
     * Removes the interface name from the list of interfaces provided by this object.
     * \param name the interface name
     */
    void RemoveInterfaceName(const qcc::String& name);

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

    /**
     * Handle a bus request to read a property from this object.
     * BusObjects that implement properties should override this method.
     * The default version simply returns ER_BUS_NO_SUCH_PROPERTY.
     *
     * @param ifcName    Identifies the interface that the property is defined on
     * @param propName   Identifies the the property to get
     * @param[out] val   Returns the property value. The type of this value is the actual value
     *                   type.
     */
    virtual QStatus Get(const char* ifcName,
                        const char* propName,
                        ajn::MsgArg& val);

    /**
     * Handle a bus attempt to write a property value to this object.
     * BusObjects that implement properties should override this method.
     * This default version just replies with ER_BUS_NO_SUCH_PROPERTY
     *
     * @param ifcName    Identifies the interface that the property is defined on
     * @param propName  Identifies the the property to set
     * @param val        The property value to set. The type of this value is the actual value
     *                   type.
     * @return #ER_BUS_NO_SUCH_PROPERTY (Should be changed by user implementation of BusObject)
     */
    virtual QStatus Set(const char* ifcName,
                        const char* propName,
                        ajn::MsgArg& val);

    QStatus Register();

    QStatus AddInterfaceToBus(const ajn::InterfaceDescription& iface);

    QStatus AddMethodHandlerToBus(const ajn::InterfaceDescription::Member* member,
                                  ajn::MessageReceiver::MethodHandler handler,
                                  void* context = NULL);

    void SetRefCountedPtr(std::weak_ptr<ProvidedObjectImpl> obj);

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
    ProvidedObjectImpl(std::weak_ptr<ObjectAdvertiserImpl> objectAdvertiser,
                       const qcc::String& path,
                       ProvidedObject& obj);

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
    ProvidedObjectImpl(std::weak_ptr<ObjectAdvertiserImpl> objectAdvertiser,
                       ProvidedObject& obj);

  private:
    /* unfortunately BusObject does not expose its interfaces.
     * Maybe in the future we can change this at BusObject level and then we don't need to store it ourselves.. */
    std::weak_ptr<ObjectAdvertiserImpl> objectAdvertiserImpl;
    ProvidedObject::State state;
    std::weak_ptr<ProvidedObjectImpl> self; // Weak pointer that can be passed to other objects
    datadriven::Mutex mutex;
    std::vector<qcc::String> interfaceNames;
    ProvidedObject& providedObject;

    void CallMethodHandler(ajn::MessageReceiver::MethodHandler handler,
                           const ajn::InterfaceDescription::Member* member,
                           ajn::Message& message,
                           void* context);

    static qcc::String GeneratePath();

    class MethodHandlerTask;
};
}

#undef QCC_MODULE
#endif /* PROVIDEDOBJECTIMPL_H_ */
