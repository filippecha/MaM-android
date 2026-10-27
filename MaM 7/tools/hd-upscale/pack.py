"""Turns the upscaled textures into the pack the game ships: pack.py <hd dir> <out dir>. Opaque textures become
JPEG (quality 90, no chroma subsampling), the few with alpha stay PNG. The engine tries .png first, then .jpg."""
import os
import sys

from PIL import Image

src, out = sys.argv[1:3]
os.makedirs(out, exist_ok=True)
for name in os.listdir(out):
    os.remove(os.path.join(out, name))
total = 0
for name in sorted(os.listdir(src)):
    if not name.lower().endswith('.png'):
        continue
    im = Image.open(os.path.join(src, name))
    base = name[:-4]
    if im.mode == 'RGBA' and im.getchannel('A').getextrema()[0] < 255:
        target = os.path.join(out, base + '.png')
        im.save(target, optimize=True)
    else:
        target = os.path.join(out, base + '.jpg')
        im.convert('RGB').save(target, quality=90, subsampling=0, optimize=True)
    total += os.path.getsize(target)
print(f'{total / 1e6:.0f} MB')
