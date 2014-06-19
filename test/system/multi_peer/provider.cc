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

#include <ostream>

#include "provider.h"

namespace test_system_multipeer {
class Provider::MultiPeerObject :
    public datadriven::ProvidedObject,
    public MultiPeerInterface {
  public:
    MultiPeerObject(datadriven::BusConnection& busconn,
                    int32_t num,
                    int peer) :
        datadriven::ProvidedObject(busconn),
        MultiPeerInterface(this),
        provId(peer)
    {
        this->id = num;
    }

    void RequestEmitNum(int32_t num, RequestEmitNumReply& _reply)
    {
        cout << "Provider " << provId << " got request for signal emit with number "
             << num << " on object with id " << id << endl;
        EmitNum(num);
        _reply.Send();
    }

  private:
    int provId;
};

Provider::Provider(int provId,
                   int numObj) :
    provId(provId), busConnection()
{
    sem_init(&done, 0, 0);
    assert(ER_OK == busConnection.GetStatus());
    for (int i = 0; i < numObj; i++) {
        int obj_id = (provId * numObj) + i;
        objects.push_back(new MultiPeerObject(busConnection, obj_id, provId));
        /* signal existence of object */
        cout << "Provider " << provId << " announces object " << i
             << " with id " << obj_id << endl;
        assert(ER_OK == objects[i]->UpdateAll());
        assert(ER_OK == objects[i]->GetStatus());
    }
    cout << "Provider " << provId << " initialized" << endl;
}

Provider::~Provider()
{
    for (vector<MultiPeerObject*>::iterator it = objects.begin(); it != objects.end(); ++it) {
        (*it)->RemoveFromBus();
        delete *it;
    }
    sem_destroy(&done);
}

void Provider::Stop()
{
    cout << "Provider " << provId << " stop requested" << endl;
    sem_post(&done);
}

void Provider::Run()
{
    cout << "Provider " << provId << " sleeps" << endl;
    while (0 != sem_wait(&done)) {
        continue;
    }
    cout << "Provider " << provId << " stopped" << endl;
}
};
