
// Hand-authored watchface (not bin2lvgl-generated): a procedurally-drawn
// classic analog dial, ported from the user's own AnalogFace app in the
// OLEDS3Watch-joaquim project (esp-brookesia, same physical hardware -
// Waveshare ESP32-S3-Touch-AMOLED-2.06). Drawn entirely with LVGL canvas/line
// primitives rather than pre-rasterized sprite images - the one exception is
// the 12 o'clock cogs, which are baked raster images (assets/cogs_img.c, see
// create_gear() below) rather than built from LVGL widget primitives.
// Watchface: classic_410

#include <math.h>
#include <string.h>
#include "classic_410.h"

#ifdef ENABLE_FACE_CLASSIC_410

lv_obj_t *face_classic_410;

static void *canvas_buf  = NULL;
static lv_obj_t *dial_canvas = NULL;
static lv_obj_t *hour_hand   = NULL;
static lv_obj_t *min_hand    = NULL;
static lv_obj_t *sec_hand    = NULL;
static lv_obj_t *hour_stripe = NULL;
static lv_obj_t *min_stripe  = NULL;
// 1px fully-opaque line drawn on top of each (now translucent, see HAND_OPA) centre stripe,
// same points every update - the "opaque core inside a transparent stripe" requested.
static lv_obj_t *hour_stripe_core = NULL;
static lv_obj_t *min_stripe_core  = NULL;
static lv_obj_t *date_box    = NULL;
static lv_obj_t *date_label  = NULL;

// Charge window - near the "3" numeral. charge_dot and charge_bolt are alternate power-state
// indicators (never shown together, see update_status_classic_410): a small circle, hollow
// when on battery / filled when plugged in but not charging, or a lightning bolt in place of
// it when actively charging.
static lv_obj_t *charge_box   = NULL;
static lv_obj_t *charge_label = NULL;
static lv_obj_t *charge_dot   = NULL;
static lv_obj_t *charge_bolt  = NULL;
static lv_point_precise_t bolt_pts[4];

// 75% visibility on the main (hour/minute) hands, requested explicitly - 0.75*255 rounded.
#define HAND_OPA 191

// Mechanical "cogs" window at 12 o'clock, ported from the original RetroPie
// LVGL-sim prototype (/home/pi/lv_port_linux/src/watch_face.c on the
// 192.168.0.37 dev Pi, create_gear()/create_mech_window()/gear_timer_cb() -
// pulled 2026-07-30 via SSH, same source this project's own dial/hands/ticks
// were originally derived from). Three interlocking gears; originally each
// was a widget tree (arc + ~20-60 tooth/body/spool lv_obj children rotated
// together via lv_obj_set_style_transform_angle), replaced 2026-07-31 with a
// single baked chrome-gear raster image per gear (assets/cogs_img.c),
// rotated as a whole via lv_image_set_rotation - see create_gear() below.
static lv_obj_t *mech_win    = NULL;
static lv_obj_t *gear_layer  = NULL;
static lv_obj_t *gear_a      = NULL;
static lv_obj_t *gear_b      = NULL;
static lv_obj_t *gear_c      = NULL;
static lv_timer_t *gear_timer = NULL;

// Cogs window frame: an annulus sector centred on the dial, not a rectangle - top/bottom
// edges are arcs concentric with the dial, left/right edges are straight radial lines (so
// unlike a rectangle's corners, no part of the frame sticks out further from the dial centre
// than its own outer edge - see the 2026-07-30 canvas pass below for the actual draw calls).
// 2026-07-31: narrowed per explicit request - radial sides moved from +-40deg (11:40/12:20)
// to +-30deg (11 and 1 o'clock exactly), and both arcs brought in by 22.5% of the original
// 64px gap (110-46) toward each other (110->95.6, 46->60.4), then pushed back out again by
// 7.3% of that same original 64px measurement (not the narrowed one) once the heavier
// clipping on the gears themselves was confirmed to look good: 95.6+4.672=100.27,
// 60.4-4.672=55.73, rounded to 101/55 (net of both passes: outer 110->101, inner 46->55).
// 2026-08-29: bottom (inner) arc raised by 1/3 of the 46px gap it had with the top (outer)
// arc: 55 + 46/3 = 70.33 -> 70, outer arc left untouched. mech_win/MECH_H below are derived
// from (COG_OUTER_R - COG_INNER_R) with the top edge pinned at COG_OUTER_R, so this alone
// also pulls the gear cluster's own recentred position up by the same construction - no
// separate offset needed to "move the mechanism" in step with the window.
// 2026-08-29 (cont.): briefly shifted both arcs outward together by +17 (to 118/87) to bring
// this window's own gap to its nearest numerals (11/12/1) in line with the date window's - put
// back to 101/70 per explicit request right after seeing it on hardware; the numeral-gap
// normalization only applies to the charge window now (see its own comment below).
// 2026-08-30: requested as target on-screen points (top-left (0,0), this file's own
// SCREEN_CX/SCREEN_CY=(205,247) frame) for where the two arcs cross the *left* radial
// line - (147,152) for the top/outer arc, (166,183) for the bottom/inner arc - with the
// radial sides (COG_HALF_ANGLE_DEG, still 30deg either side of 12 o'clock) and the dial
// centre left untouched, i.e. only the two radii change, not the angle or centre. Both
// given points land within ~1-2px of the existing -30deg line (measured angle -31.4deg/
// -31.3deg respectively, well inside "around" tolerance for an eyeballed target), so this
// is a pure radius solve: R = distance from (205,247) to each point - outer
// sqrt(58^2+95^2)=111.3 -> 111, inner sqrt(39^2+64^2)=75.0 -> 75.
#define COG_OUTER_R 111
#define COG_INNER_R 75
#define COG_HALF_ANGLE_DEG 30.0f
// First attempt at the "top arc looks thin/messy" clipping bug added an *outward* margin
// to the mask's own bounds so it wouldn't eat the border's stroke - wrong fix, confirmed
// on hardware: it let the mask's actual clip boundary sit *outside* the visible border
// line instead of at/inside it, so gear teeth could poke out past the red line before
// finally getting cut off. Reverted - the mask clips at the *exact* window bounds again
// (no margin at all; see the real fix, a separate topmost border_canvas, further below).

