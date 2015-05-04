/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
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

#include <ostream>
#include <datadriven/Mutex.h>

#include "consumer.h"

#define TIMEOUT 10000

namespace test_system_multipeer {
class Consumer::EmitNumListener :
    public datadriven::SignalListener<MultiPeerProxy, MultiPeerProxy::EmitNum> {
  public:
    EmitNumListener(int consId) :
        consId(consId)
    {
    }

    /**
     * Add expected (object ID, signal number) mapping.
     */
    void Expect(int id, int num)
    {
        mutex.Lock();
        idNumMap[id] = num;
        idOk[id] = false;
        mutex.Unlock();
    }

    /**
     * Validate that all signals for requests we fired have been received.
     */
    void Validate()
    {
        mutex.Lock();
        for (map<int, bool>::iterator it = idOk.begin(); it != idOk.end(); ++it) {
            assert(true == it->second);
        }
        mutex.Unlock();
    }

    /**
     * Reset the expected and received signal containers.
     */
    void Reset()
    {
        mutex.Lock();
        idNumMap.clear();
        idOk.clear();
        mutex.Unlock();
    }

    void OnSignal(const MultiPeerProxy::EmitNum& signal)
    {
        const std::shared_ptr<MultiPeerProxy> mpp = signal.GetEmitter();
        int32_t id = mpp->GetProperties().id;

        mutex.Lock();
        if (signal.num == idNumMap[id]) {
            // signal from the object for which we fired the request
            cout << "Consumer " << consId << " receives signal with num "
                 << signal.num << " from object with id " << id << endl;
            idOk[id] = true;
            assert(ER_OK == sema.Post());
        }
        mutex.Unlock();
    }

    QStatus TimedWait(uint32_t ms)
    {
        return sema.TimedWait(ms);
    }

  private:
    mutable datadriven::Mutex mutex;
    int consId;
    datadriven::Semaphore sema;
    map<int, int> idNumMap;
    map<int, bool> idOk;
};

Consumer::Consumer(int consId,
                   int numObj) :
    consId(consId), numObj(numObj),
    observer(datadriven::Observer<MultiPeerProxy>::Create(this)), enl(NULL)
{
    enl = new EmitNumListener(consId);
    observer->AddSignalListener(*enl);
};

Consumer::~Consumer()
{
    observer->RemoveSignalListener(*enl);
    delete enl;
}

void Consumer::OnUpdate(const shared_ptr<MultiPeerProxy>& mpp)
{
    assert(ER_OK == mpp->GetStatus());
    int obj_id = mpp->GetProperties().id;
    cout << "Consumer " << consId << " receives object with id " << obj_id
         << " and path " << mpp->GetObjectId().GetBusObjectPath().c_str() << endl;
    if (objects.end() == objects.find(obj_id)) {
        objects[obj_id] = mpp;
        assert(ER_OK == objSync.Post());
    }
}

void Consumer::Test(int numLoops)
{
    // wait for objects
    for (int i = 0; i < numObj; i++) {
        cout << "Consumer " << consId << " waits for " << (numObj - i)
             << " object(s)" << endl;
        assert(ER_OK == objSync.TimedWait(TIMEOUT));
    }
    // loop discovered objects in ascending ID order
    cout << "Consumer " << consId << " checks all objects discovered" << endl;
    for (int i = 0; i < numObj; i++) {
        assert(objects.end() != objects.find(i));
    }
    for (int loop = 1; loop <= numLoops; loop++) {
        int cnt = 0;

        cout << "Consumer " << consId << " starting loop " << loop << "/" << numLoops << endl;
        // iterate objects known to observer
        enl->Reset();
        for (datadriven::Observer<MultiPeerProxy>::iterator it = observer->begin();
             it != observer->end(); ++it) {
            int32_t id = it->GetProperties().id;
            int num = (consId * (numObj * numLoops)) + ((loop - 1) * numObj) + cnt;

            assert(objects[id] == *it);
            // call method that will trigger signal
            cout << "Consumer " << consId << " loop " << loop << "/" << numLoops
                 << " calls method on object with id "
                 << id << " and number " << num << endl;
            enl->Expect(id, num);
            assert(ER_OK == it->RequestEmitNum(num));
            // wait for 'response' signal (the one with id == testNum)
            assert(ER_OK == enl->TimedWait(TIMEOUT));
            cnt++;
        }
        assert(numObj == cnt);
        // validate all signals received
        cout << "Consumer " << consId << " loop " << loop << "/" << numLoops
             << " checks all signals received" << endl;
        enl->Validate();
    }
    // cleaning up
    objects.clear();
    // done
    cout << "Consumer " << consId << " is done" << endl;
}
};
