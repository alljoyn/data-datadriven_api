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

#ifndef CONSUMER_H_
#define CONSUMER_H_

#include <datadriven/Observer.h>
#include <datadriven/Semaphore.h>

#include "MultiPeerProxy.h"

using namespace std;
using namespace gen::org_allseenalliance_test;

namespace test_system_multipeer {
class Consumer :
    public datadriven::Observer<MultiPeerProxy>::Listener {
  public:
    Consumer(int consId,
             int numObj);
    ~Consumer();

    void OnUpdate(const shared_ptr<MultiPeerProxy>& mpp);

    /**
     * \test
     * - The multi-peer provider(s) will:
     *   -# publish a number of objects (with unique and sequential IDs)
     * - The multi-peer consumer will:
     *   -# wait for all objects of all providers
     *   -# validate that objects with all necessary IDs have been discovered
     *   -# for each object
     *      -# read out the properties
     *      -# invoke a method
     *      -# wait for a signal
     */
    void Test(int num_loops = 1);

  private:
    class EmitNumListener;
    datadriven::Semaphore sync;
    int consId;
    int numObj;
    shared_ptr<datadriven::Observer<MultiPeerProxy> > observer;
    map<int, shared_ptr<MultiPeerProxy> > objects;
    EmitNumListener* enl;
};
};

#endif /* CONSUMER_H_ */
