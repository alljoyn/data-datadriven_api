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

#include <assert.h>
#include <iostream>

#include "ConsumerBase.h"
#include "data.h"

namespace test_system_properties {
using namespace std;

ConsumerBase::ConsumerBase() :
    updated(0), invalidated(0), signal_offset(0)
{
}

ConsumerBase::~ConsumerBase()
{
}

/**
 * \test interop properties test:
 *       -# Get and set properties
 *       -# Get and set invalidated properties
 *       -# Test that update and updateAll trigger only one onUpdate (ASACORE-47)
 */
void ConsumerBase::Test()
{
    int32_t val;

    // wait for object
    cout << "Consumer waiting for object" << endl;
    WaitForPeer();

    // testing of non-cacheable properties
    val = GetProperty(PROP_RO);
    assert(INITIAL_RO == val);

    SetProperty(PROP_WO, val * MULTIPLIER);
    val = GetProperty(PROP_RO);
    assert(MULTIPLIER * INITIAL_RO == val);

    val = GetProperty(PROP_RW);
    assert(INITIAL_RW == val);

    SetProperty(PROP_RW, val * MULTIPLIER);
    val = GetProperty(PROP_RW);
    assert(MULTIPLIER * INITIAL_RW == val);

    // EmitsChangedSignal variations
    signal_offset = 100;
    SetProperty(PROP_ET, signal_offset + INITIAL_ET);

    WaitForPeer();

    val = GetProperty(PROP_EI);
    assert(INITIAL_EI == val);

    val = GetProperty(PROP_EF);
    assert(INITIAL_EF == val);

    SetProperty(PROP_EF, val * MULTIPLIER);
    val = GetProperty(PROP_EF);
    assert(MULTIPLIER * INITIAL_EF == val);

    SetProperty(PROP_EI, INITIAL_EI * MULTIPLIER);

    WaitForPeer();

    /* Check the invalidated property */
    val = GetProperty(PROP_EI);
    assert(INITIAL_EI * MULTIPLIER == val);

    assert(3 == updated);
    /* Test to make sure that the onUpdate is only called one time per Update or UpdateAll so 3 times*/

    cout << "Consumer done" << endl;
}
} /* namespace test_system_properties */
