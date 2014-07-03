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

/* This hybrid door_provider extends the regular door_consumer in the following ways:
 * - We use the standard core alljoyn API to create a busattachment (which we pass to the datadriven busconnection)
 * - The application will advertise itself with About
 */

#include <cstdio>
#include <iostream>
#include <vector>
#include <semaphore.h>
#include <memory>
#include <stdlib.h>
#include <pthread.h>
#include <datadriven/datadriven.h>
#include "DoorInterface.h"
#include "DoorProxy.h"

#include "CommonSampleUtil.h"
#include <alljoyn/about/AboutServiceApi.h>
#include <alljoyn/services_common/LogModulesNames.h>
#include <alljoyn/about/AnnouncementRegistrar.h>

using namespace std;
using namespace::gen::org_allseenalliance_sample;
using namespace ajn;
using namespace services;
using namespace qcc;

static SessionId s_sessionId = 0; /* both for provider and consumer */
class Door;
vector<std::unique_ptr<Door> > g_doors;
int g_turn = 0;

static int run_test(datadriven::Observer<DoorProxy>& observer);

class Door :
    public datadriven::ProvidedObject, public DoorInterface {
    friend int::run_test(datadriven::Observer<DoorProxy>&observer);

  private:
    pthread_mutex_t mutex;
    uint32_t code;

  public:
    Door(shared_ptr<datadriven::ObjectAdvertiser> advertiser,
         qcc::String location,
         bool open = false,
         qcc::String path = "") :
        datadriven::ProvidedObject(advertiser, path),   /* If you don't pass a path, it will be constructed for you */
        DoorInterface(this), mutex(PTHREAD_MUTEX_INITIALIZER)
    {
        this->open = open;
        this->location = location;
        code = 1234;
    }

    ~Door()
    {
    }

    const qcc::String& GetLocation() const
    {
        return location;
    }

    /* Implement pure virtual function */
    void Open(OpenReply& reply)
    {
        pthread_mutex_lock(&mutex);
        cout << "Door @ " << location.c_str() << " was requested to open." << endl;
        if (this->open) {
            cout << "\t... but it was already open." << endl;
            reply.SendErrorCode(ER_FAIL);
        } else {
            cout << "\t... and it was closed, so we can comply." << endl;
            this->open = true;
            this->DoorInterface::Update();
            reply.Send();
        }
        cout << "[next up is " << g_doors[g_turn]->location.c_str() << "] >";
        pthread_mutex_unlock(&mutex);
        cout.flush();
    }

    /* Implement pure virtual function */
    void Close(CloseReply& reply)
    {
        pthread_mutex_lock(&mutex);
        cout << "Door @ " << location.c_str() << " was requested to close." << endl;
        if (this->open) {
            cout << "\t... and it was open, so we can comply." << endl;
            this->open = false;
            this->DoorInterface::Update();
            reply.Send();
        } else {
            cout << "\t... but it was already closed." << endl;
            reply.SendError("org.allseenalliance.sample.Door.CloseError", "Could not close the door, already closed");
        }
        cout << "[next up is " << g_doors[g_turn]->location.c_str() << "] >";
        pthread_mutex_unlock(&mutex);
        cout.flush();
    }

    /* Implement pure virtual function */
    void KnockAndRun()
    {
        pthread_mutex_lock(&mutex);
        cout << "Door @ " << location.c_str() << " has being knocked upon." << endl;
        if (!this->open) {
            // see who's there
            this->open = true;
            this->DoorInterface::Update();
            cout << "\t... GRRRR... damn children" << endl;
            this->open = false;
            this->DoorInterface::Update();
        }
        pthread_mutex_unlock(&mutex);
    }

    void FlipOpen()
    {
        pthread_mutex_lock(&mutex);
        const char* action = open ? "Closing" : "Opening";
        cout << action << " door @ " << location.c_str() << "." << endl;
        open = !open;
        this->DoorInterface::Update();
        pthread_mutex_unlock(&mutex);
    }

    // only here to be able to do extra tracing
    void PersonPassThrough(const qcc::String& who)
    {
        pthread_mutex_lock(&mutex);
        cout << who.c_str() << " will pass through door @ " << location.c_str() << "." << endl;
        DoorInterface::PersonPassedThrough(who);
        pthread_mutex_unlock(&mutex);
    }

    void ChangeCode()
    {
        pthread_mutex_lock(&mutex);
        cout << "door @ " << location.c_str() << ": change code" << endl;
        code = rand() % 10000; //code of max 4 digits
        Invalidatecode();
        UpdateAll();
        pthread_mutex_unlock(&mutex);
    }

    QStatus Getcode(uint32_t& _code) const
    {
        _code = code;
        return ER_OK;
    }
};

