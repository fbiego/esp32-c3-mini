#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*encoder_event_cb_t)(long position, int32_t change);
typedef void (*button_event_cb_t)(bool pressed);

void input_bus_add_encoder_sub(encoder_event_cb_t cb);
void input_bus_remove_encoder_sub(encoder_event_cb_t cb);

void input_bus_add_button_sub(button_event_cb_t cb);
void input_bus_remove_button_sub(button_event_cb_t cb);

void input_bus_emit_encoder_event(long position, int32_t change);
void input_bus_emit_button_event(bool pressed);

#ifdef __cplusplus
}
#endif
