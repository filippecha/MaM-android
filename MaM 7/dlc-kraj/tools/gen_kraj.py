"""Generates data/kraj.lod of the MaM 7 expansion "Stín nad Krajem": the Shire map (out16.odm + out16.ddm), its
event script and strings, the map table row (kmapstats.txt), Hobbiton's houses, inhabitants and dialogues (see
hobitin.py) and the pictures and textures (see art.py).

    python gen_kraj.py <games.lod of MM7> <output kraj.lod> [preview.png]

Only the sky, the tile set list and the default weather are taken from Emerald Island, everything else is generated.
"""
import math
import random
import struct
import sys

sys.path.insert(0, __file__.rsplit('/', 1)[0] if '/' in __file__ else '.')
import art  # noqa: E402
import hobitin  # noqa: E402
import lodimg  # noqa: E402
import lodw  # noqa: E402
import models  # noqa: E402
import odm  # noqa: E402

GRASS, WATER, SWAMP, ROAD = 90, 126, 162, 198  # Base local tile ids of the four tile sets of Emerald Island.
ROAD_VARIANT = {'NSEW': 0, 'NS': 1, 'EW': 2, 'NE': 3, 'NW': 4, 'SE': 5, 'SW': 6, 'NSE': 7, 'NSW': 8, 'NEW': 9,
                'SEW': 10, 'S': 11, 'W': 12, 'N': 13, 'E': 14, 'N_S': 1, '': 0}

VILLAGE_ROAD_X = 64
LANES = (84, 98)  # Village lanes (grid rows).
HOLE_COLUMNS = (44, 52, 76, 84, 92)


def lane_y(lane, x):
    """Village lanes wind a little."""
    return lane + int(round(2 * math.sin(x / 8.0 + lane)))


def road_x(y):
    """The main road winds through the Shire from the south valley to the north."""
    return VILLAGE_ROAD_X + int(round(4 * math.sin(y / 11.0) if y < 108 else 0))


def hobbit_holes():
    """20 hills with a hobbit hole: 5 on each side of the two village lanes. Returns (x, y, door side)."""
    holes = []
    for lane in LANES:
        for side in (-1, 1):
            for x in HOLE_COLUMNS:
                holes.append((x, lane_y(lane, x) + side * 5, -side))  # The door faces the lane.
    return holes


DOOR_HEIGHT = 352  # Height of the entrance facade above the ground, in world units.


def entrance(number, hx, hy, d, ground):
    """Model of a hole entrance: a facade dug into the hill foot with a round door and two round windows.

    The facade stands on the terrain vertex row hy + 2d, d is the direction the door faces in grid rows."""
    x0 = (hx - 64) * 512
    yf = (64 - (hy + 2 * d)) * 512
    yb = yf + d * 512
    z0, zt = ground - 64, ground + DOOR_HEIGHT
    n = (0, -d, 0)
    right = (d, 0)
    faces = [models.Face([(x0 - 256, yf, z0), (x0 + 256, yf, z0), (x0 + 256, yf, zt), (x0 - 256, yf, zt)], n,
                         'hobfasad', models.wall_uv(n)),
             models.Face([(x0 - 256, yf, z0), (x0 - 256, yb, z0), (x0 - 256, yb, zt), (x0 - 256, yf, zt)],
                         (-1, 0, 0), 'hobfasad', models.wall_uv((-1, 0, 0))),
             models.Face([(x0 + 256, yf, z0), (x0 + 256, yb, z0), (x0 + 256, yb, zt), (x0 + 256, yf, zt)],
                         (1, 0, 0), 'hobfasad', models.wall_uv((1, 0, 0))),
             models.Face([(x0 - 256, yf, zt), (x0 + 256, yf, zt), (x0 + 256, yb, zt), (x0 - 256, yb, zt)],
                         (0, 0, 1), 'hobtrava', models.floor_uv())]
    door = (x0, yf - 2 * d, ground + 124)
    faces.append(models.Face(models.circle(door, right, 120, 16), n, 'hobdvere',
                             models.wall_uv(n, door, (128, 128)), event_id=number))
    for side in (-1, 1):
        window = (x0 + side * d * 176, yf - 2 * d, ground + 200)
        faces.append(models.Face(models.circle(window, right, 40, 12), n, 'hobokno',
                                 models.wall_uv(n, window, (64, 64))))
    return models.build(f'nora{number:02}', faces)


