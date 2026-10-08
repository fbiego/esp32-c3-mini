import os, sys
from PIL import Image, ImageDraw
import numpy as np

sys.path.insert(0, os.path.dirname(__file__))
from gen_cogs_classic_410 import render_gear, GEARS, SRC_PNG_DIR

# Takes the user's hand-authored hi-res chrome renders (gear_{a,b,c}_hi.jpg) and
# clips them to the *procedural* silhouette from gen_cogs_classic_410.py - the
# hi-res art has its own outer bevel/highlight that puffs outside the flat
# tooth silhouette (real gear teeth can't mesh if their edges are rounded
# blobs), and its own window/hole shapes that don't match the spoke design.
# Only the metal texture/colour is kept from the hi-res art; the shape itself
# (teeth count, module, taper, spoke windows, spool) stays exactly the
# geometrically-correct one already established.

MASTER_DIR = os.path.join(SRC_PNG_DIR, "hi_masked")


def detect_bounds(im_rgb):
    """Corner-seeded flood fill to find the gear's own centre/outer radius in
    the hi-res art, so our mask can be scaled/positioned to match it - the two
    images were authored independently, not pixel-registered."""
    w, h = im_rgb.size
    seed = (0, 255, 0)
    filled = im_rgb.copy()
    for corner in [(0, 0), (w - 1, 0), (0, h - 1), (w - 1, h - 1)]:
        ImageDraw.floodfill(filled, corner, seed, thresh=20)
    arr = np.array(filled)
    bg = (arr[:, :, 0] == 0) & (arr[:, :, 1] == 255) & (arr[:, :, 2] == 0)
    ys, xs = np.nonzero(~bg)
    cx, cy = xs.mean(), ys.mean()
    outer_r = float(np.hypot(xs - cx, ys - cy).max())
    return cx, cy, outer_r


def build(name, pitch_r, teeth, embed_size):
    hi_path = os.path.join(SRC_PNG_DIR, f"gear_{name}_hi.jpg")
    hi = Image.open(hi_path).convert("RGB")
    cx, cy, outer_r = detect_bounds(hi)

    # Scale our silhouette so its own tip radius matches the hi-res art's
    # detected outer radius exactly, then render it directly at that pixel
    # density (not upscaled from the small embed-resolution PNG - a fresh
    # render keeps the anti-aliased edges crisp).
    base_tip_r = pitch_r + 4.0  # MODULE=4 in gen_cogs_classic_410
    scale = outer_r / base_tip_r
    mask_img = render_gear(pitch_r, teeth, (255, 255, 255), scale=scale)
    alpha = mask_img.split()[-1]

    px = cx - mask_img.width / 2
    py = cy - mask_img.height / 2
    shifted_alpha = Image.new("L", hi.size, 0)
    shifted_alpha.paste(alpha, (round(px), round(py)))
    canvas = hi.convert("RGBA")
    canvas.putalpha(shifted_alpha)

    bbox = canvas.getbbox()
    cropped = canvas.crop(bbox)

    os.makedirs(MASTER_DIR, exist_ok=True)
    master_path = os.path.join(MASTER_DIR, f"gear_{name}_hi_masked.png")
    cropped.save(master_path)

    embedded = cropped.resize((embed_size, embed_size), Image.LANCZOS)
    embed_path = os.path.join(SRC_PNG_DIR, f"gear_{name}.png")
    embedded.save(embed_path)
    print(f"gear_{name}: hi-res {hi.size} -> mask scale {scale:.3f} -> master {cropped.size} -> embed {embedded.size}")
    return embedded


def main():
    for (name, pitch_r, teeth, _color) in GEARS:
        base_tip_r = pitch_r + 4.0
        base_pad = 3.0
        embed_size = int((base_tip_r + base_pad) * 2 + 0.999)
        build(name, pitch_r, teeth, embed_size)


if __name__ == "__main__":
    main()
