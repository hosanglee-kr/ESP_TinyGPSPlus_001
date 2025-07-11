#include <Arduino.h>

#define T10
#ifdef T10
	#include "T10/T10_GPS_cls_004.h"
    T10_GpsModule myGps(G_T10_GPS_SERIAL, G_T10_GPS_BAUD_RATE, 1000);

#endif

void setup() {
	// delay(5000);
	Serial.begin(115200);
	
    #ifdef T10
	    //T10_initGpsModule();
	    // GPS 모듈 초기화:
        // GPS 모듈의 시리얼 통신을 설정하고 초기 메시지를 출력합니다.
        myGps.begin();

	    // 예시: 목적지를 남양주시청으로 설정 (임의의 좌표)
        // 남양주시청 (Namyangju City Hall) 좌표: 37.6358, 127.2166
        double targetLat = 37.6358;
        double targetLng = 127.2166;
        T10_setDestination(targetLat, targetLng); 

	
    #endif

	Serial.println("11111");
}

void loop() {
    #ifdef T10
	   // T10_GPS_run();
	   // GPS 데이터 처리:
       // GPS 모듈로부터 수신되는 NMEA 데이터를 읽고 파싱합니다.
       // 이 함수는 가능한 한 자주 호출되어야 합니다 (non-blocking).
       myGps.processData();

     // GPS 정보 출력 실행:
     // 설정된 간격(여기서는 1초)마다 GPS 데이터를 업데이트하고 시리얼 모니터에 출력합니다.
     // 이 함수 내부에서 데이터 업데이트와 출력이 모두 이루어집니다.
     myGps.run();

    // 추가적인 로직을 여기에 구현할 수 있습니다.
    // 예를 들어, 특정 조건에서 GPS 데이터를 얻어 다른 용도로 사용하고 싶다면:
    /*
    // 예시: 위도/경도가 유효할 때만 특정 작업을 수행
    T10_GPS_ALL_DATA currentGpsInfo = myGps.getCurrentGpsData();
    if (currentGpsInfo.location.isValid) {
      // Serial.printf("현재 위도(별도): %.6f, 경도(별도): %.6f\n",
      //              currentGpsInfo.location.latitude, currentGpsInfo.location.longitude);
      // 여기에 GPS 데이터를 활용한 로직 추가
    }
    */
    #endif
}


