import struct
class PE:
    def __init__(s, path):
        s.d = open(path, 'rb').read()
        pe = struct.unpack_from('<I', s.d, 0x3c)[0]
        n = struct.unpack_from('<H', s.d, pe + 6)[0]
        opt = struct.unpack_from('<H', s.d, pe + 20)[0]
        s.base = struct.unpack_from('<I', s.d, pe + 24 + 28)[0]
        s.secs = []
        o = pe + 24 + opt
        for i in range(n):
            name, vsize, va, rsize, raw = struct.unpack_from('<8sIIII', s.d, o + 40 * i)
            s.secs.append((va + s.base, vsize, raw, rsize))
    def off(s, a):
        for va, vs, raw, rs in s.secs:
            if va <= a < va + max(vs, rs):
                return raw + a - va
        raise KeyError(hex(a))
    def u32(s, a): return struct.unpack_from('<I', s.d, s.off(a))[0]
    def u8(s, a): return s.d[s.off(a)]
