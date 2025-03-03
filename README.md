# IM73 Defect Detection System for Astra Cups

## Overview
The **IM73 defect detection system** is an automated vision inspection setup designed to detect and reject defective Astra cups. The system integrates **vision cameras, stepper motors, and pneumatic actuators** to ensure high-precision defect identification and rejection.

---

## Key Components & Hardware
- **ClearCore Industrial Microcontroller with Expansion I/O Board** – for motor and actuator control  
- **LattePanda - x86 Windows/Linux Single Board Computer** – for running the HMI system  
- **SC3000 Vision Cameras** – for defect detection and image acquisition  
- **P Series Nema 23 Closed Loop Stepper Motors** – for conveyor belt actuation  
- **PLF060-10-P2-BJ-8-36 Gearboxes** – for stepper motor torque control  
- **Pneumatic Actuators & Valves** – for rejection and flipping mechanisms  

---

## Developer Tools & Software
- **C++ (Embedded Systems Programming)** – for coding the entire mechanism, including motor actuation, camera triggering, and system logic  
- **SCMVS Software** – for training vision models and defect detection tuning  
- **Unity** – for designing the HMI system and integrating it with the ClearCore environment  

---

## Protocols & Implementation
- **Camera Calibration & Model Training** – Configured **line, pattern, edge, and defect detection** for accurate inspections.  
- **Stepper Motor Control** – Synchronized **ClearCore-controlled stepper motors** for precise conveyor movement.  
- **Servo Motor Feedback** – Used **incremental encoder feedback** to fine-tune Astra cup feeding.  
- **HMI System** – Developed a **Unity-based interface** on **LattePanda x86**, integrated with **ClearCore** for real-time monitoring and control.  

---

## Key Stages of System Development
1. **Hardware Assembly & Integration** – Mounted vision cameras, stepper motors, and ClearCore controller.  
2. **Software Development & Testing** – Programmed motor actuation, defect detection logic, and HMI interface.  
3. **System Calibration & Optimization** – Fine-tuned camera settings, stepper synchronization, and rejection mechanisms.  
4. **Final Testing & Debugging** – Verified system accuracy, improved efficiency, and ensured seamless operation.  

---

## Images from Development
![System Overview](path/to/system-overview-image.jpg)  
![HMI Interface](path/to/hmi-interface-image.jpg)  
![Hardware Integration](path/to/hardware-integration-image.jpg)  

---

## GitHub Repository
🔗 [Project Repository](https://github.com/TharumalDinuranga/IM73-Defect-Detection-System)

---

📌 **Contributors**: _Your Name & Team_
