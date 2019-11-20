# evomin (python)
evomin is a device and platform independent communication protocol with automatic checksum calculation and verification (CRC).
The payload is wrapped into a specific protocol to ensure a valid transmission and reception. 

## Initialization
`Evomin` is an abstract class that needs to be implemented by the user, depending on your application.
Therefor, override the class: 

````python
from evomin.evomin import Evomin


class EvominImpl(Evomin):
    """
    Concrete application side implementation
    """

    def reply_received(self, reply_payload: bytes) -> None:
        print('** Reply received, {ps} bytes **'.format(ps=len(reply_payload)))
        for b in reply_payload:
            print(b)

    def frame_received(self, frame: EvominFrame) -> None:
        print('** Frame received **')
        self.log_debug('[Evomin] EvominFrame received, Command: {}, Payload length: {}'.format(frame.command, frame.payload_length))

        print('Payload: ')
        for b in frame.get_payload():
            print(b)

        # replying happens here using reply(..)
````

The two abstract methods ``reply_received`` and ``frame_received`` are callbacks that get automatically called by the
protocol whenever there's valid data available. 

### reply_received(self, reply_payload: bytes) -> None
Gets called when an attached slave has replied. Find the replied payload in ``payload_bytes``.

> Note: This is only happening in a master-slave setup, i.e. when using SPI as the low-level transportation layer.

#### frame_received(self, frame: EvominFrame) -> None:
Gets called when a slave received a frame *until* the CRC byte from the master in a master-slave setup **OR** any participant 
received a *complete frame* at the end of the ``EOF`` byte. 

This callback gets called at different states depending on whether the setup is master-slave. If in master-slave setup, 
the callback is executed right before the reception of the CRC byte, to allow the slave to prepare the answer (i.e. reading and pushing a sensor value).

#### Communication device
Before initialization, you need to provide an implementation of ``EvominComInterface``. This class acts as the low-level wrapper 
and is independent to the underlying transportation layer, i.e. UART, SPI, I2C etc. 

It features two methods for transportation (``send_byte()`` and ``receive_byte()``) as well as a method called ``describe()`` which 
is used to provide information about the communication layer through the ``ComDescription`` tuple. 
Currently the only valid option for ``ComDescription`` is ``is_master_slave`` that can either be ``True`` or ``False``.
For master-slave communication like SPI, choose ``True``, else ``False`` (i.e. UART).

#### send_byte(self, byte: int) -> Optional[int]
Gets called whenever evomin wants to send a byte over the low-level layer. For master-slave communication, it is possible
to return a byte from the ``send_byte()`` method in order to allow the slave device to reply to a master byte. For a mocked SPI
implementation, refer to ``EvominFakeSPIInterface`` in ``com_fake.py``.

#### receive_byte(self) -> Generator[int, None, None]:
Gets called by the internal state machine. You need to implement your low-level receiving procedure here in order to 
receive bytes. For UART this could be reading the ``DR`` register of your UART device. 

> Note: The ``receive_byte()`` method is only valid for a **non** master-slave setup! In a master-slave setup, the
> received bytes must be read within the ``send_byte()`` method as the slave cannot transmit any bytes without
> a corresponding master byte. 

### Concrete initialization
As we now have implemented both abstract classes, we can now initialize a ``Evomin`` instance.

Initialize an instance of the inherited `Evomin` interface:
````python
# Initialize evomin communication interface with SPI transport
evomin = EvominImpl(com_interface=EvominFakeSPIInterface())
````

## Defining own commands
You can define your own application dependent commands within the `EvominFrameCommandType` enumeration, to be found in `frame.py`.
By default, there are two commands available:

````python
class EvominFrameCommandType(Enum):
    """
    EvominFrameCommandType defines all protocol internal command types.
    These can be extended, but sometimes it's better to use a predefined command and add a payload, than extending
    the command list for every new operation.
    RESERVED: Not used
    SEND_IDN: Used for defining a self identification frame
    """
    RESERVED = 0x00
    SEND_IDN = 0xCD
