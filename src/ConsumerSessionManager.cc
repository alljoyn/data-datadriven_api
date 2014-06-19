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

#include "ConsumerSessionManager.h"
#include "ConsumerSessionManagerImpl.h"
#include <qcc/String.h>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

using namespace datadriven;

ConsumerSessionManager::ConsumerSessionManager(const char* _appName) :
    consumerSessionManagerImpl(new ConsumerSessionManagerImpl(_appName))
{
}

ConsumerSessionManager::~ConsumerSessionManager()
{
}

QStatus ConsumerSessionManager::GetStatus() const
{
    return consumerSessionManagerImpl->GetStatus();
}

QStatus ConsumerSessionManager::RegisterObserver(ObserverRef observer, const qcc::String& interfaceName)
{
    return consumerSessionManagerImpl->RegisterObserver(observer, interfaceName);
}

QStatus ConsumerSessionManager::UnregisterObserver(ObserverRef observer)
{
    return consumerSessionManagerImpl->UnregisterObserver(observer);
}

QStatus ConsumerSessionManager::GetObjectIdsForObserver(ObserverRef rr, iterateObjectIdCb iterate, void* ctx)
{
    return consumerSessionManagerImpl->GetObjectIdsForObserver(rr, iterate, ctx);
}

void ConsumerSessionManager::RegisterInterfaceListener(ConsumerInterfaceListener* intfListener)
{
    consumerSessionManagerImpl->RegisterInterfaceListener(intfListener);
}

ajn::BusAttachment& ConsumerSessionManager::GetBusAttachment()
{
    return consumerSessionManagerImpl->GetBusAttachment();
}
