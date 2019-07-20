//Arduino128_VS.ino
//깃허브 

#include <Arduino.h>	//PIO에서는 Arduino.h 인클루드 해야함.
#include <MsTimer2.h> // timer2 를 사용함. 변경
#include <avr/wdt.h> //watchdog timeout
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "src/IRremote_fix/IRremote.h" //irsend에 timer3을 사용하기 위해 따로 만듦
#include "LongIrSignalRecv.h"
#include "IRrecvDumpV2.h"

IRrecv irrecv(20); //IRrecv 객체속성
IRsend irsend;	//PE3 사용

LiquidCrystal_I2C led(0x27, 16, 2); //I2C lcd 객체

volatile bool irrecv_mode = 0; //ir리시브 모드 바꾸기

const long BaudRate = 9600;
//115200;
const byte ResetPin = 17; //
const byte led_D1 = 41;
const byte led_D2 = 42;
const byte led_D3 = 43;
const byte led_D4 = 44;

//Atmega128 아두이노 핀번호
const byte PA[8] = { 44, 43, 42, 41, 40, 39, 38, 37 };
const byte PB[8] = { 8, 9, 10, 11, 12, 13, 14, 15 };
const byte PC[8] = { 28, 29, 30, 31, 32, 33, 34, 35 };
const byte PD[8] = { 18, 19, 20, 21, 22, 23, 24, 25 };
const byte PE[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
const byte PF[8] = { 45, 46, 47, 48, 49, 50, 51, 52 };
const byte PG[5] = { 26, 27, 36, 16, 17, };

volatile int timer1_counter; //타이머1 카운터
volatile bool led_D1_state, led_D2_state, led_D3_state, led_D4_state = HIGH; //LED 상태 나타내는 변수

//7sengment 16진수 숫자 배열
const byte seg7[16] = { 0x03, 0x9F, 0x25, 0x0D, 0x99, 0x49, 0x41, 0x1B, 0x01, 0x09, 0x11, 0xC1, 0x63, 0x85, 0x61, 0x71 };

void(*resetFunc) (void) = 0; //리셋func

void setup()
{
	//리셋핀 설정 //리셋핀 설정때문에 avr 스튜디오 4.19 연결 안됨
	digitalWrite(ResetPin, HIGH); //무한리셋 할까봐 먼저 HIGH설정
	delay_ms(200); // 셋업에 딜레이 함수쓰면 avrstudio 고장남.
	pinMode(ResetPin, OUTPUT); //리셋

	kit_init(); //키트 설정
	timer_init(); //타이머 설정
	switch_init();	//스위치 설정
	lcd_init();	//lcd 설정

	irrecv.enableIRIn(); // ir리시버 시작
	irrecv_mode = 0;

	Serial.begin(BaudRate); //시리얼 통신 시작
	Serial.println("atmega128 아두이노 실행");
}

int testNum = 0;
unsigned long previousMillis = 0; //이전시간
const long delayTime = 30000; //5초 대기시간
boolean ledState = false; //LED 현재상태
void loop()
{
	unsigned long currentMillis = millis(); //현재시간값 가져옴

	if (currentMillis - previousMillis >= delayTime)  //1000 = 1초 시간이 흘렀는지 체크
	{
		previousMillis = currentMillis; //1초가 지나 참임으로 1초 지난 현재시간을 이전시간에 저장
		ledState = !ledState;
		digitalWrite(led_D2, ledState);
		Serial.print("Test ");
		Serial.println(testNum++);

		//Serial.println("문장:" + str);
	}

	//  ir리시버
	if (irrecv_mode == 0)
	{
		decode_results  results;        // Somewhere to store the results

		if (irrecv.decode(&results)) {  // Grab an IR code
		//Serial.println(results.value, HEX); //16진수로 출력
		//Serial.println(results.bits, DEC);  //송신 데이터의 길이 10진수로 출력
		//dumpInfo(&results);           // Output the results
		//dumpRaw(&results);            // Output the results in RAW format
			dumpCode(&results);           // Output the results as source code
			Serial.println("");           // Blank line between entries
			irrecv.resume();              // Prepare for the next value
		}
	}
	else
	{
		longIrSignalRecv(); //긴 ir신호 받는 모드
	}

	/*
	lcd.setCursor(5, 1);
	lcd.print("ABC");
	delay_ms(3000);
	lcd.setCursor(5, 1);
	lcd.print("CBA");
	delay_ms(3000);
	//*/
	delay_ms(10);
	//delay_ms(1000);
}

void delay_ms(int nn) //딜레이할 때 다른 인터럽트 가능
{
	unsigned long previousMillis_1 = millis();
	unsigned long currentMillis_1 = millis();
	while (currentMillis_1 - previousMillis_1 < nn)
	{
		currentMillis_1 = millis();
		delay(1);
	}
}

void kit_init()
{
	//pinMode(led_D4, OUTPUT); // PA0 led1
	//pinMode(led_D3, OUTPUT); // PA1 led2
	//pinMode(led_D2, OUTPUT); // PA2 led3
	//pinMode(led_D1, OUTPUT); // PA3 led4
	DDRA = 0xF;

	digitalWrite(led_D4, HIGH); // HIGH가 끄는거
	digitalWrite(led_D3, HIGH);
	digitalWrite(led_D2, HIGH);
	digitalWrite(led_D1, HIGH);

	DDRB = 0xFF;  //7segment
	DDRC = 0xF;   //7segment 자릿수 on:1 off:0
}

void lcd_init()
{
	//lcd.begin();
	//lcd.backlight();
	//lcd.clear();
	//lcd.print("Hello, world!");
}

void switch_init() {
	pinMode(18, INPUT_PULLUP); //스위치u4
	pinMode(19, INPUT_PULLUP); //스위치u3
	attachInterrupt(digitalPinToInterrupt(18), switchU4, FALLING);
	attachInterrupt(digitalPinToInterrupt(19), switchU3, FALLING);
}

void switchU3()
{
	led_D4_state = !led_D4_state;
	digitalWrite(led_D4, led_D4_state);
	//delay_ms(1000);
	//digitalWrite(led_D4, HIGH);
}

void switchU4()
{
	irrecv_mode = !irrecv_mode;
	if (irrecv_mode == 0)
	{
		Serial.println("IRrecvDumpV2 모드");
	}
	else
	{
		Serial.println("LongIrSignalRecv 모드");
	}

}

void timer_init()
{
	MsTimer2::set(1, msTimer); // Timer0 함수를 1ms마다 호출 한다
	MsTimer2::start();

	//timer1
	noInterrupts();
	TCCR1A = 0;
	TCCR1B = ((1 << CS12) | (0 << CS11) | (0 << CS10));
	//TCCR3B |= (1 << CS32);
	timer1_counter = 3036; // preload timer 65536-16MHz/256/1Hz
	TCNT1 = timer1_counter;   // preload timer
	//TCCR3B |= (1 << CS32);    // 256 prescaler
	TIMSK |= (1 << TOIE1);   // enable timer overflow interrupt
	interrupts();

}

ISR(TIMER1_OVF_vect)        // 타이머1 오버플로우 벡터 1초마다 작동
{
	TCNT1 = timer1_counter;   // preload timer
	led_D3_state = !led_D3_state;
	digitalWrite(led_D3, led_D3_state);
}


volatile unsigned int x, y = 0; //volatile 변수 최적화하지 않음.
volatile unsigned int ss, mm = 0;
void msTimer() //타이머0 함수 1ms마다
{
	x++;
	y++;
	if (x > 1000)      //x가 1000이면 1ms*1000=1�?
	{
		x = 0;
		ss++;
		if (ss > 59)
		{
			ss = 0;
			mm++;
		}
	}
	switch (y) //7segment 시간
	{
	case 1:
		PORTC = 0x08;
		PORTB = seg7[ss % 10];
		break;
	case 6:
		PORTC = 0x04;
		PORTB = seg7[ss / 10];
		break;
	case 11:
		PORTC = 0x02;
		PORTB = seg7[mm % 10];
		break;
	case 16:
		PORTC = 0x01;
		PORTB = seg7[mm / 10];
		break;
	}
	if (y >= 20)
		y = 0;
}

//삼성 에어컨 켜기 ir값
unsigned int  samsung_raw[] = { 3016, 9012, 512, 556, 512, 1524, 512, 536, 500, 564, 512, 532, 512, 532, 512, 532, 512, 556, 484, 560, 516, 1524, 508, 560, 512, 528, 516, 1544, 480, 564, 516, 528, 516, 1544, 516, 1524, 512, 1572, 516, 1544, 516, 1524, 512, 556, 480, 564, 512, 532, 512, 532, 512, 556, 432, 612, 512, 532, 512, 532, 512, 536, 508, 556, 516, 528, 512, 532, 512, 532, 512, 556, 452, 592, 516, 528, 516, 528, 512, 536, 456, 608, 516, 528, 516, 528, 516, 532, 512, 556, 480, 560, 516, 528, 516, 532, 512, 556, 480, 564, 512, 528, 516, 532, 512, 532, 512, 556, 512, 1524, 512, 1548, 516, 1544, 512, 1524, 480, 2504, 2968, 9012, 456, 1600, 516, 528, 516, 532, 512, 556, 540, 532, 508, 536, 508, 556, 480, 564, 516, 528, 516, 1544, 516, 528, 516, 528, 516, 1544, 516, 528, 516, 1548, 516, 1516, 508, 1556, 516, 1596, 432, 1604, 516, 1544, 480, 564, 516, 528, 516, 532, 512, 532, 500, 564, 516, 528, 504, 568, 516, 528, 512, 532, 512, 556, 488, 556, 508, 536, 512, 532, 480, 568, 508, 556, 508, 536, 516, 528, 516, 528, 512, 556, 480, 564, 460, 612, 504, 540, 512, 528, 516, 532, 512, 556, 452, 592, 512, 532, 456, 584, 516, 556, 488, 556, 516, 528, 512, 532, 512, 532, 512, 556, 460, 588, 512, 552, 464, 1388, 2992, 9032, 516, 1548, 476, 620, 504, 540, 516, 528, 512, 532, 512, 556, 480, 564, 536, 532, 508, 536, 516, 1524, 512, 552, 508, 536, 516, 528, 516, 532, 456, 1600, 516, 1548, 480, 564, 512, 1524, 528, 1532, 516, 1544, 516, 1544, 492, 1544, 516, 1544, 516, 1520, 516, 1572, 516, 528, 516, 552, 480, 564, 516, 1524, 456, 1600, 516, 1548, 512, 532, 512, 532, 512, 532, 512, 556, 476, 568, 516, 1520, 512, 556, 508, 536, 516, 1544, 480, 1556, 516, 532, 496, 1560, 516, 1544, 508, 536, 516, 528, 516, 532, 456, 612, 504, 540, 512, 532, 512, 532, 512, 556, 504, 1528, 520, 1544, 516, 1576, 512, 1544, 464 };

void serialEvent() //ISR(USART0_RX_vect)아두이노 버전. 인터럽트
{
	led_D1_state = !led_D1_state;
	digitalWrite(led_D1, led_D1_state);
	WatchUpload();
	while (Serial.available())
	{
		//char c = Serial.read();
		//Serial.print(c);
		String inString = Serial.readStringUntil('\n');
		Serial.println("avr: " + inString);

		if (inString.indexOf("on") >= 0)
		{
			irsend.sendRaw(samsung_raw, sizeof(samsung_raw) / sizeof(samsung_raw[0]), 38);
			Serial.println("Carrier on");
		}
	}
}

void WatchUpload() //업로드를 하는지 감시해서 리셋
{
	if (BaudRate == 115200)
	{
		if (Serial.available() > 1 && Serial.peek() == '0')
		{
			if (Serial.read() == '0' && Serial.read() == ' ')
			{
				digitalWrite(ResetPin, LOW); //리셋
				delay(3000);
				Serial.print("Reset failed");
				//resetFunc(); //리셋함수
				//wdt_enable(WDTO_15MS); //워치독타이머
				//while(1) {};
			}
		}
	}
	else if (BaudRate == 9600)
	{
		if (Serial.available() > 0 && Serial.peek() == 255)
		{
			digitalWrite(ResetPin, LOW);
			delay(3000);
			Serial.print("Reset failed");
		}
	}

}
