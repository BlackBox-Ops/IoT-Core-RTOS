#include <OneWire.h>
#include <DallasTemperature.h>

// ============================= KONFIGURASI =====================================
#define ONE_WIRE_BUS      4    // Pin GPIO untuk bus onewire 
#define NUM_SENSORS       5    // Jumlah maksimal sensor DS18B20
#define READ_INTERVAL     200  // Interval pembacaan loop DS18B20
#define PER_SENSOR_TIME   40   // Target waktu per sensor untuk baca & kirim (ms)
#define TOLERANCE_MS      5    // Batas toleransi timing (ms)

// Struktur data untuk pembacaabn sensor yang dikirim antar task 
typedef struct {
  uint8_t  id;          // ID Sensor (0-4)
  float temp;           // Temperature yang terbaca (Celcius)
  uint32_t timestamp;   // Timestamp saat pembacaan (ms)
} ReadingData_t;


// ============================= VARIABEL GLOBAL ==================================
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
QueueHandle_t dataQueue;
DeviceAddress sensorAddresses[NUM_SENSORS];
uint8_t numValidSensors = 0;

// ============================= FUNGSI HELPER =====================================
/**
 * Inisialisasi Sensor DS18B20
 * return : Jika minimal 1 sensor terdeteksi, false jika tidak ada 
 */

bool initSensors() {
  sensors.begin();

  // Mode non-blocking untuk konversi asinkron
  sensors.setWaitForConversion(false);

  // Set resolusi 9-bit untuk konversi cepat yang terhubung 
  sensors.setResolution(9);

  // Deteksi dan simpan , alamat semua sensor yang terhubung 
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
      if (sensors.getAddress(sensorAddresses[i], i)) {
        numValidSensors++;
        Serial.printf("sensor %d terdeteksi\n", i);
      } else {
        Serial.printf("sesnor %d tidak ditemukan!\n", i);
      }
  }
  return numValidSensors > 0;
}


// ============================= TASK PEMBACAAN SENSOR =====================================

/**
 * Task untuk membaca semua sensor secara periodik 
 * Berjalan di Core 0 dengan prioritas tinggi  
 */
void taskReadSensors(void *parameter) {
  TickType_t lastWakeTime = xTaskGetTickCount();

  for(;;) {
    uint32_t startLoopTime = millis();

    // Loop untuk setiap sensor yang valid 
    for (uint8_t i = 0; i < numValidSensors; i++) {
      uint32_t startSensorTime = millis();

      // Request pembacaan suhu untuk sensor tertentu secara individual
      sensors.requestTemperaturesByAddress(sensorAddresses[i]);

      // Tunggu Konversi selesai dengan polling (non-blocking)
      while (!sensors.isConversionComplete()) {
        // Timeout jika melebihi waktu target + toleransi
        if (millis() - startSensorTime > PER_SENSOR_TIME + TOLERANCE_MS) {
          Serial.printf("Sensor %d timeout konversi!\n", i);
          break;
        } 
        // Delay agar CPU core 0 tidak terbebani
        vTaskDelay(pdMS_TO_TICKS(5));
      }

      // Ambil hasil pembacaan temperature
      float temperature = sensors.getTempC(sensorAddresses[i]);

      if (temperature != DEVICE_DISCONNECTED_C) {
          ReadingData_t sensorData = {
            .id = i,
            .temp = temperature,
            .timestamp = millis()
          };
          xQueueSend(dataQueue, &sensorData, pdMS_TO_TICKS(10));
      }
 
      // Kontrol Timing per sensor
      uint32_t elapsed = millis() - startSensorTime;
      if (i < numValidSensors - 1) {
        int32_t waitTime = PER_SENSOR_TIME - elapsed;
        if (waitTime > 0) vTaskDelay(pdMS_TO_TICKS(waitTime));
      }
    }
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(READ_INTERVAL));
  }
}

// ==================== TASK TRANSMISI DATA (Core 1) ====================
/**
 * Task untuk menerima dan mengirimkan data sensor via Serial atau (Console)
 * Berjalan di core 1 dengan prioritas sedang 
 */
void taskTransmit(void *parameter) {
  ReadingData_t receivedData;

    for(;;) {
      // Terima data dari queue dengan toleransi maksimal 5ms
      if (xQueueReceive(dataQueue, &receivedData, pdMS_TO_TICKS(5))) {
         // Hitung latensi dari waktu pembacaan hingga transmisi
         uint32_t latencyMs = millis() - receivedData.timestamp;

         // Kirim data ke Serial Monitor
         Serial.printf("ID: %d\t Temp: %.2f°C\t Latency: %3d ms\t [Core %d]\n", 
              receivedData.id,   // data id
              receivedData.temp, // data temperarture
              latencyMs,         // data latensi pengiriman per sensor
              xPortGetCoreID()); // data core id esp 32 
      }
    }
}


// ==================== SETUP ====================
/**
 * Fungsi setup - dijalankan sekali saat startup 
 */
void setup() {
  // inisialisasi serial untuk monitoring 
  Serial.begin(9600);
  Serial.println("Inisialisasi sistem");

  // Inisialisasi sensor DS18B20
  if (!initSensors()) {
    Serial.println("ERROR: Tidak ada sensor valid! Program dihentikan");
    while(true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  Serial.printf("Total sensor yang terdeteksi: %d\n", numValidSensors);

  // Buat queue untuk komunikasi antar task
  dataQueue = xQueueCreate(10, sizeof(ReadingData_t));

  if (dataQueue != NULL) {
      // Buat task pembacaan sensor di Core 0 dengan (pioritas 3)
      xTaskCreatePinnedToCore(
          taskReadSensors, // Fungsi task 
          "ReadSensors",   // Nama task 
          2048,            // Stack size (bytes)
          NULL,            // Parameter
          3,               // Prioritas (tinggi)
          NULL,            // task handle
          0                // Core ID
      );
      // Buat task pembacaan sensor di Core 0 dengan (pioritas 2)
      xTaskCreatePinnedToCore(
         taskTransmit,  
         "TransmitData",
         2048,
         NULL,
         2,
         NULL,
         1
      );

      // Print ke Serial
      Serial.println("Task berhasil dibuat!"); 
  } else {
    Serial.println("ERROR: Gagal membuar queue!");
    while(true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}

/*
 * Void Loop tidak digunaka karena sudah di handle oleh Free RTOS
 */
void loop() {}
