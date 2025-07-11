#ifndef T10_GPSMODULE_CLASS_H
#define T10_GPSMODULE_CLASS_H

#include <Arduino.h>
#include <TinyGPSPlus.h>

// GPS 정보 출력 간격 (밀리초)
static const unsigned long GPS_PRINT_INTERVAL_MS = 1000; // 1초

// T10_GPS_ALL_DATA 구조체는 그대로 클래스 외부에 두어 데이터를 전달할 때 활용
// 또는 클래스 내부에 private struct로 정의할 수도 있습니다.
// 여기서는 편의상 그대로 외부 정의를 유지합니다.
struct T10_GPS_ALL_DATA {
    // ... (기존 T10_GPS_ALL_DATA 구조체 정의 내용 동일) ...
    struct LocationData {
        double latitude;
        double longitude;
        unsigned long ageMs;
        bool isValid;
    } location;

    struct DateData {
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint32_t value;
        unsigned long ageMs;
        bool isValid;
    } date;

    struct TimeData {
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint8_t centisecond;
        uint32_t value;
        unsigned long ageMs;
        bool isValid;
    } time;

    struct SpeedData {
        double mps;
        double kmph;
        unsigned long ageMs;
        bool isValid;
    } speed;

    struct CourseData {
        double degrees;
        unsigned long ageMs;
        bool isValid;
    } course;

    struct AltitudeData {
        double meters;
        double kilometers;
        unsigned long ageMs;
        bool isValid;
    } altitude;

    struct SatellitesData {
        uint32_t value;
        unsigned long ageMs;
        bool isValid;
    } satellites;

    struct HdopData {
        double hdop;
        unsigned long ageMs;
        String accuracyLevel;
        bool isValid;
    } hdop;

    struct DiagnosticsData {
        uint32_t charsProcessed;
        uint32_t sentencesWithFix;
        uint32_t failedChecksum;
        uint32_t passedChecksum;
    } diagnostics;

    struct DestinationData {
        double latitude;
        double longitude;
        double distanceKm;
        double courseToDeg;
        bool isSet;
        bool isValid;
    } destination;

    bool hasFix;
};


class GPSModule {
public:
    // 생성자: GPS 시리얼 포트와 통신 속도를 인자로 받아 초기화
    GPSModule(HardwareSerial& gpsSerial, long baudRate)
        : _gpsSerial(gpsSerial), _baudRate(baudRate), 
          _lastGpsPrintTime(0), _destinationLat(0.0), _destinationLng(0.0), _isDestinationSet(false) {}

    // 모듈 초기화
    void begin() {
        _gpsSerial.begin(_baudRate);
        Serial.println("--- GPS 모듈 초기화 ---");
        Serial.println("TinyGPSPlus 라이브러리 v. " + String(TinyGPSPlus::libraryVersion()));
        Serial.println("GPS 신호 수신에는 시간이 걸릴 수 있습니다 (특히 실내에서).");
    }

    // 시리얼 데이터 처리 (loop()에서 호출)
    void processData() {
        while (_gpsSerial.available() > 0) {
            _gps.encode(_gpsSerial.read());
        }
    }

    // 목적지 설정
    void setDestination(double latitude, double longitude) {
        _destinationLat = latitude;
        _destinationLng = longitude;
        _isDestinationSet = true;
        Serial.printf("목적지 설정됨: 위도 %.6f, 경도 %.6f\n", _destinationLat, _destinationLng);
    }

