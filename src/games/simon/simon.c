// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.2
// LVGL version: 8.3.6
// Project name: nav

/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.
    For a copy, see <https://opensource.org/licenses/MIT>.
*/

#include "simon.h"

#ifdef ENABLE_GAME_SIMON

#define NUM_PANELS 4
#define MAX_SEQUENCE_LENGTH 50 // Game max levels

uint16_t tones[NUM_PANELS] = {220 * 2, 247 * 2, 277 * 2, 311 * 2};
uint32_t panel_colors[NUM_PANELS] = {0x00801F, 0xD20000, 0xCDED07, 0x0A36E8}; // default colors
// uint32_t panel_colors[NUM_PANELS] = {0xF35F20, 0x343839, 0x62D367, 0x3C65F8}; // lvgl logo colors
int sequence[MAX_SEQUENCE_LENGTH];
int player_sequence[MAX_SEQUENCE_LENGTH];
int current_step = 0;
int sequence_index = 0;
int current_sequence_length = 1; // Start with a sequence of 1
bool is_player_turn = false;
int highScoreSimon;


REGISTER_APP("Simon Says", &ui_img_colors_png, ui_simonScreen, ui_simonScreen_screen_init);

lv_obj_t *ui_simonMainPanel;
lv_obj_t *ui_simonScorePanel;
lv_obj_t *ui_simonScoreText;
lv_obj_t *ui_simonArc;
lv_obj_t *ui_simonOverPanel;
lv_obj_t *panels[NUM_PANELS];
lv_obj_t *ui_simonPanel;
lv_obj_t *ui_simonTitle;
lv_obj_t *ui_simonInfo;
lv_obj_t *ui_simonLabel;
lv_obj_t *ui_simonScore;
void ui_event_simonStart(lv_event_t *e);
lv_obj_t *ui_simonStart;
lv_obj_t *ui_simonButtonText;
void ui_event_exitSimon(lv_event_t *e);
lv_obj_t *ui_exitSimon;
lv_obj_t *ui_exitSimonText;

typedef struct
{
    lv_obj_t *panel;
    uint16_t tone;
} panel_data_t;

typedef struct
{
    const char *title;
    const char *message;
} score_info_t;

score_info_t levels[] = {
    {"Beginner", "You are just starting. Keep practicing!"},
    {"Learner", "You're getting the hang of it!"},
    {"Reminiscer", "You're recalling some details now!"},
    {"Frequent", "You're improving with regular attempts!"},
    {"Intermediate", "You're halfway there. Stay focused!"},
    {"Reliable", "You're consistent with your memory!"},
    {"Proficient", "You're handling things with confidence!"},
    {"Sharp", "Your memory is sharp and reliable!"},
    {"Expert", "You've mastered most of the challenges!"},
    {"Master", "You're a memory master. Exceptional work!"},
    {"Champion", "The G.O.A.T! Increase MAX_SEQUENCE_LENGTH for more."}
};

void ui_event_simonScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_SCREEN_LOAD_START)
    {
        onGameOpened();

        highScoreSimon = getPrefInt("simon_high", 0);

        lv_label_set_text(ui_simonButtonText, "Start");
        lv_label_set_text(ui_simonLabel, "High Score");
        lv_label_set_text(ui_simonInfo, "");
        lv_label_set_text(ui_simonScoreText, "0");
        lv_label_set_text_fmt(ui_simonScore, "%d", highScoreSimon);
        lv_obj_remove_flag(ui_simonPanel, LV_OBJ_FLAG_HIDDEN); /// Flags

        simonTone(0, 0);
    }
    if (event_code == LV_EVENT_SCREEN_UNLOAD_START)
    {
    }
    if (event_code == LV_EVENT_SCREEN_UNLOADED)
    {
        onGameClosed();

        lv_obj_delete(ui_simonScreen);
        ui_simonScreen = NULL;
    }

    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_RIGHT)
    {
        if (lv_obj_has_flag(ui_simonPanel, LV_OBJ_FLAG_HIDDEN))
        {
            lv_obj_remove_flag(ui_simonPanel, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            ui_app_exit();
        }
    }
}

void ui_event_simonStart(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        lv_obj_add_flag(ui_simonPanel, LV_OBJ_FLAG_HIDDEN); /// Flags

        start_game();
    }
}

