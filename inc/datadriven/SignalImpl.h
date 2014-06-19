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

#ifndef SIGNAL_IMPL_H_
#define SIGNAL_IMPL_H_

#include "ObserverImpl.h"
#include <alljoyn/MessageReceiver.h>

namespace datadriven {
/** \private */
class SignalListenerImpl :
    public ajn::MessageReceiver {
  public:
    SignalListenerImpl() :
        observerimpl(NULL), member(NULL) { }

    /** Object cleanup */
    virtual ~SignalListenerImpl() { }

    static MessageReceiver::SignalHandler GetHandler()
    {
        return static_cast<MessageReceiver::SignalHandler>(&SignalListenerImpl::ThreadHandler);
    }

    ObserverImpl* GetObserver();

    ajn::InterfaceDescription::Member* GetMember();

  protected:
    virtual void SignalHandler(const ajn::Message& message) = 0;

    ObserverImpl* observerimpl;
    ajn::InterfaceDescription::Member* member;

  private:
    void ThreadHandler(const ajn::InterfaceDescription::Member* member,
                       const char* srcPath,
                       ajn::Message& message);

    class SignalTaskData;
};
}
#endif
