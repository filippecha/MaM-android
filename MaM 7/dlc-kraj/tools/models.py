"""Builds outdoor models (ODM bmodels) out of flat polygons, in the binary layout read by odm.py.

Textures map 1 texel to 1 world unit, as in the original maps. The engine recomputes normals, BSP nodes aren't used
outdoors.
"""
import math
import struct

FACE_XY_PLANE, FACE_XZ_PLANE, FACE_YZ_PLANE, FACE_CLICKABLE = 0x100, 0x200, 0x400, 0x2000000
POLYGON_VERTICAL_WALL, POLYGON_FLOOR = 1, 3


def _newell(points):
    nx = ny = nz = 0.0
    for (x0, y0, z0), (x1, y1, z1) in zip(points, points[1:] + points[:1]):
        nx += (y0 - y1) * (z0 + z1)
        ny += (z0 - z1) * (x0 + x1)
        nz += (x0 - x1) * (y0 + y1)
    return nx, ny, nz


class Face:
    def __init__(self, points, normal, texture, uv, event_id=0):
        """points: polygon corners, normal: axis-aligned unit normal it faces, uv: point -> (u, v)."""
        n = _newell(points)
        # The engine takes cross(v1 - v0, v2 - v1) as the normal, which winds the same way as Newell's normal.
        if n[0] * normal[0] + n[1] * normal[1] + n[2] * normal[2] < 0:
            points = points[::-1]
        self.points, self.normal, self.texture, self.uv, self.event_id = points, normal, texture, uv, event_id


def circle(center, right, radius, count):
    """Points of a regular polygon standing in a vertical plane: center, horizontal unit vector to the right."""
    cx, cy, cz = center
    rx, ry = right
    return [(cx + rx * radius * math.cos(a), cy + ry * radius * math.cos(a), cz + radius * math.sin(a))
            for a in (2 * math.pi * i / count for i in range(count))]


def wall_uv(normal, origin=(0, 0, 0), offset=(0, 0)):
    """Maps a vertical face so that the texture isn't mirrored when looking at it, origin at texel offset."""
    rx, ry = -normal[1], normal[0]
    return lambda p: ((p[0] - origin[0]) * rx + (p[1] - origin[1]) * ry + offset[0], -(p[2] - origin[2]) + offset[1])


def floor_uv():
    return lambda p: (p[0], -p[1])


def build(name, faces):
    """Returns (model data, extras) for odm['models']."""
    verts, index = [], {}

    def vid(p):
        key = tuple(int(round(c)) for c in p)
        if key not in index:
            index[key] = len(verts)
            verts.append(key)
        return index[key]

    face_blobs, textures = [], []
    for f in faces:
        ids = [vid(p) for p in f.points]
        pts = [verts[i] for i in ids]
        nx, ny, nz = f.normal
        dist = -int(round((nx * pts[0][0] + ny * pts[0][1] + nz * pts[0][2]) * 65536))
        if nz:
            attributes, polygon = FACE_XY_PLANE, POLYGON_FLOOR
        elif ny:
            attributes, polygon = FACE_XZ_PLANE, POLYGON_VERTICAL_WALL
        else:
            attributes, polygon = FACE_YZ_PLANE, POLYGON_VERTICAL_WALL
        if f.event_id:
            attributes |= FACE_CLICKABLE
        us, vs = zip(*(tuple(int(round(c)) for c in f.uv(p)) for p in pts))
        pad = lambda a: list(a) + [0] * (20 - len(a))  # noqa: E731
        xs, ys, zs = zip(*pts)
        b = struct.pack('<4i', int(nx * 65536), int(ny * 65536), int(nz * 65536), dist)
        b += struct.pack('<3i', 0, 0, 0)
        b += struct.pack('<I', attributes)
        b += struct.pack('<20h', *pad(ids))
        b += struct.pack('<20h', *pad(us)) + struct.pack('<20h', *pad(vs))
        b += bytes(3 * 40)
        b += struct.pack('<3h', 0, 0, 0)
        b += struct.pack('<6h', min(xs), max(xs), min(ys), max(ys), min(zs), max(zs))
        b += struct.pack('<hhh', 0, f.event_id, 0)
        b += bytes(6)
        b += struct.pack('<BBBBBB', len(ids), polygon, 0, 0, 0, 0)
        assert len(b) == 308, len(b)
        face_blobs.append(b)
        textures.append(f.texture.encode('ascii').ljust(10, b'\0'))

    xs, ys, zs = zip(*verts)
    lo, hi = (min(xs), min(ys), min(zs)), (max(xs), max(ys), max(zs))
    center = tuple((a + b) // 2 for a, b in zip(lo, hi))
    radius = int(math.dist(lo, hi) / 2) + 1
    md = name.encode('ascii').ljust(32, b'\0') * 2
    md += struct.pack('<iIIIIIIIIIii', 0, len(verts), 0, len(faces), 0, 0, 0, 0, 0, 0, 0, 0)
    md += struct.pack('<3i', center[0], center[1], 0)
    md += struct.pack('<6i', *lo, *hi) * 2
    md += struct.pack('<3ii', *center, radius)
    assert len(md) == 188, len(md)
    extra = {'vertices': b''.join(struct.pack('<3i', *v) for v in verts), 'faces': b''.join(face_blobs),
             'ordering': b''.join(struct.pack('<h', i) for i in range(len(faces))), 'nodes': b'',
             'textures': b''.join(textures)}
    return md, extra


def face_attributes(model):
    """Attributes of all faces of a model, for the delta (ddm)."""
    md, extra = model
    faces = extra['faces']
    return [struct.unpack_from('<I', faces, i + 28)[0] for i in range(0, len(faces), 308)]
