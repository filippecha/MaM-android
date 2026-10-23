"""Pictures of the MaM 7 expansion "Stín nad Krajem", painted by code: the textures of the hobbit hole entrances
(bitmaps) and the interiors shown in Hobbiton's houses instead of the house movies (icons, 460x344).

    python art.py <output folder>      saves all pictures as PNG for a look
"""
import math
import random
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFilter

INTERIOR = (460, 344)


def noise(w, h, scale, seed, octaves=3):
    """Smooth tileable value noise in [0, 1]."""
    r = np.random.default_rng(seed)
    out = np.zeros((h, w))
    amp, total = 1.0, 0.0
    for o in range(octaves):
        cells = max(1, int(scale * 2 ** o))
        g = r.random((cells, cells))
        ys = np.arange(h) * cells / h
        xs = np.arange(w) * cells / w
        y0, x0 = np.floor(ys).astype(int), np.floor(xs).astype(int)
        fy, fx = ys - y0, xs - x0
        fy, fx = fy * fy * (3 - 2 * fy), fx * fx * (3 - 2 * fx)
        y1, x1 = (y0 + 1) % cells, (x0 + 1) % cells
        y0, x0 = y0 % cells, x0 % cells
        a = g[np.ix_(y0, x0)] * (1 - fx) + g[np.ix_(y0, x1)] * fx
        b = g[np.ix_(y1, x0)] * (1 - fx) + g[np.ix_(y1, x1)] * fx
        out += amp * (a * (1 - fy[:, None]) + b * fy[:, None])
        total += amp
        amp *= 0.5
    return out / total


def tint(n, dark, light):
    """Colors between dark and light by a noise array."""
    d, l = np.array(dark, float), np.array(light, float)
    return Image.fromarray((d + (l - d) * n[..., None]).clip(0, 255).astype(np.uint8), 'RGB')


def shade(img, factor):
    return Image.fromarray((np.asarray(img, float) * factor).clip(0, 255).astype(np.uint8), 'RGB')


# ---------------------------------------------------------------------------------------------------------------------
# Textures of the entrances.

def grass(size=128):
    n = noise(size, size, 8, 11) * 0.6 + noise(size, size, 32, 12, 1) * 0.4
    return tint(n, (38, 78, 22), (96, 140, 50))


def facade(size=128):
    """Packed earth with field stones, tileable."""
    img = tint(noise(size, size, 6, 21), (92, 70, 45), (140, 112, 76))
    d = ImageDraw.Draw(img)
    r = random.Random(22)
    for _ in range(14):
        x, y = r.randrange(size), r.randrange(size)
        rx, ry = r.randint(7, 14), r.randint(5, 9)
        c = r.randint(110, 150)
        for dx in (-size, 0, size):
            for dy in (-size, 0, size):
                box = [x + dx - rx, y + dy - ry, x + dx + rx, y + dy + ry]
                d.ellipse(box, fill=(c, c - 6, c - 16), outline=(60, 48, 34))
                d.arc([box[0] + 2, box[1] + 1, box[2] - 2, box[3] - 2], 200, 320, fill=(c + 30, c + 24, c + 12))
    return img.filter(ImageFilter.SMOOTH)


def stone_ring(img, cx, cy, r0, r1, count, seed):
    d = ImageDraw.Draw(img)
    r = random.Random(seed)
    for i in range(count):
        a0, a1 = 2 * math.pi * i / count, 2 * math.pi * (i + 1) / count
        pts = [(cx + rr * math.cos(a), cy + rr * math.sin(a)) for rr, a in
               ((r0, a0), (r1, a0), (r1, a1), (r0, a1))]
        c = r.randint(118, 156)
        d.polygon(pts, fill=(c, c - 4, c - 14), outline=(64, 56, 46))


