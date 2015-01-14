# The Data-Driven API (DDAPI) for AllJoyn

## About the DDAPI

The Data-driven API (DDAPI) for AllJoyn is an alternative API for the AllJoyn
framework. It is built on top of the standard AllJoyn API and  is specifically
tailored to use cases for the Internet of Things. Instead of the standard
framework's service-oriented paradigm, it uses the publish/subcribe paradigm.

Some conceptual knowledge of the standard AllJoyn framework is required to use the DDAPI.
for application development.

## Get to Know the DDAPI

To get to know the DDAPI, here are some sources of information:

* Introduction to the DDAPI in the [learn section of the AllSeen documentation website][ddapi_intro]

* Detailed API guides and tutorial in the [develop section of the AllSeen documentation website][ddapi_guide]

* Using doxygen, you can build a reference manual in HTML format directly from
  the source code.

* [AllSeen DDAPI Workgroup Proposal](https://wiki.allseenalliance.org/tsc/technical_steering_committee/proposals/simplifiedapi)

* [DDAPI Mailing list](https://lists.allseenalliance.org/mailman/listinfo/allseen-datadriven)

## General Installation Process

The general installation process for the DDAPI is as follows:

1. [Download][ajdl] and [Install][ajinst] AllJoyn Core.
2. [Install][cginst] the AllJoyn Code Generator.
3. Unpack the DDAPI tarball in the directory where you installed the AllJoyn Core files.
4. Go to the directory `/data/datadriven_api/` and run the scons script.

Read the rest of this document for more information.

## Patch When Running On Top of AllJoyn Core 14.12 (ASACORE-1238)

If you are building the DDAPI on top of AllJoyn Core 14.12, you need to run a patch. This issue is solved from AllJoyn Core release 14.12a onwards.

You will get this error:
.../build/linux/x86_64/debug/dist/cpp/inc/alljoyn/AutoPinger.h:31:23: fatal error: qcc/Timer.h: No such file or directory
compilation terminated.

The patch is located in the patches directory. Run it to patch the necessary files:

% patch -Np1 -i ../alljoyn-ddapi-0.0.1-src/data/datadriven_api/patches/alljoyn_core_14.12_autopinger.patch
patching file alljoyn_core/inc/alljoyn/AutoPinger.h
patching file alljoyn_core/src/AutoPinger.cc
patching file alljoyn_core/src/AutoPingerInternal.cc
patching file alljoyn_core/src/AutoPingerInternal.h
patching file alljoyn_core/unit_test/AutoPingerTest.cc

## General Installation Process

The general installation process for the DDAPI is as follows:

1. [Download][ajdl] and [Install][ajinst] AllJoyn Core.
   If AllJoyn core is not installed globally on your system, set ALLJOYN_DISTDIR
   to the path where alljoyn is compiled. For Linux 64bit this is ${ALLJOYN_SRC_DIR}/build/linux/x86_64/debug/dist/
2. [Install][cginst] the AllJoyn Code Generator.
   If the Code Generator is not installed globally on your system, make sure ajcodegen.py is in your PATH and
   PYTHONPATH point to dist/lib/python in the codegen compilation.
3. Unpack the DDAPI tarball in the directory where you installed the AllJoyn Core files.
4. Go to the directory `/data/datadriven_api/` and run the scons script.

Read the rest of this document for more information. 
 
## How to Run the Samples

The samples have been compiled with a built-in router. Therefore, there is no
need to start the AllJoyn routing daemon.

DDAPI applications depend on `liballjoyn_ddapi.so`, which in turn depends on
`liballjoyn.so` and `liballjoyn_about.so`. You'll need to include the location
where you installed these libraries in the `LD_LIBRARY_PATH` environment
variable. For example:

    $ LD_LIBRARY_PATH=${ALLSEEN_INSTALL_PATH}/lib/:${LD_LIBRARY_PATH}
    $ export LD_LIBRARY_PATH

There are currently two sample applications to illustrate the use of the DDAPI:

* a simple chat application (in `samples/ddchat`)
* a toy home security system simulation (in `samples/door`).

## Building your own applications with the DDAPI

### Prerequisites

Before you can use the DDAPI, the following needs to be fulfilled:

* Your compiler should at least support C++0x.
* You must make sure you [downloaded][ajdl] and [installed][ajinst] AllJoyn Core Framework.
* You must make sure you [installed][cginst] the AllJoyn Code Generator.

### Using the Data-driven API code generator

The AllJoyn Code Generator is a command line driven tool which reads AllJoyn introspection XML files
and outputs both client and server code ready for compilation and running. The Code Generator creates
the type support classes for all interfaces you defined using the AllJoyn Introspection XML. 

The code generator is located in the devtools folder. Use the SCONS script to build it.

For detailed instructions on how to use the code generator, we refer you to the
[code generator's documentation][codegen].

As a quick instruction, use the following command to convert an Introspection XML file:

`$ ajcodegen.py -t ddcpp your_interface_definition.xml`

For each interface specified in the XML file, 3 header files and 3 source files
are generated:

* One of each for the consumer side (`<intfname>Proxy.{cc|h}`).
* One of each for the provider side (`<intfname>Interface.{cc|h}`).
* One of each that is common to both consumer and provider and that
  describes the interface type (`<intfname>TypeDescription.{cc|h}`).

### Building and linking

To install the DDAPI, unpack the DDAPI tarball in your AllJoyn installation folder. The
DDAPI files are located in the `data/datadriven/` folder. Use the SCONS script to build it.

To buiild your own Linux application with the DDAPI, you must pass two AllJoyn-related defines:

* `QCC_OS_LINUX`
* `QCC_OS_GROUP_POSIX`

All files pertaining to the DDAPI are found in the datadriven directory.
Regarding header dependencies, you should include the individual directories
found under cpp/inc in your compilation directive.

Besides these framework include directories, you still need to make sure the
generated header files (based on your data model) are found.

After the compilation step of your application (make sure to also compile
the generated code), you should link your application to:

* liballjoyn_ddapi.so
* liballjoyn_about.so
* liballjoyn.so

[ddapi_intro]: https://allseenalliance.org/developers/learn/ddapi
[ddapi-guide]: http://allseenalliance.org/developers/develop/api-guide/ddapi
[codegen]: https://wiki.allseenalliance.org/devtools/code_generator

[ajdl]: https://allseenalliance.org/developers/download
[ajinst]: https://wiki.allseenalliance.org/develop/building_and_running

[cgdl]: 
[cginst]: https://wiki.allseenalliance.org/devtools/code_generator
