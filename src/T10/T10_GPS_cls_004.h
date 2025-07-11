#ifndef T10_GPSMODULE_H
#define T10_GPSMODULE_H

#include <Arduino.h>
#include <TinyGPSPlus.h> // TinyGPSPlus 라이브러리 포함

// =========================================================================
// G_T10_ 전역 상수 정의 (클래스 내부 또는 네임스페이스로 이동 가능하지만,
// 일반적으로 상수 정의는 전역 또는 enum class로 유지하는 경우가 많음)
// =========================================================================

// GPS 모듈과 통신할 ESP32의 하드웨어 UART 포트 정의 (Serial2 사용)
#define G_T10_GPS_SERIAL Serial2
// BN-280의 일반적인 기본 통신 속도 (9600bps)
#define G_T10_GPS_BAUD_RATE 9600

// 서울시청의 위도와 경도 정의
static const double G_T10_SEOUL_CITYHALL_LAT = 37.566535;
static const double G_T10_SEOUL_CITYHALL_LNG = 126.977969;

// =========================================================================
// T10_GPS_ALL_DATA 구조체 정의 (공개 접근을 위해 클래스 외부에 유지)
// =========================================================================

// GPS의 모든 정보를 담을 구조체 정의
struct T10_GPS_ALL_DATA {
    // 위치 정보
    struct LocationData {
        double          latitude;           // 위도 (도 단위)
        double          longitude;          // 경도 (도 단위)
        unsigned long   ageMs;              // 데이터 갱신 시간 (밀리초)
        bool            isValid;            // 유효성 여부
    } location;

    // 날짜 정보
    struct DateData {
        uint16_t        year;       // 연도
        uint8_t         month;      // 월
        uint8_t         day;        // 일
        uint32_t        value;      // 원시 날짜 값
        unsigned long   ageMs;      // 데이터 갱신 시간 (밀리초)
        bool isValid;               // 유효성 여부
    } date;

    // 시간 정보
    struct TimeData {
        uint8_t         hour;           // 시 (UTC)
        uint8_t         minute;         // 분 (UTC)
        uint8_t         second;         // 초 (UTC)
        uint8_t         centisecond;    // 1/100 초
        uint32_t        value;          // 원시 시간 값
        unsigned long   ageMs;          // 데이터 갱신 시간 (밀리초)
        bool            isValid;        // 유효성 여부
    } time;

    // 속도 정보
    struct SpeedData {
        double          mps;            // 속도 (미터/초)
        double          kmph;           // 속도 (킬로미터/시)
        unsigned long   ageMs;          // 데이터 갱신 시간 (밀리초)
        bool            isValid;        // 유효성 여부
    } speed;

    // 진행 방향 정보
    struct CourseData {
        double          degrees;        // 진행 방향 (도 단위)
        unsigned long   ageMs;          // 데이터 갱신 시간 (밀리초)
        bool            isValid;        // 유효성 여부
    } course;

    // 고도 정보
    struct AltitudeData {
        double          meters;         // 고도 (미터)
        double          kilometers;     // 고도 (킬로미터)
        unsigned long   ageMs;          // 데이터 갱신 시간 (밀리초)
        bool            isValid;        // 유효성 여부
    } altitude;

    // 위성 수 정보
    struct SatellitesData {
        uint32_t        value;          // 위성 수
        unsigned long   ageMs;          // 데이터 갱신 시간 (밀리초)
        bool            isValid;        // 유효성 여부
    } satellites;

    // HDOP (Horizontal Dilution of Precision) 정보
    struct HdopData {
        double          hdop;           // HDOP 값
        unsigned long   ageMs;          // 데이터 갱신 시간 (밀리초)
        String          accuracyLevel;  // HDOP 정확도 수준 추가
        bool            isValid;        // 유효성 여부
    } hdop;

