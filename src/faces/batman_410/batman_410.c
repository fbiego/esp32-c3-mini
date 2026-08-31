
// Hand-authored watchface (not bin2lvgl-generated): Batman chronograph dial.
// See batman_410.h for the delivery story. Background is a single opaque
// 410x494 raster (already the project's native "content canvas" size, see
// SCREEN_H in citizen_410.c/classic_410.c, so no scale/letterbox math is
// needed here unlike faces ported from other resolutions).
//
// Rebuilt again 2026-08-01 against a redelivered, cleaner source package
// (batman_410/batman_v4_410x494/ - "Fully Clean" background per its own
// README) that fixed the first delivery's messy geometry: numerals now sit
// evenly around a real circle, both subdials are blank (no baked-in "10"/"20"
// labels or numbers), and the hand sprites are proper tapered/arrowhead art
// instead of thin placeholder lines. Two subdials only, same as before - the
// 6 o'clock position is still fully occupied by the bat emblem and "6"
// numeral, so there's no room for a third.
//
// Centers below were measured programmatically off the source PNG (thresholding
// the tick-mark pixels and taking bounding-box/centroid, not eyeballed) and
// confirmed by overlaying crosshairs on the art. Still not a mathematically
// perfect circle fit - this is AI-generated dial art - but noticeably tighter
// than the first delivery.
//
// Rebuilt again 2026-08-20 with a v5 source package (batman_v5_410x494/) -
// wider background (pillarboxed down to the 410x494 canvas) and hour/min/sec
// hands with a baked-in hub+tail drawn at an angle, rotated straight during
// pre-processing rather than as-authored. See that directory's README.md for
// the pixel math and the HAND_BASE_DEG / HAND_*_PIVOT_* comments below for
// how that changed the hand placement code. Sub-hand art (hand_sub) is still
// the unchanged v4 asset - none was redelivered this round.
//
// v5.1 (2026-08-21): minute hand lengthened (see HAND_MIN_PIVOT_* below) and
// two of the background's baked dummy readouts wired up to live data: the
// top-left "FRI" day window now shows the real weekday, and the number
// baked above the bat emblem (was a static "31") now shows live seconds as
// an experiment. Both reuse the citizen_410 trick of an opaque patch image
// (a crop of the same background with the old text erased via inpainting,
// see day_bg_patch.png/sec_bg_patch.png and gen_batman_410.py's v5.1 note)
// laid over the original art, with an LVGL label on top. Unlike citizen_410's
// LCD windows the FRI pill is drawn at a diagonal tilt in the art; the day
// label was first tried flat (non-rotated) but confirmed wrong on real
// hardware, so it's now rotated via style_transform_rotation to match the
// pill's tilt (see DAY_LABEL_ROTATION below for the measured angle). The
// seconds label above the bat isn't rotated - that number sits flat in the
// original art.
//
// v5.2 (2026-08-21, same day): the rest of the dial's baked readouts wired
// up the same way - top-right "78%" pill -> live battery (rotated the other
// way from the day pill, see BATT_LABEL_ROTATION), bottom-center "14:37" ->
// live digital time, the black "31" box between the 4 and 5 numerals ->
// live day-of-month, and the "22°C" next to the sun icon -> live
// temperature (the sun icon itself is left as static art - no icon-code
// convention exists anywhere else in this codebase to drive a real icon
// swap from). Also: all six of these windows (day/sec/time/batt/weather/
// date bg+label pairs) moved to be created *before* the clock hands, not
// after - they were drawn on top of the hands, which looked wrong wherever
// a hand could sweep over a window (confirmed wrong on hardware for the
// seconds number, which sits well within the hour hand's reach).
//
// v5.3 (2026-08-21, hardware-feedback round): day/battery pill rotations
// tuned to a symmetric +/-27deg (was -26.6/+40.6 - see DAY_LABEL_ROTATION/
// BATT_LABEL_ROTATION), day label nudged 1px down (toward dial center) per
// user request, weather temperature rotated -20deg and nudged toward the
// dial's 23rd minute tick (tick-23 measured at radius 162 from MAIN_CX/CY,
// angle 138deg - see WEATHER_LABEL_ROTATION/WEATHER_BG_X comment; moved only
// a conservative amount since a full move to that tick's radius would clip
// the "5" numeral - the patch there is a plain background crop, not
// inpainted, so it can't safely overlap baked art), all six readout fonts
// bumped up a size and swapped from stock (regular-weight) montserrat to a
// custom-generated semibold montserrat (Google's Montserrat SemiBold/600,
// baked via lv_font_conv same as citizen_410's dseg14_bold_18 - LVGL's
// built-in fonts have no bold weight compiled in, "half bold" isn't a
// scriptable LVGL style so this is the closest real equivalent), and the
// static baked sun icon patched out and replaced with a live weather-state
// icon (sunny/cloudy/rain/etc, 8 states) - stolen directly from red_magic_
// 410's bin2lvgl-sourced weather icon set (its grayscale/silver look fits
// this dial's black-and-gold palette far better than 2151_410's brown-chip
// or 1167_410's blue/orange sets, which were also considered), reusing the
// same `icon % 8` indexing already proven live on that face and on 1167_410/
// 2151_410 - no new guessing about what the phone app's icon codes mean.
// Icons downscaled and padded to a fixed 26x26 canvas so swapping the image
// src between states never shifts the object's on-screen position (LVGL
// repositions image objects by top-left, not centroid, and the source icons
// aren't uniformly sized).
// Watchface: batman_410