static void help()
{
    cout << "q         quit" << endl;
    cout << "t         run test" << endl;
    cout << "Provider actions" << endl;
    cout << "------------------------------------------" << endl;
    cout << "f         flip the open state of the door" << endl;
    cout << "p <who>   signal that <who> passed through the door" << endl;
    cout << "r         remove or reattach the door to the bus" << endl;
    cout << "n         move to the next door in the list" << endl;
    cout << "s         emit signal" << endl;
    cout << "x         change code" << endl;
    cout << "Consumer actions" << endl;
    cout << "------------------------------------------" << endl;
    cout << "l             list all discovered doors" << endl;
    cout << "o <location>  open door at <location>" << endl;
    cout << "c <location>  close door at <location>" << endl;
    cout << "h             display this help message" << endl;
    cout << "m             concatenate string" << endl;
    cout << "h         show this help message" << endl;
}

BusAttachment* bus = 0;
AboutPropertyStoreImpl* propertyStoreImpl = 0;
CommonBusListener*  busListener = 0;

static void cleanup(bool busHalted = false)
{
    if (bus && busListener) {
        CommonSampleUtil::aboutServiceDestroy(bus, busListener);
    }
    if (busListener) {
        delete busListener;
        busListener = NULL;
    }
    if (propertyStoreImpl) {
        delete propertyStoreImpl;
        propertyStoreImpl = NULL;
    }
    if (bus) {
        if (busHalted == false) {
            bus->Disconnect();
            bus->Stop();
            bus->Join();
        }
        delete bus;
        bus = NULL;
    }
    std::cout << "Goodbye!" << std::endl;
}

/******************************************************************/
/*      BASICSAMPLEOBJECT START                                   */
/******************************************************************/

static const char* INTERFACE_NAME = "org.alljoyn.Bus.sample";
static const char* SERVICE_NAME = "org.alljoyn.Bus.sample";
static const char* SERVICE_PATH = "/sample";
static const char* SERVICE_PATH2 = "/sample2";
static const SessionPort SERVICE_PORT = 25;

class BasicSampleObject :
    public BusObject {
  private:
    const InterfaceDescription::Member* nameChangedMember;

  public:
    BasicSampleObject(BusAttachment& bus,
                      const char* path) :
        BusObject(path)
    {
        /** Add the test interface to this object */
        const InterfaceDescription* exampleIntf = bus.GetInterface(INTERFACE_NAME);
        nameChangedMember = exampleIntf->GetMember("nameChanged");
        assert(exampleIntf);
        AddInterface(*exampleIntf);

        /** Register the method handlers with the object */
        const MethodEntry methodEntries[] = {
            { exampleIntf->GetMember("cat"), static_cast<MessageReceiver::MethodHandler>(&BasicSampleObject::Cat) }
        };
        QStatus status = AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(methodEntries[0]));
        if (ER_OK != status) {
            printf("Failed to register method handlers for BasicSampleObject.\n");
        }
    }

    QStatus EmitNameChangedSignal(qcc::String newName)
    {
        printf("Emiting Name Changed Signal.\n");
        assert(nameChangedMember);
        printf("Sending NameChanged signal without a session id\n");
        MsgArg arg("s", newName.c_str());
        uint8_t flags = 0;
        QStatus status = Signal(NULL, s_sessionId, *nameChangedMember, &arg, 1, 0, flags);
        if (status != ER_OK) {
            printf("Could not emit signal\r\n");
        }

        return status;
    }

    void ObjectRegistered()
    {
        BusObject::ObjectRegistered();
        printf("ObjectRegistered has been called.\n");
    }

    void Cat(const InterfaceDescription::Member* member, Message& msg)
    {
        /* Concatenate the two input strings and reply with the result. */
        qcc::String inStr1 = msg->GetArg(0)->v_string.str;
        qcc::String inStr2 = msg->GetArg(1)->v_string.str;
        qcc::String outStr = inStr1 + inStr2;

        MsgArg outArg("s", outStr.c_str());
        QStatus status = MethodReply(msg, &outArg, 1);
        if (ER_OK != status) {
            printf("Ping: Error sending reply.\n");
        }
    }
};
static BasicSampleObject* bso;

