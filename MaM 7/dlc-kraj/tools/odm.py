"""Reads and writes MM7 outdoor maps (.odm) and their deltas (.ddm), following the OpenEnroth snapshot structs
(Engine/Snapshots/CompositeSnapshots.h, OutdoorLocation_MM7 and OutdoorDelta_MM7)."""
import struct
import zlib

N = 128 * 128


def unpack_lod_entry(b):
    """Decompresses a games.lod entry ("mvii" header) or returns it as is."""
    if len(b) >= 16 and b[4:8] == b'mvii':
        _, _, size, unpacked = struct.unpack_from('<I4sII', b, 0)
        return zlib.decompress(b[16:16 + size]) if unpacked else b[16:16 + size]
    return b


def pack_lod_entry(b):
    c = zlib.compress(b, 9)
    return struct.pack('<I4sII', 91969, b'mvii', len(c), len(b)) + c


class Reader:
    def __init__(self, b):
        self.b, self.o = b, 0

    def take(self, n):
        r = self.b[self.o:self.o + n]
        assert len(r) == n, 'unexpected end of data'
        self.o += n
        return r

    def u32(self):
        return struct.unpack('<I', self.take(4))[0]

    def items(self, size):
        """Vector with an uint32 count prefix."""
        n = self.u32()
        return [self.take(size) for _ in range(n)]


def vec(items):
    return struct.pack('<I', len(items)) + b''.join(items)


MODEL_SIZE, FACE_SIZE, NODE_SIZE, DECO_SIZE, SPAWN_SIZE = 188, 308, 8, 32, 24


def read_odm(b):
    r = Reader(b)
    m = {}
    for k in ('name', 'fileName', 'description', 'skyTexture', 'groundTileset'):
        m[k] = r.take(32)
    m['tileTypes'] = r.take(16)
    m['heightMap'] = bytearray(r.take(N))
    m['tileMap'] = bytearray(r.take(N))
    m['attributeMap'] = bytearray(r.take(N))
    normal_count = r.u32()
    m['someOtherMap'] = r.take(N * 2 * 4)
    m['normalMap'] = r.take(N * 2 * 2)
    m['normals'] = r.take(normal_count * 12)
    m['normalCount'] = normal_count
    models = r.items(MODEL_SIZE)
    m['models'] = []
    for md in models:
        nv, = struct.unpack_from('<I', md, 68)
        nf, = struct.unpack_from('<I', md, 76)
        nn, = struct.unpack_from('<I', md, 92)
        extra = {'vertices': r.take(nv * 12), 'faces': r.take(nf * FACE_SIZE), 'ordering': r.take(nf * 2),
                 'nodes': r.take(nn * NODE_SIZE), 'textures': r.take(nf * 10)}
        m['models'].append((md, extra))
    decos = r.items(DECO_SIZE)
    m['decorations'] = list(zip(decos, [r.take(32) for _ in decos]))
    m['decorationPidList'] = r.items(2)
    m['decorationMap'] = r.take(N * 4)
    m['spawnPoints'] = r.items(SPAWN_SIZE)
    assert r.o == len(b), f'{len(b) - r.o} bytes left over'
    return m


def write_odm(m):
    out = [m[k] for k in ('name', 'fileName', 'description', 'skyTexture', 'groundTileset')]
    out += [m['tileTypes'], bytes(m['heightMap']), bytes(m['tileMap']), bytes(m['attributeMap'])]
    out += [struct.pack('<I', len(m['normals']) // 12), m['someOtherMap'], m['normalMap'], m['normals']]
    out.append(vec([md for md, _ in m['models']]))
    for _, e in m['models']:
        out += [e['vertices'], e['faces'], e['ordering'], e['nodes'], e['textures']]
    out.append(vec([d for d, _ in m['decorations']]))
    out += [n for _, n in m['decorations']]
    out.append(vec(m['decorationPidList']))
    out.append(m['decorationMap'])
    out.append(vec(m['spawnPoints']))
    return b''.join(out)


def read_ddm(b, odm):
    total_faces = sum(struct.unpack_from('<I', md, 76)[0] for md, _ in odm['models'])
    r = Reader(b)
    d = {'header': r.take(40), 'fully': r.take(88 * 11), 'partially': r.take(88 * 11),
         'faceAttributes': r.take(total_faces * 4), 'decorationFlags': r.take(len(odm['decorations']) * 2)}
    d['actors'] = r.items(0x344)
    d['spriteObjects'] = r.items(0x70)
    d['chests'] = r.items(5324)
    d['eventVariables'] = r.take(0xC8)
    d['lastVisitTime'] = r.take(8)
    d['weather'] = r.take(0x30)
    assert r.o == len(b), f'{len(b) - r.o} bytes left over'
    return d


def write_ddm(d):
    return b''.join([d['header'], d['fully'], d['partially'], d['faceAttributes'], d['decorationFlags'],
                     vec(d['actors']), vec(d['spriteObjects']), vec(d['chests']), d['eventVariables'],
                     d['lastVisitTime'], d['weather']])
