/***************************************************
 * SMART FARM SYSTEM - Kelompok ....
 * Fitur:
 *  - Penyiraman interval timer
 *  - Penyiraman berdasarkan soil threshold
 *  - Penyiraman manual dari tombol & Blynk
 *  - Warning suhu, kelembapan, kualitas udara
 ***************************************************/

#define BLYNK_TEMPLATE_ID "TMPL6iLhzYrwu"
#define BLYNK_TEMPLATE_NAME "ProyekAkhir"
#define BLYNK_AUTH_TOKEN "mgDJ5ZeVSOWgJtP-XtuUHCConoORnOY8"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// ====== WiFi ======
char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";

// ====== Pin Assignment ======
#define DHTPIN 15
#define DHTTYPE DHT22
#define SOIL_PIN 4
#define MQ_PIN 34
#define PUMP_LED 2
#define LOCAL_BUTTON 26

DHT dht(DHTPIN, DHTTYPE);

// ====== Threshold ======
int soilThreshold = 30;
float tempThreshold = 32.0; 
float humThreshold = 40.0;
int mqThreshold = 300;     // Nggak tau, deng. Angka asal ini

// ====== Irrigation Settings ======
unsigned long interval = 10000;
unsigned long lastIrrigation = 0;

// ====== Blynk Button State ======
bool blynkManual = false;

// Blynk V0 → manual control
BLYNK_WRITE(V0) {
  blynkManual = param.asInt();
}

void irrigate() {
  Serial.println(">>> AIR AIT AIR <<<");
  digitalWrite(PUMP_LED, HIGH);
  delay(2000);
  digitalWrite(PUMP_LED, LOW);
  Serial.println(">>> IRIGASI SELESAI <<<");
}

int readSoil() {
  int raw = analogRead(SOIL_PIN);
  int moisture = map(raw, 4095, 0, 0, 100);
  return constrain(moisture, 0, 100);
}

int readMQ() {
  int mq = analogRead(MQ_PIN);
  return mq;
}

void setup() {
  Serial.begin(115200);

  pinMode(PUMP_LED, OUTPUT);
  pinMode(LOCAL_BUTTON, INPUT_PULLUP);

  dht.begin();

  // --- WIFI ---
  Serial.print("WiFi, Wifi, ");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n WiFi Connected!");
  Serial.println(WiFi.localIP());

  // --- BLYNK ---
  Serial.print("Menghubungkan ke Blynk");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("\n Blynk Connected!");
}

void loop() {
  Blynk.run();

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int soil = readSoil();
  int mq = readMQ();

  Serial.println("\n========== MONITOR ==========");
  Serial.print("Suhu: "); Serial.println(temperature);
  Serial.print("Kelembapan Udara: "); Serial.println(humidity);
  Serial.print("Soil Moisture: "); Serial.println(soil);
  Serial.print("Kualitas Udara: "); Serial.println(mq);

  // ================ WARNING SYSTEM ======================
  if (temperature > tempThreshold) {
    Serial.println("WARNING: Suhu terlalu tinggi!");
    Blynk.logEvent("warning_temp", "Suhu di atas batas!");
  }

  if (humidity < humThreshold) {
    Serial.println(" WARNING: Kelembapan udara rendah!");
    Blynk.logEvent("warning_hum", "Kelembapan udara rendah!");
  }

  if (mq > mqThreshold) {
    Serial.println("WARNING: Kualitas udara buruk!");
    Blynk.logEvent("warning_mq", "Kualitas udara buruk!");
  }

  // ================ IRRIGATION CONTROL ==================
  if (millis() - lastIrrigation >= interval) {
    Serial.println("Pemicu: TIMER");
    irrigate();
    lastIrrigation = millis();
  }

  if (soil < soilThreshold) {
    Serial.println("Pemicu: SOIL DI BAWAH THRESHOLD");
    irrigate();
    lastIrrigation = millis();
  }

  if (digitalRead(LOCAL_BUTTON) == LOW) {
    Serial.println("Pemicu: TOMBOL LOKAL");
    irrigate();
  }

  if (blynkManual) {
    Serial.println("Pemicu: BLYNK BUTTON");
    irrigate();
    blynkManual = false;
    Blynk.virtualWrite(V0, 0);
  }

  // SEND TO BLYNK
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
  Blynk.virtualWrite(V3, soil);
  Blynk.virtualWrite(V4, mq);

  delay(1000);
}
