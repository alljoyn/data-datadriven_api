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

#ifndef CONSUMERMETHODREPLY_H_
#define CONSUMERMETHODREPLY_H_

#include <alljoyn/Status.h>
#include <alljoyn/Message.h>

#define QCC_MODULE "DD_CONSUMER"
namespace datadriven {
/**
 * \class ConsumerMethodReply
 * \brief Encapsulates a method response.
 *
 * This is an abstract base class that represents the general concept of a
 * method reply. The code generator will derive concrete subclasses for each
 * method in an AllJoyn interface. For example, for an AllJoyn interface Foo
 * that contains a method Bar, the code generator will create a class FooProxy
 * that represents objects implementing Foo at consumer side, and a class
 * FooProxy::BarReply that represents a method reply received upon invocation
 * of Foo::Bar().
 *
 * You should never interact with an object of this class directly. Always use
 * the concrete subclasses created by the code generator. The only method in
 * this class that is relevant to application developers is
 * ConsumerMethodReply::GetStatus.
 */
class ConsumerMethodReply {
  public:
    /**
     * Destructor.
     */
    virtual ~ConsumerMethodReply();

    /**
     * Check whether the method call has successfully concluded.
     * \retval ER_OK Success.
     * \retval ER_BUS_REPLY_IS_ERROR_MESSAGE The MethodReply send an error
     *                string. The error string can be fetched by calling GetErrorName()
     *                and the error description by calling GetErrorDescription().
     * \retval others The invocation of the method call failed. The actual
     *                error code provides more insight on the reason of the
     *                failure. This error is either caused by a framework issue
     *                or by a QStatus code send in the MethodReply.
     */
    QStatus GetStatus() const;

    /**
     * \brief Get the method reply errorName.
     *
     * \return Error string or empty string if no error
     */
    const qcc::String GetErrorName();

    /**
     * \brief Get the method reply errorDescription
     *
     * \return Empty string on no error or error description on error
     */
    const qcc::String GetErrorDescription();

  protected:
    /** \private
     * Unmarshal communication message to extract relevant information
     *
     * Abstract method.
     *
     * \param[in] msgarg  Array of message arguments received from the
     *                    underlying communication layer
     * \param[in] numArgs Number of message arguments received from the
     *                    underlying communication layer
     * \retval ER_OK on success
     * \retval others on failure
     */
    virtual QStatus Unmarshal(const ajn::MsgArg* msgarg,
                              size_t numArgs) = 0;

    /** \private
     * Set the status of the method reply (OK, various failure reasons)
     * \param[in] status The integrity state
     */
    void SetStatus(const QStatus status);

    /** \private
     * Set the errorName received in the MethodReply
     * \param[in] errorName the errorName string
     */
    void SetErrorName(qcc::String errorName);

    /** \private
     * Set the errorDescription received in the MethodReply
     * \param[in] errorDescription the errorDescription string
     */
    void SetErrorDescription(qcc::String errorDescription);

  private:
    /** Status */
    QStatus status;
    qcc::String errorName;
    qcc::String errorDescription;

    friend class MethodInvocationBase;
};

/**
 * \class ConsumerMethodNoReply
 * \brief Empty method response for fire-and-forget method calls.
 */
class ConsumerMethodNoReply :
    public ConsumerMethodReply {
  protected:
    virtual QStatus Unmarshal(const ajn::MsgArg* msgarg,
                              size_t numArgs)
    {
        return ER_OK;
    }
};
}

#undef QCC_MODULE
#endif /* CONSUMERMETHODREPLY_H_ */
