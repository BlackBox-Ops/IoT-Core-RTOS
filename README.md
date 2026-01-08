# IoT-Core-RTOS
Implementation of advanced embedded systems concepts on ESP32: Dual-core multitasking with FreeRTOS, Thread-safe shared memory management, and Low-level bit-banging for custom DHT libraries.

# ESP32 Advanced Firmware: Multitasking, Shared Memory, and Custom Drivers

Repositori ini berisi serangkaian proyek pengembangan firmware tingkat lanjut menggunakan **ESP32**, **C++ Object-Oriented Programming (OOP)**, dan **FreeRTOS**. Proyek ini mendemonstrasikan kemampuan arsitektur sistem tertanam yang efisien, aman (thread-safe), dan performa tinggi.

## 📂 Struktur Repositori
Setiap modul folder berisi file `.ino` dan gambar skema rangkaian:

1.  **`01_LED_Controller`**: Kontrol LED berbasis objek dengan enkapsulasi logika hardware.
2.  **`02_DHT_Custom_Librar/`**: Implementasi driver sensor DHT kustom berbasis bit-banging.
3.  **`03_Multisensor_RTOS_Queue/`**: Sistem monitoring sensor DS18B20 menggunakan Dual-Core dan Message Queue.
4.  **`04_RTOS_Shared_Buffer_Mutex/`**: Manajemen shared memory menggunakan Mutex Semaphore.

---

## 🛠️ Penjelasan Detail Modul

### 1. LED Controller (Object-Oriented Programming)
Mengimplementasikan manajemen LED menggunakan paradigma OOP untuk meningkatkan kemudahan pemeliharaan kode (*maintainability*).
* [cite_start]**Enkapsulasi**: Menyembunyikan kompleksitas logika *Active LOW* dan *Active HIGH* di dalam class `LedController`[cite: 1, 6, 7].
* [cite_start]**State Management**: Mendukung mode statis dan mode kedip (*blink*) tanpa menggunakan fungsi `delay()` yang menghambat proses utama[cite: 10, 15].
* [cite_start]**Scenario Control**: Menggunakan skenario demo otomatis yang berpindah setiap 2 detik menggunakan *state machine* sederhana[cite: 15, 18].

![Skema LED](./01_LED_Controller/Led_Controler.png)

### 2. Custom DHT Driver (Low-Level Bit Banging)
Pengembangan library mandiri untuk membaca sensor DHT11/DHT22 dengan mematuhi protokol komunikasi data 40-bit sesuai datasheet.
* [cite_start]**Signal Timing**: Mengirim sinyal *start* (18ms/1ms) dan mendeteksi pulsa respon sensor (80µs)[cite: 41, 44].
* [cite_start]**Bit Parsing**: Membedakan bit 0 dan 1 berdasarkan durasi pulsa HIGH (26-28µs vs 70µs)[cite: 46].
* [cite_start]**Thread Safety**: Menggunakan `noInterrupts()` saat proses pembacaan bit kritis untuk menjaga akurasi timing mikrodetik dari gangguan task sistem lain[cite: 43].
* [cite_start]**Data Integrity**: Memvalidasi data melalui perhitungan *checksum* 8-bit[cite: 49].

![Diagram Timing DHT](./02_DHT_Custom_Library/RTOS.png)

### 3. Multisensor Monitoring (FreeRTOS Dual-Core)
Optimasi pembacaan 5 sensor DS18B20 menggunakan fitur *Multitasking* asli ESP32.
* **Dual-Core Task Pinning**: 
    * [cite_start]**TaskReadSensors**: Berjalan di Core 0 untuk akuisisi data sensor DS18B20 secara deterministik[cite: 65, 83].
    * [cite_start]**TaskTransmit**: Berjalan di Core 1 untuk menangani output data tanpa mengganggu proses pembacaan[cite: 75, 84].
* [cite_start]**Message Queue**: Menggunakan `xQueueCreate` untuk memindahkan data antar task secara asinkron, memastikan tidak ada data yang hilang saat transmisi[cite: 81, 82].
* [cite_start]**High Efficiency**: Mengatur resolusi sensor ke 9-bit untuk mempercepat waktu konversi suhu[cite: 62].

![Skema Multisensor](./03_Multisensor_RTOS_Queue/multisensor_schema.png)

### 4. Shared Buffer Management (Mutex Semaphore)
Solusi untuk mengelola akses ke sumber daya bersama (*shared resource*) guna mencegah *Race Condition*.
* [cite_start]**Critical Section**: Menggunakan **Mutex Semaphore** (`xSemaphoreCreateMutex`) untuk mengunci akses ke buffer saat data sedang diperbarui[cite: 93, 100].
* **Producer-Consumer Pattern**:
    * [cite_start]**Sensor Task**: Menghasilkan data simulasi dan memperbarui buffer[cite: 93].
    * [cite_start]**Comm Task**: Membaca data dari buffer dan mengirimkannya secara periodik[cite: 97].
* [cite_start]**Thread Safety**: Menjamin integritas data meskipun diakses oleh dua task dengan prioritas berbeda secara bersamaan[cite: 88, 101].

![Alur Kerja Mutex](./04_RTOS_Shared_Buffer_Mutex/mutex_flow.png)

---

## 🚀 Cara Menjalankan
1. Clone repositori ini.
2. Buka folder proyek menggunakan Arduino IDE.
3. Pastikan Board **ESP32 Dev Module** sudah terpasang.
4. Install library **OneWire** dan **DallasTemperature** melalui Library Manager.
5. Upload kode dan buka Serial Monitor pada baud rate 9600 atau 115200.

---
**Author**: [Nama Kamu]
*Spesialis IoT & Embedded Systems*
