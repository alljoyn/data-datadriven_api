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
    MultiPeerObject(shared_ptr<datadriven::ObjectAdvertiser> advertiser,
                    int32_t num,
                    int peer) :
        datadriven::ProvidedObject(advertiser),
        MultiPeerInterface(this),
        provId(peer)
    {
        this->id = num;
    }

    void RequestEmitNum(int32_t num)
    {
        cout << "Provider " << provId << " got request for signal emit with number "
             << num << " on object with id " << id << endl;
        EmitNum(num);
    }

  private:
    int provId;
};

Provider::Provider(int provId,
                   int numObj) :
    provId(provId), advertiser(datadriven::ObjectAdvertiser::Create())
{
    assert(nullptr != advertiser);
    for (int i = 0; i < numObj; i++) {
        int obj_id = (provId * numObj) + i;
        MultiPeerObject* o = new MultiPeerObject(advertiser, obj_id, provId);
        objects.push_back(o);
        /* signal existence of object */
        cout << "Provider " << provId << " announces object " << i
             << " with id " << obj_id << " and path " << o->GetPath() << endl;
        assert(ER_OK == objects[i]->UpdateAll());
        assert(ER_OK == objects[i]->GetStatus());
    }
    cout << "Provider " << provId << " initialized" << endl;
}

Provider::~Provider()
{
    cout << "Provider " << provId << " destroying" << endl;
    for (vector<MultiPeerObject*>::iterator it = objects.begin(); it != objects.end(); ++it) {
        (*it)->RemoveFromBus();
        delete *it;
    }
}

void Provider::Stop()
{
    cout << "Provider " << provId << " stop requested" << endl;
    assert(ER_OK == done.Post());
}

void Provider::Run()
{
    cout << "Provider " << provId << " sleeps" << endl;
    assert(ER_OK == done.Wait());
    cout << "Provider " << provId << " stopped" << endl;
}
};
