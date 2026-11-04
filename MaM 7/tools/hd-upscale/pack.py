"""Turns the upscaled textures into the pack the game ships: pack.py <hd dir> <out dir> [<orig dir>]. Opaque
textures become JPEG (quality 90, no chroma subsampling), the few with alpha stay PNG. The engine tries .png first,
then .jpg. With <orig dir> every texture's per-channel mean is matched to the original's, the upscaler shifts the
tone a little (darker, more contrast) and that shift would show up in the game."""
import os
import sys

import numpy as np
from PIL import Image

src, out = sys.argv[1:3]
orig_dir = sys.argv[3] if len(sys.argv) > 3 else None
os.makedirs(out, exist_ok=True)
for name in os.listdir(out):
    os.remove(os.path.join(out, name))
total = 0
for name in sorted(os.listdir(src)):
    if not name.lower().endswith('.png'):
        continue
    im = Image.open(os.path.join(src, name))
    base = name[:-4]
    has_alpha = im.mode == 'RGBA' and im.getchannel('A').getextrema()[0] < 255
    if orig_dir and os.path.exists(os.path.join(orig_dir, name)):
        orig = Image.open(os.path.join(orig_dir, name)).convert('RGBA')
        hd = np.asarray(im.convert('RGBA')).astype(np.float32)
        o = np.asarray(orig).astype(np.float32)
        # Opaque pixels only, the transparent ones carry no colour of their own.
        hm = hd[:, :, 3] > 0
        om = o[:, :, 3] > 0
        if hm.any() and om.any():
            for c in range(3):
                mo = o[:, :, c][om].mean()
                mh = hd[:, :, c][hm].mean()
                if mh > 1:
                    hd[:, :, c] = np.clip(hd[:, :, c] * (mo / mh), 0, 255)
        im = Image.fromarray(hd.astype(np.uint8), 'RGBA')
    if has_alpha:
        target = os.path.join(out, base + '.png')
        im.save(target, optimize=True)
    else:
        target = os.path.join(out, base + '.jpg')
        im.convert('RGB').save(target, quality=90, subsampling=0, optimize=True)
    total += os.path.getsize(target)
print(f'{total / 1e6:.0f} MB')
