
#pragma once
#include <Arduino.h>

#define LGFX_USE_V1
#include "Arduino.h"
#include <LovyanGFX.hpp>
#include "Arduino_GFX_Library.h"
#include "Wire.h"

#include "pins.h"

#define GC9309_TFTWIDTH 240
#define GC9309_TFTHEIGHT 296

#define GC9309_RST_DELAY 200   ///< delay ms wait for reset finish
#define GC9309_SLPIN_DELAY 200 ///< delay ms wait for sleep in finish
#define GC9309_SLPOUT_DELAY 80 ///< delay ms wait for sleep out finish

#define GC9309_NOP 0x00
#define GC9309_SWRESET 0x01
#define GC9309_RDDID 0x04
#define GC9309_RDDST 0x09

#define GC9309_SLPIN 0x10
#define GC9309_SLPOUT 0x11
#define GC9309_PTLON 0x12
#define GC9309_NORON 0x13

#define GC9309_INVOFF 0x20
#define GC9309_INVON 0x21
#define GC9309_DISPOFF 0x28
#define GC9309_DISPON 0x29

#define GC9309_CASET 0x2A // Column Address Set
#define GC9309_RASET 0x2B // Page Address Set
#define GC9309_RAMWR 0x2C // Memory Write
#define GC9309_RAMRD 0x2E // Memory Read

#define GC9309_PTLAR 0x30  // Partial Area
#define GC9309_COLMOD 0x3A // Pixel fmt set
#define GC9309_MADCTL 0x36 // Memory Access Ctl

#define GC9309_MADCTL_MY 0x80
#define GC9309_MADCTL_MX 0x40
#define GC9309_MADCTL_MV 0x20
#define GC9309_MADCTL_ML 0x10
#define GC9309_MADCTL_MH 0x04
#define GC9309_MADCTL_RGB 0x08
#define GC9309_MADCTL_BGR 0x00

#define GC9309_RDID1 0xDA // Read ID1
#define GC9309_RDID2 0xDB // Read ID2
#define GC9309_RDID3 0xDC // Read ID3

#define AXS5106L_ADDR 0x63
#define AXS5106L_ID_REG 0x08

#define AXS5106L_TOUCH_DATA_REG 0X01

