"""Extracts the UI images of icons.lod as RGB PNGs for upscaling: python extract_icons.py <icons.lod> <out dir>.
Entries are either paletted LOD images or zlib-packed PCX files. Transparent areas (palette index 0 of images that
use it, the teal colour key of PCX files) are inpainted so that the upscaler does not smear them into the edges.
The engine keeps the original transparency anyway, see applyHdIcon()."""
import io
import os
import struct
import sys
import zlib

import cv2
import numpy as np
from PIL import Image

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'dlc-kraj', 'tools'))
import lod  # noqa: E402

SKIP_PREFIX = ('pal', 'font', 'pending')


def inpaint(rgb, mask):
    if not mask.any() or mask.all():
        return rgb
    return cv2.inpaint(rgb[:, :, ::-1].copy(), mask.astype(np.uint8), 3, cv2.INPAINT_TELEA)[:, :, ::-1]


src, out = sys.argv[1:3]
os.makedirs(out, exist_ok=True)
count = 0
skipped = 0
for key, (name, blob) in sorted(lod.entries(src).items()):
    if key.startswith(SKIP_PREFIX):
        continue
    b = blob
    try:
        size, dsz, w, h = struct.unpack_from('<IIHH', b, 16)
        dec, = struct.unpack_from('<I', b, 40)
        if w and h and dsz and 48 + dsz + 768 <= len(b) + 4:
            px = b[48:48 + dsz]
            px = zlib.decompress(px) if dec else px
            pal = b[48 + dsz:48 + dsz + 768]
            if len(px) < w * h or len(pal) < 768:
                raise ValueError('short image')
            idx = np.frombuffer(px[:w * h], dtype=np.uint8).reshape(h, w)
            palette = np.frombuffer(pal, dtype=np.uint8).reshape(256, 3)
            rgb = palette[idx]
            mask = idx == 0
        else:
            data_size, = struct.unpack_from('<I', b, 20)
            unpacked, = struct.unpack_from('<I', b, 40)
            raw = b[48:48 + data_size]
            pcx = zlib.decompress(raw) if unpacked else raw
            rgb = np.array(Image.open(io.BytesIO(pcx)).convert('RGB'))
            mask = np.all(rgb == (0, 255, 255), axis=2)
    except Exception:  # noqa: BLE001
        skipped += 1
        continue
    if rgb.shape[0] < 4 or rgb.shape[1] < 4:
        skipped += 1
        continue
    rgb = inpaint(np.ascontiguousarray(rgb), mask)
    Image.fromarray(rgb.astype(np.uint8), 'RGB').save(os.path.join(out, key + '.png'))
    count += 1
print(count, 'images,', skipped, 'skipped')