#include <math.h>
#include "batman_410.h"

#ifdef ENABLE_FACE_BATMAN_410

lv_obj_t *face_batman_410;

static lv_obj_t *face_bg     = NULL;
static lv_obj_t *hour_hand   = NULL;
static lv_obj_t *min_hand    = NULL;
static lv_obj_t *sec_hand    = NULL;
static lv_obj_t *sub_hand_l  = NULL;  // left (blank) subdial, running-seconds sweep
static lv_obj_t *sub_hand_r  = NULL;  // right (blank) subdial, slow hour-style counter
static lv_obj_t *day_bg      = NULL;  // patched-blank copy of the top-left "FRI" pill
static lv_obj_t *day_label   = NULL;  // live weekday text drawn on top of day_bg
static lv_obj_t *sec_bg      = NULL;  // patched-blank copy of the number above the bat
static lv_obj_t *sec_label   = NULL;  // live seconds text drawn on top of sec_bg
static lv_obj_t *time_bg     = NULL;  // patched-blank copy of the bottom-center "14:37"
static lv_obj_t *time_label  = NULL;  // live HH:MM text drawn on top of time_bg
static lv_obj_t *batt_bg     = NULL;  // patched-blank copy of the top-right "78%" pill
static lv_obj_t *batt_label  = NULL;  // live battery text drawn on top of batt_bg
static lv_obj_t *weather_label = NULL; // live temperature text drawn directly on the (now sun/temp-free) background
static lv_obj_t *date_bg     = NULL;  // patched-blank copy of the "31" box between 4 and 5
static lv_obj_t *date_label  = NULL;  // live day-of-month text drawn on top of date_bg
static lv_obj_t *weather_icon = NULL; // live weather-state icon drawn directly on the background

// Canvas is already 410x494, the project's standard "content" size - see
// header comment above.
#define CW 410
#define CH 494

// Main dial center: re-measured 2026-08-20 against the v5 background - the
// canvas's raw geometric center (CH/2 = 247) was ~15px too high. Found the
// true center by locating the gold index ticks at the 3 and 9 o'clock
// positions (thresholded gold pixels outside the numeral ring, at x~45-59
// and x~359-366) - both sit at y=262, not 247, confirming the drawn dial
// isn't vertically centered on the canvas. CX unaffected (tick x-midpoint is
// 206, a rounding error from CW/2's 205).
#define MAIN_CX (CW / 2)
#define MAIN_CY 262

// Subdial centers, measured from the tick-ring bounding box on each subdial
// (see header comment). Right mirrors left around the canvas's horizontal
// center since the art is left/right symmetric at a glance.
#define SUB_L_CX 86
#define SUB_L_CY 257
#define SUB_R_CX (CW - SUB_L_CX)
#define SUB_R_CY 257

// hand_sub still follows the original v4 authoring convention: uniform-width
// shaft, pivot at the sprite's top row (blunt end), tip at the bottom row -
// i.e. "as drawn" pointing straight down, clock-angle 180 (0 = 12 o'clock/
// up, clockwise) before rotation. See its own lv_image_set_pivot(..., 6, 0)
// call below.
//
// hand_hour/min/sec are v5 sprites (2026-08-20 delivery): each was rotated
// during pre-processing (see batman_v5_410x494/README.md) so its tip points
// straight up, i.e. clock-angle 0, before rotation - the opposite of
// hand_sub's convention - and each keeps its own baked-in gold pivot hub +
// counterweight tail, with the pivot point sitting inside the image rather
// than at row 0 (see HAND_HOUR_PIVOT_* etc below).
#define HAND_BASE_DEG 0.0f
#define HAND_SUB_BASE_DEG 180.0f

