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
