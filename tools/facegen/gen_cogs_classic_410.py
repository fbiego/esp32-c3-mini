import math, os, sys
from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(__file__))
from lvgl_img import encode_from_rgba, format_c_array

OUT_DIR = "/home/greg/chronos-watch/src/faces/classic_410/assets"
# Per-gear source PNGs live here, separate from the generated LVGL C assets -
# this is the file to hand-edit for future customisation. Re-running this
# script overwrites them (it's a *generator*), so once you start hand-editing
# a given gear's PNG, regenerate the others individually or copy your edit
# back in afterwards.
SRC_PNG_DIR = "/home/greg/chronos-watch/src/faces/classic_410/assets/cogs_src"
PREVIEW_PATH = "/tmp/claude-1000/-home-greg/84c32fc4-24a9-42f8-a6d4-70d87c43a124/scratchpad/cogs_preview.png"

SS = 4  # supersample factor for anti-aliasing

# Common module across the whole gear train - this is what makes the teeth pitch
# (spacing) identical on all three gears so they can actually mesh, same idea as
# real watch-movement gear trains. r_i (treated here as each gear's PITCH radius,
# not its outer tip radius) and teeth_i must satisfy 2*r_i/N_i == MODULE for every
# gear. The existing radii (34, 28, 22) were picked before any of this was a
# consideration, but they happen to divide out to exact integer teeth counts at
# MODULE=4 (34/2=17, 28/2=14, 22/2=11) - no rounding/fudging needed.
MODULE = 4.0

GEARS = [
    # name,  pitch_r, teeth, color
    ("a", 34, 17, (160, 160, 160)),
    ("b", 28, 14, (160, 160, 160)),
    ("c", 22, 11, (160, 160, 160)),
]

TIP_TAPER = 0.55           # tooth tip thickness as a fraction of its root thickness - the actual "one simple trapezoid" taper


def polar(cx, cy, r, ang_rad):
    return (cx + r * math.sin(ang_rad), cy - r * math.cos(ang_rad))


def render_gear(pitch_r, teeth, color, ss=SS, scale=1.0):
    # `scale` zooms the whole drawing uniformly (module, pad, spool included) - used
    # to re-render this exact silhouette at whatever pixel density another image
    # (e.g. hand-authored hi-res art) needs to align with it, rather than upscaling
    # an already-downsampled mask.
    module = MODULE * scale
    pitch_r = pitch_r * scale
    root_r = pitch_r - module * 1.25
    tip_r = pitch_r + module
    pad = 3 * scale
    canvas_r = tip_r + pad
    size = int(math.ceil(canvas_r * 2)) * ss
    cx = cy = size / 2.0

    spool_r_base = 2 * scale  # matches flashed `int32_t spool_r = 22 / 8` (integer truncation, not 2.75)

    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    def bbox(r):
        return [cx - r * ss, cy - r * ss, cx + r * ss, cy + r * ss]

    # Body: solid disc out to the root circle - teeth (below) extend past this.
    d.ellipse(bbox(root_r), fill=(*color, 255))

    # Teeth: single trapezoid polygon per tooth (wide at the root, narrower at the
    # tip) - real gear-tooth taper, not the old two-rectangle stack.
    circular_pitch = math.pi * module
    root_thickness = circular_pitch * 0.62      # thickness at the root circle
    tip_thickness = root_thickness * TIP_TAPER  # thickness at the tip circle
    half_base_ang = (root_thickness / 2) / root_r
    half_tip_ang = (tip_thickness / 2) / tip_r
    for i in range(teeth):
        a = i * 2 * math.pi / teeth
        pts = [
            polar(cx, cy, root_r * ss, a - half_base_ang),
            polar(cx, cy, tip_r * ss, a - half_tip_ang),
            polar(cx, cy, tip_r * ss, a + half_tip_ang),
            polar(cx, cy, root_r * ss, a + half_base_ang),
        ]
        d.polygon(pts, fill=(*color, 255))

    # Decorative "windows" in the body ring, shaped like the outer mech-window
    # itself (an annulus sector: two concentric arcs top/bottom, straight radial
    # edges left/right) instead of plain circular holes - same silhouette as the
    # keystone frame, just small and repeated as spokes. Evenly spaced, independent
    # of tooth count/phase (purely decorative, like a real bridge plate) - punched
    # as fully transparent so the black dial shows through.
    n_windows = 5
    win_inner_r = max(spool_r_base + 3, root_r * 0.28)
    win_outer_r = root_r * 0.82
    win_fill_frac = 0.55  # fraction of each window's angular slot actually cut out (rest is the spoke)
    win_half_ang = (2 * math.pi / n_windows) * win_fill_frac / 2
    arc_samples = 10
    for i in range(n_windows):
        a = i * 2 * math.pi / n_windows
        pts = []
        for t in [a - win_half_ang + (2 * win_half_ang) * k / (arc_samples - 1) for k in range(arc_samples)]:
            pts.append(polar(cx, cy, win_outer_r * ss, t))
        for t in [a + win_half_ang - (2 * win_half_ang) * k / (arc_samples - 1) for k in range(arc_samples)]:
            pts.append(polar(cx, cy, win_inner_r * ss, t))
        d.polygon(pts, fill=(0, 0, 0, 0))

    # Spool: a true hole (alpha 0) at the true centre (the axle), not the flashed
    # widget version's opaque-black fill - that only read as a "hole" because it
    # happened to sit on a solid black dial; real transparency works regardless
    # of what's underneath (including the hi-res art this mask gets composited
    # over) and looks identical on the black dial either way. Fixed size across
    # all three gears (`spool_r = 22 / 8` in integer C arithmetic truncates to 2).
    spool_r = spool_r_base
    d.ellipse([cx - spool_r * ss, cy - spool_r * ss, cx + spool_r * ss, cy + spool_r * ss],
              fill=(0, 0, 0, 0))

    final_size = int(math.ceil(canvas_r * 2))
    img = img.resize((final_size, final_size), Image.LANCZOS)
    return img


def main():
    imgs = [render_gear(r, n, color) for (_, r, n, color) in GEARS]

    os.makedirs(SRC_PNG_DIR, exist_ok=True)
    for (name, r, n, color), im in zip(GEARS, imgs):
        im.save(os.path.join(SRC_PNG_DIR, f"gear_{name}.png"))

    # Side-by-side preview composite, generous padding, on a black background
    # matching the dial so it reads the way it will on-device.
    pad = 20
    total_w = sum(im.width for im in imgs) + pad * (len(imgs) + 1)
    total_h = max(im.height for im in imgs) + pad * 2
    composite = Image.new("RGBA", (total_w, total_h), (0, 0, 0, 255))
    x = pad
    for im, (name, r, n, color) in zip(imgs, GEARS):
        y = pad + (total_h - pad * 2 - im.height) // 2
        composite.paste(im, (x, y), im)
        x += im.width + pad
    os.makedirs(os.path.dirname(PREVIEW_PATH), exist_ok=True)
    composite.convert("RGB").save(PREVIEW_PATH)
    print(f"wrote preview: {PREVIEW_PATH}")
    for (name, r, n, color), im in zip(GEARS, imgs):
        print(f"  gear {name}: pitch_r={r} teeth={n} module={2*r/n} image={im.size}")


if __name__ == "__main__":
    main()