/** Create the interface, report the result to stdout, and return the result status. */
QStatus CreateInterface(void)
{
    /* Add org.alljoyn.Bus.method_sample interface */
    InterfaceDescription* testIntf = NULL;
    QStatus status = bus->CreateInterface(INTERFACE_NAME, testIntf);

    if (status == ER_OK) {
        printf("Interface created.\n");
        testIntf->AddMethod("cat", "ss",  "s", "inStr1,inStr2,outStr", 0);
        testIntf->AddSignal("nameChanged", "s", "newName", 0);
        testIntf->Activate();
    } else {
        printf("Failed to create interface '%s'.\n", INTERFACE_NAME);
    }

    return status;
}

/** Register the bus object and connect, report the result to stdout, and return the status code. */
QStatus RegisterBusObject(BasicSampleObject* obj)
{
    QStatus status = bus->RegisterBusObject(*obj);

    if (ER_OK == status) {
        printf("RegisterBusObject succeeded.\n");
    } else {
        printf("RegisterBusObject failed (%s).\n", QCC_StatusText(status));
    }

    return status;
}

/** Advertise the service name, report the result to stdout, and return the status code. */
QStatus AdvertiseName(TransportMask mask)
{
    QStatus status = bus->AdvertiseName(SERVICE_NAME, mask);

    if (ER_OK == status) {
        printf("Advertisement of the service name '%s' succeeded.\n", SERVICE_NAME);
    } else {
        printf("Failed to advertise name '%s' (%s).\n", SERVICE_NAME, QCC_StatusText(status));
    }

    return status;
}

/** Request the service name, report the result to stdout, and return the status code. */
QStatus RequestName(void)
{
    const uint32_t flags = DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE;
    QStatus status = bus->RequestName(SERVICE_NAME, flags);

    if (ER_OK == status) {
        printf("RequestName('%s') succeeded.\n", SERVICE_NAME);
    } else {
        printf("RequestName('%s') failed (status=%s).\n", SERVICE_NAME, QCC_StatusText(status));
    }

    return status;
}

/******************************************************************/
/*      BASICSAMPLEOBJECT END                                     */
/******************************************************************/

static int StartAlljoynApplication()
{
    QStatus status;

    propertyStoreImpl = new AboutPropertyStoreImpl();
    busListener = new CommonBusListener();
    status = CommonSampleUtil::prepareAboutService(bus, propertyStoreImpl, busListener, SERVICE_PORT);
    if (status != ER_OK) {
        std::cout << "Could not set up the AboutService." << std::endl;
        cleanup();
        return 1;
    }

    status = CommonSampleUtil::aboutServiceAnnounce();
    if (status != ER_OK) {
        std::cout << "Could not announce." << std::endl;
        cleanup();
        return 1;
    }

    status = CreateInterface();
    if (status != ER_OK) {
        std::cerr << "Failure" << __LINE__ << std::endl;
    }
    status = RegisterBusObject(bso = new BasicSampleObject(*bus, SERVICE_PATH));
    if (status != ER_OK) {
        std::cerr << "Failure" << __LINE__ << std::endl;
    }
    const TransportMask SERVICE_TRANSPORT_TYPE = TRANSPORT_ANY;
    status = RequestName();
    if (status != ER_OK) {
        std::cerr << "Failure" << __LINE__ << std::endl;
    }
    status = AdvertiseName(SERVICE_TRANSPORT_TYPE);
    if (status != ER_OK) {
        std::cerr << "Failure" << __LINE__ << std::endl;
    }

    return 0;
}