````

Feel free to add commands, i.e. `LIGHT_ON = 0xA0`.

> Note: The highest enumeration value must not exceed 255 (0xFF)! This ensures compatibility with the C-API.

## Sending
All data is sent within a package of bytes, called ``EvominFrame``. A ``EvominFrame`` consists of the following bytes:

````text
3 start bytes (SOF)
-------------------
0xAA
0xAA
0xAA

1 command type byte, i.e. SEND_IDN = 0xCD
-----------------------------------------
0xCD

1 payload length byte, i.e. 2 bytes
-----------------------------------
0x2

n payload bytes
---------------
0x00
..
0x05

1 CRC8 checksum byte
--------------------
..

2 end of frame bytes (EOF)
--------------------------
0x55
0x55

````

Note that this is a minimum frame, there's also a feature called *automatic stuff byte insertion*, which inserts additional bytes
after two consequent ``0xAA`` bytes in the payload to ensure the state machine won't mistake three ``0xAA``s as the beginning
of a new frame. Those stuffed bytes are automatically removed by the protocol on reception and are invisible to the user.

## Sending a frame
To send a frame, call ``evomin.send()`` and provide the desired command type and the payload as 
a ``bytes`` array, i.e. ``evomin.send(EvominFrameCommandType.SEND_IDN, bytes([0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xBB, 0xFF]))``.

## Processing data
There's a single method called ``poll()`` which needs to be called wherever your application's logic lives, i.e. in your ``main()`` loop
or in a thread etc.
The ``poll()`` method performs both, receiving and sending of bytes and frames.

Simple example:

````python
# Polling interface
while True:
    evomin.poll()
    sleep(1)
````

## Replying (only on a master-slave setup)
To reply directly to master's message, i.e. to reply to a ``READ_SENSOR`` message, use the ``reply()`` method.
This method call needs to be placed inside the ``frame_received()`` method inside your concrete implementation of the ``Evomin`` class.

````python
def frame_received(self, frame: EvominFrame) -> None:
    print('** Frame received **')
    
    # Reply with two bytes 0x1 and 0x2
    self.reply(bytes([0x01, 0x02]))
````

The reply bytes are then transferred to an internal buffer and sent on the next master bytes.

## Examples
### Mocked SPI interface
#### Master sends a test frame, slave replies with 4 answer bytes
Below is a complete frame with comments to show how evomin performs on a single transfer of 8 bytes from the master
and a 4 bytes length reply from the attached slave device.

> Note: This excerpt has been simulated using the `EvominFakeSPIInterface`.

