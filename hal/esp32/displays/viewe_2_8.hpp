#pragma once

#define LGFX_USE_V1
#include "Arduino.h"
#include <LovyanGFX.hpp>
#include "Arduino_GFX_Library.h"
#include "TouchDrvCSTXXX.hpp"

#define CLAMP(a, x, b) ((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))

#define TFT_BLACK 0x00000

class DisplayWrapper
{
public:
    Arduino_DataBus *bus = new Arduino_ESP32SPIDMA(DC /* DC */, CS /* CS */, SCK /* SCK */, MOSI /* MOSI */, MISO /* MISO */, SPI2_HOST /* spi_num */);

    Arduino_GFX *gfx = new Arduino_ST7789(
        bus, RST /* RST */, 0 /* rotation */, false /* IPS */,
        SCREEN_WIDTH /* width */, SCREEN_HEIGHT /* height */);
    TouchDrvCST816 touch;
    lgfx::Light_PWM light;

    DisplayWrapper()
    {
        { // Set backlight control. (delete if not necessary)

            auto cfg = light.config(); // Get the structure for backlight configuration.

            cfg.pin_bl = BL;     // pin number to which the backlight is connected
            cfg.invert = false;  // true to invert backlight brightness
            cfg.freq = 44100;    // backlight PWM frequency
            cfg.pwm_channel = 1; // PWM channel number to use

            light.config(cfg);
        }
    }

    void initDMA(void) {}

    void fillScreen(uint16_t color)
    {
        gfx->fillScreen(color);
    }

    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data)
    {
        gfx->draw16bitBeRGBBitmap(x, y, data, w, h);
    }

    void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data)
    {
        gfx->draw16bitBeRGBBitmap(x, y, data, w, h);
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
            *x = SCREEN_WIDTH - 1 - ty;
            *y = tx;
            break;

        case 2: // 180°
            *x = SCREEN_WIDTH - 1 - tx;
            *y = SCREEN_HEIGHT - 1 - ty;
            break;

        case 3: // 270°
            *x = ty;
            *y = SCREEN_HEIGHT - 1 - tx;
            break;
        }

        return touched;
    }

    void setBrightness(uint8_t brightness)
    {
        light.setBrightness(brightness);
    }

    void init(void)
    {
        pinMode(LCD_IM0, OUTPUT);
        digitalWrite(LCD_IM0, LOW);
        pinMode(LCD_IM1, OUTPUT);
        digitalWrite(LCD_IM1, HIGH);

        gfx->begin();
        setRotation(0);

        Wire.begin(I2C_SDA, I2C_SCL);

        touch.setPins(TOUCH_RST, TOUCH_INT);
        touch.begin(Wire, TOUCH_ADDR, I2C_SDA, I2C_SCL);

        light.init(0);
    }

    void setRotation(uint8_t r)
    {
        r = (r + 4) % 8;
        gfx->setRotation(r);
        switch (r)
        {
        case 1:
            r = ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB;
            break;
        case 2:
            r = ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB;
            break;
        case 3:
            r = ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB;
            break;
        case 4:
            r = ST7789_MADCTL_MX | ST7789_MADCTL_RGB;
            break;
        case 5:
            r = ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB;
            break;
        case 6:
            r = ST7789_MADCTL_MY | ST7789_MADCTL_RGB;
            break;
        case 7:
            r = ST7789_MADCTL_MV | ST7789_MADCTL_RGB;
            break;
        default: // case 0:
            r = ST7789_MADCTL_RGB;
            break;
        }
        bus->beginWrite();
        bus->writeC8D8(ST7789_MADCTL, r | 1 << 3);
        bus->endWrite();
    }

    void startWrite(void) {}

    uint32_t getStartCount(void)
    {
        return 0;
    }

    void endWrite(void) {}
};

DisplayWrapper tft;
