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
using namespace::gen::org_allseenalliance_sample;

class MyDoorListener :
    public datadriven::Observer<DoorProxy>::Listener {
  public:
    void OnUpdate(const std::shared_ptr<DoorProxy>& door)
    {
        const datadriven::ObjectId& id = door->GetObjectId();
        const DoorProxy::Properties prop = door->GetProperties();

        cout << "[listener] Update for door " << id << ": location = "
             << prop.location.c_str() << " open = " << prop.open << "." << endl;
        cout << "> ";
        cout.flush();
    }

    void OnRemove(const std::shared_ptr<DoorProxy>& door)
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
        const std::shared_ptr<DoorProxy> door = signal.GetEmitter(); //Retrieve the door
        const datadriven::ObjectId& id = door->GetObjectId();
        const DoorProxy::Properties prop = door->GetProperties();

        cout << "[listener] " << signal.who.c_str() << " passed through a door " << id
             << ": location = " << prop.location.c_str() << endl;
        cout << "> ";
        cout.flush();
    }
};

datadriven::BusConnection g_busConnection;
MyDoorListener dl = MyDoorListener();
datadriven::Observer<DoorProxy> g_observer(g_busConnection, &dl);

static void help()
{
    cout << "q             quit" << endl;
    cout << "l             list all discovered doors" << endl;
    cout << "o <location>  open door at <location>" << endl;
    cout << "c <location>  close door at <location>" << endl;
    cout << "h             display this help message" << endl;
}

static void list_doors()
{
    datadriven::Observer<DoorProxy>::iterator it = g_observer.begin();

    for (; it != g_observer.end(); ++it) {
        datadriven::ObjectId id = it->GetObjectId();
        DoorProxy::Properties prop = it->GetProperties();

        cout << "Door " << id << " location: " << prop.location.c_str() << " open: " << prop.open << endl;
    }
}

static shared_ptr<DoorProxy> get_door_at_location(const string& location)
{
    datadriven::Observer<DoorProxy>::iterator it = g_observer.begin();

    for (; it != g_observer.end(); ++it) {
        DoorProxy::Properties prop = it->GetProperties();

        if (!strcmp(prop.location.c_str(), location.c_str())) {
            return *it;
        }
    }
    cout << "Error: could not find door @ " << location << "." << endl;
    return shared_ptr<DoorProxy>();
}

static void open_door(const string& location)
{
    shared_ptr<DoorProxy> door = get_door_at_location(location);

    if (door) {
        std::shared_ptr<datadriven::MethodInvocation<DoorProxy::OpenReply> > invocation = door->Open();
        DoorProxy::OpenReply reply = invocation->GetReply();

        if (ER_OK == reply.GetStatus()) {
            cout << "Opening of door " << (reply.success ? "succeeded" : "failed") << endl;
        } else {
            cout << "Invocation error occurred while trying to open a door @ location: " << location.c_str() << endl;
        }
    }
}

static void close_door(const string& location)
{
    shared_ptr<DoorProxy> door = get_door_at_location(location);

    if (door) {
        std::shared_ptr<datadriven::MethodInvocation<DoorProxy::CloseReply> > invocation = door->Close();
        DoorProxy::CloseReply reply = invocation->GetReply();
        if (ER_OK == reply.GetStatus()) {
            cout << "Closing of door " << (reply.success ? "succeeded" : "failed") << endl;
        } else {
            cout << "Invocation error occurred while trying to close a door @ location: " << location.c_str() << endl;
        }
    }
}

static bool parse(const string& input)
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
        list_doors();
        break;

    case 'o':
        open_door(arg);
        break;

    case 'c':
        close_door(arg);
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
    if (ER_OK != g_busConnection.GetStatus()) {
        cerr << "Bus Connection not correctly initialized !!!" << endl;
        return EXIT_FAILURE;
    }

    if (ER_OK != g_observer.GetStatus()) {
        cerr << "Observer not correctly initialized !!!" << endl;
        return EXIT_FAILURE;
    }

    MyDoorSignalListener myDoorSignalListener = MyDoorSignalListener();
    if (ER_OK != g_observer.AddSignalListener<DoorProxy::PersonPassedThrough>(myDoorSignalListener)) {
        cerr << "Could not add Signal Listener to the Observer !!!" << endl;
        return EXIT_FAILURE;
    }

    bool done = false;
    while (!done) {
        string input;
        cout << "> ";
        getline(cin, input);
        done = !parse(input);
    }

    // Cleanup
    int result = EXIT_SUCCESS;
    if (ER_OK != g_observer.RemoveSignalListener<DoorProxy::PersonPassedThrough>(myDoorSignalListener)) {
        cerr << "Could not remove Signal Listener from the Observer !!!" << endl;
        result = EXIT_FAILURE;
    }

    return result;
}
