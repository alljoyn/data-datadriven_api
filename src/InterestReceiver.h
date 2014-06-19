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

#ifndef INTERESTRECEIVER_H_
#define INTERESTRECEIVER_H_

#include <alljoyn/BusAttachment.h>
#include <alljoyn/Status.h>

namespace datadriven {
class ProviderSessionManagerImpl;

class InterestReceiver :
    public ajn::BusObject {
  public:
    InterestReceiver(ProviderSessionManagerImpl& providerSessionManagerImpl,
                     const ajn::InterfaceDescription& intf);
    virtual ~InterestReceiver();

    QStatus GetStatus();

    static QStatus CreateInterface(ajn::BusAttachment& ba,
                                   ajn::InterfaceDescription*&);

    static const char* DATADRIVEN_INTEREST_INTF;
    static const char* DATADRIVEN_INTEREST_PATH;

    static const char* REGISTER_INTEREST_METHOD_CALL;
    static const char* UNREGISTER_INTEREST_METHOD_CALL;

    static const char* DATADRIVEN_PREFIX;

  private:
    ProviderSessionManagerImpl* providerSessionManagerImpl;
    void RegisterInterestMethodCallImpl(const ajn::InterfaceDescription::Member* member,
                                        ajn::Message& msg);

    void UnregisterInterestMethodCallImpl(const ajn::InterfaceDescription::Member* member,
                                          ajn::Message& msg);

    QStatus errorStatus;
};
} /* namespace datadriven */

#endif /* INTERESTRECEIVER_H_ */
