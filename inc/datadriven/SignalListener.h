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

#ifndef SIGNALLISTENER_H_
#define SIGNALLISTENER_H_

#include <iostream>
#include <memory>
#include <cassert>

#include <datadriven/Observer.h>
#include <datadriven/SignalListenerBase.h>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
template <typename> class Observer;

/**
 * \class SignalListener
 * \brief Signal listener base class.
 *
 * Application developers should derive from this abstract class to receive
 * notifications when a specific signal on a specific AllJoyn interface is
 * received.
 *
 * \note An instance of the concrete listener class must be activated via
 *       Observer::AddSignalListener before it can receive any notifications.
 *
 * \tparam T The generated proxy class that corresponds to the AllJoyn interface
 *           that contains the signal in question. For example, an AllJoyn
 *           interface Foo would cause the code generator to create a class
 *           FooProxy that represents the interface at consumer side.
 * \tparam Signal The generated class that represents the signal we're
 *                listening for. For example, if interface Foo has a signal
 *                Bar, the code generator would create a class FooProxy::Bar.
 */
template <typename T, typename Signal> class SignalListener :
    private SignalListenerBase {
  public:
    /** Constructor */
    SignalListener() { }

    /**
     * Callback method, invoked whenever a signal is received.
     *
     * \param[in] signal Object encapsulating the signal's properties (emitter and arguments).
     */
    virtual void OnSignal(const Signal& signal) = 0;

  private:
    virtual void SignalHandler(const ajn::Message& message)
    {
        std::shared_ptr<ObserverBase> obs = observerBase.lock();
        if (obs) {
            Observer<T>* observer = reinterpret_cast<Observer<T>*>(obs.get());
            std::shared_ptr<T> obj = observer->Get(message);
            // Only let the user know if the object was created. Object creation happens upon
            // reception of about signal.
            if (nullptr != obj) {
                Signal s(obj);
                QStatus status = s.Unmarshal(const_cast<ajn::Message&>(message));
                if (ER_OK == status) {
                    OnSignal(s);
                } else {
                    QCC_LogError(status, ("Signal unmarshaling failed"));
                }
            }
        }
    }

    template <typename> friend class Observer;
};
}

#undef QCC_MODULE
#endif /* SIGNALLISTENER_H_ */