static const uint8_t GC9309_init_operations[] = {
    BEGIN_WRITE,
    WRITE_COMMAND_8, 0xFE,
    WRITE_COMMAND_8, 0xEF,
    WRITE_C8_D8, 0x80, 0xC0,
    WRITE_C8_D8, 0x81, 0x01,
    WRITE_C8_D8, 0x82, 0x07,
    WRITE_C8_D8, 0x83, 0x38,
    WRITE_C8_D8, 0x88, 0x64,
    WRITE_C8_D8, 0x89, 0x86,
    WRITE_C8_D8, 0x8B, 0x3C,
    WRITE_C8_D8, 0x8D, 0x51,
    WRITE_C8_D8, 0x8E, 0x70,

    WRITE_C8_D8, 0x35, 0x00,
    WRITE_C8_D8, 0x36, 0x48,
    WRITE_C8_D8, 0x3A, 0x05,
    WRITE_C8_D8, 0xBF, 0x1F,

    WRITE_C8_D16, 0x7D, 0x45, 0x06,
    WRITE_C8_D16, 0xEE, 0x00, 0x06,

    WRITE_C8_D8, 0xF4, 0x53,

    WRITE_C8_D16, 0xF6, 0x17, 0x08,
    WRITE_C8_D16, 0x70, 0x4F, 0x4F,
    WRITE_C8_D16, 0x71, 0x12, 0x20,
    WRITE_C8_D16, 0x72, 0x12, 0x20,

    WRITE_C8_D8, 0xB5, 0x50,
    WRITE_C8_D8, 0xBA, 0x00,
    WRITE_C8_D8, 0xEC, 0x71,

    WRITE_C8_D16, 0x7B, 0x00, 0x0D,
    WRITE_C8_D16, 0x7C, 0x0D, 0x03,

    WRITE_COMMAND_8, 0xF5,
    WRITE_BYTES, 3, 0x02, 0x10, 0x12,

    WRITE_COMMAND_8, 0xF0,
    WRITE_BYTES, 12, 0x0C, 0x11, 0x0B, 0x0A, 0x05, 0x32,
    0x44, 0x8E, 0x9A, 0x29, 0x2E, 0x5F,

    WRITE_COMMAND_8, 0xF1,
    WRITE_BYTES, 12, 0x0B, 0x11, 0x0B, 0x07, 0x07, 0x32,
    0x45, 0xBD, 0x8D, 0x21, 0x28, 0xAF,

    // 240
    WRITE_COMMAND_8, 0x2A,
    WRITE_BYTES, 4, 0x00, 0x00, 0x00, 0xEF,
    // 296
    WRITE_COMMAND_8, 0x2B,
    WRITE_BYTES, 4, 0x00, 0x00, 0x01, 0x27,

    WRITE_C8_D8, 0x66, 0x2C,
    WRITE_C8_D8, 0x67, 0x18,
    WRITE_C8_D8, 0x68, 0x3E,
    WRITE_C8_D8, 0xCA, 0x0E,
    WRITE_C8_D8, 0xE8, 0xF0,
    WRITE_C8_D8, 0xCB, 0x06,

    WRITE_COMMAND_8, 0xB6,
    WRITE_BYTES, 3, 0x5C, 0x40, 0x40,

    WRITE_C8_D8, 0xCC, 0x33,
    WRITE_C8_D8, 0xCD, 0x33,

    WRITE_COMMAND_8, GC9309_SLPOUT, // Sleep OUT
    END_WRITE,

    DELAY, GC9309_SLPOUT_DELAY, // 200 ms

    BEGIN_WRITE,
    WRITE_C8_D8, 0xE8, 0xA0,
    WRITE_C8_D8, 0xE8, 0xF0,
    WRITE_COMMAND_8, 0xFE,
    WRITE_COMMAND_8, 0xEE,
    WRITE_COMMAND_8, GC9309_DISPON, // Display on
    WRITE_COMMAND_8, GC9309_RAMWR,  // Memory Write
    END_WRITE,

    DELAY, 10};

#define TFT_BLACK 0x00000

class Arduino_GC9309 : public Arduino_TFT
{
public:
    Arduino_GC9309(
        Arduino_DataBus *bus, int8_t rst = GFX_NOT_DEFINED, uint8_t r = 0,
        bool ips = false, int16_t w = GC9309_TFTWIDTH, int16_t h = GC9309_TFTHEIGHT,
        uint8_t col_offset1 = 0, uint8_t row_offset1 = 0, uint8_t col_offset2 = 0, uint8_t row_offset2 = 0) : Arduino_TFT(bus, rst, r, ips, w, h,
                                                                                                                          col_offset1, row_offset1,
                                                                                                                          col_offset2, row_offset2)
    {
    }

    bool begin(int32_t speed = GFX_NOT_DEFINED) override
    {
        _override_datamode = SPI_MODE0; // always use SPI_MODE0

        return Arduino_TFT::begin(speed);
    }
    void writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h) override
    {
        if ((x != _currentX) || (w != _currentW) || (y != _currentY) || (h != _currentH))
        {
            _bus->writeC8D16D16(GC9309_CASET, x + _xStart, x + w - 1 + _xStart);
            _bus->writeC8D16D16(GC9309_RASET, y + _yStart, y + h - 1 + _yStart);

            _currentX = x;
            _currentY = y;
            _currentW = w;
            _currentH = h;
        }

        _bus->writeCommand(GC9309_RAMWR); // write to RAM
    }
    void setRotation(uint8_t r) override
    {
        Arduino_TFT::setRotation(r);
        switch (_rotation % 4)
        {
        case 1: // Landscape (Portrait + 90°)
            r = GC9309_MADCTL_MV | GC9309_MADCTL_RGB;
            break;
        case 2: // Inverter Portrait
            r = GC9309_MADCTL_MY | GC9309_MADCTL_RGB;
            break;
        case 3: // Inverted Landscape
            r = GC9309_MADCTL_MX | GC9309_MADCTL_MY | GC9309_MADCTL_MV | GC9309_MADCTL_RGB;
            break;
        default: // case 0: (Portrait)
            r = GC9309_MADCTL_MX | GC9309_MADCTL_RGB;
            break;
        }
        _bus->beginWrite();
        _bus->writeC8D8(GC9309_MADCTL, r);
        _bus->endWrite();
    }
    void invertDisplay(bool i) override
    {
        {
            _bus->sendCommand(_ips ? (i ? GC9309_INVOFF : GC9309_INVON) : (i ? GC9309_INVON : GC9309_INVOFF));
        }
    }
    void displayOn() override
    {
        _bus->sendCommand(GC9309_SLPOUT);
        delay(GC9309_SLPOUT_DELAY);
    }
    void displayOff() override
    {
        _bus->sendCommand(GC9309_SLPIN);
        delay(GC9309_SLPIN_DELAY);
    }

    // protected:
    void tftInit() override
    {
        if (_rst != GFX_NOT_DEFINED)
        {
            pinMode(_rst, OUTPUT);
            digitalWrite(_rst, HIGH);
            delay(GC9309_RST_DELAY);
            digitalWrite(_rst, LOW);
            delay(GC9309_RST_DELAY);
            digitalWrite(_rst, HIGH);
            delay(GC9309_RST_DELAY);
        }
        else
        {
            // Software Rest
        }

        _bus->batchOperation(GC9309_init_operations, sizeof(GC9309_init_operations));

        invertDisplay(false);
    }

