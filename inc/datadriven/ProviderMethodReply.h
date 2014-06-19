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

#ifndef PROVIDERMETHODREPLY_H_
#define PROVIDERMETHODREPLY_H_

#include <alljoyn/Message.h>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

namespace datadriven {
class ProvidedObject;

/**
 * \class ProviderMethodReply
 * \brief Encapsulates a method response to be send back to the method invoker.
 *
 * Base class for a Reply object that encapsulates an invoked method response, which
 * needs to be send back to the method invoker via the underlying communication layer.\
 *
 * A derived object of this class will be constructed in the generated code.
 */
class ProviderMethodReply {
  public:
    /** Internal cleanup */
    virtual ~ProviderMethodReply();

  protected:
    /**
     * Initializes the object
     * \param providedObject ProvidedObject on which the response is send back
     * \param message Marshaled <em>response content</em> to be send back
     */
    ProviderMethodReply(ProvidedObject& providedObject,
                        ajn::Message& message);

    /**
     * Sends a marshaled response back to the method invoker via the underlying communication layer.
     * \param args List of input arguments to be taken by the returned response.
     * \param numArgs Number of input arguments.
     */
    QStatus MethodReply(const ajn::MsgArg* msgarg = NULL,
                        size_t numArgs = 0);

  private:
    ProvidedObject& providedObject;
    ajn::Message& message;
};
}

#undef QCC_MODULE
#endif /* PROVIDERMETHODREPLY_H_ */