    // 진단 정보
    struct DiagnosticsData {
        uint32_t        charsProcessed;   // 처리된 문자 수
        uint32_t        sentencesWithFix; // Fix가 있는 문장 수
        uint32_t        failedChecksum;   // 체크섬 실패 수
        uint32_t        passedChecksum;   // 체크섬 성공 수
    } diagnostics;

    // 특정 목적지 (서울시청) 관련 정보
    struct DestinationData {
        double          distanceKm;     // 목적지와의 거리 (킬로미터)
        double          courseToDeg;    // 목적지로 향하는 방향 (도 단위)
        bool            isValid;        // 계산 유효성 여부 (위치 데이터가 유효할 때만 유효)
    } seoulCityHall;                    // 서울시청에 대한 정보를 담기 위한 구조체 변수명

    // 기타 상태
    bool hasFix;                   // 현재 Fix 여부 (위치 고정)
};

// =========================================================================
// T10_GpsModule 클래스 정의
// =========================================================================

class T10_GpsModule {
public:
    /**
     * @brief T10_GpsModule 클래스의 생성자입니다.
     * @param gpsSerial GPS 모듈이 연결된 HardwareSerial 객체 (예: Serial2)
     * @param baudRate GPS 모듈의 통신 속도 (예: 9600)
     * @param printIntervalMs GPS 정보를 시리얼 모니터에 출력할 간격 (밀리초)
     */
    T10_GpsModule(HardwareSerial& gpsSerial, long baudRate, unsigned long printIntervalMs)
      : _gps(TinyGPSPlus()), _gpsSerial(gpsSerial), _baudRate(baudRate), _printIntervalMs(printIntervalMs), _lastGpsPrintTime(0) {}

    /**
     * @brief GPS 모듈 통신을 초기화하고 시리얼 포트를 설정합니다.
     */
    void begin() {
        _gpsSerial.begin(_baudRate);

        Serial.println("--- ESP32 GPS 모듈 초기화 ---");
        Serial.println("TinyGPSPlus 라이브러리 v. " + String(TinyGPSPlus::libraryVersion()));
        Serial.println("BN-280 GPS 모듈을 ESP32 Serial2 (RX: GPIO16, TX: GPIO17)에 연결했는지 확인하세요.");
        Serial.println("GPS 신호 수신에는 시간이 걸릴 수 있습니다 (특히 실내에서).");
    }

    /**
     * @brief GPS 모듈로부터 시리얼 데이터를 읽어 TinyGPSPlus로 파싱합니다.
     * 이 함수는 loop() 함수에서 계속 호출되어야 합니다.
     */
    void processData() {
        while (_gpsSerial.available() > 0) {
            _gps.encode(_gpsSerial.read());
        }
    }

