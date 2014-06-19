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

#ifndef WRITERCACHE_H_
#define WRITERCACHE_H_

#include <map>
#include <vector>

#include <alljoyn/MsgArg.h>
#include <qcc/Mutex.h>

#include <ProviderSessionManager.h>
#include <datadriven/datadriven.h>

#include <qcc/Debug.h>
#define QCC_MODULE "DD_PROVIDER"
namespace datadriven {
class WriterCache {
  public:
    typedef std::vector<ajn::MsgArg> Properties;
    typedef std::map<ProvidedObject*, Properties> Cache;

    WriterCache(const RegisteredTypeDescription& itd,
                ProviderSessionManager& sm);

    /********** SessionManager-facing API **********/
    size_t GetElementCount() const;

    Cache GetAll();

    const ajn::InterfaceDescription::Member* GetPropUpdateSignalMember() const;

    /********** ProvidedObject-facing API **********/
    QStatus Update(ProvidedObject* obj,
                   const Properties& prop);

    void Remove(ProvidedObject* obj);

  protected:

  private:
    mutable qcc::Mutex mutex;
    qcc::String interface;
    const RegisteredTypeDescription& typedesc;
    ProviderSessionManager& providersessionmanager;
    Cache cache;
};
}

#undef QCC_MODULE
#endif
