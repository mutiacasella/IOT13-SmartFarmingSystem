/***************************************************
 * SMART FARM SYSTEM - Kelompok 13
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
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// ====== WiFi ======
char ssid[] = "Wi-Fi"; 
char pass[] = "password";

// ====== Pin Assignment ======
#define DHTPIN 15
#define DHTTYPE DHT22
#define SOIL_PIN 35     // pin ADC buat aslinya nanti
#define MQ_PIN 34
#define PUMP_LED 2
#define LOCAL_BUTTON 26

DHT dht(DHTPIN, DHTTYPE);

// ====== Threshold ======
int soilThreshold = 40;
float tempThreshold = 32.0;
float humThreshold = 40.0;
int mqThreshold = 1500;  // perkiraan, referensi pada pake ppm soalnya bukan adc, harus dihitung dulu entar

// ====== Irrigation Settings ======
unsigned long interval = 10000;  // 21600000 (tiap 6 jam)
unsigned long lastIrrigation = 0;

// ====== Task Management ======
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t controlTaskHandle = NULL;
TaskHandle_t blynkTaskHandle = NULL;
TaskHandle_t buttonTaskHandle = NULL;

// ====== Queue Management ======
QueueHandle_t sensorQueue = NULL;
QueueHandle_t irrigationQueue = NULL;

// ====== Mutex Synchronization ======
SemaphoreHandle_t pumpMutex = NULL;
SemaphoreHandle_t sensorMutex = NULL;
SemaphoreHandle_t blynkMutex = NULL;

// ====== Shared Data Structure ======
typedef struct {
  float temperature;
  float humidity;
  int soil;
  int mq;
} SensorData;

typedef struct {
  int triggerType;  // 1 = TIMER, 2 = SOIL, 3 = TOMBOL, 4 = BLYNK
  unsigned long time;
} IrrigationCommand;

// ====== Blynk Button State ======
bool blynkManual = false;

// Blynk V0 → manual control
BLYNK_WRITE(V0) {
  if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
    blynkManual = param.asInt();
    xSemaphoreGive(blynkMutex);
  }
}

void irrigate() {
  if (xSemaphoreTake(pumpMutex, portMAX_DELAY) == pdTRUE) {
    Serial.println(">>> AIR AIT AIR <<<");
    digitalWrite(PUMP_LED, HIGH);
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // Non-blocking delay
    digitalWrite(PUMP_LED, LOW);
    Serial.println(">>> IRIGASI SELESAI <<<");
    xSemaphoreGive(pumpMutex);
  }
}

int readSoil() {
  int raw = analogRead(SOIL_PIN);
  int moisture = map(raw, 4095, 1500, 0, 100); // 4095 = kering (di udara), 1500 = basah (di air)
  return constrain(moisture, 0, 100);
}

int readMQ() {
  int mq = analogRead(MQ_PIN);
  return mq;
}

// ====== TASK FUNCTIONS ======
void vSensorTask(void *pvParameter) {
  SensorData sensorData;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(1000);
  
  while (1) {
    if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
      sensorData.temperature = dht.readTemperature();
      sensorData.humidity = dht.readHumidity();
      xSemaphoreGive(sensorMutex);
    }
    
    sensorData.soil = readSoil();
    sensorData.mq = readMQ();
    
    // Kirim data sensor ke queue
    if (sensorQueue != NULL) {
      xQueueSend(sensorQueue, &sensorData, pdMS_TO_TICKS(100));
    }
    
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

void vControlTask(void *pvParameter) {
  SensorData sensorData;
  IrrigationCommand irrCmd;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(1000);
  
  while (1) {
    // Terima data sensor dari queue
    if (xQueueReceive(sensorQueue, &sensorData, 0) == pdTRUE) {
      Serial.println("\n========== MONITOR ==========");
      Serial.print("Suhu: "); Serial.println(sensorData.temperature);
      Serial.print("Kelembapan Udara: "); Serial.println(sensorData.humidity);
      Serial.print("Soil Moisture: "); Serial.println(sensorData.soil);
      Serial.print("Kualitas Udara: "); Serial.println(sensorData.mq);

      // ================ WARNING SYSTEM ======================
      if (sensorData.temperature > tempThreshold) {
        Serial.println("WARNING: Suhu terlalu tinggi!");
        if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
          Blynk.logEvent("warning_temp", "Suhu di atas batas!");
          xSemaphoreGive(blynkMutex);
        }
      }

      if (sensorData.humidity < humThreshold) {
        Serial.println(" WARNING: Kelembapan udara rendah!");
        if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
          Blynk.logEvent("warning_hum", "Kelembapan udara rendah!");
          xSemaphoreGive(blynkMutex);
        }
      }

      if (sensorData.mq > mqThreshold) {
        Serial.println("WARNING: Kualitas udara buruk!");
        if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
          Blynk.logEvent("warning_mq", "Kualitas udara buruk!");
          xSemaphoreGive(blynkMutex);
        }
      }

      // ================ IRRIGATION CONTROL ==================
      if (millis() - lastIrrigation >= interval) {
        irrCmd.triggerType = 1; // TIMER
        irrCmd.time = millis();
        xQueueSend(irrigationQueue, &irrCmd, pdMS_TO_TICKS(100));
      }

      if (sensorData.soil < soilThreshold) {
        irrCmd.triggerType = 2; // SOIL DI BAWAH THRESHOLD
        irrCmd.time = millis();
        xQueueSend(irrigationQueue, &irrCmd, pdMS_TO_TICKS(100));
      }
    }
    
    // Proses command irigasi dari queue
    if (xQueueReceive(irrigationQueue, &irrCmd, 0) == pdTRUE) {
      switch (irrCmd.triggerType) {
        case 1:
          Serial.println("Pemicu: TIMER");
          irrigate();
          lastIrrigation = irrCmd.time;
          break;
        case 2:
          Serial.println("Pemicu: SOIL DI BAWAH THRESHOLD");
          irrigate();
          lastIrrigation = irrCmd.time;
          break;
        case 3:
          Serial.println("Pemicu: TOMBOL LOKAL");
          irrigate();
          lastIrrigation = irrCmd.time;
          break;
        case 4:
          Serial.println("Pemicu: BLYNK BUTTON");
          irrigate();
          lastIrrigation = irrCmd.time;
          break;
      }
    }
    
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

void vButtonTask(void *pvParameter) {
  IrrigationCommand irrCmd;
  bool lastButtonState = HIGH;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(50); // Debounce 50ms
  
  while (1) {
    bool currentButtonState = digitalRead(LOCAL_BUTTON);
    
    // Deteksi falling edge (tekan tombol)
    if (lastButtonState == HIGH && currentButtonState == LOW) {
      Serial.println("Pemicu: TOMBOL LOKAL");
      irrCmd.triggerType = 3; // Tombol Lokal
      irrCmd.time = millis();
      xQueueSend(irrigationQueue, &irrCmd, pdMS_TO_TICKS(100));
      vTaskDelay(300 / portTICK_PERIOD_MS); // Debounce delay
    }
    
    lastButtonState = currentButtonState;
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

void vBlynkTask(void *pvParameter) {
  SensorData sensorData;
  bool localBlynkManual = false;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(2000);
  
  while (1) {
    // Cek Blynk manual
    if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
      if (blynkManual) {
        localBlynkManual = true;
        blynkManual = false;
      }
      xSemaphoreGive(blynkMutex);
    }
    
    // Proses Blynk manual di luar mutex
    if (localBlynkManual) {
      Serial.println("Pemicu: BLYNK BUTTON");
      localBlynkManual = false;
      
      IrrigationCommand irrCmd;
      irrCmd.triggerType = 4; // BLYNK BUTTON
      irrCmd.time = millis();
      xQueueSend(irrigationQueue, &irrCmd, pdMS_TO_TICKS(100));
      
      // Kirim update ke Blynk
      if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
        Blynk.virtualWrite(V0, 0);
        xSemaphoreGive(blynkMutex);
      }
    }
    
    // Kirim data sensor ke Blynk
    if (xQueueReceive(sensorQueue, &sensorData, 0) == pdTRUE) {
      if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
        Blynk.virtualWrite(V1, sensorData.temperature);
        Blynk.virtualWrite(V2, sensorData.humidity);
        Blynk.virtualWrite(V3, sensorData.soil);
        Blynk.virtualWrite(V4, sensorData.mq);
        xSemaphoreGive(blynkMutex);
      }
    }
    
    // Run Blynk
    if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
      Blynk.run();
      xSemaphoreGive(blynkMutex);
    }
    
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Tunggu serial ready

  pinMode(PUMP_LED, OUTPUT);
  digitalWrite(PUMP_LED, LOW);  // pompa mati saat startup
  
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

  // ====== INITIALIZE RTOS COMPONENTS ======
  // Create queues
  sensorQueue = xQueueCreate(10, sizeof(SensorData));
  irrigationQueue = xQueueCreate(10, sizeof(IrrigationCommand));
  
  // Create mutexes
  pumpMutex = xSemaphoreCreateMutex();
  sensorMutex = xSemaphoreCreateMutex();
  blynkMutex = xSemaphoreCreateMutex();
  
  // Create tasks
  xTaskCreatePinnedToCore(vSensorTask, "SensorTask", 4096, NULL, 2, &sensorTaskHandle, 0);
  xTaskCreatePinnedToCore(vControlTask, "ControlTask", 4096, NULL, 3, &controlTaskHandle, 0);
  xTaskCreatePinnedToCore(vButtonTask, "ButtonTask", 2048, NULL, 1, &buttonTaskHandle, 1);
  xTaskCreatePinnedToCore(vBlynkTask, "BlynkTask", 4096, NULL, 2, &blynkTaskHandle, 1);
  
  Serial.println("RTOS Tasks Initialized!");
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}