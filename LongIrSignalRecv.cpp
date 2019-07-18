/*﻿
Author: AnalysIR
Revision: 1.0 - Initial release
Revision: 1.1 - update generic digitalPinToInterrupt to support most arduino platform

This code is provided to overcome an issue with Arduino IR libraries
It allows you to capture raw timings for signals longer than 255 marks & spaces.
Typical use case is for long Air conditioner signals.

You can use the output to plug back into IRremote, to resend the signal.

This Software was written by AnalysIR.

Usage: Free to use, subject to conditions posted on blog below.
Please credit AnalysIR and provide a link to our website/blog, where possible.

Copyright AnalysIR 2014-2019

Please refer to the blog posting for conditions associated with use.
http://www.analysir.com/blog/2014/03/19/air-conditioners-problems-recording-long-infrared-remote-control-signals-arduino/

Connections:
IR Receiver      Arduino
V+          ->  +5v
GND          ->  GND
Signal Out   ->  Digital Pin 2
(If using a 3V3 Arduino, you should connect V+ to +3V3)

Tested on UNO only
*/

#include "LongIrSignalRecv.h"

//you may increase this value on Arduinos with greater than 2k SRAM
#define maxLen 340
#define rxPinIR 20 //pin D2 or D3 on standard arduinos. (other pins may be available on More mordern modules like MEga2560, DUE, ESP8266, ESP32)
//원래 2지만 atmega128에 맞춰서 바꿈

volatile  unsigned int irBuffer[maxLen]; //stores timings - volatile because changed by ISR
volatile unsigned int pnum = 0; //Pointer thru irBuffer - volatile because changed by ISR

void longIrSignalRecv() {
	attachInterrupt(digitalPinToInterrupt(rxPinIR), rxIR_Interrupt_Handler, CHANGE);//set up ISR for receiving IR signal

	Serial.println(F("Press the button on the remote now - once only"));
	delay(5000); // pause 5 secs
	if (pnum) { //if a signal is captured

		Serial.println();
		Serial.print(F("Raw: (")); //dump raw header format - for library
		Serial.print((pnum - 1));
		Serial.print(F(") "));
		detachInterrupt(digitalPinToInterrupt(rxPinIR));//stop interrupts & capture until finshed here
		for (int i = 1; i < pnum; i++) { //now dump the times
		  //if (!(i & 0x1)) Serial.print(F("-")); // - 지우기
			Serial.print(irBuffer[i] - irBuffer[i - 1]);
			Serial.print(F(", "));
		}
		pnum = 0;
		Serial.println();
		Serial.println();

		//attachInterrupt(digitalPinToInterrupt(rxPinIR), rxIR_Interrupt_Handler, CHANGE);//re-enable ISR for receiving IR signal
	}

}

void rxIR_Interrupt_Handler() {
	if (pnum > maxLen) return; //ignore if irBuffer is already full
	irBuffer[pnum++] = micros(); //just continually record the time-stamp of signal transitions

}