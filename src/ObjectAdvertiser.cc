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

#include <datadriven/ObjectAdvertiser.h>
#include "ObjectAdvertiserImpl.h"

#include <qcc/Debug.h>

#define QCC_MODULE "DD_PROVIDER"

namespace datadriven {
using namespace std;

shared_ptr<ObjectAdvertiser> ObjectAdvertiser::Create(ajn::BusAttachment* ba,
                                                      ajn::AboutData* aboutData,
                                                      ajn::AboutObj* aboutObj,
                                                      ajn::SessionOpts* opts,
                                                      ajn::SessionPort sp)
{
    shared_ptr<ObjectAdvertiser> advertiser(new ObjectAdvertiser(ba, aboutData));
    if (nullptr != advertiser) {
        if (ER_OK != advertiser->objectAdvertiserImpl->GetStatus()) {
            advertiser = nullptr;
        }
    }
    return advertiser;
}

ObjectAdvertiser::ObjectAdvertiser(ajn::BusAttachment* ba,
                                   ajn::AboutData* aboutData,
                                   ajn::AboutObj* aboutObj,
                                   ajn::SessionOpts* opts,
                                   ajn::SessionPort sp) :
    objectAdvertiserImpl(new ObjectAdvertiserImpl(ba, aboutData, aboutObj, opts, sp))
{
}

ObjectAdvertiser::~ObjectAdvertiser()
{
}

std::shared_ptr<ObjectAdvertiserImpl> ObjectAdvertiser::GetImpl()
{
    return objectAdvertiserImpl;
}
}
