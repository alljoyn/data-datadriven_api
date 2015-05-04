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

#ifndef SIGNALBASE_H_
#define SIGNALBASE_H_

namespace datadriven {
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
}
#undef QCC_MODULE
#endif /* SIGNALBASE_H_ */
