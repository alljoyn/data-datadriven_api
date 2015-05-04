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

#include <datadriven/MethodInvocationBase.h>
#include <datadriven/Marshal.h>

#include "RegisteredTypeDescription.h"
#include "ObserverManager.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
class MethodTask :
    public ObserverManager::Task {
  public:
    MethodTask(std::weak_ptr<MethodInvocationBase> inv) :
        inv(inv) { }

    void Execute() const
    {
        std::shared_ptr<MethodInvocationBase> invoc = inv.lock();
        if (invoc) {
            invoc->HandleReply();
        }
    }

  private:
    std::weak_ptr<MethodInvocationBase> inv;
};

void MethodInvocationBase::ScheduleMethodReplyListener()
{
    std::shared_ptr<ObserverManager> mgr = ObserverManager::GetInstance();
    if (mgr) {
        MethodTask* task = new MethodTask(weak_this);
        mgr->Enqueue(task);
    }
}

MethodInvocationBase::MethodInvocationBase() :
    state(WAITING),
    shared_this(nullptr),
    sem(new Semaphore())
{
}

MethodInvocationBase::~MethodInvocationBase()
{
    if (nullptr != sem) {
        // Note: We cannot cancel started async method calls in Alljoyn, so we can
        //       still get OnReplyMessage calls during destruction of MethodInvocationBase.
        //       Putting the semaphore to null before deleting it reduces the chance of
        //       getting core dumps when Alljoyn calls back to this object.
        //       But there is still a race condition here, in which a reply message is using
        //       the sem member variable while we are deleting it.
        Semaphore* tmp = sem;
        sem = nullptr;
        delete tmp;
    }
    weak_this.reset();
}

MethodInvocationBase::MethodInvocationBase(MethodInvocationBase&& inv) :
    state(std::move(inv.state))
{
    // move semaphore ownership
    sem = inv.sem;
    inv.sem = nullptr;
}

void MethodInvocationBase::SetRefCountedPtr(std::shared_ptr<MethodInvocationBase> inv)
{
    shared_this = inv;
    weak_this = inv;
}

MethodInvocationBase::InvState MethodInvocationBase::GetState() const
{
    return state;
}

void MethodInvocationBase::WaitForReply()
{
    if (WAITING == state) {
        if (nullptr == sem) {
            SetReplyStatus(ER_FAIL);
        } else {
            sem->Wait();
        }
    }
}

void MethodInvocationBase::SetReplyStatus(const QStatus status)
{
    GetConsumerMethodReply().SetStatus(status);
    state = READY;
    shared_this = nullptr;
}

void MethodInvocationBase::Cancel()
{
    GetConsumerMethodReply().SetStatus(ER_FAIL);
    state = CANCELLED;
    weak_this.reset();
}

void MethodInvocationBase::Exec(const ProxyInterface& intf,
                                int memberNumber,
                                const ajn::MsgArg* msgarg,
                                size_t numArgs,
                                uint32_t timeout)
{
    QStatus status;
    qcc::String noReply = "";

    if (intf.IsAlive()) {
        MessageReceiver::ReplyHandler handler =
            static_cast<MessageReceiver::ReplyHandler>(&MethodInvocationBase::OnReplyMessage);
        const ajn::InterfaceDescription::Member& member = intf.GetTypeDescription().GetMember(memberNumber);
        member.GetAnnotation(ajn::org::freedesktop::DBus::AnnotateNoReply, noReply);

        if (noReply == "true") {
            status = intf.GetProxyBusObject().MethodCall(member, msgarg, numArgs);
        } else {
            status = intf.GetProxyBusObject().MethodCallAsync(member, this, handler, msgarg, numArgs, NULL, timeout);
        }
    } else {
        status = ER_FAIL;   /* Is there a more suitable error ? */
        QCC_LogError(status, ("Cannot call method on dead object"));
    }
    if (ER_OK != status) {
        SetReplyStatus(status);
    } else if (noReply == "true") {
        // always mark a fire-and-forget call as successfully completed if no error occurred while firing
        SetReplyStatus(ER_OK);
    }
}

