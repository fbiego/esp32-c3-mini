
#include <Arduino.h>
#include "feedback.h"
#include "app_hal.h"
#include "displays/pins.h"

// A struct to hold a sequence of notes
struct NoteSequence
{
    Note notes[MAX_NOTES_PER_SEQUENCE];
    int count;
    int repeat;
};

struct VibPattern
{
    Vibration steps[MAX_VIB_PATTERN_LENGTH];
    int count;
};

QueueHandle_t noteQueue;
TaskHandle_t toneTaskHandle = nullptr;
volatile bool playing = false;
volatile ToneType currentToneType = T_USER;
volatile bool forceInterrupt = false;

QueueHandle_t vibQueue;
TaskHandle_t vibTaskHandle = nullptr;
volatile bool vibrating = false;

Note tone_melody[] = {
    {440, 300}, // A4
    {0, 150},   // Rest
    {494, 300}, // B4
    {0, 150},   // Rest
    {523, 300}, // C5
    {0, 300},   // Longer rest
    {587, 300}  // D5
};

Note tone_startup[] = {
    {TONE_EN * 2, 170},
    {TONE_FS * 2, 170},
    {TONE_GN * 2, 170}};

// 📩 Notification Received (Double Beep)
Note tone_notification[] = {
    {1000, 100},
    {0, 100},
    {1200, 100},
};

// 📞 Call Received (Melody)
Note tone_call[] = {
    {880, 200},
    {988, 200},
    {1046, 200},
    {0, 100},
    {1046, 300},
    {880, 200},
    {988, 200},
    {0, 200},
};

// ⏰ Alarm Ring
Note tone_alarm[] = {
    {1500, 150},
    {0, 100},
    {1500, 150},
    {0, 100},
    {1500, 150},
    {0, 100},
    {2000, 300},
};

Note tone_off[] = {
    {0, 50}};

// ⏲️ Timer Ended
Note tone_timer[] = {
    {880, 200},
    {0, 100},
    {660, 200},
    {0, 100},
    {880, 200},
    {0, 100},
};

// 🔘 Button Press
Note tone_button[] = {
    {1000, 50},
};

// 🎮 Simon Says - Intro
Note tone_simonsays_intro[] = {
    {659, 150},
    {784, 150},
    {987, 150},
    {1319, 200},
};

// 🎮 Simon Says - Game Over
Note tone_simonsays_gameover[] = {
    {659, 300},
    {622, 300},
    {587, 300},
    {554, 400},
};

Vibration v_notif[] = {
    {true, 100}, // ON for 200ms
    {false, 50}, // OFF for 100ms
};

Vibration pattern[] = {
    {true, 200},  // ON for 200ms
    {false, 100}, // OFF for 100ms
    {true, 300},  // ON for 300ms
    {false, 200}, // OFF for 200ms
};

void toneTask(void *param)
{
#if defined(BUZZER_PIN) && (BUZZER_PIN != -1)

    NoteSequence sequence;

    while (true)
    {
        if (xQueueReceive(noteQueue, &sequence, portMAX_DELAY) == pdTRUE)
        {
            playing = true;

            ledcAttach(BUZZER_PIN, 500, 8);

            if (sequence.repeat == 0)
            {
                sequence.repeat = 1;
            }

            for (int r = 0; r < sequence.repeat; r++)
            {
                for (int n = 0; n < sequence.count; n++)
                {
                    if (forceInterrupt)
                    {
                        forceInterrupt = false;
                        break;
                    }

                    int pitch = sequence.notes[n].pitch;
                    int duration = sequence.notes[n].duration;

                    if (pitch > 0)
                    {
                        ledcWriteTone(BUZZER_PIN, pitch);
                    }
                    else
                    {
                        ledcWriteTone(BUZZER_PIN, 0); // Rest
                    }
                    vTaskDelay(pdMS_TO_TICKS(duration));
                }
            }

            ledcDetach(BUZZER_PIN);

            playing = false;
        }
    }
#endif
}

