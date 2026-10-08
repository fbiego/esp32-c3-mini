import os, sys
sys.path.insert(0, os.path.dirname(__file__))
from lvgl_img import encode_from_rgba, format_c_array
from PIL import Image

# One-off: converts the hand-drawn Batman chronograph face (delivered as a zip
# of plain PNGs + a generic SPIFFS-loading .c/.h that doesn't match this
# project's compiled-in lv_img_dsc_t convention - see src/faces/batman_410/
# batman_watchface_410x494.zip's own README) into the project's normal asset
# shape, same as merge_citizen_410_bg.py / gen_red_magic_410.py did for their
# faces. Background here is already pre-scaled to exactly 410x494 (this
# project's standard "content canvas" size, see citizen_410.c/classic_410.c
# SCREEN_H) before this script runs, so no scaling/letterboxing math is
# needed in here itself.
#
# v5 (2026-08-20): source dir now batman_v5_410x494/, which already holds
# pre-processed assets (background pillarboxed to 410x494, hands rotated
# from their as-delivered diagonal pose to point straight up and rescaled to
# the canvas) - see that directory's own README for the full pixel-math.
# Unlike v4, these hand sprites carry a baked-in hub+tail with the pivot
# point inside the image rather than at row 0, so batman_410.c's per-hand
# pivot coordinates and HAND_BASE_DEG changed accordingly.
#
# v5.1 (2026-08-21): hand_min.png rescaled in place (24x138 -> 31x179, pivot
# 12,125 -> 16,162) so the minute hand's pivot-to-tip length reaches the
# measured 3-o'clock minute-tick radius (~162px, see batman_410.c HAND_MIN_
# PIVOT_* comment) instead of undershooting it. Also added day_bg_patch.png
# and sec_bg_patch.png: small crops of the real background with the baked
# "FRI" day-window text and the baked "31" number above the bat symbol
# removed (OpenCV inpaint over a hand-picked letter mask, done once outside
# this script), used as opaque overlay windows so a live weekday label and a
# live seconds counter can be drawn on top without disturbing the rest of
# the art. See batman_410.c for the label placement.
#
# v5.2 (2026-08-21, same day): four more baked readouts patched the same way
# - time_bg_patch.png ("14:37" -> live HH:MM), batt_bg_patch.png ("78%" ->
# live battery), weather_bg_patch.png ("22°C" -> live temperature, the sun
# icon itself left as static art - no icon-code convention exists anywhere
# else in this codebase to drive a real icon swap from, see batman_410.c),
# date_bg_patch.png ("31" -> live day-of-month). The weather and date masks
# use a yellow-excluding threshold (keep only near-neutral bright pixels,
# not gold ones) since the sun icon and the bat's wingtip sit close enough
# to those two crops that a plain brightness threshold would eat into them.

SRC_DIR = "/home/greg/chronos-watch/src/faces/batman_410/batman_v5_410x494"
OUT_DIR = "/home/greg/chronos-watch/src/faces/batman_410/assets"
os.makedirs(OUT_DIR, exist_ok=True)


def write_asset(name, img, has_alpha, header_extra=""):
    data = encode_from_rgba(img, has_alpha)
    w, h = img.size
    cf = "LV_COLOR_FORMAT_NATIVE_WITH_ALPHA" if has_alpha else "LV_COLOR_FORMAT_RGB565"
    comment = "//RGB565 data with alpha" if has_alpha else "//RGB565 data"
    c_src = f'''{header_extra}#include "../batman_410.h"

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

const LV_ATTRIBUTE_MEM_ALIGN uint8_t {name}_data[] = {{
\t{comment}
{format_c_array(data)}
}};

const lv_img_dsc_t {name} = {{
    .header.magic = LV_IMAGE_HEADER_MAGIC,
    .header.w = {w},
    .header.h = {h},
    .data_size = sizeof({name}_data),
    .header.cf = {cf},
    .data = {name}_data}};
'''
    out_path = os.path.join(OUT_DIR, f"{name}.c")
    with open(out_path, "w") as f:
        f.write(c_src)
    print(f"wrote {out_path} ({w}x{h}, {'alpha' if has_alpha else 'opaque'}, {len(data)} bytes)")


# ---- Background: already opaque 410x494, straight RGB565 encode ----
bg = Image.open(os.path.join(SRC_DIR, "batman_bg_410x494.png")).convert("RGBA")
assert bg.size == (410, 494), bg.size
write_asset("face_batman_410_face_bg", bg, has_alpha=False)

# ---- Preview: 160x160 face-picker thumbnail. Per gen_red_magic_410.py's own
# note, the picker card (addWatchface() in ui.c) is a fixed 160x160 slot -
# every other _410 face keeps its preview at exactly that size. Square crop
# centered on the main dial (canvas center (205,247), see batman_410.c),
# vertical span 42..452 keeps it centered and inside the 0..494 canvas,
# then downscaled to 160x160. ----
preview_src = bg.crop((0, 42, 410, 452))
preview = preview_src.resize((160, 160), Image.LANCZOS).convert("RGB").convert("RGBA")
write_asset("face_batman_410_dial_img_preview", preview, has_alpha=False)

# ---- Hands: RGB565 + alpha, straight passthrough (already correctly sized/
# transparent PNGs, no scaling needed since bg is already native 410x494). ----
for fname, outname in [
    ("hand_hour.png", "face_batman_410_hand_hour"),
    ("hand_min.png", "face_batman_410_hand_minute"),
    ("hand_sec.png", "face_batman_410_hand_second"),
    ("hand_sub.png", "face_batman_410_hand_sub"),
]:
    img = Image.open(os.path.join(SRC_DIR, fname)).convert("RGBA")
    write_asset(outname, img, has_alpha=True)

# ---- Window patches: opaque, straight passthrough, no scaling (already
# native-resolution crops of the background, see v5.1 note above). ----
for fname, outname in [
    ("day_bg_patch.png", "face_batman_410_day_bg"),
    ("sec_bg_patch.png", "face_batman_410_sec_bg"),
    ("time_bg_patch.png", "face_batman_410_time_bg"),
    ("batt_bg_patch.png", "face_batman_410_batt_bg"),
    ("weather_bg_patch.png", "face_batman_410_weather_bg"),
    ("date_bg_patch.png", "face_batman_410_date_bg"),
]:
    img = Image.open(os.path.join(SRC_DIR, fname)).convert("RGBA")
    write_asset(outname, img, has_alpha=False)

print("done")
