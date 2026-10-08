# Batman Chronograph Watchface – 410 × 494

Ready-to-drop package for **Chronos** (fbiego) + LVGL on the  
**Waveshare ESP32-S3-Touch-AMOLED-2.06** (native panel is 410×502).

This face is deliberately sized at the resolution you requested (**410 × 494**).

## Contents

| File                        | Purpose                                      |
|-----------------------------|----------------------------------------------|
| `batman_bg_410x494.png`     | Full background (Batman head + dial art)     |
| `hand_hour.png`             | Main hour hand (yellow, transparent)         |
| `hand_min.png`              | Main minute hand                             |
| `hand_sec.png`              | Main second hand                             |
| `hand_sub.png`              | Small hand for the three chronograph subdials|
| `batman_watchface.h`        | Header                                       |
| `batman_watchface.c`        | Implementation (LVGL + timer)                |
| `README.md`                 | This file                                    |

## Integration (Claude Code / PlatformIO / Arduino)

1. Copy the whole folder into your project (e.g. `src/watchfaces/batman/` or `data/` for FS).
2. Make the PNG assets available to LVGL:
   - **Preferred**: put them on SPIFFS / LittleFS / FFAT and use the `"S:"` (or your FS letter) prefix already in the code.
   - Or convert each PNG to an `lv_img_dsc_t` with LVGL’s online image converter / `lv_img_conv` and change the `lv_img_set_src` calls.
3. In your main setup (after `lv_init()` and display driver):

```c
#include "batman_watchface.h"

void setup() {
    // ... display + LVGL + ChronosESP32 init ...
    // ChronosESP32 watch("YourName", CF_WAVESHARE_410x502);

    batman_watchface_create();
}

void loop() {
    // watch.loop();
    lv_timer_handler();
    delay(5);
}
```

4. Fine-tune the three `lv_obj_set_pos()` calls for the subdial hands so they sit exactly in the centre of the three subdial circles on the background.
5. Replace the hardcoded `"78%"` and `"22°C"` with live data from ChronosESP32 or the AXP2101 fuel gauge.

## Notes

- The background was generated to match the approved design (realistic side-on shadowy Batman cowl, small yellow bat emblem, integrated day/battery areas, chronograph subdials).
- Hands are simple geometric placeholders in Batman yellow (`#FFD700`). You can replace them with higher-detail transparent PNGs later.
- Because the physical panel is 410×502 you can either:
  - letterbox / center the 494-high face, or
  - stretch slightly, or
  - regenerate the background at 410×502 if you change your mind.
- Subdial hands are animated (running seconds + slower counters) as requested.

## Chronos screen flag

```c
ChronosESP32 watch("Batman", CF_WAVESHARE_410x502);
```

Claude Code should now be able to drop this package straight into the project and wire the filesystem / image descriptors.
