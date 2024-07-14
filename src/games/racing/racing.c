

#include "racing.h"

#ifdef ENABLE_GAME_RACING

#define ROAD_HEIGHT 270
#define NPC_CAR_HEIGHT 400
#define NPC_CAR_OFFSET_Y -110
#define NPC_CAR_X_OFFSET -50
#define SPEED_DIVISOR 5000.0
#define SCORE_MULTIPLIER 10
#define MIN_SPEED 0.5
#define INITIAL_SPEED 3.5
#define SPEED_LIMIT_FACTOR 3.5

void ui_event_raceScreen(lv_event_t *e);
lv_obj_t *ui_raceScreen;
void ui_event_roadPanel(lv_event_t *e);
lv_obj_t *ui_roadPanel;
lv_obj_t *ui_roadImage;
lv_obj_t *ui_distanceLabel;
lv_obj_t *ui_speedLabel;
lv_obj_t *ui_carPlayer;
void ui_event_leftButton(lv_event_t *e);
lv_obj_t *ui_leftButton;
lv_obj_t *ui_leftButtonText;
void ui_event_rightButton(lv_event_t *e);
lv_obj_t *ui_rightButton;
lv_obj_t *ui_rightButtonText;
lv_obj_t *ui_carNPC1;
lv_obj_t *ui_carNPC2;
lv_obj_t *ui_carNPC3;
lv_obj_t *ui_racePanel;
lv_obj_t *ui_raceTitle;
lv_obj_t *ui_raceInfo;
lv_obj_t *ui_raceLabel;
lv_obj_t *ui_raceScore;
void ui_event_raceStart(lv_event_t *e);
lv_obj_t *ui_raceStart;
lv_obj_t *ui_raceButtonText;
void ui_event_exitRace(lv_event_t *e);
lv_obj_t *ui_exitRace;
lv_obj_t *ui_exitRaceText;

void ui_event_roadPanel(lv_event_t *e);

bool active;

int highScore, currentScore;
bool isRacing, gameOver;
float speed = 1;
float m = INITIAL_SPEED;
float speedFactor;
float roadProgress = 0.0f;
float npcProgress = 0.0f;
uint64_t counter;
float npcSpeed = INITIAL_SPEED / 1.5;

bool gen = false;
int rd;

typedef struct CarPosition
{
    int x;
    int y;
} CarP;

const lv_img_dsc_t *carIcons[] = {
    &ui_img_car_green_png,
    &ui_img_car_red_png,
    &ui_img_car_yellow_png,
};

CarP player;
CarP npc1;
CarP npcBuf;
CarP plBuf;

// Function to check if two cars have crashed into each other
bool haveCrashed(CarP car1, CarP car2)
{
    // Calculate the absolute differences in x and y coordinates
    int dx = LV_ABS(car1.x - car2.x);
    int dy = LV_ABS(car1.y - car2.y);

    // Check if the differences are less than the minimum required distances
    // x 28  y 58
    if (dx < 28 && dy < 58)
    {
        return true; // The cars have crashed
    }
    else
    {
        return false; // The cars have not crashed
    }
}

void ui_event_raceScreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_SCREEN_LOAD_START)
    {
        onGameOpened();
        active = true;
        isRacing = false;

        lv_label_set_text(ui_distanceLabel, "0");
        lv_label_set_text(ui_speedLabel, "0");
        lv_label_set_text(ui_raceButtonText, "Start");
        lv_label_set_text(ui_raceLabel, "High Score");
        lv_label_set_text_fmt(ui_raceScore, "%dm", highScore);
        lv_obj_clear_flag(ui_racePanel, LV_OBJ_FLAG_HIDDEN); /// Flags
        lv_obj_add_flag(ui_raceInfo, LV_OBJ_FLAG_HIDDEN);    /// Flags

        currentScore = 0;
        gameOver = false;
        counter = 0;
        npc1.x = 0;
        npc1.y = 0;
        player.x = 0;
        player.y = 160;
        roadProgress = 0.0f;
        npcProgress = 0.0f;
        npcSpeed = INITIAL_SPEED / 1.5;
        gen = true;
    }
    if (event_code == LV_EVENT_SCREEN_UNLOAD_START)
    {
        active = false;
        isRacing = false;
    }
    if (event_code == LV_EVENT_SCREEN_UNLOADED)
    {
        onGameClosed();
    }
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
    {
        if (isRacing)
        {
            ui_event_roadPanel(e);
        }
        else
        {
            ui_gameExit();
        }
    }
}

