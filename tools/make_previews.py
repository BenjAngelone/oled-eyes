#!/usr/bin/env python3
"""Genera las imagenes del README (docs/) con el render real del firmware (tools/preview.exe).

Uso:  python tools/make_previews.py
Requiere: preview.exe compilado (tools/build_preview.bat) y Pillow.
"""
import subprocess
from pathlib import Path

from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent
OUT = HERE.parent / "docs"
EXE = HERE / "preview.exe"
TICK_MS = 33
W, H, S = 128, 64, 4          # panel y escala
PAD = 16
ON, OFF, BEZEL = (230, 240, 255), (8, 10, 14), (22, 24, 30)
STYLES = {"bloque": 0, "visor": 1, "escaner": 2, "lente": 4, "casco": 5, "mancuerna": 6}


def frames(style: int, scenario: str, ms: int) -> list[bytes]:
    raw = HERE / "_tmp.raw"
    subprocess.run([str(EXE), str(style), scenario, str(ms), str(raw)], check=True)
    data = raw.read_bytes()
    raw.unlink()
    return [data[i:i + W * H] for i in range(0, len(data), W * H)]


def panel(px: bytes, invert: bool) -> Image.Image:
    img = Image.new("RGB", (W * S + 2 * PAD, H * S + 2 * PAD), BEZEL)
    d = ImageDraw.Draw(img)
    for y in range(H):
        row = px[y * W:(y + 1) * W]
        for x in range(W):
            on = row[x] ^ invert
            x0, y0 = PAD + x * S, PAD + y * S
            d.rectangle([x0, y0, x0 + S - 2, y0 + S - 2], fill=ON if on else OFF)
    return img


def gif(name: str, style: str, scenario: str, ms: int, step: int = 2) -> None:
    fr = frames(STYLES[style], scenario, ms)[::step]
    inv = style == "mancuerna"
    imgs = [panel(f, inv).convert("P", palette=Image.ADAPTIVE, colors=4) for f in fr]
    imgs[0].save(OUT / name, save_all=True, append_images=imgs[1:], duration=TICK_MS * step, loop=0, optimize=True)
    print("escrito", OUT / name, len(imgs), "frames")


def grid(name: str, items: list[tuple[str, str, str, int]], cols: int) -> None:
    from PIL import ImageFont
    tiles = []
    for label, style, scenario, ms in items:
        f = frames(STYLES[style], scenario, ms)[-1]
        tiles.append((label, panel(f, style == "mancuerna")))
    tw, th = tiles[0][1].size
    rows = (len(tiles) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * (tw + 12) + 12, rows * (th + 40) + 12), (14, 15, 19))
    d = ImageDraw.Draw(sheet)
    try:
        font = ImageFont.truetype("arial.ttf", 18)
    except OSError:
        font = ImageFont.load_default()
    for i, (label, im) in enumerate(tiles):
        x, y = 12 + (i % cols) * (tw + 12), 12 + (i // cols) * (th + 40)
        sheet.paste(im, (x, y))
        d.text((x + 4, y + th + 8), label, fill=(170, 176, 190), font=font)
    sheet.save(OUT / name)
    print("escrito", OUT / name)


if __name__ == "__main__":
    OUT.mkdir(exist_ok=True)
    gif("tour.gif", "mancuerna", "tour", 16000, step=2)
    gif("bloque_tour.gif", "bloque", "tour", 16000, step=2)
    gif("trompo.gif", "bloque", "trompo", 3000)
    grid("moods.png", [(f"{m}", "bloque", m, 1200) for m in
                       ["tranquilo", "atento", "preocupado", "estresado", "agotado", "sin_wifi"]], 3)
    grid("styles.png", [(s, s, "atento", 1200) for s in STYLES], 3)
