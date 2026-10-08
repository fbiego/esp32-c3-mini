import re, os, sys, math
sys.path.insert(0, os.path.dirname(__file__))
from lvgl_img import decode_to_rgba, encode_from_rgba, format_c_array
from PIL import Image

SRC_DIR = "/home/greg/chronos-watch/src/faces/75_2"
OUT_DIR = "/home/greg/chronos-watch/src/faces/75_2_410"
OLD = "75_2"
NEW = "75_2_410"
FACTOR = 410.0 / 240.0
Y_OFFSET = round((494 - round(240 * FACTOR)) / 2)  # vertical centering on the taller canvas

os.makedirs(os.path.join(OUT_DIR, "assets"), exist_ok=True)

def sc(v):
    return round(v * FACTOR)

# ---- Parse+scale every asset file (handles multi-frame files like the weekday group) ----
def parse_all_frames(path):
    src = open(path).read()
    frames = []
    for m in re.finditer(
        r'const LV_ATTRIBUTE_MEM_ALIGN uint8_t (\w+)_data_(\d+)\[\] = \{(.*?)\};\s*'
        r'.*?const lv_img_dsc_t (\w+) = \{\s*'
        r'(?:\.header\.(?:always_zero = 0|magic = \w+),\s*)?'
        r'\.header\.w = (\d+),\s*'
        r'\.header\.h = (\d+),\s*'
        r'\.data_size = sizeof\([^)]*\),\s*'
        r'\.header\.cf = (LV_COLOR_FORMAT_\w+),',
        src, re.S):
        base, idx, body, dscname, w, h, cf = m.groups()
        body = re.sub(r'//[^\n]*', '', body)
        vals = bytes(int(x.strip(), 16) for x in body.split(',') if x.strip())
        frames.append({
            "base": base, "idx": idx, "dscname": dscname,
            "w": int(w), "h": int(h), "cf": cf, "vals": vals,
            "has_alpha": "WITH_ALPHA" in cf,
        })
    return frames

def process_asset_file(fname):
    # NOTE: the face-picker's thumbnail card (addWatchface() in ui.c) is a fixed
    # 160x160 slot regardless of main canvas resolution - the preview asset must stay
    # unscaled or it overflows that slot (this bug shipped once already: 75_2_410's
    # preview went out at 273x273 and looked like the swipe only moving the image
    # within its own frame rather than the whole card; fixed by hand afterward,
    # excluding it here too so a future re-run of this script doesn't reintroduce it).
    scale = "preview" not in fname
    path = os.path.join(SRC_DIR, "assets", fname)
    frames = parse_all_frames(path)
    assert frames, f"no frames parsed from {fname}"
    out_lines = []
    header = open(path).read().split("const LV_ATTRIBUTE_MEM_ALIGN", 1)[0]
    header = header.replace(OLD, NEW)
    out_lines.append(header.rstrip())
    out_lines.append("")
    new_dims = None
    for fr in frames:
        img = decode_to_rgba(fr["w"], fr["h"], fr["vals"], fr["has_alpha"])
        new_w = max(1, sc(fr["w"])) if scale else fr["w"]
        new_h = max(1, sc(fr["h"])) if scale else fr["h"]
        new_dims = (new_w, new_h)
        # Source RGB is already alpha-premultiplied (confirmed by inspecting raw
        # partial-alpha pixels), so a direct resize is the mathematically correct
        # approach - no separate premultiply/unpremultiply step needed. LANCZOS
        # rings badly on these tiny (~10px) sprites though, producing colour
        # speckling once requantized to RGB565's 5/6/5 bits, so use BICUBIC
        # instead, which is smooth without the ringing.
        img2 = img.resize((new_w, new_h), Image.BICUBIC)
        data = encode_from_rgba(img2, fr["has_alpha"])
        new_base = fr["base"].replace(OLD, NEW)
        new_dscname = fr["dscname"].replace(OLD, NEW)
        arr_name = f"{new_base}_data_{fr['idx']}"
        out_lines.append(f"const LV_ATTRIBUTE_MEM_ALIGN uint8_t {arr_name}[] = {{")
        out_lines.append("\t//RGB565 data" if not fr["has_alpha"] else "\t//RGB565 data with alpha")
        out_lines.append(format_c_array(data))
        out_lines.append("};")
        out_lines.append("")
        out_lines.append(f"const lv_img_dsc_t {new_dscname} = {{")
        out_lines.append("    .header.magic = LV_IMAGE_HEADER_MAGIC,")
        out_lines.append(f"    .header.w = {new_w},")
        out_lines.append(f"    .header.h = {new_h},")
        out_lines.append(f"    .data_size = sizeof({arr_name}),")
        out_lines.append(f"    .header.cf = {fr['cf']},")
        out_lines.append(f"    .data = {arr_name}}};")
        out_lines.append("")
    new_fname = fname.replace(OLD, NEW)
    with open(os.path.join(OUT_DIR, "assets", new_fname), "w") as f:
        f.write("\n".join(out_lines))
    print(f"{fname} -> {new_fname}  ({frames[0]['w']}x{frames[0]['h']} -> {new_dims[0]}x{new_dims[1]}, {len(frames)} frame(s))")
    return new_dims