/**************************************************************/
/*          CONSUMER START                                    */
/*************************************************************/
static sem_t namechanged;
static char name[128];

class SignalListeningObject :
    public BusObject {
  public:
    SignalListeningObject(BusAttachment& bus,
                          const char* path) :
        BusObject(path),
        nameChangedMember(NULL)
    {
        const InterfaceDescription* intf = NULL;
        intf = bus.GetInterface(INTERFACE_NAME);
        QStatus status = AddInterface(*intf);

        if (status == ER_OK) {
            printf("Interface successfully added to the object.\n");
            /* Register the signal handler 'nameChanged' with the bus*/
            nameChangedMember = intf->GetMember("nameChanged");
            assert(nameChangedMember);
        } else {
            printf("Failed to Add interface: %s.", INTERFACE_NAME);
        }

        /* register the signal handler for the the 'nameChanged' signal */
        status =  bus.RegisterSignalHandler(this,
                                            static_cast<MessageReceiver::SignalHandler>(&SignalListeningObject::
                                                                                        NameChangedSignalHandler),
                                            nameChangedMember,
                                            NULL);
        if (status != ER_OK) {
            printf("Failed to register signal handler for %s.nameChanged.\n", SERVICE_NAME);
        } else {
            printf("Registered signal handler for %s.nameChanged.\n", SERVICE_NAME);
        }
    }

    QStatus SubscribeNameChangedSignal(void)
    {
        assert(bus);
        return bus->AddMatch("type='signal',interface='org.alljoyn.Bus.sample',member='nameChanged'");
    }

    void NameChangedSignalHandler(const InterfaceDescription::Member* member,
                                  const char* sourcePath,
                                  Message& msg)
    {
        printf("--==## signalConsumer: Name Changed signal Received ##==--\n");
        printf("\tNew name: '%s'.\n", msg->GetArg(0)->v_string.str);
        strcpy(name, msg->GetArg(0)->v_string.str);
        sem_post(&namechanged);
    }

  private:
    const InterfaceDescription::Member* nameChangedMember;
};

static void list_doors(datadriven::Observer<DoorProxy>& observer)
{
    datadriven::Observer<DoorProxy>::iterator it = observer.begin();

    for (; it != observer.end(); ++it) {
        datadriven::ObjectId id = it->GetObjectId();
        DoorProxy::Properties prop = it->GetProperties();

        cout << "Door " << id << " location: " << prop.location.c_str() << " open: " << prop.open << endl;
    }
}

static shared_ptr<DoorProxy> get_door_at_location(datadriven::Observer<DoorProxy>& observer, const string& location)
{
    datadriven::Observer<DoorProxy>::iterator it = observer.begin();

    for (; it != observer.end(); ++it) {
        DoorProxy::Properties prop = it->GetProperties();

        if (!strcmp(prop.location.c_str(), location.c_str())) {
            return *it;
        }
    }
    cout << "Error: could not find door @ " << location << "." << endl;
    return shared_ptr<DoorProxy>();
}

static void open_door(datadriven::Observer<DoorProxy>& observer, const string& location)
{
    shared_ptr<DoorProxy> door = get_door_at_location(observer, location);

    if (door) {
        std::shared_ptr<datadriven::MethodInvocation<DoorProxy::OpenReply> > invocation = door->Open();
        DoorProxy::OpenReply reply = invocation->GetReply();

        if (ER_OK == reply.GetStatus()) {
            /* No error */

            cout << "Opening of door succeeded" << endl;
        } else if (ER_BUS_REPLY_IS_ERROR_MESSAGE == reply.GetStatus()) {
            /* MethodReply Error received (an error string) */

            cout << "Opening of door @ location " << location.c_str()
                 << " returned an error \"" << reply.GetErrorName().c_str()
                 << "\" (" << reply.GetErrorDescription().c_str() << ")" << endl;
        } else {
            /* Framework error or MethodReply error code */

            cout << "Opening of door @ location " << location.c_str()
                 << " returned an error \"" <<  QCC_StatusText(reply.GetStatus()) << "\"" << endl;
        }
    }
}

