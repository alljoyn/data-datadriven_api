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

#ifndef PROVIDERSESSIONMANAGER_H_
#define PROVIDERSESSIONMANAGER_H_
#include <qcc/String.h>

#include <alljoyn/BusAttachment.h>
#include "../inc/datadriven/datadriven.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"

namespace datadriven {
class InterestReceiver;
class ProviderSessionManagerImpl;

/* The responsibility of this class is to establish and teardown session to providers that advertise
 * BusObjects which implement interfaces we are interested in */
class ProviderSessionManager {
  public:
    static const int DATADRIVEN_SERVICE_PORT = 5001;

    /* The only argument is passed directly to BusAttachment (AFAIK it has little or no use) */
    ProviderSessionManager(const char* appName,
                           BusConnectionImpl* fac);
    virtual ~ProviderSessionManager();

    QStatus AdvertiseBusObject(ProvidedObject& busObject);

    QStatus RemoveBusObject(ProvidedObject& busObject);

    QStatus BusObjectSignal(ProvidedObject& busObject,
                            const qcc::String& interface,
                            const ajn::InterfaceDescription::Member* member,
                            const ajn::MsgArg* msgArg,
                            size_t len);

    /* To be used by higher layers to create BusObject */
    ajn::BusAttachment& GetBusAttachment();

    QStatus GetStatus() const;

  private:
    std::unique_ptr<ProviderSessionManagerImpl> providerSessionManagerImpl;

    ProviderSessionManager(const ProviderSessionManager&);
    ProviderSessionManager& operator=(const ProviderSessionManager&);
};
}

#undef QCC_MODULE
#endif