class Noise:
    def __init__(self, seed):
        r = random.Random(seed)
        self.grid = [[r.random() for _ in range(40)] for _ in range(40)]

    def __call__(self, x, y):
        x0, y0 = int(x), int(y)
        fx, fy = x - x0, y - y0
        sx, sy = fx * fx * (3 - 2 * fx), fy * fy * (3 - 2 * fy)
        g = self.grid
        a = g[y0 % 40][x0 % 40] * (1 - sx) + g[y0 % 40][(x0 + 1) % 40] * sx
        b = g[(y0 + 1) % 40][x0 % 40] * (1 - sx) + g[(y0 + 1) % 40][(x0 + 1) % 40] * sx
        return a * (1 - sy) + b * sy


def river_x(y):
    """The Brandywine flows from the north down the east side."""
    return 100 + 6 * math.sin(y / 11.0) + 3 * math.sin(y / 4.3)


def generate(template):
    rnd = random.Random(1937)
    n1, n2 = Noise(1), Noise(2)
    h = [[0.0] * 128 for _ in range(128)]
    tile = [[GRASS] * 128 for _ in range(128)]
    road = [[False] * 128 for _ in range(128)]

    # Rolling green hills, mountains around the playable area, a valley in the south where the road leaves.
    for y in range(128):
        for x in range(128):
            v = 42 + 22 * n1(x / 14, y / 14) + 8 * n2(x / 5, y / 5)
            d = min(x, 127 - x, y, 127 - y) + 10 * (n1(x / 6 + 17, y / 6 + 23) - 0.5)
            if d < 20 and not (y > 100 and abs(x - VILLAGE_ROAD_X) < 7):
                v += (20 - d) * 8
            h[y][x] = v

    # The village is flatter.
    for y in range(128):
        for x in range(128):
            r = math.hypot((x - 68) / 34.0, (y - 92) / 18.0)
            w = 1.0 if r < 0.8 else (0.0 if r > 1.3 else 0.5 + 0.5 * math.cos(math.pi * (r - 0.8) / 0.5))
            h[y][x] = h[y][x] - w * 0.65 * (h[y][x] - 40)

    # Hills of the hobbit holes.
    for hx, hy, _ in hobbit_holes():
        for y in range(hy - 4, hy + 5):
            for x in range(hx - 4, hx + 5):
                r = math.hypot(x - hx, y - hy)
                if r < 4:
                    h[y][x] += 20 * (0.5 + 0.5 * math.cos(math.pi * r / 4))

    # A flat yard in front of every door and a steep hill foot behind the facade.
    entrance_ground = []
    for hx, hy, d in hobbit_holes():
        g = round(h[hy + 4 * d][hx])
        entrance_ground.append(g)
        for x in range(hx - 2, hx + 3):
            for k in (2, 3, 4):
                h[hy + k * d][x] = g
            h[hy + d][x] = max(h[hy + d][x], g + 14 if abs(x - hx) <= 1 else g + 6)
            h[hy][x] = max(h[hy][x], g + 18 if abs(x - hx) <= 1 else g + 10)

    # River with two fords where the ground stays dry.
    fords = (46, 88)
    for y in range(0, 128):
        cx = river_x(y)
        for x in range(int(cx) - 3, int(cx) + 4):
            if abs(x - cx) <= 2.2 and all(abs(y - f) > 1 for f in fords):
                tile[y][x] = WATER
                h[y][x] = min(h[y][x], 30)
            elif abs(x - cx) <= 3.5:
                h[y][x] = min(h[y][x], 36)

    # Misty marshes in the north-east.
    for y in range(22, 40):
        for x in range(74, 96):
            if n2(x / 3, y / 3) > 0.45:
                tile[y][x] = SWAMP
                h[y][x] = min(h[y][x], 34)

    # Roads: from the south edge up through the village, on to the north; the two village lanes; to the ford.
    def mark(x, y):
        road[y][x] = True

    def path(points):
        """Marks a 4-connected road through the given cells."""
        for (x0, y0), (x1, y1) in zip(points, points[1:]):
            x, y = x0, y0
            mark(x, y)
            while (x, y) != (x1, y1):
                if x != x1:
                    x += 1 if x1 > x else -1
                else:
                    y += 1 if y1 > y else -1
                mark(x, y)
    path([(road_x(y), y) for y in range(110, 23, -1)])
    for lane in LANES:
        path([(x, lane_y(lane, x)) for x in range(40, 97)])
    path([(x, 46 + int(round(math.sin(x / 5.0)))) for x in range(road_x(46), int(river_x(46)) + 5)])
    for y in range(128):
        for x in range(128):
            if road[y][x]:
                tile[y][x] = ROAD
                h[y][x] = max(min(h[y][x], 60), 32)

    def is_road(x, y):
        return 0 <= x < 128 and 0 <= y < 128 and road[y][x]
    for y in range(128):
        for x in range(128):
            if road[y][x]:
                k = ('N' if is_road(x, y - 1) else '') + ('S' if is_road(x, y + 1) else '') + \
                    ('E' if is_road(x + 1, y) else '') + ('W' if is_road(x - 1, y) else '')
                tile[y][x] = ROAD + ROAD_VARIANT.get(k, 0)

    heights = bytearray(128 * 128)
    tiles = bytearray(128 * 128)
    for y in range(128):
        for x in range(128):
            heights[y * 128 + x] = max(0, min(255, int(round(h[y][x]))))
            tiles[y * 128 + x] = tile[y][x]

    def ground(wx, wy):
        gx, gy = (wx >> 9) + 64, 63 - (wy >> 9)
        return 32 * heights[max(0, min(127, gy)) * 128 + max(0, min(127, gx))]

    def to_world(gx, gy):
        return (gx - 64) * 512 + 256, (63 - gy) * 512 + 256

    decorations = []

    def deco(name, wx, wy, yaw=0):
        decorations.append((struct.pack('<HH3iiHHHhhh', 0, 0, wx, wy, ground(wx, wy), yaw, 0, 0, 0, 0, 0, 0),
                            name.encode('ascii').ljust(32, b'\0')))

    sx, sy = to_world(VILLAGE_ROAD_X, 106)
    deco('Party Start', sx, sy, 512)
    deco('south start', sx, sy, 512)

    blocked = set()
    for y in range(128):
        for x in range(128):
            if road[y][x] or tile[y][x] in (WATER,):
                for dy in (-1, 0, 1):
                    for dx in (-1, 0, 1):
                        blocked.add((x + dx, y + dy))
    for hx, hy, d in hobbit_holes():
        for dy in range(-3, 4):
            for dx in range(-3, 4):
                blocked.add((hx + dx, hy + dy))
        for k in (4, 5):
            for dx in range(-2, 3):
                blocked.add((hx + dx, hy + k * d))

    trees = ['tree13', 'tree16', 'tree22']  # Trees of Harmondale, tree33 and tree36 of Tatalia are snowy.
    forest = ['tree25', 'tree25', 'tree07', 'bush03']  # Tularean Forest.
    for y in range(18, 110):
        for x in range(18, 110):
            if (x, y) in blocked:
                continue
            old_forest = 20 <= x <= 46 and 22 <= y <= 56
            village = 36 <= x <= 100 and 76 <= y <= 108
            chance = 0.55 if old_forest else (0.04 if village else 0.10)
            if tile[y][x] == SWAMP:
                chance = 0.03
            if rnd.random() < chance:
                wx, wy = to_world(x, y)
                wx += rnd.randint(-200, 200)
                wy += rnd.randint(-200, 200)
                deco(rnd.choice(forest if old_forest else trees), wx, wy, rnd.randint(0, 2047))
            elif village and rnd.random() < 0.18:
                wx, wy = to_world(x, y)
                deco(rnd.choice(['flower01', 'flower03', 'flower05', 'flower06', 'bush03']), wx + rnd.randint(-200, 200),
                     wy + rnd.randint(-200, 200))
            elif rnd.random() < 0.01:
                wx, wy = to_world(x, y)
                deco(rnd.choice(['rock01', 'rock08', 'rock09']), wx, wy)

    # Decoration lookup grid used for collisions: every cell lists the decorations in it and its neighbours.
    cells = [[] for _ in range(128 * 128)]
    for i, (d, _) in enumerate(decorations):
        wx, wy = struct.unpack_from('<2i', d, 4)
        gx, gy = (wx >> 9) + 64, 63 - (wy >> 9)
        for dy in (-1, 0, 1):
            for dx in (-1, 0, 1):
                if 0 <= gx + dx < 128 and 0 <= gy + dy < 128:
                    cells[(gy + dy) * 128 + gx + dx].append(i)
    pid_list = [0]
    omap = [0] * (128 * 128)
    for c, ids in enumerate(cells):
        if ids:
            omap[c] = len(pid_list)
            pid_list += [(i << 3) | 5 for i in ids] + [0]

    # Monsters: skřeti around the village outskirts and the roads, pavouci in the Old Forest, trolls in the north.
    spawns = []

    def spawn(gx, gy, index):
        wx, wy = to_world(gx, gy)
        spawns.append(struct.pack('<3iHHHHI', wx, wy, ground(wx, wy), 32, 3, index, 0, 0))
    for gx, gy in ((30, 30), (38, 44), (26, 50), (42, 34), (32, 40)):
        spawn(gx, gy, 2)
    for gx, gy in ((56, 60), (78, 62), (50, 70), (86, 52), (60, 36), (70, 40), (46, 64)):
        spawn(gx, gy, 1)
    for gx, gy in ((84, 26), (92, 30), (54, 24), (76, 28)):
        spawn(gx, gy, 3)

    m = dict(template)
    m['name'] = b'kraj'.ljust(32, b'\0')
    m['fileName'] = b'out16.odm'.ljust(32, b'\0')
    m['heightMap'] = heights
    m['tileMap'] = tiles
    m['attributeMap'] = bytearray(128 * 128)
    m['someOtherMap'] = bytes(len(template['someOtherMap']))
    m['normalMap'] = bytes(len(template['normalMap']))
    m['normals'] = b''
    m['models'] = [entrance(i + 1, hx, hy, d, 32 * g)
                   for i, ((hx, hy, d), g) in enumerate(zip(hobbit_holes(), entrance_ground))]
    m['decorations'] = decorations
    m['decorationPidList'] = [struct.pack('<H', p) for p in pid_list]
    m['decorationMap'] = struct.pack(f'<{128 * 128}I', *omap)
    m['spawnPoints'] = spawns
    return m