void MethodInvocationBase::OnReplyMessage(ajn::Message& message, void* context)
{
    QStatus status = ER_OK;
    qcc::String errorName;
    qcc::String errorDescription;

    if (shared_this.use_count() > 1) {
        // we are not the only ones referring to this invocation

        // Check if this message is an error message and set the appropriate error variables
        if (ajn::MESSAGE_ERROR == message->GetType()) {
            /* We set the status so the application knows the MethodReply was an error message */
            status = ER_BUS_REPLY_IS_ERROR_MESSAGE;

            /* Get errorName and ErrorDescription */
            errorName = message->GetErrorName(&errorDescription);

            /* Check if it's a Timeout Error */
            if (0 == strcmp("org.alljoyn.Bus.Timeout", errorName.c_str())) {
                status = ER_TIMEOUT;

                /* Check if it's an error sent by a QStatus code */
            } else if (0 == strcmp("org.alljoyn.Bus.ErStatus", errorName.c_str())) {
                /* Get the msgArgs from the Message */
                size_t numArgs;
                const ajn::MsgArg* msgArgs;
                uint16_t statusCode;
                message->GetArgs(numArgs, msgArgs);

                /* Get QStatus code from the msgArgs */
                char* str;
                if (numArgs == 2) {
                    if (ER_OK == (status = msgArgs->Get(msgArgs, numArgs, "sq", &str, &statusCode))) {
                        /* Set status to the MethodReply status code */
                        status = (QStatus)statusCode;
                    }
                }
            }
        } else if (ajn::MESSAGE_INVALID == message->GetType()) {
            /* Not a valid message */
            status = ER_FAIL;
        } else {
            /*
             * NOTE: there is a special case. When a MethodReply is sent with ER_OK QStatus code
             * we end up here and treat it like a succeeded MethodReply
             */

            const ajn::MsgArg* msgarg;
            size_t numArgs;

            message->GetArgs(numArgs, msgarg);
            if (ER_OK != (status = GetConsumerMethodReply().Unmarshal(msgarg, numArgs))) {
                QCC_LogError(status, ("Failed to unmarshal method reply data"));
            }
        }
        GetConsumerMethodReply().SetErrorName(errorName);
        GetConsumerMethodReply().SetErrorDescription(errorDescription);
        if (shared_this->HasListener() && state != CANCELLED) {
            ScheduleMethodReplyListener();
        }
        SetReplyStatus(status);

        if (nullptr != sem) {
            sem->Post();
        }
    } else {
        SetReplyStatus(ER_FAIL); // no-one listening anymore
    }
}

/** \private
 * Executes an asynchronous get property call on the underlying communication layer.
 * \param[in] intf Proxy object for a remote interface on which the call is invoked.
 * \param[in] propName Name of the property to get.
 * \param[in] timeout Timeout (in ms) to wait for a reply.
 */
void MethodInvocationBase::GetProperty(const ProxyInterface& intf,
                                       const char* propName,
                                       uint32_t timeout)
{
    QStatus status;

    if (intf.IsAlive()) {
        const char* ifName = intf.GetTypeDescription().GetDescription().GetName().c_str();
        // remove const from ProxyBusObject because GetPropertyAsync() is
        // non-const although it calls the const MethodCall()
        ajn::ProxyBusObject& proxy = const_cast<ajn::ProxyBusObject&>(intf.GetProxyBusObject());
        ajn::ProxyBusObject::Listener::GetPropertyCB cb =
            static_cast<ajn::ProxyBusObject::Listener::GetPropertyCB>(&MethodInvocationBase::OnGetProperty);
        status = proxy.GetPropertyAsync(ifName, propName, this, cb, NULL, timeout);
    } else {
        status = ER_FAIL;   /* Is there a more suitable error ? */
        QCC_LogError(status, ("Cannot get property from dead object"));
    }
    if (ER_OK != status) {
        SetReplyStatus(status);
    }
}

void MethodInvocationBase::OnGetProperty(QStatus status,
                                         ajn::ProxyBusObject* obj,
                                         const ajn::MsgArg& value,
                                         void* context)
{
    if (ER_OK == status) {
        const ajn::MsgArg& msgarg = datadriven::MsgArgDereference(value);
        status = GetConsumerMethodReply().Unmarshal(&msgarg, 1);
        if (ER_OK != status) {
            QCC_LogError(status, ("Failed to unmarshal get property data"));
        }
    }
    SetReplyStatus(status);
    if (nullptr != sem) {
        sem->Post();
    }
}

/** \private
 * Executes an asynchronous set property call on the underlying communication layer.
 * \param[in] intf Proxy object for a remote interface on which the call is invoked.
 * \param[in] propName Name of the property to set.
 * \param[in] propValue Value of the property to set.
 * \param[in] timeout Timeout (in ms) to wait for a reply.
 */
void MethodInvocationBase::SetProperty(const ProxyInterface& intf,
                                       const char* propName,
                                       ajn::MsgArg& propValue,
                                       uint32_t timeout)
{
    QStatus status;

    if (intf.IsAlive()) {
        const char* ifName = intf.GetTypeDescription().GetDescription().GetName().c_str();
        // remove const from ProxyBusObject because SetPropertyAsync() is
        // non-const although it calls the const MethodCall()
        ajn::ProxyBusObject& proxy = const_cast<ajn::ProxyBusObject&>(intf.GetProxyBusObject());
        ajn::ProxyBusObject::Listener::SetPropertyCB cb =
            static_cast<ajn::ProxyBusObject::Listener::SetPropertyCB>(&MethodInvocationBase::OnSetProperty);
        status = proxy.SetPropertyAsync(ifName, propName, propValue, this, cb, NULL, timeout);
    } else {
        status = ER_FAIL;   /* Is there a more suitable error ? */
        QCC_LogError(status, ("Cannot set property on dead object"));
    }
    if (ER_OK != status) {
        SetReplyStatus(status);
    }
}

void MethodInvocationBase::OnSetProperty(QStatus status,
                                         ajn::ProxyBusObject* obj,
                                         void* context)
{
    SetReplyStatus(status);
    if (nullptr != sem) {
        sem->Post();
    }
}
}
