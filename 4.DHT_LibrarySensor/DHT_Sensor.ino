/**
 * Library kustom untuk membaca data dari sensor DHT 
 * Link Datasheet : https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf
 * 
 */

// Buat Class untuk sensor DHT
class DHTSensor {
  private:
    uint8_t pin;
    uint8_t type;               // Untuk Sensor DHT dibagi menjadi versi DHT 11, 22 dan 21
    uint8_t data[5];            // Tampung dalam List untuk pembacaan 5 byte data sensor (integral, desimal dan checksum)
    unsigned long lastReadTime;

    /*
     * menunggu sinyal pulsa pada level tertentu dan menghitung durasinya
     * durasi pulsa dalam micro seconds 
     */
     uint32_t expectPulse(bool level) {
      uint32_t count = 0;
        // Sensor DHT mengirimkan pulsa dalam rentang rentang 20-80us
        while (digitalRead(this->pin) == level) {
          // Timeout jika pulsa terlalu lama
          if (count++ > 1000) return 0;
        }
        return count;
     }
  
  public:
    DHTSensor(uint8_t pin, uint8_t type) {
      this->pin = pin;        // GPIO Pin 
      this->type= type;       // Tipe Sensor yang dipakai
      this->lastReadTime =0;  // Pembacaan terakhir 
    }

    // buat fungsi untuk mengatifkan pin GPIO dalam keadaan input pull up
    void begin() {
      pinMode(this->pin, INPUT_PULLUP);
      this->lastReadTime = millis();
    }

    /**
     * buat logika boolean untuk status baca sensor dht 
     * karena sensor dht bekerja dengan pertukaran bin 0 dan 1 secara bergantian 
     */

     bool read() {
       unsigned long currentTime = millis();
       if (currentTime - this->lastReadTime < (this->type == 11 ? 1000 : 2000)){
          return false;
       }
       this->lastReadTime = currentTime;

       // Kirim sinyal ke controller
       pinMode(this->pin, OUTPUT);
       digitalWrite(this->pin, LOW);

       // Inisialisasi waktu pembacaan sensor 
       delay(this->type == 11 ? 18 : 1);

       digitalWrite(this->pin, HIGH);
       delayMicroseconds(30); // Tunggu 20-40us
       // aktifkan pin sebagai input 
       pinMode(this->pin, INPUT_PULLUP);

       digitalWrite(this->pin, HIGH);
       delayMicroseconds(30);
       pinMode(this->pin, INPUT_PULLUP);

       // matikan interupt agar pembacaan mkrodetik tidak terganggu
       noInterrupts();

       // sensor akan merespons pada saat sinyal 80 us dari keadaan low ke high
       if (expectPulse(LOW) == 0  || expectPulse(HIGH) == 0) {
         interrupts();
         return false; 
       }
       // membaca data dalam byte 40 bit (5 byte)
       for (int i = 0; i < 40; i++) {
          expectPulse(LOW); // setiap bi dimulai dari 50us dalam keadaan LOW

          // Durasi HIGH menentukan nilai bit 26-28us = bit 0 & 80us = bit 1 
          uint32_t highDuration = expectPulse(HIGH);

          this->data[i / 8] <<= 1;
          // Jika lebih dari 40 us, maka bit 1 
          if (highDuration > 40) {
            this->data[i / 8] |= 1;
          }
       }
       // hidupkan kembali interrupt
       interrupts();
       // checsum 
       return (this->data[4] == ((this->data[0] + this->data[1] + this->data[2] + this->data[3]) & 0xFF));
    }
    
    float getHumidity() {
      if (this->type == 11) return (float)this->data[0]; // DHT11 hanya Byte 0
      return ((this->data[0] << 8) | this->data[1]) * 0.1; // DHT22 butuh 2 Byte
    }

    float getTemperature() {
      if (this->type == 11) return (float)this->data[2];
      float temp = ((this->data[2] & 0x7F) << 8 | this->data[3]) * 0.1;
      if (this->data[2] & 0x80) temp *= -1; // Bit tertinggi untuk tanda negatif
      return temp;
    }
};

// Contoh Penggunaan 

DHTSensor myDht(4, 11);

void setup() {
  Serial.begin(115200);
  myDht.begin();
}

void loop() {
  if (myDht.read()) {
    Serial.printf("Suhu: %.2f °C | Kelembapan: %.2f %%\n", 
                  myDht.getTemperature(), myDht.getHumidity());
  } else {
    // Gagal baca biasanya karena sampling rate terlalu cepat atau kabel longgar
    Serial.println("Gagal membaca dari sensor!");
  }
  delay(2000); 
}
