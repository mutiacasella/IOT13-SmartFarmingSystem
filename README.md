# Smart Farming System

## Introduction to the problem and the solution
Pertanian modern membutuhkan sistem pemantauan yang efisien agar kondisi tanaman tetap optimal tanpa harus melakukan pengecekan manual secara terus-menerus. Monitoring tradisional cenderung memakan waktu, kurang akurat, dan dapat menyebabkan keterlambatan tindakan seperti penyiraman ketika tanah sudah kering.

Smart Farming System menawarkan solusi otomatis berbasis IoT yang mampu:
1. Memonitor kondisi udara dan tanah secara real-time.
2. Mengatur penyiraman tanaman secara otomatis berdasarkan parameter tertentu.
3. Meningkatkan efisiensi waktu dan produktivitas petani dengan intervensi minimal namun tetap akurat.


---

## Hardware Design and Implementation Details

### Komponen Utama
- **DHT22 / DHT11**: Mengukur temperatur dan kelembaban udara.
- **MQ-Series Gas Sensor**: Mendeteksi kualitas udara dan keberadaan gas berbahaya.
- **Soil Moisture Sensor**: Mengukur tingkat kelembapan tanah.
- **Relay Module**: Mengendalikan pompa irigasi.
- **Water Pump**: Mengalirkan air irigasi ke tanaman.
- **ESP32**: Mengolah input sensor dan mengendalikan output sistem.

### Desain Hardware
- Rangkaian Sensor ditempatkan pada area tanaman untuk membaca kondisi lingkungan secara langsung.
- Pompa dan relay terhubung ke sumber air untuk mengaktifkan pompa.
- Mikrokontroler menghubungkan seluruh sensor dan aktuator

---

## Software Implementation Details

### Perangkat Lunak
Perangkat lunak dikembangkan menggunakan Arduino IDE. Fitur utamanya mencakup:

- **Pembacaan Sensor Lingkungan**: Mengambil data suhu, kelembapan udara, gas, dan kelembapan tanah secara berkala.
- **Autonomous Irrigation Logic**: Sistem akan mengaktifkan penyiraman dalam tiga kondisi:
    - Tanah terlalu kering berdasarkan nilai sensor
    - Berdasarkan jadwal interval waktu tertentu
    - Ketika suhu udara meningkat drastis yang berpotensi mengeringkan tanah
- Data Notification: Mengirim status sistem ke Blynk.

### Integrasi
- **Pemantauan Lingkungan**: Sistem membaca suhu & kelembapan udara, kualitas udara, serta kelembapan tanah secara berkala.
- **Keputusan Penyiraman**: Penyiraman otomatis aktif jika tanah terlalu kering, suhu terlalu tinggi, atau interval waktu penyiraman sudah tercapai.
- **Aktuasi Irigasi**: Mikrokontroler menyalakan pompa melalui relay dan menghentikannya ketika kelembapan tanah telah mencapai batas optimal.

---

## Test Results and Performance Evaluation

Pengujian dilakukan untuk memastikan keandalan sistem dalam berbagai kondisi:
- **Akurasi Sensor DHT**: 
- **Deteksi Gas**: 
- **Soil Moisture Sensor**: 
- **Mekanisme Penyiraman**: 
- **Stabilitas Sistem**: 

Hasil pengujian menunjukkan bahwa sistem mampu bekerja secara mandiri dan efisien dalam menjaga kondisi tanaman.

---

## Final Product

![Image 1]()

![Image 2]()

---

## Conclusion and Future Work
Smart Farm Monitoring & Irrigation System terbukti efektif dalam mengotomatiskan kontrol pertanian dan meminimalkan intervensi manual tanpa mengurangi akurasi monitoring.

### Rencana Pengembangan
- 

---

## References
- [1]ESP32 I/O, “ESP32 - Gas Sensor,” ESP32 Tutorial, 2025. https://esp32io.com/tutorials/esp32-gas-sensor (accessed Dec. 08, 2025).
- [2]Digilab UI, “Module 2 - Task Manage... | Digilab UI,” Digilabdte.com, 2025. https://learn.digilabdte.com/books/internet-of-things/chapter/module-2-task-management (accessed Dec. 08, 2025).
- [3]Digilab UI, “Module 3 - Memory Mana... | Digilab UI,” Digilabdte.com, 2018. https://learn.digilabdte.com/books/internet-of-things/chapter/module-3-memory-management-queue (accessed Dec. 08, 2025).
- [4]Digilab UI, “Module 4 - Deadlock & ... | Digilab UI,” Digilabdte.com, 2025. https://learn.digilabdte.com/books/internet-of-things/chapter/module-4-deadlock-synchronization (accessed Dec. 08, 2025).
- [5]Digilab UI, “Module 5 - Software Timer | Digilab UI,” Digilabdte.com, 2025. https://learn.digilabdte.com/books/internet-of-things/chapter/module-5-software-timer (accessed Dec. 08, 2025).
- [6]Digilab UI, “Module 7 - MQTT, HTTP,... | Digilab UI,” Digilabdte.com, 2025. https://learn.digilabdte.com/books/internet-of-things/chapter/module-7-mqtt-http-wifi (accessed Dec. 08, 2025).
- [7]Digilab UI, “Module 9 - IoT Platfor... | Digilab UI,” Digilabdte.com, 2025. https://learn.digilabdte.com/books/internet-of-things/chapter/module-9-iot-platforms-blynk-and-red-node (accessed Dec. 08, 2025).
- [8]Random Nerd Tutorials, “ESP32 FreeRTOS: Software Timers/Timer Interrupts (Arduino) | Random Nerd Tutorials,” Random Nerd Tutorials, Nov. 20, 2025. https://randomnerdtutorials.com/esp32-freertos-software-timers-interrupts/ (accessed Dec. 08, 2025).
- [9]Random Nerd Tutorials, “ESP32 Bluetooth Low Energy (BLE) on Arduino IDE | Random Nerd Tutorials,” Random Nerd Tutorials, May 16, 2019. https://randomnerdtutorials.com/esp32-bluetooth-low-energy-ble-arduino-ide/ (accessed Dec. 08, 2025).
- [10]ESP32 I/0, “ESP32 - Soil Moisture Sensor,” ESP32 Tutorial, 2025. https://esp32io.com/tutorials/esp32-soil-moisture-sensor (accessed Dec. 08, 2025).
- [11]ESP32 I/O, “ESP32 - Soil Moisture Sensor Pump | ESP32 Tutorial,” ESP32 Tutorial, 2018. https://esp32io.com/tutorials/esp32-soil-moisture-sensor-pump (accessed Dec. 08, 2025).
- [12]ESP32 I/O, “ESP32 - Automatic Irrigation System,” ESP32 Tutorial, 2018. https://esp32io.com/tutorials/esp32-automatic-irrigation-system (accessed Dec. 08, 2025).

---

## Authors 
- Azra Nabila Azzahra (2306161782)
- Muhamad Rey Kafaka Fadlan (2306250573)
- Mutia Casella (2306202870)
- Wilman Saragih Sitio (2306161776)

 **Universitas Indonesia** 
 
 Departement of Electrical Engineering 
 
 December 8, 2025