    /**
     * @brief TinyGPSPlus 객체에서 현재 GPS 정보를 읽어 T10_GPS_ALL_DATA 구조체에 저장합니다.
     * @param p_gpsData 저장할 T10_GPS_ALL_DATA 구조체 변수의 참조
     */
    void updateAllData(T10_GPS_ALL_DATA &p_gpsData) {
        // 위치 정보 업데이트
        p_gpsData.location.latitude         = _gps.location.lat();
        p_gpsData.location.longitude        = _gps.location.lng();
        p_gpsData.location.ageMs            = _gps.location.age();
        p_gpsData.location.isValid          = _gps.location.isValid();

        // 날짜 정보 업데이트   
        p_gpsData.date.year                 = _gps.date.year();
        p_gpsData.date.month                = _gps.date.month();
        p_gpsData.date.day                  = _gps.date.day();
        p_gpsData.date.value                = _gps.date.value();
        p_gpsData.date.ageMs                = _gps.date.age();
        p_gpsData.date.isValid              = _gps.date.isValid();

        // 시간 정보 업데이트   
        p_gpsData.time.hour                 = _gps.time.hour();
        p_gpsData.time.minute               = _gps.time.minute();
        p_gpsData.time.second               = _gps.time.second();
        p_gpsData.time.centisecond          = _gps.time.centisecond();
        p_gpsData.time.value                = _gps.time.value();
        p_gpsData.time.ageMs                = _gps.time.age();
        p_gpsData.time.isValid              = _gps.time.isValid();

        // 속도 정보 업데이트   
        p_gpsData.speed.mps                 = _gps.speed.mps();
        p_gpsData.speed.kmph                = _gps.speed.kmph();
        p_gpsData.speed.ageMs               = _gps.speed.age();
        p_gpsData.speed.isValid             = _gps.speed.isValid();

        // 진행 방향 정보 업데이트  
        p_gpsData.course.degrees            = _gps.course.deg();
        p_gpsData.course.ageMs              = _gps.course.age();
        p_gpsData.course.isValid            = _gps.course.isValid();

        // 고도 정보 업데이트   
        p_gpsData.altitude.meters           = _gps.altitude.meters();
        p_gpsData.altitude.kilometers       = _gps.altitude.kilometers();
        p_gpsData.altitude.ageMs            = _gps.altitude.age();
        p_gpsData.altitude.isValid          = _gps.altitude.isValid();

        // 위성 수 정보 업데이트    
        p_gpsData.satellites.value          = _gps.satellites.value();
        p_gpsData.satellites.ageMs          = _gps.satellites.age();
        p_gpsData.satellites.isValid        = _gps.satellites.isValid();

        // HDOP 정보 업데이트   
        p_gpsData.hdop.hdop                 = _gps.hdop.hdop();
        p_gpsData.hdop.ageMs                = _gps.hdop.age();
        p_gpsData.hdop.isValid              = _gps.hdop.isValid();

        // HDOP 정확도 수준 업데이트    
        if (p_gpsData.hdop.isValid) {   
            p_gpsData.hdop.accuracyLevel        = getHdopAccuracyLevel(p_gpsData.hdop.hdop);
        } else {    
            p_gpsData.hdop.accuracyLevel        = "알 수 없음 (Not Available)"; // 유효하지 않을 경우
        }

        // 진단 정보 업데이트
        p_gpsData.diagnostics.charsProcessed    = _gps.charsProcessed();
        p_gpsData.diagnostics.sentencesWithFix  = _gps.sentencesWithFix();
        p_gpsData.diagnostics.failedChecksum    = _gps.failedChecksum();
        p_gpsData.diagnostics.passedChecksum    = _gps.passedChecksum();

        // 서울시청 관련 정보 업데이트
        p_gpsData.seoulCityHall.isValid         = p_gpsData.location.isValid; // 현재 위치가 유효해야 서울시청 정보도 유효
        if (p_gpsData.seoulCityHall.isValid) {
            p_gpsData.seoulCityHall.distanceKm  = TinyGPSPlus::distanceBetween(
                                                    p_gpsData.location.latitude, p_gpsData.location.longitude,
                                                    G_T10_SEOUL_CITYHALL_LAT, G_T10_SEOUL_CITYHALL_LNG) / 1000.0; // 미터 -> 킬로미터 변환
            p_gpsData.seoulCityHall.courseToDeg = TinyGPSPlus::courseTo(
                                                    p_gpsData.location.latitude, p_gpsData.location.longitude,
                                                    G_T10_SEOUL_CITYHALL_LAT, G_T10_SEOUL_CITYHALL_LNG);
        } else {
            p_gpsData.seoulCityHall.distanceKm  = 0.0;
            p_gpsData.seoulCityHall.courseToDeg = 0.0;
        }

        // 기타 상태 업데이트
        p_gpsData.hasFix                        = _gps.location.isValid(); // 위치 데이터가 유효하면 fix로 간주
    }

