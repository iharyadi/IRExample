/*
 * IRremote: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

#include <slip.h>
#include <SoftwareSerial.h>

#include <stdint.h>

#define MAX_RAW_DATA 32
SoftwareSerial mySerial(4, 5);
SoftwareSlip slip(mySerial);

int RECV_PIN = 2;

IRrecv irrecv(RECV_PIN);

IRsend irsend;

decode_results results;

struct __attribute__((packed)) irReceiverDataBase
{
  irReceiverDataBase(uint8_t _command,uint8_t _page,decode_type_t _decode_type):
  command{_command}, page{_page}, decode_type{_decode_type}
  {
  };
  uint8_t command;
  uint8_t page;
  decode_type_t decode_type;
};

struct __attribute__((packed)) irReceiverData:public irReceiverDataBase
{
  irReceiverData(uint8_t _command,
    uint8_t _page,
    decode_type_t _decode_type,
    uint16_t _address,
    uint32_t _value,
    int16_t  _bits):
    irReceiverDataBase {_command, _page, _decode_type},
    address { _address },
    value{_value},
    bits{_bits}
    {
    };
	uint16_t address;     
	uint32_t value;       
	int16_t  bits;
};

struct __attribute__((packed)) ASCIIReceiverData:public irReceiverDataBase
{
  ASCIIReceiverData(uint8_t _command,
    uint8_t _page,
    decode_type_t _decode_type,
    char* str):
    irReceiverDataBase {_command, _page, _decode_type}
    {
      strncpy(asciiData,str,sizeof(asciiData));
    };
	char asciiData[10];     
};

struct __attribute__((packed)) irReceiverUnknownData:public irReceiverDataBase
{
  irReceiverUnknownData(uint8_t _command,
    uint8_t _page,
    decode_type_t _decode_type,
    uint8_t _buflen):
    irReceiverDataBase {_command, _page, _decode_type},
    buflen { _buflen }
    {
    };
  uint8_t buflen;
	uint8_t data[MAX_RAW_DATA];     
};

struct __attribute__((packed)) irTransmitDataBase
{
  uint8_t page;
  decode_type_t decode_type;   
};

struct __attribute__((packed)) irTransmitData:public irTransmitDataBase
{
  uint16_t address;      
  uint32_t value;        
  int16_t  bits; 
  uint8_t  repeat;  
};

struct __attribute__((packed)) irTransmitUnknownData:public irTransmitDataBase
{
  uint8_t repeat;
  uint8_t buflen;      
  uint8_t data[];   
};

void slipReadCallback(uint8_t * buff,uint8_t len)
{ 

  Serial.println("Packet Received");

  irTransmitDataBase* pIrData = (irTransmitDataBase*) buff;

  if(pIrData->page != 0)
  {
    return;
  }

  switch(pIrData->decode_type)
  {
  case UNKNOWN:
  {
    irTransmitUnknownData* unk = (irTransmitUnknownData*) buff;
    unsigned int signalBuffer[MAX_RAW_DATA];
    for(int i = 0; i < unk->buflen; i++)
    {
      signalBuffer[i] = (unsigned int)unk->data[i]*USECPERTICK;
    }

    for(int i = 0; i < unk->repeat; i++)
    {
      irsend.sendRaw(signalBuffer, unk->buflen, 38);
    }

    break;
  }
  case UNUSED:
    break; 
  case SAMSUNG:
  {
    irTransmitData* data = (irTransmitData*) buff;
    irsend.sendSAMSUNG(data->value,data->bits);
    break;
  }
  }
 
  irrecv.enableIRIn();
  
}

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  mySerial.begin(9600);
  slip.setCallback(slipReadCallback);

  pinMode(7,INPUT_PULLUP);
}

void SendToHubIRReceived(decode_results& irpacket)
{
  switch (irpacket.decode_type)
  {
  case UNKNOWN:
  {
    irReceiverUnknownData buffer {0,0,UNKNOWN,(uint8_t)(irpacket.rawlen-1)};

    if(irpacket.rawlen-1 <= sizeof(buffer.data))
    {
      for(int i = 1; i < irpacket.rawlen; i++)
      {
        buffer.data[i-1] = (uint8_t) irpacket.rawbuf[i];
      }

      slip.sendpacket((uint8_t*)&buffer, sizeof(buffer));
    }
    break;
  } 
  case UNUSED:
    break;
  default:
    irReceiverData buffer = {0,
      0,
      irpacket.decode_type,
      irpacket.address,
      irpacket.value,
      irpacket.bits};
      
    slip.sendpacket((uint8_t*)&buffer, sizeof(buffer));
  }
}

void ExampleToSendAscii()
{


  if(digitalRead(7) == LOW)
  {
    char* Test[] = {"helo","world"};
    
    static uint8_t i = 0;

    ASCIIReceiverData data(0u,1u,UNUSED,Test[i]);

    slip.sendpacket((uint8_t*)&data,sizeof(data));
    
    i = ++i%2;
  }
}
void loop() {
  slip.proc();
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value

    SendToHubIRReceived(results);
  }

  ExampleToSendAscii();

  delay(100);
}