void remove_state_cb(lv_timer_t *timer)
{
    lv_obj_t *panel = (lv_obj_t *)timer->user_data;
    lv_obj_remove_state(panel, LV_STATE_PRESSED);

    play_sequence(NULL);
}

void add_state_cb(lv_timer_t *timer)
{
    panel_data_t *data = (panel_data_t *)timer->user_data;
    lv_obj_add_state(data->panel, LV_STATE_PRESSED);

    lv_timer_t *remove_t = lv_timer_create(remove_state_cb, 500, data->panel);
    lv_timer_set_repeat_count(remove_t, 1);
    lv_timer_set_auto_delete(remove_t, true);

    simonTone(2, data->tone);
}

/* Event handler for when a panel is clicked */
void panel_event_handler(lv_event_t *e)
{

    if (!is_player_turn)
        return;

    lv_obj_t *panel = lv_event_get_target(e);
    int panel_idx = (intptr_t)lv_event_get_user_data(e);

    // Check if the clicked panel matches the current step in the sequence
    if (panel_idx != sequence[current_step])
    {
        simonTone(1, 0);

        lv_label_set_text_fmt(ui_simonScore, "%d/%d", current_sequence_length - 1, MAX_SEQUENCE_LENGTH);
        float div = (MAX_SEQUENCE_LENGTH / 10.0);
        if (div == 0){
            div = 1; // prevent divide by zero
        }
        lv_label_set_text(ui_simonLabel, "Game Over!");
        lv_label_set_text(ui_simonLabel, levels[(int)((current_sequence_length - 1) / div)].title);
        lv_label_set_text(ui_simonInfo, levels[(int)((current_sequence_length - 1) / div)].message);

        lv_obj_remove_flag(ui_simonPanel, LV_OBJ_FLAG_HIDDEN);
        if (current_sequence_length - 1 > highScoreSimon)
        {
            highScoreSimon = current_sequence_length - 1;

            savePrefInt("simon_high", highScoreSimon);
        }
        return;
    }

    simonTone(2, tones[panel_idx]);

    current_step++;

    lv_arc_set_value(ui_simonArc, (int32_t)((current_step * 360.0) / current_sequence_length));
    lv_obj_set_style_arc_color(ui_simonArc, lv_color_hex(0x50FF00), LV_PART_INDICATOR | LV_STATE_DEFAULT);

    // Check if the player has completed the current sequence
    if (current_step == current_sequence_length)
    {
        if (current_sequence_length == MAX_SEQUENCE_LENGTH)
        {

            highScoreSimon = current_sequence_length;
            lv_label_set_text_fmt(ui_simonScore, "%d", highScoreSimon);
            // lv_label_set_text(ui_simonLabel, "You Win!");
            lv_label_set_text(ui_simonLabel, levels[10].title);
            lv_label_set_text(ui_simonInfo, levels[10].message);

            lv_obj_remove_flag(ui_simonPanel, LV_OBJ_FLAG_HIDDEN);

            savePrefInt("simon_high", highScoreSimon);
        }
        else
        {
            // Proceed to the next round
            current_sequence_length++;

            lv_obj_remove_flag(ui_simonOverPanel, LV_OBJ_FLAG_HIDDEN);
            lv_arc_set_value(ui_simonArc, 360);
            lv_obj_set_style_arc_color(ui_simonArc, lv_color_hex(0x50FF00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_timer_t *next_round_timer = lv_timer_create(play_sequence, 1000, NULL); // Wait 1 second before the next round
            lv_timer_set_repeat_count(next_round_timer, 1);
            lv_timer_set_auto_delete(next_round_timer, true);
        }
    }
}

/* Function to generate random sequence */
void generate_sequence()
{
    for (int i = 0; i < MAX_SEQUENCE_LENGTH; i++)
    {
        sequence[i] = lv_rand(0, 3); // Random panel between 0 and 3
    }
}

/* Function to highlight a panel */
void flash_panel(int panel_idx)
{
    panel_data_t *data = lv_malloc(sizeof(panel_data_t));
    data->panel = panels[panel_idx];
    data->tone = tones[panel_idx];
    lv_timer_t *timer = lv_timer_create(add_state_cb, 500, data);
    lv_timer_set_repeat_count(timer, 1);
    lv_timer_set_auto_delete(timer, true);
}

/* Function to play the sequence */
void play_sequence(lv_timer_t *timer)
{
    lv_obj_remove_flag(ui_simonOverPanel, LV_OBJ_FLAG_HIDDEN);
    lv_arc_set_value(ui_simonArc, 360);
    lv_obj_set_style_arc_color(ui_simonArc, lv_color_hex(0xCCFFF9), LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_label_set_text_fmt(ui_simonScoreText, "%d", current_sequence_length);
    is_player_turn = false;
    if (sequence_index < current_sequence_length)
    {
        flash_panel(sequence[sequence_index]);
        sequence_index++;
    }
    else
    {
        sequence_index = 0;
        is_player_turn = true;
        current_step = 0; // Reset player input step
        lv_obj_add_flag(ui_simonOverPanel, LV_OBJ_FLAG_HIDDEN);
        lv_arc_set_value(ui_simonArc, 0);
        lv_obj_set_style_arc_color(ui_simonArc, lv_color_hex(0x2A2A2A), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    }
}

/* Start the Simon Says game */
void start_game()
{
    generate_sequence();
    current_step = 0;
    sequence_index = 0;
    current_sequence_length = 1; // Start with sequence of length 1
    play_sequence(NULL);
}


void ui_event_exitSimon(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        ui_app_exit();
    }
}

void ui_simonScreen_screen_init()
{

    ui_simonScreen = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_simonScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_simonScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_simonScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_simonMainPanel = lv_obj_create(ui_simonScreen);
    lv_obj_set_width(ui_simonMainPanel, 240);
    lv_obj_set_height(ui_simonMainPanel, 240);
    lv_obj_set_align(ui_simonMainPanel, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_simonMainPanel, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(ui_simonMainPanel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_remove_flag(ui_simonMainPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_simonMainPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_simonMainPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_simonMainPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_simonMainPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_simonMainPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_simonMainPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_simonMainPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_simonMainPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_simonMainPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_simonMainPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    for (int i = 0; i < NUM_PANELS; i++)
    {
        panels[i] = lv_obj_create(ui_simonMainPanel);
        lv_obj_set_width(panels[i], 120);
        lv_obj_set_height(panels[i], 120);
        lv_obj_set_align(panels[i], LV_ALIGN_CENTER);
        lv_obj_remove_flag(panels[i], LV_OBJ_FLAG_SCROLLABLE); /// Flags
        lv_obj_set_style_radius(panels[i], 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(panels[i], lv_color_hex(panel_colors[i]), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(panels[i], 150, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(panels[i], lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(panels[i], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(panels[i], 3, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(panels[i], lv_color_hex(panel_colors[i]), LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(panels[i], 255, LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_border_color(panels[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_border_opa(panels[i], 255, LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_border_width(panels[i], 1, LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_outline_width(panels[i], 0, LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_outline_pad(panels[i], 0, LV_PART_MAIN | LV_STATE_PRESSED);

        lv_obj_add_event_cb(panels[i], panel_event_handler, LV_EVENT_CLICKED, (void *)(intptr_t)i);
    }

    ui_simonScorePanel = lv_obj_create(ui_simonScreen);
    lv_obj_set_width(ui_simonScorePanel, 60);
    lv_obj_set_height(ui_simonScorePanel, 60);
    lv_obj_set_align(ui_simonScorePanel, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_simonScorePanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_simonScorePanel, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_simonScorePanel, lv_color_hex(0x171717), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_simonScorePanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_simonScorePanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_simonScorePanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_simonScorePanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(ui_simonScorePanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_simonScorePanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_pad(ui_simonScorePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_simonScoreText = lv_label_create(ui_simonScorePanel);
    lv_obj_set_width(ui_simonScoreText, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_simonScoreText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_simonScoreText, LV_ALIGN_CENTER);
    lv_label_set_text(ui_simonScoreText, "0");
    lv_obj_set_style_text_font(ui_simonScoreText, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_simonArc = lv_arc_create(ui_simonScorePanel);
    lv_obj_set_width(ui_simonArc, 60);
    lv_obj_set_height(ui_simonArc, 60);
    lv_obj_set_align(ui_simonArc, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_simonArc, LV_OBJ_FLAG_CLICKABLE); /// Flags
    lv_arc_set_range(ui_simonArc, 0, 360);
    lv_arc_set_value(ui_simonArc, 360);
    lv_arc_set_bg_angles(ui_simonArc, 0, 360);
    lv_arc_set_rotation(ui_simonArc, 270);
    lv_obj_set_style_arc_width(ui_simonArc, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(ui_simonArc, lv_color_hex(0xD2D2FF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_simonArc, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_simonArc, 5, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_simonArc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_simonArc, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_simonArc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_simonOverPanel = lv_obj_create(ui_simonScreen);
    lv_obj_set_width(ui_simonOverPanel, 240);
    lv_obj_set_height(ui_simonOverPanel, 240);
    lv_obj_set_align(ui_simonOverPanel, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_simonOverPanel, LV_OBJ_FLAG_HIDDEN);       /// Flags
    lv_obj_remove_flag(ui_simonOverPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_simonOverPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_simonOverPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_simonOverPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_simonOverPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_simonPanel = lv_obj_create(ui_simonScreen);
    lv_obj_set_width(ui_simonPanel, 240);
    lv_obj_set_height(ui_simonPanel, 240);
    lv_obj_set_align(ui_simonPanel, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_simonPanel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_simonPanel, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(ui_simonPanel, LV_OBJ_FLAG_HIDDEN);       /// Flags
    lv_obj_remove_flag(ui_simonPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_simonPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_simonPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_simonPanel, 230, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_simonPanel, lv_color_hex(0x968100), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_simonPanel, 200, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_simonPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_simonPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_simonPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_simonPanel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_simonPanel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_simonTitle = lv_label_create(ui_simonPanel);
    lv_obj_set_width(ui_simonTitle, 150);
    lv_obj_set_height(ui_simonTitle, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_simonTitle, LV_ALIGN_CENTER);
    lv_label_set_text(ui_simonTitle, "Simon Says");
    lv_obj_set_style_text_align(ui_simonTitle, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_simonTitle, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_simonTitle, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_simonTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_simonTitle, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_simonTitle, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_simonLabel = lv_label_create(ui_simonPanel);
    lv_obj_set_width(ui_simonLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_simonLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_simonLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_simonLabel, "Your Score");
    lv_obj_set_style_text_font(ui_simonLabel, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_simonScore = lv_label_create(ui_simonPanel);
    lv_obj_set_width(ui_simonScore, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_simonScore, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_simonScore, LV_ALIGN_CENTER);
    lv_label_set_text(ui_simonScore, "0");
    lv_obj_set_style_text_color(ui_simonScore, lv_color_hex(0x18F80E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_simonScore, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_simonScore, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_simonInfo = lv_label_create(ui_simonPanel);
    lv_obj_set_width(ui_simonInfo, 180);              /// 1
    lv_obj_set_height(ui_simonInfo, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_simonInfo, LV_ALIGN_CENTER);
    lv_label_set_text(ui_simonInfo, "Start");
    lv_obj_set_style_text_opa(ui_simonInfo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_simonInfo, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_simonInfo, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_simonStart = lv_button_create(ui_simonPanel);
    lv_obj_set_width(ui_simonStart, 100);
    lv_obj_set_height(ui_simonStart, 40);
    lv_obj_set_align(ui_simonStart, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_simonStart, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_remove_flag(ui_simonStart, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_radius(ui_simonStart, 20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_simonButtonText = lv_label_create(ui_simonStart);
    lv_obj_set_width(ui_simonButtonText, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_simonButtonText, LV_SIZE_CONTENT); /// 40
    lv_obj_set_align(ui_simonButtonText, LV_ALIGN_CENTER);
    lv_label_set_text(ui_simonButtonText, "Start");

    ui_exitSimon = lv_button_create(ui_simonPanel);
    lv_obj_set_width(ui_exitSimon, 100);
    lv_obj_set_height(ui_exitSimon, 40);
    lv_obj_set_align(ui_exitSimon, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_exitSimon, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_remove_flag(ui_exitSimon, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_radius(ui_exitSimon, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_exitSimon, lv_color_hex(0xF30000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_exitSimon, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_exitSimonText = lv_label_create(ui_exitSimon);
    lv_obj_set_width(ui_exitSimonText, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_exitSimonText, LV_SIZE_CONTENT); /// 40
    lv_obj_set_align(ui_exitSimonText, LV_ALIGN_CENTER);
    lv_label_set_text(ui_exitSimonText, "Exit");

    lv_obj_add_event_cb(ui_simonStart, ui_event_simonStart, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_exitSimon, ui_event_exitSimon, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_simonScreen, ui_event_simonScreen, LV_EVENT_ALL, NULL);


}

#endif