def door(size=256):
    """The round green door of a hobbit hole with a brass knob in the middle, in a ring of stones."""
    c = size // 2
    img = facade(size)
    stone_ring(img, c, c, 102, 124, 22, 31)
    planks = tint(noise(size, size, 5, 32) * 0.5 + noise(size, 16, 1, 33, 1)[0:1].repeat(size, 0) * 0.5,
                  (26, 84, 38), (52, 132, 62))
    d = ImageDraw.Draw(planks)
    for x in range(c - 104, c + 105, 26):
        d.line([(x, 0), (x, size)], fill=(18, 58, 26), width=2)
    for y in (c - 58, c + 58):
        d.rectangle([c - 104, y - 5, c + 104, y + 5], fill=(40, 34, 30))
        for x in range(c - 90, c + 91, 26):
            d.ellipse([x - 2, y - 2, x + 2, y + 2], fill=(150, 130, 90))
    mask = Image.new('L', (size, size), 0)
    ImageDraw.Draw(mask).ellipse([c - 102, c - 102, c + 102, c + 102], fill=255)
    # Rounded look: darker towards the rim.
    yy, xx = np.mgrid[0:size, 0:size]
    rim = 1.0 - 0.35 * np.clip((np.hypot(xx - c, yy - c) - 60) / 42, 0, 1)
    planks = shade(planks, rim[..., None])
    img.paste(planks, (0, 0), mask)
    d = ImageDraw.Draw(img)
    d.ellipse([c - 103, c - 103, c + 103, c + 103], outline=(30, 26, 20), width=3)
    d.ellipse([c - 11, c - 11, c + 11, c + 11], fill=(150, 112, 30), outline=(70, 50, 12), width=2)
    d.ellipse([c - 6, c - 7, c + 2, c - 1], fill=(236, 206, 110))
    return img


def window(size=128):
    """A round window with a wooden cross, warm light behind the glass."""
    c = size // 2
    img = facade(size)
    stone_ring(img, c, c, 30, 42, 12, 41)
    d = ImageDraw.Draw(img)
    yy, xx = np.mgrid[0:size, 0:size]
    glow = np.clip(1 - np.hypot(xx - c + 6, yy - c + 6) / 34, 0, 1)
    glass = tint(glow, (120, 80, 30), (255, 226, 140))
    mask = Image.new('L', (size, size), 0)
    ImageDraw.Draw(mask).ellipse([c - 30, c - 30, c + 30, c + 30], fill=255)
    img.paste(glass, (0, 0), mask)
    d.rectangle([c - 3, c - 30, c + 3, c + 30], fill=(80, 52, 28))
    d.rectangle([c - 30, c - 3, c + 30, c + 3], fill=(80, 52, 28))
    d.ellipse([c - 31, c - 31, c + 31, c + 31], outline=(70, 46, 24), width=3)
    return img


# ---------------------------------------------------------------------------------------------------------------------
# Interiors. A hobbit hole is a round tunnel: curved plastered walls, wooden panels and beams, a round window.

# Interiors are painted 34 pixels taller and the top is cut off, which lifts the room above the dialogue text that
# covers the bottom of the picture in the game.
LIFT = 34
W, H = INTERIOR[0], INTERIOR[1] + LIFT
FLOOR_Y = 250  # Where the back wall meets the floor.


def room_base(seed):
    r = random.Random(seed)
    img = tint(noise(W, H, 5, seed), (176, 138, 88), (222, 190, 136))  # Plaster.
    d = ImageDraw.Draw(img)
    # Arched ceiling: the room is a tunnel, dark wooden ribs following the arch.
    d.rectangle([0, 0, W, 40], fill=(92, 60, 34))
    for i in range(7):
        k = i / 6
        d.arc([-120 + k * 20, -80 + k * 30, W + 120 - k * 20, 330 - k * 10], 190, 350, fill=(84, 54, 30), width=9)
    d.pieslice([-140, -40, W + 140, 520], 180, 360, outline=(70, 44, 24), width=14)
    # Wooden panelling on the lower wall.
    panel = tint(noise(W, 90, 3, seed + 1), (96, 60, 30), (136, 90, 48))
    img.paste(panel, (0, FLOOR_Y - 90))
    for x in range(0, W, 46):
        d.line([(x, FLOOR_Y - 90), (x, FLOOR_Y)], fill=(70, 42, 20), width=2)
    d.rectangle([0, FLOOR_Y - 96, W, FLOOR_Y - 88], fill=(80, 50, 26))
    # Floor boards in perspective.
    floor = tint(noise(W, H - FLOOR_Y, 4, seed + 2), (98, 64, 34), (150, 104, 60))
    img.paste(floor, (0, FLOOR_Y))
    for i in range(-12, 13):
        d.line([(W / 2 + i * 22, FLOOR_Y), (W / 2 + i * 70, H)], fill=(74, 46, 22), width=2)
    for y in (272, 300, 334):
        d.line([(0, y), (W, y)], fill=(86, 54, 28), width=1)
    d.line([(0, FLOOR_Y), (W, FLOOR_Y)], fill=(60, 36, 18), width=3)
    return img, d, r


