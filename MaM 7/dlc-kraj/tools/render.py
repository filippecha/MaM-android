"""Renders an ODM terrain overview (tileset colour, height shading, models and decorations) into a PNG."""
import struct
from PIL import Image, ImageDraw

COLORS = {0: (60, 150, 60), 1: (230, 230, 240), 2: (220, 200, 120), 3: (90, 70, 60), 4: (140, 100, 60),
          5: (40, 90, 200), 6: (120, 100, 80), 7: (70, 110, 70), 8: (100, 90, 70), 9: (200, 190, 140),
          10: (150, 140, 130), 11: (150, 120, 80)}


def render(m, path, scale=6):
    tts = [struct.unpack_from('<HH', m['tileTypes'], i * 4)[0] for i in range(4)]
    img = Image.new('RGB', (128 * scale, 128 * scale))
    d = ImageDraw.Draw(img)
    for y in range(128):
        for x in range(128):
            t = m['tileMap'][y * 128 + x]
            ts = tts[(t - 90) // 36] if 90 <= t < 234 else 4
            c = COLORS.get(ts, (255, 0, 255))
            k = 0.45 + 0.55 * m['heightMap'][y * 128 + x] / 255
            d.rectangle([x * scale, y * scale, x * scale + scale - 1, y * scale + scale - 1], fill=tuple(int(v * k) for v in c))
    for md, _ in m['models']:
        mn = struct.unpack_from('<6i', md, 124)
        x0, x1 = ((mn[0] >> 9) + 64) * scale, ((mn[3] >> 9) + 64) * scale
        y0, y1 = (63 - (mn[4] >> 9)) * scale, (63 - (mn[1] >> 9)) * scale
        d.rectangle([x0, y0, x1 + scale, y1 + scale], outline=(255, 0, 0))
    for dec, name in m['decorations']:
        x, y = struct.unpack_from('<2i', dec, 4)
        gx, gy = ((x >> 9) + 64) * scale, (63 - (y >> 9)) * scale
        nm = name.split(b'\0')[0].lower()
        col = (255, 255, 0) if b'start' in nm else (0, 60, 0)
        d.point((gx + scale // 2, gy + scale // 2), fill=col)
        if b'start' in nm:
            d.ellipse([gx - 4, gy - 4, gx + 4, gy + 4], outline=col)
    for y in range(0, 128, 16):
        d.line([0, y * scale, 8, y * scale], fill=(255, 255, 255))
        d.text((10, y * scale - 5), str(y), fill=(255, 255, 255))
    for x in range(0, 128, 16):
        d.line([x * scale, 0, x * scale, 8], fill=(255, 255, 255))
        d.text((x * scale + 2, 10), str(x), fill=(255, 255, 255))
    img.save(path)
    return tts