asset_files = [
    "face_75_2_dial_img_0_1024.c",
    "face_75_2_dial_img_2_216824.c",
    "face_75_2_dial_img_3_59132.c",
    "face_75_2_dial_img_19_89191.c",
    "face_75_2_dial_img_35_138999.c",
    "face_75_2_dial_img_preview.c",
]
dims = {}
for af in asset_files:
    dims[af] = process_asset_file(af)

print("Y_OFFSET:", Y_OFFSET)
print("dims:", dims)

# ---- Generate the .c / .h source, scaling every position/pivot ----
def scx(v):
    return round(v * FACTOR)

def scy(v):
    return round(v * FACTOR) + Y_OFFSET

def scp(v):
    return round(v * FACTOR)

c_src = f'''
// File generated by bin2lvgl, then scaled 240x240 -> 410x410 (centered on a 410x494 canvas)
// developed by fbiego.
// https://github.com/fbiego
// Watchface: 75_2_410 (derived from 75_2 "Analog")

#include "75_2_410.h"

#ifdef ENABLE_FACE_75_2_410

lv_obj_t *face_75_2_410;
lv_obj_t *face_75_2_410_0_1024;
lv_obj_t *face_75_2_410_2_216824;
lv_obj_t *face_75_2_410_3_59132;
lv_obj_t *face_75_2_410_19_89191;
lv_obj_t *face_75_2_410_35_138999;


#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH should be 16bit for watchfaces"
#endif

const lv_img_dsc_t *face_75_2_410_dial_img_2_216824_group[] = {{
	&face_75_2_410_dial_img_2_216824_0,
	&face_75_2_410_dial_img_2_216824_1,
	&face_75_2_410_dial_img_2_216824_2,
	&face_75_2_410_dial_img_2_216824_3,
	&face_75_2_410_dial_img_2_216824_4,
	&face_75_2_410_dial_img_2_216824_5,
	&face_75_2_410_dial_img_2_216824_6,
}};




#endif

void init_face_75_2_410(void (*callback)(const char*, const lv_img_dsc_t *, lv_obj_t **, lv_obj_t **)){{
#ifdef ENABLE_FACE_75_2_410
    face_75_2_410 = lv_obj_create(NULL);
    lv_obj_remove_flag(face_75_2_410, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(face_75_2_410, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(face_75_2_410, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(face_75_2_410, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(face_75_2_410, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(face_75_2_410, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(face_75_2_410, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(face_75_2_410, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

    lv_obj_add_event_cb(face_75_2_410, onFaceEvent, LV_EVENT_ALL, NULL);


    face_75_2_410_0_1024 = lv_image_create(face_75_2_410);
    lv_image_set_src(face_75_2_410_0_1024, &face_75_2_410_dial_img_0_1024_0);
    lv_obj_set_width(face_75_2_410_0_1024, LV_SIZE_CONTENT);
    lv_obj_set_height(face_75_2_410_0_1024, LV_SIZE_CONTENT);
    lv_obj_set_x(face_75_2_410_0_1024, {scx(0)});
    lv_obj_set_y(face_75_2_410_0_1024, {scy(0)});
    lv_obj_add_flag(face_75_2_410_0_1024, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_75_2_410_0_1024, LV_OBJ_FLAG_SCROLLABLE );

    face_75_2_410_2_216824 = lv_image_create(face_75_2_410);
    lv_image_set_src(face_75_2_410_2_216824, &face_75_2_410_dial_img_2_216824_0);
    lv_obj_set_width(face_75_2_410_2_216824, LV_SIZE_CONTENT);
    lv_obj_set_height(face_75_2_410_2_216824, LV_SIZE_CONTENT);
    lv_obj_set_x(face_75_2_410_2_216824, {scx(103)});
    lv_obj_set_y(face_75_2_410_2_216824, {scy(68)});
    lv_obj_add_flag(face_75_2_410_2_216824, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_75_2_410_2_216824, LV_OBJ_FLAG_SCROLLABLE );

    face_75_2_410_3_59132 = lv_image_create(face_75_2_410);
    lv_image_set_src(face_75_2_410_3_59132, &face_75_2_410_dial_img_3_59132_0);
    lv_obj_set_width(face_75_2_410_3_59132, LV_SIZE_CONTENT);
    lv_obj_set_height(face_75_2_410_3_59132, LV_SIZE_CONTENT);
    lv_obj_set_x(face_75_2_410_3_59132, {scx(115)});
    lv_obj_set_y(face_75_2_410_3_59132, {scy(59)});
    lv_obj_add_flag(face_75_2_410_3_59132, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_75_2_410_3_59132, LV_OBJ_FLAG_SCROLLABLE );
	lv_image_set_pivot(face_75_2_410_3_59132, {scp(5)}, {scp(61)});

    face_75_2_410_19_89191 = lv_image_create(face_75_2_410);
    lv_image_set_src(face_75_2_410_19_89191, &face_75_2_410_dial_img_19_89191_0);
    lv_obj_set_width(face_75_2_410_19_89191, LV_SIZE_CONTENT);
    lv_obj_set_height(face_75_2_410_19_89191, LV_SIZE_CONTENT);
    lv_obj_set_x(face_75_2_410_19_89191, {scx(115)});
    lv_obj_set_y(face_75_2_410_19_89191, {scy(36)});
    lv_obj_add_flag(face_75_2_410_19_89191, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_75_2_410_19_89191, LV_OBJ_FLAG_SCROLLABLE );
	lv_image_set_pivot(face_75_2_410_19_89191, {scp(5)}, {scp(84)});

    face_75_2_410_35_138999 = lv_image_create(face_75_2_410);
    lv_image_set_src(face_75_2_410_35_138999, &face_75_2_410_dial_img_35_138999_0);
    lv_obj_set_width(face_75_2_410_35_138999, LV_SIZE_CONTENT);
    lv_obj_set_height(face_75_2_410_35_138999, LV_SIZE_CONTENT);
    lv_obj_set_x(face_75_2_410_35_138999, {scx(115)});
    lv_obj_set_y(face_75_2_410_35_138999, {scy(12)});
    lv_obj_add_flag(face_75_2_410_35_138999, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag(face_75_2_410_35_138999, LV_OBJ_FLAG_SCROLLABLE );
	lv_image_set_pivot(face_75_2_410_35_138999, {scp(5)}, {scp(108)});


    callback("Analog", &face_75_2_410_dial_img_preview_0, &face_75_2_410, &face_75_2_410_35_138999);

#endif
}}

void update_time_75_2_410(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday)
{{
#ifdef ENABLE_FACE_75_2_410
    if (!face_75_2_410)
    {{
        return;
    }}
	lv_image_set_src(face_75_2_410_2_216824, face_75_2_410_dial_img_2_216824_group[((weekday + 6) / 1) % 7]);
	lv_image_set_rotation(face_75_2_410_3_59132, hour * 300 + (minute * 5) + (second * (5 / 60)));
	lv_image_set_rotation(face_75_2_410_19_89191, (minute * 60) + second);
	// lv_image_set_rotation(face_75_2_410_35_138999, second * 60);

#endif
}}

void update_weather_75_2_410(int temp, int icon)
{{
#ifdef ENABLE_FACE_75_2_410
    if (!face_75_2_410)
    {{
        return;
    }}

#endif
}}

void update_status_75_2_410(int battery, bool connection){{
#ifdef ENABLE_FACE_75_2_410
    if (!face_75_2_410)
    {{
        return;
    }}

#endif
}}

void update_activity_75_2_410(int steps, int distance, int kcal)
{{
#ifdef ENABLE_FACE_75_2_410
    if (!face_75_2_410)
    {{
        return;
    }}

#endif
}}

void update_health_75_2_410(int bpm, int oxygen)
{{
#ifdef ENABLE_FACE_75_2_410
    if (!face_75_2_410)
    {{
        return;
    }}

#endif
}}

void update_all_75_2_410(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday, 
    int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{{
#ifdef ENABLE_FACE_75_2_410
    update_time_75_2_410(second, minute, hour, mode, am, day, month, year, weekday);
    update_weather_75_2_410(temp, icon);
    update_status_75_2_410(battery, connection);
    update_activity_75_2_410(steps, distance, kcal);
    update_health_75_2_410(bpm, oxygen);
#endif
}}

void update_check_75_2_410(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday, 
    int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{{
#ifdef ENABLE_FACE_75_2_410
    if (root != face_75_2_410)
    {{
        return;
    }}
    update_time_75_2_410(second, minute, hour, mode, am, day, month, year, weekday);
    update_weather_75_2_410(temp, icon);
    update_status_75_2_410(battery, connection);
    update_activity_75_2_410(steps, distance, kcal);
    update_health_75_2_410(bpm, oxygen);
#endif
}}
'''

