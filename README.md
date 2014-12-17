# The Data-driven API for AllJoyn

## About the Data-Driven API

The Data-driven API (DDAPI) for AllJoyn is an alternative API for the AllJoyn
framework. It is built on top of the standard AllJoyn API and  is specifically
tailored to use cases for the Internet of Things. Instead of the standard
framework's service-oriented paradigm, it uses the publish/subcribe paradigm.

Some conceptual knowledge of the standard AllJoyn framework is required to use the DDAPI
for application development.

## The Publish/Subscribe Pattern

Publish–subscribe (pub/sub) is a messaging pattern where creators of objects, called providers,
do not program the objects to be observed directly by specific observers, called subscribers.
Instead, published objects are characterized into classes, without knowledge of what,
if any, observers there may be.

Similarly, observers express interest in one or more objects, and only observe
those that are of interest, without knowledge of what, if any, providers there are.

This pattern offers two major advantages:

* Scalability: pub/sub offers more scalability.
 
* Loosely coupled: providers are loosely coupled to observers, and need not even
  know of their existence. With the object interface(also called a "topic"
  in more general pub/sub terms) being the focus, providers and observers
  are allowed to remain ignorant of system topology.
  Each can continue to operate normally regardless of the other.


## The Differences Between Standard AllJoyn and the DDAPI

The DDAPI distinguishes itself from the standard AllJoyn API in the
following ways:

* The DDAPI uses a pub/sub pattern instead of the service-oriented, RPC-based approach
  of standard AllJoyn. However, it fully supports this service-oriented paradigm, and even
  allows hybrid scenarios where DDAPI-based applications communicate with standard AllJoyn
  applications.

* The DDAPI uses a  unified approach to discovery and session setup. It enforces a single
  mechanism for discovery and session setup. This increases interoperability
  between devices and applications made by different vendors.
  The main driver behind the unified discovery and session setup is the desire
  to avoid interoperablity conflicts. Imagine a situation where you need two
  different light control applications.
  for your smart light bulbs just because vendor A decided to use a different 
  session port or discovery string than vendor B.

* Compared to standard AllJoyn, the DDAPI is radically simplified. The unified
  discovery and session setup system requires virtually no API. There is only
  one way to do things, and the DDAPI library does most of the hard
  work for you. On top of this, the DDAPI leverages the AllJoyn Code
  Generator to turn interface specifications (in XML format) into code that
  deals with type registration and message marshaling and unmarshaling.
  
  This means that the DDAPI lets the application developer deal with the business
  logic of the application instead of the communication logic.

## Current Status

The DDAPI is stable and can already be used to develop applications. Since development
is continuously ongoing, both the code and the documentation are subject to change.

## More Information

The documentation for the DDAPI has not yet been integrated into the AllSeen Alliance
documentation system, since at the time of writing of this document, that system
is still undergoing major changes. However, we provided a number of documents that will
get you started in application development using the DDAPI.

* The TUTORIAL.md document included in the source distribution provides an easy
  tutorial for the DDAPI.

* Using doxygen, you can build a reference manual in HTML format directly from
  the source code.

* [AllSeen DDAPI Workgroup Proposal](https://wiki.allseenalliance.org/tsc/technical_steering_committee/proposals/simplifiedapi)

* [DDAPI Mailing list](https://lists.allseenalliance.org/mailman/listinfo/allseen-datadriven)


## How to Run the Samples

The samples have been compiled with a built-in router. Therefore, there is no
need to start the AllJoyn routing daemon.

DDAPI applications depend on `libajdd.so`, which in turn depends on
`liballjoyn.so` and `liballjoyn_about.so`. You'll need to include the location
where you installed these libraries in the `LD_LIBRARY_PATH` environment
variable. For example:

    $ LD_LIBRARY_PATH=${ALLSEEN_INSTALL_PATH}/lib/:${LD_LIBRARY_PATH}
    $ export LD_LIBRARY_PATH

There are currently two sample applications to illustrate the use of the DDAPI:

* a simple chat application (in `samples/ddchat`)
* a toy home security system simulation (in `samples/door`).

## Building your own applications with the DDAPI

### Requirements

Your compiler should at least support C++0x.

### Using the Data-driven API code generator

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

All files pertaining to the DDAPI are found in the datadriven directory.
Regarding header dependencies, you should include the individual directories
found under cpp/inc in your compilation directive.

Besides these framework include directories, you still need to make sure the
generated header files (based on your data model) are found.

After the compilation step of your application (make sure to also compile
the generated code), you should link your application to

* libajdd.so
* liballjoyn_about.so
* liballjoyn.so