    // 모든 GPS 데이터 업데이트 (내부적으로 TinyGPSPlus 객체에서 읽어옴)
    void updateAllData(T10_GPS_ALL_DATA &data) {
        // 위치 정보
        data.location.latitude = _gps.location.lat();
        data.location.longitude = _gps.location.lng();
        data.location.ageMs = _gps.location.age();
        data.location.isValid = _gps.location.isValid();

        // 날짜 정보
        data.date.year = _gps.date.year();
        data.date.month = _gps.date.month();
        data.date.day = _gps.date.day();
        data.date.value = _gps.date.value();
        data.date.ageMs = _gps.date.age();
        data.date.isValid = _gps.date.isValid();

        // 시간 정보
        data.time.hour = _gps.time.hour();
        data.time.minute = _gps.time.minute();
        data.time.second = _gps.time.second();
        data.time.centisecond = _gps.time.centisecond();
        data.time.value = _gps.time.value();
        data.time.ageMs = _gps.time.age();
        data.time.isValid = _gps.time.isValid();

        // 속도 정보
        data.speed.mps = _gps.speed.mps();
        data.speed.kmph = _gps.speed.kmph();
        data.speed.ageMs = _gps.speed.age();
        data.speed.isValid = _gps.speed.isValid();

        // 진행 방향 정보
        data.course.degrees = _gps.course.deg();
        data.course.ageMs = _gps.course.age();
        data.course.isValid = _gps.course.isValid();

        // 고도 정보
        data.altitude.meters = _gps.altitude.meters();
        data.altitude.kilometers = _gps.altitude.kilometers();
        data.altitude.ageMs = _gps.altitude.age();
        data.altitude.isValid = _gps.altitude.isValid();

        // 위성 수 정보
        data.satellites.value = _gps.satellites.value();
        data.satellites.ageMs = _gps.satellites.age();
        data.satellites.isValid = _gps.satellites.isValid();

        // HDOP 정보
        data.hdop.hdop = _gps.hdop.hdop();
        data.hdop.ageMs = _gps.hdop.age();
        data.hdop.isValid = _gps.hdop.isValid();
        if (data.hdop.isValid) {
            data.hdop.accuracyLevel = getHdopAccuracyLevel(data.hdop.hdop);
        } else {
            data.hdop.accuracyLevel = "알 수 없음 (Not Available)";
        }

        // 진단 정보
        data.diagnostics.charsProcessed = _gps.charsProcessed();
        data.diagnostics.sentencesWithFix = _gps.sentencesWithFix();
        data.diagnostics.failedChecksum = _gps.failedChecksum();
        data.diagnostics.passedChecksum = _gps.passedChecksum();

        // 목적지 관련 정보
        data.destination.isSet = _isDestinationSet;
        data.destination.isValid = data.location.isValid && _isDestinationSet;

        if (data.destination.isValid) {
            data.destination.latitude = _destinationLat;
            data.destination.longitude = _destinationLng;
            data.destination.distanceKm = TinyGPSPlus::distanceBetween(
                                              data.location.latitude, data.location.longitude,
                                              _destinationLat, _destinationLng) / 1000.0;
            data.destination.courseToDeg = TinyGPSPlus::courseTo(
                                              data.location.latitude, data.location.longitude,
                                              _destinationLat, _destinationLng);
        } else {
            data.destination.latitude = 0.0;
            data.destination.longitude = 0.0;
            data.destination.distanceKm = 0.0;
            data.destination.courseToDeg = 0.0;
        }

        // 기타 상태
        data.hasFix = _gps.location.isValid();
    }

