<div align="center">

# 🤖 Dumby

### Small Screen. Big Personality.

An ESP8266-powered interactive desktop companion inspired by Dasai Mochi, featuring animated expressions, real-time weather updates, digital clock, mini-games, sound effects, and WiFi connectivity.

![License](https://img.shields.io/badge/License-MIT-green)
![Platform](https://img.shields.io/badge/Platform-ESP8266-blue)
![Status](https://img.shields.io/badge/Status-Active-success)

</div>

---

## 🌟 Overview

Dumby is a smart desktop companion designed to bring life, personality, and functionality to your workspace.

Unlike traditional desk gadgets, Dumby combines entertainment and utility into a compact device powered by the ESP8266.

Whether it's displaying the current time, showing weather information, playing mini-games, or simply reacting with expressive animations, Dumby is always there to keep your desk a little less boring.

---

## ✨ Features

- 🎭 Animated facial expressions
- ⏰ Real-time clock synchronization
- 🌤 Live weather updates
- 🎮 Built-in mini-games
- 🔔 Interactive sound effects
- 🌐 WiFi connectivity
- 🖥 OLED display interface
- 🔋 Portable & rechargeable design
- ⚡ Lightweight and efficient ESP8266 firmware

---

## 📸 Project Preview

### Hardware Architecture

<p align="center">
  ![image](https://github.com/Aditya-Kushwahaa/Dumby/blob/34d99763865ca05d698abb5ade960572020c732e/images/Circuit%20Diagram.png)
</p>

---

## 🛠 Hardware Components

| Component | Quantity |
|------------|-----------|
| NodeMCU ESP8266 | 1 |
| SSD1306 OLED Display | 1 |
| Push Button | 1 |
| Active Buzzer | 1 |
| Li-ion Battery (Optional) | 1 |
| TP4056 Charging Module (Optional) | 1 |
| Power Switch | 1 |

---

## ⚙️ Software Stack

- Arduino IDE
- ESP8266 Board Package
- Adafruit SSD1306 Library
- Adafruit GFX Library
- WiFi Library
- NTP Client
- OpenWeather API

---

## 🚀 Current Functions

### 🕒 Clock Mode
Displays synchronized real-time clock using NTP servers.

### 🌤 Weather Mode
Fetches live weather information through WiFi.

### 🎮 Game Mode
Simple OLED-based mini-games for quick entertainment.

### 😄 Expression Mode
Displays various animated faces and reactions.

### 🔔 Notification Mode
Provides audio feedback through the buzzer.

---

## 📂 Project Structure

```text
Dumby/
│
├── firmware/
│   └── dumby.ino
│
├── images/
│   ├── circuit-diagram.png
│   └── demo.gif
│
├── README.md
└── LICENSE
```

---

## 🔌 Getting Started

### 1. Clone Repository

```bash
git clone https://github.com/yourusername/Dumby.git
```

### 2. Open Project

Open `dumby.ino` using Arduino IDE.

### 3. Install Required Libraries

- Adafruit SSD1306
- Adafruit GFX
- NTP Client
- ArduinoJson

### 4. Configure WiFi

```cpp
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
```

### 5. Upload to ESP8266

Select:

```
Board: NodeMCU 1.0 (ESP-12E Module)
```

Upload and enjoy!

---

## 🌍 Future Roadmap

- 🤖 AI-powered interactions
- 📱 Mobile application
- 🎵 Music visualizer
- 🌈 Custom themes
- 📡 Telegram notifications
- 🛰 Space dashboard mode
- 🎙 Voice commands
- ☁ Cloud synchronization

---

## 💡 Inspiration

Dumby was inspired by modern desktop companions such as Dasai Mochi and the idea that technology should feel fun, expressive, and personal.

The goal was to create a device that is more than a gadget — a tiny companion that adds personality to any workspace.

---

## 👨‍💻 Developer

### Aditya Kushwaha

- 🤖 Robotics Enthusiast
- 🚁 Drone Developer
- 💻 AI & ML Student
- 🌌 Space Technology Explorer

> Building technology that feels like science fiction.

---

<div align="center">

### ⭐ If you like this project, consider starring the repository!

**Dumby — The Not-So-Dumb Desktop Companion**

</div>
