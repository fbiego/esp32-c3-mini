#ifndef DRIVER_H
#define DRIVER_H

#ifdef NO_WATCHFACES


#elif defined(ESPS3_1_69) || defined(ELECROW_S3) || defined(ESP32_CYD) || defined(VIEWE_2_8)
// 240x280 watchfaces

#define ENABLE_FACE_174 // (174)
#define ENABLE_FACE_1041 // (1041)
#define ENABLE_FACE_2151 // (2151)
#if (FLASH_SIZE >= 8)
#define ENABLE_FACE_228 // (228)
#define ENABLE_FACE_1167 // (1167)
#endif
#if (FLASH_SIZE >= 16)
#define ENABLE_FACE_1169 // (1169)
#define ENABLE_FACE_2051 // (2051)
#define ENABLE_FACE_3589 // (3589)
#endif

#elif defined(ESPS3_2_06)
#define ENABLE_FACE_174_410 // (174)
#define ENABLE_FACE_228_410 // (228)
#define ENABLE_FACE_1167_410 // (1167)
#define ENABLE_FACE_2051_410 // (2051)
#define ENABLE_FACE_1041_410 // (1041)
#define ENABLE_FACE_2151_410 // (2151)

#elif defined(VIEWE_SMARTRING) || defined(VIEWE_KNOB_15) || defined(ESPS3_1_75) 

#define ENABLE_FACE_756_2_466 // (Red)
#define ENABLE_FACE_RADAR_466 // (Radar)
#define ENABLE_FACE_75_2_466 // (Analog)
#define ENABLE_FACE_34_2_466 // (Shadow) 
#define ENABLE_FACE_79_2_466 // (Blue)
#define ENABLE_FACE_PIXEL_RESIZED_466 // (Pixel)
// #define ENABLE_FACE_SMART_RESIZED_466 // (Smart)

#else
// 240x240 watchfaces

#define ENABLE_FACE_75_2 // (Analog)
#define ENABLE_FACE_756_2 // (Red)
#define ENABLE_FACE_RADAR // (Radar)
#define ENABLE_FACE_34_2 // (Shadow) 

#if (FLASH_SIZE >= 8)
#define ENABLE_FACE_ELECROW // elecrow analog
#define ENABLE_FACE_79_2 // (Blue)
#define ENABLE_FACE_116_2 // (Outline)
#define ENABLE_FACE_B_W_RESIZED // (B & W)
#define ENABLE_FACE_PIXEL_RESIZED // (Pixel)
#define ENABLE_FACE_SMART_RESIZED // (Smart)
#endif
#if (FLASH_SIZE >= 16)
#define ENABLE_FACE_KENYA // (Kenya)
#define ENABLE_FACE_TIX_RESIZED // (Tix)
#define ENABLE_FACE_WFB_RESIZED // (WFB)
#endif

#endif

#if defined(ESPS3_1_69) || defined(ESPS3_1_28) || defined(VIEWE_SMARTRING) || defined(ESPS3_1_75) || defined(ESPS3_2_06)
#define ENABLE_APP_QMI8658C
#define ENABLE_APP_ATTITUDE
#endif

#if defined(ELECROW_C3) || defined(ELECROW_S3)
#define ENABLE_RTC

#ifndef ENABLE_FACE_ELECROW
#define ENABLE_FACE_ELECROW //  elecrow analog
#if defined(ELECROW_C3)
#undef ENABLE_FACE_34_2
#endif
#endif
#endif

#if defined(M5_STACK_DIAL) || defined(VIEWE_KNOB_15) || defined(ELECROW_S3)
#define ENABLE_APP_RANGE
#endif

#define ENABLE_APP_CALENDAR
#define ENABLE_APP_SAMPLE
#define ENABLE_APP_TOUCH_TEST
// #define ENABLE_GAME_TASK
// #define ENABLE_GAME_RACING

#define ENABLE_GAME_SIMON

#define ENABLE_APP_NAVIGATION
#define ENABLE_APP_CONTACTS
#define ENABLE_APP_TIMER

#ifdef __cplusplus
extern "C" {
#endif


void hal_setup(void);
void hal_loop(void);


void vibratePin(bool state);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DRIVER_H*/
