# esp32-c3-mini
A demo LVGL Watch project for ESP32 C3 mini 240*240 touch display development board. Can also be built natively to test the LVGL UI.

## Videos

- [`ESP32 C3 mini LVGL`](https://youtu.be/u96OkjxC0Ro)
- [`ESP32  Watchfaces LVGL`](https://youtu.be/lvRsTp9v6_k)
- [`Waveshare ESP32 S3 (240x240 1.28” Round & 240x280 1.69” Rectangular)`](https://youtu.be/WXER_NX7LWI)


## Screens Preview

![Preview](preview.png?raw=true "preview")

![Preview2](preview_2.png?raw=true "preview2")

## Screens
 - Time (Time, Date, Day, Weather[Icon, Temp]) + Custom Watchfaces
 - Weather (City,Icon, Temp, Update time) (1 week forecast [Day, Icon, Temp])
 - Notifications (Icon, Time, Text) (List [Icon, Text] - 10 notifications) (Incoming call)
 - Settings (Brightness, Timeout, Battery, About)
 - Control (Music Control, Find Phone, Bluetooth State) (Camera Capture)
 - QR Codes
 - A racing game

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

 #### Watchfaces

| | | |
| -- | -- | -- |
| !["Analog"](src/faces/75_2_dial/watchface.png?raw=true "75_2_dial") | !["Shadow"](src/faces/34_2_dial/watchface.png?raw=true "34_2_dial") | !["Blue"](src/faces/79_2_dial/watchface.png?raw=true "79_2_dial") |
| !["Radar"](src/faces/radar/watchface.png?raw=true "radar") | !["Outline"](src/faces/116_2_dial/watchface.png?raw=true "116_2_dial") | !["Red"](src/faces/756_2_dial/watchface.png?raw=true "756_2_dial") |
| !["Tix"](src/faces/tix_resized/watchface.png?raw=true "tix_resized") | !["Pixel"](src/faces/pixel_resized/watchface.png?raw=true "pixel_resized") | !["Smart"](src/faces/smart_resized/watchface.png?raw=true "smart_resized") |
| !["Kenya"](src/faces/kenya/watchface.png?raw=true "kenya") | !["B & W"](src/faces/b_w_resized/watchface.png?raw=true "b_w_resized") | !["WFB"](src/faces/wfb_resized/watchface.png?raw=true "wfb_resized") |

Check out [`esp32-lvgl-watchface`](https://github.com/fbiego/esp32-lvgl-watchface) project to see how watchfaces are converted from binary to LVGL code. You can add more watchfaces but you will be limited by the ESP32 flash size. 
In that case you can only compile your favorite watchfaces. Links to the pre-built binary watchfaces are included.
Enable them in `app_hal.h` according to your build platform.

## Chronos App
This is needed for additional functions on esp32 hardware as listed below.

Click to download
[<img src="chronos.png?raw=true" width=100 align=left>](https://fbiego.com/chronos/app?id=c3-mini)
<br><br><br><br>

### App functions (ESP32)
- Sync time
- Send notifications and call alerts
- Sync weather info
- Sync QR Links (Incomplete)
- Music control, find phone & Camera

