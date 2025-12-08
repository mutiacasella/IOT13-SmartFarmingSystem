# Smart Farming System

## Table of Contents
- [I. Introduction](#i-introduction)
- [II. Implementation](#ii-implementation)
- [III. Testing and Evaluation](#iii-testing-and-evaluation)
- [IV. Conclusion](#iv-conclusion)
- [V. References](#v-references)

---

## I. Introduction
Pertanian modern membutuhkan sistem pemantauan yang efisien agar kondisi tanaman tetap optimal tanpa harus melakukan pengecekan manual secara terus-menerus. Monitoring tradisional cenderung memakan waktu, kurang akurat, dan dapat menyebabkan keterlambatan tindakan seperti penyiraman ketika tanah sudah kering.

Smart Farming System menawarkan solusi otomatis berbasis IoT yang mampu:
1. Memonitor kondisi udara dan tanah secara real-time.
2. Mengatur penyiraman tanaman secara otomatis berdasarkan parameter tertentu.
3. Meningkatkan efisiensi waktu dan produktivitas petani dengan intervensi minimal namun tetap akurat.


---

## II. Implementation

### Prerequisites & System Requirements

#### ESP32 Partition Scheme
Ukuran dari hasil upload ini mencapai 1.2MB, diperlukan pengaturan partition scheme pada Arduino IDE:

1. Buka Arduino IDE
2. Pilih **Tools** → **Partition Scheme**
3. Pilih **"No OTA (2MB APP/2MB SPIFFS)"**
   - APP Space: 2MB untuk aplikasi
   - SPIFFS: 2MB untuk file system
   
> Tanpa konfigurasi ini, proses upload firmware akan gagal karena ukuran .bin melebihi ukuran default partition.

#### Library Dependencies
Pastikan library berikut telah terinstall:
- DHT sensor library
- Blynk library
- ESP32 BLE Arduino

---

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

<br>
<div align="center">
  <table>
    <tr>
      <td align="center" width="180">
        <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/a/ae/WiFi_Logo.svg/320px-WiFi_Logo.svg.png" width="85" alt="WiFi"><br>
        <b>Wi-Fi Connectivity</b>
      </td>
      <td align="center" width="180">
        <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/d/da/Bluetooth.svg/500px-Bluetooth.svg.png" width="55" alt="BLE"><br>
        <b>Bluetooth LE</b>
      </td>
      <td align="center" width="180">
        <img src="https://stories.enkosa.com/shared-files/3802/download-logo-blynk-png.png?" width="100" alt="Blynk"><br>
        <b>Blynk IoT</b>
      </td>
    </tr>
  </table>
</div>
<br>


---

## III. Testing and Evaluation

Pengujian dilakukan untuk memastikan keandalan sistem dalam berbagai kondisi:
- **Akurasi Sensor DHT**: Sensor membaca suhu dan kelembaban udara dengan stabil dan respons yang lumayan cepat.
- **Deteksi Gas**: Sensor mampu mendeteksi peningkatan kadar gas secara real-time dan menampilkan peringatan pada dashboard.
- **Soil Moisture Sensor**: Pembacaan kelembaban tanah akurat dengan perubahan nilai yang konsisten sesuai kondisi kering/basah tanah.
- **Mekanisme Penyiraman**: Pompa irigasi bekerja sesuai tiga pemicu: interval waktu, nilai kelembaban tanah di bawah threshold, dan perintah manual melalui aplikasi/tombol. Walau terkendala yang disebabkan dari masalah wiring yang rentan terlepas, dan memompa air yang membuat pelepasan selang input harus disingkirkan terlebih dahulu.
- **Stabilitas Sistem**: FreeRTOS memastikan task berjalan tanpa konflik.

Hasil pengujian menunjukkan bahwa sistem mampu bekerja secara mandiri dan efisien dalam menjaga kondisi tanaman.

---

## Final Product

![Foto Rangkaian Asli](https://hackmd.io/_uploads/rk2El_4fZg.jpg)

---

## IV. Conclusion
Smart Farm Monitoring & Irrigation System terbukti efektif dalam mengotomatiskan kontrol pertanian dan meminimalkan intervensi manual tanpa mengurangi akurasi monitoring.

### Rencana Pengembangan
- Penambahan sensor tambahan untuk analisis kondisi tanaman yang lebih akurat.
- Sistem penyimpanan data untuk memonitor data historis.
- Implementasi panel surya sebagai sumber daya agar sistem lebih hemat energi dan mandiri.

---

## V. References
- [1]ESP32 I/O, “ESP32 - Gas Sensor,” ESP32 Tutorial, 2018. https://esp32io.com/tutorials/esp32-gas-sensor (accessed Dec. 08, 2025).
- [2]Digilab UI, “Module 2 - Task Manage... | Digilab UI,” Digilabdte.com, 2025. https://learn.digilabdte.com/books/internet-of-things/chapter/module-2-task-management (accessed Dec. 08, 2025).
- [3]Digilab UI, “Module 3 - Memory Mana... | Digilab UI,” Digilabdte.com, 2018. https://learn.digilabdte.com/books/internet-of-things/chapter/module-3-memory-management-queue (accessed Dec. 08, 2025).
- [4]Digilab UI, “Module 4 - Deadlock & ... | Digilab UI,” Digilabdte.com, 2025. https://learn.digilabdte.com/books/internet-of-things/chapter/module-4-deadlock-synchronization (accessed Dec. 08, 2025).
- [5]Digilab UI, “Module 5 - Software Timer | Digilab UI,” Digilabdte.com, 2025. https://learn.digilabdte.com/books/internet-of-things/chapter/module-5-software-timer (accessed Dec. 08, 2025).
- [6]Digilab UI, “Module 6 - Bluetooth &... | Digilab UI,” Digilabdte.com, 2025. https://learn.digilabdte.com/books/internet-of-things/chapter/module-6-bluetooth-ble (accessed Dec. 08, 2025).
- [7]Digilab UI, “Module 7 - MQTT, HTTP,... | Digilab UI,” Digilabdte.com, 2025. https://learn.digilabdte.com/books/internet-of-things/chapter/module-7-mqtt-http-wifi (accessed Dec. 08, 2025).
- [8]Digilab UI, “Module 9 - IoT Platfor... | Digilab UI,” Digilabdte.com, 2025. https://learn.digilabdte.com/books/internet-of-things/chapter/module-9-iot-platforms-blynk-and-red-node (accessed Dec. 08, 2025).
- [9]Random Nerd Tutorials, “ESP32 FreeRTOS: Software Timers/Timer Interrupts (Arduino) | Random Nerd Tutorials,” Random Nerd Tutorials, Nov. 20, 2025. https://randomnerdtutorials.com/esp32-freertos-software-timers-interrupts/ (accessed Dec. 08, 2025).
- [10]Random Nerd Tutorials, “ESP32 Bluetooth Low Energy (BLE) on Arduino IDE | Random Nerd Tutorials,” Random Nerd Tutorials, May 16, 2019. https://randomnerdtutorials.com/esp32-bluetooth-low-energy-ble-arduino-ide/ (accessed Dec. 08, 2025).
- [11]ESP32 I/0, “ESP32 - Soil Moisture Sensor,” ESP32 Tutorial, 2018. https://esp32io.com/tutorials/esp32-soil-moisture-sensor (accessed Dec. 08, 2025).
- [12]ESP32 I/O, “ESP32 - Soil Moisture Sensor Pump | ESP32 Tutorial,” ESP32 Tutorial, 2018. https://esp32io.com/tutorials/esp32-soil-moisture-sensor-pump (accessed Dec. 08, 2025).
- [13]ESP32 I/O, “ESP32 - Automatic Irrigation System,” ESP32 Tutorial, 2018. https://esp32io.com/tutorials/esp32-automatic-irrigation-system (accessed Dec. 08, 2025).

---

## Authors 
- [Azra Nabila Azzahra](https://github.com/AzraNA24) (2306161782)
- [Muhamad Rey Kafaka Fadlan](https://github.com/MuhamadReyKF) (2306250573)
- [Mutia Casella](https://github.com/mutiacasella/) (2306202870)
- [Wilman Saragih Sitio](https://github.com/Tinkermannn) (2306161776)