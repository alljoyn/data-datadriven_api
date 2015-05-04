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

#ifndef METHODREPLYLISTENER_H_
#define METHODREPLYLISTENER_H_

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
/**
 * \class MethodReplyListener
 *
 * Application developers should derive from this abstract class to receive
 * notifications when a specific method on a specific AllJoyn interface receives
 * a reply
 *
 * \tparam T The generated class that represents the method reply we're
 *           listening for. For example, if interface Foo has a method
 *           Bar, the code generator would create a class FooProxy::BarReply.
 */
template <typename T> class MethodReplyListener {
  public:
    virtual ~MethodReplyListener() { }

    /**
     * Callback method, invoked whenever a method reply is received.
     *
     * \param[in] reply Object encapsulating the methods reply properties.
     */
    virtual void OnReply(const T& reply) = 0;
};
}

#endif /* METHODREPLYLISTENER_H_ */
