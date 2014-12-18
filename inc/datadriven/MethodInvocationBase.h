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

#ifndef METHODINVOCATIONBASE_H_
#define METHODINVOCATIONBASE_H_

#include <memory>

#include <alljoyn/InterfaceDescription.h> // needed by MessageReceiver.h
#include <alljoyn/Message.h>
#include <alljoyn/MessageReceiver.h>
#include <alljoyn/MsgArg.h>
#include <alljoyn/ProxyBusObject.h>

#include <datadriven/ConsumerMethodReply.h>
#include <datadriven/ProxyInterface.h>
#include <datadriven/Semaphore.h>

namespace datadriven {
class MethodReplyListenerBase;

/**
 * \class MethodInvocationBase
 * \brief Base class for an invocation of a method on an AllJoyn interface.
 */
class MethodInvocationBase :
    public ajn::MessageReceiver,
    public ajn::ProxyBusObject::Listener {
  public:
    /**
     * The default timeout for method calls (25 seconds)
     */
    static const uint32_t DefaultCallTimeout = ajn::ProxyBusObject::DefaultCallTimeout;

    /**
     * \enum InvState
     * Method invocation states.
     */
    enum InvState {
        WAITING, /**< Waiting for remote side to respond */
        READY, /**< Method reply (or error message) received and processed */
        CANCELLED /**< Invocation was cancelled by the user */
    };

    /** \private Initializes the Future object. */
    MethodInvocationBase();

    /** \private Cleanup of the Future object. */
    virtual ~MethodInvocationBase();

    /** \private
     * Move constructor.
     * \param inv Original object to be moved.
     */
    MethodInvocationBase(MethodInvocationBase && inv);

    /**
     * \brief Get the current state of the method invocation.
     * \retval READY The method invocation is complete.
     * \retval WAITING We are still waiting for the method reply.
     */
    InvState GetState() const;

    /**
     * \private Set the status of the method reply and mark the invocation as ready.
     * \param[in] status Method reply error code.
     */
    void SetReplyStatus(const QStatus status);

    /**
     *  \brief Tell the framework you do not care about the reply of this method call.
     *
     * This is only a local reinforcement, it explicitly tells the framework
     * you are no longer interested in the reply of the method call.
     */
    void Cancel();

    /**
     * Block calling thread until a reply has arrived.  This could also be a
     * time out or an error
     */
    void WaitForReply();

    /** \private
     * Executes an asynchronous method call on the underlying communication layer.
     * \param[in] intf Proxy object for a remote interface on which the call is invoked.
     * \param[in] member Member of the interface for which the the call is intended.
     * \param[in] args List of input arguments to be taken by the Method invocation.
     * \param[in] numArgs Number of input arguments.
     * \param[in] timeout Timeout (in ms) to wait for a reply.
     */
    void Exec(const ProxyInterface& intf,
              int memberNumber,
              const ajn::MsgArg* msgarg = nullptr,
              size_t numArgs = 0,
              uint32_t timeout = DefaultCallTimeout);

    /** \private
     * Executes an asynchronous get property call on the underlying communication layer.
     * \param[in] intf Proxy object for a remote interface on which the call is invoked.
     * \param[in] propName Name of the property to get.
     * \param[in] timeout Timeout (in ms) to wait for a reply.
     */
    void GetProperty(const ProxyInterface& intf,
                     const char* propName,
                     uint32_t timeout = DefaultCallTimeout);

    /** \private
     * Executes an asynchronous set property call on the underlying communication layer.
     * \param[in] intf Proxy object for a remote interface on which the call is invoked.
     * \param[in] propName Name of the property to set.
     * \param[in] propValue Value of the property to set.
     * \param[in] timeout Timeout (in ms) to wait for a reply.
     */
    void SetProperty(const ProxyInterface& intf,
                     const char* propName,
                     ajn::MsgArg& propValue,
                     uint32_t timeout = DefaultCallTimeout);

    /**
     * \private
     * Call the OnReplyMessage function on the MethodReplyListener
     */
    virtual void HandleReply() = 0;

    /**
     * \private
     * To check if there is a listener set. We need to know this
     * whether we want to schedule the listener or not.
     *
     * \retval true When a listener is set.
     * \retval false When a listener is not set.
     */
    virtual bool HasListener() = 0;

  protected:
    /** \private
     * state of the method reply, can be either WAITING or READY
     */
    MethodInvocationBase::InvState state;

    /** \private
     * Sets shared_this.
     *
     * \param[in] inv the new shared pointer to set to shared_this
     */
    void SetRefCountedPtr(std::shared_ptr<MethodInvocationBase> inv);

    /**
     * \private
     * Schedule the MethodReplyListener on the correct thread
     */
    void ScheduleMethodReplyListener();

    /**
     * Returns the method reply linked to this method invocation.
     *
     * \return the method reply
     */
    virtual ConsumerMethodReply& GetConsumerMethodReply() = 0;

  private:
    /**
     * Shared pointer to ensure that the invocation object stays alive until
     * the response from the Core framework arrives even if the caller does
     * not keep a reference.
     */
    mutable std::shared_ptr<MethodInvocationBase> shared_this;

    /**
     * Weak pointer to be able to check whether the caller still has a
     * reference to the invocation in the case where a listener is used.
     */
    mutable std::weak_ptr<MethodInvocationBase> weak_this;

    Semaphore* sem;

    // prevent copy by construction or assignment (can not copy semaphore)
    MethodInvocationBase(const MethodInvocationBase&);
    void operator=(const MethodInvocationBase&);

    void OnReplyMessage(ajn::Message& message,
                        void* context);

    void OnGetProperty(QStatus status,
                       ajn::ProxyBusObject* obj,
                       const ajn::MsgArg& value,
                       void* context);

    void OnSetProperty(QStatus status,
                       ajn::ProxyBusObject* obj,
                       void* context);
};
}

#endif /* METHODINVOCATIONBASE_H_ */
