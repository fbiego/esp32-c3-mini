/**
 * Batman Chronograph Watchface for Chronos / LVGL
 * Target resolution: 410 x 494
 * Hardware: Waveshare ESP32-S3-Touch-AMOLED-2.06 (native 410x502)
 * Use with ChronosESP32 by fbiego
 */

#ifndef BATMAN_WATCHFACE_H
#define BATMAN_WATCHFACE_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Create the Batman watchface on the active screen.
 *  Call after LVGL and display are initialized.
 *  Assets (batman_bg_410x494.png + hands) must be available
 *  via LVGL filesystem driver (e.g. "S:" for SPIFFS/FFAT) or as C arrays.
 */
void batman_watchface_create(void);

/** Optional: force an immediate update of hands + labels */
void batman_watchface_update(void);

#ifdef __cplusplus
}
#endif

#endif /* BATMAN_WATCHFACE_H */
