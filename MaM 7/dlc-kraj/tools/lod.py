"""Reads entries of MM6/7/8 LOD archives: python tools/lod.py <lod> [name] lists entries or dumps one to stdout."""
import struct
import sys
import zlib


def entries(path):
    """Returns {lowercase name: (name, raw entry data)}. MM8 entries keep their 64 byte name prefix."""
    d = open(path, 'rb').read()
    _, off, _, _, cnt = struct.unpack_from('<16sIIII', d, 0x100)
    mm8 = b'MMVIII' in d[:16]
    size, name_size = (76, 64) if mm8 else (32, 16)
    result = {}
    for i in range(cnt):
        e = d[off + i * size:off + (i + 1) * size]
        name = e[:name_size].split(b'\0')[0].decode('latin1')
        o, s = struct.unpack_from('<II', e, name_size)
        result[name.lower()] = (name, d[off + o:off + o + s])
    return result


def data(path, name):
    """Returns an entry with an MM8 64 byte name cut down to the usual 16, so that headers start at the same place."""
    b = entries(path)[name.lower()][1]
    return b[48:] if b'MMVIII' in open(path, 'rb').read(16) else b


def text(path, name):
    """Returns a decompressed text or table from an MM7/MM8 events LOD."""
    b = data(path, name)
    data_size, = struct.unpack_from('<I', b, 20)
    unpacked, = struct.unpack_from('<I', b, 40)
    raw = b[48:48 + data_size]
    return zlib.decompress(raw) if unpacked else raw


if __name__ == '__main__':
    es = entries(sys.argv[1])
    if len(sys.argv) < 3:
        for name, blob in es.values():
            print(name, len(blob))
    else:
        sys.stdout.buffer.write(es[sys.argv[2].lower()][1])


def image(path, name):
    """Decodes an MM6/7/8 LOD image into a PIL image."""
    from PIL import Image
    b = data(path, name)
    size, dsz, w, h = struct.unpack_from('<IIHH', b, 16)
    dec, = struct.unpack_from('<I', b, 40)
    px = b[48:48 + dsz]
    px = zlib.decompress(px) if dec else px
    pal = b[48 + dsz:48 + dsz + 768]
    im = Image.frombytes('P', (w, h), px[:w * h])
    im.putpalette(pal)
    return im.convert('RGB')