void ui_event_roadPanel(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        if (isRacing)
        {
            isRacing = false;
            if (!gameOver)
            {
                lv_label_set_text(ui_raceButtonText, "Continue");
                lv_label_set_text(ui_raceLabel, "Paused");
                lv_label_set_text_fmt(ui_raceScore, "%dm", currentScore);
                lv_obj_add_flag(ui_raceInfo, LV_OBJ_FLAG_HIDDEN); /// Flags
            }
            else
            {
                lv_label_set_text(ui_raceButtonText, "Start");
                lv_label_set_text(ui_raceLabel, "Your Score");
                lv_label_set_text_fmt(ui_raceScore, "%dm", currentScore);
                lv_obj_clear_flag(ui_raceInfo, LV_OBJ_FLAG_HIDDEN); /// Flags
            }
            lv_obj_clear_flag(ui_racePanel, LV_OBJ_FLAG_HIDDEN); /// Flags
        }
    }
}

void ui_event_leftButton(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_PRESSING)
    {
        player.x--;
        if (player.x < -60)
        {
            player.x = -60;
        }
    }
}

void ui_event_rightButton(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_PRESSING)
    {
        player.x++;
        if (player.x > 60)
        {
            player.x = 60;
        }
    }
}

void ui_event_raceStart(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        if (gameOver)
        {
            counter = 0;
            gameOver = false;
            npc1.x = 0;
            npc1.y = 0;
            player.x = 0;
            player.y = 160;
            npcSpeed = INITIAL_SPEED / 1.5;
            roadProgress = 0.0f;
            npcProgress = 0.0f;
            gen = true;
        }
        isRacing = true;
        lv_obj_add_flag(ui_racePanel, LV_OBJ_FLAG_HIDDEN); /// Flags
    }
}

#endif

void ui_event_exitRace(lv_event_t *e)
{
#ifdef ENABLE_GAME_RACING
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        ui_gameExit();
    }
#endif
}

