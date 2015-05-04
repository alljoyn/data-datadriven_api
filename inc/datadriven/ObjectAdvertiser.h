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

#ifndef OBJECTADVERTISER_H_
#define OBJECTADVERTISER_H_

#include <memory>

#include <alljoyn/Status.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/AboutData.h>
#include <alljoyn/AboutObj.h>

namespace datadriven {
class ObjectAdvertiserImpl;

static const int DATADRIVEN_SERVICE_PORT = 5001;

/**
 * \class ObjectAdvertiser
 * \brief Advertises the ProvidedObject instances to the bus.
 *
 * You need to create an object of this class before you can create a ProvidedObject.
 */
class ObjectAdvertiser {
  public:

    /**
     * Creates an Advertiser object and optionally sets up a connection to the AllJoyn bus.
     *
     * If no BusAttachment is provided, one will be created internally and connected
     * to the AllJoyn bus. If a BusAttachment is passed, it must already be connected
     * to the bus.
     * If one of the following: (AboutData, AboutObj, SessionOpts) is provided then all other
     * objects must be provided as well. If none of the aforementioned objects is provided,
     * then they will all be created internally (with banal about data).
     *
     * The ObjectAdvertiser takes no ownership of passed objects. It is up to the
     * application to release them when necessary.
     *
     * \param[in] bus The (optional) AllJoyn BusAttachment to be used for interactions with the bus.  If
     *                not provided one will be created.
     * \param[in] aboutData (conditionally optional) metadata on the application
     * \param[in] aboutObj (conditionally optional) about object used for announcements
     * \param[in] opts (conditionally optional) session options used when binding
     * \param[in] sp (conditionally optional) session port used when binding
     *
     * \return the ObjectAdvertiser shared pointer or nullptr in case of an error
     *
     */
    static std::shared_ptr<ObjectAdvertiser> Create(ajn::BusAttachment* bus = nullptr,
                                                    ajn::AboutData* aboutData = nullptr,
                                                    ajn::AboutObj* aboutObj = nullptr,
                                                    ajn::SessionOpts* opts = nullptr,
                                                    ajn::SessionPort sp = DATADRIVEN_SERVICE_PORT);

    /**
     * Destroys the advertiser and optionally disconnects from the AllJoyn bus.
     */
    virtual ~ObjectAdvertiser();

    /** \private
     * Get the implementation of the ObjectAdvertiser. Internal use only.
     */
    std::shared_ptr<ObjectAdvertiserImpl> GetImpl();

  private:
    ObjectAdvertiser(ajn::BusAttachment* bus = nullptr,
                     ajn::AboutData* aboutData = nullptr,
                     ajn::AboutObj* aboutObj = nullptr,
                     ajn::SessionOpts* opts = nullptr,
                     ajn::SessionPort sp = DATADRIVEN_SERVICE_PORT);

    ObjectAdvertiser(const ObjectAdvertiser&);

    void operator=(const ObjectAdvertiser&);

    std::shared_ptr<ObjectAdvertiserImpl> objectAdvertiserImpl;
};
}

#endif /* OBJECTADVERTISER_H_ */
