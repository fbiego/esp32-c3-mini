
#pragma once


#define LGFX_USE_V1
#include "Arduino.h"
#include <LovyanGFX.hpp>
#include "Arduino_GFX_Library.h"
#include "TouchDrvCSTXXX.hpp"

#include "pins.h"


#define CLAMP(a, x, b) ((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))


#define TFT_BLACK 0x00000


static const uint8_t st77916_init[] = {
    BEGIN_WRITE,
    WRITE_C8_D8, 0xF0, 0x28,
    WRITE_C8_D8, 0xF2, 0x28,
    WRITE_C8_D8, 0x73, 0xF0,
    WRITE_C8_D8, 0x7C, 0xD1,
    WRITE_C8_D8, 0x83, 0xE0,
    WRITE_C8_D8, 0x84, 0x61,
    WRITE_C8_D8, 0xF2, 0x82,
    WRITE_C8_D8, 0xF0, 0x00,
    WRITE_C8_D8, 0xF0, 0x01,
    WRITE_C8_D8, 0xF1, 0x01,
    WRITE_C8_D8, 0xB0, 0x69,
    WRITE_C8_D8, 0xB1, 0x4A,
    WRITE_C8_D8, 0xB2, 0x2F,
    WRITE_C8_D8, 0xB3, 0x01,
    WRITE_C8_D8, 0xB4, 0x69,
    WRITE_C8_D8, 0xB5, 0x45,
    WRITE_C8_D8, 0xB6, 0xAB,
    WRITE_C8_D8, 0xB7, 0x41,
    WRITE_C8_D8, 0xB8, 0x86,
    WRITE_C8_D8, 0xB9, 0x15,
    WRITE_C8_D8, 0xBA, 0x00,
    WRITE_C8_D8, 0xBB, 0x08,
    WRITE_C8_D8, 0xBC, 0x08,
    WRITE_C8_D8, 0xBD, 0x00,
    WRITE_C8_D8, 0xBE, 0x00,
    WRITE_C8_D8, 0xBF, 0x07,
    WRITE_C8_D8, 0xC0, 0x80,
    WRITE_C8_D8, 0xC1, 0x10,
    WRITE_C8_D8, 0xC2, 0x37,
    WRITE_C8_D8, 0xC3, 0x80,
    WRITE_C8_D8, 0xC4, 0x10,
    WRITE_C8_D8, 0xC5, 0x37,
    WRITE_C8_D8, 0xC6, 0xA9,
    WRITE_C8_D8, 0xC7, 0x41,
    WRITE_C8_D8, 0xC8, 0x01,
    WRITE_C8_D8, 0xC9, 0xA9,
    WRITE_C8_D8, 0xCA, 0x41,
    WRITE_C8_D8, 0xCB, 0x01,
    WRITE_C8_D8, 0xCC, 0x7F,
    WRITE_C8_D8, 0xCD, 0x7F,
    WRITE_C8_D8, 0xCE, 0xFF,
    WRITE_C8_D8, 0xD0, 0x91,
    WRITE_C8_D8, 0xD1, 0x68,
    WRITE_C8_D8, 0xD2, 0x68,
    WRITE_C8_D16, 0xF5, 0x00, 0xA5,
    WRITE_C8_D8, 0xF1, 0x10,
    WRITE_C8_D8, 0xF0, 0x00,
    WRITE_C8_D8, 0xF0, 0x02,

    WRITE_COMMAND_8, 0xE0,
    WRITE_BYTES, 14,
    0xF0, 0x10, 0x18, 0x0D,
    0x0C, 0x38, 0x3E, 0x44,
    0x51, 0x39, 0x15, 0x15,
    0x30, 0x34,

    WRITE_COMMAND_8, 0xE1,
    WRITE_BYTES, 14,
    0xF0, 0x0F, 0x17, 0x0D,
    0x0B, 0x07, 0x3E, 0x33,
    0x51, 0x39, 0x15, 0x15,
    0x30, 0x34,

    WRITE_C8_D8, 0xF0, 0x10,
    WRITE_C8_D8, 0xF3, 0x10,
    WRITE_C8_D8, 0xE0, 0x08,
    WRITE_C8_D8, 0xE1, 0x00,
    WRITE_C8_D8, 0xE2, 0x00,
    WRITE_C8_D8, 0xE3, 0x00,
    WRITE_C8_D8, 0xE4, 0xE0,
    WRITE_C8_D8, 0xE5, 0x06,
    WRITE_C8_D8, 0xE6, 0x21,
    WRITE_C8_D8, 0xE7, 0x03,
    WRITE_C8_D8, 0xE8, 0x05,
    WRITE_C8_D8, 0xE9, 0x02,
    WRITE_C8_D8, 0xEA, 0xE9,
    WRITE_C8_D8, 0xEB, 0x00,
    WRITE_C8_D8, 0xEC, 0x00,
    WRITE_C8_D8, 0xED, 0x14,
    WRITE_C8_D8, 0xEE, 0xFF,
    WRITE_C8_D8, 0xEF, 0x00,
    WRITE_C8_D8, 0xF8, 0xFF,
    WRITE_C8_D8, 0xF9, 0x00,
    WRITE_C8_D8, 0xFA, 0x00,
    WRITE_C8_D8, 0xFB, 0x30,
    WRITE_C8_D8, 0xFC, 0x00,
    WRITE_C8_D8, 0xFD, 0x00,
    WRITE_C8_D8, 0xFE, 0x00,
    WRITE_C8_D8, 0xFF, 0x00,
    WRITE_C8_D8, 0x60, 0x40,
    WRITE_C8_D8, 0x61, 0x05,
    WRITE_C8_D8, 0x62, 0x00,
    WRITE_C8_D8, 0x63, 0x42,
    WRITE_C8_D8, 0x64, 0xDA,
    WRITE_C8_D8, 0x65, 0x00,
    WRITE_C8_D8, 0x66, 0x00,
    WRITE_C8_D8, 0x67, 0x00,
    WRITE_C8_D8, 0x68, 0x00,
    WRITE_C8_D8, 0x69, 0x00,
    WRITE_C8_D8, 0x6A, 0x00,
    WRITE_C8_D8, 0x6B, 0x00,
    WRITE_C8_D8, 0x70, 0x40,
    WRITE_C8_D8, 0x71, 0x04,
    WRITE_C8_D8, 0x72, 0x00,
    WRITE_C8_D8, 0x73, 0x42,
    WRITE_C8_D8, 0x74, 0xD9,
    WRITE_C8_D8, 0x75, 0x00,
    WRITE_C8_D8, 0x76, 0x00,
    WRITE_C8_D8, 0x77, 0x00,
    WRITE_C8_D8, 0x78, 0x00,
    WRITE_C8_D8, 0x79, 0x00,
    WRITE_C8_D8, 0x7A, 0x00,
    WRITE_C8_D8, 0x7B, 0x00,
    WRITE_C8_D8, 0x80, 0x48,
    WRITE_C8_D8, 0x81, 0x00,
    WRITE_C8_D8, 0x82, 0x07,
    WRITE_C8_D8, 0x83, 0x02,
    WRITE_C8_D8, 0x84, 0xD7,
    WRITE_C8_D8, 0x85, 0x04,
    WRITE_C8_D8, 0x86, 0x00,
    WRITE_C8_D8, 0x87, 0x00,
    WRITE_C8_D8, 0x88, 0x48,
    WRITE_C8_D8, 0x89, 0x00,
    WRITE_C8_D8, 0x8A, 0x09,
    WRITE_C8_D8, 0x8B, 0x02,
    WRITE_C8_D8, 0x8C, 0xD9,
    WRITE_C8_D8, 0x8D, 0x04,
    WRITE_C8_D8, 0x8E, 0x00,
    WRITE_C8_D8, 0x8F, 0x00,
    WRITE_C8_D8, 0x90, 0x48,
    WRITE_C8_D8, 0x91, 0x00,
    WRITE_C8_D8, 0x92, 0x0B,
    WRITE_C8_D8, 0x93, 0x02,
    WRITE_C8_D8, 0x94, 0xDB,
    WRITE_C8_D8, 0x95, 0x04,
    WRITE_C8_D8, 0x96, 0x00,
    WRITE_C8_D8, 0x97, 0x00,
    WRITE_C8_D8, 0x98, 0x48,
    WRITE_C8_D8, 0x99, 0x00,
    WRITE_C8_D8, 0x9A, 0x0D,
    WRITE_C8_D8, 0x9B, 0x02,
    WRITE_C8_D8, 0x9C, 0xDD,
    WRITE_C8_D8, 0x9D, 0x04,
    WRITE_C8_D8, 0x9E, 0x00,
    WRITE_C8_D8, 0x9F, 0x00,
    WRITE_C8_D8, 0xA0, 0x48,
    WRITE_C8_D8, 0xA1, 0x00,
    WRITE_C8_D8, 0xA2, 0x06,
    WRITE_C8_D8, 0xA3, 0x02,
    WRITE_C8_D8, 0xA4, 0xD6,
    WRITE_C8_D8, 0xA5, 0x04,
    WRITE_C8_D8, 0xA6, 0x00,
    WRITE_C8_D8, 0xA7, 0x00,
    WRITE_C8_D8, 0xA8, 0x48,
    WRITE_C8_D8, 0xA9, 0x00,
    WRITE_C8_D8, 0xAA, 0x08,
    WRITE_C8_D8, 0xAB, 0x02,
    WRITE_C8_D8, 0xAC, 0xD8,
    WRITE_C8_D8, 0xAD, 0x04,
    WRITE_C8_D8, 0xAE, 0x00,
    WRITE_C8_D8, 0xAF, 0x00,
    WRITE_C8_D8, 0xB0, 0x48,
    WRITE_C8_D8, 0xB1, 0x00,
    WRITE_C8_D8, 0xB2, 0x0A,
    WRITE_C8_D8, 0xB3, 0x02,
    WRITE_C8_D8, 0xB4, 0xDA,
    WRITE_C8_D8, 0xB5, 0x04,
    WRITE_C8_D8, 0xB6, 0x00,
    WRITE_C8_D8, 0xB7, 0x00,
    WRITE_C8_D8, 0xB8, 0x48,
    WRITE_C8_D8, 0xB9, 0x00,
    WRITE_C8_D8, 0xBA, 0x0C,
    WRITE_C8_D8, 0xBB, 0x02,
    WRITE_C8_D8, 0xBC, 0xDC,
    WRITE_C8_D8, 0xBD, 0x04,
    WRITE_C8_D8, 0xBE, 0x00,
    WRITE_C8_D8, 0xBF, 0x00,
    WRITE_C8_D8, 0xC0, 0x10,
    WRITE_C8_D8, 0xC1, 0x47,
    WRITE_C8_D8, 0xC2, 0x56,
    WRITE_C8_D8, 0xC3, 0x65,
    WRITE_C8_D8, 0xC4, 0x74,
    WRITE_C8_D8, 0xC5, 0x88,
    WRITE_C8_D8, 0xC6, 0x99,
    WRITE_C8_D8, 0xC7, 0x01,
    WRITE_C8_D8, 0xC8, 0xBB,
    WRITE_C8_D8, 0xC9, 0xAA,
    WRITE_C8_D8, 0xD0, 0x10,
    WRITE_C8_D8, 0xD1, 0x47,
    WRITE_C8_D8, 0xD2, 0x56,
    WRITE_C8_D8, 0xD3, 0x65,
    WRITE_C8_D8, 0xD4, 0x74,
    WRITE_C8_D8, 0xD5, 0x88,
    WRITE_C8_D8, 0xD6, 0x99,
    WRITE_C8_D8, 0xD7, 0x01,
    WRITE_C8_D8, 0xD8, 0xBB,
    WRITE_C8_D8, 0xD9, 0xAA,
    WRITE_C8_D8, 0xF3, 0x01,
    WRITE_C8_D8, 0xF0, 0x00,

    WRITE_C8_D8, 0x3A, 0x05,
    WRITE_C8_D8, 0x35, 0x00,
    WRITE_COMMAND_8, 0x21,
    WRITE_COMMAND_8, 0x11,
    END_WRITE,

    DELAY, ST77916_SLPOUT_DELAY,

    BEGIN_WRITE,
    WRITE_COMMAND_8, 0x29,
    WRITE_COMMAND_8, 0x2c,
    END_WRITE};


