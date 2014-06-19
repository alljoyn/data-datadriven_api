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

#ifndef SIGNAL_H_
#define SIGNAL_H_

#include <datadriven/datadriven.h>
#include <qcc/Debug.h>
#include "SignalImpl.h"

#include <iostream>
#include <memory>
#include <cassert>

#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
template <typename T> class Observer;

/**
 * \class SignalBase
 * \brief Encapsulates a signal that is emitted by an object on the bus.
 *
 * This is a templated base class that represents signals emitted by an
 * observed object that implements interface T. The code generator will derive
 * a specific subclass for each signal that is declared in interface T.
 *
 * You should never interact with an object of this class directly, always
 * use the subclasses created by the code generator. The only method in this
 * class that is relevant to application developers is SignalBase::GetEmitter.
 *
 * \tparam T The generated proxy class that corresponds to the AllJoyn interface
 *           that contains the signal in question. For example, an AllJoyn
 *           interface Foo with a single signal Bar would cause the code generator
 *           to create a class FooProxy that represents the interface itself, and
 *           a class FooProxy::Bar (derived from SignalBase<FooProxy>) to represent
 *           the Bar signal.
 */
template <typename T> class SignalBase {
  public:
    /**
     * Destructor.
     */
    virtual ~SignalBase() { }

    /**
     * Retrieve the observed object ('emitter') this signal was emitted from.
     * \return Shared pointer to the observed object
     */
    std::shared_ptr<T> GetEmitter() const
    {
        return emitter;
    }

  protected:
    /** \private
     * Constructor.
     * \param[in] emitter Shared pointer to the observed object
     */
    SignalBase(const std::shared_ptr<T>& emitter) :
        emitter(emitter) { }

    /** \private
     * To be implemented by derived class to unmarshal the message from the signal into properties.
     *
     * Abstract method.
     *
     * \param[in] message Communication message to be unmarshaled
     * \retval ER_OK on success
     * \retval others on failure
     */
    virtual QStatus Unmarshal(ajn::Message& message) = 0;

  private:
    std::shared_ptr<T> emitter;
};

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
    private SignalListenerImpl {
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
        assert(observerimpl != NULL);

        Observer<T>* observer = reinterpret_cast<Observer<T>*>(observerimpl);
        std::shared_ptr<T> obj = observer->Get(message);
        assert(obj);
        Signal s(obj);
        QStatus status = s.Unmarshal(const_cast<ajn::Message&>(message));
        if (ER_OK == status) {
            OnSignal(s);
        } else {
            QCC_LogError(status, ("Signal unmarshaling failed"));
        }
    }

    template <typename> friend class Observer;

    void SetObserver(Observer<T>& observer)
    {
        this->observerimpl = reinterpret_cast<ObserverImpl*>(&observer);
    }

    void SetMember(ajn::InterfaceDescription::Member* _member)
    {
        this->member = _member;
    }
};
}

#undef QCC_MODULE
#endif /* SIGNAL_H_ */
