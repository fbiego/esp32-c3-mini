import os, sys
from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
from lvgl_img import encode_from_rgba, format_c_array
from gen_cogs_classic_410 import SRC_PNG_DIR, GEARS

OUT_PATH = "/home/greg/chronos-watch/src/faces/classic_410/assets/cogs_img.c"

# Bakes whatever RGBA PNGs currently sit in cogs_src/gear_{a,b,c}.png (the masked
# hi-res chrome art, or a hand-edited replacement) into the LVGL C image array
# format this codebase's other faces use - same encoding lvgl_img.py already
# round-trips (RGB565 colour plane + planar A8 alpha, LV_COLOR_FORMAT_NATIVE_WITH_ALPHA
# at LV_COLOR_DEPTH=16). Deliberately separate from the generator/masking scripts:
# rerun this alone any time cogs_src/*.png changes, without regenerating them.

HEADER = '''// Cogs (12 o'clock mechanical window) gear artwork for classic_410.
// Hi-res chrome renders (cogs_src/gear_{a,b,c}_hi.jpg) masked to the
// procedurally-correct meshing silhouette (tools/facegen/mask_cogs_hi.py),
// downscaled, and baked to LVGL image data here (tools/facegen/bake_cogs_to_c.py).
// Watchface: classic_410

#include "../classic_410.h"

#ifdef ENABLE_FACE_CLASSIC_410

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

'''

FOOTER = "\n#endif\n"


def bake_one(name):
    path = os.path.join(SRC_PNG_DIR, f"gear_{name}.png")
    img = Image.open(path).convert("RGBA")
    w, h = img.size
    data = encode_from_rgba(img, has_alpha=True)
    arr_name = f"classic_410_gear_{name}_data"
    dsc_name = f"classic_410_gear_{name}_img"
    lines = []
    lines.append(f"const LV_ATTRIBUTE_MEM_ALIGN uint8_t {arr_name}[] = {{")
    lines.append("\t//RGB565 data with alpha")
    lines.append(format_c_array(data))
    lines.append("};")
    lines.append("")
    lines.append(f"const lv_img_dsc_t {dsc_name} = {{")
    lines.append("    .header.magic = LV_IMAGE_HEADER_MAGIC,")
    lines.append(f"    .header.w = {w},")
    lines.append(f"    .header.h = {h},")
    lines.append(f"    .data_size = sizeof({arr_name}),")
    lines.append("    .header.cf = LV_COLOR_FORMAT_NATIVE_WITH_ALPHA,")
    lines.append(f"    .data = {arr_name}}};")
    lines.append("")
    return "\n".join(lines), (w, h)


def main():
    out = [HEADER]
    dims = {}
    for (name, _r, _n, _color) in GEARS:
        src, wh = bake_one(name)
        out.append(src)
        dims[name] = wh
    out.append(FOOTER)
    with open(OUT_PATH, "w") as f:
        f.write("\n".join(out))
    print(f"wrote {OUT_PATH}")
    for name, wh in dims.items():
        print(f"  gear_{name}: {wh[0]}x{wh[1]}")


if __name__ == "__main__":
    main()
