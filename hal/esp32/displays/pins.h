/*
   MIT License

  Copyright (c) 2023 Felix Biego

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  ______________  _____
  ___  __/___  /_ ___(_)_____ _______ _______
  __  /_  __  __ \__  / _  _ \__  __ `/_  __ \
  _  __/  _  /_/ /_  /  /  __/_  /_/ / / /_/ /
  /_/     /_.___/ /_/   \___/ _\__, /  \____/
                              /____/

*/

#pragma once

#ifdef ELECROW_C3

// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// touch
#define I2C_SDA 4
#define I2C_SCL 5
#define TP_INT 0
#define TP_RST -1

// display
#define SPI SPI2_HOST

#define SCLK 6
#define MOSI 7
#define MISO -1
#define DC 2
#define CS 10
#define RST -1

#define BL -1 // unused (connected on IO extender)
#define VIBRATION_PIN 0 // dummy (connected on IO extender)

#define BUZZER_PIN 3

#define MAX_FILE_OPEN 10

#elif ESPC3

// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// touch
#define I2C_SDA 4
#define I2C_SCL 5
#define TP_INT 0
#define TP_RST 1

// display
#define SPI SPI2_HOST

#define SCLK 6
#define MOSI 7
#define MISO -1
#define DC 2
#define CS 10
#define RST -1

#define BL 3

#define BUZZER_PIN -1

#define MAX_FILE_OPEN 10

#elif ESPS3_1_28

// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// touch
#define I2C_SDA 6
#define I2C_SCL 7
#define TP_INT 5
#define TP_RST 13

// display
#define SPI SPI2_HOST

#define SCLK 10
#define MOSI 11
#define MISO 12
#define DC 8
#define CS 9
#define RST 14

#define BL 2

#define BUZZER_PIN -1

#define MAX_FILE_OPEN 50

#elif ESPS3_1_69

// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 280
#define OFFSET_X 0
#define OFFSET_Y 20
#define RGB_ORDER true

// touch
#define I2C_SDA 11
#define I2C_SCL 10
#define TP_INT 14
#define TP_RST 13

// display
#define SPI SPI2_HOST

#define SCLK 6
#define MOSI 7
#define MISO -1
#define DC 4
#define CS 5
#define RST 8

#define BL 15

#define BUZZER_PIN -1 //33

#define MAX_FILE_OPEN 20

#elif ESPS3_2_06

#define SCREEN_WIDTH 410
#define SCREEN_HEIGHT 502

#define LCD_CS 12
#define LCD_SCK 11
#define LCD_SD0 4
#define LCD_SD1 5
#define LCD_SD2 6
#define LCD_SD3 7
#define LCD_RST 8

#define TOUCH_SDA 15
#define TOUCH_SCL 14
#define TOUCH_RST 9
#define TOUCH_IRQ 38

#define MAX_FILE_OPEN 10

#elif M5_STACK_DIAL

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

#define BUZZER_PIN 3

#define MAX_FILE_OPEN 10

#elif ESPS3_1_75

#define SCREEN_WIDTH 466
#define SCREEN_HEIGHT 466

#define LCD_CS 12
#define LCD_SCK 38
#define LCD_SD0 4
#define LCD_SD1 5
#define LCD_SD2 6
#define LCD_SD3 7
#define LCD_RST 39

#define TOUCH_SDA 15
#define TOUCH_SCL 14
#define TOUCH_RST 40
#define TOUCH_IRQ 11


#define MAX_FILE_OPEN 10


#elif M5_STACK_DIAL

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

#define BUZZER_PIN 3

#define MAX_FILE_OPEN 10

#elif VIEWE_SMARTRING
#define SCREEN_WIDTH 466
#define SCREEN_HEIGHT 466

#define LCD_CS 7
#define LCD_SCK 13
#define LCD_SD0 12
#define LCD_SD1 8
#define LCD_SD2 14
#define LCD_SD3 9
#define LCD_RST 11

#define TOUCH_SDA 41
#define TOUCH_SCL 45
#define TOUCH_RST 46
#define TOUCH_IRQ 42

#define MAX_FILE_OPEN 10

#elif VIEWE_KNOB_15
#define SCREEN_WIDTH 466
#define SCREEN_HEIGHT 466

#define LCD_CS 12
#define LCD_SCK 10
#define LCD_SD0 13
#define LCD_SD1 11
#define LCD_SD2 14
#define LCD_SD3 9
#define LCD_RST 8
#define LCD_EN 17

#define TOUCH_SDA 1
#define TOUCH_SCL 3
#define TOUCH_RST 2
#define TOUCH_IRQ 4

#define ENCODER_A 6
#define ENCODER_B 5

#define MAX_FILE_OPEN 10


#else

// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// touch
#define I2C_SDA 21
#define I2C_SCL 22
#define TP_INT 14
#define TP_RST 5

// display
#define SPI VSPI_HOST

#define SCLK 18
#define MOSI 23
#define MISO -1
#define DC 4
#define CS 15
#define RST 13

#define BL 2

#define BUZZER_PIN -1

#define MAX_FILE_OPEN 10

#endif