static void close_door(datadriven::Observer<DoorProxy>& observer, const string& location)
{
    shared_ptr<DoorProxy> door = get_door_at_location(observer, location);

    if (door) {
        std::shared_ptr<datadriven::MethodInvocation<DoorProxy::CloseReply> > invocation = door->Close();
        DoorProxy::CloseReply reply = invocation->GetReply();
        if (ER_OK == reply.GetStatus()) {
            /* No error */

            cout << "Closing of door succeeded" << endl;
        } else if (ER_BUS_REPLY_IS_ERROR_MESSAGE == reply.GetStatus()) {
            /* MethodReply Error received (an error string) */

            cout << "Closing of door @ location " << location.c_str()
                 << " returned an error \"" << reply.GetErrorName().c_str()
                 << "\" (" << reply.GetErrorDescription().c_str() << ")" << endl;
        } else {
            /* Framework error or MethodReply error code */

            cout << "Closing of door @ location " << location.c_str()
                 << " returned an error \"" <<  QCC_StatusText(reply.GetStatus()) << "\"" << endl;
        }
    }
}

static bool method_call()
{
    bool retval = false;
    ProxyBusObject remoteObj(*bus, SERVICE_NAME, SERVICE_PATH, s_sessionId);
    const InterfaceDescription* alljoynTestIntf = bus->GetInterface(INTERFACE_NAME);

    assert(alljoynTestIntf);
    remoteObj.AddInterface(*alljoynTestIntf);

    Message reply(*bus);
    MsgArg inputs[2];

    inputs[0].Set("s", "Hello ");
    inputs[1].Set("s", "World!");

    QStatus status = remoteObj.MethodCall(SERVICE_NAME, "cat", inputs, 2, reply, 5000);

    if (ER_OK == status) {
        printf("'%s.%s' (path='%s') returned '%s'.\n", SERVICE_NAME, "cat",
               SERVICE_PATH, reply->GetArg(0)->v_string.str);
        if (strcmp(reply->GetArg(0)->v_string.str, "Hello World!") == 0) {
            retval = true;
        }
    } else {
        printf("MethodCall on '%s.%s' failed.", SERVICE_NAME, "cat");
    }

    return retval;
}

class MyBusListener :
    public BusListener, public SessionListener {
  public:
    void FoundAdvertisedName(const char* name, TransportMask transport, const char* namePrefix)
    {
        if (0 == strcmp(name, SERVICE_NAME)) {
            printf("FoundAdvertisedName(name='%s', prefix='%s')\n", name, namePrefix);

            /* We found a remote bus that is advertising basic service's well-known name so connect to it. */
            /* Since we are in a callback we must enable concurrent callbacks before calling a synchronous method. */
            bus->EnableConcurrentCallbacks();
            SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
            QStatus status = bus->JoinSession(name, SERVICE_PORT, this, s_sessionId, opts);
            if (ER_OK == status) {
                printf("JoinSession SUCCESS (Session id=%d).\n", s_sessionId);
            } else {
                printf("JoinSession failed (status=%s).\n", QCC_StatusText(status));
            }
        }
    }

    void NameOwnerChanged(const char* busName, const char* previousOwner, const char* newOwner)
    {
        if (newOwner && (0 == strcmp(busName, SERVICE_NAME))) {
            printf("NameOwnerChanged: name='%s', oldOwner='%s', newOwner='%s'.\n",
                   busName,
                   previousOwner ? previousOwner : "<none>",
                   newOwner ? newOwner : "<none>");
        }
    }
};

