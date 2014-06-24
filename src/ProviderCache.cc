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

#include "ProviderCache.h"
#include <qcc/Mutex.h>

#include <map>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

using namespace ajn;
using namespace datadriven;
using namespace std;

ProviderCache::ProviderCache(const RegisteredTypeDescription& td,
                             ProviderSessionManager& psm) :
    typedesc(td), providersessionmanager(psm)
{
    interface = typedesc.GetDescription().GetName();
}

size_t ProviderCache::GetElementCount() const
{
    mutex.Lock();
    size_t size = cache.size();
    mutex.Unlock();
    return size;
}

ProviderCache::Cache ProviderCache::GetAll()
{
    //TODO this is horribly inefficient: it copies the complete cache
    //     I chose this approach for now because it radically simplifies
    //     all locking issues.
    mutex.Lock();
    Cache copy = cache;
    mutex.Unlock();
    return copy;
}

const InterfaceDescription::Member* ProviderCache::GetPropUpdateSignalMember() const
{
    mutex.Lock();
    /* the properties update signal is always member 0 */
    const InterfaceDescription::Member* sigmember = &(typedesc.GetMember(0));
    mutex.Unlock();
    return sigmember;
}

QStatus ProviderCache::Update(ProvidedObject* obj, const Properties& prop)
{
    mutex.Lock();
    QStatus status = ER_OK;
    Properties copy = cache[obj] = prop;
    const InterfaceDescription::Member* sigmember = GetPropUpdateSignalMember();
    status = providersessionmanager.BusObjectSignal(*obj, interface, sigmember, &copy[0], copy.size());
    mutex.Unlock();
    return status;
}

void ProviderCache::Remove(ProvidedObject* obj)
{
    mutex.Lock();
    cache.erase(obj);
    mutex.Unlock();
}
