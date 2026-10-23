"""Encodes images into MM7 LOD image entries (8 bit paletted, zlib compressed), as read by OpenEnroth."""
import math
import struct
import zlib

from PIL import Image


def encode(name, img, colors=256):
    """img: PIL image. Returns the LOD entry bytes (header, compressed pixels, palette)."""
    p = img.convert('RGB').quantize(colors=colors, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    w, h = p.size
    pixels = p.tobytes()
    pal = bytes(p.getpalette()[:768]).ljust(768, b'\0')
    comp = zlib.compress(pixels, 9)
    ln = lambda v: int(math.log2(v)) if v & (v - 1) == 0 else 0  # noqa: E731
    header = name.encode('ascii').ljust(16, b'\0')
    header += struct.pack('<IIHHhhhhhhII', w * h, len(comp), w, h, ln(w), ln(h), w - 1, h - 1, 0, 0, len(pixels), 16)
    assert len(header) == 48
    return header + comp + pal