class DisplayWrapper
{
public:
    Arduino_DataBus *bus = new Arduino_ESP32QSPI(
            LCD_CS /* CS */, LCD_SCK /* SCK */, LCD_SD0 /* SDIO0 */, LCD_SD1 /* SDIO1 */,
            LCD_SD2 /* SDIO2 */, LCD_SD3 /* SDIO3 */);
    Arduino_GFX *gfx = new Arduino_ST77916(
            bus,
            LCD_RST /* RST */,
            0 /* rotation */,
            true /* IPS */,
            SCREEN_WIDTH,
            SCREEN_HEIGHT
        );
    TouchDrvCST816 touch;
    lgfx::Light_PWM light;


    DisplayWrapper()
    {
        { // Set backlight control. (delete if not necessary)

            auto cfg = light.config(); // Get the structure for backlight configuration.

            cfg.pin_bl = LCD_BL;     // pin number to which the backlight is connected
            cfg.invert = false;  // true to invert backlight brightness
            cfg.freq = 44100;    // backlight PWM frequency
            cfg.pwm_channel = 1; // PWM channel number to use
            cfg.invert = true;

            light.config(cfg);
        }
    }

    void reInit()
    {
        if (LCD_RST != GFX_NOT_DEFINED)
        {
            pinMode(LCD_RST, OUTPUT);
            digitalWrite(LCD_RST, HIGH);
            delay(100);
            digitalWrite(LCD_RST, LOW);
            delay(ST77916_RST_DELAY);
            digitalWrite(LCD_RST, HIGH);
            delay(ST77916_RST_DELAY);
        }
        else
        {
            // Software Rest
            bus->sendCommand(ST77916_SWRESET);
            delay(ST77916_RST_DELAY);
        }

        bus->batchOperation(st77916_init, sizeof(st77916_init));

        gfx->invertDisplay(false);
    }

