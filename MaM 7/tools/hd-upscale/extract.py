"""Extracts the textures of bitmaps.lod as PNG for upscaling: python extract.py <bitmaps.lod> <out dir>.
Palettes, light maps, the placeholder and textures with lettering or drawings (signs, banners, tapestries, the
upscaler invents letters) are skipped. Colour-keyed textures (the engine's list in ImageLoader.cpp: palette index 0
is transparent) get an alpha channel, the upscaler handles it separately."""
import os
import struct
import sys
import zlib

from PIL import Image

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'dlc-kraj', 'tools'))
import lod  # noqa: E402

TRANSPARENT = {'hwtrdre', 'hwtrdrne', 'hwtrdrs', 'hwtrdrsw', 'hwtrdrxne', 'hwtrdrxse', 'hwtrdrn', 'hwtrdrnw', 'hwtrdrse',
               'hwtrdrw', 'hwtrdrxnw', 'hwtrdrxsw'}
SKIP_PREFIX = ('pal', 'lightmap', 'shdwmap', 'pending', 'sign', 'warnsign', 'wharsign', 'bannwc', 'banwiz', 'wizh',
               'ceb1tap', 'tap', 'splmsk', 'effpar', 'hwsplat', 'solid',
               # MM8: letters of the alphabet puzzles and shop signs.
               'alphanum', 'sgalch', 'sgarm', 'sgbank', 'sginn', 'sgmagic', 'sgspell', 'sgstabl', 'sgtrain', 'sgtvrn',
               'sgweap')

src, out = sys.argv[1:3]
os.makedirs(out, exist_ok=True)
mm8 = b'MMVIII' in open(src, 'rb').read(16)
# MM8 (EnglishD.lod next to the LOD): the MM6/MM8 engine copy also keys the shore tiles without the "h" prefix.
if any(f.lower() == 'englishd.lod' for f in os.listdir(os.path.dirname(os.path.abspath(src)))):
    TRANSPARENT |= {t[1:] for t in TRANSPARENT}
count = 0
for key, (name, blob) in sorted(lod.entries(src).items()):
    if key.startswith(SKIP_PREFIX):
        continue
    b = blob[48:] if mm8 else blob  # MM8 entries start with a 64 byte name instead of 16.
    size, dsz, w, h = struct.unpack_from('<IIHH', b, 16)
    dec, = struct.unpack_from('<I', b, 40)
    px = b[48:48 + dsz]
    px = zlib.decompress(px) if dec else px
    pal = b[48 + dsz:48 + dsz + 768]
    if w == 0 or h == 0 or len(px) < w * h:
        print('skip', name, w, h)
        continue
    im = Image.frombytes('P', (w, h), px[:w * h])
    im.putpalette(pal)
    rgb = im.convert('RGB')
    if key in TRANSPARENT:
        alpha = Image.frombytes('L', (w, h), bytes(0 if p == 0 else 255 for p in px[:w * h]))
        rgb.putalpha(alpha)
    rgb.save(os.path.join(out, key + '.png'))
    count += 1
print(count, 'textures')
