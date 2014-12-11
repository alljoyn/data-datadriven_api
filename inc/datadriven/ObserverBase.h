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

#ifndef OBSERVERBASE_H_
#define OBSERVERBASE_H_

#include <vector>
#include <map>
#include <memory>

#include <datadriven/Mutex.h>

#include <datadriven/ObjectAllocator.h>
#include <datadriven/ProxyInterface.h>

namespace datadriven {
class SignalListenerBase;
class BusConnectionImpl;
class ObserverManager;
class ObserverCache;

/** \private */
class ObserverBase :
    protected ajn::ProxyBusObject::PropertiesChangedListener,
    public ObjectAllocator {
  public:
    virtual ~ObserverBase();

    /**
     * Return a shared pointer to a ::BusConnectionImpl
     */
    std::shared_ptr<BusConnectionImpl> GetBusConnection() const;

    /**
     * \brief Object added to bus
     *
     * This method will be called when a remote ProvidedObject was added
     * to the bus. The Observer can take necessary steps to update its
     * local cache (if applicable).
     *
     * \param[in] objProxy Proxy interface object
     */
    virtual void AddObject(const std::shared_ptr<ProxyInterface>& objProxy) = 0;

    /**
     * \brief Object Removed from bus
     *
     * This method will be called when a remote ProvidedObject was removed
     * from the bus. The Observer can take necessary steps to update its
     * local cache (if applicable).
     *
     * \param[in] objProxy Proxy interface object
     */
    virtual void RemoveObject(const std::shared_ptr<ProxyInterface>& objProxy) = 0;

    /**
     * \brief Object properties changed.
     *
     * This method will be called when properties of a remote ProvidedObject
     * were changed or when the ProvidedObject was first put on the bus.
     * The observer can update its local cache (if applicable).
     *
     * \param[in] objProxy Proxy interface object
     */
    virtual void UpdateObject(const std::shared_ptr<ProxyInterface>& objProxy) = 0;

    /**
     * \private
     *
     * Sets the weak pointer to be able to guarantee that ObserverBase destruction is not done while
     * there are still tasks running on them
     * Due to the fact that the cache keeps track of all related observers we can only add this object
     * to the cache in this method.
     *
     * \param[in] observer the observer to be kept alive
     */
    QStatus SetRefCountedPtr(std::weak_ptr<ObserverBase> observer);

    /**
     * \private
     *
     * Called to handle an incoming signal.
     *
     * \param[in] listener The listener to dispatch the signal to.
     * \param[in] message Incoming signal message.
     */
    void HandleSignal(SignalListenerBase* listener,
                      const ajn::Message& message);

    /**
     * Return the number of discovered objects.
     * \return number of discovered objects
     */
    size_t Size();

  protected:
    /**
     * \private
     * \brief Constructor.
     *
     * \param[in] listener Notification handler for property changes and changes to discovered objects<br>
     *                     A NULL value will discard any detected property and discovered object changes.
     * \param[in] bus The (optional) AllJoyn BusAttachment to be used for interactions with the bus.  If
     *                not provided one will be created.
     */
    ObserverBase(const TypeDescription& typedesc,
                 ajn::BusAttachment* bus = nullptr);

    /**
     * \private
     * \brief Add a signal listener for a specific signal.
     *
     * It is allowed to add multiple signal listeners for the same signal to a
     * single Observer.
     *
     * \tparam S the signal class created by the code generator for the signal
     *           in question.
     * \param[in] listener the signal listener.
     * \param[in] memberNumber the number of the signal member linked to the listener
     * \retval ER_OK    success
     * \retval other    failure
     */
    QStatus AddSignalListener(SignalListenerBase* listener,
                              int memberNumber);

    /**
     * \private
     * \brief Remove a signal listener.
     *
     * \tparam S the signal class created by the code generator for the signal
     *           in question.
     * \param[in] listener the signal listener
     * \retval ER_OK    success
     * \retval other    failure
     */
    QStatus RemoveSignalListener(SignalListenerBase* listener);

    /**
     * \private
     * \brief Reports whether the Observer is successfully constructed and
     *        initialized.
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus GetStatus() const;

    /**
     * \private
     * Returns the registered type description for the objects this class
     * is responsible for.
     *
     * \return the registered type description
     */
    const RegisteredTypeDescription& GetRegisteredTypeDescription() const;

    /**
     * \private
     * Returns the ObjectId derived from the \a message
     *
     * \param[in] message the message received
     * \return the derived ObjectId
     */
    ObjectId* GetObjectId(ajn::Message message);

    /**
     * \private
     * Retrieves an object identified by \a objId from the cache linked to this observer base
     *
     * \param[in] objId the object identifier
     * \return a shared pointer to the object
     */
    std::shared_ptr<ProxyInterface> GetObject(const ObjectId& objId);

    /**
     * \private
     * Returns a vector containing all objects from the cache linked to this observer base
     */
    std::vector<std::shared_ptr<ProxyInterface> > GetObjects() const;

    /**
     * Callback for handling property updates in the ajn::ProxyBusObject::PropertiesChangedListener interface.
     */
    virtual void PropertiesChanged(ajn::ProxyBusObject& obj,
                                   const char* ifaceName,
                                   const ajn::MsgArg& changed,
                                   const ajn::MsgArg& invalidated,
                                   void* context);

    /**
     * Pointer to the observer manager. Once the ObserverBase class has been instantiated
     * it can be registered to the observer manager.
     */
    std::shared_ptr<ObserverManager> observerMgr;

    /**
     * Unique pointer to the registered type description of the objects this observer
     * is responsible for.
     */
    std::unique_ptr<RegisteredTypeDescription> registeredTypeDesc;

  private:
    class ObserverTask;

    /**
     * The status of the observer base class. Can be queried at all times.
     */
    QStatus status;

    /**
     * A shared pointer to the BusConnectionImpl.
     */
    std::shared_ptr<BusConnectionImpl> busConnectionImpl;

    /**
     * A weak pointer to be able to guarantee that ObserverBase is not deleted while
     * there are still tasks running on them
     */
    std::weak_ptr<ObserverBase> observerBase;

    /**
     * Signal listeners map type.
     */
    typedef std::map<const ajn::InterfaceDescription::Member*, SignalListenerBase*> SignalListenerMap;

    /**
     * Signal listeners map.
     */
    SignalListenerMap signalListeners;

    /**
     * Mutex protecting the signal listeners map.
     */
    datadriven::Mutex signalListenersMutex;

    /**
     * Called when the object identified by \a objId needs to be updated. This actually
     * means the the remote BusObject was updated.
     *
     * \param[in] objId the object identifier
     * \param[in] changedProps the (optional) list of changed properties
     * \param[in] invalidatedProps the (optional) list of invalidated properties
     */
    void UpdateObject(const ObjectId& objId,
                      const ajn::MsgArg* changedProps = nullptr,
                      const ajn::MsgArg* invalidatedProps = nullptr);

    // prevent copy by construction or assignment
    ObserverBase(const ObserverBase&);
    void operator=(const ObserverBase&);
};
}

#endif /* OBSERVERBASE_H_ */
