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

#ifndef METHODINVOCATION_H_
#define METHODINVOCATION_H_

#include <memory>
#include <semaphore.h>

#include <qcc/Debug.h>
#include <alljoyn/Status.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
/**
 * \class MethodInvocation
 * \brief Represents an invocation of a method on an AllJoyn interface.
 *
 * All method invocations on remote objects are performed asynchronously. The
 * MethodInvocation object serves as a means of tracking the status of a given
 * method invocation. It is implemented as a <a
 * href="http://en.wikipedia.org/wiki/Future_(programming)">Future</a>.
 *
 * \tparam T The method reply class created by the code generator for the
 *           invoked method. For example, for an AllJoyn interface Foo with
 *           method Bar, T would be the FooProxy::BarReply generated class.
 */
template <typename T> class MethodInvocation :
    public ajn::MessageReceiver {
  public:
    /**
     * \enum InvState
     * Method invocation states.
     */
    enum InvState {
        WAITING, /**< Waiting for remote side to respond */
        READY /**< Method reply (or error message) received and processed */
    };

    /**
     * The default timeout for method calls (25 seconds)
     */
    static const uint32_t DefaultCallTimeout = ajn::ProxyBusObject::DefaultCallTimeout;

  public:
    /** \private Initializes the Future object. */
    MethodInvocation() :
        shared_this(this), state(WAITING)
    {
        sem_init(&sem, 0, 0);
    }

    /** \private Cleanup of the Future object. */
    ~MethodInvocation()
    {
        sem_destroy(&sem);
    }

    /** \private
     * Use by generated code for getting a shared pointer to the invocation.
     * \return A shared pointer to the invocation.
     */
    std::shared_ptr<MethodInvocation<T> > GetSharedThis()
    {
        return shared_this;
    }

    /** \private
     * Set the status of the method reply and mark the invocation as ready.
     * \param[in] status Method reply error code.
     */
    void SetReplyStatus(const QStatus status)
    {
        reply.SetStatus(status);
        state = READY;
    }

    /**
     * \brief Get the method reply object.
     *
     * This call will block until the method reply message has been received,
     * or the invocation has timed out. Upon return of this method, the
     * MethodInvocation's state is guaranteed to be InvState::READY.
     *
     * \return ConsumerMethodReply-derived object that encapsulates the
     *         received method reply (or error message)
     */
    const T& GetReply()
    {
        if (WAITING == state) {
            sem_wait(&sem);
        }
        return reply;
    }

    /**
     * \brief Get the current state of the method invocation.
     * \retval READY The method invocation is complete.
     * \retval WAITING We are still waiting for the method reply.
     */
    InvState GetState() const
    {
        return state;
    }

    /** \private
     * Executes an asynchronous method call on the underlying communication layer.
     * \param[in] intf Proxy object for a remote interface on which the call is invoked.
     * \param[in] member Member of the interface for which the the call is intended.
     * \param[in] args List of input arguments to be taken by the Method invocation.
     * \param[in] numArgs Number of input arguments.
     * \param[in] timeout Timeout (in ms) to wait for a reply.
     */
    void Exec(const ProxyInterface& intf,
              const ajn::InterfaceDescription::Member& member,
              const ajn::MsgArg* msgarg = NULL,
              size_t numArgs = 0,
              uint32_t timeout = DefaultCallTimeout)
    {
        QStatus status;

        if (intf.IsAlive()) {
            MessageReceiver::ReplyHandler handler =
                static_cast<MessageReceiver::ReplyHandler>(&MethodInvocation<T>::OnReplyMessage);
            status = intf.GetProxyBusObject().MethodCallAsync(member, this, handler, msgarg, numArgs, NULL, timeout);
        } else {
            status = ER_FAIL;   /* Is there a more suitable error ? */
            QCC_LogError(status, ("Cannot call method on dead object"));
        }
        if (ER_OK != status) {
            SetReplyStatus(status);
        }
    }

    /** \private
     * Move constructor.
     * \param inv Original MethodInvocation object to be moved.
     */
    MethodInvocation(const MethodInvocation && inv) :
        sem(std::move(inv.sem)),
        state(std::move(inv.state)),
        reply(std::move(inv.reply))
    { }

  private:
    // ensure we stay alive until response arrives even if caller does not
    // keep reference
    mutable std::shared_ptr<MethodInvocation<T> > shared_this;
    sem_t sem;
    MethodInvocation::InvState state;
    T reply;

    void OnReplyMessage(ajn::Message& message, void* context)
    {
        QStatus status = ER_FAIL;

        if (shared_this.use_count() > 1) {
            // we are not the only ones referring to this invocation
            if (ajn::MESSAGE_ERROR == message->GetType()) {
                const char* name = message->GetErrorName(NULL);

                if (name && (0 == strcmp("org.alljoyn.Bus.Timeout", name))) {
                    status = ER_TIMEOUT;
                }
            } else {
                if (ER_OK != (status = reply.Unmarshal(message))) {
                    QCC_LogError(status, ("Failed to unmarshal data"));
                }
            }
            SetReplyStatus(status);
            sem_post(&sem);
        }
        shared_this = nullptr; // no longer needed
    }

    // prevent copy by assignment (can not copy semaphore)
    MethodInvocation(const MethodInvocation&);
    void operator=(const MethodInvocation&);
};
}

#undef QCC_MODULE
#endif /* METHODINVOCATION_H_ */