SignalListeningObject* object;

/** Register a bus listener in order to get discovery indications and report the event to stdout. */
void RegisterBusListener(void)
{
    /* Static bus listener */
    static MyBusListener s_busListener;

    bus->RegisterBusListener(s_busListener);
    printf("BusListener Registered.\n");
}

/** Begin discovery on the well-known name of the service to be called, report the result to
   stdout, and return the result status. */
QStatus FindAdvertisedName(void)
{
    /* Begin discovery on the well-known name of the service to be called */
    QStatus status = bus->FindAdvertisedName(SERVICE_NAME);

    if (status == ER_OK) {
        printf("org.alljoyn.Bus.FindAdvertisedName ('%s') succeeded.\n", SERVICE_NAME);
    } else {
        printf("org.alljoyn.Bus.FindAdvertisedName ('%s') failed (%s).\n", SERVICE_NAME, QCC_StatusText(status));
    }

    return status;
}

static QStatus ConsumerInit()
{
    RegisterBusListener();
    QStatus status = FindAdvertisedName();

    SignalListeningObject* object = new SignalListeningObject(*bus, SERVICE_PATH2);

    if (bus->RegisterBusObject(*object) != ER_OK) {
        std::cerr << "ERROR: Could not register object" << std::endl;
        return ER_FAIL;
    }

    object->SubscribeNameChangedSignal();

    return status;
}

static sem_t onupdate;
static sem_t onremove;
/**************************************************************/
/*          CONSUMER END                                    */
/*************************************************************/
class MyDoorListener :
    public datadriven::Observer<DoorProxy>::Listener {
  public:
    void OnUpdate(const std::shared_ptr<DoorProxy>& door)
    {
        const datadriven::ObjectId& id = door->GetObjectId();
        const DoorProxy::Properties prop = door->GetProperties();

        cout << "[listener] Update for door " << id << ": location = "
             << prop.location.c_str() << ", open = " << prop.open << ", code = " << door->Getcode()->GetReply().code <<
        "." << endl;
        cout << "> ";
        cout.flush();
        sem_post(&onupdate);
    }

    void OnRemove(const std::shared_ptr<DoorProxy>& door)
    {
        const datadriven::ObjectId& id = door->GetObjectId();
        const DoorProxy::Properties prop = door->GetProperties();

        cout << "[listener] Door " << id << " at location "
             << prop.location.c_str() << " does no longer exist." << endl;
        cout << "> ";
        cout.flush();
        sem_post(&onremove);
    }
};

static sem_t onsignal;
static qcc::String lastPerson;

class MyDoorSignalListener :
    public datadriven::SignalListener<DoorProxy, DoorProxy::PersonPassedThrough> {
    void OnSignal(const DoorProxy::PersonPassedThrough& signal)
    {
        const std::shared_ptr<DoorProxy> door = signal.GetEmitter(); //Retrieve the door
        const datadriven::ObjectId& id = door->GetObjectId();
        const DoorProxy::Properties prop = door->GetProperties();

        cout << "[listener] " << signal.who.c_str() << " passed through a door " << id
             << ": location = " << prop.location.c_str() << endl;
        cout << "> ";
        cout.flush();
        lastPerson = signal.who;
        sem_post(&onsignal);
    }
};

