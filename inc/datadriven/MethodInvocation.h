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

#include <datadriven/MethodInvocationBase.h>
#include <datadriven/MethodReplyListener.h>

#include <qcc/Debug.h>
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
    public MethodInvocationBase {
  public:
    /** \private
     * Use by generated code for creating a shared pointer to an invocation.
     * \return A shared pointer to the invocation.
     */
    static std::shared_ptr<MethodInvocation<T> > Create()
    {
        MethodInvocation<T>* inv = new datadriven::MethodInvocation<T>();
        std::shared_ptr<MethodInvocationBase> sp(inv);
        inv->SetRefCountedPtr(sp);
        inv->methodReplyListener = nullptr;
        return std::static_pointer_cast<MethodInvocation<T> >(sp);
    }

    /** Cleanup of the Future object. */
    virtual ~MethodInvocation()
    { }

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
        WaitForReply();
        return reply;
    }

    /**
     * \brief Add a listener for the method reply
     *
     * This call will add a listener that will be called when the method call
     * has received a reply. This method can only be called once.
     * When the listener is set, it cannot be reset by another listener.
     *
     * The most common practice is to call this right after receiving
     * the method invocation from the method call.
     *
     * \retval ER_OK When the listener is set correctly.
     * \retval ER_FAIL When the listener is already set.
     */
    QStatus SetListener(MethodReplyListener<T>& listener)
    {
        QStatus status = ER_OK;
        methodReplyListenerMutex.Lock();
        if (methodReplyListener) {
            status = ER_FAIL;
        } else {
            methodReplyListener = &listener;

            /* If we notice that a reply is already processed,
             * we can immediately schedule the listener */
            if (READY == state) {
                ScheduleMethodReplyListener();
            }
        }
        methodReplyListenerMutex.Unlock();
        return status;
    }

    /**
     * \brief Tell the framework you do not care about the reply of this method call.
     *
     * This is only a local reinforcement, it explicitly tells the framework
     * you are no longer interested in the reply of the method call.
     */
    void Cancel()
    {
        methodReplyListenerMutex.Lock();
        MethodInvocationBase::Cancel();
        methodReplyListenerMutex.Unlock();
    }

    /** \private
     * Move constructor.
     * \param inv Original MethodInvocation object to be moved.
     */
    MethodInvocation(const MethodInvocation && inv) :
        reply(std::move(inv.reply)), methodReplyListener(inv.methodReplyListener)
    { inv.methodReplyListener =  nullptr; }

  protected:
    /**
     * \private
     */
    virtual ConsumerMethodReply& GetConsumerMethodReply()
    {
        return reply;
    }

    /**
     * \private
     * Call the OnReplyMessage function on the MethodReplyListener
     */
    virtual void HandleReply()
    {
        methodReplyListenerMutex.Lock();
        if (methodReplyListener) {
            methodReplyListener->OnReply(reply);
        }
        methodReplyListenerMutex.Unlock();
    }

  private:
    T reply;
    MethodReplyListener<T>* methodReplyListener;
    datadriven::Mutex methodReplyListenerMutex;

    /** Initializes the Future object. */
    MethodInvocation() :
        methodReplyListener(nullptr)
    { }

    // prevent copy by assignment (can not copy semaphore)
    MethodInvocation(const MethodInvocation&);
    void operator=(const MethodInvocation&);
};
}

#undef QCC_MODULE
#endif /* METHODINVOCATION_H_ */
