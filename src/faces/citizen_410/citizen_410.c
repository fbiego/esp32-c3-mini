
// Hand-authored watchface (not bin2lvgl-generated): Citizen Eco-Drive "Red
// Arrows" Skyhawk, built from a real product photo. See citizen_410.h for
// the full story - dial background is a photo (hands inpainted out), hands
// and the date window are drawn fresh on top with LVGL primitives, same
// technique as classic_410.
// Watchface: citizen_410

#include <math.h>
#include "citizen_410.h"

#ifdef ENABLE_FACE_CITIZEN_410

lv_obj_t *face_citizen_410;

static lv_obj_t *body_bg      = NULL;
static lv_obj_t *dial_img     = NULL;
static lv_obj_t *hour_hand    = NULL;
static lv_obj_t *min_hand     = NULL;
static lv_obj_t *sec_hand     = NULL;
static lv_obj_t *date_box     = NULL;
static lv_obj_t *day_shadow   = NULL;
static lv_obj_t *day_label    = NULL;
static lv_obj_t *date_shadow  = NULL;
static lv_obj_t *date_label   = NULL;
static lv_obj_t *batt_box     = NULL;
static lv_obj_t *batt_shadow  = NULL;
static lv_obj_t *batt_label   = NULL;

// Soft "blur" halo behind each hard-edged shadow copy: LVGL's own shadow
// style blurs a widget's rounded-rect box, not glyph shapes, so it can't
// blur text directly - this fakes it with 4 extra low-opacity copies of the
// shadow text at diagonal 1px offsets around the real shadow, same trick as
// a cheap CSS multi-layer text-shadow blur.
static lv_obj_t *day_shadow_halo[4]  = {NULL};
static lv_obj_t *date_shadow_halo[4] = {NULL};
static lv_obj_t *batt_shadow_halo[4] = {NULL};

// Source photo was cropped/centered so the dial's true center lands at exactly
// (205,205) in its own 410x410 image - confirmed by construction (crop was
// centered on the bezel before any resize). This board reports 410x494, so -
// same as classic_410 - centered with a top/bottom letterbox rather than
// stretched.
#define CW 410
#define CH 410
#define SCREEN_H 494
#define Y_OFFSET ((SCREEN_H - CH) / 2)
// Whole dial+hands+text assembly nudged 3px left on-screen 2026-07-20 (user
// noticed it read slightly right-of-center) - dial_img's own raster is
// untouched, only its draw position and everything anchored to it moves.
// face_citizen_410_body_bg is a separate case, already re-cropped/shifted to
// match wherever the dial ends up (see that asset's own header comment).
#define DIAL_SHIFT_X -3
#define SCREEN_CX (CW / 2 + DIAL_SHIFT_X)
#define SCREEN_CY (Y_OFFSET + CH / 2)
#define PI_F 3.14159265f

// Right-hand LCD window ("TYO"/digits in the source photo) location, re-measured
// directly off the 2026-07-17 (v3) background - the true physical panel is
// x250-303,y205-271 this time (taller than before). Box grown to 53x54 (from
// 53x44) to fit the larger day/date font plus a divider line, still comfortably
// inside the true panel.
#define DATE_BOX_X (250 + DIAL_SHIFT_X)
#define DATE_BOX_Y (203 + Y_OFFSET)
#define DATE_BOX_W 53
#define DATE_BOX_H 62
// DSEG14 (real LCD-style 14-segment font) - fixed 2026-07-20: lv_font_conv
// defaults to RLE-compressed output (bitmap_format=1), but LV_USE_FONT_COMPRESSED
// is 0 in include/lv_conf.h, so lv_font_get_bitmap_fmt_txt() silently returned
// NULL for every glyph -> the null-deref crash-loop from the earlier session.
// Regenerated with --no-compress (bitmap_format=0, PLAIN) instead of touching
// the global lv_conf.h flag - isolated test label confirmed clean boot.
#define DATE_FONT &dseg14_bold_18
// Day-name label is drawn wider than the true 53px physical panel (see
// OVERFLOW_VISIBLE note at date_box's creation) since the widest day names
// ("MON" 57.4px, "WED" 57.7px per lv_font_montserrat_22's own adv_w table)
// don't fit inside it - 62px covers the worst case (WED) with a few px of
// margin, spilling only ~4-5px past each true panel edge, same ballpark as
// the user's own "widen a touch" ask.
#define DAY_LABEL_W 62
#define DAY_LABEL_X ((DATE_BOX_W - DAY_LABEL_W) / 2)
// Right-justified date number: same inset pattern as BATT_RIGHT_PAD below,
// clears date_box's own rounded corner (radius 4).
#define DATE_RIGHT_PAD 3

