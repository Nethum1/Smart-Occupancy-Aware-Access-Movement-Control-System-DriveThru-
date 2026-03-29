# Smart-Occupancy-Aware-Access-Movement-Control-System-DriveThru-
This project presents a smart IoT-based system that detects occupancy and movement using multiple sensors and makes intelligent decisions to control physical movement.
`

![Anatomy of a Drive-Thru 102024](https://github.com/user-attachments/assets/70511835-ad8e-4704-bb11-a0dfe7ea3151)

The system follows a modern edge-to-gateway architecture, where:

The ESP32 (edge device) collects real-time sensor data
Node-RED (gateway) processes the data and makes decisions
Actuators (motors) respond based on those decisions

This approach demonstrates sensor fusion, decision-making logic, and real-world automation concepts.
</hr>

## System Architecture

🔹 Edge Layer (ESP32)
- Reads data from multiple sensors
- Sends real-time data to Node-RED
- Receives control commands
- Drives motors accordingly

🔹 Gateway Layer (Node-RED)
- Receives sensor data from ESP32
- Processes and analyzes inputs
- Applies decision logic (functions/rules)
- Sends commands back to ESP32
</hr>

## Hardware Components

🔸 Sensors
- IR Sensor (Object detection)
- Ultrasonic Sensor (Distance measurement)
- LDR (Light intensity detection)
  
🔸 Actuators
- N20 Motors
- Motor Driver Module
  
🔸 Controller
ESP32 WROOM Dev Module
</hr>

