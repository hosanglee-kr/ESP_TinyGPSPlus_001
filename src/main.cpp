#include <Arduino.h>

#define T10
#ifdef T10
	#include "T10/T10_GPS_cls_004.h"
    // ESP32 Serial2 (RX: GPIO16, TX: GPIO17)를 GPSModule 객체에 전달
    GPSModule myGPS(Serial2, 9600); 
#endif

void setup() {
	// delay(5000);
	Serial.begin(115200);
	
    #ifdef T10
	    //T10_initGpsModule();
	    myGPS.begin(); // GPS 모듈 초기화

        // 목적지 설정 (예: 남양주시청)
        // 남양주시청 (Namyangju City Hall) 좌표: 37.6358, 127.2166
        myGPS.setDestination(37.6358, 127.2166); 	
    #endif

	Serial.println("11111");
}

void loop() {
    #ifdef T10
	   myGPS.run(); 
    #endif
}


