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

#include <cstdio>
#include <iostream>
#include <vector>
#include <memory>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <datadriven/datadriven.h>
#include "DoorInterface.h"

using namespace std;
using namespace gen::com_example;

class Door;
vector<unique_ptr<Door> > g_doors;
int g_turn = 0;

class Door :
    public datadriven::ProvidedObject, public DoorInterface {
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
        IsOpen = open;
        Location = location;
        code = 1234;
    }

    ~Door()
    {
    }

    const qcc::String& GetLocation() const
    {
        return Location;
    }

    QStatus GetKeyCode(uint32_t& _code) const
    {
        _code = code;
        return ER_OK;
    }

    /* Implement pure virtual function */
    void Open(shared_ptr<OpenReply> reply)
    {
        pthread_mutex_lock(&mutex);
        cout << "Door @ " << Location.c_str() << " was requested to open." << endl;
        if (IsOpen) {
            cout << "\t... but it was already open." << endl;
            /* Send an errorCode */
            reply->SendErrorCode(ER_FAIL);
        } else {
            cout << "\t... and it was closed, so we can comply." << endl;
            IsOpen = true;
            DoorInterface::Update();
            reply->Send();
        }
        cout << "[next up is " << g_doors[g_turn]->Location.c_str() << "] >";
        pthread_mutex_unlock(&mutex);
        cout.flush();
    }

    /* Implement pure virtual function */
    void Close(shared_ptr<CloseReply> reply)
    {
        pthread_mutex_lock(&mutex);
        cout << "Door @ " << Location.c_str() << " was requested to close." << endl;
        if (IsOpen) {
            cout << "\t... and it was open, so we can comply." << endl;
            IsOpen = false;
            DoorInterface::Update();
            reply->Send();
        } else {
            cout << "\t... but it was already closed." << endl;
            /* Send an error with a description */
            reply->SendError("org.allseenalliance.sample.Door.CloseError", "Could not close the door, already closed");
        }
        cout << "[next up is " << g_doors[g_turn]->Location.c_str() << "] >";
        pthread_mutex_unlock(&mutex);
        cout.flush();
    }

    /* Implement pure virtual function */
    void KnockAndRun()
    {
        pthread_mutex_lock(&mutex);
        if (!IsOpen) {
            // see who's there
            cout << "Someone knocked on door @ " << Location.c_str() << endl;
            cout << "\t... opening door" << endl;
            IsOpen = true;
            DoorInterface::Update();
            cout << "\t... GRRRR damn children!!!" << endl;
            cout << "\t... slamming door shut" << endl;
            IsOpen = false;
            DoorInterface::Update();
        } else {
            // door was open while knocking
            cout << "GOTCHA!!! @ " << Location.c_str() << " door" << endl;
        }
        pthread_mutex_unlock(&mutex);
    }

    void FlipOpen()
    {
        pthread_mutex_lock(&mutex);
        const char* action = IsOpen ? "Closing" : "Opening";
        cout << action << " door @ " << Location.c_str() << "." << endl;
        IsOpen = !IsOpen;
        DoorInterface::Update();
        pthread_mutex_unlock(&mutex);
    }

    void ChangeCode()
    {
        pthread_mutex_lock(&mutex);
        cout << "door @ " << Location.c_str() << ": change code" << endl;
        code = rand() % 10000; //code of max 4 digits
        InvalidateKeyCode();
        UpdateAll();
        pthread_mutex_unlock(&mutex);
    }

    // only here to be able to do extra tracing
    void PersonPassThrough(const qcc::String& who)
    {
        pthread_mutex_lock(&mutex);
        cout << who.c_str() << " will pass through door @ " << Location.c_str() << "." << endl;
        DoorInterface::PersonPassedThrough(who);
        pthread_mutex_unlock(&mutex);
    }
};

static void help()
{
    cout << "q         quit" << endl;
    cout << "f         flip the open state of the door" << endl;
    cout << "p <who>   signal that <who> passed through the door" << endl;
    cout << "r         remove or reattach the door to the bus" << endl;
    cout << "n         move to the next door in the list" << endl;
    cout << "c         change the code of the door" << endl;
    cout << "h         show this help message" << endl;
}

int main(int argc, char** argv)
{
    shared_ptr<datadriven::ObjectAdvertiser> advertiser = datadriven::ObjectAdvertiser::Create();
    if (nullptr == advertiser) {
        cerr << "Object advertiser not correctly initialized !!!" << endl;
        return EXIT_FAILURE;
    }

    /* parse command line arguments */
    if (argc == 1) {
        cerr << "Usage: " << argv[0] << " location1 [location2 [... [locationN] ...]]" << endl;
        return EXIT_FAILURE;
    }

    string path_root = "/Door/";
    for (int i = 1; i < argc; ++i) {
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "%s%d", path_root.c_str(), i);
        string path = string(buffer);
        unique_ptr<Door> door = unique_ptr<Door>(new Door(advertiser, argv[i], false, path.c_str()));

        if (ER_OK == door->GetStatus()) {
            if (ER_OK != door->PutOnBus()) {
                cerr << "Failed to announce door existence !" << endl;
            }
            g_doors.push_back(move(door));
        } else {
            cerr << "Failed to construct a door on location: " << argv[i] << " properly" << endl;
        }
    }

    if (g_doors.empty()) {
        cerr << "No doors available" << endl;
        return EXIT_FAILURE;
    }

    bool done = false;
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
                break;
            }

        case 'f': {
                g_doors[g_turn]->FlipOpen();
                break;
            }

        case 'c': {
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

        case 'n': {
                break;
            }

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

    return EXIT_SUCCESS;
}