// Pivot point of each v5 hand sprite (hub center). Re-measured 2026-08-20
// after a second downscale pass off the original (pre-v5-final-scale)
// normalized hand crops - the first v5 delivery's hands were too long
// (second hand overshot the bezel entirely). New target: second hand's
// pivot-to-tip length just clears the dial's inner edge at 3 o'clock
// (measured bezel inner edge at x=373 from MAIN_CX=205, i.e. radius 168;
// tip length set to 165 for a few px of clearance) - hour/minute rescaled
// by the same factor to keep their original relative proportions.
#define HAND_HOUR_PIVOT_X 20
#define HAND_HOUR_PIVOT_Y 104
// Minute hand rescaled 2026-08-21 (24x138 -> 31x179 in the source asset) so
// its pivot-to-tip length reaches the 3 o'clock minute-tick ring instead of
// stopping well short of it. Target radius measured directly off this
// background's pixels: the two minute ticks flanking the 3 o'clock position
// (the exact 3 o'clock tick itself is hidden under the right subdial) have
// their outer tips at (366,244) and (366,280), both radius 162.0px from
// (MAIN_CX,MAIN_CY) - old pivot-to-tip was only 125px, noticeably short.
// Scale factor 162/125 = 1.296 applied uniformly to the whole sprite so the
// hand doesn't distort, pivot coordinates scaled by the same factor.
#define HAND_MIN_PIVOT_X 16
#define HAND_MIN_PIVOT_Y 162
#define HAND_SEC_PIVOT_X 12
#define HAND_SEC_PIVOT_Y 166

// Day-of-week window: opaque day_bg patch is a straight crop of the
// background at (DAY_BG_X,DAY_BG_Y), see gen_batman_410.py's v5.1 note for
// how the baked "FRI" text was removed from it. day_label is rotated to
// match the pill's own diagonal tilt in the art (confirmed needed on real
// hardware 2026-08-21 - a flat label read visibly wrong against the tilted
// window). Tilt angle measured off the original baked "FRI" glyphs: bottom-
// left corner of "F" at (73,88) to top-right corner of "I" at (133,58),
// atan2(58-88, 133-73) = atan2(-30,60) = -26.57 deg - i.e. the pill's text
// leans with its right side raised, so the label is rotated the same way
// (LVGL's transform_rotation is clockwise-positive, so a "leans right side
// up" tilt is a negative angle). Rotation pivot is the label box's own
// center so the box position/size below doesn't need to change to keep the
// text centered on the window.
#define DAY_BG_X 58
#define DAY_BG_Y 46
// Round 3 (2026-08-21, third hardware-feedback pass): another "2px towards
// center", same (1,2) unit direction as round 2 (the ratio barely changes
// this close to the original position) - from round 2's (74,61) to (75,63).
// 2026-08-30: rotated about the dial center (MAIN_CX,MAIN_CY) in the
// positive (clockwise, per LVGL's transform_rotation convention - see header
// comment above) direction by a 4px arc, DAY_LABEL only - r=220.2px from dial
// center at this point, so dtheta=4/r=1.04deg, giving a (+4,-2) tangential
// translation (rounded to whole pixels): (75,63) -> (79,61). DAY_BG does NOT
// move with it and never should - unlike the label, it's a fixed crop of the
// original art (with "FRI" inpainted out) that has to stay exactly where it
// was cut from to blend seamlessly with the surrounding pill border/tick
// marks (see this window's own header comment above); moving it broke that
// seam on real hardware (visible left-edge glitch + disjointed nearby
// ticks) - reverted back to its original (58,46) after that was caught.
#define DAY_LABEL_X 79
#define DAY_LABEL_Y 61
#define DAY_LABEL_W 60
#define DAY_LABEL_H 26
// -28.0 deg, unchanged this round - only position moved, not asked to
// retry the angle again.
#define DAY_LABEL_ROTATION (-280)

// Seconds-experiment window: same technique, patched over the number that
// used to sit statically above the bat emblem (was baked as "31").
#define SEC_BG_X 180
#define SEC_BG_Y 294
#define SEC_LABEL_X 180
#define SEC_LABEL_Y 296
#define SEC_LABEL_W 55
#define SEC_LABEL_H 30
// Gold sampled directly off the original baked digit pixels (e.g. (215,310)
// = 251,227,96), used so the live seconds text keeps the same look.
#define SEC_TEXT_COLOR 0xFBE360

