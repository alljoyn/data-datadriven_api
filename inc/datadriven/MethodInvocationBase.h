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
        READY /**< Method reply (or error message) received and processed */
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

  protected:
    /** \private
     * Shared pointer to ensure we stay alive until response arrives
     * even if caller does not keep reference
     */
    mutable std::shared_ptr<MethodInvocationBase> shared_this;

    /** \private
     * Sets shared_this.
     *
     * \param[in] inv the new shared pointer to set to shared_this
     */
    void SetRefCountedPtr(std::shared_ptr<MethodInvocationBase> inv);

    /**
     * Returns the method reply linked to this method invocation.
     *
     * \return the method reply
     */
    virtual ConsumerMethodReply& GetConsumerMethodReply() = 0;

  private:
    MethodInvocationBase::InvState state;
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