    bool init(void)
    {
        
#ifdef LCD_EN
        pinMode(LCD_EN, OUTPUT);
        digitalWrite(LCD_EN, HIGH);
#endif
        
        bool state = gfx->begin();

        reInit(); // ST77916 initialization with custom sequence

        touch.setPins(TOUCH_RST, TOUCH_IRQ);
        touch.begin(Wire, 0x15, TOUCH_SDA, TOUCH_SCL);


        light.init(0);

        return state;
    }

    void initDMA(void) {}


    void fillScreen(uint16_t color)
    {
        gfx->fillScreen(color);
    }

    void setRotation(uint8_t rotation)
    {
        gfx->setRotation(rotation);
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
        light.setBrightness(brightness);
    }

    void writePixel(int32_t x, int32_t y, const uint16_t color)
    {
        gfx->writePixel(x, y, color);
    }

    bool getTouch(uint16_t *x, uint16_t *y)
    {
        int16_t x_arr[5], y_arr[5];
        uint8_t touched = touch.getPoint(x_arr, y_arr, touch.getSupportTouchPoint());

        uint8_t rotation = gfx->getRotation();
        uint16_t tx = CLAMP(0, x_arr[0], SCREEN_WIDTH - 1);
        uint16_t ty = CLAMP(0, y_arr[0], SCREEN_HEIGHT - 1);

        switch (rotation & 3)
        {

        case 0: // 0°
            *x = tx;
            *y = ty;
            break;

        case 1: // 90°
            *x = ty;
            *y = SCREEN_HEIGHT - 1 - tx;
            break;

        case 2: // 180°
            *x = SCREEN_WIDTH - 1 - tx;
            *y = SCREEN_HEIGHT - 1 - ty;
            break;

        case 3: // 270°
            *x = SCREEN_WIDTH - 1 - ty;
            *y = tx;
            break;
        }
        return touched;
    }
};

DisplayWrapper tft;