// Digital-time window: bottom center, sits flat on the dial (no pill/border
// under it, same as the seconds number). No 12/24h ("mode"/"am") handling -
// the original art is 24h-style ("14:37") and no other hand-authored face
// in this codebase has an established digital-clock text convention to
// match, so this just prints `hour` as given.
#define TIME_BG_X 168
#define TIME_BG_Y 390
#define TIME_LABEL_X 168
// Down 2px again in round 3 (2026-08-21), same as round 2's move - TIME_BG
// stays put for the same reason WEATHER_BG does (see that comment) - it
// blanks the original baked "14:37" at its fixed art location.
#define TIME_LABEL_Y 395
#define TIME_LABEL_W 80
#define TIME_LABEL_H 24

// Battery window: top-right pill, mirrors the day-of-week pill's technique
// but tilts the other way. Angle measured the same way as DAY_LABEL_ROTATION
// off the original baked "78%": top-left corner of "7" at (283,52) to
// bottom-right corner of "%"'s "0" at (325,88), atan2(88-52,325-283) =
// atan2(36,42) = 40.6 deg - this pill's text leans with its *left* side
// raised (opposite of the day pill), a positive angle in LVGL's
// clockwise-positive convention.
#define BATT_BG_X 270
#define BATT_BG_Y 46
// Round 3 (2026-08-21): another "2px towards center", same (-1,2) unit
// direction as round 2 - from round 2's (276,60) to (275,62).
#define BATT_LABEL_X 275
#define BATT_LABEL_Y 62
#define BATT_LABEL_W 60
#define BATT_LABEL_H 26
// +27.0 deg, unchanged since round 2.
#define BATT_LABEL_ROTATION 270

// Weather window: temperature only. WEATHER_BG stays put - it blanks the
// original baked "22C" text at its fixed art location, and must keep
// covering exactly that spot regardless of where the live label draws.
// WEATHER_LABEL moved three times now (2026-08-21): round 1 nudged it
// +9/-6 toward the dial's 23rd minute tick (tick 23 measured at angle
// 138deg, radius 162 from (MAIN_CX,MAIN_CY), between the 4 and 5 numerals -
// see header comment). Rounds 2-4 (repeat hardware-feedback passes) each
// add the same combined move on top: 5px further toward the tick (unit
// direction (0.83,-0.55) * 5 = (+4,-3)) plus 4px directly away from the
// dial center along the radius through the label's own position at that
// point (direction recomputed each round from the current label center,
// but barely changes - always rounds to (+2,+3)) - combined delta (+6,+1)
// per round: (284,431) -> (290,432) -> (296,433). The numeral-overlap
// concern flagged in round 1 has been easing each round as the label
// clears the "5" (round 4's yellow-pixel check: 7/1680px, down from higher
// counts in rounds 2-3) - user tuning against the real hardware render,
// applied as asked each round rather than re-litigated.
#define WEATHER_BG_X 267
#define WEATHER_BG_Y 433
// 2026-08-30: +3 right from round 4's 296 (WEATHER_BG stays put - see comment
// above; it just blanks the original fixed art, unrelated to where this
// live label draws).
#define WEATHER_LABEL_X 299
#define WEATHER_LABEL_Y 433
#define WEATHER_LABEL_W 60
#define WEATHER_LABEL_H 28
// -26.0 deg (2026-08-30, was -24.0): another 2 degrees of tilt, same
// direction as before.
#define WEATHER_LABEL_ROTATION (-260)

// Day-of-month window: the black box sitting between the 4 and 5 numerals,
// closer to the dial center than either. Flat, no rotation - unlike the
// day/battery pills this box isn't drawn at an angle in the art.
#define DATE_BG_X 262
#define DATE_BG_Y 336
#define DATE_LABEL_X 266
// Down 2px again in round 3 (2026-08-21), same reasoning as TIME_LABEL_Y -
// DATE_BG stays fixed over the original baked "31".
#define DATE_LABEL_Y 353
#define DATE_LABEL_W 56
#define DATE_LABEL_H 29

