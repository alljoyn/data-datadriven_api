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

#include "ProviderSessionManager.h"
#include "ProviderSessionManagerImpl.h"
#include <algorithm>
#include <alljoyn/about/AboutServiceApi.h>
#include <alljoyn/about/AnnouncementRegistrar.h>
#include <qcc/GUID.h>
#include "ProviderCache.h"
#include "BusConnectionImpl.h"
#include "InterestReceiver.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

using namespace ajn;
using namespace ajn::services;
using namespace datadriven;
using namespace qcc;

ProviderSessionManager::ProviderSessionManager(const char* _appName,
                                               BusConnectionImpl* busConnectionImpl) :
    providerSessionManagerImpl(new ProviderSessionManagerImpl(_appName, busConnectionImpl))
{
}

ProviderSessionManager::~ProviderSessionManager()
{
}

BusAttachment& ProviderSessionManager::GetBusAttachment()
{
    return providerSessionManagerImpl->GetBusAttachment();
}

QStatus ProviderSessionManager::GetStatus() const
{
    return providerSessionManagerImpl->GetStatus();
}

QStatus ProviderSessionManager::AdvertiseBusObject(ProvidedObject& busObject)
{
    return providerSessionManagerImpl->AdvertiseBusObject(busObject);
}

QStatus ProviderSessionManager::RemoveBusObject(ProvidedObject& busObject)
{
    return providerSessionManagerImpl->RemoveBusObject(busObject);
}

QStatus ProviderSessionManager::BusObjectSignal(ProvidedObject& busObject,
                                                const qcc::String& interfaceName,
                                                const InterfaceDescription::Member* member,
                                                const MsgArg* msgArg,
                                                size_t len)
{
    return providerSessionManagerImpl->BusObjectSignal(busObject,
                                                       interfaceName,
                                                       member,
                                                       msgArg,
                                                       len);
}
