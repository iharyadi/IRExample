# IRExample
 This is an example to use Environment Sensor USART expansion using arduino.  This sketch show how to deliver IR packets from remote control received using IRRemote Library to a home automation hub through Environment sensor.
 
 In this example,  using Environment sensor that act as Arduino shield through UART port.   The UART must be set as 9600 baud rate.  In this example,  the UART port us pin 4 and 5 for (RX and TX) using ardunio SoftwareSerial library.
 
 Environment sensor will take a packet from the UART and deliver it back and forth to a hub using Zigbee protocol.  In a way,  it is a tunneling mechanism to tunnel SLIP packets back and forth between Arduino and hub.
 
 This packet is a binary form of maximum 64 bytes of data.  Please note,  SLIP will use 2 bytes for a byte that is one of its escape charcter.   Effectively this will reduce the size of the size of a packet that can be sent.   At absolute case where every bytes are escape characters,  the maximum bytes can be sent is 32.  for SLIP proctocol this wiki link is helpful https://en.wikipedia.org/wiki/Serial_Line_Internet_Protocol  

In this example,  I show a rudimentary frame strategy which may not be make sense in this case.  I want to explain it a bit.  I use a concept of command and page byte to start a frame.  This is useful if you have multiples sensor or things that you control from a single Arduino.  You can send the data from each sensor using different page id and command id as way to multiplex them.   In this example,  we only have one thing which is IR.  Therefore,  I set them to 0s.   This may look strange.  That is why I think I need to explain it if you see the code.

There is no restriction on how you form a packet.   The restriction on 64 byte is reasonable in small MCU ecosystem.  Using the slip protocol,  we can open a posibility to support larger packet by using fragmentation concept. 
