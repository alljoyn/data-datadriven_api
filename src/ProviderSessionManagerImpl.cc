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

ProviderSessionManagerImpl::ProviderSessionManagerImpl(const char* _appName,
                                                       BusConnectionImpl* busConnectionImpl) :
    busConnectionImpl(busConnectionImpl),
    errorStatus(ER_FAIL),
    serverBusAttachment(_appName, true)
{
    SessionPort sp = ProviderSessionManager::DATADRIVEN_SERVICE_PORT;
    SessionOpts opts;

    InterfaceDescription* serverintf;

    do {
        if (NULL == busConnectionImpl) {
            QCC_LogError(errorStatus, ("Provider session manager, bus connection is NULL"));
            break;
        }

        if (NULL == _appName) {
            errorStatus = ER_FAIL;
            QCC_LogError(errorStatus, ("Provider session manager, provided application name is invalid"));
            break;
        }

        errorStatus = serverBusAttachment.Start();
        if (errorStatus != ER_OK) {
            QCC_LogError(errorStatus, ("Failed to start bus attachment"));
            break;
        }

        errorStatus = serverBusAttachment.Connect();
        if (errorStatus != ER_OK) {
            QCC_LogError(errorStatus, ("Failed to connect bus attachment"));
            break;
        }

        errorStatus = InterestReceiver::CreateInterface(serverBusAttachment, serverintf);
        if (errorStatus != ER_OK) {
            QCC_LogError(errorStatus, ("Failed to create interest receiver interface"));
            break;
        }

        ir = std::unique_ptr<InterestReceiver>(new InterestReceiver(*this, *serverintf));
        if (ir->GetStatus() != ER_OK) {
            QCC_LogError(ir->GetStatus(), ("failed to allocate interest receiver"));
            break;
        }

        errorStatus = serverBusAttachment.RegisterBusObject(*ir);
        if (errorStatus != ER_OK) {
            QCC_LogError(errorStatus, ("Failed to register interest receiver"));
            break;
        }

        errorStatus = serverBusAttachment.BindSessionPort(sp, opts, *this);
        if (errorStatus != ER_OK) {
            QCC_LogError(errorStatus, ("Failed to bind session port"));
            break;
        }

        /* Construct WKN */
        qcc::GUID128 guid;
        const qcc::String& guidstring = guid.ToShortString();
        qcc::String wkn = InterestReceiver::DATADRIVEN_PREFIX + guidstring;
        QCC_DbgPrintf(("Requesting WKN '%s'", wkn.c_str()));
        const uint32_t flags = DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE;
        errorStatus = serverBusAttachment.RequestName(wkn.c_str(), flags);
        if (errorStatus != ER_OK) {
            QCC_LogError(errorStatus, ("Failed to request WKN"));
            break;
        }

        QCC_DbgPrintf(("Advertising WKN '%s'", wkn.c_str()));
        errorStatus = serverBusAttachment.AdvertiseName(wkn.c_str(), TRANSPORT_ANY);
        if (errorStatus != ER_OK) {
            QCC_LogError(errorStatus, ("Failed to advertise WKN"));
            break;
        }

        errorStatus = aboutPropertyStore.setAppName(wkn); /* abuse appname .. */
        if (errorStatus != ER_OK) {
            QCC_LogError(errorStatus, ("Failed to set AppName property in the About store"));
            break;
        }

        AboutServiceApi::Init(serverBusAttachment, aboutPropertyStore);
        if (AboutServiceApi::getInstance() == NULL) {
            QCC_LogError(ER_FAIL, ("Failed to initialize AboutServiceApi"));
            errorStatus = ER_FAIL;
            break;
        }

        errorStatus = AboutServiceApi::getInstance()->Register(DATADRIVEN_SERVICE_PORT);
        if (errorStatus != ER_OK) {
            QCC_LogError(errorStatus,
                         ("Failed to register AboutServiceApi for port %lu", (unsigned long)DATADRIVEN_SERVICE_PORT));
            break;
        }

        errorStatus = serverBusAttachment.RegisterBusObject(*AboutServiceApi::getInstance());
        if (errorStatus != ER_OK) {
            QCC_LogError(errorStatus, ("Failed to register AboutServiceApi bus object"));
            break;
        }

        errorStatus = ER_OK;
    } while (0);
}

ProviderSessionManagerImpl::~ProviderSessionManagerImpl()
{
    if (AboutServiceApi::getInstance() != NULL) {
        AboutServiceApi::getInstance()->Unregister();
        serverBusAttachment.UnregisterBusObject(*AboutServiceApi::getInstance());
        AboutServiceApi::DestroyInstance();
    }

    SessionPort sp = ProviderSessionManager::DATADRIVEN_SERVICE_PORT;
    serverBusAttachment.UnbindSessionPort(sp);

    if (ir != nullptr) {
        serverBusAttachment.UnregisterBusObject(*ir);
    }

    serverBusAttachment.Disconnect();
    serverBusAttachment.Stop();
    serverBusAttachment.Join();
}

BusAttachment& ProviderSessionManagerImpl::GetBusAttachment()
{
    return serverBusAttachment;
}

