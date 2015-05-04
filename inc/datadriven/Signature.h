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

#ifndef SIGNATURE_H_
#define SIGNATURE_H_

#include <qcc/String.h>

namespace datadriven {
/**
 * \class Signature
 * \brief Class for a signature type ('g' signature) defined in the data model.
 *
 * This is a simple subclass of \c qcc::String. It exists mainly as an aid to the
 * marshaling code to disambiguate between plain strings ('s'), signatures
 * ('g') and object paths ('o').
 */
class Signature :
    public qcc::String {
  public:
    /**
     * Construct an empty signature.
     */
    Signature() :
        qcc::String() { };

    /**
     * Construct a signature from a <tt>const char*</tt>.
     *
     * \param[in] str        \c char* array to use as initial value for the signature.
     * \param[in] strLen     Length of string or 0 if \a str is null terminated
     */
    Signature(const char* str,
              size_t strLen = 0) :
        qcc::String(str, strLen) { };

    /**
     * Construct a signature from a plain \c qcc::String.
     *
     * \param[in] str        String to copy
     */
    Signature(const qcc::String& str) :
        qcc::String(str) { };
};
}

#endif /* SIGNATURE_H_ */