// Gears a/b/c have 17/14/11 teeth (tools/facegen/gen_cogs_classic_410.py, module 4:
// 2*34/17 = 2*28/14 = 2*22/11 = 4 exactly - same tooth pitch on all three, so they
// actually mesh instead of just spinning near each other). Needed here (not just by
// gear_timer_cb below) for the phase-offset derivation right after.
#define GEAR_A_TEETH 17.0f
#define GEAR_B_TEETH 14.0f
#define GEAR_C_TEETH 11.0f

// Gear-mesh placement constants - see the exact-tangent derivation next to their use in
// init_face_classic_410() below. GEAR_BA_CLOCK_DEG (bearing from b to a) was the original
// hand-placed layout's angle, kept as-is; GEAR_BC_CLOCK_DEG (bearing from b to c) is
// deliberately *not* the original hand-placed angle - it's snapped so that
// (GEAR_BC_CLOCK_DEG - GEAR_BA_CLOCK_DEG) lands on an exact multiple of gear_b's own
// tooth pitch (360/14 = 25.714deg). That's what makes gear_b's single fixed tooth
// pattern able to face a *valley* toward both neighbours at once (see the phase-offset
// #defines below) - without this snap, at most one of the two meshes can ever look
// right, and which one drifts as the whole cluster's arm angle gets tuned. Reported as
// "gear C's rotation looks wrong vs gear B" - this is the fix (both position AND the
// dependent phase offsets below).
#define GEAR_BA_CLOCK_DEG (-58.0f)
#define GEAR_BC_CLOCK_DEG (GEAR_BA_CLOCK_DEG + 5.0f * (360.0f / GEAR_B_TEETH))
// User feedback round-trip: no clearance (0, narrow angles) read as "too close" (teeth
// interpenetrating heavily); +14px (still narrow angles) read as "too far apart"; +18px
// (wider angles above) was flagged as still a tooth-length too far; +8px confirmed
// closer but still wanted a *tiny* bit tighter.
#define GEAR_MESH_CLEARANCE 5.0f
#define GEAR_GROUP_SCALE 0.62f

// Static initial rotation for each gear (added to gear_timer_cb's animation from then
// on) so a *valley*, not a tooth, faces the neighbour it meshes with - tooth 0 sits at
// clock-angle 0 in the baked art (gen_cogs_classic_410.py's render_gear: `a = i*2*pi/N`
// starts at 0 = "up"), so facing a valley toward bearing `angle` needs a rotation of
// `angle - (half a tooth pitch)`. gear_b only gets one shot at this (a single fixed
// tooth pattern can't face two different directions at once) - the angle snap above
// makes both neighbours land on the same tooth-pitch residue, so in principle one
// rotation should satisfy both. On real hardware it didn't: user confirmed twice,
// looking at the actual device, that gear_b specifically needed an *additional* half a
// tooth-pitch rotation on top of that derivation to read as meshing rather than
// tooth-on-tooth. Model says this shouldn't be needed - going with what's confirmed on
// the physical part over the untested theory (`+ (180.0f / GEAR_B_TEETH)` at the end).
#define GEAR_A_PHASE_DEG ((GEAR_BA_CLOCK_DEG + 180.0f) - (180.0f / GEAR_A_TEETH))
#define GEAR_B_PHASE_DEG (GEAR_BC_CLOCK_DEG)
#define GEAR_C_PHASE_DEG ((GEAR_BC_CLOCK_DEG + 180.0f) - (180.0f / GEAR_C_TEETH))

static lv_point_precise_t hour_pts[2];
static lv_point_precise_t min_pts[2];
static lv_point_precise_t sec_pts[2];

// Source dial was authored at 410x462 for this exact screen (410-wide AMOLED).
// This board reports 410x494, so the dial is vertically centered with a
// small black letterbox top/bottom rather than stretched to fill the extra
// height.
#define CW 410
#define CH 462
#define SCREEN_H 494
#define Y_OFFSET ((SCREEN_H - CH) / 2)
#define CANVAS_CX (CW / 2)
#define CANVAS_CY (CH / 2)
#define SCREEN_CX (CW / 2)
#define SCREEN_CY (Y_OFFSET + CH / 2)
#define PI_F 3.14159265f

static const char *const DAY_NAMES[7] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

// A gear is now one baked chrome-gear image (assets/cogs_img.c, generated by
// tools/facegen/{gen_cogs_classic_410,mask_cogs_hi,bake_cogs_to_c}.py from the
// user's hi-res art, masked to the exact meshing silhouette: tooth count/taper/
// pitch chosen so all three gears share one module - see gear_timer_cb below).
// `r` is still each gear's *pitch* radius (same value the silhouette was built
// from), not the image's own on-disk size - the image is slightly larger than
// 2r (it includes the tooth tips and a little padding), so position it by its
// own centre (x+r, y+r) rather than assuming a 2r x 2r bounding box.
// `cx,cy` is this gear's true centre (not top-left) - scale is a transform around the
// pivot (set to the image's own centre), so the object's position/pivot math doesn't need
// to change with scale, only the drawn size does.
static lv_obj_t *create_gear(lv_obj_t *parent, int32_t cx, int32_t cy, float scale, const lv_img_dsc_t *img)
{
    lv_obj_t *g = lv_image_create(parent);
    lv_image_set_src(g, img);
    lv_obj_set_pos(g, cx - img->header.w / 2, cy - img->header.h / 2);
    lv_image_set_pivot(g, img->header.w / 2, img->header.h / 2);
    lv_image_set_scale(g, (uint32_t)lroundf(scale * 256.0f));
    lv_obj_remove_flag(g, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(g, LV_OBJ_FLAG_CLICKABLE);
    return g;
}

// Meshing gears turn at a rate inversely proportional to their tooth count
// (omega * teeth == constant across the train) and alternate direction gear-
// to-gear along the a-b-c chain (b sits between and meshes both a and c, so
// it turns opposite to both). K=51 keeps gear_a close to its old ~3deg/tick
// pace; b and c are then *derived*, not independently chosen.
#define GEAR_TRAIN_K 51.0f

// Angle accumulators kept in float degrees (not the tenths-of-degree
// lv_image_set_rotation itself takes) so the non-integer per-tick deltas
// (e.g. 51/14 = 3.643 deg) don't drift from repeated rounding - only the
// final lv_image_set_rotation call rounds, the running total stays exact.
// Start from the GEAR_*_PHASE_DEG offsets derived above (not 0) so the meshed
// look is correct from the very first frame, not just eventually by luck.
static float gear_a_deg = GEAR_A_PHASE_DEG;
static float gear_b_deg = GEAR_B_PHASE_DEG;
static float gear_c_deg = GEAR_C_PHASE_DEG;

// Only does work while the classic face is actually the visible screen, so it
// doesn't spend cycles animating an off-screen watchface.
static void gear_timer_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    if (lv_screen_active() != face_classic_410) return;

    gear_a_deg = fmodf(gear_a_deg + (GEAR_TRAIN_K / GEAR_A_TEETH), 360.0f);
    gear_b_deg = fmodf(gear_b_deg - (GEAR_TRAIN_K / GEAR_B_TEETH) + 360.0f, 360.0f);
    gear_c_deg = fmodf(gear_c_deg + (GEAR_TRAIN_K / GEAR_C_TEETH), 360.0f);

    lv_image_set_rotation(gear_a, (int32_t)lroundf(gear_a_deg * 10));
    lv_image_set_rotation(gear_b, (int32_t)lroundf(gear_b_deg * 10));
    lv_image_set_rotation(gear_c, (int32_t)lroundf(gear_c_deg * 10));
}