// Sun-icon window (2026-08-21): the static sun icon that used to sit left of
// the temperature was measured by thresholding its own baked yellow pixels
// (bbox (252,447)-(276,470)); SUN_BG is a slightly larger 36x35 patch
// covering that with margin, filled from a verified-clean nearby dial-black
// texture sample (250-270,436-446, no baked art) rather than an inpaint,
// same idea as the other _bg patches. WEATHER_ICON_W/H is a fixed square
// (see file header note) both icon sizes and the patch don't need to move
// again if the icon set ever changes.
//
// Bug found+fixed same day: that 36x35 patch also silently covered a real
// dial minute tick (the 28th tick, at angle 168deg - confirmed by measuring
// its actual pixel position (248,459)-(256,470), which sits well inside
// SUN_BG's box, and noting that radius from (MAIN_CX,MAIN_CY) is ~207px,
// NOT the 162px used for the temperature's "23rd tick" estimate elsewhere
// in this file - the tick ring apparently isn't a single circle, or 162 was
// specific to the 3 o'clock hand-clearance measurement, not the ring in
// general). Fixed by pasting that exact tick's pixels (cropped straight
// from the untouched source art, not redrawn) into the generated patch at
// the matching offset - see gen_batman_410.py's asset-gen note. User caught
// this on real hardware; nothing in this file's own measurements would
// have surfaced it.
#define SUN_BG_X 246
#define SUN_BG_Y 441
#define WEATHER_ICON_W 26
#define WEATHER_ICON_H 26
// "The weather state symbol can follow it" - given the same (+6,+1) delta
// as WEATHER_LABEL_X/Y each round (see that comment): round 1's SUN_BG-
// relative position (251,445) -> round 2 (257,446) -> round 3 (263,447) ->
// round 4 (269,448). No longer tied to SUN_BG by formula since it moves
// independently of the patch - SUN_BG has to stay put (it blanks the
// original baked sun icon at its fixed art location), but the live icon
// drawn on top of it doesn't.
// 2026-08-30: +4 right, -1 up from round 4's (269,448), independent nudge
// (no longer following WEATHER_LABEL_X/Y's own delta - see comment above).
// Same day, second pass: another +3 right, no further vertical change.
#define WEATHER_ICON_X 276
#define WEATHER_ICON_Y 447

// Readout fonts (2026-08-21): bumped a size up from the original 20/24 and
// switched from LVGL's stock (regular-weight only) montserrat to a
// custom-baked semibold instance - see file header note for why ("half
// bold" isn't an LVGL style, this is the real equivalent) and
// montserrat_semibold_22/26's own generated-file header for the exact
// lv_font_conv invocation.
#define READOUT_FONT_SM &montserrat_semibold_22
#define READOUT_FONT_LG &montserrat_semibold_26

static const char *const DAY_NAMES[7] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

// Weather-state icon set stolen from red_magic_410's bin2lvgl-sourced
// asset pack (see file header note) - same `icon % 8` indexing already
// proven live on that face.
static const lv_img_dsc_t *const WEATHER_ICONS[8] = {
    &face_batman_410_weather_icon_0,
    &face_batman_410_weather_icon_1,
    &face_batman_410_weather_icon_2,
    &face_batman_410_weather_icon_3,
    &face_batman_410_weather_icon_4,
    &face_batman_410_weather_icon_5,
    &face_batman_410_weather_icon_6,
    &face_batman_410_weather_icon_7,
};

static int32_t norm_angle_deci(float deg)
{
    while (deg < 0.0f) deg += 360.0f;
    while (deg >= 360.0f) deg -= 360.0f;
    return (int32_t)(deg * 10.0f);
}

#endif