with open(os.path.join(OUT_DIR, "75_2_410.c"), "w") as f:
    f.write(c_src)

h_src = '''
// File generated by bin2lvgl, then scaled 240x240 -> 410x410 (centered on a 410x494 canvas)
// developed by fbiego.
// https://github.com/fbiego
// Watchface: 75_2_410 (derived from 75_2 "Analog")

#ifndef _FACE_75_2_410_H
#define _FACE_75_2_410_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#include "app_hal.h"

//#define ENABLE_FACE_75_2_410 // (Analog, 410-wide) uncomment to enable or define it elsewhere

#ifdef ENABLE_FACE_75_2_410
    extern lv_obj_t *face_75_2_410;
	extern lv_obj_t *face_75_2_410_0_1024;
	extern lv_obj_t *face_75_2_410_2_216824;
	extern lv_obj_t *face_75_2_410_3_59132;
	extern lv_obj_t *face_75_2_410_19_89191;
	extern lv_obj_t *face_75_2_410_35_138999;


	LV_IMG_DECLARE(face_75_2_410_dial_img_0_1024_0);
	LV_IMG_DECLARE(face_75_2_410_dial_img_2_216824_0);
	LV_IMG_DECLARE(face_75_2_410_dial_img_2_216824_1);
	LV_IMG_DECLARE(face_75_2_410_dial_img_2_216824_2);
	LV_IMG_DECLARE(face_75_2_410_dial_img_2_216824_3);
	LV_IMG_DECLARE(face_75_2_410_dial_img_2_216824_4);
	LV_IMG_DECLARE(face_75_2_410_dial_img_2_216824_5);
	LV_IMG_DECLARE(face_75_2_410_dial_img_2_216824_6);
	LV_IMG_DECLARE(face_75_2_410_dial_img_3_59132_0);
	LV_IMG_DECLARE(face_75_2_410_dial_img_19_89191_0);
	LV_IMG_DECLARE(face_75_2_410_dial_img_35_138999_0);
	LV_IMG_DECLARE(face_75_2_410_dial_img_preview_0);

#endif
    void onFaceEvent(lv_event_t * e);

    void init_face_75_2_410(void (*callback)(const char*, const lv_img_dsc_t *, lv_obj_t **, lv_obj_t **));
    void update_time_75_2_410(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday);
    void update_weather_75_2_410(int temp, int icon);
    void update_status_75_2_410(int battery, bool connection);
    void update_activity_75_2_410(int steps, int distance, int kcal);
    void update_health_75_2_410(int bpm, int oxygen);
    void update_all_75_2_410(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday, 
                int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen);
    void update_check_75_2_410(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday, 
                int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen);


#ifdef __cplusplus
}
#endif

#endif
'''
with open(os.path.join(OUT_DIR, "75_2_410.h"), "w") as f:
    f.write(h_src)

print("wrote 75_2_410.c and 75_2_410.h")
