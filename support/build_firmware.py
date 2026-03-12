#    MIT License
#
#   Copyright (c) 2025 Felix Biego
#
#   Permission is hereby granted, free of charge, to any person obtaining a copy
#   of this software and associated documentation files (the "Software"), to deal
#   in the Software without restriction, including without limitation the rights
#   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#   copies of the Software, and to permit persons to whom the Software is
#   furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included in all
#   copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#   SOFTWARE.
#
#   ______________  _____
#   ___  __/___  /_ ___(_)_____ _______ _______
#   __  /_  __  __ \__  / _  _ \__  __ `/_  __ \ 
#   _  __/  _  /_/ /_  /  /  __/_  /_/ / / /_/ /
#   /_/     /_.___/ /_/   \___/ _\__, /  \____/
# 							  /____/
#

import re
import os, shutil
from datetime import datetime
from pathlib import Path
import json

Import("env")

sep = os.sep

FILE_PATH = Path(f"src{sep}ui{sep}ui.c")
PIO_INI_PATH = Path("platformio.ini")
FIRMWARE_JSON_PATH = Path(f"firmware{sep}firmware.json")

print("Extra Script")

def get_env_name(env_name):
    with open(PIO_INI_PATH) as f:
        content = f.read()

    pattern = rf"default_envs\s*=\s*{re.escape(env_name)}\s*;\s*(.+)"
    match = re.search(pattern, content)

    if match:
        description = match.group(1).strip()
        return description
    else:
        return env_name

def merge_bins(pairs, out_path, new_pairs, chip, env):
    """
    Merge ESP32 .bin segments into one file starting from the lowest offset.

    pairs: list of (offset, path) tuples (e.g. (0x1000, "bootloader.bin"))
    out_path: output file path
    """

    # Normalize and load
    segs = []
    for off, path in pairs:
        off = int(off, 0) if isinstance(off, str) else off
        with open(path, "rb") as f:
            data = f.read()
        segs.append((off, data, path))

    # Find address bounds
    min_off = min(o for o, _, _ in segs)
    max_end = max(o + len(d) for o, d, _ in segs)
    total_size = max_end - min_off

    out_path = out_path.replace('.bin', f"_0x{min_off:X}.bin")

    print(f"Merging {len(segs)} segments into {out_path}")
    print(f"Start offset: 0x{min_off:X}, total size: {total_size} bytes")

    buf = bytearray([0xFF]) * total_size

    for off, data, path in segs:
        rel_off = off - min_off
        # print(f"  {path}: offset=0x{off:X}, rel=0x{rel_off:X}, size={len(data)}")
        buf[rel_off:rel_off + len(data)] = data

    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "wb") as f:
        f.write(buf)

    data = {}
    data[env] = {}
    data[env]["id"] = env
    data[env]["name"] = get_env_name(env)
    data[env]["file"] = f"{out_path.split(sep)[-1]}"
    data[env]["address"] = f"0x{min_off:X}"
    data[env]["size"] = total_size
    data[env]["chip"] = chip


    existing_data = {}
    if os.path.isfile(FIRMWARE_JSON_PATH):
        with open(FIRMWARE_JSON_PATH, 'r', encoding='utf-8') as f:
            existing_data = json.load(f)

    existing_data.update(data)

    with open(FIRMWARE_JSON_PATH, 'w', encoding='utf-8') as f:
        json.dump(existing_data, f, ensure_ascii=False, indent=4)

    print(f"✅ Wrote merged file: {out_path} ({len(buf)} bytes)")

def read_version(content):
    version = re.search(r'#define\s+UI_VERSION\s+"([^"]+)"', content)
    if not all([version]):
        raise ValueError("Missing version fields in file")
    return {
        "version": version.group(1)
    }
    
def version_utils(file_path):
    content = file_path.read_text(encoding="utf-8")
    version = read_version(content)
    return version


def after_build(source, target, env):
    env_name = str(source[0]).split(sep)[-2]

    dest_dir = f"firmware"
    os.makedirs(dest_dir, exist_ok=True)

    skip_files = []
    skip_files.append("firmware.json") # always skip firmware.json
    
    if os.path.isfile(FIRMWARE_JSON_PATH):
        with open(FIRMWARE_JSON_PATH, 'r', encoding='utf-8') as f:
            data = json.load(f)
            for env_key, env_data in data.items():
                if "file" in env_data:
                    skip_files.append(env_data["file"])

    # Remove all files and subfolders inside dest_dir
    for filename in os.listdir(dest_dir):
        if filename in skip_files:
            print(f"Skipping {filename} as it's in the skip list")
            continue
        file_path = os.path.join(dest_dir, filename)
        try:
            if os.path.isfile(file_path) or os.path.islink(file_path):
                os.remove(file_path)  # remove file or symlink
            elif os.path.isdir(file_path):
                shutil.rmtree(file_path)  # remove directory
        except Exception as e:
            print(f"Failed to delete {file_path}: {e}")

    
    info = version_utils(FILE_PATH)
    vers = f"v{info['version']}"

    merged_path = f"{dest_dir}{sep}{env_name}_{vers}.bin"

    # Get full upload command (PlatformIO’s real flash command)
    upload_cmd = env.subst("$UPLOADCMD") + f" {str(source[0])}"

    # print(f"Upload cmd: {upload_cmd}")

    # Parse the chip type
    chip_match = re.search(r"--chip\s+(\S+)", upload_cmd)
    chip = chip_match.group(1) if chip_match else "esp32"

    # Parse offset + .bin pairs
    pairs = re.findall(r"(0x[0-9a-fA-F]+)\s+(\S+\.bin)", upload_cmd)
    if not pairs:
        print("❌ Could not detect any binary segments from upload command!")
        print(upload_cmd)
        return

    merge_bins(pairs, merged_path, pairs, chip, env_name) # use custom merge function
    

# Run after main program build
env.AddPostAction("upload", after_build)
env.AddPostAction("buildprog", after_build)
