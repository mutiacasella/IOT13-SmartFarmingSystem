/***************************************************
 * SMART FARM SYSTEM - Kelompok 13
 * Fitur:
 *  - Penyiraman interval timer (SOFTWARE TIMER)
 *  - Penyiraman berdasarkan soil threshold
 *  - Penyiraman manual dari tombol & Blynk
 *  - Warning suhu, kelembapan, kualitas udara
 *  - BLE Server untuk monitoring sensor 
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
#include <freertos/timers.h>
#include <BLEDevice.h>       
#include <BLEServer.h>       
#include <BLEUtils.h>        
#include <BLE2902.h>         

// ====== WiFi ====== (for Blynk)
char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";


// ====== Pin Assignment ======
#define DHTPIN 15
#define DHTTYPE DHT22
#define SOIL_PIN 35
#define MQ_PIN 34
#define PUMP_LED 2
#define LOCAL_BUTTON 26

DHT dht(DHTPIN, DHTTYPE);

// ====== Threshold ======
int soilThreshold = 40;
float tempThreshold = 32.0;
float humThreshold = 40.0;
int mqThreshold = 1500;

// ====== Irrigation Settings ======
unsigned long interval = 10000;  // 10 detik untuk testing

// ====== BLE Configuration ======  
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHAR_TEMP_UUID      "12345678-1234-1234-1234-123456789ab1"
#define CHAR_HUM_UUID       "12345678-1234-1234-1234-123456789ab2"
#define CHAR_SOIL_UUID      "12345678-1234-1234-1234-123456789ab3"
#define CHAR_MQ_UUID        "12345678-1234-1234-1234-123456789ab4"

BLEServer* pServer = NULL;
BLECharacteristic* pCharTemp = NULL;
BLECharacteristic* pCharHum = NULL;
BLECharacteristic* pCharSoil = NULL;
BLECharacteristic* pCharMQ = NULL;
bool bleDeviceConnected = false;

// ====== Task Management ======
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t controlTaskHandle = NULL;
TaskHandle_t blynkTaskHandle = NULL;
TaskHandle_t buttonTaskHandle = NULL;
TaskHandle_t bleTaskHandle = NULL;  

// ====== Queue Management ======
QueueHandle_t sensorQueue = NULL;
QueueHandle_t irrigationQueue = NULL;

// ====== Mutex Synchronization ======
SemaphoreHandle_t pumpMutex = NULL;
SemaphoreHandle_t sensorMutex = NULL;
SemaphoreHandle_t blynkMutex = NULL;
SemaphoreHandle_t bleMutex = NULL; 

// ====== Software Timer for Irrigation ======
TimerHandle_t irrigationTimer = NULL;

// ====== Shared Data Structure ======
typedef struct {
  float temperature;
  float humidity;
  int soil;
  int mq;
} SensorData;

typedef struct {
  int triggerType;
  unsigned long time;
} IrrigationCommand;

// ====== Blynk Button State ======
bool blynkManual = false;

// ====== Circular Buffer for Sensor Data ======
#define BUFFER_SIZE 5
SensorData sensorBuffer[BUFFER_SIZE];
int bufferIndex = 0;

// ====== BLE Server Callbacks ======  
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    bleDeviceConnected = true;
    Serial.println("BLE Client Connected!");
  }
  
  void onDisconnect(BLEServer* pServer) {
    bleDeviceConnected = false;
    Serial.println("BLE Client Disconnected!");
    // Auto restart advertising
    pServer->startAdvertising();
  }
};

// ====== SOFTWARE TIMER CALLBACK ======
void irrigationTimerCallback(TimerHandle_t xTimer) {
  IrrigationCommand irrCmd;
  irrCmd.triggerType = 1;
  irrCmd.time = millis();
  
  if (xQueueSend(irrigationQueue, &irrCmd, 0) != pdPASS) {
    Serial.println("WARNING: Irrigation queue penuh dari Timer!");
  }
}

// Blynk V0 → manual control
BLYNK_WRITE(V0) {
  if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
    blynkManual = param.asInt();
    xSemaphoreGive(blynkMutex);
  }
}

void irrigate() {
  if (xSemaphoreTake(pumpMutex, portMAX_DELAY) == pdTRUE) {
    Serial.println(">>> AIR AIR AIR <<<");
    digitalWrite(PUMP_LED, HIGH);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    digitalWrite(PUMP_LED, LOW);
    Serial.println(">>> IRIGASI SELESAI <<<");
    xSemaphoreGive(pumpMutex);
  }
}

int readSoil() {
  int raw = analogRead(SOIL_PIN);
  int moisture = map(raw, 4095, 1500, 0, 100);
  return constrain(moisture, 0, 100);
}

int readMQ() {
  int mq = analogRead(MQ_PIN);
  return mq;
}

// ====== TASK FUNCTIONS ======
void vSensorTask(void *pvParameter) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(1000);
  
  while (1) {
    SensorData sensorData;
    
    if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
      sensorData.temperature = dht.readTemperature();
      sensorData.humidity = dht.readHumidity();
      xSemaphoreGive(sensorMutex);
    }
    
    sensorData.soil = readSoil();
    sensorData.mq = readMQ();
    
    if (xQueueSend(sensorQueue, &sensorData, pdMS_TO_TICKS(100)) != pdPASS) {
      Serial.println("WARNING: Sensor queue penuh!");
    }
    
    if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
      sensorBuffer[bufferIndex] = sensorData;
      bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
      xSemaphoreGive(blynkMutex);
    }
    
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

void vControlTask(void *pvParameter) {
  SensorData sensorData;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(1000);
  
  while (1) {
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
      if (sensorData.soil < soilThreshold) {
        IrrigationCommand irrCmd;
        irrCmd.triggerType = 2;
        irrCmd.time = millis();
        
        if (xQueueSend(irrigationQueue, &irrCmd, pdMS_TO_TICKS(100)) != pdPASS) {
          Serial.println("WARNING: Irrigation queue penuh!");
        }
      }
    }
    
    IrrigationCommand irrCmd;
    if (xQueueReceive(irrigationQueue, &irrCmd, 0) == pdTRUE) {
      switch (irrCmd.triggerType) {
        case 1:
          Serial.println("Pemicu: SOFTWARE TIMER");
          irrigate();
          break;
        case 2:
          Serial.println("Pemicu: SOIL DI BAWAH THRESHOLD");
          irrigate();
          break;
        case 3:
          Serial.println("Pemicu: TOMBOL LOKAL");
          irrigate();
          break;
        case 4:
          Serial.println("Pemicu: BLYNK BUTTON");
          irrigate();
          break;
      }
    }
    
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

void vButtonTask(void *pvParameter) {
  bool lastButtonState = HIGH;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(50);
  
  while (1) {
    bool currentButtonState = digitalRead(LOCAL_BUTTON);
    
    if (lastButtonState == HIGH && currentButtonState == LOW) {
      IrrigationCommand irrCmd;
      irrCmd.triggerType = 3;
      irrCmd.time = millis();
      
      if (xQueueSend(irrigationQueue, &irrCmd, pdMS_TO_TICKS(100)) != pdPASS) {
        Serial.println("WARNING: Irrigation queue penuh!");
      }
      vTaskDelay(300 / portTICK_PERIOD_MS);
    }
    
    lastButtonState = currentButtonState;
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

void vBlynkTask(void *pvParameter) {
  bool localBlynkManual = false;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(2000);
  
  while (1) {
    if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
      if (blynkManual) {
        localBlynkManual = true;
        blynkManual = false;
      }
      xSemaphoreGive(blynkMutex);
    }
    
    if (localBlynkManual) {
      localBlynkManual = false;
      
      IrrigationCommand irrCmd;
      irrCmd.triggerType = 4;
      irrCmd.time = millis();
      
      if (xQueueSend(irrigationQueue, &irrCmd, pdMS_TO_TICKS(100)) != pdPASS) {
        Serial.println("WARNING: Irrigation queue full!");
      }
      
      if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
        Blynk.virtualWrite(V0, 0);
        xSemaphoreGive(blynkMutex);
      }
    }
    
    if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
      int latestIndex = (bufferIndex == 0) ? BUFFER_SIZE - 1 : bufferIndex - 1;
      Blynk.virtualWrite(V1, sensorBuffer[latestIndex].temperature);
      Blynk.virtualWrite(V2, sensorBuffer[latestIndex].humidity);
      Blynk.virtualWrite(V3, sensorBuffer[latestIndex].soil);
      Blynk.virtualWrite(V4, sensorBuffer[latestIndex].mq);
      xSemaphoreGive(blynkMutex);
    }
    
    if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
      Blynk.run();
      xSemaphoreGive(blynkMutex);
    }
    
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

// ====== BLE TASK ======  
void vBLETask(void *pvParameter) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(2000);  // Update setiap 2 detik
  
  while (1) {
    // Kirim data sensor via BLE jika ada client yang connect
    if (bleDeviceConnected && xSemaphoreTake(bleMutex, portMAX_DELAY) == pdTRUE) {
      // Ambil data terbaru dari buffer
      if (xSemaphoreTake(blynkMutex, portMAX_DELAY) == pdTRUE) {
        int latestIndex = (bufferIndex == 0) ? BUFFER_SIZE - 1 : bufferIndex - 1;
        SensorData data = sensorBuffer[latestIndex];
        xSemaphoreGive(blynkMutex);
        
        // Update BLE characteristics
        String tempStr = String(data.temperature, 1) + " C";
        String humStr = String(data.humidity, 1) + " %";
        String soilStr = String(data.soil) + " %";
        String mqStr = String(data.mq);
        
        pCharTemp->setValue(tempStr.c_str());
        pCharTemp->notify();
        
        pCharHum->setValue(humStr.c_str());
        pCharHum->notify();
        
        pCharSoil->setValue(soilStr.c_str());
        pCharSoil->notify();
        
        pCharMQ->setValue(mqStr.c_str());
        pCharMQ->notify();
        
        Serial.println("BLE: Data sent to client");
      }
      xSemaphoreGive(bleMutex);
    }
    
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PUMP_LED, OUTPUT);
  digitalWrite(PUMP_LED, LOW);
  
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

  // ====== INITIALIZE BLE ======  
  Serial.println("Initializing BLE...");
  BLEDevice::init("SmartFarm_K13");  // Nama BLE device

  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Create BLE Characteristics
  pCharTemp = pService->createCharacteristic(
    CHAR_TEMP_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharTemp->addDescriptor(new BLE2902());
  
  pCharHum = pService->createCharacteristic(
    CHAR_HUM_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharHum->addDescriptor(new BLE2902());
  
  pCharSoil = pService->createCharacteristic(
    CHAR_SOIL_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharSoil->addDescriptor(new BLE2902());
  
  pCharMQ = pService->createCharacteristic(
    CHAR_MQ_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharMQ->addDescriptor(new BLE2902());
  
  // Start service
  pService->start();
  
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE Ready! Device name: SmartFarm_K13");

  // ====== INITIALIZE RTOS COMPONENTS ======
  sensorQueue = xQueueCreate(10, sizeof(SensorData));
  irrigationQueue = xQueueCreate(10, sizeof(IrrigationCommand));
  
  pumpMutex = xSemaphoreCreateMutex();
  sensorMutex = xSemaphoreCreateMutex();
  blynkMutex = xSemaphoreCreateMutex();
  bleMutex = xSemaphoreCreateMutex();  
  
  for (int i = 0; i < BUFFER_SIZE; i++) {
    sensorBuffer[i].temperature = 0.0;
    sensorBuffer[i].humidity = 0.0;
    sensorBuffer[i].soil = 0;
    sensorBuffer[i].mq = 0;
  }

  irrigationTimer = xTimerCreate(
    "IrrigationTimer",
    pdMS_TO_TICKS(interval),
    pdTRUE,
    (void *)0,
    irrigationTimerCallback
  );
  
  if (irrigationTimer != NULL) {
    xTimerStart(irrigationTimer, 0);
    Serial.println("Software Timer Started!");
  }
  
  // Create tasks
  xTaskCreatePinnedToCore(vSensorTask, "SensorTask", 4096, NULL, 2, &sensorTaskHandle, 0);
  xTaskCreatePinnedToCore(vControlTask, "ControlTask", 4096, NULL, 3, &controlTaskHandle, 0);
  xTaskCreatePinnedToCore(vButtonTask, "ButtonTask", 2048, NULL, 1, &buttonTaskHandle, 1);
  xTaskCreatePinnedToCore(vBlynkTask, "BlynkTask", 4096, NULL, 2, &blynkTaskHandle, 1);
  xTaskCreatePinnedToCore(vBLETask, "BLETask", 4096, NULL, 2, &bleTaskHandle, 0); 
  
  Serial.println("RTOS Tasks Initialized!");
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}