// Left-hand LCD window (the "USA/EUR/JOU/CHN" gauge's neighbour panel), newly
// wired up this pass to show battery %. True panel is x95-158,y200-231; box
// inset from the left edge specifically to avoid the small red gauge-pointer
// element that sits just outside it at x~85-93.
#define BATT_BOX_X (100 + DIAL_SHIFT_X)
#define BATT_BOX_Y (203 + Y_OFFSET)
#define BATT_BOX_W 54
#define BATT_BOX_H 27
#define BATT_FONT &dseg14_bold_18
// Right-justified battery number: small inset from the box's own right edge
// so the text doesn't touch the rounded corner (batt_box radius 4).
#define BATT_RIGHT_PAD 3

// Real hand sprites (see face_citizen_410_hand_*.c) are cut straight from the
// source photo at whatever angle they happened to be posed at, not pointing
// "up" - so each has its own as-photographed base angle (measured off the
// same traced tip coordinates used to cut the sprite) that has to be
// subtracted out before applying the real time-driven rotation. Lengths
// (pivot-to-tip) tuned 2026-07-17 so minute/second reach the middle of the
// dial's main hour markers and hour reads clearly shorter: hour 95px, minute
// 140px, second 140px - hour/second are uniform rescales of the previous
// sprites (base angle unchanged by a uniform scale), minute is a fresh
// red-bleed-cleaned cut so its base angle was recomputed from scratch.
#define HAND_HOUR_BASE_DEG   302.4f
#define HAND_MINUTE_BASE_DEG  56.6f
#define HAND_SECOND_BASE_DEG 208.8f

static const char *const DAY_NAMES[7] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

static int32_t norm_angle_deci(float deg)
{
    while (deg < 0.0f) deg += 360.0f;
    while (deg >= 360.0f) deg -= 360.0f;
    return (int32_t)(deg * 10.0f);
}

