# Sample: simple chat application

## Introduction

This sample implements a multi-participant chat application. It is
deliberately kept extremely simple: there is no support for private messages or
multiple chat rooms.

## Usage

Make sure you are running the AllJoyn routing daemon, and that the
`LD_LIBRARY_PATH` environment variable includes the location of the AllJoyn
libraries. See the top-level `README.md` file in this distribution for details.

To join the chat conversation, simply run

    $ ./ddchat [<name>]

If you do not provide a name, a random name will be generated for you.

## Sample output

    Welcome, Participant23458.
    Type ".q" to quit. Anything else will be published as a chat message.
    > "Participant23458" joined the conversation.
    > hi
    > [Participant23458] hi

## Discussion


### Data Model

The data model, defined in `ddchat.xml`, is straightforward. There is a single
interface `org.allseenalliance.sample.ChatParticipant` that represents a
participant in the conversation. A participant has a single observable
property, the name. In addition, participants can say something (obviously),
which is represented as the `Message` signal.

### Implementation

The chat application acts as both provider (we're publishing a ChatParticipant
object and we emit Message signals) and consumer (we're interested in other
ChatParticipants and what they have to say).

The provider part is very straightforward: we define a concrete class
`Participant` that derives from `datadriven::ProvidedObject` and the generated
`gen::org_allseenalliance_sample::ChatParticipantInterface` classes. In the
`main` function, we create a `Participant` object, and publish it on the bus.
Whenever the user enters a chat message, we publish it with the
`Participant::Message()` call.

The consumer part is not too complicated either: in `main`, we create an
`Observer<gen::org_allseenalliance_sample::ChatParticipantProxy>`. We attach
two listeners to this Observer:

* the `ChatListener` (a subclass of `Observer::Listener`) receives
  notifications whenever chat participants join or leave the conversation.
* the `MessageListener` (a subclass of `SignalListener`) receives notifications
  whenever a chat participant emits the `Message` signal.

Note that the provider and consumer aspects of the chat application are
completely independent: the consumer will also receive notifications when the
provider in the same chat application joins the conversation, and when the
provider in the same chat application emits a `Message` signal.