```text
-> Send byte:  170                                  Master -> 0xAA SOF
Received response byte in:  0                       Slave <- dummy byte
-> Send byte:  170                                  Master -> 0xAA SOF
Received response byte in:  1                       Slave <- dummy byte
-> Send byte:  170                                  Master -> 0xAA SOF
Received response byte in:  2                       Slave <- dummy byte
-> Send byte:  205                                  Master -> Command type (CMD_IDN)
Received response byte in:  3                       Slave <- dummy byte
-> Send byte:  8                                    Master -> Payload length (8 bytes)
Received response byte in:  4                       Slave <- dummy byte
-> Send byte:  170                                  Master -> Payload byte #1 (0xAA)
Received response byte in:  5                       Slave <- dummy byte
-> Send byte:  170                                  Master -> Payload byte #2 (0xAA)
Received response byte in:  6                       Slave <- dummy byte
-> Send byte:  85                                   Master -> Automatically inserted stuff byte (not part of payload!)
Received response byte in:  7                       Slave <- dummy byte
-> Send byte:  170                                  Master -> Payload byte #3 (0xAA)
Received response byte in:  8                       ...
-> Send byte:  170                                  Master -> Payload byte #4 (0xAA)
Received response byte in:  9
-> Send byte:  85                                   Master -> Automatically inserted stuff byte (not part of payload!)
Received response byte in:  10
-> Send byte:  170                                  Master -> Payload byte #5 (0xAA)
Received response byte in:  11
-> Send byte:  170                                  Master -> Payload byte #6 (0xAA)
Received response byte in:  12
-> Send byte:  85                                   Master -> Automatically inserted stuff byte (not part of payload!)
Received response byte in:  13
-> Send byte:  187                                  Master -> Payload byte #7
Received response byte in:  14
-> Send byte:  255                                  Master -> Payload byte #8
Received response byte in:  15
-> Send byte:  159                                  Master -> CRC8 checksum (159 dec)
Received response byte in:  241                     Slave <- dummy byte
-> Send byte:  85                                   Master -> EOF #1
Received response byte in:  255                     Slave <- ACK (checksum correct)
-> Send byte:  85                                   Master -> EOF #2
Received response byte in:  4                       Slave <- Number of slave`s answer bytes (reply)
-> Send byte:  240                                  Master -> Dummy byte to keep SPI communication
Received response byte in:  222                     Slave <- Reply byte #1
-> Send byte:  240                                  ...
Received response byte in:  173                     Slave <- Reply byte #2
-> Send byte:  240
Received response byte in:  190                     Slave <- Reply byte #3
-> Send byte:  240
Received response byte in:  239                     Slave <- Reply byte #4
** Reply received, 4 bytes **
222
173
190
239
-> Send byte:  255                                  Master -> Send ACK to finalize communication
```
# evoMIN
evoMIN is a device to device communication interface written in ANSI C, based on the idea of https://github.com/min-protocol/min, but in a simpler and different implementation.

> Note: evoMIN works best with an asynchronous transport layer, i.e. UART, as it depends on various callbacks from the wires

## Procedure:


  1. Enable transport layer (low-level), i.e. UART on both, target and host
  2. Use `evoMin_Init()` to initialize a passed instance of evoMin_Interface
  3. `evoMin_RXHandler()` needs to be called from within the low-level transport routine (i.e. IRQHandler for SPI/I2C/UART)
    with the received byte (if you want to receive data, otherwise just skip this method)
  4. Whenever a new, valid frame is received, the `evoMin_Handler_FrameRecvd()` callback gets called from evoMIN
  5. To send data, one must implement a TX callback method (low-level implementation for the send transport layer, i.e. I2C, SPI)
    and assign the implemented callback to the interface by using `evoMin_SetTXHandler()`


If the device is receive-only, you can disable sending by compiling with a EVOMIN_TX_DISABLE preprocessor statement.
To send a frame:
- First create a transport frame for your command and payload using `evoMin_CreateFrame()`
  This method needs to be called with a user allocated evoMin_Frame reference.
- To eventually send the previously created frame, use the `evoMin_QueueFrame()` function and pass by copy the created frame.
  Your frame is now enqueued, but hasn't been sent yet. Sending and resending (on previous failure) of frames is automatically done
  by the `evoMin_SendResendLastFrame()` function - note: this function has to be called regularly, i.e. inside your application's main loop,
  or a timer. The `evoMin_SendResendLastFrame()` function is responsible for sending and resending frames until the frame's retry count is zero.

You can define the retry count through `EVOMIN_SEND_RETRIES_ON_FAIL` (evoMIN_specific.h), default is 3

## Example for sending a frame:
```
struct evoMin_Frame sendFrame;
// ... allocate payload
evoMin_CreateFrame(&sendFrame, command, payload, payloadLength);
evoMin_QueueFrame(&evoMinInterface, &sendFrame);
```
Main loop (or timer IRQ, etc.)
---------
```
while(true) {
// ...
 evoMin_SendResendLastFrame(&evoMinInterface);
// ...
```

## Buffer status management:
The buffer of each individual frame can be checked against overflows etc. Therefor, use the `EVOMIN_BUF_STATUS_MASK_xxx` masks

## CRC8:
The checksum CRC8 is only calculated over the payload bytes EXCLUDING the stuff bytes and also EXCLUDING the frame header bytes,
but INCLUDING the command and the payload length byte!

0	 	command byte
1	 	payload length byte
2..n 	payload bytes 
