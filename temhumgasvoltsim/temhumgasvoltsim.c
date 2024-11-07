// 온도, 습도, 가스농도, 전압, 심장박동 측정 아두이노 코드 

#include <PulseSensorPlayground.h>   // PulseSensorPlayground 라이브러리 포함
#include <MQUnifiedsensor.h>         // MQ-5 가스 센서 라이브러리 포함
#include <DHT.h>                     // DHT11 온습도 센서 라이브러리 포함
#include <DHT_U.h>

// PulseSensor 정의
const int PulseWire = 2;       // PulseSensor의 보라색 와이어를 아날로그 핀 2에 연결
const int LED = LED_BUILTIN;   // 아두이노 내장 LED 핀, 보통 핀 13에 가까이 있음
const int VIBRATION_PIN = 3;   // 진동 모듈 핀
int Threshold = 550;           // 심박으로 인식할 신호의 임계값 설정

PulseSensorPlayground pulseSensor;   // PulseSensor 객체 생성

// MQ-5 정의
#define placa "Arduino UNO"
#define Voltage_Resolution 5
#define MQ5_pin A0                  // MQ-5 센서의 아날로그 입력 핀
#define MQ5_type "MQ-5"
#define ADC_Bit_Resolution 10
#define RatioMQ6CleanAir 10

MQUnifiedsensor MQ5(placa, Voltage_Resolution, ADC_Bit_Resolution, MQ5_pin, MQ5_type);

// DHT11 정의
#define DHTPIN 4                    // DHT11 데이터 핀
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// 전압 측정에 사용될 저항 값
#define R1 220.0
#define R2 4700.0

unsigned long previousMillis = 0;  // 이전 시간 추적 변수
int seconds = 0;                   // 초 단위 시간 변수

void setup() {
  Serial.begin(9600);  // 시리얼 모니터 시작

  // 진동 모듈 핀 초기화
  pinMode(VIBRATION_PIN, OUTPUT);
  digitalWrite(VIBRATION_PIN, LOW); // 초기 상태는 OFF

  // PulseSensor 초기 설정
  pulseSensor.analogInput(PulseWire);
  pulseSensor.blinkOnPulse(LED);
  pulseSensor.setThreshold(Threshold);
  
  if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");
  }

  // DHT11 초기 설정
  dht.begin();
  
  // MQ-5 초기 설정
  MQ5.setRegressionMethod(1); // _PPM = a * ratio^b
  MQ5.setA(2127.2); 
  MQ5.setB(-2.526);
  MQ5.init();

  // MQ-5 보정
  Serial.print("Calibrating MQ-5, please wait.");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ5.update();
    calcR0 += MQ5.calibrate(RatioMQ6CleanAir);
    Serial.print(".");
  }
  MQ5.setR0(calcR0 / 10);
  Serial.println(" done!");

  if (isinf(calcR0)) {
    Serial.println("Warning: Connection issue, R0 is infinite (open circuit detected), please check wiring.");
    while (1);
  }
  if (calcR0 == 0) {
    Serial.println("Warning: Connection issue, R0 is zero (short circuit detected), please check wiring.");
    while (1);
  }

  MQ5.serialDebug(true); // MQ-5 시리얼 디버그 활성화
}

void loop() {
  unsigned long currentMillis = millis();

  // 1초마다 seconds 증가
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    seconds++;
  }

  // 심박수 읽기
  if (pulseSensor.sawStartOfBeat()) {
    int myBPM = pulseSensor.getBeatsPerMinute();
    Serial.print("BPM:");
    Serial.print(myBPM);
    Serial.print(", ");
    
    // 심박수 조건 확인 (40bpm 이하 또는 120bpm 이상)
    if (myBPM < 40 || myBPM > 120) {
      delay(10000); // 10초 동안 조건 확인
      if (myBPM < 40 || myBPM > 120) {
        digitalWrite(VIBRATION_PIN, HIGH); // 진동 모듈 켜기
      }
    }
  }
  
  // MQ-5 센서 데이터 읽기
  MQ5.update();
  float ppm = MQ5.readSensor();
  Serial.print("ppm:");
  Serial.print(ppm);
  Serial.print(", ");
  
  // DHT11 온습도 데이터 읽기
  float temp = dht.readTemperature();
  float humi = dht.readHumidity();
  
  if (isnan(humi) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(1000);
    return;
  }
  
  Serial.print("temp:");
  Serial.print(temp);
  Serial.print(", humi:");
  Serial.print(humi);
  Serial.print(", ");
  
  // 전압 측정
  int data = analogRead(A1); // A1 핀에서 아날로그 값 읽기
  float volt = (5 * data / 1024.0) / (R2 / (R1 + R2));
  Serial.print("volt:");
  Serial.print(volt);
  Serial.print(", ");

  // 조건 확인 및 진동 모듈 활성화
  if (temp > 60 || ppm > 1000 || humi > 85 || volt > 4.3 || volt < 2.5) {
    digitalWrite(VIBRATION_PIN, HIGH); // 진동 모듈 켜기
  } else {
    digitalWrite(VIBRATION_PIN, LOW); // 진동 모듈 끄기
  }

  // 초 단위 시간 출력
  Serial.print("time:");
  Serial.println(seconds);

  // 샘플링 간격
  delay(1000); // 1초 간격으로 데이터 읽기
}
