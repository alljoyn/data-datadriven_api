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

#ifndef OBSERVERIMPL_H_
#define OBSERVERIMPL_H_

#include <vector>
#include <memory>

#include "BusConnection.h"
#include "ProxyInterface.h"

namespace datadriven {
class ObserverCache;
class ConsumerInterfaceListener;
class SignalListenerImpl;

/** \private */
class ObserverImpl {
  public:
    virtual ProxyInterface* Alloc(const ObjectId& objId) const = 0;

    void Remove(const ObjectId& objId);

  protected:
    /** \private */
    class iterator :
        public std::iterator<std::input_iterator_tag, std::shared_ptr<ProxyInterface> >{
      public:
        /** \private */
        iterator(const iterator& i);
        /** \private */
        iterator(const ObserverImpl& cache);

        /** \private */
        iterator& operator=(const iterator& i);

        /** \private */
        iterator& operator++();

        /** \private */
        iterator operator++(int);

        /** \private */
        bool operator==(const iterator& i) const;

        /** \private */
        bool operator!=(const iterator& it) const;

        /** \private */
        const std::shared_ptr<ProxyInterface>& operator*();

        /** \private */
        static iterator end();

      private:
        std::vector<std::shared_ptr<ProxyInterface> > objects;
        std::vector<std::shared_ptr<ProxyInterface> >::const_iterator it;

        iterator();
    };

    /** \private */
    class Listener {
      public:
        typedef void (*CallBack)(Listener&, const std::shared_ptr<ProxyInterface>&);
        virtual ~Listener() { }
    };

    typedef ajn::MessageReceiver* SignalListenerRef;

    ObserverImpl(BusConnection& busConnection,
                 const TypeDescription& typedesc);
    virtual ~ObserverImpl();
    QStatus AddSignalListener(SignalListenerImpl* l);

    QStatus RemoveSignalListener(SignalListenerImpl* l);

    QStatus GetStatus() const;

    std::shared_ptr<ProxyInterface> Get(const ObjectId& objId);

    std::shared_ptr<ProxyInterface> Get(const::ajn::Message& message);

    const RegisteredTypeDescription& GetRegisteredTypeDescription() const;

    void Notify(const std::shared_ptr<ProxyInterface>& objProxy,
                Listener::CallBack cb) const;

    QStatus AddListener(Listener& l);

    QStatus RemoveListener(Listener& l);

    virtual void NotifyRemove(const std::shared_ptr<ProxyInterface>& objProxy) = 0;

    std::vector<Listener*>  interfaceListeners;

  private:
    mutable qcc::Mutex listenersMutex;
    QStatus status;

    std::weak_ptr<BusConnectionImpl> busConnectionImpl;
    //BusConnection& busConnection;
    std::unique_ptr<ObserverCache> cache;
    std::unique_ptr<RegisteredTypeDescription> iface;
    std::unique_ptr<ConsumerInterfaceListener> observerListener;

    ObserverImpl(const ObserverImpl&);
    void operator=(const ObserverImpl&);

    class RemoveTaskData;
    friend class RemoveTaskData;
    friend class SignalListenerImpl;
};
}

#endif /* OBSERVERIMPL_H_ */
