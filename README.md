# KneeFlexIQ ![License: MIT](https://img.shields.io/badge/license-MIT-green)
Empowering knee rehabilitation with smart sensors and cloud-powered ML. Real-time feedback, seamless IoT integration, and AWS-driven insights for a stronger recovery.
A complete end-to-end solution for post-surgery knee rehabilitation combining wearable hardware with a cloud-based machine learning pipeline.
---
## 🔍 Project Overview
**KneeFlexIQ** integrates an ESP8266/Arduino wearable device with multiple sensors (flex sensor, MPU6050, etc.) and an AWS-powered machine learning backend. It collects motion data, classifies knee exercise quality (correct, too much, too little), and delivers real-time feedback via OLED, buzzer, and Bluetooth.

Key components:
- **Hardware**: ESP8266-based Arduino device collects sensor data, runs local rule-based classification, and streams data.
- **Cloud ML**: Random Forest model trained/deployed on AWS SageMaker for advanced classification.
- **Serverless API**: AWS Lambda & API Gateway enable real-time inference.
- **Data Storage & Infra**: AWS S3 stores datasets; AWS CDK automates resource provisioning.

---

## ⚙️ Features

- **Multi-Sensor Integration**: Flex sensor, MPU6050, OLED display, buzzer, SD card, Bluetooth.
- **Local Classification**: Rule-based detection of exercise quality (correct, too much, too little).
- **Cloud ML Pipeline**: Random Forest on SageMaker for high-accuracy movement classification.
- **Real-Time Feedback**: OLED messages & buzzer alerts on exercise performance.
- **Data Logging**: Session data saved to SD card and sent via Bluetooth.
- **Scalable Cloud**: AWS S3, SageMaker, Lambda, API Gateway.
- **IaC**: AWS CDK scripts for infrastructure provisioning.
- **Robustness**: Comprehensive error handling and logging.

---
## 🛠️ Prerequisites

### Hardware

- ESP8266 Board (for cloud connectivity)  
- Arduino Uno (for RehabDevice sketch)  
- Flex sensor (A0, with voltage divider)  
- MPU6050 (I2C)  
- SSD1306 OLED (I2C)  
- Buzzer (D9)  
- SD card module (SPI, CS=D10)  
- HC-05 Bluetooth (RX=D11, TX=D12)  
- Push button (D2, pull-up)  
- Battery monitor (A1)  
- Optional: Light/temperature sensors, potentiometer  

### Software & Cloud

- **AWS Account** with S3, SageMaker, Lambda, API Gateway, IAM  
- AWS CLI (`aws configure`)  
- Node.js & AWS CDK (`npm install -g aws-cdk`)  
- Python 3.8+ 
- Arduino IDE (with libraries):  
  - Adafruit_GFX  
  - Adafruit_SSD1306  
  - MPU6050  
  - SD, SPI, SoftwareSerial  
  - ESP8266WiFi, ESP8266HTTPClient  

---

## 🚀 Setup Instructions

1. **Clone the repository**  
   ```bash
   git clone https://github.com/your-username/flex-sensor-ml-rehab.git
   cd flex-sensor-ml-rehab
   
2. **Install dependencies**
  ```bash
  npm install -g aws-cdk