void ui_raceScreen_screen_init(void (*callback)(const char *, const lv_img_dsc_t *, lv_obj_t **))
{
#ifdef ENABLE_GAME_RACING
    ui_raceScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_raceScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_raceScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_raceScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_roadPanel = lv_obj_create(ui_raceScreen);
    lv_obj_set_width(ui_roadPanel, 150);
    lv_obj_set_height(ui_roadPanel, 240);
    lv_obj_set_align(ui_roadPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_roadPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_border_width(ui_roadPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_roadPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_roadPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_roadPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_roadPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_roadImage = lv_img_create(ui_roadPanel);
    lv_img_set_src(ui_roadImage, &ui_img_road_png);
    lv_obj_set_width(ui_roadImage, 150);
    lv_obj_set_height(ui_roadImage, 540);
    lv_obj_set_align(ui_roadImage, LV_ALIGN_BOTTOM_MID);
    lv_obj_add_flag(ui_roadImage, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_roadImage, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_distanceLabel = lv_label_create(ui_raceScreen);
    lv_obj_set_width(ui_distanceLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_distanceLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_distanceLabel, 30);
    lv_obj_set_y(ui_distanceLabel, 51);
    lv_label_set_text(ui_distanceLabel, "0");
    lv_obj_set_style_radius(ui_distanceLabel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_distanceLabel, lv_color_hex(0xAC8900), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_distanceLabel, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_distanceLabel, lv_color_hex(0xAC8900), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(ui_distanceLabel, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_distanceLabel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_pad(ui_distanceLabel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_speedLabel = lv_label_create(ui_raceScreen);
    lv_obj_set_width(ui_speedLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_speedLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_speedLabel, -30);
    lv_obj_set_y(ui_speedLabel, 51);
    lv_obj_set_align(ui_speedLabel, LV_ALIGN_TOP_RIGHT);
    lv_label_set_text(ui_speedLabel, "0");
    lv_obj_set_style_radius(ui_speedLabel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_speedLabel, lv_color_hex(0xAC8900), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_speedLabel, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_speedLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_speedLabel, lv_color_hex(0xAC8900), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(ui_speedLabel, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_speedLabel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_pad(ui_speedLabel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_carPlayer = lv_img_create(ui_raceScreen);
    lv_img_set_src(ui_carPlayer, &ui_img_car_png);
    lv_obj_set_width(ui_carPlayer, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_carPlayer, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_carPlayer, 0);
    lv_obj_set_y(ui_carPlayer, 160);
    lv_obj_set_align(ui_carPlayer, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(ui_carPlayer, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_carPlayer, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_leftButton = lv_btn_create(ui_raceScreen);
    lv_obj_set_width(ui_leftButton, 50);
    lv_obj_set_height(ui_leftButton, 100);
    lv_obj_set_x(ui_leftButton, -100);
    lv_obj_set_y(ui_leftButton, 1);
    lv_obj_set_align(ui_leftButton, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_leftButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_leftButton, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_radius(ui_leftButton, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_leftButton, lv_color_hex(0x665104), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_leftButton, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_leftButton, lv_color_hex(0x4B9F31), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_leftButton, 150, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui_leftButton, lv_color_hex(0x4B9F31), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(ui_leftButton, 150, LV_PART_MAIN | LV_STATE_PRESSED);

    ui_leftButtonText = lv_label_create(ui_leftButton);
    lv_obj_set_width(ui_leftButtonText, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_leftButtonText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_leftButtonText, LV_ALIGN_CENTER);
    lv_label_set_text(ui_leftButtonText, "<");
    lv_obj_set_style_text_font(ui_leftButtonText, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_rightButton = lv_btn_create(ui_raceScreen);
    lv_obj_set_width(ui_rightButton, 50);
    lv_obj_set_height(ui_rightButton, 100);
    lv_obj_set_x(ui_rightButton, 100);
    lv_obj_set_y(ui_rightButton, 1);
    lv_obj_set_align(ui_rightButton, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_rightButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_rightButton, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_radius(ui_rightButton, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_rightButton, lv_color_hex(0x665104), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_rightButton, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_rightButton, lv_color_hex(0x4B9F31), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(ui_rightButton, 150, LV_PART_MAIN | LV_STATE_PRESSED);

    ui_rightButtonText = lv_label_create(ui_rightButton);
    lv_obj_set_width(ui_rightButtonText, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_rightButtonText, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_rightButtonText, LV_ALIGN_CENTER);
    lv_label_set_text(ui_rightButtonText, ">");
    lv_obj_set_style_text_font(ui_rightButtonText, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_carNPC1 = lv_img_create(ui_raceScreen);
    lv_img_set_src(ui_carNPC1, &ui_img_car_green_png);
    lv_obj_set_width(ui_carNPC1, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_carNPC1, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_carNPC1, 20);
    lv_obj_set_y(ui_carNPC1, -1);
    lv_obj_set_align(ui_carNPC1, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(ui_carNPC1, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_carNPC1, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_carNPC2 = lv_img_create(ui_raceScreen);
    lv_img_set_src(ui_carNPC2, &ui_img_car_red_png);
    lv_obj_set_width(ui_carNPC2, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_carNPC2, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_carNPC2, -21);
    lv_obj_set_y(ui_carNPC2, -60);
    lv_obj_set_align(ui_carNPC2, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(ui_carNPC2, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_carNPC2, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_carNPC3 = lv_img_create(ui_raceScreen);
    lv_img_set_src(ui_carNPC3, &ui_img_car_yellow_png);
    lv_obj_set_width(ui_carNPC3, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_carNPC3, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_carNPC3, -21);
    lv_obj_set_y(ui_carNPC3, -60);
    lv_obj_set_align(ui_carNPC3, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(ui_carNPC3, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_carNPC3, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_racePanel = lv_obj_create(ui_raceScreen);
    lv_obj_set_width(ui_racePanel, 240);
    lv_obj_set_height(ui_racePanel, 240);
    lv_obj_set_align(ui_racePanel, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(ui_racePanel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_racePanel, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(ui_racePanel, LV_OBJ_FLAG_HIDDEN);       /// Flags
    lv_obj_clear_flag(ui_racePanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_racePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_racePanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_racePanel, 230, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_racePanel, lv_color_hex(0x968100), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_racePanel, 200, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_racePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_racePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_racePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_racePanel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_racePanel, 20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_raceTitle = lv_label_create(ui_racePanel);
    lv_obj_set_width(ui_raceTitle, 100);
    lv_obj_set_height(ui_raceTitle, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_raceTitle, LV_ALIGN_CENTER);
    lv_label_set_text(ui_raceTitle, "Race");
    lv_obj_set_style_text_align(ui_raceTitle, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_raceTitle, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_raceTitle, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_raceTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_raceTitle, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_raceTitle, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_raceInfo = lv_label_create(ui_racePanel);
    lv_obj_set_width(ui_raceInfo, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_raceInfo, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_raceInfo, LV_ALIGN_CENTER);
    lv_label_set_text(ui_raceInfo, "Crashed");
    lv_obj_add_flag(ui_raceInfo, LV_OBJ_FLAG_HIDDEN); /// Flags
    lv_obj_set_style_text_color(ui_raceInfo, lv_color_hex(0xF77272), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_raceInfo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_raceInfo, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_raceLabel = lv_label_create(ui_racePanel);
    lv_obj_set_width(ui_raceLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_raceLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_raceLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_raceLabel, "Your Score");
    lv_obj_set_style_text_font(ui_raceLabel, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_raceScore = lv_label_create(ui_racePanel);
    lv_obj_set_width(ui_raceScore, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_raceScore, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_raceScore, LV_ALIGN_CENTER);
    lv_label_set_text(ui_raceScore, "27m");
    lv_obj_set_style_text_color(ui_raceScore, lv_color_hex(0x18F80E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_raceScore, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_raceScore, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_raceStart = lv_btn_create(ui_racePanel);
    lv_obj_set_width(ui_raceStart, 100);
    lv_obj_set_height(ui_raceStart, 40);
    lv_obj_set_align(ui_raceStart, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_raceStart, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_raceStart, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_radius(ui_raceStart, 20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_raceButtonText = lv_label_create(ui_raceStart);
    lv_obj_set_width(ui_raceButtonText, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_raceButtonText, LV_SIZE_CONTENT); /// 40
    lv_obj_set_align(ui_raceButtonText, LV_ALIGN_CENTER);
    lv_label_set_text(ui_raceButtonText, "Start");

    ui_exitRace = lv_btn_create(ui_racePanel);
    lv_obj_set_width(ui_exitRace, 100);
    lv_obj_set_height(ui_exitRace, 40);
    lv_obj_set_align(ui_exitRace, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_exitRace, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_exitRace, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_radius(ui_exitRace, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_exitRace, lv_color_hex(0xF30000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_exitRace, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_exitRaceText = lv_label_create(ui_exitRace);
    lv_obj_set_width(ui_exitRaceText, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_exitRaceText, LV_SIZE_CONTENT); /// 40
    lv_obj_set_align(ui_exitRaceText, LV_ALIGN_CENTER);
    lv_label_set_text(ui_exitRaceText, "Exit");

    lv_obj_add_event_cb(ui_roadPanel, ui_event_roadPanel, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_leftButton, ui_event_leftButton, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_rightButton, ui_event_rightButton, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_raceStart, ui_event_raceStart, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_exitRace, ui_event_exitRace, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_raceScreen, ui_event_raceScreen, LV_EVENT_ALL, NULL);

    callback("Racing", &ui_img_car_png, &ui_raceScreen);
    active = false;

#endif
}

void ui_raceScreen_screen_loop()
{
#ifdef ENABLE_GAME_RACING
    if (!active)
    {
        return;
    }
    if (!ui_raceScreen)
    {
        return;
    }

    if (isRacing)
    {
        if (plBuf.x != player.x)
        {
            lv_obj_set_x(ui_carPlayer, player.x);
            plBuf.x = player.x;
        }
        if (plBuf.y != player.y)
        {
            lv_obj_set_y(ui_carPlayer, player.y);
            plBuf.y = player.y;
        }

        // Calculate speed modifier
        m = INITIAL_SPEED - (counter / SPEED_DIVISOR);
        if (m < MIN_SPEED)
        {
            m = MIN_SPEED;
        }

        // Calculate speed factor
        speedFactor = INITIAL_SPEED / m;
        if (speedFactor > SPEED_LIMIT_FACTOR)
        {
            speedFactor = SPEED_LIMIT_FACTOR;
        }

        // Update current score
        currentScore = (counter / 100) + ((INITIAL_SPEED - m) * SCORE_MULTIPLIER);
        lv_label_set_text_fmt(ui_distanceLabel, "%d", currentScore);

        // Update road movement progress
        roadProgress += speedFactor;

        // Update speed label
        lv_label_set_text_fmt(ui_speedLabel, "%d", (int)(speedFactor * 100));

        // Move the road image vertically
        lv_obj_set_y(ui_roadImage, ((int)roadProgress) % ROAD_HEIGHT);

        // NPC car logic
        if (((int)npcProgress % NPC_CAR_HEIGHT) > (NPC_CAR_HEIGHT - 10))
        {
            gen = true; // Flag to generate new NPC car position
        }
        if (((int)npcProgress % NPC_CAR_HEIGHT) == 0 && gen)
        {
            npc1.x = (lv_rand(0, 99) % 100) + NPC_CAR_X_OFFSET;
            lv_obj_set_x(ui_carNPC1, npc1.x);
            lv_img_set_src(ui_carNPC1, carIcons[lv_rand(0, 2) % 3]);
            gen = false;
        }

        // Update NPC car speed
        // npcSpeed -= 0.001; // Decrease NPC speed gradually
        // if (npcSpeed < MIN_SPEED)
        // {
        //     npcSpeed = MIN_SPEED;
        // }

        // Update NPC car y-position
        npcProgress += MIN_SPEED;
        npc1.y = ((int)npcProgress % NPC_CAR_HEIGHT) + NPC_CAR_OFFSET_Y;
        if (npcBuf.y != npc1.y)
        {
            npcBuf.y = npc1.y;
            lv_obj_set_y(ui_carNPC1, npc1.y);
        }

        if (haveCrashed(player, npc1))
        {
            active = true;
            isRacing = false;
            gameOver = true;

            lv_label_set_text(ui_distanceLabel, "0");
            lv_label_set_text(ui_speedLabel, "0");

            lv_obj_clear_flag(ui_racePanel, LV_OBJ_FLAG_HIDDEN); /// Flags
            lv_label_set_text(ui_raceButtonText, "Start");
            lv_label_set_text(ui_raceLabel, "Your Score");
            lv_label_set_text_fmt(ui_raceScore, "%dm", currentScore);
            lv_obj_clear_flag(ui_raceInfo, LV_OBJ_FLAG_HIDDEN); /// Flags

            if (currentScore > highScore)
            {
                highScore = currentScore;
                lv_label_set_text(ui_raceLabel, "New High Score");
            }
            currentScore = 0;
        }

        counter++;
    }

#endif
}
