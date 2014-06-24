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

#include "InterestReceiver.h"
#include "ProviderSessionManagerImpl.h"
#include "BusConnectionImpl.h"
#include "ProviderCache.h"
#include <qcc/String.h>
#include <qcc/Debug.h>

#define QCC_MODULE "DD_COMMON"

using namespace ajn;
using namespace datadriven;
using namespace qcc;

const char* InterestReceiver::DATADRIVEN_INTEREST_INTF = "org.alljoyn.datadriven.interest_receiver";
const char* InterestReceiver::DATADRIVEN_INTEREST_PATH = "/org/alljoyn/datadriven/interest_receiver";
const char* InterestReceiver::REGISTER_INTEREST_METHOD_CALL = "RegisterInterest";
const char* InterestReceiver::UNREGISTER_INTEREST_METHOD_CALL = "UnregisterInterest";
const char* InterestReceiver::DATADRIVEN_PREFIX = "org.alljoyn.datadriven.A";  /* append A to make sure it starts with a letter (AJ does not like it to start with a number) */

QStatus InterestReceiver::CreateInterface(ajn::BusAttachment& bus, ajn::InterfaceDescription*& intf)
{
    QStatus status = bus.CreateInterface(DATADRIVEN_INTEREST_INTF, intf);
    if (ER_OK != status) {
        QCC_LogError(status, ("Failed to create interface '%s' on provider bus attachment", DATADRIVEN_INTEREST_INTF));
        return status;
    }
    intf->AddMethod(REGISTER_INTEREST_METHOD_CALL, "s",  "", "inStr1", 0);
    intf->AddMethod(UNREGISTER_INTEREST_METHOD_CALL, "s",  "", "inStr1", 0);
    intf->Activate();

    return ER_OK;
}

InterestReceiver::InterestReceiver(ProviderSessionManagerImpl& _providerSessionManagerImpl,
                                   const ajn::InterfaceDescription& intf) :
    BusObject(DATADRIVEN_INTEREST_PATH), providerSessionManagerImpl(&_providerSessionManagerImpl),
    errorStatus(ER_FAIL)
{
    AddInterface(intf);

    const MethodEntry methodEntries[] = {
        { intf.GetMember(REGISTER_INTEREST_METHOD_CALL),
          static_cast<MessageReceiver::MethodHandler>(&InterestReceiver::RegisterInterestMethodCallImpl) },
        { intf.GetMember(UNREGISTER_INTEREST_METHOD_CALL),
          static_cast<MessageReceiver::MethodHandler>(&InterestReceiver::UnregisterInterestMethodCallImpl) },
    };

    errorStatus = AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(methodEntries[0]));
    if (ER_OK != errorStatus) {
        QCC_LogError(errorStatus, ("Failed to register method handlers for interest receiver"));
    }
}

InterestReceiver::~InterestReceiver()
{
}

QStatus InterestReceiver::GetStatus()
{
    return errorStatus;
}

void InterestReceiver::RegisterInterestMethodCallImpl(const ajn::InterfaceDescription::Member* member,
                                                      ajn::Message& message)
{
    if (ER_OK != errorStatus) {
        QCC_LogError(errorStatus, ("Provider session manager interest receiver not properly initialized"));
        return;
    }

    const String& intfName = message->GetArg(0)->v_string.str;
    const SessionId& sessionId = message->GetSessionId();
    QCC_DbgHLPrintf(("Session %lu is interested in interface '%s'", (unsigned long)sessionId, intfName.c_str()));

    providerSessionManagerImpl->mutex.Lock(__func__, __LINE__);
    ProviderSessionManagerImpl::InterfacesPerSessionMap::iterator it =
        providerSessionManagerImpl->registeredInterfaces.find(sessionId);

    if (it == providerSessionManagerImpl->registeredInterfaces.end()) {
        std::set<String> interfaces;
        interfaces.insert(intfName);
        providerSessionManagerImpl->registeredInterfaces.insert(std::pair<SessionId, std::set<String> >(sessionId,
                                                                                                        interfaces));
        QCC_DbgPrintf(("This is the first thing the session is interested in"));
    } else {
        std::set<String>& interfaces = it->second;
        interfaces.insert(intfName);
        QCC_DbgPrintf(("The session is also interested in other interfaces"));
    }

    /* For each object matching this interface, we need to Signal() the update */
    ProviderCache* cache = providerSessionManagerImpl->busConnectionImpl->GetProviderCache(intfName);
    providerSessionManagerImpl->mutex.Unlock(__func__, __LINE__);

    if (NULL == cache) {
        QCC_LogError(errorStatus, ("Provider cache not properly initialized"));

        return;
    }

    ProviderCache::Cache contents = cache->GetAll();
    ProviderCache::Cache::iterator cit = contents.begin();
    ProviderCache::Cache::iterator endcit = contents.end();
    for (; cit != endcit; ++cit) {
        ProvidedObject* const& obj = cit->first;
        ProviderCache::Properties& args = cit->second;
        obj->Signal(NULL, sessionId, *cache->GetPropUpdateSignalMember(),
                    &args[0], args.size(), 0, ajn::ALLJOYN_FLAG_GLOBAL_BROADCAST);
    }
}

void InterestReceiver::UnregisterInterestMethodCallImpl(const ajn::InterfaceDescription::Member* member,
                                                        ajn::Message& message)
{
    if (ER_OK != errorStatus) {
        QCC_LogError(errorStatus, ("Provider Session Manager Interest Receiver not properly initialized"));
        return;
    }

    const String& intfName = message->GetArg(0)->v_string.str;
    const SessionId& sessionId = message->GetSessionId();
    QCC_DbgHLPrintf(("Session %lu is no longer interested in interface '%s'", (unsigned long)sessionId, intfName.c_str()));

    providerSessionManagerImpl->mutex.Lock(__func__, __LINE__);
    ProviderSessionManagerImpl::InterfacesPerSessionMap::iterator it =
        providerSessionManagerImpl->registeredInterfaces.find(sessionId);

    if (it != providerSessionManagerImpl->registeredInterfaces.end()) {
        std::set<String>& interfaces = it->second;
        interfaces.erase(intfName);
        if (interfaces.size() == 0) {
            providerSessionManagerImpl->registeredInterfaces.erase(it);
        }
    }
    providerSessionManagerImpl->mutex.Unlock(__func__, __LINE__);
}