void vibrationTask(void *param)
{
#if defined(VIBRATION_PIN) && (VIBRATION_PIN != -1)
#ifndef ELECROW_C3
    pinMode(VIBRATION_PIN, OUTPUT);
#endif
    VibPattern pattern;

    while (true)
    {
        if (xQueueReceive(vibQueue, &pattern, portMAX_DELAY) == pdTRUE)
        {
            vibrating = true;

#ifdef ELECROW_C3
            vibratePin(true);
            for (int i = 0; i < pattern.count; i++)
            {
                delay(pattern.steps[i].duration);
            }
#else
            for (int i = 0; i < pattern.count; i++)
            {
                vibratePin(pattern.steps[i].on);
                delay(pattern.steps[i].duration);
            }
#endif

            vibratePin(false);
            vibrating = false;
        }
    }
#endif
}

void startToneSystem()
{
#if defined(BUZZER_PIN) && (BUZZER_PIN != -1)

    if (!noteQueue)
    {
        noteQueue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(NoteSequence));
    }
    if (!toneTaskHandle)
    {
        xTaskCreatePinnedToCore(toneTask, "toneTask", 2048, nullptr, 1, &toneTaskHandle, 0);
    }
#endif
}

void startVibrationSystem()
{
#if defined(VIBRATION_PIN) && (VIBRATION_PIN != -1)
    if (!vibQueue)
    {
        vibQueue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(VibPattern));
    }
    if (!vibTaskHandle)
    {
        xTaskCreatePinnedToCore(vibrationTask, "vibrationTask", 2048, nullptr, 1, &vibTaskHandle, 0);
    }
#endif
}

void feedbackTone(Note *notes, int count, ToneType type, int repeat)
{
#if defined(BUZZER_PIN) && (BUZZER_PIN != -1)
    if (check_alert_state(ALERT_SOUND))
    {
        if (!noteQueue)
            startToneSystem();

        if (count > MAX_NOTES_PER_SEQUENCE)
            count = MAX_NOTES_PER_SEQUENCE;

        if (playing && currentToneType >= type)
        {
            // If the current tone is of the same or higher priority, reset the queue
            forceInterrupt = true;
            xQueueReset(noteQueue);
        }

        NoteSequence sequence;
        sequence.count = count;
        sequence.repeat = repeat;
        memcpy(sequence.notes, notes, count * sizeof(Note));

        xQueueSend(noteQueue, &sequence, 0);
        currentToneType = type;
    }
#endif
}

void feedbackVibrate(Vibration *steps, int count, bool force)
{
#if defined(VIBRATION_PIN) && (VIBRATION_PIN != -1)
    if (check_alert_state(ALERT_VIBRATE))
    {
        if (!vibQueue)
            startVibrationSystem();

        if (count > MAX_VIB_PATTERN_LENGTH)
            count = MAX_VIB_PATTERN_LENGTH;

        if (force && vibrating)
        {
            xQueueReset(vibQueue);
        }

        VibPattern pattern;
        pattern.count = count;
        memcpy(pattern.steps, steps, count * sizeof(Vibration));

        xQueueSend(vibQueue, &pattern, 0);
    }
#endif
}

void feedbackRun(ToneType type)
{
    switch (type)
    {
    case T_CALLS:
        feedbackTone(tone_call, 8, T_CALLS, 3);
        feedbackVibrate(pattern, 4, true);
        break;
    case T_NOTIFICATION:
        feedbackTone(tone_notification, 3, T_NOTIFICATION, 2);
        feedbackVibrate(v_notif, 2, true);
        break;
    case T_TIMER:
        feedbackTone(tone_alarm, 7, T_TIMER);
        feedbackVibrate(pattern, 4, true);
        break;
    case T_ALARM:
        feedbackTone(tone_timer, 6, T_TIMER, 3);
        feedbackVibrate(pattern, 4, true);
        break;
    case T_SYSTEM:
        feedbackTone(tone_button, 1, T_SYSTEM);
        feedbackVibrate(pattern, 2, true);
        break;
    default:
        break;
    }


    if (check_alert_state(ALERT_SCREEN))
    {
        switch (type)
        {
        case T_CALLS:
            screen_on(50);
            break;
        default:
            screen_on();
            break;
        }
    }
}
