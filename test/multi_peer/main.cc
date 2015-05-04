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

#include <iostream>
#include <sstream>
#include <vector>

#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <datadriven/datadriven.h>
#include <alljoyn/Init.h>

#include "MultiPeerInterface.h"
#include "consumer.h"
#include "provider.h"

using namespace std;
using namespace gen::org_allseenalliance_test;

/**
 * Multi-peer tests.
 */
namespace test_system_multipeer { };

using namespace test_system_multipeer;

static pid_t* _pid;

static int wait_for_completion(int num_prov, int num_cons)
{
    bool done = false;
    int status = 0;
    pid_t pid;
    int i;
    int proc_cnt = num_prov + num_cons;

    while (!done && (0 == status)) {
        pid = waitpid(-1, &status, 0);
        if (-1 == pid) {
            status = errno;
        } else {
            // mark process as terminated
            for (i = 0; i < proc_cnt; i++) {
                if (pid == _pid[i]) {
                    cout << "main: " << (i < num_prov ? "provider" : "consumer")
                         << " process " << pid << " exited with status " << status << endl;
                    _pid[i] = -1;
                    break;
                }
            }
            // stop on error
            if (0 != status) {
                cout << "main: error occurred, terminating" << endl;
                break;
            }
            // stop if all consumers exited
            for (i = num_prov; i < proc_cnt; i++) {
                if (-1 != _pid[i]) {
                    break;
                }
            }
            if (proc_cnt == i) {
                cout << "main: all consumers exited, terminating" << endl;
                done = true;
            }
        }
    }
    return status;
}

static Provider* _prov = NULL;

static void sig_handler(int sig)
{
    if (NULL != _prov) {
        cout << "Signal " << sig << " received" << endl;
        _prov->Stop();
    }
}

static void init()
{
    struct sigaction act;

    memset(&act, '\0', sizeof(act));
    act.sa_handler = &sig_handler;
    assert(0 == sigaction(SIGTERM, &act, NULL));
}

static void cleanup(int num_prov, int num_cons)
{
    int proc_cnt = num_prov + num_cons;

    // kill remaining processes
    for (int i = 0; i < proc_cnt; i++) {
        if (-1 != _pid[i]) {
            kill(_pid[i], SIGTERM);
        }
    }
    sleep(1);
    free(_pid);
}

static int fork_children(int num_prov, int num_cons,
                         int argc, char** argv)
{
    int rc = EXIT_SUCCESS;
    char* args[argc + 2];

    _pid = (pid_t*)calloc(num_prov + num_cons, sizeof(pid_t));
    // prepare exec args
    args[0] = argv[0];
    for (int i = 1; i < argc; i++) {
        args[i + 1] = argv[i];
    }
    args[argc + 1] = NULL;
    // start providers and consumers
    for (int i = 0; i < num_prov + num_cons; i++) {
        bool isprov = (i < num_prov ? true : false);

        _pid[i] = fork();
        assert(-1 != _pid[i]);
        if (0 == _pid[i]) {
            stringstream id;

            free(_pid);
            id << (isprov ? 'p' : 'c') << i;
            /**
             * Exec new process (actually same process) to avoid possible
             * issues with globals from ThreadListInitializer.
             */
            args[1] = strdup(id.str().c_str());
            execv(argv[0], args);
            return EXIT_FAILURE;
        } else {
            cout << "main: started " << (isprov ? "provider" : "consumer")
                 << " with id " << i << ", pid = " << _pid[i] << endl;
        }
    }
    // wait for child process termination
    cout << "main: waiting for completion" << endl;
    rc = wait_for_completion(num_prov, num_cons);
    // clean up
    cout << "main: cleaning up" << endl;
    cleanup(num_prov, num_cons);
    cout << "main: exit with " << rc << endl;
    return rc;
}

static void usage(const char* app)
{
    cout << "Usage: " << app << " [p<n>|c<n>] <num-cons> <num-prov> <num-obj> [<loops>]" << endl;
    exit(1);
}

int main(int argc, char** argv)
{
    int rc = EXIT_SUCCESS;
    bool fork = true;
    bool provider;
    int idx = 1;
    int num_cons = 2;
    int num_prov = 2;

    if (argc <= 3) {
        usage(argv[0]);
    }
    if (argc >= 5) {
        if (('p' == *argv[1]) || ('c' == *argv[1])) {
            fork = false;
            provider = ('p' == *argv[1] ? true : false);
            idx = 2;
        }
    }
    init();

    num_cons = atoi(argv[idx + 0]);
    num_prov = atoi(argv[idx + 1]);
    if (fork) {
        rc = fork_children(num_prov, num_cons, argc, argv);
    } else {
        if (AllJoynInit() != ER_OK) {
            return EXIT_FAILURE;
        }
#ifdef ROUTER
        if (AllJoynRouterInit() != ER_OK) {
            AllJoynShutdown();
            return EXIT_FAILURE;
        }
#endif

        int id = atoi(&argv[1][1]);
        int num_obj = atoi(argv[idx + 2]);

        if (provider) {
            _prov = new Provider(id, num_obj);
            _prov->Run();
            delete _prov;
        } else {
            Consumer* cons = new Consumer(id, num_prov * num_obj);
            int loops = 1;

            if (argc - 1 == idx + 3) {
                loops = atoi(argv[idx + 3]);
            }
            cons->Test(loops);

            delete cons;
        }

#ifdef ROUTER
        AllJoynRouterShutdown();
#endif
        AllJoynShutdown();
    }
    return rc;
}
