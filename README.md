# ESP32-S3 TFT Display Tutorial

This repository provides a comprehensive guide to interfacing an ESP32-S3 microcontroller with a TFT display. The tutorial covers hardware setup, software configuration, and example code to help you get started with your display project.

## Table of Contents

- [ESP32-S3 TFT Display Tutorial](#esp32-s3-tft-display-tutorial)
  - [Table of Contents](#table-of-contents)
  - [Hardware Requirements](#hardware-requirements)
  - [Wiring Diagram](#wiring-diagram)
  - [Software Setup](#software-setup)
    - [PlatformIO Installation](#platformio-installation)
  - [Library Installation](#library-installation)
  - [Configuration](#configuration)
  - [Example Code](#example-code)
  - [Troubleshooting](#troubleshooting)
  - [References](#references)

## Hardware Requirements

To follow this tutorial, you will need the following components:

- [ESP32-S3 Development Board](https://www.adafruit.com/product/5328)
- [3.5-inch TFT LCD Touchscreen Display (ILI9341)](https://www.adafruit.com/product/1770)
- Breadboard and jumper wires

## Wiring Diagram

Connect the TFT display to the ESP32-S3 as follows:

| TFT Pin | ESP32-S3 Pin |
|---------|--------------|
| VCC     | 3.3V         |
| GND     | GND          |
| CS      | GPIO 15      |
| RESET   | GPIO 4       |
| DC/RS   | GPIO 2       |
| SDI(MOSI)| GPIO 13     |
| SCK     | GPIO 14      |
| LED     | 3.3V         |
| SDO(MISO)| GPIO 12     |
| T_CLK   | GPIO 25      |
| T_CS    | GPIO 33      |
| T_DIN   | GPIO 32      |
| T_DO    | GPIO 39      |
| T_IRQ   | GPIO 36      |

*Note: Ensure that the TFT's backlight (LED) is connected to 3.3V to avoid damage.*

## Software Setup

### PlatformIO Installation

This project utilizes PlatformIO for development. Follow these steps to set up your environment:

1. **Install PlatformIO:**
   - Install the [PlatformIO IDE](https://platformio.org/install) as an extension in Visual Studio Code.

2. **Clone the Repository:**
   ```bash
   git clone https://github.com/stephennacion06/esp32-s3_tft_display_tutorial.git
   ```
   Open the cloned repository in Visual Studio Code.

3. **Open the Project:**
   - Launch Visual Studio Code.
   - Navigate to `File` > `Open Folder` and select the cloned repository.

## Library Installation

The project requires specific libraries to interface with the TFT display:

1. **TFT_eSPI Library:**
   - This library is essential for driving the TFT display.
   - Install it via PlatformIO by adding the following to your `platformio.ini` file:
     ```ini
     lib_deps =
       bodmer/TFT_eSPI
     ```

2. **Touchscreen Library (XPT2046):**
   - If your display includes a touchscreen with the XPT2046 controller, install:
     ```ini
     lib_deps =
       https://github.com/stephennacion06/XPT2046_Touchscreen_esp32-s3.git
     ```

## Configuration

After installing the necessary libraries, configure the `TFT_eSPI` library to match your hardware setup:

1. **Locate the Configuration File:**
   - Navigate to `.pio/libdeps/esp32-s3-dev/TFT_eSPI/User_Setup.h`.

2. **Edit the Configuration:**
   - Define the display driver and pins according to your wiring:
     ```c
     #define ILI9341_DRIVER
     #define TFT_CS   15
     #define TFT_DC    2
     #define TFT_RST   4
     #define TFT_MOSI 13
     #define TFT_SCLK 14
     #define TFT_MISO 12
     ```

3. **Save the Changes:**
   - Ensure all configurations match your hardware connections.

## Example Code

The `src` directory contains example code demonstrating basic usage of the TFT display. To upload the example:

1. **Open `src/main.cpp`:**
   - Review the code to understand its functionality.

2. **Build and Upload:**
   - In the PlatformIO toolbar, click on the check mark (Build) to compile the code.
   - Click on the right arrow (Upload) to flash the code to your ESP32-S3 board.

3. **Monitor Output:**
   - Use the serial monitor (plug icon) to view debug messages and ensure the display initializes correctly.

## Troubleshooting

If you encounter issues:

- **Display Not Working:**
  - Verify all wiring connections.
  - Ensure the `User_Setup.h` configuration matches your wiring.
  - Check that the display's backlight is powered.

- **Touchscreen Issues:**
  - Confirm that the touchscreen controller matches the library used.
  - Ensure touch-related pins are correctly connected and defined.

## References

- [Adafruit ESP32-S3 TFT Feather Guide](https://learn.adafruit.com/adafruit-esp32-s3-tft-feather)
- [Random Nerd Tutorials: ESP32 with TFT Display](https://randomnerdtutorials.com/esp32-tft-touchscreen-display-2-8-ili9341-arduino/)
- [TFT_eSPI Library GitHub Repository](https://github.com/Bodmer/TFT_eSPI)
- [XPT2046 Touchscreen ESP32-S3 Library](https://github.com/stephennacion06/XPT2046_Touchscreen_esp32-s3)

For further assistance, refer to the [issues section](https://github.com/stephennacion06/esp32-s3_tft_display_tutorial/issues) or submit a new issue describing your problem.

---

By following this guide, you should be able to set up and run a TFT display with your ESP32-S3 successfully.