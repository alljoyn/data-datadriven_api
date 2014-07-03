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

#include <iostream>
#include <vector>
#include <stdlib.h>

#include <datadriven/datadriven.h>

#include "DoorProxy.h"

using namespace std;
using namespace gen::org_allseenalliance_sample;

class MyDoorListener :
    public datadriven::Observer<DoorProxy>::Listener {
  public:
    void OnUpdate(const shared_ptr<DoorProxy>& door)
    {
        const datadriven::ObjectId& id = door->GetObjectId();
        const DoorProxy::Properties prop = door->GetProperties();

        cout << "[listener] Update for door " << id << ": location = "
             << prop.location.c_str() << ", open = " << prop.open << ", code = "
             << door->Getcode()->GetReply().code << "." << endl;
        cout << "> ";
        cout.flush();
    }

    void OnRemove(const shared_ptr<DoorProxy>& door)
    {
        const datadriven::ObjectId& id = door->GetObjectId();
        const DoorProxy::Properties prop = door->GetProperties();

        cout << "[listener] Door " << id << " at location "
             << prop.location.c_str() << " does no longer exist." << endl;
        cout << "> ";
        cout.flush();
    }
};

class MyDoorSignalListener :
    public datadriven::SignalListener<DoorProxy, DoorProxy::PersonPassedThrough> {
    void OnSignal(const DoorProxy::PersonPassedThrough& signal)
    {
        const shared_ptr<DoorProxy> door = signal.GetEmitter(); //Retrieve the door
        const datadriven::ObjectId& id = door->GetObjectId();
        const DoorProxy::Properties prop = door->GetProperties();

        cout << "[listener] " << signal.who.c_str() << " passed through a door " << id
             << ": location = " << prop.location.c_str() << endl;
        cout << "> ";
        cout.flush();
    }
};

static void help()
{
    cout << "q             quit" << endl;
    cout << "l             list all discovered doors" << endl;
    cout << "o <location>  open door at <location>" << endl;
    cout << "c <location>  close door at <location>" << endl;
    cout << "k <location>  knock-and-run at <location>" << endl;
    cout << "h             display this help message" << endl;
}

static void list_doors(datadriven::Observer<DoorProxy>* observer)
{
    datadriven::Observer<DoorProxy>::iterator it = observer->begin();

    for (; it != observer->end(); ++it) {
        datadriven::ObjectId id = it->GetObjectId();
        DoorProxy::Properties prop = it->GetProperties();

        cout << "Door " << id << " location: " << prop.location.c_str() << " open: " << prop.open << endl;
    }
}

static shared_ptr<DoorProxy> get_door_at_location(datadriven::Observer<DoorProxy>* observer, const string& location)
{
    datadriven::Observer<DoorProxy>::iterator it = observer->begin();

    for (; it != observer->end(); ++it) {
        DoorProxy::Properties prop = it->GetProperties();

        if (!strcmp(prop.location.c_str(), location.c_str())) {
            return *it;
        }
    }
    cout << "Error: could not find door @ " << location << "." << endl;
    return shared_ptr<DoorProxy>();
}

static void open_door(datadriven::Observer<DoorProxy>* observer, const string& location)
{
    shared_ptr<DoorProxy> door = get_door_at_location(observer, location);

    if (door) {
        shared_ptr<datadriven::MethodInvocation<DoorProxy::OpenReply> > invocation = door->Open();
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

static void close_door(datadriven::Observer<DoorProxy>* observer, const string& location)
{
    shared_ptr<DoorProxy> door = get_door_at_location(observer, location);

    if (door) {
        shared_ptr<datadriven::MethodInvocation<DoorProxy::CloseReply> > invocation = door->Close();
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

static void knock_and_run(datadriven::Observer<DoorProxy>* observer, const string& location)
{
    shared_ptr<DoorProxy> door = get_door_at_location(observer, location);

    if (door) {
        if (ER_OK != door->KnockAndRun()) {
            cout << "A framework error occurred while trying to knock on door @ location "
                 << location.c_str() << endl;
        }
    }
}

static bool parse(datadriven::Observer<DoorProxy>* observer, const string& input)
{
    char cmd;
    size_t argpos;
    string arg = "";

    if (input.length() == 0) {
        return true;
    }

    cmd = input[0];
    argpos = input.find_first_not_of(" \t", 1);
    if (argpos != input.npos) {
        arg = input.substr(argpos);
    }

    switch (cmd) {
    case 'q':
        return false;

    case 'l':
        list_doors(observer);
        break;

    case 'o':
        open_door(observer, arg);
        break;

    case 'c':
        close_door(observer, arg);
        break;

    case 'k':
        knock_and_run(observer, arg);
        break;

    case 'h':
    default:
        help();
        break;
    }

    return true;
}

int main(int argc, char** argv)
{
    MyDoorListener dl = MyDoorListener();
    shared_ptr<datadriven::Observer<DoorProxy> > observer = datadriven::Observer<DoorProxy>::Create(&dl);

    if (nullptr == observer) {
        cerr << "Observer not correctly initialized !!!" << endl;
        return EXIT_FAILURE;
    }

    MyDoorSignalListener myDoorSignalListener = MyDoorSignalListener();
    if (ER_OK != observer->AddSignalListener<DoorProxy::PersonPassedThrough>(myDoorSignalListener)) {
        cerr << "Could not add Signal Listener to the Observer !!!" << endl;
        return EXIT_FAILURE;
    }

    bool done = false;
    while (!done) {
        string input;
        cout << "> ";
        getline(cin, input);
        done = !parse(observer.get(), input);
    }

    // Cleanup
    int result = EXIT_SUCCESS;
    if (ER_OK != observer->RemoveSignalListener<DoorProxy::PersonPassedThrough>(myDoorSignalListener)) {
        cerr << "Could not remove Signal Listener from the Observer !!!" << endl;
        result = EXIT_FAILURE;
    }

    return result;
}
