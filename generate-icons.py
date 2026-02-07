#!/usr/bin/env python3
import glob
import json
import base64
from pathlib import Path
from PIL import Image
from io import BytesIO, TextIOWrapper

WIDGET_ICON_SIZE = (64, 64)
UI_ICON_SIZE = (256, 256)
HEADER = """// AUTO-GENERATED FILE — DO NOT EDIT
#pragma once
#include <pgmspace.h>
#include <cstdint>
#include <cstddef>

"""


def bmp_to_c_array(name: str, bmp_bytes: bytes) -> str:
    hex_bytes = ", ".join(f"0x{b:02X}" for b in bmp_bytes)
    return (
        f"const uint8_t {name}[] PROGMEM = {{ {hex_bytes} }};\n"
        f"const size_t {name}_len = {len(bmp_bytes)};\n"
    )


def process_image(path: Path, size: tuple[int, int]) -> tuple[bytes, str]:
    """Process image and return (bmp_bytes, base64_png_preview)"""
    im = Image.open(path).convert("RGBA")

    # Resize
    white_bg = Image.new("RGBA", im.size, (255, 255, 255, 255))
    im = Image.alpha_composite(white_bg, im)
    im = im.resize(size, Image.Resampling.LANCZOS)

    # Convert to 1-bit monochrome
    bw = im.convert("L").point(lambda x: 0 if x < 128 else 255, "1")

    # Save to BMP in memory
    buf = BytesIO()
    bw.save(buf, format="BMP")
    bmp_bytes = buf.getvalue()

    # Also create a PNG preview for the configurator
    preview_buf = BytesIO()
    bw.convert("RGB").save(preview_buf, format="PNG")
    preview_b64 = base64.b64encode(preview_buf.getvalue()).decode("ascii")

    return bmp_bytes, f"data:image/png;base64,{preview_b64}"


def handle_file(file_path: str, out: TextIOWrapper, size: tuple[int, int], manifest: list) -> None:
    path = Path(file_path)
    name = path.stem.replace("-", "_").replace(" ", "_")
    bmp_data, preview_data_url = process_image(path, size)

    print(f"Processing {path} → {name}")

    out.write(bmp_to_c_array(name, bmp_data))
    out.write("\n")

    # Add to manifest for configurator
    manifest.append({
        "name": name,
        "preview": preview_data_url,
        "size": size[0]
    })


def main() -> None:
    widget_icons = []
    ui_icons = []

    with open("src/assets/icons.h", "w") as out:
        out.write(HEADER)
        for file_path in sorted(glob.glob("icons-buttons/*.png")):
            handle_file(file_path, out, WIDGET_ICON_SIZE, widget_icons)
        for file_path in sorted(glob.glob("icons-ui/*.png")):
            handle_file(file_path, out, UI_ICON_SIZE, ui_icons)

    # Write manifest for configurator
    manifest = {
        "widgetIcons": widget_icons,
        "uiIcons": ui_icons
    }
    with open("tools/icons-manifest.json", "w") as f:
        json.dump(manifest, f, indent=2)

    print(f"Generated src/assets/icons.h with {len(widget_icons)} widget icons and {len(ui_icons)} UI icons")
    print(f"Generated tools/icons-manifest.json for configurator")
    print("Done")


if __name__ == "__main__":
    main()
