The Data-driven API for AllJoyn
===============================

The Data-driven API (DDAPI) for AllJoyn is an alternative API for the AllJoyn
framework (a layer on top of AllJoyn Core, if you will) that is specifically
tailored to use cases for the Internet of Things.

The DDAPI distinguishes itself from the standard AllJoyn API in the
following ways:

* The data-centric, publish-subscribe paradigm as a first-class citizen. We
  believe this paradigm is more suitable for the Internet of Things than the
  closely coupled, service-oriented, RPC-based approach advocated by standard
  AllJoyn. However, the DDAPI also fully supports the service-oriented paradigm
  if you need it.
* A unified approach to discovery and session setup. The DDAPI enforces a single
  mechanism for discovery and session setup. This increases interoperability
  between devices and applications made by different vendors. 
  The main driver behind the unified discovery and session setup is the desire
  to avoid situations where you need two different light control applications
  for your smart light bulbs just because vendor A decided to use a different
  session port or discovery string than vendor B.
* A radically simplified API compared to standard AllJoyn. The unified
  discovery and session setup system requires virtually no API (there is only
  one way to do things, and the DDAPI library does most of the hard
  work for you). On top of this, the DDAPI leverages the AllJoyn Code
  Generator to turn interface specifications (in XML format) into code that
  deals with type registration and message marshaling and unmarshaling. The end
  result is an API that lets the application developer deal with the business
  logic of the application instead of the communication logic.

Current Status
--------------

This is the first release of the DDAPI for AllJoyn. The main aim of this release
is to provide you with a taste of the things to come. **The current version of the DDAPI
is not yet ready for production use.** In this release, the DDAPI is only available
for Linux/C++ on x86 and x86_64 systems.

We are working hard to incorporate the following features in the R14.10 AllJoyn release:

* A production-ready core implementation.
* More language and platform bindings (Windows, iOS, Android, a Data-driven
  Thin Library, etc.).
* A unified approach to security, with the same rationale as the unified
  discovery and session setup mechanism.

More information
----------------

* The TUTORIAL.md document included in the source distribution provides a gentle
  tutorial for the DDAPI.
* Using doxygen, you can build a reference manual in HTML format directly from
  the source code.
* [AllSeen DDAPI Workgroup Proposal](https://wiki.allseenalliance.org/tsc/technical_steering_committee/proposals/simplifiedapi)
* [DDAPI Mailing list](https://lists.allseenalliance.org/mailman/listinfo/allseen-datadriven)


How to run the samples
======================
The samples have been compiled with the router builtin. Therefore, there is no
need to start the AllJoyn routing daemon.

DDAPI applications depend on `libajdd.so`, which in turn depends on
`liballjoyn.so`, `liballjoyn_about.so` and `liballjoyn_services_common.so`.
You'll need to include the location where you installed these libraries in the
`LD_LIBRARY_PATH` environment variable. For example:

    $ LD_LIBRARY_PATH=${ALLSEEN_INSTALL_PATH}/lib/:${LD_LIBRARY_PATH}
    $ export LD_LIBRARY_PATH

There are currently two sample applications to illustrate the use of the DDAPI:

* a simple chat application (in `samples/ddchat`)
* a toy home security system simulation (in `samples/door`).

Building your own applications with the DDAPI
=============================================
Requirements
------------
Your compiler should at least support C++0x.

Using the Data-driven API code generator
----------------------------------------
In order to use the DDAPI effectively, you need to make use of the AllJoyn code
generator (ajcodegen.py). The code generator will create type support classes
for each of the AllJoyn interfaces you defined in the previous step.

For detailed instructions on how to use the code generator, we refer you to the
code generator's own documentation. The following should get you up and
running though:

    $ ajcodegen.py -t ddcpp your_interface_definition.xml

For each interface specified in the XML file, 3 header files and 3 source files
will be generated:

* One of each for the consumer side (`<intfname>Proxy.{cc|h}`).
* One of each for the provider side (`<intfname>Interface.{cc|h}`).
* One of each that is common to both consumer and provider and that
  describes the interface type (`<intfname>TypeDescription.{cc|h}`).

Building and linking
--------------------
To build your own Linux application with the DDAPI, you must pass two AllJoyn-related defines:

* `QCC_OS_LINUX`
* `QCC_OS_GROUP_POSIX`

All files pertaining to the datadriven_api are found in the datadriven directory.
Regarding header dependencies, you should include the individual directories
found under cpp/inc in your compilation directive.

Besides these framework include directories, you still need to make sure the
generated header files (based on your data model) are found.

After the compilation step of your application (do not forget to also compile
the generated code), you should link your application to 

* libajdd.so 
* liballjoyn_about.so 
* liballjoyn.so
* liballjoyn_services_common.so

