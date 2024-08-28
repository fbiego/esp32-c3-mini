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

#include <ChronosESP32.h>

#define UI_VERSION "4.1.0"

#ifdef ELECROW_C3

// screen configs
#define WIDTH 240
#define HEIGHT 240
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

#define BUZZER 3

#define MAX_FILE_OPEN 10

#elif ESPC3

// screen configs
#define WIDTH 240
#define HEIGHT 240
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

#define BUZZER -1

#define MAX_FILE_OPEN 10

#elif ESPS3_1_28

// screen configs
#define WIDTH 240
#define HEIGHT 240
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

#define BUZZER -1

#define MAX_FILE_OPEN 50

#elif ESPS3_1_69

// screen configs
#define WIDTH 240
#define HEIGHT 280
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

#define BUZZER 33

#define MAX_FILE_OPEN 20

#define CS_CONFIG CS_240x296_191_RTF

#else

// screen configs
#define WIDTH 240
#define HEIGHT 240
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

#define BUZZER -1

#define MAX_FILE_OPEN 10

#endif
