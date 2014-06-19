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

#include <datadriven/SignalImpl.h>
#include "BusConnectionImpl.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
class SignalListenerImpl::SignalTaskData :
    public BusConnectionImpl::BusConnTaskData {
  private:
    SignalListenerImpl* si;
    std::weak_ptr<BusConnectionImpl> busConnectionImpl;
    ajn::Message message;

  public:
    SignalTaskData(SignalListenerImpl* _si,
                   std::weak_ptr<BusConnectionImpl> _busConnectionImpl,
                   ajn::Message& m) :
        si(_si), busConnectionImpl(_busConnectionImpl), message(m)
    { };

    void Execute(const BusConnTaskData* ot) const
    {
        std::shared_ptr<BusConnectionImpl> busConn = busConnectionImpl.lock();
        if (busConn) {
            SignalTaskData* std  = const_cast<SignalTaskData*>(static_cast<const SignalTaskData*>(ot));
            QStatus result = busConn->LockForSignalListener(std->si);

            if (ER_OK == result) {
                std->si->SignalHandler(std->message);
                busConn->UnlockObserver();
            }
        }
    }
};

void SignalListenerImpl::ThreadHandler(const ajn::InterfaceDescription::Member* member,
                                       const char* srcPath,
                                       ajn::Message& message)
{
    if (member == NULL || srcPath == NULL) {
        QCC_LogError(ER_FAIL, ("Invalid arguments"));
        return;
    }

    QCC_DbgPrintf(("Got signal '%s' from path '%s' in '%s'", member->name.c_str(), srcPath, message->GetSender()));

    std::shared_ptr<BusConnectionImpl> busConn = observerimpl->busConnectionImpl.lock();
    if (busConn) {
        SignalTaskData* signalTaskData = new SignalTaskData(this, observerimpl->busConnectionImpl, message);
        busConn->ConsumerAsyncEnqueue(signalTaskData);
    }
}

ObserverImpl* SignalListenerImpl::GetObserver()
{
    return observerimpl;
}

ajn::InterfaceDescription::Member* SignalListenerImpl::GetMember()
{
    return member;
}
}
