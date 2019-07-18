// IRrecvDumpV2.h

#ifndef _IRRECVDUMPV2_h
#define _IRRECVDUMPV2_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#endif

#include "IRremote_fix/IRremote.h"

//cpp파일의 함수들을 여기서 선언해야 함.
void  dumpInfo(decode_results* results);
void  dumpRaw(decode_results* results);
void  dumpCode(decode_results* results);