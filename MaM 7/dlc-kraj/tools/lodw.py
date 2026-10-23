"""Writes an MM7 LOD archive (single root directory), as read by OpenEnroth's LodReader."""
import struct


def write_lod(path, files, version=b'MMVII', directory=b'maps'):
    """files: {name: bytes}. Names are stored lowercased, at most 15 characters."""
    names = sorted(files, key=lambda n: n.lower())
    header = b'LOD\0' + version.ljust(80, b'\0') + b'Kraj - MaM 7 rozsireni'.ljust(80, b'\0')
    header += struct.pack('<III', 100, 0, 1) + b'\0' * 80
    assert len(header) == 256
    dir_size = 32 * len(names)
    offset = dir_size
    entries, blobs = [], []
    for n in names:
        b = files[n]
        nm = n.lower().encode('ascii')
        assert len(nm) <= 15, n
        entries.append(nm.ljust(16, b'\0') + struct.pack('<IIIHH', offset, len(b), 0, 0, 0))
        blobs.append(b)
        offset += len(b)
    root = directory.ljust(16, b'\0') + struct.pack('<IIIHH', 288, offset, 0, len(names), 0)
    with open(path, 'wb') as f:
        f.write(header + root + b''.join(entries) + b''.join(blobs))