def minimap(m):
    """Painted-looking 512x512 overview in the style of the original minimaps."""
    from PIL import Image, ImageDraw, ImageFilter
    import random as rnd_mod
    r = rnd_mod.Random(7)
    img = Image.new('RGB', (512, 512))
    px = img.load()
    hm, tm = m['heightMap'], m['tileMap']
    for y in range(512):
        for x in range(512):
            gx, gy = min(127, x // 4), min(127, y // 4)
            hgt = hm[gy * 128 + gx]
            t = tm[gy * 128 + gx]
            # Light from the north-west.
            hn = hm[max(0, gy - 1) * 128 + max(0, gx - 1)]
            shade = 1.0 + (hn - hgt) * -0.012
            if WATER <= t < SWAMP:
                c = (40, 70, 150)
            elif SWAMP <= t < ROAD:
                c = (70, 90, 60)
            elif t >= ROAD:
                c = (150, 130, 100)
            elif hgt > 120:
                c = (110, 100, 70)
            else:
                c = (70, 120, 45)
            k = shade * (0.85 + 0.3 * hgt / 255) * (0.93 + 0.14 * r.random())
            px[x, y] = tuple(max(0, min(255, int(v * k))) for v in c)
    d = ImageDraw.Draw(img)
    for dec, name in m['decorations']:
        if name.startswith(b'tree'):
            wx, wy = struct.unpack_from('<2i', dec, 4)
            x, y = (wx + 32768) / 128, (32768 - wy) / 128
            d.ellipse([x - 2, y - 2, x + 2, y + 2], fill=(35, 70, 30))
    for hx, hy, _ in hobbit_holes():
        d.ellipse([hx * 4 - 9, hy * 4 - 9, hx * 4 + 9, hy * 4 + 9], outline=(60, 100, 40))
    return img.filter(ImageFilter.SMOOTH)


def delta(m, template_ddm):
    attributes = [a for model in m['models'] for a in models.face_attributes(model)]
    header = bytes(16) + struct.pack('<3I', len(attributes), len(m['decorations']), len(m['models'])) + bytes(12)
    return {'header': header, 'fully': bytes(88 * 11), 'partially': bytes(88 * 11),
            'faceAttributes': struct.pack(f'<{len(attributes)}I', *attributes),
            'decorationFlags': bytes(2 * len(m['decorations'])), 'actors': [], 'spriteObjects': [], 'chests': [],
            'eventVariables': bytes(0xC8), 'lastVisitTime': bytes(8), 'weather': template_ddm['weather']}


MAPSTATS = (
    '\tMap Stats (Stín nad Krajem)\r\n'
    '\t\t\tReset\r\n'
    '#\tName\tFile name\r\n'
    '77\tKraj\tOut16.odm\t0\t0\t4\t672\t7\t3\t4\t3\t3\t10\t40\t30\t30\t'
    'Goblin\tGoblin\t3\t 2-4\tSpider\tSpider\t3\t 1-3\tTroll\tTroll\t3\t 1-2\t4\tFOREST\t0\tKraj\t\t\r\n'
)


def main():
    import lod
    games, out = sys.argv[1], sys.argv[2]
    entries = lod.entries(games)
    template = odm.read_odm(odm.unpack_lod_entry(entries['out01.odm'][1]))
    template_ddm = odm.read_ddm(odm.unpack_lod_entry(entries['out01.ddm'][1]), template)
    m = generate(template)
    files = {
        'out16.odm': odm.pack_lod_entry(odm.write_odm(m)),
        'out16.ddm': odm.pack_lod_entry(odm.write_ddm(delta(m, template_ddm))),
        'out16.evt': hobitin.door_events(),
        'out16.str': hobitin.LEVEL_STRINGS,
        'kmapstats.txt': MAPSTATS.encode('cp1250'),
        'out16': lodimg.encode('out16', minimap(m)),
        'k2devents.txt': hobitin.houses_table(),
        'knpcdata.txt': hobitin.npc_table(),
        'knpcgreet.txt': hobitin.greet_table(),
        'knpctopic.txt': hobitin.topic_table(),
        'knpctext.txt': hobitin.text_table(),
        'kglobal.evt': hobitin.global_events(),
    }
    for name, img in art.images().items():
        files[name] = lodimg.encode(name, img)
    lodw.write_lod(out, files)
    if len(sys.argv) > 3:
        import render
        render.render(m, sys.argv[3])
    print('decorations', len(m['decorations']), 'spawns', len(m['spawnPoints']))


if __name__ == '__main__':
    main()
