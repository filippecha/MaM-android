"""The four character buff icons next to the portraits (isg-01..04: Bless, Preservation, Hammerhands, Pain Reflection,
one set per frame a/b/c) without their background square, with the object's drop shadow: python isg_alpha.py <icons.lod> <hd icons dir>.

The originals are opaque 17x17 pictures cut out of the bottom frame at the third portrait, with the object's shadow
painted on that stone (ice, dark) background. On the other portraits the square doesn't match the frame. Without the
shadow the icons looked odd to the user, so it stays. Writes isg-0K-F.png (RGBA) next to the upscaled JPEGs, the engine prefers PNG and keeps its
alpha (applyHdIcon).

1. Original pixel by pixel against the frame (ib-b-F.pcx) at the place it was cut from: equal = background, a darker
   copy of the frame = shadow, the rest = object. The dark hammer handle and the striped disc look like shadow in some
   frames, the frames a and b have the same object 1 px apart, so a takes b's mask for icons 1-3 and b takes a's disc.
2. Object | shadow (everything but the background) upscaled the same way, joined with the object mask below.
3. The mask upscaled to the HD icon, its edge band decided by colour (nearest cluster of sure object vs. background).
   The disc is the convex hull of its blue pixels plus the golden rim, the hammer keeps the upscaled outline in a and c.
"""
import os
import struct
import sys
import zlib
import io

import cv2
import numpy as np
from PIL import Image
from scipy import ndimage as nd

# Where the icons were cut out of the frames, at icon 1 (each next icon is 17 rows lower).
CUT = {'a': (338, 42), 'b': (339, 41), 'c': (339, 41)}


def read_lod(path):
    d = open(path, 'rb').read()
    _, off, _, _, count, _ = struct.unpack_from('<16sIIIHH', d, 256)
    out = {}
    for i in range(count):
        nm, o, s, _, _, _ = struct.unpack_from('<16sIIIHH', d, off + 32 * i)
        out[nm.split(b'\0')[0].decode('latin-1').lower()] = d[off + o:off + o + s]
    return out


def lod_entry(b):
    _, _, dsize, w, h = struct.unpack_from('<16sIIhh', b, 0)
    unpacked = struct.unpack_from('<I', b, 40)[0]
    data = b[48:48 + dsize]
    if unpacked:
        data = zlib.decompress(data)
    return data, b[48 + dsize:48 + dsize + 768], w, h


def lod_image(b):
    data, pal, w, h = lod_entry(b)
    pal = np.frombuffer(pal, np.uint8).reshape(256, 3)
    return pal[np.frombuffer(data[:w * h], np.uint8)].reshape(h, w, 3).astype(np.float32)


def lod_pcx(b):
    data, _, _, _ = lod_entry(b)
    return np.asarray(Image.open(io.BytesIO(data)).convert('RGB')).astype(np.float32)


def clean(m, frac=0.1):
    m = nd.binary_fill_holes(m)
    lab, n = nd.label(m, np.ones((3, 3)))
    if n > 1:
        sizes = nd.sum(m, lab, range(1, n + 1))
        m = np.isin(lab, [i + 1 for i, v in enumerate(sizes) if v >= frac * sizes.max()])
    return m


def mask_1x(icon, bg):
    """(object, object with its shadow)"""
    m = np.zeros(icon.shape[:2], bool)
    full = np.zeros(icon.shape[:2], bool)
    for y in range(icon.shape[0]):
        for x in range(icon.shape[1]):
            p, q = icon[y, x], bg[y, x]
            if np.abs(p - q).sum() < 24:
                continue
            full[y, x] = True
            ratio = (p.sum() + 1) / (q.sum() + 1)
            m[y, x] = not (ratio < 0.85 and np.abs(p - ratio * q).sum() < 40)
    return clean(m), clean(full, 0.3)


def centroids(x, k):
    _, _, c = cv2.kmeans(x, k, None, (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 20, 1.0), 3,
                         cv2.KMEANS_PP_CENTERS)
    return c


def mask_hd(img, m1, k, f):
    s = img.shape[0] // m1.shape[0]
    m = cv2.resize(m1.astype(np.float32), (img.shape[1], img.shape[0]), interpolation=cv2.INTER_LINEAR)
    cf, cb = centroids(img[m > 0.97], 6), centroids(img[m < 0.03], 8)
    df = np.min(np.linalg.norm(img[:, :, None] - cf[None, None], axis=3), axis=2)
    db = np.min(np.linalg.norm(img[:, :, None] - cb[None, None], axis=3), axis=2)
    a = (m >= 0.97) | ((m > 0.03) & (m < 0.97) & (df < db))
    if k == 3 and f != 'b':
        a = m > 0.5  # The dark handle looks like the shadow, keep the original outline.
    if k == 4:
        r, b = img[..., 0], img[..., 2]
        blue = (b > r + 25) & (b > 110) & (m > 0.03)
        hull = cv2.convexHull(np.argwhere(blue)[:, ::-1].astype(np.int32))
        disc = np.zeros(m.shape, np.uint8)
        cv2.fillConvexPoly(disc, hull, 1)
        a = nd.binary_dilation(disc.astype(bool), iterations=s // 2 + 1)
    return clean(nd.binary_opening(a, np.ones((2, 2))))


def shadow_hd(img, m1):
    m = cv2.resize(m1.astype(np.float32), (img.shape[1], img.shape[0]), interpolation=cv2.INTER_LINEAR)
    cf, cb = centroids(img[m > 0.97], 8), centroids(img[m < 0.03], 8)
    df = np.min(np.linalg.norm(img[:, :, None] - cf[None, None], axis=3), axis=2)
    db = np.min(np.linalg.norm(img[:, :, None] - cb[None, None], axis=3), axis=2)
    a = (m >= 0.97) | ((m > 0.03) & (m < 0.97) & (df < db))
    return clean(nd.binary_opening(a, np.ones((2, 2))), 0.3)


def main():
    lod = read_lod(sys.argv[1])
    hd_dir = sys.argv[2]
    masks, shadows = {}, {}
    for f in 'abc':
        frame = lod_pcx(lod[f'ib-b-{f}.pcx'])
        x0, y0 = CUT[f]
        for k in range(1, 5):
            y = y0 + 17 * (k - 1)
            masks[f, k], shadows[f, k] = mask_1x(lod_image(lod[f'isg-0{k}-{f}']), frame[y:y + 17, x0:x0 + 17])
    for k in (1, 2, 3):
        masks['a', k] = np.roll(masks['b', k], 1, 1)
    masks['b', 4] = np.roll(masks['a', 4], -1, 1)
    for f in 'abc':
        for k in range(1, 5):
            name = f'isg-0{k}-{f}'
            img = np.asarray(Image.open(os.path.join(hd_dir, name + '.jpg')).convert('RGB')).astype(np.float32)
            a = mask_hd(img, masks[f, k], k, f)
            # The user wanted the shadow back: the object with its shadow, only the background square goes away.
            a = clean(a | shadow_hd(img, shadows[f, k]))
            # Transparent pixels take the colour of the nearest object pixel, the downscale doesn't darken the edge.
            _, (iy, ix) = nd.distance_transform_edt(~a, return_indices=True)
            rgb = img[iy, ix].astype(np.uint8)
            rgba = np.dstack([rgb, np.where(a, 255, 0).astype(np.uint8)])
            Image.fromarray(rgba, 'RGBA').save(os.path.join(hd_dir, name + '.png'), optimize=True)
            print(name, int(a.sum()))


if __name__ == '__main__':
    main()
