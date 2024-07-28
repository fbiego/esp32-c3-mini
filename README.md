# esp32-c3-mini
A demo LVGL Watch project for ESP32 C3 mini 240*240 touch display development board. Can also be built natively to test the LVGL UI.

## Videos

- [`ESP32 C3 mini LVGL`](https://youtu.be/u96OkjxC0Ro)
- [`ESP32  Watchfaces LVGL`](https://youtu.be/lvRsTp9v6_k)
- [`Waveshare ESP32 S3 (240x240 1.28” Round & 240x280 1.69” Rectangular)`](https://youtu.be/WXER_NX7LWI)
- [`Installing custom watchfaces`](https://youtu.be/qXx6tj7s6pQ)



https://github.com/user-attachments/assets/3cdc30e3-8383-4227-b88c-32bd1d464c70



## Screens Preview

![Preview](preview.png?raw=true "preview")

![Preview2](preview_2.png?raw=true "preview2")

## Screens
 - Time (Time, Date, Day, Weather[Icon, Temp]) + Custom Watchfaces
 - Installable custom watchfaces from Chronos app
 - Weather (City,Icon, Temp, Update time) (1 week forecast [Day, Icon, Temp])
 - Notifications (Icon, Time, Text) (List [Icon, Text] - 10 notifications) (Incoming call)
 - Settings (Brightness, Timeout, Battery, About)
 - Control (Music Control, Find Phone, Bluetooth State) (Camera Capture)
 - QR Codes
 - A racing game (Need to enable)

 ## Building

 Select your build environment in platformio. Default is `emulator_64bits` native.

 When building for native check that you have configured SDL according to your platform. Follow the instructions here
 https://github.com/lvgl/lv_platformio?tab=readme-ov-file#install-sdl-drivers

 The SDL path might be different depending on your configuration and you will need to update [`platformio.ini`](platformio.ini) accordingly

 ### Prebuilt Native

 The prebuilt native applications have been included in the [`test folder`](test/), however you might still require SDL installed before running them.


## Boards

![Boards](boards.png?raw=true "boards")

- C3 Mini: https://www.aliexpress.com/item/1005006451631422.html
- Waveshare S3 1.28: https://www.waveshare.com/product/esp32-s3-touch-lcd-1.28.htm
- Waveshare S3 1.69: https://www.waveshare.com/esp32-s3-touch-lcd-1.69.htm

 ## Watchfaces

This project supports two types of watchfaces in addition to the default one:

#### 1. External Precompiled Binary Watchfaces

These watchfaces are binary files converted into LVGL code and compiled along with the main code. To add or remove these watchfaces, you need to recompile and flash the firmware.

- Check out the [`esp32-lvgl-watchface`](https://github.com/fbiego/esp32-lvgl-watchface) project for details on converting watchfaces from binary to LVGL code.
- You can add more watchfaces, but be mindful of the ESP32's flash size limitations. Prioritize compiling only your favorite watchfaces.
- Links to pre-built binary watchfaces are included. Enable them in `app_hal.h` according to your build platform.

#### 2. External Installable Binary Watchfaces

This project now supports the installation of binary watchfaces after the initial code compilation and flashing. You can add or remove watchfaces via the Chronos app using BLE. Once transferred to the ESP32, the watchface will be parsed and executed.

- Ensure there is sufficient storage space on the ESP32 flash. Using the FFAT partition is recommended.

> [!IMPORTANT]
> This feature is experimental and may not work 100% reliably.
> Ensure your partition is mounted successfully for proper functionality.

> [!WARNING]  
> This has issues running on ESP32 C3 Mini due to smaller SRAM size


## Chronos App
This is needed for additional functions on esp32 hardware as listed below.

[<img src="chronos.png?raw=true" width=100 align=left>](https://chronos.ke/app?id=c3-mini)
<br><br><br><br>

[ChronosESP32 Website](https://chronos.ke/esp32)


### App functions (ESP32)
[ChronosESP32](https://github.com/fbiego/chronos-esp32) library handles communication with the Chronos app over BLE
- Sync time
- Install additional watchfaces
- Send notifications and call alerts
- Sync weather info
- Sync QR Links (Incomplete)
- Music control, find phone & Camera

