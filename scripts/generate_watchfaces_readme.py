#!/usr/bin/env python3
"""Generate a README listing watchfaces from src/faces."""

from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
FACES_DIR = REPO_ROOT / "src" / "faces"
OUTPUT_FILE = FACES_DIR / "README.md"
PREVIEW_FILE = "watchface.png"


def find_watchfaces():
    faces = []

    for path in sorted(FACES_DIR.iterdir(), key=lambda item: item.name.lower()):
        if not path.is_dir():
            continue

        preview = path / PREVIEW_FILE
        if preview.exists():
            faces.append(path)

    return faces


def build_readme(faces):
    lines = [
        "# Watchfaces",
        "",
        "Generated from the folders in `src/faces` that include `watchface.png`.",
        "",
        f"Total watchfaces: {len(faces)}",
        "",
        "| Preview | Watchface |",
        "| --- | --- |",
    ]

    for face in faces:
        name = face.name
        lines.append(
            f"| [![{name}]({name}/{PREVIEW_FILE}?raw=true)]({name}/) | [{name}]({name}/) |"
        )

    lines.append("")
    return "\n".join(lines)


def main():
    if not FACES_DIR.exists():
        raise SystemExit(f"Faces directory not found: {FACES_DIR}")

    OUTPUT_FILE.write_text(build_readme(find_watchfaces()), encoding="utf-8")
    print(f"Wrote {OUTPUT_FILE.relative_to(REPO_ROOT)}")


if __name__ == "__main__":
    main()