QStatus ProviderSessionManagerImpl::_GetStatus(bool lock, bool trace) const
{
    if (lock) {
        mutex.Lock(__func__, __LINE__);
    }
    QStatus ret = errorStatus;
    if (trace && (ER_OK != ret)) {
        QCC_LogError(errorStatus, ("Failed to initialize provider session manager"));
    }
    if (lock) {
        mutex.Unlock(__func__, __LINE__);
    }
    return ret;
}

QStatus ProviderSessionManagerImpl::GetStatus() const
{
    return _GetStatus(true, false);
}

QStatus ProviderSessionManagerImpl::AboutUpdate(const ProvidedObject& busObject,
                                                bool add)
{
    QStatus status = ER_FAIL;
    AboutServiceApi* about = NULL;
    std::vector<String> interfaces;

    busObject.GetInterfaceNames(interfaces);
    aboutMutex.Lock(__func__, __LINE__);
    about = AboutServiceApi::getInstance();
    if (add) {
        status = about->AddObjectDescription(busObject.GetPath(), interfaces);
    } else {
        status = about->RemoveObjectDescription(busObject.GetPath(), interfaces);
    }
    if (status != ER_OK) {
        QCC_LogError(status, ("Failed to  %sadvertise bus object at path '%s'",
                              (add ? "" : "un"), busObject.GetPath()));
    } else {
        status = about->Announce();
        if (status != ER_OK) {
            QCC_LogError(status, ("Failed to announce registered objects"));
        }
    }
    aboutMutex.Unlock(__func__, __LINE__);
    return status;
}

QStatus ProviderSessionManagerImpl::AdvertiseBusObject(ProvidedObject& busObject)
{
    QStatus status = ER_FAIL;

    status = _GetStatus();
    if (ER_OK == status) {
        status = serverBusAttachment.RegisterBusObject(busObject);
        if (status != ER_OK) {
            QCC_LogError(status, ("Failed to register bus object at path '%s'", busObject.GetPath()));
        } else {
            status = AboutUpdate(busObject, true);
        }
    }
    return status;
}

QStatus ProviderSessionManagerImpl::RemoveBusObject(ProvidedObject& busObject)
{
    QStatus status = ER_OK;

    status = _GetStatus();
    if (ER_OK == status) {
        status = AboutUpdate(busObject, false);
        if (ER_OK == status) {
            serverBusAttachment.UnregisterBusObject(busObject);
        }
    }
    return status;
}

QStatus ProviderSessionManagerImpl::BusObjectSignal(ProvidedObject& busObject,
                                                    const qcc::String& interfaceName,
                                                    const InterfaceDescription::Member* member,
                                                    const MsgArg* msgArg, size_t len)
{
    QStatus status = ER_OK;

    mutex.Lock(__func__, __LINE__);
    status = _GetStatus(false);
    if (ER_OK == status) {
        /* Trigger signal to all sessions (that care) */
        for (InterfacesPerSessionMap::const_iterator it = registeredInterfaces.begin();
             it != registeredInterfaces.end();
             ++it) {
            const SessionId& sessionId = it->first;
            const std::set<qcc::String>& interfaces = it->second;
            if (interfaces.find(interfaceName) != interfaces.end()) {
                /* trigger signal ! */
                QCC_DbgPrintf(("Emitting signal '%s' in interface '%s' for bus object at path '%s'",
                               member->name.c_str(), interfaceName.c_str(), busObject.GetPath()));

                status = busObject.Signal(NULL /*broadcast in session*/,
                                          sessionId, *member, msgArg, len, 0, ajn::ALLJOYN_FLAG_GLOBAL_BROADCAST);
                if (status != ER_OK) {
                    QCC_LogError(status,
                                 ("Failed to emit signal '%s' in interface '%s' for bus object at path '%s'",
                                  member->name.c_str(), interfaceName.c_str(), busObject.GetPath()));
                    break;
                }
            }
        }
    }
    mutex.Unlock(__func__, __LINE__);
    return status;
}

void ProviderSessionManagerImpl::SessionLost(ajn::SessionId sessionId, SessionLostReason reason)
{
    QCC_DbgPrintf(("Lost session %lu", (unsigned long)sessionId));

    mutex.Lock(__func__, __LINE__);
    if (ER_OK == _GetStatus(false)) {
        InterfacesPerSessionMap::iterator it = registeredInterfaces.find(sessionId);
        if (it != registeredInterfaces.end()) {
            registeredInterfaces.erase(it);
        }
    }
    mutex.Unlock(__func__, __LINE__);
}

bool ProviderSessionManagerImpl::AcceptSessionJoiner(ajn::SessionPort sessionPort,
                                                     const char* joiner,
                                                     const ajn::SessionOpts& opts)
{
    QCC_DbgPrintf(("Accepting session from '%s'", joiner));
    return true;
}

void ProviderSessionManagerImpl::SessionJoined(ajn::SessionPort sessionPort, ajn::SessionId id, const char* joiner)
{
    QStatus status = ER_FAIL;

    QCC_DbgPrintf(("Session %lu is joined by '%s'", (unsigned long)id, joiner));
    status = _GetStatus();
    if (ER_OK == status) {
        status = serverBusAttachment.SetSessionListener(id, this);
        if (status != ER_OK) {
            QCC_LogError(status, ("Failed to attach session listener to newly joined session"));
        }
    }
}