private:
};

class DisplayWrapper
{
public:
    Arduino_GFX *gfx;
    lgfx::Light_PWM light;

    DisplayWrapper()
    {

        static Arduino_DataBus *bus = new Arduino_ESP32SPIDMA(
            DC /* DC */, CS /* CS */, SCLK /* SCK */, MOSI /* MOSI */, MISO /* MISO */, HSPI, true);
        gfx = new Arduino_GC9309(bus, RST /* RST */, 0 /* rotation */, false /* IPS */, SCREEN_WIDTH /* width */, SCREEN_HEIGHT /* height */);

        { // Set backlight control. (delete if not necessary)

            auto cfg = light.config(); // Get the structure for backlight configuration.

            cfg.pin_bl = BL;     // pin number to which the backlight is connected
            cfg.invert = false;  // true to invert backlight brightness
            cfg.freq = 44100;    // backlight PWM frequency
            cfg.pwm_channel = 1; // PWM channel number to use

            light.config(cfg);
        }
    }

    bool init(void)
    {
        pinMode(47, OUTPUT);
        digitalWrite(47, HIGH);

#ifdef LCD_EN
        pinMode(LCD_EN, OUTPUT);
        digitalWrite(LCD_EN, LOW);
#endif

        pinMode(16, OUTPUT);
        digitalWrite(16, HIGH);

        bool state = gfx->begin();

        pinMode(TP_RST, OUTPUT);

        digitalWrite(TP_RST, LOW);
        delay(200);
        digitalWrite(TP_RST, HIGH);
        delay(300);

        Wire.begin(I2C_SDA, I2C_SCL);

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
        uint8_t data[14] = {0};
        touch_i2c_read(AXS5106L_ADDR, AXS5106L_TOUCH_DATA_REG, data, 14);
        bool touched = data[1] > 0;
        if (touched)
        {
            uint16_t tx = ((uint16_t)(data[2] & 0x0F) << 8) | data[3];
            uint16_t ty = ((uint16_t)(data[4] & 0x0F) << 8) | data[5];
            switch (gfx->getRotation())
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
        }
        return touched;
    }

private:
    bool touch_i2c_write(uint8_t driver_addr, uint8_t reg_addr, const uint8_t *data, uint32_t length)
    {
        Wire.beginTransmission(driver_addr);
        Wire.write(reg_addr);
        Wire.write(data, length);

        if (Wire.endTransmission() != 0)
        {
            return false;
        }
        return true;
    }

    bool touch_i2c_read(uint8_t driver_addr, uint8_t reg_addr, uint8_t *data, uint32_t length)
    {
        Wire.beginTransmission(driver_addr);
        Wire.write(reg_addr);
        if (Wire.endTransmission() != 0)
        {
            return false;
        }

        Wire.requestFrom(driver_addr, length);
        if (Wire.available() != length)
        {
            return false;
        }
        Wire.readBytes(data, length);
        return true;
    }
};

DisplayWrapper tft;
