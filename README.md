# Particle IoT Project Collection

> A comprehensive collection of Particle Photon2 and Boron projects for home automation, caregiving, and outdoor adventures. Each project includes complete hardware documentation, CLI workflows, and troubleshooting guides.

## Table of Contents

- [Projects Overview](#projects-overview)
- [Hardware Requirements](#hardware-requirements)
- [Installation & Setup](#installation--setup)
- [Particle CLI Workflow](#particle-cli-workflow)
- [Project Details](#project-details)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## Projects Overview

|
 Project 
|
 Device 
|
 Purpose 
|
 Key Features 
|
|
---------
|
--------
|
---------
|
--------------
|
|
 PapaDuckie Room Controller 
|
 Photon2 
|
 Centralized home automation 
|
 Capacitive touch, local fallback, webhook integration 
|
|
 Helping Hands Memory Companion 
|
 Boron LTE 
|
 Caregiver assistance 
|
 Medication reminders, location sharing, offline safety 
|
|
 Automated Plant Watering 
|
 Photon2 
|
 Smart irrigation 
|
 Soil moisture sensing, pump control, cloud monitoring 
|
|
 Potty Training Urinal 
|
 Photon2 
|
 Child training aid 
|
 Motion detection, reward system, milestone tracking 
|
|
 Kids GPS Tracker 
|
 Boron LTE 
|
 Outdoor safety 
|
 Geofencing, breadcrumb trails, low-power design 
|

## Hardware Requirements

### Common Components

- **Particle Photon2** - Wi-Fi enabled microcontroller
- **Particle Boron** - LTE-M enabled microcontroller  
- **Breadboard & Jumper Wires** - For prototyping
- **Power Supply** - 5V 2A adapter or battery pack
- **USB-C Cable** - Programming and power

### Device-Specific Components

#### PapaDuckie Room Controller
- Capacitive touch sensors (×4)
- LED indicators (×4)
- 5V Relay module (×3)
- DHT22 Temperature/Humidity sensor

#### Helping Hands Memory Companion
- GPS FeatherWing
- OLED Display 0.96"
- Vibration motor
- Piezo buzzer

#### Automated Plant Watering System
- Soil moisture sensor (capacitive)
- 12V water pump
- Relay module
- Water level sensor

#### Potty Training Urinal
- PIR motion sensor
- Water flow sensor
- RGB LED strip
- Speaker module

#### Kids GPS Tracker
- GPS FeatherWing
- LiPo battery (2000mAh)
- Solar panel (optional)
- Enclosure (IP65 rated)

## Installation & Setup

### 1. Prerequisites

Install the Particle CLI based on your operating system:

**macOS / Linux:**
```bash
bash <(curl -sL https://particle.io/install-cli)
```

**Windows:**
Download and run the self-contained Windows CLI installer from the Particle website.

### 2. Authentication

```bash
particle login
```

### 3. Device Setup

1. Connect your Particle device via USB
2. Put the device in listening mode (hold MODE for 3 seconds)
3. Claim the device:
   ```bash
   particle device add
   ```

### 4. Clone Repository

```bash
git clone https://github.com/yourusername/particle-iot-projects.git
cd particle-iot-projects
```

## Particle CLI Workflow

### Basic Commands

```bash
# List all devices
particle list

# Compile firmware
particle compile photon2 project-folder --saveTo firmware.bin

# Flash firmware to device
particle flash  firmware.bin

# Monitor serial output
particle serial monitor

# Call cloud function
particle call   

# Publish event
particle publish  
```

### OTA Updates

```bash
# Target specific Device OS version
particle flash  firmware.bin --target 5.6.0

# Bundle assets with firmware
particle compile photon2 . --saveTo bundle.bin
```

## Project Details

### PapaDuckie Room Controller

**Purpose:** Centralized control for lights, blinds, and appliances with tactile interface.

**Wiring:**
- D0-D3: Capacitive touch sensors
- D4-D6: Relay control pins
- A0: DHT22 data pin
- VIN: 5V power

**Key Functions:**
- `toggleLight` - Control main lights
- `setBlindPosition` - Adjust blind height (0-100%)
- `movieMode` - Activate preset scene

**Particle CLI Usage:**
```bash
particle call photon2 toggleLight on
particle call photon2 setBlindPosition 50
particle publish roomController/movieMode
```

### Helping Hands Memory Companion

**Purpose:** Gentle reminders and location sharing for elderly care.

**Power Management:**
- ULP sleep mode between checks
- Battery monitoring with low-power alerts
- LTE connection only when needed

**Key Functions:**
- `setReminder` - Schedule medication/appointment
- `getLocation` - Request current GPS coordinates
- `emergencyAlert` - Send urgent notification

**Particle CLI Usage:**
```bash
particle call boron setReminder "Medication,09:00,daily"
particle call boron getLocation
particle subscribe helpingHands/alerts
```

### Automated Plant Watering System

**Purpose:** Soil-moisture driven irrigation with manual override.

**Sensor Calibration:**
- Dry soil: >3000 ADC
- Optimal: 1500-2000 ADC  
- Wet soil: <1000 ADC

**Key Functions:**
- `waterNow` - Manual watering cycle
- `setThreshold` - Configure moisture trigger
- `getStatus` - Return system state

**Particle CLI Usage:**
```bash
particle call photon2 waterNow
particle call photon2 setThreshold 1800
particle subscribe plantWatering/status
```

### Potty Training Urinal

**Purpose:** Reward-based training with progress tracking.

**Detection Logic:**
- PIR sensor detects approach
- Flow sensor confirms usage
- Success triggers reward sequence

**Key Functions:**
- `calibrate` - Set sensitivity thresholds
- `recordAttempt` - Log training session
- `getStats` - Retrieve progress data

**Particle CLI Usage:**
```bash
particle call photon2 calibrate
particle call photon2 recordAttempt success
particle call photon2 getStats
```

### Kids GPS Tracker

**Purpose:** Child safety with geofencing and breadcrumb tracking.

**Low Power Features:**
- GPS warm-start with coin cell backup
- Publish interval: 5 minutes (configurable)
- Sleep mode: <10µA current draw

**Key Functions:**
- `setGeofence` - Define safe area
- `updateInterval` - Change reporting frequency
- `emergencyMode` - High-frequency tracking

**Particle CLI Usage:**
```bash
particle call boron setGeofence "37.7749,-122.4194,0.5"
particle call boron updateInterval 300
particle subscribe gpsTracker/emergency
```

## Troubleshooting

### Common Issues

**Device Not Connecting**
- Check Wi-Fi/cellular signal strength
- Verify device is in breathing cyan state
- Run `particle serial monitor` for debug output

**Compilation Errors**
- Ensure all libraries are installed:
  ```bash
  particle library add <library-name>
  ```
- Check Device OS version compatibility

**OTA Update Failures**
- Verify device has sufficient battery/power
- Check available flash space
- Use `particle firmware list` to manage versions

**Sensor Issues**
- Verify wiring connections
- Check power supply voltage
- Calibrate sensors per project documentation

### Debug Commands

```bash
# Check device status
particle device inspect 

# View device variables
particle get  

# Monitor events
particle subscribe mine

# Reset device
particle call  reset
```

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Support

- **Documentation:** [Particle Docs](https://docs.particle.io/)
- **Community:** [Particle Forums](https://community.particle.io/)
- **Issues:** [GitHub Issues](https://github.com/yourusername/particle-iot-projects/issues)

## Acknowledgments

- Particle Inc. for the excellent hardware platform
- The open-source community for libraries and examples
- Contributors who help improve these projects

---

**Last Updated:** ```date```
**Version:** 1.0.0
