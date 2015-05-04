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

#include <algorithm>
#include <alljoyn/AboutData.h>
#include <alljoyn/AboutObj.h>

#include "ProvidedObjectImpl.h"
#include "ObjectAdvertiserImpl.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

using namespace ajn;
using namespace datadriven;
using namespace qcc;

ObjectAdvertiserImpl::ObjectAdvertiserImpl(BusAttachment* bus,
                                           AboutData* _aboutData,
                                           ajn::AboutObj* _aboutObj,
                                           SessionOpts* _opts,
                                           SessionPort sp) :
    providerAsync(this, true),
    busConnection(BusConnectionImpl::GetInstance(bus)),
    errorStatus(ER_FAIL),
    aboutData(_aboutData),
    aboutObj(_aboutObj),
    opts(_opts)
{
    do {
        if (aboutData && aboutObj && opts) {
            ownAboutLogic = false;
        } else if (!aboutData && !aboutObj && !opts) {
            ownAboutLogic = true;
        } else {
            QCC_LogError(errorStatus,
                         ("All relevant About elements must be provided if at least one is indeed provided"));
            break;
        }

        // We populate an incomplete trivial about data
        if (ownAboutLogic) {
            aboutData = new AboutData("en");
            aboutObj = new AboutObj(busConnection->GetBusAttachment());
            opts = new SessionOpts();
            uint8_t appId[] = { 0x53, 0x61, 0x79, 0x20, 0x68, 0x65, 0x6c, 0x6c,
                                0x6f, 0x20, 0x74, 0x6f, 0x20, 0x51, 0x65, 0x6f };
            aboutData->SetAppId(appId, 16);
            aboutData->SetDeviceName("QEO Advertiser");
            //DeviceId is a string encoded 128bit UUID
            aboutData->SetDeviceId("2fd25710-ee7b-11e4-90ec-1681e6b88ec1");
            aboutData->SetAppName("QEO Advertiser");
            aboutData->SetManufacturer("QEO LLC");
            aboutData->SetModelNumber("181180");
            aboutData->SetDescription("QEO Advertiser");
            aboutData->SetDateOfManufacture("2014-03-24");
            aboutData->SetSoftwareVersion("1.0.0");
            aboutData->SetHardwareVersion("1.0.0");
            aboutData->SetSupportUrl(
                "https://allseenalliance.org/developers/learn/ddapi");
        }

        if (!(aboutData->IsValid())) {
            QCC_LogError(ER_FAIL, ("Invalid about data"));
            break;
        }
        errorStatus = busConnection->GetBusAttachment().BindSessionPort(sp, *opts, *this);
        if (errorStatus != ER_OK) {
            QCC_LogError(errorStatus, ("Failed to bind session port"));
            break;
        }
        providerAsync.AsyncTaskQueue::Start();
        errorStatus = ER_OK;
    } while (0);
}

ObjectAdvertiserImpl::~ObjectAdvertiserImpl()
{
    providerAsync.Stop();

    SessionPort sp = DATADRIVEN_SERVICE_PORT;

    // We use lock here in case an asynchronous task was started before we stopped the queue
    providersMutex.Lock();
    // Remove providers and related busObjects and cached data
    std::set<std::weak_ptr<ProvidedObjectImpl> >::const_iterator objIt = providers.begin();
    std::set<std::weak_ptr<ProvidedObjectImpl> >::const_iterator objIt_end = providers.end();

    for (; objIt != objIt_end; ++objIt) {
        std::shared_ptr<ProvidedObjectImpl> shobj = objIt->lock();
        if (shobj) {
            // Remove associated busObject from bus
            UnadvertiseBusObject(shobj);
        }
    }
    if (ownAboutLogic) {
        delete opts;
        delete aboutObj;
        delete aboutData;
    }

    busConnection->GetBusAttachment().UnbindSessionPort(sp);

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
            result = AdvertiseBusObject(shobj);
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
            QStatus result = UnadvertiseBusObject(shobj);
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

QStatus ObjectAdvertiserImpl::AdvertiseBusObject(std::shared_ptr<BusObject> busObject)
{
    QStatus status = ER_INIT_FAILED;

    status = busConnection->GetBusAttachment().RegisterBusObject(*(busObject.get()));
    if (status != ER_OK) {
        QCC_LogError(status, ("Failed to register bus object at path '%s'", busObject->GetPath()));
    } else {
        aboutMutex.Lock(MUTEX_CONTEXT);
        status = aboutObj->Announce(DATADRIVEN_SERVICE_PORT, *aboutData);
        if (status != ER_OK) {
            QCC_LogError(status, ("Failed to announce registered object(s)"));
        }
        aboutMutex.Unlock(MUTEX_CONTEXT);
    }
    return status;
}

QStatus ObjectAdvertiserImpl::UnadvertiseBusObject(std::shared_ptr<BusObject> busObject)
{
    QStatus status = ER_INIT_FAILED;

    busConnection->GetBusAttachment().UnregisterBusObject(*(busObject.get()));
    aboutMutex.Lock(MUTEX_CONTEXT);
    status = aboutObj->Announce(DATADRIVEN_SERVICE_PORT, *aboutData);
    if (status != ER_OK) {
        QCC_LogError(status, ("Failed to unannounce previously registered object(s)"));
    }
    aboutMutex.Unlock(MUTEX_CONTEXT);

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

void ObjectAdvertiserImpl::OnTask(TaskData const* taskData)
{
    const Task* task = static_cast<const Task*>(taskData);
    task->Execute();
}
