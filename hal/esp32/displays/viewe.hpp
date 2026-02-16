
#pragma once
#include <Arduino.h>

#include "Arduino_GFX_Library.h"
#ifdef ESPS3_2_06
#include "TouchDrvFT6X36.hpp"
#else
#include "TouchDrvCSTXXX.hpp"
#endif
#include "pins.h"

#define TFT_BLACK 0x00000

class DisplayWrapper
{
public:
    Arduino_GFX *gfx;

#ifdef ESPS3_2_06
    TouchDrvFT6X36 touch;
#else
    TouchDrvCSTXXX touch;
#endif

    DisplayWrapper()
    {

        static Arduino_DataBus *bus = new Arduino_ESP32QSPI(
            LCD_CS /* CS */, LCD_SCK /* SCK */, LCD_SD0 /* SDIO0 */, LCD_SD1 /* SDIO1 */,
            LCD_SD2 /* SDIO2 */, LCD_SD3 /* SDIO3 */);

        gfx = new Arduino_CO5300(
            bus,
            LCD_RST /* RST */,
            0 /* rotation */,
            false /* IPS */,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
#ifdef ESPS3_2_06
            22 /* col_offset1 */,
#else
            6 /* col_offset1 */,
#endif
            0 /* row_offset1 */,
            0 /* col_offset2 */,
            0 /* row_offset2 */
        );
    }

    bool init(void)
    {
#ifdef LCD_EN
        pinMode(LCD_EN, OUTPUT);
        digitalWrite(LCD_EN, HIGH);
#endif
        bool state = gfx->begin();
        touch.setPins(TOUCH_RST, TOUCH_IRQ);

#if defined(ESPS3_1_75)
        touch.begin(Wire, 0x5A, TOUCH_SDA, TOUCH_SCL);
        touch.setMaxCoordinates(466, 466);
        touch.setMirrorXY(true, true);
#elif defined(ESPS3_2_06)
        touch.begin(Wire, 0x38, TOUCH_SDA, TOUCH_SCL);
#else
        touch.begin(Wire, 0x15, TOUCH_SDA, TOUCH_SCL);
#endif
        return state;
    }

    void initDMA(void) {}

    void fillScreen(uint16_t color)
    {
        gfx->fillScreen(color);
    }

    void setRotation(uint8_t rotation)
    {
        // gfx->setRotation(rotation); // Not supported in CO5300
    }

    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data)
    {
        gfx->draw16bitBeRGBBitmap(x, y, data, w, h);
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