#endif

void init_face_classic_410(void (*callback)(const char*, const lv_img_dsc_t *, lv_obj_t **, lv_obj_t **)){
#ifdef ENABLE_FACE_CLASSIC_410
    face_classic_410 = lv_obj_create(NULL);
    lv_obj_remove_flag(face_classic_410, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(face_classic_410, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(face_classic_410, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(face_classic_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(face_classic_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(face_classic_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(face_classic_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(face_classic_410, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(face_classic_410, onFaceEvent, LV_EVENT_ALL, NULL);

    canvas_buf = lv_malloc(CW * CH * 2);
    if (!canvas_buf) {
        callback("Classic", &face_classic_410_dial_img_preview_0, &face_classic_410, NULL);
        return;
    }
    memset(canvas_buf, 0, CW * CH * 2);

    dial_canvas = lv_canvas_create(face_classic_410);
    lv_obj_add_flag(dial_canvas, LV_OBJ_FLAG_HIDDEN);
    lv_canvas_set_buffer(dial_canvas, canvas_buf, CW, CH, LV_COLOR_FORMAT_RGB565);
    lv_obj_set_pos(dial_canvas, 0, Y_OFFSET);

    lv_layer_t layer;

    /* ---- Pass 1: red border arc + 12 major ticks ---- */
    lv_canvas_init_layer(dial_canvas, &layer);

    lv_draw_arc_dsc_t arc;
    lv_draw_arc_dsc_init(&arc);
    arc.color = lv_color_hex(0xFF0000); arc.width = 8;
    arc.center.x = CANVAS_CX; arc.center.y = CANVAS_CY;
    arc.radius = 192; arc.start_angle = 0; arc.end_angle = 360;
    arc.opa = LV_OPA_COVER;
    lv_draw_arc(&layer, &arc);

    for (int i = 0; i < 12; i++) {
        float a = i * 30.0f * PI_F / 180.0f;
        lv_draw_line_dsc_t line; lv_draw_line_dsc_init(&line);
        line.color = lv_color_white(); line.width = 4; line.opa = LV_OPA_COVER;
        line.round_start = line.round_end = 1;
        line.p1.x = (int32_t)(CANVAS_CX + sinf(a) * 165); line.p1.y = (int32_t)(CANVAS_CY - cosf(a) * 165);
        line.p2.x = (int32_t)(CANVAS_CX + sinf(a) * 184); line.p2.y = (int32_t)(CANVAS_CY - cosf(a) * 184);
        lv_draw_line(&layer, &line);
    }
    lv_canvas_finish_layer(dial_canvas, &layer);

    /* ---- Pass 2: 48 minor ticks ---- */
    lv_canvas_init_layer(dial_canvas, &layer);
    for (int i = 0; i < 60; i++) {
        if (i % 5 == 0) continue;
        float a = i * 6.0f * PI_F / 180.0f;
        lv_draw_line_dsc_t line; lv_draw_line_dsc_init(&line);
        line.color = lv_color_white(); line.width = 2; line.opa = LV_OPA_COVER;
        line.round_start = line.round_end = 1;
        line.p1.x = (int32_t)(CANVAS_CX + sinf(a) * 176); line.p1.y = (int32_t)(CANVAS_CY - cosf(a) * 176);
        line.p2.x = (int32_t)(CANVAS_CX + sinf(a) * 184); line.p2.y = (int32_t)(CANVAS_CY - cosf(a) * 184);
        lv_draw_line(&layer, &line);
    }
    lv_canvas_finish_layer(dial_canvas, &layer);

    /* ---- Pass 3: all 12 hour numerals - the date window now sits between the dial centre
       and "6" rather than on top of it, so nothing needs to be skipped any more. ---- */
    static const char *hour_nums[12] = {
        "12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"
    };
    lv_canvas_init_layer(dial_canvas, &layer);
    for (int i = 0; i < 12; i++) {
        float a = i * 30.0f * PI_F / 180.0f;
        int32_t nx = CANVAS_CX + (int32_t)(sinf(a) * 143.0f);
        int32_t ny = CANVAS_CY - (int32_t)(cosf(a) * 143.0f);
        lv_draw_label_dsc_t lbl; lv_draw_label_dsc_init(&lbl);
        lbl.font = &lv_font_montserrat_36; lbl.color = lv_color_white();
        lbl.align = LV_TEXT_ALIGN_CENTER; lbl.opa = LV_OPA_COVER;
        lbl.text = hour_nums[i];
        lv_area_t area = { nx - 30, ny - 22, nx + 30, ny + 22 };
        lv_draw_label(&layer, &lbl, &area);
    }
    lv_canvas_finish_layer(dial_canvas, &layer);

    /* ---- Pass 4: cogs window frame - dark band fill only. The red border (arcs + radial
       sides) used to be drawn right here too, but that put it *underneath* the gear
       erase-mask in z-order (mask created later, as a sibling of dial_canvas, paints over
       whatever's below it in that region) - the mask had to carry an outward margin just
       to avoid eating the border's own stroke, which then let gear teeth poke out past the
       visible red line before the (now too-generous) mask finally clipped them. Moved to a
       dedicated border_canvas created *after* the mask (see below), so the mask can clip
       tightly at the exact window bounds and the border is simply always on top,
       independent of that entirely. */
    lv_canvas_init_layer(dial_canvas, &layer);
    {
        // Filled band: a wide arc stroke, radius = band midline, width = band thickness.
        lv_draw_arc_dsc_t band; lv_draw_arc_dsc_init(&band);
        band.color = lv_color_hex(0x141414); band.opa = LV_OPA_COVER;
        band.center.x = CANVAS_CX; band.center.y = CANVAS_CY;
        band.radius = (COG_OUTER_R + COG_INNER_R) / 2;
        band.width = COG_OUTER_R - COG_INNER_R;
        band.rounded = 0;
        band.start_angle = 270.0f - COG_HALF_ANGLE_DEG;
        band.end_angle   = 270.0f + COG_HALF_ANGLE_DEG;
        lv_draw_arc(&layer, &band);
    }
    lv_canvas_finish_layer(dial_canvas, &layer);

    lv_obj_clear_flag(dial_canvas, LV_OBJ_FLAG_HIDDEN);

    /* ---- Date window - between the dial centre and the "6" numeral. Radius offset 55 is
       halfway between the previous pass's 40 and the pass before that's 71.
       2026-08-29: shifted down by half its own height (44/2=22, so 55-22 -> 55) per
       explicit request.
       2026-08-29 (cont.): this box's own closest-numeral gap - its bottom edge (canvas y
       55+44=99 from centre) to numerals "7"/"5" (radius 143, 30deg either side of straight
       down, label half-height 22: near edge at 143*cos(30deg)-22 = 123.84-22 = 101.84 from
       centre) - is G = 101.84-99 = 2.84 canvas px, confirmed as the target look. The charge
       window to its right was re-derived to hit this same G against its own nearest numeral
       (3) - see that window's own comment below. The cogs window was also tried against this
       G at one point (its own comment above still notes the numbers) but was put back to its
       prior position on request, so it's the odd one out here. ---- */
    date_box = lv_obj_create(face_classic_410);
    // 2026-08-29: width brought in 16px (136 -> 120), kept centred (half-width 68 -> 60) - the
    // vertical gap to numerals "7"/"5" (G, see above) is unaffected since it only depends on
    // the box's top/bottom edges and x-overlap with those numerals still holds at this width.
    lv_obj_set_size(date_box, 120, 44);
    lv_obj_set_pos(date_box, SCREEN_CX - 60, SCREEN_CY + 55);
    lv_obj_set_style_bg_color(date_box, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(date_box, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(date_box, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_border_width(date_box, 2, 0);
    lv_obj_set_style_radius(date_box, 8, 0);
    lv_obj_set_style_pad_all(date_box, 4, 0);
    lv_obj_remove_flag(date_box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(date_box, LV_OBJ_FLAG_CLICKABLE);

    date_label = lv_label_create(date_box);
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(date_label, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(date_label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(date_label, 0, 0);
    lv_label_set_text(date_label, "??? --");
    lv_obj_center(date_label);

    /* ---- Charge window - near the "3" numeral. Box is narrower than the date window (92 vs
       136) since "100%" plus the power-state indicator needs less width than "SUN 31".
       Radius offset history, tuned against the actual hardware render rather than pure
       label-box math (which twice missed - the "3" glyph's real ink extent inside its 60px
       label box, sized for two-digit numerals, isn't something this code can measure):
       55 (original) -> 64 (matched to G against the label box's full edge - confirmed still
       too far, i.e. gap bigger than the date window's own G) -> 84 (matched to G against an
       assumed 10px glyph half-width - confirmed overshot, gap smaller than G). 64 and 84
       bracket the real answer from opposite sides, so rather than guess a third glyph width,
       split the difference: 74. ---- */
    {
        const int32_t CHG_W = 92, CHG_H = 40;
        const int32_t CHG_RADIUS = 74;
        int32_t chg_x = SCREEN_CX + CHG_RADIUS - CHG_W / 2;
        int32_t chg_y = SCREEN_CY - CHG_H / 2;

        charge_box = lv_obj_create(face_classic_410);
        lv_obj_set_size(charge_box, CHG_W, CHG_H);
        lv_obj_set_pos(charge_box, chg_x, chg_y);
        lv_obj_set_style_bg_color(charge_box, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(charge_box, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(charge_box, lv_palette_main(LV_PALETTE_RED), 0);
        lv_obj_set_style_border_width(charge_box, 2, 0);
        lv_obj_set_style_radius(charge_box, 8, 0);
        lv_obj_set_style_pad_all(charge_box, 4, 0);
        lv_obj_remove_flag(charge_box, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(charge_box, LV_OBJ_FLAG_CLICKABLE);

        charge_label = lv_label_create(charge_box);
        lv_obj_set_style_text_font(charge_label, &lv_font_montserrat_22, 0);
        lv_obj_set_style_text_color(charge_label, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(charge_label, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(charge_label, 0, 0);
        lv_label_set_text(charge_label, "--%");
        // 2026-08-29: swapped to the right side of the box (was LEFT_MID) so the
        // power-state indicator - now on the left, see charge_dot/charge_bolt below - reads
        // before the percentage, left to right. Nudged +2px right again right after (-4 -> -2).
        lv_obj_align(charge_label, LV_ALIGN_RIGHT_MID, -2, 0);

        // Power-state indicator, left side of the box (2026-08-29: swapped from the right,
        // per explicit request), vertically centred on it (chg_cy). None of this project's
        // built-in font sizes carry LV_SYMBOL_USB/CHARGE/BATTERY_* glyphs (checked against the
        // actual compiled lv_font_montserrat_*.c files - only a small hand-picked symbol
        // subset is baked in, and charge-related ones aren't in it), and the file header above
        // commits this face to LVGL primitives rather than new raster assets, so the indicator
        // is drawn as a plain circle (charge_dot) plus a separate bolt line (charge_bolt)
        // instead of a font glyph or image.
        int32_t chg_cy = chg_y + CHG_H / 2;

        charge_dot = lv_obj_create(face_classic_410);
        lv_obj_set_size(charge_dot, 14, 14);
        // 2026-08-29: nudged +2px right (chg_x+6 -> chg_x+8), same request as charge_label above.
        lv_obj_set_pos(charge_dot, chg_x + 8, chg_cy - 7);
        lv_obj_set_style_radius(charge_dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(charge_dot, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(charge_dot, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_color(charge_dot, lv_color_white(), 0);
        lv_obj_set_style_border_width(charge_dot, 2, 0);
        lv_obj_remove_flag(charge_dot, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(charge_dot, LV_OBJ_FLAG_CLICKABLE);

        // Static zigzag bolt, same absolute-screen-coordinate convention the hands use below
        // (lv_line points given directly in screen space since face_classic_410's own origin
        // is the screen origin) - the shape never changes at runtime, only its hidden flag
        // does, toggled opposite to charge_dot in update_status_classic_410.
        // 2026-08-29: nudged +2px right (chg_x+9 -> chg_x+11), same request as charge_dot above.
        int32_t bolt_x = chg_x + 11, bolt_y = chg_cy - 8;
        bolt_pts[0] = (lv_point_precise_t){ (lv_value_precise_t)(bolt_x + 3), (lv_value_precise_t)bolt_y };
        bolt_pts[1] = (lv_point_precise_t){ (lv_value_precise_t)(bolt_x - 3), (lv_value_precise_t)(bolt_y + 8) };
        bolt_pts[2] = (lv_point_precise_t){ (lv_value_precise_t)(bolt_x + 1), (lv_value_precise_t)(bolt_y + 8) };
        bolt_pts[3] = (lv_point_precise_t){ (lv_value_precise_t)(bolt_x - 3), (lv_value_precise_t)(bolt_y + 16) };

        charge_bolt = lv_line_create(face_classic_410);
        lv_obj_set_style_line_color(charge_bolt, lv_color_hex(0xFFD400), 0);
        lv_obj_set_style_line_width(charge_bolt, 3, 0);
        lv_obj_set_style_line_rounded(charge_bolt, true, 0);
        lv_line_set_points(charge_bolt, bolt_pts, 4);
        lv_obj_remove_flag(charge_bolt, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(charge_bolt, LV_OBJ_FLAG_HIDDEN);
    }

    /* ---- Cogs window - 12 o'clock. The visible frame (dark fill, red arc/radial border) is
       now drawn on the canvas in Pass 4 above, at radius COG_OUTER_R..COG_INNER_R; mech_win
       here is just a plain (invisible) clipping rect for the gears, sized/positioned to bound
       that same annulus sector so the two line up. Gears themselves (below) are untouched from
       the original rectangular-window version - same sizes, same layout relative to this rect. ---- */
    {
        const int32_t MECH_W = 128;
        // Padding beyond the sector's own radial span so gear content (which can legitimately
        // extend past COG_INNER_R before the curved erase-mask clips it) doesn't hit mech_win's
        // own flat rectangle edge first - that was "a straight horizontal clip on the bottom
        // of the middle cog". Bottom-only, *not* symmetric: padding the top edge too (this
        // rect's top sits right at COG_OUTER_R, and the "11" numeral's bounding box comes within
        // ~1px of that same radius - see Pass 3) pushed the rect far enough out to newly start
        // covering part of "11" with the mask's opaque fill. The top edge stays exactly at
        // COG_OUTER_R; only MECH_H grows, extending the bottom edge further from centre.
        const int32_t GEAR_RECT_PAD = 20;
        const int32_t MECH_H = (COG_OUTER_R - COG_INNER_R) + GEAR_RECT_PAD;
        int32_t mech_x = SCREEN_CX - MECH_W / 2;
        int32_t mech_y = SCREEN_CY - COG_OUTER_R;

        mech_win = lv_obj_create(face_classic_410);
        lv_obj_set_size(mech_win, MECH_W, MECH_H);
        lv_obj_set_pos(mech_win, mech_x, mech_y);
        lv_obj_set_style_bg_opa(mech_win, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(mech_win, 0, 0);
        lv_obj_set_style_pad_all(mech_win, 0, 0);
        lv_obj_remove_flag(mech_win, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(mech_win, LV_OBJ_FLAG_CLICKABLE);

        // Oversized inner layer (window clips it) so gear coordinates stay simple positive
        // numbers, same trick as the source's gear_layer.
        gear_layer = lv_obj_create(mech_win);
        lv_obj_set_pos(gear_layer, 0, 0);
        lv_obj_set_size(gear_layer, 260, 200);
        lv_obj_set_style_bg_opa(gear_layer, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(gear_layer, 0, 0);
        lv_obj_set_style_pad_all(gear_layer, 0, 0);
        lv_obj_remove_flag(gear_layer, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(gear_layer, LV_OBJ_FLAG_CLICKABLE);

        const int32_t ga_r = 34, gb_r = 28, gc_r = 22;

        // Exact mesh placement: a chain (a-b mesh, b-c mesh; a and c do NOT mesh with each
        // other) - matches gear_timer_cb's alternating a+/b-/c+ direction assumption, and is
        // the only topology that *can* rotate at all: three mutually-meshing external gears
        // in a closed triangle is a mechanically impossible odd cycle (going around the loop,
        // each external mesh reverses direction, so after 3 reversals you're back to needing
        // the first gear to spin both ways at once) - same parity argument as why a triangle
        // isn't 2-colourable. Two circles mesh exactly when their centre-to-centre distance
        // equals the sum of their pitch radii - the previous hand-placed positions only
        // approximated this (~49px apart where 62px was required for a-b), which the old
        // widget teeth could get away with visually but the raster images exposed as heavy
        // overlap ("resized the cogs wrongly" - the images were fine, the *spacing* had
        // always been approximate, just not visible until now).
        //
        // Arm directions below preserve the original hand-placed layout's angles (b->a up-
        // left, b->c up-right, same asymmetry as before) in this file's existing clock-angle
        // convention (0=up/12, +90=right/3, matches the erase-mask sector test below) - only
        // the LENGTH changes, from eyeballed to exact. GEAR_GROUP_SCALE (exact-tangent spacing
        // plus the raster images' own tooth-tip/padding margin makes the whole cluster wider/
        // taller than the 128x64 mech window can show uncropped) scales position offsets AND
        // each gear's own displayed size together - uniform, so it only changes absolute
        // size, not the ratios meshing depends on.
        float ba_dist = (float)(ga_r + gb_r + GEAR_MESH_CLEARANCE) * GEAR_GROUP_SCALE;
        float bc_dist = (float)(gb_r + gc_r + GEAR_MESH_CLEARANCE) * GEAR_GROUP_SCALE;
        float ba_rad = GEAR_BA_CLOCK_DEG * PI_F / 180.0f;
        float bc_rad = GEAR_BC_CLOCK_DEG * PI_F / 180.0f;

        // b is the reference point - NOT (0,0): gear_layer clips its own children to its
        // declared 260x200 box by default (LVGL clips to parent bounds unless
        // LV_OBJ_FLAG_OVERFLOW_VISIBLE is set, which it isn't), so a and c's up-left/up-right
        // offsets from b would silently vanish if b were at the origin and they ended up at
        // negative local coordinates. b is placed comfortably inside that box instead; the
        // recentre step below shifts gear_layer's own position afterwards regardless (that
        // shift isn't clipped, only the children's positions relative to gear_layer are).
        float bx = 130.0f, by = 100.0f;
        float ax = bx + sinf(ba_rad) * ba_dist, ay = by - cosf(ba_rad) * ba_dist;
        float ccx = bx + sinf(bc_rad) * bc_dist, ccy = by - cosf(bc_rad) * bc_dist;

        gear_a = create_gear(gear_layer, (int32_t)lroundf(ax), (int32_t)lroundf(ay), GEAR_GROUP_SCALE, &classic_410_gear_a_img);
        gear_b = create_gear(gear_layer, (int32_t)lroundf(bx), (int32_t)lroundf(by), GEAR_GROUP_SCALE, &classic_410_gear_b_img);
        gear_c = create_gear(gear_layer, (int32_t)lroundf(ccx), (int32_t)lroundf(ccy), GEAR_GROUP_SCALE, &classic_410_gear_c_img);

        // Recentre the group of three gear centres inside the MECH_W x MECH_H window by
        // shifting gear_layer as a whole (same approach as the source).
        float minx = ax, maxx = ax, miny = ay, maxy = ay;
        if (bx < minx) minx = bx; if (bx > maxx) maxx = bx;
        if (ccx < minx) minx = ccx; if (ccx > maxx) maxx = ccx;
        if (by < miny) miny = by; if (by > maxy) maxy = by;
        if (ccy < miny) miny = ccy; if (ccy > maxy) maxy = ccy;
        float group_cx = (minx + maxx) * 0.5f, group_cy = (miny + maxy) * 0.5f;
        float target_cx = (float)MECH_W * 0.5f, target_cy = (float)MECH_H * 0.5f;
        lv_obj_set_pos(gear_layer, (int32_t)lroundf(target_cx - group_cx), (int32_t)lroundf(target_cy - group_cy));

        // Erase mask: "a window is a window" - anything the gears draw outside the actual
        // keystone shape (not just the rectangular gear_layer clip) needs to disappear, same
        // hard edge the rectangle already gives on its own sides. Since LVGL has no built-in
        // way to clip a widget tree to an arbitrary sector, this is done as a same-size
        // overlay canvas sitting on top of the gears (created after them, so it paints over
        // them), opaque everywhere in the MECH_W x MECH_H rect that's NOT inside the sector
        // (radius/angle test per pixel, done once at init - the sector itself is static, only
        // the gears under it rotate), fully transparent everywhere inside it so the gears show
        // through untouched. Mask only ever needs black-or-nothing, so this is an A8
        // (1 byte/px alpha-only) buffer with image_recolor forcing the opaque pixels to black,
        // not ARGB8888 (4 bytes/px) - same visual result at 1/4 the permanent heap footprint
        // (this buffer is never freed, it has to stay resident for every subsequent frame).
        void *mask_buf = lv_malloc(MECH_W * MECH_H * 1);
        if (mask_buf) {
            lv_obj_t *mask_canvas = lv_canvas_create(face_classic_410);
            lv_canvas_set_buffer(mask_canvas, mask_buf, MECH_W, MECH_H, LV_COLOR_FORMAT_A8);
            lv_obj_set_pos(mask_canvas, mech_x, mech_y);
            lv_obj_remove_flag(mask_canvas, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_image_recolor(mask_canvas, lv_color_black(), 0);
            lv_obj_set_style_image_recolor_opa(mask_canvas, LV_OPA_COVER, 0);
            lv_canvas_fill_bg(mask_canvas, lv_color_black(), LV_OPA_TRANSP);

            for (int32_t py = 0; py < MECH_H; py++) {
                for (int32_t px = 0; px < MECH_W; px++) {
                    float dx = (float)(mech_x + px) - CANVAS_CX;
                    float dy = (float)(mech_y - Y_OFFSET + py) - CANVAS_CY;
                    float rad = sqrtf(dx * dx + dy * dy);
                    float clock_deg = atan2f(dx, -dy) * 180.0f / PI_F;
                    bool inside = (rad >= COG_INNER_R && rad <= COG_OUTER_R &&
                                   fabsf(clock_deg) <= COG_HALF_ANGLE_DEG);
                    if (!inside) {
                        lv_canvas_set_px(mask_canvas, px, py, lv_color_black(), LV_OPA_COVER);
                    }
                }
            }
        }

        // Border (red arcs + radial sides): a small ARGB8888 canvas (needs real colour +
        // per-pixel alpha, unlike the black-only mask above) created *after* mask_canvas so
        // it always paints on top of it - the border is then simply never affected by the
        // mask's clipping at all, however tight, which is what actually fixes "top arc
        // looks messy/thin" without reintroducing the "clips outside the window lines"
        // problem the first (margin-based) attempt caused.
        void *border_buf = lv_malloc(MECH_W * MECH_H * 4);
        if (border_buf) {
            lv_obj_t *border_canvas = lv_canvas_create(face_classic_410);
            lv_canvas_set_buffer(border_canvas, border_buf, MECH_W, MECH_H, LV_COLOR_FORMAT_ARGB8888);
            lv_obj_set_pos(border_canvas, mech_x, mech_y);
            lv_obj_remove_flag(border_canvas, LV_OBJ_FLAG_CLICKABLE);
            lv_canvas_fill_bg(border_canvas, lv_color_black(), LV_OPA_TRANSP);

            // Same geometry as the band above and the old border, just re-expressed in
            // border_canvas's own local coordinate space - its origin is (mech_x,mech_y) on
            // screen, not dial_canvas's own (0,Y_OFFSET) origin, so the dial's true centre
            // here is (bcx,bcy), not (CANVAS_CX,CANVAS_CY).
            float half_a = COG_HALF_ANGLE_DEG * PI_F / 180.0f;
            float bcx = (float)(CANVAS_CX - mech_x);
            float bcy = (float)(SCREEN_CY - mech_y);

            lv_layer_t blayer;
            lv_canvas_init_layer(border_canvas, &blayer);

            lv_draw_arc_dsc_t border; lv_draw_arc_dsc_init(&border);
            border.color = lv_color_hex(0xFF0000); border.opa = LV_OPA_COVER;
            border.center.x = (int32_t)lroundf(bcx); border.center.y = (int32_t)lroundf(bcy);
            border.width = 2; border.rounded = 0;
            border.start_angle = 270.0f - COG_HALF_ANGLE_DEG;
            border.end_angle   = 270.0f + COG_HALF_ANGLE_DEG;

            border.radius = COG_OUTER_R;
            lv_draw_arc(&blayer, &border);
            border.radius = COG_INNER_R;
            lv_draw_arc(&blayer, &border);

            lv_draw_line_dsc_t side; lv_draw_line_dsc_init(&side);
            side.color = lv_color_hex(0xFF0000); side.width = 2; side.opa = LV_OPA_COVER;
            side.round_start = side.round_end = 0;

            side.p1.x = (int32_t)(bcx + sinf(-half_a) * COG_OUTER_R);
            side.p1.y = (int32_t)(bcy - cosf(-half_a) * COG_OUTER_R);
            side.p2.x = (int32_t)(bcx + sinf(-half_a) * COG_INNER_R);
            side.p2.y = (int32_t)(bcy - cosf(-half_a) * COG_INNER_R);
            lv_draw_line(&blayer, &side);

            side.p1.x = (int32_t)(bcx + sinf(half_a) * COG_OUTER_R);
            side.p1.y = (int32_t)(bcy - cosf(half_a) * COG_OUTER_R);
            side.p2.x = (int32_t)(bcx + sinf(half_a) * COG_INNER_R);
            side.p2.y = (int32_t)(bcy - cosf(half_a) * COG_INNER_R);
            lv_draw_line(&blayer, &side);

            lv_canvas_finish_layer(border_canvas, &blayer);
        }
    }

    /* ---- Clock hands ---- */
    // Main hands (hour/minute) only, per explicit request - the second hand stays fully
    // opaque as an accent, not a "main hand". The hour/minute centre stripes get the same
    // HAND_OPA applied below too (also requested), each with a separate 1px fully-opaque
    // core line (hour_stripe_core/min_stripe_core, same points, drawn right on top) so a
    // thin opaque line still shows through the middle of each translucent stripe.
    // 2026-08-29: widened 7 -> 11 (+2px, then +2px more on top of that per explicit request -
    // this was originally applied to the wrong hand, see min_hand below) - only the white
    // part, the red centre stripe/core on top are unchanged ("not inner red parts"). Tip
    // length shortened separately below in update_time_classic_410 (hour_pts[1]) to 75% of
    // its original 85 per explicit request; tail length (hour_pts[0]) untouched.
    hour_hand = lv_line_create(face_classic_410);
    lv_obj_set_style_line_color(hour_hand, lv_color_white(), 0);
    lv_obj_set_style_line_width(hour_hand, 11, 0);
    lv_obj_set_style_line_rounded(hour_hand, true, 0);
    lv_obj_set_style_line_opa(hour_hand, HAND_OPA, 0);

    // 2026-08-29: widened 4 -> 6 (+2px, explicit request), separate from the hour hand's own
    // width/length changes above.
    min_hand = lv_line_create(face_classic_410);
    lv_obj_set_style_line_color(min_hand, lv_color_white(), 0);
    lv_obj_set_style_line_width(min_hand, 6, 0);
    lv_obj_set_style_line_rounded(min_hand, true, 0);
    lv_obj_set_style_line_opa(min_hand, HAND_OPA, 0);

    hour_stripe = lv_line_create(face_classic_410);
    lv_obj_set_style_line_color(hour_stripe, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_line_width(hour_stripe, 2, 0);
    lv_obj_set_style_line_rounded(hour_stripe, true, 0);
    lv_obj_set_style_line_opa(hour_stripe, HAND_OPA, 0);

    hour_stripe_core = lv_line_create(face_classic_410);
    lv_obj_set_style_line_color(hour_stripe_core, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_line_width(hour_stripe_core, 1, 0);
    lv_obj_set_style_line_rounded(hour_stripe_core, true, 0);

    min_stripe = lv_line_create(face_classic_410);
    lv_obj_set_style_line_color(min_stripe, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_line_width(min_stripe, 2, 0);
    lv_obj_set_style_line_rounded(min_stripe, true, 0);
    lv_obj_set_style_line_opa(min_stripe, HAND_OPA, 0);

    min_stripe_core = lv_line_create(face_classic_410);
    lv_obj_set_style_line_color(min_stripe_core, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_line_width(min_stripe_core, 1, 0);
    lv_obj_set_style_line_rounded(min_stripe_core, true, 0);

    sec_hand = lv_line_create(face_classic_410);
    lv_obj_set_style_line_color(sec_hand, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_line_width(sec_hand, 2, 0);
    lv_obj_set_style_line_rounded(sec_hand, true, 0);

    lv_obj_t *cap = lv_obj_create(face_classic_410);
    lv_obj_set_size(cap, 14, 14);
    lv_obj_set_pos(cap, SCREEN_CX - 7, SCREEN_CY - 7);
    lv_obj_set_style_radius(cap, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(cap, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(cap, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cap, 0, 0);

    /* Cogs animation timer - same 50ms cadence as the RetroPie source, gated to only spend
       cycles while this face is the visible screen (see gear_timer_cb above). */
    gear_timer = lv_timer_create(gear_timer_cb, 50, NULL);
    gear_timer_cb(gear_timer);

    callback("Classic", &face_classic_410_dial_img_preview_0, &face_classic_410, &sec_hand);

#endif
}

static lv_point_precise_t hand_tip(int cx, int cy, float deg, float len)
{
#ifdef ENABLE_FACE_CLASSIC_410
    float rad = deg * PI_F / 180.0f;
    lv_point_precise_t p = {
        (lv_value_precise_t)(cx + (int32_t)(sinf(rad) * len)),
        (lv_value_precise_t)(cy - (int32_t)(cosf(rad) * len)),
    };
    return p;
#else
    lv_point_precise_t p = {0, 0};
    return p;
#endif
}

void update_time_classic_410(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday)
{
#ifdef ENABLE_FACE_CLASSIC_410
    if (!face_classic_410 || !hour_hand)
    {
        return;
    }

    lv_label_set_text_fmt(date_label, "%s %02d", DAY_NAMES[weekday % 7], day);

    float hour_angle = (hour % 12) * 30.0f + minute * 0.5f;
    float min_angle  = minute * 6.0f;
    float sec_angle  = second * 6.0f;

    hour_pts[0] = hand_tip(SCREEN_CX, SCREEN_CY, hour_angle + 180.0f, 15.0f);
    // 2026-08-29: shortened 85 -> 64 (75% of original 85, i.e. 85*0.75=63.75 rounded), per
    // explicit request - this was originally (mis-)applied to the minute hand below instead.
    hour_pts[1] = hand_tip(SCREEN_CX, SCREEN_CY, hour_angle, 64.0f);
    lv_line_set_points(hour_hand, hour_pts, 2);
    lv_line_set_points(hour_stripe, hour_pts, 2);
    lv_line_set_points(hour_stripe_core, hour_pts, 2);

    min_pts[0] = hand_tip(SCREEN_CX, SCREEN_CY, min_angle + 180.0f, 20.0f);
    min_pts[1] = hand_tip(SCREEN_CX, SCREEN_CY, min_angle, 130.0f);
    lv_line_set_points(min_hand, min_pts, 2);
    lv_line_set_points(min_stripe, min_pts, 2);
    lv_line_set_points(min_stripe_core, min_pts, 2);

    sec_pts[0] = (lv_point_precise_t){ (lv_value_precise_t)SCREEN_CX, (lv_value_precise_t)SCREEN_CY };
    sec_pts[1] = hand_tip(SCREEN_CX, SCREEN_CY, sec_angle, 150.0f);
    lv_line_set_points(sec_hand, sec_pts, 2);

#endif
}

void update_weather_classic_410(int temp, int icon)
{
#ifdef ENABLE_FACE_CLASSIC_410
    if (!face_classic_410)
    {
        return;
    }

#endif
}

void update_status_classic_410(int battery, bool connection, bool plugged, bool charging){
#ifdef ENABLE_FACE_CLASSIC_410
    if (!face_classic_410)
    {
        return;
    }

    lv_label_set_text_fmt(charge_label, "%d%%", battery);

    if (charging) {
        lv_obj_add_flag(charge_dot, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(charge_bolt, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(charge_bolt, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(charge_dot, LV_OBJ_FLAG_HIDDEN);
        // Filled when plugged in (not charging - topped up), hollow ring when on battery.
        lv_obj_set_style_bg_opa(charge_dot, plugged ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    }

#endif
}

void update_activity_classic_410(int steps, int distance, int kcal)
{
#ifdef ENABLE_FACE_CLASSIC_410
    if (!face_classic_410)
    {
        return;
    }

#endif
}

void update_health_classic_410(int bpm, int oxygen)
{
#ifdef ENABLE_FACE_CLASSIC_410
    if (!face_classic_410)
    {
        return;
    }

#endif
}

void update_all_classic_410(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
    int temp, int icon, int battery, bool connection, bool plugged, bool charging, int steps, int distance, int kcal, int bpm, int oxygen)
{
#ifdef ENABLE_FACE_CLASSIC_410
    update_time_classic_410(second, minute, hour, mode, am, day, month, year, weekday);
    update_weather_classic_410(temp, icon);
    update_status_classic_410(battery, connection, plugged, charging);
    update_activity_classic_410(steps, distance, kcal);
    update_health_classic_410(bpm, oxygen);
#endif
}

void update_check_classic_410(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday,
    int temp, int icon, int battery, bool connection, bool plugged, bool charging, int steps, int distance, int kcal, int bpm, int oxygen)
{
#ifdef ENABLE_FACE_CLASSIC_410
    if (root != face_classic_410)
    {
        return;
    }
    update_time_classic_410(second, minute, hour, mode, am, day, month, year, weekday);
    update_weather_classic_410(temp, icon);
    update_status_classic_410(battery, connection, plugged, charging);
    update_activity_classic_410(steps, distance, kcal);
    update_health_classic_410(bpm, oxygen);
#endif
}