def round_window(img, d, cx, cy, radius):
    yy, xx = np.mgrid[0:2 * radius, 0:2 * radius]
    view = np.zeros((2 * radius, 2 * radius, 3))
    t = yy / (2 * radius)
    view[...] = (np.array([150, 196, 236]) * (1 - t[..., None]) + np.array([214, 232, 244]) * t[..., None])
    hills = 1.2 * radius + 10 * np.sin(xx / 9.0) + 6 * np.sin(xx / 4.0 + 1)
    view[yy > hills] = (70, 130, 50)
    view[yy > hills + 14] = (56, 112, 40)
    img.paste(Image.fromarray(view.astype(np.uint8), 'RGB'), (cx - radius, cy - radius),
              _circle_mask(2 * radius))
    d.ellipse([cx - radius, cy - radius, cx + radius, cy + radius], outline=(80, 50, 24), width=7)
    d.line([(cx, cy - radius), (cx, cy + radius)], fill=(92, 58, 28), width=5)
    d.line([(cx - radius, cy), (cx + radius, cy)], fill=(92, 58, 28), width=5)
    d.rectangle([cx - radius - 6, cy + radius - 2, cx + radius + 6, cy + radius + 6], fill=(96, 62, 30))
    d.ellipse([cx - 16, cy + radius - 22, cx + 4, cy + radius - 2], fill=(200, 60, 60))  # Geranium.
    d.ellipse([cx - 4, cy + radius - 26, cx + 14, cy + radius - 8], fill=(226, 90, 80))
    d.polygon([(cx - 14, cy + radius - 4), (cx + 12, cy + radius - 4), (cx + 8, cy + radius + 2),
               (cx - 10, cy + radius + 2)], fill=(170, 90, 50))


def _circle_mask(size):
    m = Image.new('L', (size, size), 0)
    ImageDraw.Draw(m).ellipse([0, 0, size - 1, size - 1], fill=255)
    return m


def round_door(img, d, cx, top, radius):
    """The round front door seen from inside, on the back wall."""
    door_img = tint(noise(2 * radius, 2 * radius, 4, 5), (30, 88, 42), (56, 136, 66))
    dd = ImageDraw.Draw(door_img)
    for x in range(0, 2 * radius, 16):
        dd.line([(x, 0), (x, 2 * radius)], fill=(20, 60, 28), width=2)
    img.paste(door_img, (cx - radius, top), _circle_mask(2 * radius))
    d.ellipse([cx - radius, top, cx + radius, top + 2 * radius], outline=(70, 44, 22), width=6)
    d.ellipse([cx - 6, top + radius - 6, cx + 6, top + radius + 6], fill=(196, 150, 50))


def fireplace(img, d, x, fire=True):
    """Stone fireplace standing at x (its left edge) on the back wall."""
    r = random.Random(x)
    d.rectangle([x, FLOOR_Y - 130, x + 110, FLOOR_Y + 6], fill=(120, 112, 100))
    for row in range(8):
        for col in range(5):
            sx = x + col * 22 + (11 if row % 2 else 0) - 6
            sy = FLOOR_Y - 128 + row * 17
            c = r.randint(104, 150)
            d.rounded_rectangle([sx, sy, sx + 20, sy + 14], 4, fill=(c, c - 6, c - 14), outline=(80, 74, 66))
    d.rectangle([x - 8, FLOOR_Y - 138, x + 118, FLOOR_Y - 126], fill=(96, 62, 34))  # Mantel.
    d.chord([x + 18, FLOOR_Y - 90, x + 92, FLOOR_Y + 30], 180, 360, fill=(30, 20, 14))
    d.rectangle([x + 18, FLOOR_Y - 30, x + 92, FLOOR_Y + 2], fill=(30, 20, 14))
    if fire:
        for i, c in enumerate(((200, 60, 20), (240, 130, 30), (255, 210, 90))):
            k = 3 * i
            d.polygon([(x + 30 + k, FLOOR_Y), (x + 40 + k, FLOOR_Y - 34 + 4 * i), (x + 50, FLOOR_Y - 50 + 8 * i),
                       (x + 62 - k, FLOOR_Y - 30 + 4 * i), (x + 80 - k, FLOOR_Y)], fill=c)
        d.rectangle([x + 28, FLOOR_Y - 4, x + 82, FLOOR_Y + 2], fill=(70, 40, 20))
    return (x + 55, FLOOR_Y - 20)