static bool init_test()
{
    if (sem_init(&onsignal, 0, 0) != 0) {
        cerr << "FAIL: Could not init semaphore " << strerror(errno) << std::endl;
        return false;
    }
    if (sem_init(&onupdate, 0, 0) != 0) {
        cerr << "FAIL: Could not init semaphore " << strerror(errno) << std::endl;
        return false;
    }
    if (sem_init(&onremove, 0, 0) != 0) {
        cerr << "FAIL: Could not init semaphore " << strerror(errno) << std::endl;
        return false;
    }
    if (sem_init(&namechanged, 0, 0) != 0) {
        cerr << "FAIL: Could not init semaphore " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

static int run_test(datadriven::Observer<DoorProxy>& observer)
{
    int ret;
    struct timespec twosec;
    twosec.tv_sec = time(NULL) + 5;
    twosec.tv_nsec = 0;

    /* --------- DDAPI ----------- */
    for (size_t i = 0; i < g_doors.size(); ++i) {
        ret = sem_timedwait(&onupdate, &twosec);
        if (ret != 0) {
            cerr << "FAIL: sem_timedwait() failed: " << strerror(errno) << std::endl;
            cerr << "FAIL: Did not see all doors " << std::endl;
            return EXIT_FAILURE;
        }
    }

    //emit signal
    g_doors[0]->PersonPassedThrough("alice");
    ret = sem_timedwait(&onsignal, &twosec);
    if (ret != 0) {
        cerr << "FAIL: sem_timedwait() failed: " << strerror(errno) << std::endl;
        cerr << "FAIL: Did not see signal " << std::endl;
        return EXIT_FAILURE;
    }

    if (lastPerson != "alice") {
        cerr << "FAIL: Door signal failed " << std::endl;
        return EXIT_FAILURE;
    }

    /* open all doors */
    datadriven::Observer<DoorProxy>::iterator it = observer.begin();
    for (; it != observer.end(); ++it) {
        datadriven::ObjectId id = it->GetObjectId();
        DoorProxy::Properties prop = it->GetProperties();
        std::shared_ptr<datadriven::MethodInvocation<DoorProxy::OpenReply> > invocation = it->Open();
        DoorProxy::OpenReply reply = invocation->GetReply();
        if (ER_OK != reply.GetStatus()) {
            cerr << "Invocation error occurred while trying to open a door " << endl;
            return EXIT_FAILURE;
        }
    }

    for (size_t i = 0; i < g_doors.size(); ++i) {
        if (g_doors[i]->open == false) {
            cerr << "Door was not open ! " << endl;
            return EXIT_FAILURE;
        }
    }

    /* remove doors */
    for (size_t i = 0; i < g_doors.size(); ++i) {
        g_doors[i]->RemoveFromBus();
    }

    for (size_t i = 0; i < g_doors.size(); ++i) {
        ret = sem_timedwait(&onremove, &twosec);
        if (ret != 0) {
            cerr << "FAIL: sem_timedwait() failed: " << strerror(errno) << std::endl;
            cerr << "FAIL: Did not see all doors removed " << std::endl;
            return EXIT_FAILURE;
        }
    }

    /* --------- AJN ----------- */
    if (method_call() == false) {
        cerr << "FAIL: method call failed" << endl;
        return EXIT_FAILURE;
    }

    bso->EmitNameChangedSignal("mynewname");
    ret = sem_timedwait(&namechanged, &twosec);
    if (ret != 0) {
        cerr << "FAIL: sem_timedwait() failed: " << strerror(errno) << std::endl;
        cerr << "FAIL: Did not see AJN signal " << std::endl;
        return EXIT_FAILURE;
    }

    if (strcmp(name, "mynewname") != 0) {
        cerr << "FAIL: received name did not match" << endl;
        return EXIT_FAILURE;
    }

    cout << "PASSED" << endl;
    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
    init_test();
    int retval = EXIT_FAILURE;
    bus = CommonSampleUtil::prepareBusAttachment();
    if (bus == NULL) {
        std::cout << "Could not initialize BusAttachment." << std::endl;
        return 1;
    }

    if (StartAlljoynApplication() == 1) {
        cerr << "Start AJN application failed !!!" << endl;
        return EXIT_FAILURE;
    }

    if (ER_OK != ConsumerInit()) {
        cerr << "ConsumerInit failed" << endl;
        return EXIT_FAILURE;
    }
    MyDoorListener dl = MyDoorListener();
    shared_ptr<datadriven::Observer<DoorProxy> > observer = datadriven::Observer<DoorProxy>::Create(&dl, bus);
    if (nullptr == observer) {
        cerr << "Observer not correctly initialized !!!" << endl;
        return EXIT_FAILURE;
    }

    MyDoorSignalListener myDoorSignalListener = MyDoorSignalListener();
    if (ER_OK != observer->AddSignalListener<DoorProxy::PersonPassedThrough>(myDoorSignalListener)) {
        cerr << "Could not add Signal Listener to the Observer !!!" << endl;
        return EXIT_FAILURE;
    }

    /* parse command line arguments */
    if (argc == 1) {
        cerr << "Usage: " << argv[0] << " location1 [location2 [... [locationN] ...]]" << endl;
        return EXIT_FAILURE;
    }

    shared_ptr<datadriven::ObjectAdvertiser> advertiser = datadriven::ObjectAdvertiser::Create(bus, propertyStoreImpl);
    if (nullptr == advertiser) {
        cerr << "Object advertiser not correctly initialized !!!" << endl;
        return EXIT_FAILURE;
    }

    string path_root = "/Door/";
    for (int i = 1; i < argc; ++i) {
        string path = path_root + std::to_string(i);
        std::unique_ptr<Door> door = std::unique_ptr<Door>(new Door(advertiser, argv[i], false, path.c_str()));

        if (ER_OK == door->GetStatus()) {
            if (ER_OK != door->PutOnBus()) {
                cerr << "Failed to announce door existence !" << endl;
            }
            g_doors.push_back(std::move(door));
        } else {
            cerr << "Failed to construct a door on location: " << argv[i] << " properly" << endl;
        }
    }

    if (g_doors.empty()) {
        cerr << "No doors available" << endl;
        return EXIT_FAILURE;
    }

    bool done = false;

    if (getenv("RUNTEST")) {
        retval = run_test(*observer);
        done = true;
    }

    while (!done) {
        cout << "[next up is " << g_doors[g_turn]->GetLocation().c_str() << "] >";
        string input;
        getline(cin, input);
        if (input.length() == 0) {
            continue;
        }
        bool nextDoor = true;
        char cmd = input[0];
        switch (cmd) {
        case 'q': {
                done = true;
                retval = EXIT_SUCCESS;
                break;
            }

        case 'f': {
                g_doors[g_turn]->FlipOpen();
                break;
            }

        case 'x': {
                g_doors[g_turn]->ChangeCode();
                break;
            }

        case 'p': {
                size_t whopos = input.find_first_not_of(" \t", 1);
                if (whopos == input.npos) {
                    help();
                    break;
                }
                string who = input.substr(whopos);
                g_doors[g_turn]->PersonPassedThrough(who.c_str());
                break;
            }

        case 'r': {
                Door& d = *g_doors[g_turn];
                if (datadriven::ProvidedObject::REGISTERED == d.GetState()) {
                    d.RemoveFromBus();
                } else if (ER_OK != d.UpdateAll()) {
                    cerr << "Failed to completely remove door !" << endl;
                }
                break;
            }

        case 's':
            bso->EmitNameChangedSignal("mynewname");
            break;

        case 'n': {
                break;
            }

        case 'l':
            list_doors(*observer);
            break;

        case 'o':
            {
                size_t doorpos = input.find_first_not_of(" \t", 1);
                if (doorpos == input.npos) {
                    help();
                    break;
                }
                string arg = input.substr(doorpos);
                open_door(*observer, arg);
                break;
            }

        case 'c':
            {
                size_t doorpos = input.find_first_not_of(" \t", 1);
                if (doorpos == input.npos) {
                    help();
                    break;
                }
                string arg = input.substr(doorpos);
                close_door(*observer, arg);
                break;
            }
            break;

        case 'm':
            method_call();
            break;

        case 't':
            retval = run_test(*observer);
            done = true;

            break;

        case 'h':
        default: {
                help();
                nextDoor = false;
                break;
            }
        }

        if (true == nextDoor) {
            g_turn = (g_turn + 1) % g_doors.size();
        }
    }

    bus->Disconnect();
    bus->Stop();
    bus->Join();

    // before really cleaning up the bus, clean up the DDAPI objects
    advertiser.reset();
    observer.reset();

    cleanup(true);

    return retval;
}
