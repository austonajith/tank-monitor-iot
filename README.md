# Smart Dual-Tank Water Level Monitoring System (ESP32 + Arduino UNO)

This open-source project monitors two overhead water tanks using **ESP32** and **Arduino UNO**.  
It provides both **local LED display** and **Wi-Fi-based monitoring** — perfect for home or apartment water management.

---

## 🌟 Features

✅ Dual-tank monitoring (5 levels per tank)  
✅ Real-time LED display (5 LEDs per tank)  
✅ Wi-Fi dashboard and JSON API  
✅ OTA (Over-the-Air) firmware update support  
✅ Works offline (no Wi-Fi required for LED display)  
✅ Low-water blinking alert  
✅ Simple and modular design  

---

## ⚙️ System Overview

- 🧠 **ESP32 (Tank Unit)** → Reads water probe levels and sends formatted data via serial  
- 💡 **Arduino UNO (Display Unit)** → Receives serial data and lights LEDs to show tank levels  
- 🌐 **Web Interface** → Hosted on ESP32 for viewing tank levels remotely  
- ⚡ **Offline Mode** → UNO display keeps working even if Wi-Fi is off  

---

## 🧩 Hardware Used

| Component | Quantity | Description |
|------------|-----------|-------------|
| ESP32 WROOM32 | 1 | Reads tank levels and hosts web server |
| Arduino UNO | 1 | Drives LED indicators |
| Stainless steel probes | 10 | (5 per tank) water level sensors |
| LEDs | 10 | (5 per tank) for visual indication |
| Resistors | 10 × 330Ω | For LED current limiting |
| Enclosures | 2 | Waterproof case (ESP32), wall mount (UNO) |
| Jumper wires | — | For connections |

---

## 🔌 Wiring Diagram

### 🔵 **ESP32 (Tank Unit)**

| Tank | Probe Level | ESP32 Pin |
|------|--------------|-----------|
| Tank 1 | 10% | GPIO32 |
| Tank 1 | 30% | GPIO33 |
| Tank 1 | 50% | GPIO25 |
| Tank 1 | 70% | GPIO26 |
| Tank 1 | 100% | GPIO27 |
| Tank 2 | 10% | GPIO14 |
| Tank 2 | 30% | GPIO12 |
| Tank 2 | 50% | GPIO13 |
| Tank 2 | 70% | GPIO4 |
| Tank 2 | 100% | GPIO5 |
| Serial TX → UNO RX | GPIO17 |
| GND | Common Ground |

### 🟢 **Arduino UNO (Display Unit)**

| Tank | LED No | Pin |
|------|---------|----|
| Tank 1 | LED1–LED5 | 4–8 |
| Tank 2 | LED1–LED5 | 9–13 |
| RX (From ESP32 TX) | Pin 0 |
| GND | Common Ground |

💡 **Optional:** Use a **1 kΩ resistor** between ESP32 TX and UNO RX for protection.

---

## 🪜 Tank Probe Connection

Each tank uses 5 stainless-steel probes connected at different water heights:

100% ────> GPIO27 / GPIO5<br>
70% ────> GPIO26 / GPIO4<br>
50% ────> GPIO25 / GPIO13<br>
30% ────> GPIO33 / GPIO12<br>
10% ────> GPIO32 / GPIO14<br>
GND ────> Common ground probe<br>


---

## 📡 Communication Format

ESP32 sends data over serial every second in this format:


Tank1:50%|Tank2:100%


UNO parses this string and updates the LED levels:
- LEDs light up according to percentage
- Bottom LED blinks if tank level = 0%

---

## 🌐 Wi-Fi Dashboard

When Wi-Fi is enabled (via long press on ESP32 BOOT button):
- **Access Point Mode:** If no saved Wi-Fi credentials  
- **Client Mode:** Connects to saved Wi-Fi network  
- Webpage available at  


http://<esp32-ip>/

- JSON data available at  


http://<esp32-ip>/data

Example JSON response:
{
"tank1": 50,
"tank2": 100
} 

🔧 OTA (Over-the-Air) Updates

Once connected to Wi-Fi, the ESP32 supports Arduino OTA updates.
You can upload new firmware wirelessly using the Arduino IDE → Port → Network Port → AJI-TankMonitor.

🏗️ Enclosure Setup

ESP32 Unit: Mounted inside a waterproof metal box on the terrace.

UNO Display: Mounted in a wall switch box on the ground floor for easy viewing.


📸 Photos
![Circuit Diagram](screenshots/circuit_image%20(3).png)
![esp32](screenshots/PXL_20251020_042112215.MP.jpg)
[Arduino Uno](screenshots/PXL_20251020_044440405~2.mp4)
[Output](screenshots/PXL_20251020_044114553~3.mp4)
🎬 [Watch Demo Video](screenshots/Arduino%20Tank%20Monitor.mp4)

	
🧾 Future Improvements

🔧 Replace UNO with ATtiny85 or STM32 Blue Pill for cost and space optimization

📶 Add LoRa or ESP-NOW for wireless long-range data transfer

📱 Integrate with Home Assistant or Blynk App for mobile notifications

❤️ Acknowledgements

Project designed and built by AJI
Special thanks to the open-source Arduino and ESP32 community.

🧠 License

This project is open-sourced under the MIT License — you’re free to use, modify, and share it with credit.