    // 모든 GPS 데이터 출력
    void printAllData(const T10_GPS_ALL_DATA &data) {
        Serial.println("--- GPS 모든 정보 출력 ---");

        Serial.println("[위치]");
        if (data.location.isValid) {
            Serial.printf("  위도: %.6f, 경도: %.6f\n", data.location.latitude, data.location.longitude);
            Serial.printf("  데이터 갱신 시간 (위치): %lu ms\n", data.location.ageMs);
        } else {
            Serial.println("  위치 데이터: 유효하지 않음");
        }

        Serial.println("[목적지와의 거리 및 방향]");
        if (data.destination.isSet) {
            if (data.destination.isValid) {
                Serial.printf("  목적지 (위도: %.6f, 경도: %.6f)까지 거리: %.2f km\n", 
                              data.destination.latitude, data.destination.longitude,
                              data.destination.distanceKm);
                Serial.printf("  목적지 방향: %.2f deg (%s)\n", data.destination.courseToDeg, TinyGPSPlus::cardinal(data.destination.courseToDeg));
            } else {
                Serial.println("  목적지 정보: 현재 위치 데이터가 유효하지 않아 계산 불가");
            }
        } else {
            Serial.println("  목적지: 설정되지 않음. setDestination() 함수를 호출하여 목적지를 설정하세요.");
        }

        Serial.println("[날짜]");
        if (data.date.isValid) {
            Serial.printf("  날짜: %04d-%02d-%02d\n", data.date.year, data.date.month, data.date.day);
            Serial.printf("  원시 날짜 값: %lu\n", data.date.value);
            Serial.printf("  데이터 갱신 시간 (날짜): %lu ms\n", data.date.ageMs);
        } else {
            Serial.println("  날짜 데이터: 유효하지 않음");
        }

        Serial.println("[시간]");
        if (data.time.isValid) {
            Serial.printf("  UTC 시간: %02d:%02d:%02d.%02d\n",
                          data.time.hour, data.time.minute, data.time.second, data.time.centisecond);
            Serial.printf("  원시 시간 값: %lu\n", data.time.value);
            Serial.printf("  데이터 갱신 시간 (시간): %lu ms\n", data.time.ageMs);
        } else {
            Serial.println("  시간 데이터: 유효하지 않음");
        }

        Serial.println("[속도]");
        if (data.speed.isValid) {
            Serial.printf("  m/s: %.2f, km/h: %.2f\n",
                          data.speed.mps, data.speed.kmph);
            Serial.printf("  데이터 갱신 시간 (속도): %lu ms\n", data.speed.ageMs);
        } else {
            Serial.println("  속도 데이터: 유효하지 않음");
        }

        Serial.println("[진행 방향]");
        if (data.course.isValid) {
            Serial.printf("  진행 방향: %.2f deg (%s)\n", data.course.degrees, TinyGPSPlus::cardinal(data.course.degrees));
            Serial.printf("  데이터 갱신 시간 (방향): %lu ms\n", data.course.ageMs);
        } else {
            Serial.println("  진행 방향 데이터: 유효하지 않음");
        }

        Serial.println("[고도]");
        if (data.altitude.isValid) {
            Serial.printf("  미터: %.2f, 킬로미터: %.2f\n",
                          data.altitude.meters, data.altitude.kilometers);
            Serial.printf("  데이터 갱신 시간 (고도): %lu ms\n", data.altitude.ageMs);
        } else {
            Serial.println("  고도 데이터: 유효하지 않음");
        }

        Serial.println("[위성 및 정밀도]");
        if (data.satellites.isValid) {
            Serial.printf("  수신 위성 수: %lu\n", data.satellites.value);
            Serial.printf("  데이터 갱신 시간 (위성): %lu ms\n", data.satellites.ageMs);
        } else {
            Serial.println("  위성 수 데이터: 유효하지 않음");
        }
        if (data.hdop.isValid) {
            Serial.printf("  HDOP: %.2f (정확도 수준: %s)\n", data.hdop.hdop, data.hdop.accuracyLevel.c_str());
            Serial.printf("  데이터 갱신 시간 (HDOP): %lu ms\n", data.hdop.ageMs);
        } else {
            Serial.println("  HDOP 데이터: 유효하지 않음");
        }
        
        Serial.println("[진단 정보]");
        Serial.printf("  처리된 문자 수: %lu\n", data.diagnostics.charsProcessed);
        Serial.printf("  Fix 있는 문장 수: %lu\n", data.diagnostics.sentencesWithFix);
        Serial.printf("  체크섬 실패 수: %lu\n", data.diagnostics.failedChecksum);
        Serial.printf("  체크섬 성공 수: %lu\n", data.diagnostics.passedChecksum);

        if (!data.hasFix) {
            Serial.println("  현재 Fix 상태: 없음 (위치 고정 대기 중)");
        } else {
            Serial.println("  현재 Fix 상태: 성공");
        }

        // 데이터 수신 경고 (_gps 객체를 직접 참조)
        if (_gps.charsProcessed() < 10) {
            Serial.println("  경고: GPS 모듈에서 데이터가 수신되지 않거나 매우 적습니다. 배선, 전원, 통신 속도를 확인하세요.");
        } else if (!_gps.location.isValid() && millis() - _gps.charsProcessed() > 5000) {
            Serial.println("  경고: GPS 신호가 약하거나 위치 고정이 안 됩니다.");
        }

        Serial.println("----------------------------------");
    }

    // 메인 루프에서 주기적으로 호출하여 데이터 처리 및 출력
    void run() {
        processData(); // 항상 데이터 파싱

        unsigned long currentTime = millis();
        if (currentTime - _lastGpsPrintTime >= GPS_PRINT_INTERVAL_MS) {
            _lastGpsPrintTime = currentTime; // 마지막 출력 시간 갱신

            T10_GPS_ALL_DATA currentGpsData;
            updateAllData(currentGpsData); // 현재 GPS 데이터를 구조체에 저장
            printAllData(currentGpsData);   // 구조체 내용 출력
        }
    }

private:
    HardwareSerial& _gpsSerial;
    long _baudRate;
    TinyGPSPlus _gps; // TinyGPSPlus 객체를 클래스 멤버 변수로 캡슐화
    unsigned long _lastGpsPrintTime;
    
    double _destinationLat;
    double _destinationLng;
    bool _isDestinationSet;

    // HDOP 정확도 수준을 반환하는 헬퍼 함수 (클래스 내부 함수로 변경)
    String getHdopAccuracyLevel(double hdop) {
        if      (hdop <= 1.0)  return "이상적인 (Excellent)";
        else if (hdop <= 2.0)  return "우수한 (Good)";
        else if (hdop <= 5.0)  return "적당한 (Moderate)";
        else if (hdop <= 10.0) return "보통의 (Fair)";
        else if (hdop <= 20.0) return "나쁜 (Poor)";
        else                   return "매우 나쁜 (Very Poor)";
    }
};

#endif // T10_GPSMODULE_CLASS_H
