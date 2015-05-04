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

#ifndef CONSUMERBASE_H_
#define CONSUMERBASE_H_

#include <datadriven/Semaphore.h>

namespace test_system_properties {
class ConsumerBase {
  public:
    ConsumerBase();
    virtual ~ConsumerBase();
    void Test();

  protected:
    datadriven::Semaphore sync;
    size_t updated;
    size_t invalidated;
    int32_t signal_offset;

    virtual void WaitForPeer() = 0;

    virtual int32_t GetProperty(const char* name) = 0;

    virtual void SetProperty(const char* name,
                             int32_t val) = 0;
};
} /* namespace test_system_properties */

#endif /* CONSUMERBASE_H_ */
