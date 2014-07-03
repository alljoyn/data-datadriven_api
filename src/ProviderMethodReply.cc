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

#include <datadriven/ProviderMethodReply.h>
#include "ProvidedObjectImpl.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

namespace datadriven {
ProviderMethodReply::ProviderMethodReply(std::shared_ptr<ProvidedObjectImpl> providedObject,
                                         ajn::Message& message) :
    providedObject(providedObject), message(message)
{
}

ProviderMethodReply::~ProviderMethodReply()
{
}

QStatus ProviderMethodReply::MethodReply(const ajn::MsgArg* args, size_t numArgs)
{
    QStatus status = providedObject->MethodReply(message, args, numArgs);

    if (ER_OK != status) {
        QCC_LogError(status, ("Failed to send method reply"));
    }
    return status;
}

QStatus ProviderMethodReply::SendError(const qcc::String& error, const qcc::String& errorMessage)
{
    QStatus status = providedObject->MethodReplyError(message, error.c_str(), errorMessage.c_str());

    if (ER_OK != status) {
        QCC_LogError(status, ("Failed to send method error reply"));
    }
    return status;
}

QStatus ProviderMethodReply::SendErrorCode(QStatus status)
{
    QStatus stat = providedObject->MethodReplyErrorCode(message, status);

    if (ER_OK != stat) {
        QCC_LogError(stat, ("Failed to send method error reply"));
    }
    return stat;
}
}
