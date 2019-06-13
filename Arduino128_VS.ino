#include <MsTimer2.h>
#include <avr/wdt.h> //watchdog timeout
#include <IRremote.h>

IRrecv irrecv(20); //IRrecv 객체속성
IRsend irsend;
decode_results IR_signals;  //IR통신 

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

volatile int timer3_counter; //타이머3 카운터
volatile bool led_D1_state, led_D2_state, led_D3_state, led_D4_state = HIGH; //LED 상태 나타내는 변수

//7sengment 16진수 숫자 배열
const byte seg7[16] = { 0x03, 0x9F, 0x25, 0x0D, 0x99, 0x49, 0x41, 0x1B, 0x01, 0x09, 0x11, 0xC1, 0x63, 0x85, 0x61, 0x71 };

void(*resetFunc) (void) = 0; //리셋func

void setup()
{
	//리셋핀 설정 //리셋핀 설정때문에 avr 스튜디오 4.19 연결 안됨
	digitalWrite(ResetPin, HIGH); //무한리셋 할까봐 먼저 HIGH설정
	delay(200); // 셋업에 딜레이 함수쓰면 avrstudio 고장남.
	pinMode(ResetPin, OUTPUT); //리셋

	kit_init(); //키트 설정
	timer_init(); //타이머 설정

	irrecv.enableIRIn(); // ir리시버 시작

	Serial.begin(BaudRate); //시리얼 통신 시작
	Serial.println("아두이노 시작");
}

int testNum = 0;
unsigned long previousMillis = 0; //이전시간
const long delayTime = 5000; //5초 대기시간
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

	/*
	  for (int i = 0; i < 3; i++)
	  {
		irsend.sendNEC(0x22AE7A2A, 32);
		delay(40);
	  }
	//	for (int i = 0; i < 3; i++)
	 // {
	//	irsend.sendNEC(0xFDAC0150, 32);
	//	delay(40);
	//  }
	  // */

	  //  ir리시버
   ///*
	if (irrecv.decode(&IR_signals))
	{
		Serial.println(IR_signals.value, HEX); //16진수로 출력     
		Serial.println(IR_signals.bits, DEC);  //송신 데이터의 길이 10진수로 출력      
		irrecv.resume(); //다음 신호를 받을 수 있게     
	}
//*/


	delay(10);
	//delay(1000); 
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

	pinMode(18, INPUT_PULLUP); //스위치u4
	pinMode(19, INPUT_PULLUP); //스위치u3
	attachInterrupt(digitalPinToInterrupt(18), switchU4, FALLING);
	attachInterrupt(digitalPinToInterrupt(19), switchU3, FALLING);
}

void switchU3()
{
	led_D4_state = !led_D4_state;
	digitalWrite(led_D4, led_D4_state);
	//delay(1000);
	//digitalWrite(led_D4, HIGH);
}

void switchU4()
{	
	digitalWrite(led_D3, LOW);
	//delay(1000);
	//digitalWrite(led_D3, HIGH);
}

void timer_init()
{
	MsTimer2::set(1, Timer0); // Timer0 함수를 1ms마다 호출 한다
	MsTimer2::start();

	//timer3 //IR리모트가 타이머 1쓰기 때문에 타이머3 쓴다.
	noInterrupts();
	TCCR3A = 0;
	TCCR3B = ((1 << CS32) | (0 << CS31) | (0 << CS30));
	//TCCR3B |= (1 << CS32);
	timer3_counter = 3036; // preload timer 65536-16MHz/256/1Hz
	TCNT3 = timer3_counter;   // preload timer
	//TCCR3B |= (1 << CS32);    // 256 prescaler
	ETIMSK |= (1 << TOIE3);   // enable timer overflow interrupt  
	interrupts();

}

ISR(TIMER3_OVF_vect)        // 타이머3 오버플로우 벡터 1초마다 작동
{
	TCNT3 = timer3_counter;   // preload timer
	led_D3_state = !led_D3_state;
	digitalWrite(led_D3, led_D3_state);	
}


volatile unsigned int x, y = 0; //volatile 변수 최적화하지 않음.
volatile unsigned int ss, mm = 0;
void Timer0() //타이머0 함수 1ms마다
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
