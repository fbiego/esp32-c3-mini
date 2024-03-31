# esp32-c3-mini
A demo Watch project for ESP32 C3 mini 240*240 touch display development board

## Video

[`Demo video`](https://youtu.be/u96OkjxC0Ro)
[`Watchfaces demo`](https://youtu.be/lvRsTp9v6_k)

## Screenshot

![screens](c3-screens.png?raw=true "screens")

## Screens
 - Time (Time, Date, Day, Weather[Icon, Temp]) + Custom Watchfaces
 - Weather (City,Icon, Temp, Update time) (1 week forecast [Day, Icon, Temp])
 - Notifications (Icon, Time, Text) (List [Icon, Text] - 10 notifications)
 - Settings (Brightness, Timeout, Battery, About)
 - Control (Music Control, Find Phone, Bluetooth State)

 #### Watchfaces

| | | |
| -- | -- | -- |
| !["Analog"](src/faces/75_2_dial/watchface.png?raw=true "75_2_dial") | !["Shadow"](src/faces/34_2_dial/watchface.png?raw=true "34_2_dial") | !["Blue"](src/faces/79_2_dial/watchface.png?raw=true "79_2_dial") |
| !["Radar"](src/faces/radar/watchface.png?raw=true "radar") | !["Outline"](src/faces/116_2_dial/watchface.png?raw=true "116_2_dial") | !["Red"](src/faces/756_2_dial/watchface.png?raw=true "756_2_dial") |
| !["Tix"](src/faces/tix_resized/watchface.png?raw=true "tix_resized") | !["Pixel"](src/faces/pixel_resized/watchface.png?raw=true "pixel_resized") | !["Smart"](src/faces/smart_resized/watchface.png?raw=true "smart_resized") |

Check out [`esp32-lvgl-watchface`](https://github.com/fbiego/esp32-lvgl-watchface) project to see how watchfaces are converted from binary to LVGL code. You can add more watchfaces but you will be limited by the ESP32 flash size. 
In that case you can only compile your favorite watchfaces. Links to the pre-built binary watchfaces are included.

## Chronos App
Click to download
[<img src="chronos.png?raw=true" width=100 align=left>](https://fbiego.com/chronos/app?id=c3-mini)
<br><br><br><br>

### App functions
- Sync time to device
- Send notifications to device
- Send weather info to device
- Change background & font color of the time screen, see below

> Watch Tab >> Custom watchface >> Settings

![config](config.png?raw=true "config")

| Button 1 | Button 2 | Color Selector | Result action | 
| --- | --- | --- | --- | 
| Style 1 | Top | Choose color | Change hour color (default face) | 
| Style 1 | Center | Choose color | Change minute color (default face) | 
| Style 1 | Bottom | Choose color | Change other text color (default face) | 
