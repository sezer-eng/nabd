// نبض: ESP32 + MAX30102 + LCD I2C 16x2 + MQTT
// المكتبات (Library Manager):
//  - SparkFun MAX3010x Pulse and Proximity Sensor Library
//  - LiquidCrystal I2C (Frank de Brabander)
//  - PubSubClient (Nick O'Leary)
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

// ===== غيّر هذه القيم =====
const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_PASSWORD";
const char* MQTT_HOST = "broker.hivemq.com";
const int   MQTT_PORT = 1883;
const char* TOPIC     = "nabd/dshbcy";   // نفس القناة بالموقع حرفياً
// ==========================

LiquidCrystal_I2C lcd(0x27, 16, 2);      // جرّب 0x3F إذا ما اشتغلت
MAX30105 sensor;
WiFiClient net;
PubSubClient mqtt(net);

const int N = 100;                       // 4 ثواني عند 25 عينة/ثانية
uint32_t irBuf[N], redBuf[N];
int32_t spo2 = 0, hr = 0;
int8_t validSpo2 = 0, validHr = 0;
int shownBpm = 0, shownSpo2 = 0;

void wifiConnect() {
  lcd.clear(); lcd.print("WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(300);
}

void mqttConnect() {
  while (!mqtt.connected()) {
    String id = "nabd_" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (!mqtt.connect(id.c_str())) delay(1500);
  }
}

void lcdShow(bool finger) {
  lcd.clear();
  if (!finger) { lcd.print("Place finger"); lcd.setCursor(0, 1); lcd.print("on sensor"); return; }
  lcd.print("BPM: ");  lcd.print(shownBpm ? String(shownBpm) : "--");
  lcd.setCursor(0, 1);
  lcd.print("SpO2: "); lcd.print(shownSpo2 ? String(shownSpo2) : "--"); lcd.print("%");
}

void publishPacket(uint32_t* ir, int count, bool finger) {
  String j = "{\"bpm\":" + String(finger ? shownBpm : 0) +
             ",\"spo2\":" + String(finger ? shownSpo2 : 0) + ",\"ir\":[";
  for (int i = 0; i < count; i++) { j += String(ir[i]); if (i < count - 1) j += ","; }
  j += "]}";
  mqtt.publish(TOPIC, j.c_str());
}

// يقرأ عينة واحدة (ينتظر حتى تتوفر)
void readSample(uint32_t &ir, uint32_t &red) {
  while (!sensor.available()) sensor.check();
  red = sensor.getRed(); ir = sensor.getIR(); sensor.nextSample();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  lcd.init(); lcd.backlight(); lcd.print("Nabd starting");
  if (!sensor.begin(Wire, I2C_SPEED_FAST)) {
    lcd.clear(); lcd.print("Sensor error");
    while (1) delay(1000);
  }
  // brightness, avg, mode(2=red+ir), sampleRate, pulseWidth, adcRange
  sensor.setup(60, 4, 2, 100, 411, 4096);
  wifiConnect();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(512);
  mqttConnect();
  lcd.clear(); lcd.print("Ready");
  for (int i = 0; i < N; i++) readSample(irBuf[i], redBuf[i]);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) wifiConnect();
  if (!mqtt.connected()) mqttConnect();
  mqtt.loop();

  // أزح آخر 75 عينة للبداية، واقرأ 25 جديدة (ثانية واحدة)
  for (int i = 25; i < N; i++) { irBuf[i - 25] = irBuf[i]; redBuf[i - 25] = redBuf[i]; }
  for (int i = 75; i < N; i += 5) {
    for (int k = i; k < i + 5; k++) readSample(irBuf[k], redBuf[k]);
    bool f = irBuf[N - 1] > 50000;
    publishPacket(&irBuf[i], 5, f);      // حزمة 5 عينات للموجة
    mqtt.loop();
  }

  bool finger = irBuf[N - 1] > 50000;
  if (finger) {
    maxim_heart_rate_and_oxygen_saturation(irBuf, N, redBuf, &spo2, &validSpo2, &hr, &validHr);
    if (validHr && hr > 40 && hr < 200) shownBpm = hr;
    if (validSpo2 && spo2 > 70 && spo2 <= 100) shownSpo2 = spo2;
  } else { shownBpm = 0; shownSpo2 = 0; }
  lcdShow(finger);
}
