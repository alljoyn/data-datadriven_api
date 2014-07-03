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

#include <datadriven/SignalListener.h>
#include <datadriven/ProvidedInterface.h>
#include "BusConnectionImpl.h"
#include "ProvidedObjectImpl.h"
#include "RegisteredTypeDescription.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_COMMON"

namespace datadriven {
using namespace std;
using namespace ajn;
using namespace ajn::services;

shared_ptr<BusConnectionImpl> BusConnectionImpl::GetInstance(BusAttachment* ba)
{
    static qcc::Mutex mutex;
    static weak_ptr<BusConnectionImpl> instance;
    shared_ptr<BusConnectionImpl> sharedInstance = nullptr;

    mutex.Lock();
    sharedInstance = instance.lock();
    if (!sharedInstance) {
        sharedInstance = shared_ptr<BusConnectionImpl>(new BusConnectionImpl(ba));
        if (ER_OK != sharedInstance->GetStatus()) {
            sharedInstance = nullptr;
        }
        instance = sharedInstance;
    }
    mutex.Unlock();
    return sharedInstance;
}

BusConnectionImpl::BusConnectionImpl(BusAttachment* _ba) :
    status(ER_OK),
    ba(_ba),
    ownBa(NULL == _ba)
{
    if (ba == NULL) {
        ba = new BusAttachment(NULL, true);
        status = ba->Start();
        if (status != ER_OK) {
            QCC_LogError(status, ("Failed to start bus attachment"));
        } else {
            status = ba->Connect();
            if (status != ER_OK) {
                QCC_LogError(status, ("Failed to connect bus attachment"));
            }
        }
    }
}

BusConnectionImpl::~BusConnectionImpl()
{
    if (ownBa == true) {
        ba->Disconnect();
        ba->Stop();
        ba->Join();
        delete ba;
    }
}

QStatus BusConnectionImpl::GetStatus() const
{
    return status;
}

ajn::BusAttachment& BusConnectionImpl::GetBusAttachment() const
{
    return *ba;
}
}
