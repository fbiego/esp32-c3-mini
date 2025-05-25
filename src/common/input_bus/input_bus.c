#include "input_bus.h"


#define MAX_SUBS 8

static encoder_event_cb_t encoder_subs[MAX_SUBS] = {0};
static button_event_cb_t button_subs[MAX_SUBS] = {0};

// --- Encoder subscriptions ---
void input_bus_add_encoder_sub(encoder_event_cb_t cb) {
    for (int i = 0; i < MAX_SUBS; i++) {
        if (encoder_subs[i] == NULL) {
            encoder_subs[i] = cb;
            break;
        }
    }
}

void input_bus_remove_encoder_sub(encoder_event_cb_t cb) {
    for (int i = 0; i < MAX_SUBS; i++) {
        if (encoder_subs[i] == cb) {
            encoder_subs[i] = NULL;
            break;
        }
    }
}

// --- Button subscriptions ---
void input_bus_add_button_sub(button_event_cb_t cb) {
    for (int i = 0; i < MAX_SUBS; i++) {
        if (button_subs[i] == NULL) {
            button_subs[i] = cb;
            break;
        }
    }
}

void input_bus_remove_button_sub(button_event_cb_t cb) {
    for (int i = 0; i < MAX_SUBS; i++) {
        if (button_subs[i] == cb) {
            button_subs[i] = NULL;
            break;
        }
    }
}

// --- Emit events ---
void input_bus_emit_encoder_event(long pos, int32_t change) {
    for (int i = 0; i < MAX_SUBS; i++) {
        if (encoder_subs[i]) encoder_subs[i](pos, change);
    }
}

void input_bus_emit_button_event(bool pressed) {
    for (int i = 0; i < MAX_SUBS; i++) {
        if (button_subs[i]) button_subs[i](pressed);
    }
}
