# Sample: toy home security system simulation

## Introduction
This sample is a simulation of a rudimentary home security system. Our
hypothetical security system monitors all doors in your house, and lets you
know in real time whether they're open or closed, and who passes through a
door. In addition, it allows for the remote opening an closing of doors.

The sample is split over two applications: `door_provider` is the security
system itself that publishes the state of the doors, `door_consumer` is a
simple monitoring user interface through which the user can monitor the state
of the doors, and can remotely open or close doors.

## Usage

Make sure you are running the AllJoyn routing daemon, and that the
`LD_LIBRARY_PATH` environment variable includes the location of the AllJoyn
libraries. See the top-level `README.md` file in this distribution for details.

### Start one or more `door_provider`s

    $ door_provider <location1> [<location2> [... [<locationN>] ...]]

e.g.:

    $ door_provider home office

`door_provider` will simulate as many doors as you supply locations on the
command line. You need to supply at least one location.

You will be dropped into a primitive command line user interface where you can
issue simulation comands. To keep the interface simple, the application
continuously cycles through all doors it maintains, and you cannot choose which
door will be the subject of your next simulation command.

The following commands are supported:

    q         quit
    f         flip (toggle) the open state of the door
    p <who>   signal that <who> passed through the door
    r         remove or reattach the door to the bus
    n         move to next door in the list
    h         show this help message

### Start a `door_consumer`

    $ door_consumer

The application will monitor the state of all doors that are published on the
bus, and will print notifications whenever doors appear, disappear, or change
state. In addition, you can perform the following actions:

    q             quit
    l             list all discovered doors
    o <location>  open door at <location>
    c <location>  close door at <location>
    h             display this help message

## Sample output

At consumer:

    > l
    Door ObjectId(bus::Yi1KSRo7.1255,obj:/Door/1) location: home open: 0
    Door ObjectId(bus::Yi1KSRo7.1255,obj:/Door/2) location: office open: 0
    > o home
    [listener] Update for door ObjectId(bus::Yi1KSRo7.1255,obj:/Door/1): location = home open = 1.
    > Opening of door succeeded
    > l
    Door ObjectId(bus::Yi1KSRo7.1255,obj:/Door/1) location: home open: 1
    Door ObjectId(bus::Yi1KSRo7.1255,obj:/Door/2) location: office open: 0
    > c home
    [listener] Update for door ObjectId(bus::Yi1KSRo7.1255,obj:/Door/1): location = home open = 0.
    > Closing of door succeeded


At provider:

    [next up is home] >Door @ home was requested to open.
    	... and it was closed, so we can comply.
    [next up is home] >Door @ home was requested to close.
    	... and it was open, so we can comply.
    [next up is home] >p john
    [next up is office] >r

At consumer:

    > [listener] john passed through a door ObjectId(bus::Yi1KSRo7.1255,obj:/Door/1): location = home
    > [listener] Door ObjectId(bus::Yi1KSRo7.1255,obj:/Door/2) at location office does no longer exist.

## Discussion

### Data Model

The data model is defined in `door.xml`. A door is represented as a single
interface. It has observable properties (its `location`, and whether it is
`open`). A door can emit a signal when someone passes through
(`PersonPassedThrough`). In addition, a door offers methods to remotely `Open`
or `Close` it.

### Implementation

On the provider side, all business logic is contained in the `Door` class,
which provides a concrete implementation of the functionality specified in the
`org.allseenalliance.sample.Door` interface defined in the data model. For each
location specified on the command line, a `Door` object is created and
published on the bus.

On the consumer side, an `Observer<gen::org_allseenalliance_sample::DoorProxy>`
is created. The listeners attached to this Observer inform us when doors
appear, disappear or change state (`MyDoorListener`) or when someone passes
through a door (`MyDoorSignalListener`). We iterate over the discovered objects
in the Observer to list all doors (`list_doors`) or to locate a specific door
(`get_door_at_location`). To open or close a door, we invoke the `Open` or
`Close` methods on the corresponding `DoorProxy` object we retrieve from the
Observer.
