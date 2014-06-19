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

#include "ConsumerSessionManager.h"
#include "ProviderSessionManager.h"
#include <map>
#include <qcc/String.h>
#include <qcc/Debug.h>
#include <qcc/Mutex.h>
#include <alljoyn/services_common/AsyncTaskQueue.h>
#include <alljoyn/Status.h>
#define QCC_MODULE "DD_COMMON"

namespace datadriven {
class WriterCache;
class RegisteredTypeDescription;
class ProvidedObject;

class BusConnectionImpl :
    private ajn::services::AsyncTask {
  public:
    BusConnectionImpl();
    virtual ~BusConnectionImpl();
    QStatus GetStatus() const;

    ConsumerSessionManager& GetConsumerSessionManager();

    ProviderSessionManager& GetProviderSessionManager();

    // TODO This is writer specific
    WriterCache* GetWriterCache(const qcc::String& intfname);

    void RegisterTypeDescription(const RegisteredTypeDescription* typedesc);

    class BusConnTaskData :
        public ajn::services::TaskData {
      public:
        virtual void Execute(const BusConnTaskData* td) const = 0;
    };

    void ProviderAsyncEnqueue(const BusConnTaskData* bctd);

    void ConsumerAsyncEnqueue(const BusConnTaskData* bctd);

    void AddObserver(const ObserverImpl* observer);

    void RemoveObserver(const ObserverImpl* observer);

    QStatus AddSignalListener(SignalListenerImpl* signalListener);

    QStatus RemoveSignalListener(SignalListenerImpl* signalListener);

    QStatus AddProvidedObject(ProvidedObject* object);

    void RemoveProvidedObject(ProvidedObject* object);

  private:
    friend class SignalListenerImpl;
    friend class ObserverImpl;
    friend class ProvidedObject;
    QStatus LockForSignalListener(SignalListenerImpl* signalListener);

    QStatus LockForObserver(ObserverImpl* observer);

    void UnlockObserver();

    QStatus LockForProvidedObject(ProvidedObject* object);

    void UnlockProvidedObject();

    BusConnectionImpl(const BusConnectionImpl&);
    QStatus status;
    ConsumerSessionManager consumerSessionManager;
    ProviderSessionManager providerSessionManager;
    std::map<qcc::String, std::unique_ptr<WriterCache> > caches;
    std::map<qcc::String, const RegisteredTypeDescription*> typedescs;

    mutable qcc::Mutex observersMutex;
    typedef std::map<ajn::InterfaceDescription::Member*, SignalListenerImpl*> SignalListenerPerMemberMap;
    std::map<const ObserverImpl*, SignalListenerPerMemberMap> observers;

    mutable qcc::Mutex providersMutex;
    std::vector<const ProvidedObject*> providers;

    /* ajn::services::AsyncTask */
    virtual void OnEmptyQueue();

    virtual void OnTask(ajn::services::TaskData const* taskdata);

    mutable ajn::services::AsyncTaskQueue providerAsync;
    mutable ajn::services::AsyncTaskQueue consumerAsync;
};
}

#undef QCC_MODULE

#endif /* BUSCONNECTIONIMPL_H_ */