void init_face_batman_410(void (*callback)(const char*, const lv_img_dsc_t *, lv_obj_t **, lv_obj_t **)){
#ifdef ENABLE_FACE_BATMAN_410
    face_batman_410 = lv_obj_create(NULL);
    lv_obj_remove_flag(face_batman_410, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(face_batman_410, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(face_batman_410, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(face_batman_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(face_batman_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(face_batman_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(face_batman_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(face_batman_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(face_batman_410, onFaceEvent, LV_EVENT_ALL, NULL);

    face_bg = lv_image_create(face_batman_410);
    lv_image_set_src(face_bg, &face_batman_410_face_bg);
    lv_obj_set_pos(face_bg, 0, 0);
    lv_obj_remove_flag(face_bg, LV_OBJ_FLAG_SCROLLABLE);

    /* ---- Day-of-week window: blanked pill patch + live weekday label. ----
       Created before the hands (see v5.2 header note) so a hand sweeping
       through this area, if it ever reaches this far out, draws on top. ---- */
    day_bg = lv_image_create(face_batman_410);
    lv_image_set_src(day_bg, &face_batman_410_day_bg);
    lv_obj_set_pos(day_bg, DAY_BG_X, DAY_BG_Y);
    lv_obj_remove_flag(day_bg, LV_OBJ_FLAG_SCROLLABLE);

    day_label = lv_label_create(face_batman_410);
    lv_obj_set_style_text_font(day_label, READOUT_FONT_SM, 0);
    lv_obj_set_style_text_color(day_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(day_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(day_label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(day_label, 0, 0);
    lv_obj_add_flag(day_label, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_set_style_transform_rotation(day_label, DAY_LABEL_ROTATION, 0);
    lv_obj_set_style_transform_pivot_x(day_label, DAY_LABEL_W / 2, 0);
    lv_obj_set_style_transform_pivot_y(day_label, DAY_LABEL_H / 2, 0);
    lv_label_set_text(day_label, "FRI");
    lv_obj_set_pos(day_label, DAY_LABEL_X, DAY_LABEL_Y);
    lv_obj_set_size(day_label, DAY_LABEL_W, DAY_LABEL_H);

    /* ---- Seconds-experiment window: blanked patch + live seconds label. ----
       This one is well within the hour hand's reach, so being under the
       hands actually matters here - confirmed wrong (drawn on top of the
       hands) on real hardware before this reorder. ---- */
    sec_bg = lv_image_create(face_batman_410);
    lv_image_set_src(sec_bg, &face_batman_410_sec_bg);
    lv_obj_set_pos(sec_bg, SEC_BG_X, SEC_BG_Y);
    lv_obj_remove_flag(sec_bg, LV_OBJ_FLAG_SCROLLABLE);

    sec_label = lv_label_create(face_batman_410);
    lv_obj_set_style_text_font(sec_label, READOUT_FONT_LG, 0);
    lv_obj_set_style_text_color(sec_label, lv_color_hex(SEC_TEXT_COLOR), 0);
    lv_obj_set_style_text_align(sec_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(sec_label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(sec_label, 0, 0);
    lv_label_set_text(sec_label, "00");
    lv_obj_set_pos(sec_label, SEC_LABEL_X, SEC_LABEL_Y);
    lv_obj_set_size(sec_label, SEC_LABEL_W, SEC_LABEL_H);

    /* ---- Digital time window: blanked patch + live HH:MM label. ---- */
    time_bg = lv_image_create(face_batman_410);
    lv_image_set_src(time_bg, &face_batman_410_time_bg);
    lv_obj_set_pos(time_bg, TIME_BG_X, TIME_BG_Y);
    lv_obj_remove_flag(time_bg, LV_OBJ_FLAG_SCROLLABLE);

    time_label = lv_label_create(face_batman_410);
    lv_obj_set_style_text_font(time_label, READOUT_FONT_SM, 0);
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(time_label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(time_label, 0, 0);
    lv_label_set_text(time_label, "00:00");
    lv_obj_set_pos(time_label, TIME_LABEL_X, TIME_LABEL_Y);
    lv_obj_set_size(time_label, TIME_LABEL_W, TIME_LABEL_H);

    /* ---- Battery window: blanked pill patch + live battery label, tilted
       the opposite way from the day pill (see BATT_LABEL_ROTATION). ---- */
    batt_bg = lv_image_create(face_batman_410);
    lv_image_set_src(batt_bg, &face_batman_410_batt_bg);
    lv_obj_set_pos(batt_bg, BATT_BG_X, BATT_BG_Y);
    lv_obj_remove_flag(batt_bg, LV_OBJ_FLAG_SCROLLABLE);

    batt_label = lv_label_create(face_batman_410);
    lv_obj_set_style_text_font(batt_label, READOUT_FONT_SM, 0);
    lv_obj_set_style_text_color(batt_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(batt_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(batt_label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(batt_label, 0, 0);
    lv_obj_add_flag(batt_label, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_set_style_transform_rotation(batt_label, BATT_LABEL_ROTATION, 0);
    lv_obj_set_style_transform_pivot_x(batt_label, BATT_LABEL_W / 2, 0);
    lv_obj_set_style_transform_pivot_y(batt_label, BATT_LABEL_H / 2, 0);
    lv_label_set_text(batt_label, "100%");
    lv_obj_set_pos(batt_label, BATT_LABEL_X, BATT_LABEL_Y);
    lv_obj_set_size(batt_label, BATT_LABEL_W, BATT_LABEL_H);

    /* ---- Weather window: live temperature label (tilted, see
       WEATHER_LABEL_ROTATION), plus the live weather-state icon (see
       WEATHER_ICON_* above). weather_bg/sun_bg (opaque patches that used to
       blank the old baked-in "22°C"+sun icon before drawing these on top)
       removed 2026-08-30 - the background art itself no longer has that
       static readout (user edited it out of batman_bg_410x494.png directly),
       so there's nothing left for those patches to hide; leaving them in
       would have pasted a now-mismatched rectangle over the plain
       background. See face_batman_410_weather_bg/face_batman_410_sun_bg's
       own .c files if this ever needs reverting - the assets themselves
       weren't deleted. */
    weather_label = lv_label_create(face_batman_410);
    lv_obj_set_style_text_font(weather_label, READOUT_FONT_SM, 0);
    lv_obj_set_style_text_color(weather_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(weather_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(weather_label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(weather_label, 0, 0);
    lv_obj_add_flag(weather_label, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_set_style_transform_rotation(weather_label, WEATHER_LABEL_ROTATION, 0);
    lv_obj_set_style_transform_pivot_x(weather_label, WEATHER_LABEL_W / 2, 0);
    lv_obj_set_style_transform_pivot_y(weather_label, WEATHER_LABEL_H / 2, 0);
    lv_label_set_text(weather_label, "--°C");
    lv_obj_set_pos(weather_label, WEATHER_LABEL_X, WEATHER_LABEL_Y);
    lv_obj_set_size(weather_label, WEATHER_LABEL_W, WEATHER_LABEL_H);

    weather_icon = lv_image_create(face_batman_410);
    lv_image_set_src(weather_icon, &face_batman_410_weather_icon_1); // sunny default
    lv_obj_set_pos(weather_icon, WEATHER_ICON_X, WEATHER_ICON_Y);
    lv_obj_remove_flag(weather_icon, LV_OBJ_FLAG_SCROLLABLE);

    /* ---- Day-of-month window: blanked box patch + live date label. ---- */
    date_bg = lv_image_create(face_batman_410);
    lv_image_set_src(date_bg, &face_batman_410_date_bg);
    lv_obj_set_pos(date_bg, DATE_BG_X, DATE_BG_Y);
    lv_obj_remove_flag(date_bg, LV_OBJ_FLAG_SCROLLABLE);

    date_label = lv_label_create(face_batman_410);
    lv_obj_set_style_text_font(date_label, READOUT_FONT_LG, 0);
    lv_obj_set_style_text_color(date_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(date_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(date_label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(date_label, 0, 0);
    lv_label_set_text(date_label, "01");
    lv_obj_set_pos(date_label, DATE_LABEL_X, DATE_LABEL_Y);
    lv_obj_set_size(date_label, DATE_LABEL_W, DATE_LABEL_H);

    /* ---- Clock hands: each v5 sprite's pivot is its own baked-in hub
       center (see HAND_*_PIVOT_* above), positioned so that pivot lands
       exactly on the dial center, then rotated. ---- */
    hour_hand = lv_image_create(face_batman_410);
    lv_image_set_src(hour_hand, &face_batman_410_hand_hour);
    lv_obj_set_pos(hour_hand, MAIN_CX - HAND_HOUR_PIVOT_X, MAIN_CY - HAND_HOUR_PIVOT_Y);
    lv_image_set_pivot(hour_hand, HAND_HOUR_PIVOT_X, HAND_HOUR_PIVOT_Y);
    lv_obj_remove_flag(hour_hand, LV_OBJ_FLAG_SCROLLABLE);

    min_hand = lv_image_create(face_batman_410);
    lv_image_set_src(min_hand, &face_batman_410_hand_minute);
    lv_obj_set_pos(min_hand, MAIN_CX - HAND_MIN_PIVOT_X, MAIN_CY - HAND_MIN_PIVOT_Y);
    lv_image_set_pivot(min_hand, HAND_MIN_PIVOT_X, HAND_MIN_PIVOT_Y);
    lv_obj_remove_flag(min_hand, LV_OBJ_FLAG_SCROLLABLE);

    sec_hand = lv_image_create(face_batman_410);
    lv_image_set_src(sec_hand, &face_batman_410_hand_second);
    lv_obj_set_pos(sec_hand, MAIN_CX - HAND_SEC_PIVOT_X, MAIN_CY - HAND_SEC_PIVOT_Y);
    lv_image_set_pivot(sec_hand, HAND_SEC_PIVOT_X, HAND_SEC_PIVOT_Y);
    lv_obj_remove_flag(sec_hand, LV_OBJ_FLAG_SCROLLABLE);

    // v5 hands carry their own baked-in gold hub, so the separately-drawn
    // cap circle below is redundant - commented out rather than deleted in
    // case we want the extra cap back on top (e.g. if the baked-in hub
    // reads too small once on real hardware).
    // lv_obj_t *cap = lv_obj_create(face_batman_410);
    // lv_obj_set_size(cap, 8, 8);
    // lv_obj_set_pos(cap, MAIN_CX - 4, MAIN_CY - 4);
    // lv_obj_set_style_radius(cap, LV_RADIUS_CIRCLE, 0);
    // lv_obj_set_style_bg_color(cap, lv_color_hex(0xFFD700), 0);
    // lv_obj_set_style_bg_opa(cap, LV_OPA_COVER, 0);
    // lv_obj_set_style_border_width(cap, 0, 0);

    /* ---- Subdial hands: reuse the single hand_sub sprite on both dials. ----
       Removed 2026-08-20 at user request ("remove vector drawing of smaller
       chronos dials, leave those alone for now") - the two subdial *faces*
       (raster, baked into face_bg) stay as-is, only the little animated
       needle sprites are pulled. Commented out, not deleted, in case they
       come back once the subdials get real chronograph data behind them. */
    // sub_hand_l = lv_image_create(face_batman_410);
    // lv_image_set_src(sub_hand_l, &face_batman_410_hand_sub);
    // lv_obj_set_pos(sub_hand_l, SUB_L_CX - 6, SUB_L_CY);
    // lv_image_set_pivot(sub_hand_l, 6, 0);
    // lv_obj_remove_flag(sub_hand_l, LV_OBJ_FLAG_SCROLLABLE);

    // sub_hand_r = lv_image_create(face_batman_410);
    // lv_image_set_src(sub_hand_r, &face_batman_410_hand_sub);
    // lv_obj_set_pos(sub_hand_r, SUB_R_CX - 6, SUB_R_CY);
    // lv_image_set_pivot(sub_hand_r, 6, 0);
    // lv_obj_remove_flag(sub_hand_r, LV_OBJ_FLAG_SCROLLABLE);

    callback("Batman", &face_batman_410_dial_img_preview, &face_batman_410, &sec_hand);

#endif
}

void update_time_batman_410(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday)
{
#ifdef ENABLE_FACE_BATMAN_410
    if (!face_batman_410 || !hour_hand)
    {
        return;
    }

    lv_label_set_text(day_label, DAY_NAMES[weekday % 7]);
    lv_label_set_text_fmt(sec_label, "%02d", second);
    lv_label_set_text_fmt(time_label, "%02d:%02d", hour, minute);
    lv_label_set_text_fmt(date_label, "%02d", day);

    float hour_angle = (hour % 12) * 30.0f + minute * 0.5f;
    float min_angle  = minute * 6.0f;
    float sec_angle  = second * 6.0f;

    lv_image_set_rotation(hour_hand, norm_angle_deci(hour_angle - HAND_BASE_DEG));
    lv_image_set_rotation(min_hand, norm_angle_deci(min_angle - HAND_BASE_DEG));
    lv_image_set_rotation(sec_hand, norm_angle_deci(sec_angle - HAND_BASE_DEG));

    // Subdial hand sweep removed 2026-08-20 along with their creation above -
    // see that comment. Left commented rather than deleted for the same
    // reason.
    // float sub_l_angle = second * 6.0f;
    // float sub_r_angle = (hour % 12) * 30.0f + minute * 0.5f;
    // lv_image_set_rotation(sub_hand_l, norm_angle_deci(sub_l_angle - HAND_SUB_BASE_DEG));
    // lv_image_set_rotation(sub_hand_r, norm_angle_deci(sub_r_angle - HAND_SUB_BASE_DEG));

#endif
}

void update_weather_batman_410(int temp, int icon)
{
#ifdef ENABLE_FACE_BATMAN_410
    if (!face_batman_410 || !weather_label)
    {
        return;
    }

    lv_label_set_text_fmt(weather_label, "%d°C", temp);

    if (weather_icon)
    {
        lv_image_set_src(weather_icon, WEATHER_ICONS[icon % 8]);
    }

#endif
}

void update_status_batman_410(int battery, bool connection){
#ifdef ENABLE_FACE_BATMAN_410
    if (!face_batman_410 || !batt_label)
    {
        return;
    }

    lv_label_set_text_fmt(batt_label, "%d%%", battery);

#endif
}

void update_activity_batman_410(int steps, int distance, int kcal)
{
#ifdef ENABLE_FACE_BATMAN_410
    if (!face_batman_410)
    {
        return;
    }

#endif
}

void update_health_batman_410(int bpm, int oxygen)
{
#ifdef ENABLE_FACE_BATMAN_410
    if (!face_batman_410)
    {
        return;
    }

#endif
}

void update_all_batman_410(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
    int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
#ifdef ENABLE_FACE_BATMAN_410
    update_time_batman_410(second, minute, hour, mode, am, day, month, year, weekday);
    update_weather_batman_410(temp, icon);
    update_status_batman_410(battery, connection);
    update_activity_batman_410(steps, distance, kcal);
    update_health_batman_410(bpm, oxygen);
#endif
}

void update_check_batman_410(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
    int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
#ifdef ENABLE_FACE_BATMAN_410
    if (root != face_batman_410)
    {
        return;
    }
    update_time_batman_410(second, minute, hour, mode, am, day, month, year, weekday);
    update_weather_batman_410(temp, icon);
    update_status_batman_410(battery, connection);
    update_activity_batman_410(steps, distance, kcal);
    update_health_batman_410(bpm, oxygen);
#endif
}