def table(d, x, y, w):
    d.rectangle([x, y, x + w, y + 12], fill=(120, 78, 38), outline=(70, 44, 20))
    d.rectangle([x, y + 12, x + w, y + 18], fill=(92, 58, 28))
    for lx in (x + 8, x + w - 16):
        d.rectangle([lx, y + 18, lx + 8, y + 70], fill=(96, 60, 30))


def teapot(d, x, y):
    d.ellipse([x, y - 22, x + 30, y + 2], fill=(210, 220, 236), outline=(90, 100, 120))
    d.polygon([(x + 28, y - 12), (x + 42, y - 22), (x + 40, y - 18), (x + 30, y - 6)], fill=(200, 210, 230))
    d.arc([x - 10, y - 20, x + 6, y - 4], 90, 270, fill=(160, 170, 190), width=3)
    d.ellipse([x + 11, y - 27, x + 19, y - 21], fill=(170, 180, 200))


def mug(d, x, y, c=(150, 110, 70)):
    d.rectangle([x, y - 16, x + 12, y], fill=c, outline=(80, 56, 30))
    d.rectangle([x, y - 18, x + 12, y - 14], fill=(250, 244, 226))
    d.arc([x + 8, y - 13, x + 18, y - 3], 270, 90, fill=c, width=3)


def bread(d, x, y):
    d.ellipse([x, y - 14, x + 34, y + 2], fill=(196, 140, 70), outline=(130, 86, 40))
    for k in range(3):
        d.arc([x + 6 + 8 * k, y - 12, x + 14 + 8 * k, y - 2], 250, 330, fill=(236, 190, 120), width=2)


def shelf(d, x, y, w, items, seed):
    r = random.Random(seed)
    d.rectangle([x, y, x + w, y + 6], fill=(104, 66, 32), outline=(70, 42, 20))
    px = x + 4
    while px < x + w - 14:
        kind = r.choice(items)
        if kind == 'jar':
            c = r.choice([(190, 60, 50), (60, 120, 190), (80, 170, 80), (210, 180, 60), (150, 80, 170)])
            h = r.randint(14, 24)
            d.rectangle([px, y - h, px + 12, y], fill=c, outline=(50, 40, 30))
            d.rectangle([px + 2, y - h - 4, px + 10, y - h], fill=(120, 90, 60))
            px += 16
        elif kind == 'plate':
            d.ellipse([px, y - 22, px + 20, y - 2], fill=(226, 226, 236), outline=(120, 120, 150))
            d.ellipse([px + 5, y - 17, px + 15, y - 7], outline=(90, 120, 190))
            px += 24
        elif kind == 'book':
            for _ in range(r.randint(2, 4)):
                c = r.choice([(120, 30, 30), (40, 70, 120), (60, 100, 50), (130, 100, 40)])
                h = r.randint(16, 22)
                d.rectangle([px, y - h, px + 6, y], fill=c, outline=(30, 20, 14))
                px += 7
            px += 4
        elif kind == 'sack':
            d.chord([px, y - 26, px + 26, y + 10], 180, 360, fill=(190, 160, 110), outline=(120, 96, 60))
            px += 30


def barrel(d, x, y, r=26):
    d.rounded_rectangle([x, y - 2 * r - 10, x + 2 * r, y], 12, fill=(128, 82, 40), outline=(70, 44, 20))
    for k in (0.2, 0.8):
        yy = y - (2 * r + 10) * k
        d.line([(x + 2, yy), (x + 2 * r - 2, yy)], fill=(60, 60, 64), width=4)
    d.ellipse([x + r - 4, y - r - 8, x + r + 4, y - r], fill=(90, 56, 26))