    /**
     * @brief T10_GPS_ALL_DATA 구조체에 저장된 모든 GPS 정보를 시리얼 모니터에 출력합니다.
     * @param p_gpsData 출력할 T10_GPS_ALL_DATA 구조체 변수의 상수 참조
     */
    void printAllData(const T10_GPS_ALL_DATA &p_gpsData) {
        Serial.println("--- GPS 모든 정보 출력 ---");

        Serial.println("[위치]");
        if (p_gpsData.location.isValid) {
            Serial.printf("  위도: %.6f, 경도: %.6f\n", p_gpsData.location.latitude, p_gpsData.location.longitude);
            Serial.printf("  데이터 갱신 시간 (위치): %lu ms\n", p_gpsData.location.ageMs);
        } else {
            Serial.println("  위치 데이터: 유효하지 않음");
        }

        Serial.println("[서울시청과의 거리 및 방향]");
        if (p_gpsData.seoulCityHall.isValid) {
            Serial.printf("  서울시청까지 거리: %.2f km\n", p_gpsData.seoulCityHall.distanceKm);
            Serial.printf("  서울시청 방향: %.2f deg (%s)\n", p_gpsData.seoulCityHall.courseToDeg, TinyGPSPlus::cardinal(p_gpsData.seoulCityHall.courseToDeg));
        } else {
            Serial.println("  서울시청 정보: 현재 위치 데이터가 유효하지 않아 계산 불가");
        }

        Serial.println("[날짜]");
        if (p_gpsData.date.isValid) {
            Serial.printf("  날짜: %04d-%02d-%02d\n", p_gpsData.date.year, p_gpsData.date.month, p_gpsData.date.day);
            Serial.printf("  원시 날짜 값: %lu\n", p_gpsData.date.value);
            Serial.printf("  데이터 갱신 시간 (날짜): %lu ms\n", p_gpsData.date.ageMs);
        } else {
            Serial.println("  날짜 데이터: 유효하지 않음");
        }

        Serial.println("[시간]");
        if (p_gpsData.time.isValid) {
            Serial.printf("  UTC 시간: %02d:%02d:%02d.%02d\n",
                          p_gpsData.time.hour, p_gpsData.time.minute, p_gpsData.time.second, p_gpsData.time.centisecond);
            Serial.printf("  원시 시간 값: %lu\n", p_gpsData.time.value);
            Serial.printf("  데이터 갱신 시간 (시간): %lu ms\n", p_gpsData.time.ageMs);
        } else {
            Serial.println("  시간 데이터: 유효하지 않음");
        }

        Serial.println("[속도]");
        if (p_gpsData.speed.isValid) {
            Serial.printf("  m/s: %.2f, km/h: %.2f\n",
                          p_gpsData.speed.mps, p_gpsData.speed.kmph);
            Serial.printf("  데이터 갱신 시간 (속도): %lu ms\n", p_gpsData.speed.ageMs);
        } else {
            Serial.println("  속도 데이터: 유효하지 않음");
        }

        Serial.println("[진행 방향]");
        if (p_gpsData.course.isValid) {
            Serial.printf("  진행 방향: %.2f deg (%s)\n", p_gpsData.course.degrees, TinyGPSPlus::cardinal(p_gpsData.course.degrees));
            Serial.printf("  데이터 갱신 시간 (방향): %lu ms\n", p_gpsData.course.ageMs);
        } else {
            Serial.println("  진행 방향 데이터: 유효하지 않음");
        }

        Serial.println("[고도]");
        if (p_gpsData.altitude.isValid) {
            Serial.printf("  미터: %.2f, 킬로미터: %.2f\n",
                          p_gpsData.altitude.meters, p_gpsData.altitude.kilometers);
            Serial.printf("  데이터 갱신 시간 (고도): %lu ms\n", p_gpsData.altitude.ageMs);
        } else {
            Serial.println("  고도 데이터: 유효하지 않음");
        }

        Serial.println("[위성 및 정밀도]");
        if (p_gpsData.satellites.isValid) {
            Serial.printf("  수신 위성 수: %lu\n", p_gpsData.satellites.value);
            Serial.printf("  데이터 갱신 시간 (위성): %lu ms\n", p_gpsData.satellites.ageMs);
        } else {
            Serial.println("  위성 수 데이터: 유효하지 않음");
        }
        if (p_gpsData.hdop.isValid) {
            Serial.printf("  HDOP: %.2f (정확도 수준: %s)\n", p_gpsData.hdop.hdop, p_gpsData.hdop.accuracyLevel.c_str());
            Serial.printf("  데이터 갱신 시간 (HDOP): %lu ms\n", p_gpsData.hdop.ageMs);
        } else {
            Serial.println("  HDOP 데이터: 유효하지 않음");
        }
        
        Serial.println("[진단 정보]");
        Serial.printf("  처리된 문자 수: %lu\n"     , p_gpsData.diagnostics.charsProcessed);
        Serial.printf("  Fix 있는 문장 수: %lu\n"   , p_gpsData.diagnostics.sentencesWithFix);
        Serial.printf("  체크섬 실패 수: %lu\n"     , p_gpsData.diagnostics.failedChecksum);
        Serial.printf("  체크섬 성공 수: %lu\n"     , p_gpsData.diagnostics.passedChecksum);

        if (!p_gpsData.hasFix) {
            Serial.println("  현재 Fix 상태: 없음 (위치 고정 대기 중)");
        } else {
            Serial.println("  현재 Fix 상태: 성공");
        }

        // 데이터 수신 경고 (클래스 내부 _gps 객체 참조)
        if (_gps.charsProcessed() < 10) {
            Serial.println("  경고: GPS 모듈에서 데이터가 수신되지 않거나 매우 적습니다. 배선, 전원, 통신 속도를 확인하세요.");
        } else if (!_gps.location.isValid() && millis() - _gps.charsProcessed() > 5000) {
            Serial.println("  경고: GPS 신호가 약하거나 위치 고정이 안 됩니다.");
        }

        Serial.println("----------------------------------");
    }

