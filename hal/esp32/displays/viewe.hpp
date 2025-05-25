
#pragma once
#include <Arduino.h>

#include "Arduino_GFX_Library.h"
#include "TouchDrvCSTXXX.hpp"

#define TFT_BLACK 0x00000

class DisplayWrapper
{
public:
    Arduino_GFX *gfx;
    TouchDrvCSTXXX touch;

    DisplayWrapper()
    {

        static Arduino_DataBus *bus = new Arduino_ESP32QSPI(
            7 /* CS */, 13 /* SCK */, 12 /* SDIO0 */, 8 /* SDIO1 */,
            14 /* SDIO2 */, 9 /* SDIO3 */);

        gfx = new Arduino_CO5300(
            bus,
            11 /* RST */,
            0 /* rotation */,
            false /* IPS */,
            466,
            466,
            6 /* col_offset1 */,
            0 /* row_offset1 */,
            0 /* col_offset2 */,
            0 /* row_offset2 */
        );
    }

    bool init(void)
    {
        bool state = gfx->begin();
        touch.setPins(46, 42);
        touch.begin(Wire, 0x15, 41, 45);

        return state;
    }

    void initDMA(void) {}

    void fillScreen(uint16_t color)
    {
        gfx->fillScreen(color);
    }

    void setRotation(uint8_t rotation)
    {
        // gfx->setRotation(rotation);
    }

    void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data)
    {
        gfx->draw16bitBeRGBBitmap(x, y, data, w, h);
    }

    void startWrite(void) {}

    uint32_t getStartCount(void)
    {
        return 0;
    }

    void endWrite(void) {}

    void setBrightness(uint8_t brightness)
    {
        ((Arduino_CO5300 *)gfx)->setBrightness(brightness);
    }

    void writePixel(int32_t x, int32_t y, const uint16_t color)
    {
        gfx->writePixel(x, y, color);
    }

    bool getTouch(uint16_t *x, uint16_t *y)
    {
        int16_t x_arr[5], y_arr[5];
        uint8_t touched = touch.getPoint(x_arr, y_arr, touch.getSupportTouchPoint());
        *x = x_arr[0];
        *y = y_arr[0];
        return touched;
    }
};

DisplayWrapper tft;
