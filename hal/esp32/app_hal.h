#ifndef DRIVER_H
#define DRIVER_H


#ifdef ESPS3_1_69
// 240x280 watchfaces

#define ENABLE_FACE_174 // (174)
#define ENABLE_FACE_228 // (228)
#define ENABLE_FACE_1041 // (1041)
// #define ENABLE_FACE_1167 // (1167)
// #define ENABLE_FACE_1169 // (1169)
// #define ENABLE_FACE_2051 // (2051)
#define ENABLE_FACE_2151 // (2151)
#define ENABLE_FACE_3589 // (3589)

#else
// 240x240 watchfaces

#define ENABLE_FACE_34_2_DIAL // (Shadow) 
#define ENABLE_FACE_75_2_DIAL // (Analog)
// #define ENABLE_FACE_79_2_DIAL // (Blue)
#define ENABLE_FACE_116_2_DIAL // (Outline)
// #define ENABLE_FACE_756_2_DIAL // (Red)
// #define ENABLE_FACE_B_W_RESIZED // (B & W)
// #define ENABLE_FACE_KENYA // (Kenya)
#define ENABLE_FACE_PIXEL_RESIZED // (Pixel)
// #define ENABLE_FACE_RADAR // (Radar)
#define ENABLE_FACE_SMART_RESIZED // (Smart)
// #define ENABLE_FACE_TIX_RESIZED // (Tix)
// #define ENABLE_FACE_WFB_RESIZED // (WFB)

#endif

#ifdef __cplusplus
extern "C" {
#endif


void hal_setup(void);
void hal_loop(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DRIVER_H*/
