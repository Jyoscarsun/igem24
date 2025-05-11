# 🧪 Lab Management System with Microcontrollers

This project implements a smart lab sample management system using Arduino microcontrollers, RFID technology, proximity sensors, and cloud connectivity. The system enables real-time sample tracking through MQTT communication and MongoDB database integration.

## 🔧 Overview

The system includes two Arduino Uno Rev2 microcontrollers:
- **Rack Arduino**: Detects sample presence using proximity sensors and updates rack occupancy status.
- **RFID Arduino**: Identifies and logs sample RFID tags, associating them with specific rack locations and timestamps.

Communication flows:
- **Microcontrollers ↔ User Interface** via MQTT (publisher-subscriber model)
- **Microcontrollers ↔ MongoDB Atlas** via HTTPS and RESTful APIs

---

## 📦 Features

- ✅ **User Interaction Protocols**: Startup, registration, check-in, check-out, and deregistration.
- ✅ **MQTT Integration**: Lightweight messaging between microcontrollers and user interface (via HiveMQ).
- ✅ **Cloud Storage**: Sample data stored in a MongoDB Atlas database under two collections:
  - `Rack Well`: Tracks rack slots and RFID association.
  - `RFID Chips`: Tracks RFID registration, usage, and timestamps.
- ✅ **Real-Time Updates**: Distance sensing and RFID scans trigger updates to database fields.
- ✅ **Modular Codebase**: Each function (e.g., registration, tap reading, check-in) is modularized in Arduino code.

---

## 🛠️ Hardware Used

- 2x **Arduino Uno WiFi Rev2**
- 6x **Proximity Sensors (IR-based)**
- 1x **MFRC522 RFID Reader**
- 1x **RTC Clock Module (MKR WiFi 1010)**
- Breadboard, jumper wires, RFID tags

---

## 🧰 Software Stack

| Component         | Details                         |
|------------------|---------------------------------|
| Arduino Libraries | `WiFiNINA`, `ArduinoHttpClient`, `ArduinoJson`, `Wire`, `RTClib`, `ArduinoMqttClient`, `MFRC522` |
| Database          | MongoDB Atlas                   |
| MQTT Broker       | HiveMQ (Cloud)                  |
| Code Language     | C++ for Arduino; Python (for setup & API) |

---

## 📑 Database Schema

### 📁 Rack Well
| Field         | Description                                |
|---------------|--------------------------------------------|
| `_id`         | MongoDB ID                                  |
| `Rack`        | Rack label (`"A"` or `"B"`)                |
| `Well`        | Integer (1–25)                              |
| `Occupied`    | Boolean (true/false)                        |
| `Occupied By` | RFID tag number (or `null`)                |

### 📁 RFID Chips
| Field            | Description                              |
|------------------|------------------------------------------|
| `_id`            | MongoDB ID                               |
| `RFID no`        | RFID identifier (1–50)                   |
| `In Use?`        | Boolean (true/false)                     |
| `Description`    | User-provided string                     |
| `Last Tapped Time` | Date timestamp of last scan             |
| `Registration Time` | Date of registration                    |
| `Well no`        | Slot location or `null`                  |

---

## 🔗 MQTT Topics

| Topic                        | Description                         |
|-----------------------------|-------------------------------------|
| `arduino/rack/registration` | Handle new sample registration      |
| `arduino/rack/checkin`      | Mark sample check-in                |
| `arduino/checkout`          | Handle sample check-out             |
| `arduino/rfid`              | Handle RFID tapping events          |

---

## 🚀 Getting Started

### 1. Configure WiFi Credentials
```cpp
const char *ssid = "your_wifi_ssid";
const char *username = ""; // if needed
const char *password = "your_wifi_password";
```
### 2. Set Up MQTT Server
```cpp
const char* mqtt_server = "your_mqtt_broker_url";
const char *mqtt_username = "your_mqtt_username";
const char *mqtt_password = "your_mqtt_password";
```
### 3. Initialize MongoDB Atlas  
Set up the `igemhardware` database with two collections:

- `Rack Well`  
- `RFID Chips`

Use HTTPS and the MongoDB Data API key for secure updates via RESTful requests.