def herbs(d, x, y, seed):
    r = random.Random(seed)
    d.line([(x + 6, y - 20), (x + 6, y)], fill=(90, 70, 40), width=2)
    for _ in range(9):
        a = r.uniform(-0.6, 0.6)
        l = r.randint(20, 34)
        c = r.choice([(90, 130, 50), (120, 150, 60), (150, 120, 170), (200, 190, 90)])
        d.line([(x + 6, y), (x + 6 + l * math.sin(a), y + l * math.cos(a))], fill=c, width=3)


def rug(d):
    d.ellipse([110, 286, 350, 340], fill=(150, 50, 40), outline=(100, 30, 26), width=3)
    d.ellipse([140, 294, 320, 332], outline=(214, 170, 80), width=3)


def lamp(d, x, y):
    d.line([(x, 0), (x, y - 16)], fill=(60, 50, 40), width=2)
    d.polygon([(x - 10, y - 16), (x + 10, y - 16), (x + 7, y + 4), (x - 7, y + 4)], fill=(250, 214, 110),
              outline=(90, 66, 30))


def warm_light(img, spots):
    """Warm light from the fire and the window, darker corners."""
    yy, xx = np.mgrid[0:H, 0:W]
    light = np.full((H, W), 0.55)
    for (sx, sy), strength, radius in spots:
        light += strength * np.exp(-((xx - sx) ** 2 + (yy - sy) ** 2) / (2 * radius ** 2))
    a = np.asarray(img, float)
    a = a * np.clip(light, 0, 1.25)[..., None]
    a[..., 0] *= 1.06
    a[..., 2] *= 0.92
    return Image.fromarray(a.clip(0, 255).astype(np.uint8), 'RGB').filter(ImageFilter.SMOOTH)


def hole_interior():
    img, d, r = room_base(101)
    round_window(img, d, 110, 120, 52)
    round_door(img, d, 240, 96, 76)
    fire = fireplace(img, d, 330)
    shelf(d, 20, 205, 140, ['plate', 'plate', 'book'], 102)
    rug(d)
    table(d, 60, 262, 150)
    teapot(d, 90, 262)
    mug(d, 136, 262, (220, 220, 230))
    bread(d, 160, 262)
    lamp(d, 180, 60)
    return warm_light(img, [(fire, 0.7, 70), ((110, 120), 0.4, 80), ((180, 70), 0.3, 60)])


def tavern_interior():
    img, d, r = room_base(201)
    round_window(img, d, 70, 110, 44)
    fire = fireplace(img, d, 340)
    # The sign of the Green Dragon on the wall.
    d.rectangle([150, 70, 270, 130], fill=(90, 58, 30), outline=(60, 36, 16), width=3)
    dragon = [(160, 118), (182, 96), (200, 104), (222, 84), (246, 92), (258, 80), (252, 104), (230, 104),
              (214, 120), (196, 112), (178, 124)]
    d.polygon(dragon, fill=(60, 150, 60), outline=(20, 70, 20))
    d.polygon([(200, 104), (212, 86), (220, 100)], fill=(90, 180, 80))
    # Barrels behind the bar and the bar counter with mugs.
    for i, x in enumerate((150, 206, 262)):
        barrel(d, x, FLOOR_Y - 6)
    d.rectangle([20, 256, 330, 280], fill=(122, 78, 36), outline=(70, 44, 20), width=2)
    d.rectangle([28, 280, 322, 344], fill=(98, 62, 30))
    for x in range(40, 320, 40):
        d.line([(x, 282), (x, 344)], fill=(74, 46, 22), width=2)
    for x in (50, 90, 150, 220, 280):
        mug(d, x, 256)
    lamp(d, 110, 50)
    lamp(d, 300, 50)
    return warm_light(img, [(fire, 0.7, 70), ((110, 50), 0.35, 60), ((300, 50), 0.35, 60)])


