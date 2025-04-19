

#ifndef FEEDBACK_H
#define FEEDBACK_H

#define TONE_AN 220 // 440 Hz
#define TONE_AS 233 // 466 Hz
#define TONE_BN 247 // 493 Hz
#define TONE_CN 261 // 523 Hz
#define TONE_CS 277 // 554 Hz
#define TONE_DN 294 // 588 Hz
#define TONE_DS 311 // 622 Hz
#define TONE_EN 330 // 658 Hz
#define TONE_FN 349 // 698 Hz
#define TONE_FS 370 // 740 Hz
#define TONE_GN 392 // 784 Hz
#define TONE_GS 415 // 830 Hz

#define BUZZER_CHANNEL 0

#define MAX_QUEUE_SIZE 8
#define MAX_NOTES_PER_SEQUENCE 16

#define MAX_VIB_PATTERN_LENGTH 16

enum ToneType
{
    T_ALARM = 0,
    T_TIMER,
    T_CALLS,
    T_NOTIFICATION,
    T_SYSTEM,
    T_USER
};

struct Note
{
    int pitch;
    int duration;
};

struct Vibration
{
    bool on;
    int duration;
};

enum AlertType
{
    ALERT_POPUP = 0x01,
    ALERT_SCREEN = 0x02,
    ALERT_SOUND = 0x04,
    ALERT_VIBRATE = 0x08
};

extern Vibration pattern[];
extern Vibration v_notif[];

extern Note tone_startup[];
extern Note tone_off[];
extern Note tone_notification[];
extern Note tone_call[];
extern Note tone_alarm[];
extern Note tone_timer[];
extern Note tone_button[];

extern Note tone_simonsays_intro[];
extern Note tone_simonsays_gameover[];

void startToneSystem();
void startVibrationSystem();

void feedbackTone(Note *notes, int count, ToneType type, int repeat = 0);
void feedbackVibrate(Vibration *steps, int count, bool force = false);

void screen_on(long extra = 0);

void feedbackRun(ToneType type);

bool check_alert_state(AlertType type);

#endif