static void create_shadow_halo(lv_obj_t *halo[4], lv_obj_t *parent, lv_text_align_t align,
                                int16_t pad_right, int32_t base_x, int32_t base_y, int32_t w,
                                const char *text)
{
    static const int8_t dx[4] = { -1,  1, -1, 1 };
    static const int8_t dy[4] = { -1, -1,  1, 1 };
    for (int i = 0; i < 4; i++)
    {
        lv_obj_t *h = lv_label_create(parent);
        lv_obj_set_style_text_font(h, DATE_FONT, 0);
        lv_obj_set_style_text_color(h, lv_color_hex(0x696969), 0);
        lv_obj_set_style_text_align(h, align, 0);
        if (pad_right) lv_obj_set_style_pad_right(h, pad_right, 0);
        lv_obj_set_style_text_opa(h, LV_OPA_20, 0);
        lv_obj_set_style_bg_opa(h, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(h, 0, 0);
        lv_label_set_text(h, text);
        lv_obj_set_pos(h, base_x + dx[i], base_y + dy[i]);
        lv_obj_set_width(h, w);
        halo[i] = h;
    }
}

#endif

void init_face_citizen_410(void (*callback)(const char*, const lv_img_dsc_t *, lv_obj_t **, lv_obj_t **)){
#ifdef ENABLE_FACE_CITIZEN_410
    face_citizen_410 = lv_obj_create(NULL);
    lv_obj_remove_flag(face_citizen_410, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(face_citizen_410, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(face_citizen_410, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(face_citizen_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(face_citizen_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(face_citizen_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(face_citizen_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(face_citizen_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(face_citizen_410, onFaceEvent, LV_EVENT_ALL, NULL);

    /* ---- Body/case background: the watch's crown/pushers/bracelet, pattern-matched (not
       eyeballed) against the existing dial so it lines up exactly - see face_citizen_410_body_bg.c.
       Drawn full-screen, behind the dial, which now has transparent corners so this shows through
       in the four triangular areas around the round dial. ---- */
    body_bg = lv_image_create(face_citizen_410);
    lv_image_set_src(body_bg, &face_citizen_410_body_bg);
    lv_obj_set_pos(body_bg, 0, 0);
    lv_obj_remove_flag(body_bg, LV_OBJ_FLAG_SCROLLABLE);

    dial_img = lv_image_create(face_citizen_410);
    lv_image_set_src(dial_img, &face_citizen_410_dial_img);
    lv_obj_set_pos(dial_img, DIAL_SHIFT_X, Y_OFFSET);
    lv_obj_remove_flag(dial_img, LV_OBJ_FLAG_SCROLLABLE);

    /* ---- Date window: real LCD-panel background (photo, blurred to erase the
       old static text but keep the real grey gradient/texture) + black text
       with a dark offset copy behind/below it for a real LCD-style drop
       shadow (deliberately offset several px, not a subtle 1px emboss - user
       asked for it much more prominent, disconnected from the text), day on
       top, date below, with a thin divider line between them (re-added
       2026-07-20 per explicit request - an earlier session had removed a
       divider here per different feedback, this is a deliberate reversal).
       ---- */
    date_box = lv_obj_create(face_citizen_410);
    lv_obj_set_size(date_box, DATE_BOX_W, DATE_BOX_H);
    lv_obj_set_pos(date_box, DATE_BOX_X, DATE_BOX_Y);
    lv_obj_set_style_bg_image_src(date_box, &face_citizen_410_lcd_bg, 0);
    lv_obj_set_style_bg_image_opa(date_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_opa(date_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(date_box, 0, 0);
    lv_obj_set_style_radius(date_box, 4, 0);
    lv_obj_set_style_pad_all(date_box, 0, 0);
    lv_obj_remove_flag(date_box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(date_box, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_clip_corner(date_box, true, 0);
    /* Day-name width overflow fix 2026-07-20: at DATE_FONT (montserrat22),
       "MON" measures ~57.4px and "WED" ~57.7px via the font's own adv_w
       table - both wider than the old DATE_BOX_W (53px) label box, so on
       any Monday/Wednesday the day label word-wrapped ("MO"/"N" on two
       lines), silently colliding with the date row below. Day label/shadow
       widened to DAY_LABEL_W (62px, just above the 57.7px worst case) and
       re-centered; OVERFLOW_VISIBLE added here so the now-wider day text
       isn't clipped by this box's own (unchanged, 53px) bounds. */
    lv_obj_add_flag(date_box, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    create_shadow_halo(day_shadow_halo, date_box, LV_TEXT_ALIGN_CENTER, 0,
                        DAY_LABEL_X + 3, 6, DAY_LABEL_W, "SUN");

    day_shadow = lv_label_create(date_box);
    lv_obj_set_style_text_font(day_shadow, DATE_FONT, 0);
    lv_obj_set_style_text_color(day_shadow, lv_color_hex(0x696969), 0);
    lv_obj_set_style_text_align(day_shadow, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(day_shadow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(day_shadow, 0, 0);
    lv_label_set_text(day_shadow, "SUN");
    lv_obj_set_pos(day_shadow, DAY_LABEL_X + 3, 6);
    lv_obj_set_width(day_shadow, DAY_LABEL_W);

    day_label = lv_label_create(date_box);
    lv_obj_set_style_text_font(day_label, DATE_FONT, 0);
    lv_obj_set_style_text_color(day_label, lv_color_black(), 0);
    lv_obj_set_style_text_align(day_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(day_label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(day_label, 0, 0);
    lv_label_set_text(day_label, "SUN");
    lv_obj_set_pos(day_label, DAY_LABEL_X, 3);
    lv_obj_set_width(day_label, DAY_LABEL_W);

    /* Divider: thin LCD-style rule between day and date, sitting in the gap
       between the day baseline (~row 27) and the date row start (row 32). */
    lv_obj_t *day_date_divider = lv_obj_create(date_box);
    lv_obj_set_size(day_date_divider, DATE_BOX_W - 12, 2);
    lv_obj_set_pos(day_date_divider, 6, 29);
    lv_obj_set_style_bg_color(day_date_divider, lv_color_hex(0x5a5a5a), 0);
    lv_obj_set_style_bg_opa(day_date_divider, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(day_date_divider, 0, 0);
    lv_obj_set_style_radius(day_date_divider, 0, 0);
    lv_obj_set_style_pad_all(day_date_divider, 0, 0);
    lv_obj_remove_flag(day_date_divider, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(day_date_divider, LV_OBJ_FLAG_CLICKABLE);

    create_shadow_halo(date_shadow_halo, date_box, LV_TEXT_ALIGN_RIGHT, DATE_RIGHT_PAD,
                        3, 42, DATE_BOX_W, "01");

    date_shadow = lv_label_create(date_box);
    lv_obj_set_style_text_font(date_shadow, DATE_FONT, 0);
    lv_obj_set_style_text_color(date_shadow, lv_color_hex(0x696969), 0);
    lv_obj_set_style_text_align(date_shadow, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_pad_right(date_shadow, DATE_RIGHT_PAD, 0);
    lv_obj_set_style_bg_opa(date_shadow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(date_shadow, 0, 0);
    lv_label_set_text(date_shadow, "01");
    lv_obj_set_pos(date_shadow, 3, 42);
    lv_obj_set_width(date_shadow, DATE_BOX_W);

    date_label = lv_label_create(date_box);
    lv_obj_set_style_text_font(date_label, DATE_FONT, 0);
    lv_obj_set_style_text_color(date_label, lv_color_black(), 0);
    lv_obj_set_style_text_align(date_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_pad_right(date_label, DATE_RIGHT_PAD, 0);
    lv_obj_set_style_bg_opa(date_label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(date_label, 0, 0);
    lv_label_set_text(date_label, "01");
    /* Nudged down ~1/4 character (DSEG14-18 line_height=18, so ~5px) per
       user request 2026-07-20. */
    lv_obj_set_pos(date_label, 0, 39);
    lv_obj_set_width(date_label, DATE_BOX_W);

    /* ---- Battery window: left-hand LCD panel, same style as the date
       window (blurred real panel background, black-on-grey with a dark
       shadow copy), single line, same font size as day/date. ---- */
    batt_box = lv_obj_create(face_citizen_410);
    lv_obj_set_size(batt_box, BATT_BOX_W, BATT_BOX_H);
    lv_obj_set_pos(batt_box, BATT_BOX_X, BATT_BOX_Y);
    lv_obj_set_style_bg_image_src(batt_box, &face_citizen_410_lcd_bg_left, 0);
    lv_obj_set_style_bg_image_opa(batt_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_opa(batt_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(batt_box, 0, 0);
    lv_obj_set_style_radius(batt_box, 4, 0);
    lv_obj_set_style_pad_all(batt_box, 0, 0);
    lv_obj_remove_flag(batt_box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(batt_box, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_clip_corner(batt_box, true, 0);

    create_shadow_halo(batt_shadow_halo, batt_box, LV_TEXT_ALIGN_RIGHT, BATT_RIGHT_PAD,
                        3, 7, BATT_BOX_W, "100");

    batt_shadow = lv_label_create(batt_box);
    lv_obj_set_style_text_font(batt_shadow, BATT_FONT, 0);
    lv_obj_set_style_text_color(batt_shadow, lv_color_hex(0x696969), 0);
    lv_obj_set_style_text_align(batt_shadow, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_pad_right(batt_shadow, BATT_RIGHT_PAD, 0);
    lv_obj_set_style_bg_opa(batt_shadow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(batt_shadow, 0, 0);
    lv_label_set_text(batt_shadow, "100");
    lv_obj_set_pos(batt_shadow, 3, 7);
    lv_obj_set_width(batt_shadow, BATT_BOX_W);

    batt_label = lv_label_create(batt_box);
    lv_obj_set_style_text_font(batt_label, BATT_FONT, 0);
    lv_obj_set_style_text_color(batt_label, lv_color_black(), 0);
    lv_obj_set_style_text_align(batt_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_pad_right(batt_label, BATT_RIGHT_PAD, 0);
    lv_obj_set_style_bg_opa(batt_label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(batt_label, 0, 0);
    lv_label_set_text(batt_label, "100");
    lv_obj_set_pos(batt_label, 0, 4);
    lv_obj_set_width(batt_label, BATT_BOX_W);

    /* ---- Clock hands: real sprites cut from the source photo, not drawn
       lines - see face_citizen_410_hand_*.c for how/why. Positioned so each
       sprite's own pivot point (baked into the crop) lands exactly on the
       dial's true center (SCREEN_CX,SCREEN_CY), then rotated. ---- */
    hour_hand = lv_image_create(face_citizen_410);
    lv_image_set_src(hour_hand, &face_citizen_410_hand_hour);
    lv_obj_set_pos(hour_hand, SCREEN_CX - 86, SCREEN_CY - 56);
    lv_image_set_pivot(hour_hand, 86, 56);
    lv_obj_remove_flag(hour_hand, LV_OBJ_FLAG_SCROLLABLE);

    min_hand = lv_image_create(face_citizen_410);
    lv_image_set_src(min_hand, &face_citizen_410_hand_minute);
    lv_obj_set_pos(min_hand, SCREEN_CX - 28, SCREEN_CY - 69);
    lv_image_set_pivot(min_hand, 28, 69);
    lv_obj_remove_flag(min_hand, LV_OBJ_FLAG_SCROLLABLE);

    sec_hand = lv_image_create(face_citizen_410);
    lv_image_set_src(sec_hand, &face_citizen_410_hand_second);
    lv_obj_set_pos(sec_hand, SCREEN_CX - 63, SCREEN_CY - 50);
    lv_image_set_pivot(sec_hand, 63, 50);
    lv_obj_remove_flag(sec_hand, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *cap = lv_obj_create(face_citizen_410);
    lv_obj_set_size(cap, 8, 8);
    lv_obj_set_pos(cap, SCREEN_CX - 4, SCREEN_CY - 4);
    lv_obj_set_style_radius(cap, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(cap, lv_color_hex(0xF41220), 0);
    lv_obj_set_style_bg_opa(cap, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cap, 0, 0);

    callback("Citizen", &face_citizen_410_dial_img_preview, &face_citizen_410, &sec_hand);

#endif
}

void update_time_citizen_410(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday)
{
#ifdef ENABLE_FACE_CITIZEN_410
    if (!face_citizen_410 || !hour_hand)
    {
        return;
    }

    lv_label_set_text(day_shadow, DAY_NAMES[weekday % 7]);
    lv_label_set_text(day_label, DAY_NAMES[weekday % 7]);
    lv_label_set_text_fmt(date_shadow, "%02d", day);
    lv_label_set_text_fmt(date_label, "%02d", day);
    for (int i = 0; i < 4; i++)
    {
        lv_label_set_text(day_shadow_halo[i], DAY_NAMES[weekday % 7]);
        lv_label_set_text_fmt(date_shadow_halo[i], "%02d", day);
    }

    float hour_angle = (hour % 12) * 30.0f + minute * 0.5f;
    float min_angle  = minute * 6.0f;
    float sec_angle  = second * 6.0f;

    lv_image_set_rotation(hour_hand, norm_angle_deci(hour_angle - HAND_HOUR_BASE_DEG));
    lv_image_set_rotation(min_hand, norm_angle_deci(min_angle - HAND_MINUTE_BASE_DEG));
    lv_image_set_rotation(sec_hand, norm_angle_deci(sec_angle - HAND_SECOND_BASE_DEG));

#endif
}

void update_weather_citizen_410(int temp, int icon)
{
#ifdef ENABLE_FACE_CITIZEN_410
    if (!face_citizen_410)
    {
        return;
    }

#endif
}

void update_status_citizen_410(int battery, bool connection){
#ifdef ENABLE_FACE_CITIZEN_410
    if (!face_citizen_410 || !batt_label)
    {
        return;
    }

    lv_label_set_text_fmt(batt_shadow, "%d", battery);
    lv_label_set_text_fmt(batt_label, "%d", battery);
    for (int i = 0; i < 4; i++)
    {
        lv_label_set_text_fmt(batt_shadow_halo[i], "%d", battery);
    }

#endif
}

void update_activity_citizen_410(int steps, int distance, int kcal)
{
#ifdef ENABLE_FACE_CITIZEN_410
    if (!face_citizen_410)
    {
        return;
    }

#endif
}

void update_health_citizen_410(int bpm, int oxygen)
{
#ifdef ENABLE_FACE_CITIZEN_410
    if (!face_citizen_410)
    {
        return;
    }

#endif
}

void update_all_citizen_410(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
    int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
#ifdef ENABLE_FACE_CITIZEN_410
    update_time_citizen_410(second, minute, hour, mode, am, day, month, year, weekday);
    update_weather_citizen_410(temp, icon);
    update_status_citizen_410(battery, connection);
    update_activity_citizen_410(steps, distance, kcal);
    update_health_citizen_410(bpm, oxygen);
#endif
}

void update_check_citizen_410(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
    int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
#ifdef ENABLE_FACE_CITIZEN_410
    if (root != face_citizen_410)
    {
        return;
    }
    update_time_citizen_410(second, minute, hour, mode, am, day, month, year, weekday);
    update_weather_citizen_410(temp, icon);
    update_status_citizen_410(battery, connection);
    update_activity_citizen_410(steps, distance, kcal);
    update_health_citizen_410(bpm, oxygen);
#endif
}
