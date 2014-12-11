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

#ifndef BUSCONNECTIONIMPL_H_
#define BUSCONNECTIONIMPL_H_

#include <map>
#include <set>

#include <qcc/String.h>

#include <alljoyn/about/AboutPropertyStoreImpl.h>
#include <alljoyn/Status.h>

#include <datadriven/ObserverBase.h>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_COMMON"

namespace datadriven {
class RegisteredTypeDescription;
class ProvidedObjectImpl;
class SignalListenerBase;

class BusConnectionImpl {
  public:
    /**
     * Checks if the singleton BusConnectionImpl is already created. If not, it will be created.
     * \return the BusConnectionImpl object or nullptr in case of an error
     */
    static std::shared_ptr<BusConnectionImpl> GetInstance(ajn::BusAttachment* ba = NULL);

    virtual ~BusConnectionImpl();

    /**
     * Returns the status of the BusConnectionImpl object
     */
    QStatus GetStatus() const;

    /**
     * Returns the BusAttachment object of this BusConnectionImpl object
     */
    ajn::BusAttachment& GetBusAttachment() const;

  private:
    BusConnectionImpl(ajn::BusAttachment* ba = NULL);
    BusConnectionImpl(const BusConnectionImpl&);

    /** The status of the object */
    QStatus status;
    /** Pointer to the BusAttachment object */
    ajn::BusAttachment* ba;
    /** Indicates that the BusAttachment was created by this object (true) or not (false) */
    bool ownBa;
};
}

#undef QCC_MODULE

#endif /* BUSCONNECTIONIMPL_H_ */
