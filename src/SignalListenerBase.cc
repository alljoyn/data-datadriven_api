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

#include <datadriven/SignalListenerBase.h>
#include "BusConnectionImpl.h"
#include "ObserverManager.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
class SignalListenerBase::SignalTask :
    public ObserverManager::Task {
  public:
    SignalTask(SignalListenerBase* listener,
               std::weak_ptr<ObserverBase> observerBase,
               ajn::Message& message) :
        listener(listener), observerBase(observerBase), message(message)
    { }

    void Execute() const
    {
        QCC_DbgPrintf(("SignalTask => Execute called"));

        std::shared_ptr<ObserverBase> obs = observerBase.lock();
        if (obs) {
            obs->HandleSignal(listener, message);
        }
    }

  private:
    SignalListenerBase* listener;
    std::weak_ptr<ObserverBase> observerBase;
    ajn::Message message;
};

SignalListenerBase::SignalListenerBase() :
    observerBase(), member(NULL)
{
}

/**
 * Object cleanup
 */
SignalListenerBase::~SignalListenerBase()
{
}

ajn::MessageReceiver::SignalHandler SignalListenerBase::GetHandler()
{
    return static_cast<MessageReceiver::SignalHandler>(&SignalListenerBase::ThreadHandler);
}

void SignalListenerBase::ThreadHandler(const ajn::InterfaceDescription::Member* member,
                                       const char* srcPath,
                                       ajn::Message& message)
{
    if (member == NULL || srcPath == NULL) {
        QCC_LogError(ER_FAIL, ("Invalid arguments"));
        return;
    }

    QCC_DbgPrintf(("SignalListenerBase: Got signal '%s' from path '%s' in '%s'", member->name.c_str(), srcPath,
                   message->GetSender()));

    std::shared_ptr<ObserverManager> mgr = ObserverManager::GetInstance();
    if (mgr) {
        SignalTask* task = new SignalTask(this, observerBase, message);
        mgr->Enqueue(task);
    }
}

void SignalListenerBase::SetObserver(std::weak_ptr<ObserverBase> observerBase)
{
    this->observerBase = observerBase;
}

const ajn::InterfaceDescription::Member* SignalListenerBase::GetMember()
{
    return member;
}

void SignalListenerBase::SetMember(const ajn::InterfaceDescription::Member* member)
{
    this->member = member;
}
}