    /**
     * @brief GPS 정보 출력 간격이 되었는지 확인하고, 그렇다면 GPS 정보를 업데이트하고 출력합니다.
     * 이 함수는 loop() 함수에서 계속 호출되어야 합니다.
     */
    void run() {
        unsigned long currentTime = millis();

        if (currentTime - _lastGpsPrintTime >= _printIntervalMs) {
            _lastGpsPrintTime = currentTime; // 마지막 출력 시간 갱신

            T10_GPS_ALL_DATA currentGpsData; // 로컬 구조체 변수 currentGpsData
            updateAllData(currentGpsData);   // 현재 GPS 데이터를 구조체에 저장
            printAllData(currentGpsData);    // 구조체 내용 출력
        }
    }

    /**
     * @brief 현재 GPS 데이터를 반환합니다.
     * @return T10_GPS_ALL_DATA 현재 GPS 데이터 구조체
     */
    T10_GPS_ALL_DATA getCurrentGpsData() {
        T10_GPS_ALL_DATA data;
        updateAllData(data);
        return data;
    }

private:
    TinyGPSPlus _gps;                 // TinyGPSPlus 객체
    HardwareSerial& _gpsSerial;       // GPS 모듈과 연결된 시리얼 포트 참조
    long _baudRate;                   // GPS 통신 속도
    unsigned long _printIntervalMs;   // GPS 정보 출력 간격 (밀리초)
    unsigned long _lastGpsPrintTime;  // 마지막 GPS 정보를 출력한 시간

    /**
     * @brief HDOP 값에 따른 정확도 수준 문자열을 반환하는 헬퍼 함수입니다.
     * 이 함수는 클래스 내부에서만 사용되므로 private으로 선언합니다.
     * @param hdop HDOP 값
     * @return 정확도 수준을 나타내는 String
     */
    String getHdopAccuracyLevel(double hdop) {
        if      (hdop <= 1.0)  return "이상적인 (Excellent)";
        else if (hdop <= 2.0)  return "우수한 (Good)";
        else if (hdop <= 5.0)  return "적당한 (Moderate)";
        else if (hdop <= 10.0) return "보통의 (Fair)";
        else if (hdop <= 20.0) return "나쁜 (Poor)";
        else                   return "매우 나쁜 (Very Poor)";
    }
};

#endif // T10_GPSMODULE_H
