// LongIrSignalRecv.h

#ifndef _LONGIRSIGNALRECV_h
#define _LONGIRSIGNALRECV_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#endif

void longIrSignalRecv();
void rxIR_Interrupt_Handler();