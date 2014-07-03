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

#include <alljoyn/InterfaceDescription.h> // needed by MessageReceiver.h
#include <alljoyn/MessageReceiver.h>

#include <datadriven/ObserverBase.h>

namespace datadriven {
/** \private */
class SignalListenerBase :
    public ajn::MessageReceiver {
  public:
    SignalListenerBase();

    /**
     * Object cleanup
     */
    virtual ~SignalListenerBase();

    /**
     * Returns the signal handler of this listener
     */
    static ajn::MessageReceiver::SignalHandler GetHandler();

    /**
     * Set the Observer base class
     */
    void SetObserver(std::weak_ptr<ObserverBase> observerBase);

    /**
     * Get the signal member
     */
    const ajn::InterfaceDescription::Member* GetMember();

    /**
     * Sets the signal member
     */
    void SetMember(const ajn::InterfaceDescription::Member* member);

  protected:
    /**
     * The actual handler of the signal. Needs to be implemented
     * by the derived templated SignalListener
     */
    virtual void SignalHandler(const ajn::Message& message) = 0;

    friend class ObserverBase;

    std::weak_ptr<ObserverBase> observerBase;
    const ajn::InterfaceDescription::Member* member;

  private:
    void ThreadHandler(const ajn::InterfaceDescription::Member* member,
                       const char* srcPath,
                       ajn::Message& message);

    class SignalTask;
};
}
#endif
