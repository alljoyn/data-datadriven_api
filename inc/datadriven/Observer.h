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

#ifndef OBSERVER_H_
#define OBSERVER_H_

#include <datadriven/datadriven.h>
#include <memory>
#include "ObserverImpl.h"

#include <qcc/Debug.h>
#define QCC_MODULE "DD_CONSUMER"

namespace datadriven {
/**
 * \class Observer
 * \brief Discovers and provides access to all objects implementing a given
 *        AllJoyn interface.
 *
 * \tparam T The proxy class for the AllJoyn interface as created by the code
 *           generator. For example, for an AllJoyn interface Foo, T would be
 *           FooProxy.
 *
 * Observers are a consumer's window into the AllJoyn world. An Observer for a
 * particular AllJoyn interface will discover, and make accessible, all objects
 * on the AllJoyn bus that implement that interface.
 *
 * \par Interaction models
 * There are basically two ways in which you can interact with an Observer. On
 * the one hand, you can treat an Observer as a passive repository of
 * information: it will know at any time which objects on the bus implement the
 * interface you're interested in,  and what their latest state (i.e. property
 * values) is. You can retrieve proxies for the discovered objects either via
 * iteration (with the standard Observer::begin and Observer::end iterator
 * functions), or via the Observer::Get method.
 *
 * \par
 * Alternatively, an Observer can actively alert you when interesting things
 * happen to a discovered objects:
 * - if you provide an Observer::Listener object via the Observer constructor
 *   you'll be notified whenever a new object implementing the interface
 *   you're interested in appears on the bus, whenever the properties
 *   of a discovered object change, or whenever a previously discovered object
 *   disappears from the bus. Observer::Listener object is called <em>state
 *   change listener</em>.
 * - if you provide SignalListener listeners via the
 *   Observer::AddSignalListener method, you'll be alerted whenever a
 *   discovered object emits the signal in question.
 *
 * \par Object Identity
 * Discovered objects are uniquely identified by their ObjectId. For the
 * purposes of the Data-driven API, the ObjectId is an opaque value type. If
 * you know a particular object's ObjectId, you can retrieve a proxy for the
 * object via the Observer::Get method. You can learn about an object's
 * ObjectId from the Observer::Listener callbacks, or because you encountered
 * an object earlier during iteration over the Observer.
 *
 * \par
 * By now, you may have noticed a discrepancy in the way providers and
 * consumers treat objects. A provider exposes objects on the bus that
 * implement one or more AllJoyn interfaces. A consumer, however, makes use of
 * Observers to discover those objects, and Observers are tied to a particular
 * interface. Moreover, the proxy objects you can retrieve from an Observer
 * expose only the interface the Observer knows about. The ObjectId is what
 * ties the different consumer-side views of the same object together: the same
 * object, as seen by different Observers, will have the same ObjectId for all
 * of these Observers.
 *
 * \par Proxy Object Lifetimes
 * The Observer will never hand out \e naked proxy objects. Instead, you will
 * always get a shared_ptr to the proxy object. The following guarantees apply:
 * - as long as the remote object is alive (i.e. it has been put on the bus,
 *   and it has not yet been removed from the bus), the Observer will always
 *   give you a reference to the \e same proxy object.
 * - as long as the application has a reference to the proxy object, the proxy
 *   object's GetProperties method will return the last known value of the
 *   object's properties. This remains true even after the object is removed
 *   from the bus (i.e. after you've seen the Observer::Listener::OnRemove
 *   callback for the object). Obviously, at that point it is no longer
 *   possible to invoke methods on the proxy object.
 * - if a remote object is removed from the bus, and the local application
 *   still holds a reference to the corresponding proxy object, that reference
 *   will remain valid after the object reappears on the bus. If the
 *   application did not hold a reference to the proxy object when the remote
 *   object is removed from the bus, the proxy object itself is cleaned up. If
 *   the remote object then reappears on the bus, a new proxy object is
 *   created.
 * - if a remote object is removed from the bus, you cannot retrieve that
 *   object's proxy any more via either iteration over the Observer, or
 *   Observer::Get. This is true regardless of whether the application still
 *   holds a reference to the proxy object.
 */
template <typename T> class Observer :
    private ObserverImpl {
  public:

    /**
     * \class Listener
     * \brief State change listener, defines callbacks for when the state of a
     *        discovered object changes.
     *
     * The listener provides two callback notifications:
     * - Listener::OnUpdate is invoked whenever a discovered object's
     *   observable properties change. The discovery of the object itself also
     *   counts as a property change, so you will learn about the discovery of
     *   a new object (or an existing object that the Observer hasn't yet
     *   discovered before) through this callback.
     * - Listener::OnRemove is invoked whenever a previously discovered object
     *   is removed from the bus.
     */
    class Listener :
        public ObserverImpl::Listener {
      public:
        /**
         * \brief Invoked whenever a new object is discovered, or an already
         *        discovered object's observable properties change.
         *
         * \param[in] obj Proxy object
         */
        virtual void OnUpdate(const std::shared_ptr<T>& obj) { };

        /**
         * \brief Invoked when a discovered object is removed from the AllJoyn
         *        bus.
         *
         * When this callback is invoked, the remote object is considered
         * "dead", and it is no longer possible to retrieve the proxy from the
         * Observer through iteration or the Observer::Get method. The proxy
         * reference you get as input argument to this callback is your last
         * chance to keep a reference to the proxy for the now-removed object.
         *
         * \param[in] obj Proxy object
         */
        virtual void OnRemove(const std::shared_ptr<T>& obj) { };
    };

    /**
     * \class iterator
     * \brief Iterator over the proxies for all objects discovered by this Observer
     *
     * The iterator is an <a
     * href="http://www.cplusplus.com/reference/iterator/InputIterator/">input
     * iterator</a>.
     *
     * The iterator does not point to \e naked proxy objects, but rather to
     * shared_ptr object encapsulating a proxy object. This is consistent with
     * the rest of the Observer interface. Dereferencing the iterator (*it)
     * will hence give you an object of shared_ptr<FooProxy>. For your
     * convenience, iterator::operator-> is defined to do a double-dereference:
     * it->SomeMethod() will call SomeMethod on the proxy object, not on the
     * shared_ptr.
     */
    class iterator :
        public std::iterator<std::input_iterator_tag, T>{
      public:
        /**
         * Copy constructor
         * \param[in] it Iterator to copy from
         */
        iterator(const iterator& it) :
            it(it.it) { };

        /**
         * Assignment operator
         * \param[in] it Iterator to copy contents from
         * \return Reference to the iterator
         */
        iterator& operator=(const iterator& it)
        {
            this->it = it.it;
            return *this;
        };

        /**
         * Advance iterator (prefix)
         * \return Reference to the iterator
         */
        iterator& operator++()
        {
            ++it;
            return *this;
        }

        /**
         * Advance iterator (postfix)
         * \param[in] x dummy input
         * \return The iterator
         */
        iterator operator++(int x)
        {
            return it++;
        }

        /**
         * Compare iterators for equality
         * \param[in] it Iterator to compare with
         * \retval true if both iterators point to the same object
         * \retval false if the iterators are different
         */
        bool operator==(const iterator& it) const
        {
            return this->it == it.it;
        }

        /**
         * Compare iterators for inequality
         * \param[in] it Iterator to compare with
         * \retval true if the iterators are different
         * \retval false if both iterators point to the same object
         */
        bool operator!=(const iterator& it) const
        {
            return this->it != it.it;      /* wouldn't it make sense to first compare the pointers ? */
        }

        /**
         * Dereference operator
         * \return Shared pointer of the observed object at the current iterator position.
         */
        std::shared_ptr<T> operator*()
        {
            return CastToTPtr(*it);
        }

        /**
         * Arrow operator, will for convenience also dereference the shared pointer.
         * \return Pointer to the observed object at the current iterator position.
         */
        const T* operator->()
        {
            return this->operator*().operator->();
        };

        /**
         * Returns iterator that points to the end
         * \return Iterator at position beyond the last element.
         */
        static iterator end()
        {
            return iterator(ObserverImpl::iterator::end());
        }

        friend class Observer<T>;

      private:
        ObserverImpl::iterator it;

        iterator() :
            it() { }

        iterator(const ObserverImpl::iterator it) :
            it(it) { }

        iterator(const Observer& observer) :
            it(observer) { }
    };

    /**
     * Return iterator pointing to the first discovered object.
     * \return iterator positioned at first element if any
     */
    iterator begin()
    {
        return iterator(*this);
    }

    /**
     * Return iterator pointing beyond the last discovered object.
     * \return iterator
     */
    iterator end() const { return iterator::end(); }

    /**
     * \brief Add a signal listener for a specific signal.
     *
     * It is allowed to add multiple signal listeners for the same signal to a
     * single Observer.
     *
     * \tparam S the signal class created by the code generator for the signal
     *           in question.
     * \param[in] l the signal listener.
     * \retval ER_OK    success
     * \retval other    failure
     */
    template <typename S> QStatus AddSignalListener(SignalListener<T, S>& l)
    {
        l.SetObserver(*this);
        l.SetMember(const_cast<ajn::InterfaceDescription::Member*>(&S::GetMember(ObserverImpl::
                                                                                 GetRegisteredTypeDescription())));
        return ObserverImpl::AddSignalListener(&l);
    }

    /**
     * \brief Remove a signal listener.
     *
     * \tparam S the signal class created by the code generator for the signal
     *           in question.
     * \param[in] l the signal listener
     * \retval ER_OK    success
     * \retval other    failure
     */
    template <typename S> QStatus RemoveSignalListener(SignalListener<T, S>& l)
    {
        return ObserverImpl::RemoveSignalListener(&l);
    }

    /**
     * \brief Get the proxy for a discovered object based on ObjectId.
     * \param objId the discovered object's ObjectId.
     * \return Shared pointer to the proxy object.
     */
    std::shared_ptr<T> Get(const ObjectId& objId)
    {
        return CastToTPtr(ObserverImpl::Get(objId));
    }

    /** \private
     * Retrieve an observed object based on a communication layer message
     * \param message Communication layer message
     * \return Shared pointer to the corresponding observed object
     */
    std::shared_ptr<T> Get(const ajn::Message& message)
    {
        return CastToTPtr(ObserverImpl::Get(message));
    }

    /**
     * \brief Constructor.
     *
     * Post construction, the application should check Observer::GetStatus to
     * ascertain that the construction and initialisation of the Observer were
     * successful.
     *
     * \param[in] busConnection Connection to the AllJoyn bus.
     * \param[in] listener Notification handler for property changes and changes to discovered objects<br>
     *                     A NULL value will discard any detected property and discovered object changes.
     */
    Observer(BusConnection& busConnection,
             Listener* listener) :
        ObserverImpl(busConnection, T::Type::GetInstance()), propertiesListener(*this), interfaceListener(listener)
    {
        if (NULL != listener) {
            ObserverImpl::AddListener(*listener);
        }
        AddSignalListener(propertiesListener);
    }

    /**
     * \brief Destructor.
     *
     * The application should not hold any references to proxy objects from
     * this Observer when the Observer object is destroyed.
     */
    ~Observer()
    {
        RemoveSignalListener(propertiesListener);
        if (NULL != interfaceListener) {
            ObserverImpl::RemoveListener(*interfaceListener);
        }
    }

    /**
     * \brief Reports whether the Observer is successfully constructed and
     *        initialised.
     * \retval ER_OK on success
     * \retval others on failure
     */
    QStatus GetStatus() const
    {
        return ObserverImpl::GetStatus();
    }

  private:    /*Observer's private members */
    static std::shared_ptr<T> CastToTPtr(const std::shared_ptr<ProxyInterface>& objProxy)
    {
        return std::static_pointer_cast<T>(objProxy);
    }

    static void UpdateCB(ObserverImpl::Listener& l, const std::shared_ptr<ProxyInterface>& objProxy)
    {
        static_cast<Listener&>(l).OnUpdate(CastToTPtr(objProxy));
    }

    static void RemoveCB(ObserverImpl::Listener& l, const std::shared_ptr<ProxyInterface>& objProxy)
    {
        static_cast<Listener&>(l).OnRemove(CastToTPtr(objProxy));
    }

    void NotifyUpdate(const std::shared_ptr<T>& objProxy) const
    {
        /* Trigger UpdateCB on all listeners */
        Notify(objProxy, &UpdateCB);
    }

    void NotifyRemove(const std::shared_ptr<ProxyInterface>& objProxy)
    {
        /* Trigger RemoveCB on all listeners */
        Notify(objProxy, &RemoveCB);
    }

    ProxyInterface* Alloc(const ObjectId& objId) const
    {
        return new T(ObserverImpl::GetRegisteredTypeDescription(), objId);
    }

    class PropertiesUpdateSignalListener :
        public datadriven::SignalListener<T, typename T::PropertiesUpdate> {
      public:
        PropertiesUpdateSignalListener(const Observer<T>& observer) :
            observer(observer) { }

        void OnSignal(const typename T::PropertiesUpdate& signal)
        {
            std::shared_ptr<T> obj = signal.GetEmitter();
            obj->SetProperties(signal.properties);
            observer.NotifyUpdate(obj);
        }

      private:
        const Observer<T>& observer;
    };

    PropertiesUpdateSignalListener propertiesListener;
    Listener* interfaceListener;
};
}

#undef QCC_MODULE
#endif /* OBSERVER_H_ */
