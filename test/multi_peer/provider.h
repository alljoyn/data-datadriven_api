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

#ifndef PROVIDER_H_
#define PROVIDER_H_

#include <vector>
#include <datadriven/Semaphore.h>

#include "MultiPeerInterface.h"

using namespace std;
using namespace gen::org_allseenalliance_test;

namespace test_system_multipeer {
class Provider {
  public:
    Provider(int provId,
             int numObj);
    ~Provider();

    void Run();

    void Stop();

  private:
    class MultiPeerObject;
    datadriven::Semaphore done;
    int provId;
    vector<MultiPeerObject*> objects;
    shared_ptr<datadriven::ObjectAdvertiser> advertiser;
};
};

#endif /* PROVIDER_H_ */