def shop_interior():
    img, d, r = room_base(301)
    round_window(img, d, 380, 110, 46)
    for i, y in enumerate((96, 146, 196, 246)):
        shelf(d, 16, y, 250, ['jar', 'jar', 'jar', 'sack'] if i < 3 else ['sack'], 302 + i)
    d.rectangle([290, 246, 454, 272], fill=(122, 78, 36), outline=(70, 44, 20), width=2)
    d.rectangle([298, 272, 446, 344], fill=(98, 62, 30))
    for x in (310, 350):
        mug(d, x, 246, (200, 200, 180))
    bread(d, 390, 246)
    lamp(d, 200, 44)
    return warm_light(img, [((380, 110), 0.45, 80), ((200, 44), 0.4, 70)])


def healer_interior():
    img, d, r = room_base(401)
    round_window(img, d, 230, 104, 50)
    fire = fireplace(img, d, 340, fire=True)
    for i, x in enumerate(range(30, 320, 36)):
        herbs(d, x, 48 + (i % 2) * 8, 402 + i)
    shelf(d, 20, 200, 150, ['jar', 'jar', 'book'], 403)
    # A bed with a patchwork blanket.
    d.rectangle([40, 270, 230, 300], fill=(236, 230, 214), outline=(120, 100, 80))
    for k, c in enumerate(((180, 70, 60), (70, 110, 170), (200, 170, 70), (90, 150, 80))):
        d.rectangle([90 + 35 * k, 270, 125 + 35 * k, 300], fill=c)
    d.rectangle([34, 250, 46, 320], fill=(110, 70, 36))
    d.rectangle([230, 264, 242, 320], fill=(110, 70, 36))
    # Mortar and pestle.
    d.chord([270, 250, 310, 290], 0, 180, fill=(150, 146, 140), outline=(90, 86, 80))
    d.line([(296, 262), (312, 236)], fill=(120, 110, 100), width=6)
    return warm_light(img, [(fire, 0.6, 70), ((230, 104), 0.45, 80)])


def smith_interior():
    img, d, r = room_base(501)
    # The forge instead of a fireplace, glowing coals.
    d.rectangle([260, FLOOR_Y - 120, 440, FLOOR_Y + 10], fill=(110, 104, 96), outline=(70, 66, 60), width=3)
    d.polygon([(280, FLOOR_Y - 120), (420, FLOOR_Y - 120), (380, 20), (320, 20)], fill=(96, 90, 84))
    d.rectangle([280, FLOOR_Y - 50, 420, FLOOR_Y - 10], fill=(40, 26, 18))
    for k in range(12):
        x = 290 + k * 11
        d.ellipse([x, FLOOR_Y - 40, x + 12, FLOOR_Y - 28], fill=(255, 120 + (k * 37) % 100, 30))
    # Anvil.
    d.polygon([(90, 262), (200, 262), (216, 250), (180, 250), (176, 240), (110, 240), (70, 248)], fill=(70, 72, 78))
    d.rectangle([118, 262, 170, 300], fill=(60, 62, 66))
    d.rectangle([104, 300, 184, 312], fill=(56, 58, 62))
    # Tools and horseshoes on the wall.
    for i, x in enumerate(range(30, 230, 40)):
        d.line([(x, 100), (x, 170)], fill=(110, 76, 40), width=5)
        d.rectangle([x - 10, 94, x + 10, 108], fill=(80, 82, 88))
    for x in (40, 90, 140, 190):
        d.arc([x, 190, x + 30, 220], 150, 30, fill=(90, 92, 98), width=6)
    return warm_light(img, [((350, FLOOR_Y - 35), 0.9, 90), ((140, 250), 0.2, 80)])


def images():
    """All pictures: {LOD entry name: image}."""
    return {
        'hobtrava': grass(),
        'hobfasad': facade(),
        'hobdvere': door(),
        'hobokno': window(),
        'krajnora': lifted(hole_interior()),
        'krajhospoda': lifted(tavern_interior()),
        'krajobchod': lifted(shop_interior()),
        'krajlecitel': lifted(healer_interior()),
        'krajkovar': lifted(smith_interior()),
    }


def lifted(img):
    return img.crop((0, LIFT, W, H))


if __name__ == '__main__':
    for name, img in images().items():
        img.save(f'{sys.argv[1]}/{name}.png')
