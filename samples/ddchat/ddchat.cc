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
#include <string>

#include <unistd.h>

#include <datadriven/datadriven.h>

#include <ChatParticipantInterface.h>
#include <ChatParticipantProxy.h>
#include <cstdio>

using namespace std;
using namespace datadriven;
using namespace gen::com_example;

class Participant :
    public ProvidedObject, public ChatParticipantInterface {
  public:
    Participant(shared_ptr<ObjectAdvertiser> advertiser,
                const qcc::String& name) :
        ProvidedObject(advertiser),
        ChatParticipantInterface(this)
    {
        Name = name;
    }
};

class ChatListener :
    public Observer<ChatParticipantProxy>::Listener {
  public:
    virtual void OnUpdate(const shared_ptr<ChatParticipantProxy>& p)
    {
        ChatParticipantProxy::Properties prop = p->GetProperties();
        cout << "\"" << prop.Name << "\" joined the conversation." << endl;
        cout << "> ";
        cout.flush();
    }

    virtual void OnRemove(const shared_ptr<ChatParticipantProxy>& p)
    {
        ChatParticipantProxy::Properties prop = p->GetProperties();
        cout << "\"" << prop.Name << "\" left the conversation." << endl;
        cout << "> ";
        cout.flush();
    }
};

class MessageListener :
    public SignalListener<ChatParticipantProxy, ChatParticipantProxy::NewMessage> {
  public:
    virtual void OnSignal(const ChatParticipantProxy::NewMessage& msg)
    {
        shared_ptr<ChatParticipantProxy> p = msg.GetEmitter();
        cout << "[" << p->GetProperties().Name << "] " << msg.message << endl;
        cout << "> ";
        cout.flush();
    }
};

int main(int argc, char** argv)
{
    shared_ptr<ObjectAdvertiser> advertiser = ObjectAdvertiser::Create();
    if (nullptr == advertiser) {
        cerr << "Bus Connection is not correctly initialized !!!" << endl;
        return EXIT_FAILURE;
    }

    ChatListener chatListener;
    shared_ptr<Observer<ChatParticipantProxy> > observer = Observer<ChatParticipantProxy>::Create(&chatListener);
    if (nullptr == observer) {
        cerr << "Observer is not correctly initialized !!!" << endl;
        return EXIT_FAILURE;
    }

    MessageListener msgListener;
    observer->AddSignalListener(msgListener);

    /* create one or more participants */
    qcc::String name;
    if (argc > 1) {
        name = argv[1];
    } else {
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "Participant%lld", (long long)getpid());
        name = qcc::String(buffer);
    }
    Participant participant(advertiser, name);
    if (ER_OK != participant.PutOnBus()) { //This effectively publishes the object
        cerr << "Failed to announce participant !" << endl;
    }
    cout << "Welcome, " << name << "." << endl;
    cout << "Type \".q\" to quit. Anything else will be published as a chat message." << endl;

    while (true) {
        cout << "> ";
        cout.flush();
        string input;
        getline(cin, input);
        if (input.length() == 0) {
            continue;
        }

        if (input == ".q") {
            break;
        }

        participant.NewMessage(input.c_str());
    }

    //participant.RemoveFromBus();

    return 0;
}
