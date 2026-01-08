/*
 * Pengujian secara dummy data tanpa sensor untuk skenario
 *  a. Membaca data dari 5 buah sensor dan menyimpan nya ke buffer 
 *  b. Menerima data buffer dan membaca nya di task b 
 *  
 * maka untuk solusi ini dengan gunakan mutex dan semaphone
 * agar kedua task baik task a maupun task b tidak menulis dan membaca data
 * secara bersamaan oleh karena itu semaphore digunakan untuk mengunci akses 
 * 
 * Library yang digunakan Free RTOS 
 * karena pakai ESP 32 maka Free RTOS sudah native dan tidak perlu pustaka eksternal 
 */

// Configuration 
#define NUM_SENSORS 5 // menggunakan 5 buah sensor 

// Buat Struktur data untuk buffer 
struct SensorData {
   float temperature;   // tipe data float untuk menyimpan data sensor suhu misal 32.28 C dll
   uint32_t lastUpdate; // gunakan tipda data int 32  untuk menyimpan update dari pembacaan sensor
};

// buat share resource 
SensorData sharedDataBuffer[NUM_SENSORS]; // Variabel untuk shared buffer 
SemaphoreHandle_t xBufferMutex;           // Buat Mutex 

// buat task baca sensor
void taskSensor(void *pvParameters) {
  for(;;) {
    // Simulasi pembacaan data sensor secara dummy 
    Serial.println(F("Sensor Task : Updating buffer..."));

    // Buat izin akses ke buffer 
    if (xSemaphoreTake(xBufferMutex, portMAX_DELAY) == pdTRUE) {
        // Buat perulangan pembacaan data sensor secara dummy
        for (int i = 0; i < NUM_SENSORS; i++) {
            // Buat data dummy 
            sharedDataBuffer[i].temperature = 20.0 + (float) random(0,1000) / 100.0;
            sharedDataBuffer[i].lastUpdate = millis();
        }
         // Lepaskan akses agar task lain bisa masuk
         xSemaphoreGive(xBufferMutex);
    }
      // Jeda pembacaan setiap 1 detik 
      vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// buat task untuk komunikasi antar task 
void taskCommunication(void *pvParameters) {
  for(;;) {
    // ambil data dari buffer untuk dikirim 
    if (xSemaphoreTake(xBufferMutex, portMAX_DELAY) == pdTRUE) {
      Serial.println(F("Communication Task : Sending data ..."));

      for (int i = 0; i < NUM_SENSORS; i++) {
        Serial.printf("[SEND] Sensor %d: %.2f °C (Time: %u ms)\n", 
                      i, sharedDataBuffer[i].temperature, sharedDataBuffer[i].lastUpdate);
      }
      xSemaphoreGive(xBufferMutex);
    }
    // kirim setiap 2 detik 
    vTaskDelay(pdMS_TO_TICKS(2000)); 
  }
}
 
void setup() {
  Serial.begin(1152000);

  // Buat Mutex sebelum task dimulai 
  xBufferMutex = xSemaphoreCreateMutex();

  if (xBufferMutex != NULL) {
    // buat task sensor dengan prioritas lebih tinggi agar data selalu siap
    xTaskCreatePinnedToCore(
        taskSensor,   // Fungsi Task
        "SensorTask", // Nama Task
        2048,         // Ukuran Task (byte)
        NULL,         // Parameter
        2,            // Prioritas 2 (Tinggi)
        NULL,         // Task Handle
        0             // Berjalan di Core 0 pada ESP 32 
    );

   // buat task untuk komunikasi data 
   xTaskCreatePinnedToCore(
      taskCommunication, // Fungsi Task
      "ComTask",         // Nama Task
      2048,              // Ukuran dalam (byte)
      NULL,              // Parameter
      1,                 // Prioritas 1 (Rendah) 
      NULL,              // Task Handle 
      1                  // Berjalan di Core 1 pada ESP 32
   );
   Serial.println(F("Sistem Manajemen Task Berjalan ..."));
  }
}

/*
 * Bagian Void loop di kosongkan karena semua task sudah di handle oleh Free RTOS
 */
void loop() {}
