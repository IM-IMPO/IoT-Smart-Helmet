# Project Overview

## Project Title
IoT-Based Smart Helmet with MPU6050 Fall Detection and NEO-6M GPS Alert System for Rider's Safety

## Introduction
This project proposes a smart helmet prototype that monitors rider movement and can initiate an emergency alert when a possible crash or abnormal movement is detected. An ESP32 serves as the main controller and processes motion data from an MPU6050 sensor.

The system also incorporates a NEO-6M GPS module for location information, a SIM800L GSM module for SMS alerts, a manual SOS button, a cancel button, a buzzer, and an LED indicator.

## Objectives
- Monitor acceleration and rotational movement using the MPU6050.
- Detect possible impacts or abnormal motion using programmed thresholds.
- Provide a countdown period for cancelling a potential false alarm.
- Allow the rider to manually trigger an SOS alert.
- Obtain GPS coordinates when a valid location fix is available.
- Attempt to send an emergency SMS through the GSM module.
- Provide audible and visual warnings through a buzzer and LED.

## Main Components
- ESP32 Dev Board
- MPU6050 accelerometer and gyroscope
- NEO-6M GPS module
- SIM800L GSM module
- SOS and cancel buttons
- Buzzer and LED
- Battery and appropriate power-management components

## System Operation
The MPU6050 continuously provides movement measurements to the ESP32. When the programmed detection conditions are met, the controller starts an emergency countdown. The rider can cancel the countdown using the cancel button.

If the countdown expires, the system constructs an emergency message and attempts to send it through the SIM800L. If a recent valid GPS fix is available, the message includes the coordinates and a map link. The manual SOS button can also start the countdown.

## Project Scope and Limitations
This project is an experimental prototype. Motion thresholds may produce false alarms or fail to detect certain crashes. GPS availability depends on satellite reception, while SMS functionality depends on the modem, SIM card, power supply, and supported cellular network.

The system is not a certified replacement for protective equipment or a dedicated emergency-response device.
