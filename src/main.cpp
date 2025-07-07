#include <Arduino.h>

#define T10
#ifdef T10
	#include "T10/T10_GPS_001.h"
#endif

void setup() {
	// delay(5000);
	Serial.begin(115200);
	
    #ifdef T10
	    T10_initGpsModule();
    #endif

	Serial.println("11111");
}

void loop() {
    #ifdef T10
	    T10_GPS_run();
    #endif
}
