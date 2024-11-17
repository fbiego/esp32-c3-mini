/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.  
    For a copy, see <https://opensource.org/licenses/MIT>.
*/


#ifndef _COMMON_API_H
#define _COMMON_API_H

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct imu_data {
    bool success;
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    float temp;
} imu_data_t;


void imu_init();
imu_data_t get_imu_data();
void imu_close();


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif