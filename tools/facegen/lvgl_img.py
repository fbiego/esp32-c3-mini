import re
from PIL import Image

def parse_asset(path):
    src = open(path).read()
    w = int(re.search(r'header\.w\s*=\s*(\d+)', src).group(1))
    h = int(re.search(r'header\.h\s*=\s*(\d+)', src).group(1))
    has_alpha = 'NATIVE_WITH_ALPHA' in src
    m = re.search(r'_data_0\[\]\s*=\s*\{(.*?)\};', src, re.S)
    body = re.sub(r'//[^\n]*', '', m.group(1))
    vals = bytes(int(x.strip(), 16) for x in body.split(',') if x.strip())
    return w, h, vals, has_alpha

def decode_to_rgba(w, h, vals, has_alpha):
    # Planar layout: all w*h RGB565 color bytes (2 bytes/pixel) first,
    # followed by all w*h alpha bytes (1 byte/pixel) - NOT interleaved.
    n = w * h
    color_bytes = vals[:n*2]
    alpha_bytes = vals[n*2:n*2+n] if has_alpha else None
    img = Image.new("RGBA", (w, h))
    px = img.load()
    for i in range(n):
        color16 = color_bytes[i*2] | (color_bytes[i*2+1] << 8)
        a = alpha_bytes[i] if has_alpha else 255
        r5 = (color16 >> 11) & 0x1F
        g6 = (color16 >> 5) & 0x3F
        b5 = color16 & 0x1F
        r = (r5 * 255 + 15) // 31
        g = (g6 * 255 + 31) // 63
        b = (b5 * 255 + 15) // 31
        x, y = i % w, i // w
        px[x, y] = (r, g, b, a)
    return img

def encode_from_rgba(img, has_alpha):
    w, h = img.size
    px = img.load()
    color_out = bytearray()
    alpha_out = bytearray()
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            r5 = (r * 31 + 127) // 255
            g6 = (g * 63 + 127) // 255
            b5 = (b * 31 + 127) // 255
            color16 = (r5 << 11) | (g6 << 5) | b5
            color_out.append(color16 & 0xFF)
            color_out.append((color16 >> 8) & 0xFF)
            if has_alpha:
                alpha_out.append(a)
    return bytes(color_out) + bytes(alpha_out)

def format_c_array(data_bytes, per_line=20):
    lines = []
    for i in range(0, len(data_bytes), per_line):
        chunk = data_bytes[i:i+per_line]
        lines.append("\t" + ",".join(f"0x{b:02X}" for b in chunk) + ",")
    return "\n".join(lines)
