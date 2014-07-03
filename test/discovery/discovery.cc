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

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <set>
#include <memory>
#include <sys/socket.h>
#include <cassert>

#include <datadriven/datadriven.h>
#include <datadriven/Semaphore.h>

#include "DiscoveryInterface.h"
#include "DiscoveryProxy.h"

/*
   This test aims to test discovery, in particular when a remote peer disappears.
   The remote peers are spawned in child processes and suspended/resumed with
   signals.
 */

using namespace gen::org_allseenalliance_test;
using namespace ajn;
using namespace datadriven;
using namespace std;

class DiscoveryObject :
    public datadriven::ProvidedObject,
    public DiscoveryInterface {
  public:
    DiscoveryObject(std::shared_ptr<datadriven::ObjectAdvertiser> advertiser,
                    pid_t pid) :
        datadriven::ProvidedObject(advertiser),
        DiscoveryInterface(this)
    {
        this->pid = pid;
    }
};

class TestListener :
    public Observer<DiscoveryProxy>::Listener {
  private:
    Semaphore& sem;

  public:
    shared_ptr<Observer<DiscoveryProxy> > obs;
    typedef std::set<std::shared_ptr<DiscoveryProxy> > ObjectSet;
    ObjectSet objects;

  public:

    TestListener(Semaphore& _sem) :
        sem(_sem)
    {
    }

    void setObserver(shared_ptr<Observer<DiscoveryProxy> > obs)
    {
        this->obs = obs;
    }

    virtual void OnUpdate(const std::shared_ptr<DiscoveryProxy>& p)
    {
        cout << "OnUpdate" << endl;
        if (objects.find(p) == objects.end()) {
            objects.insert(p);
            assert(ER_OK == sem.Post());
        }
    }

    virtual void OnRemove(const std::shared_ptr<DiscoveryProxy>& p)
    {
        cout << "OnRemove" << endl;
        objects.erase(p);
        assert(ER_OK == sem.Post());
    }
};

struct ChildData {
    pid_t pid;
    int fd;
};

enum ChildCmd {
    START,
    CONNECT,
    STOP,
    DISCONNECT,
    UPDATE,
    REMOVE,
    CLOSE
};

static int child(int fd)
{
    ChildCmd cc;
    BusAttachment ba("test");
    assert(ER_OK == ba.Start());
    assert(ER_OK == ba.Connect());
    std::shared_ptr<datadriven::ObjectAdvertiser> advertiser = datadriven::ObjectAdvertiser::Create(&ba);
    assert(nullptr != advertiser);
    DiscoveryObject to(advertiser, getpid());

    cout << "child start " << endl;
    bool run = true;

    while (run && read(fd, &cc, sizeof(cc))) {
        switch (cc) {
        case START:
            assert(ER_OK == ba.Start());
            break;

        case CONNECT:
            assert(ER_OK == ba.Connect());
            break;

        case STOP:
            assert(ER_OK == ba.Stop());
            break;

        case DISCONNECT:
            assert(ER_OK == ba.Disconnect());
            break;

        case UPDATE:
            assert(ER_OK == to.UpdateAll());
            break;

        case REMOVE:
            to.RemoveFromBus();
            break;

        case CLOSE:
            close(fd);
            run = false;
            break;
        }
    }

    cout << "child stop " << endl;

    return 0;
}

static void sendUpdate(vector<ChildData>& children)
{
    cout << "update " << children.size() << endl;
    for (size_t i = 0; i < children.size(); ++i) {
        ChildCmd cc = UPDATE;
        int ret = write(children[i].fd, &cc, sizeof(cc));
        if (ret < 0) {
            perror("write");
        }
        assert(ret == sizeof(cc));
    }
}

static int controller(vector<ChildData>& children)
{
    int rc = 0;
    Semaphore sem;

    TestListener testObjectListener(sem);
    shared_ptr<datadriven::Observer<DiscoveryProxy> > obs = datadriven::Observer<DiscoveryProxy>::Create(
        &testObjectListener);
    testObjectListener.setObserver(obs);
    size_t numPubObjs = children.size();

    sendUpdate(children);

    cout << "wait for objects " << numPubObjs << endl;
    for (size_t i = 0; i < numPubObjs; ++i) {
        assert(ER_OK == sem.Wait());
    }

    assert(testObjectListener.objects.size() == numPubObjs);

    cout << "suspend children " << endl;
    for (size_t i = 0; i < children.size(); ++i) {
        if (kill(children[i].pid, SIGSTOP) < 0) {
            perror("kill");
        }
    }

    cout << "wait for removal" << endl;
    for (size_t i = 0; i < numPubObjs; ++i) {
        assert(ER_OK == sem.Wait());
    }

    assert((size_t)0 == testObjectListener.objects.size());

    cout << "resume children " << endl;
    for (size_t i = 0; i < children.size(); ++i) {
        if (kill(children[i].pid, SIGCONT) < 0) {
            perror("kill");
        }
    }

    cout << "wait for objects" << endl;
    for (size_t i = 0; i < numPubObjs; ++i) {
        assert(ER_OK == sem.Wait());
    }

    assert(testObjectListener.objects.size() == numPubObjs);

    cout << "closing all children" << endl;
    for (size_t i = 0; i < children.size(); ++i) {
        int status;
        ChildCmd cc = CLOSE;
        assert(write(children[i].fd, &cc, sizeof(cc)) == sizeof(cc));

        cout << "closing fd " << children[i].fd << endl;
        if (close(children[i].fd) < 0) {
            perror("close");
        }

        cout << "wait for child " << children[i].pid << endl;
        if (waitpid(children[i].pid, &status, 0) < 0) {
            perror("waitpid");
        }
        cout << "child " << children[i].pid << " returned " << status << endl;

        if (!(WIFEXITED(status))) {
            rc = 1;
        }
    }

    return rc;
}

int main(int argc, char** argv)
{
    vector<ChildData> children;

    int numberOfChildren = 1;
    if (argc == 2) {
        numberOfChildren = atoi(argv[1]);
    }

    cout << numberOfChildren << " children" << endl;
    for (int i = 0; i < numberOfChildren; ++i) {
        int fd[2];
        if (socketpair(AF_LOCAL, SOCK_DGRAM, 0, fd) < 0) {
            perror("socketpair");
        }
        ChildData childdata;
        childdata.fd = fd[0];
        childdata.pid = fork();

        if (childdata.pid == 0) {
            close(fd[0]);
            return child(fd[1]);
        } else {
            close(fd[1]);
        }

        children.push_back(childdata);
    }

    return controller(children);
}

//namespace
