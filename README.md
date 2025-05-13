## KneeFlexIQ  ![License: MIT](https://img.shields.io/badge/license-MIT-green)
---
Empowering knee rehabilitation with smart sensors and cloud-powered ML. Real-time feedback, seamless IoT integration, and AWS-driven insights for a stronger recovery.
A complete end-to-end solution for post-surgery knee rehabilitation combining wearable hardware with a cloud-based machine learning pipeline.

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
   git clone https://github.com/elpantherd/KneeFlexIQ.git
   ```
   
2. **Install dependencies**
   ```bash
   npm install -g aws-cdk
   ```
   
3. **Configure AWS credentials**
   ```bash
   aws configure
   ```

## 🛠️ Hardware Setup

- **RehabDevice.ino**  
  - Wire flex sensor (A0), MPU6050, OLED, buzzer, SD, Bluetooth, button, battery monitor  
  - Upload via Arduino IDE

- **flex_sensor.ino**  
  - Wire flex sensor to ESP8266 A0  
  - Set `ssid`, `password`, `api_url`  
  - Upload via Arduino IDE



## 🚀 Upload Training Data

```bash
python upload_data.py
```

## 🤖 Train & Deploy ML Model

```bash
python train_and_deploy.py
```

## ☁️ Deploy Cloud Infra
```bash
cd cdk_app
cdk bootstrap
cdk deploy
```
Remember to update flex_sensor.ino with the provided API URL.

## 🔧 Configure Lambda

Set ENDPOINT_NAME in lambda/index.py to your SageMaker endpoint
```bash
cdk deploy
```

---

## 🔧 Customization
1. Replace train.csv with your dataset (flex_value, label)

2. Tune train.py hyperparameters (e.g., n_estimators, max_depth)

3. Add sensors in knee.flex_iq.ino & update the cloud pipeline

4. Integrate a mobile app via Bluetooth or extend the API for additional feedback
---

## 🐞 Troubleshooting
1. S3 bucket name: Must be globally unique

2. SageMaker errors: Check IAM permissions

3. WiFi issues: Verify SSID/password and signal strength

4. API errors: Ensure ENDPOINT_NAME and api_url match your deployments

5. Arduino errors: Confirm wiring, library versions, and review Serial Monitor logs

6. Check CloudWatch (Lambda) and SageMaker logs for deeper insights.

## 📄 License
This project is licensed under the MIT License. [LICENSE](LICENSE)

## Contact Information

For questions or support, contact TEAM GLITCH at [dthayalan760@gmail.com]
 
