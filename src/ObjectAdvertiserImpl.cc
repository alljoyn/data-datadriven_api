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

#include <algorithm>
#include <alljoyn/about/AboutServiceApi.h>
#include <alljoyn/about/AnnouncementRegistrar.h>

#include "ProvidedObjectImpl.h"

#include "ObjectAdvertiserImpl.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

using namespace ajn;
using namespace ajn::services;
using namespace datadriven;
using namespace qcc;

static const int DATADRIVEN_SERVICE_PORT = 5001;

ObjectAdvertiserImpl::ObjectAdvertiserImpl(BusAttachment* bus,
                                           AboutPropertyStoreImpl* externalStore) :
    providerAsync(this, true),
    busConnection(BusConnectionImpl::GetInstance(bus)),
    errorStatus(ER_FAIL),
    aboutPropertyStore(externalStore),
    ownAboutPropertyStore(aboutPropertyStore == nullptr)
{
    SessionPort sp = DATADRIVEN_SERVICE_PORT;
    SessionOpts opts;

    do {
        errorStatus = busConnection->GetBusAttachment().BindSessionPort(sp, opts, *this);
        if (errorStatus != ER_OK) {
            QCC_LogError(errorStatus, ("Failed to bind session port"));
            break;
        }

        if (ownAboutPropertyStore) {
            aboutPropertyStore = new AboutPropertyStoreImpl();
            AboutServiceApi::Init(busConnection->GetBusAttachment(), *aboutPropertyStore);
            if (AboutServiceApi::getInstance() == nullptr) {
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

            errorStatus = busConnection->GetBusAttachment().RegisterBusObject(*AboutServiceApi::getInstance());
            if (errorStatus != ER_OK) {
                QCC_LogError(errorStatus, ("Failed to register AboutServiceApi bus object"));
                break;
            }
        }

        providerAsync.AsyncTaskQueue::Start();
        errorStatus = ER_OK;
    } while (0);
}

ObjectAdvertiserImpl::~ObjectAdvertiserImpl()
{
    providerAsync.Stop();

    if (ownAboutPropertyStore) {
        if (AboutServiceApi::getInstance() != nullptr) {
            AboutServiceApi::getInstance()->Unregister();
            busConnection->GetBusAttachment().UnregisterBusObject(*AboutServiceApi::getInstance());
        }
    }

    SessionPort sp = DATADRIVEN_SERVICE_PORT;
    busConnection->GetBusAttachment().UnbindSessionPort(sp);

    // We use lock here in case an asynchronous task was started before we stopped the queue
    providersMutex.Lock();
    // Remove providers and related busObjects and cached data
    std::set<std::weak_ptr<ProvidedObjectImpl> >::const_iterator objIt = providers.begin();
    std::set<std::weak_ptr<ProvidedObjectImpl> >::const_iterator objIt_end = providers.end();

    for (; objIt != objIt_end; ++objIt) {
        std::shared_ptr<ProvidedObjectImpl> shobj = objIt->lock();
        if (shobj) {
            // Remove associated busObject from bus
            UnadvertiseBusObject(shobj, shobj->GetInterfaceNames());
        }
    }
    if (ownAboutPropertyStore) {
        if (AboutServiceApi::getInstance() != nullptr) {
            AboutServiceApi::DestroyInstance();
        }
        delete aboutPropertyStore;
    }

    providers.clear();
    providersMutex.Unlock();
}

ajn::BusAttachment& ObjectAdvertiserImpl::GetBusAttachment() const
{
    return busConnection->GetBusAttachment();
}

QStatus ObjectAdvertiserImpl::GetStatus() const
{
    return errorStatus;
}

QStatus ObjectAdvertiserImpl::AddProvidedObject(std::weak_ptr<ProvidedObjectImpl> object)
{
    QStatus result = ER_FAIL;

    std::shared_ptr<ProvidedObjectImpl> shobj = object.lock();
    if (shobj) {
        providersMutex.Lock();
        std::set<std::weak_ptr<ProvidedObjectImpl> >::iterator objIt = providers.find(object);
        if (objIt == providers.end()) {
            result = AdvertiseBusObject(shobj, shobj->GetInterfaceNames());
            if (ER_OK == result) {
                providers.insert(object);
            } else {
                QCC_LogError(result, ("Could not advertise object on the bus"));
            }
        }
        providersMutex.Unlock();
    }
    return result;
}

void ObjectAdvertiserImpl::RemoveProvidedObject(std::weak_ptr<ProvidedObjectImpl> object)
{
    std::shared_ptr<ProvidedObjectImpl> shobj = object.lock();
    if (shobj) {
        providersMutex.Lock();
        std::set<std::weak_ptr<ProvidedObjectImpl> >::iterator objIt = providers.find(object);
        if (objIt != providers.end()) {
            QStatus result = UnadvertiseBusObject(shobj, shobj->GetInterfaceNames());
            if (ER_OK != result) {
                QCC_LogError(result, ("Remove of object from bus failed!"));
            }
            providers.erase(objIt);
        }
        providersMutex.Unlock();
    }
}

void ObjectAdvertiserImpl::CallMethodHandler(std::weak_ptr<ProvidedObjectImpl> object,
                                             ajn::MessageReceiver* ctx,
                                             ajn::MessageReceiver::MethodHandler handler,
                                             const ajn::InterfaceDescription::Member* member,
                                             ajn::Message& message)
{
    providersMutex.Lock();
    std::set<std::weak_ptr<ProvidedObjectImpl> >::iterator objIt = providers.find(object);
    if (objIt != providers.end()) {
        (ctx->*handler)(member, const_cast<ajn::Message&>(message));
    }
    providersMutex.Unlock();
}

void ObjectAdvertiserImpl::ProviderAsyncEnqueue(const Task* task)
{
    providerAsync.Enqueue(task);
}

QStatus ObjectAdvertiserImpl::AboutUpdate(const std::shared_ptr<BusObject> busObject,
                                          const std::vector<String>& interfaceNames,
                                          bool add)
{
    QStatus status = ER_FAIL;
    AboutServiceApi* about = nullptr;

    aboutMutex.Lock(__func__, __LINE__);
    about = AboutServiceApi::getInstance();
    if (add) {
        status = about->AddObjectDescription(busObject->GetPath(), interfaceNames);
    } else {
        status = about->RemoveObjectDescription(busObject->GetPath(), interfaceNames);
    }
    if (status != ER_OK) {
        QCC_LogError(status, ("Failed to  %sadvertise bus object at path '%s'",
                              (add ? "" : "un"), busObject->GetPath()));
    } else {
        status = about->Announce();
        if (status != ER_OK) {
            QCC_LogError(status, ("Failed to announce registered objects"));
        }
    }
    aboutMutex.Unlock(__func__, __LINE__);
    return status;
}

QStatus ObjectAdvertiserImpl::AdvertiseBusObject(std::shared_ptr<BusObject> busObject,
                                                 const std::vector<String>& interfaceNames)
{
    QStatus status = ER_INIT_FAILED;

    status = busConnection->GetBusAttachment().RegisterBusObject(*(busObject.get()));
    if (status != ER_OK) {
        QCC_LogError(status, ("Failed to register bus object at path '%s'", busObject->GetPath()));
    } else {
        status = AboutUpdate(busObject, interfaceNames, true);
    }
    return status;
}

QStatus ObjectAdvertiserImpl::UnadvertiseBusObject(std::shared_ptr<BusObject> busObject,
                                                   const std::vector<String>& interfaceNames)
{
    QStatus status = ER_INIT_FAILED;

    status = AboutUpdate(busObject, interfaceNames, false);
    if (ER_OK == status) {
        busConnection->GetBusAttachment().UnregisterBusObject(*(busObject.get()));
    }
    return status;
}

bool ObjectAdvertiserImpl::AcceptSessionJoiner(ajn::SessionPort sessionPort,
                                               const char* joiner,
                                               const ajn::SessionOpts& opts)
{
    QCC_DbgPrintf(("Accepting session from '%s'", joiner));
    return true;
}

void ObjectAdvertiserImpl::SessionJoined(ajn::SessionPort sessionPort, ajn::SessionId id, const char* joiner)
{
    QCC_DbgPrintf(("Session %lu is joined by '%s'", (unsigned long)id, joiner));
}

void ObjectAdvertiserImpl::OnEmptyQueue()
{
}

void ObjectAdvertiserImpl::OnTask(ajn::services::TaskData const* taskData)
{
    const Task* task = static_cast<const Task*>(taskData);
    task->Execute();
}
