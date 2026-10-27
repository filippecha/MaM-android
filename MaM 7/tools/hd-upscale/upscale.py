"""Runs inside the mm7-upscale-gpu container. Upscales the PNGs of /work/orig with an ESRGAN-family model (loaded
through spandrel) into /work/hd-<model>/, textures padded with their own wrapped-around edges so that they stay
seamless. Colour-keyed textures (RGBA input) get their alpha upscaled separately and thresholded.
Usage: upscale.py <model name>  (names of MODELS below). Env: OUT_SCALE=4 (2 averages the 4x result down)."""
import os
import sys

import cv2
import numpy as np
import torch
from spandrel import ModelLoader

MODELS = {
    'x4plus': 'https://github.com/xinntao/Real-ESRGAN/releases/download/v0.1.0/RealESRGAN_x4plus.pth',
    'remacri': 'https://huggingface.co/FacehugmanIII/4x_foolhardy_Remacri/resolve/main/4x_foolhardy_Remacri.pth',
}
PAD = 16  # Source pixels of padding, enough for the model's receptive field.
OUT_SCALE = int(os.environ.get('OUT_SCALE', '4'))
# PAD_MODE=wrap for seamless textures (the opposite edge continues the picture), reflect for everything else.
PAD_MODE = cv2.BORDER_REFLECT if os.environ.get('PAD_MODE', 'wrap') == 'reflect' else cv2.BORDER_WRAP
DEVICE = 'cuda' if torch.cuda.is_available() else 'cpu'


def fetch(name):
    path = f'/work/weights/{name}.pth'
    if not os.path.exists(path):
        import urllib.request
        os.makedirs('/work/weights', exist_ok=True)
        print('downloading', name, flush=True)
        urllib.request.urlretrieve(MODELS[name], path)
    return path


def run(model, img):
    """img: HxWx3 BGR uint8 -> upscaled BGR uint8."""
    t = torch.from_numpy(img[:, :, ::-1].astype(np.float32) / 255.0).permute(2, 0, 1).unsqueeze(0).to(DEVICE)
    with torch.no_grad():
        out = model(t)
    out = out.squeeze(0).permute(1, 2, 0).clamp(0, 1).cpu().numpy()
    return (out[:, :, ::-1] * 255.0 + 0.5).astype(np.uint8)


TILE = 256  # Big UI screens are run in tiles, the whole 640x480 at once does not fit the GPU.


def run_tiles(model, img):
    """Runs the model on overlapping tiles and stitches the results, same output as one run on a small image."""
    h, w = img.shape[:2]
    if h * w <= TILE * TILE:
        return run(model, img)
    scale = None
    out = None
    for y0 in range(0, h, TILE):
        for x0 in range(0, w, TILE):
            y1, x1 = min(h, y0 + TILE), min(w, x0 + TILE)
            sy0, sx0 = max(0, y0 - PAD), max(0, x0 - PAD)
            sy1, sx1 = min(h, y1 + PAD), min(w, x1 + PAD)
            piece = run(model, img[sy0:sy1, sx0:sx1])
            if scale is None:
                scale = piece.shape[0] // (sy1 - sy0)
                out = np.zeros((h * scale, w * scale, 3), dtype=np.uint8)
            oy, ox = (y0 - sy0) * scale, (x0 - sx0) * scale
            out[y0 * scale:y1 * scale, x0 * scale:x1 * scale] = piece[oy:oy + (y1 - y0) * scale, ox:ox + (x1 - x0) * scale]
    return out


def upscale_tiled(model, img):
    h, w = img.shape[:2]
    padded = cv2.copyMakeBorder(img, PAD, PAD, PAD, PAD, PAD_MODE)
    out = run_tiles(model, padded)
    scale = out.shape[0] // (h + 2 * PAD)
    out = out[scale * PAD:scale * (PAD + h), scale * PAD:scale * (PAD + w)]
    if OUT_SCALE < scale:
        out = cv2.resize(out, (w * OUT_SCALE, h * OUT_SCALE), interpolation=cv2.INTER_AREA)
    return out


name = sys.argv[1]
model = ModelLoader().load_from_file(fetch(name)).model.eval().to(DEVICE)
print('device', DEVICE, flush=True)
out_dir = f'/work/hd-{name}'
os.makedirs(out_dir, exist_ok=True)
for file in sorted(os.listdir('/work/orig')):
    if not file.lower().endswith('.png'):
        continue
    target = os.path.join(out_dir, file)
    if os.path.exists(target):
        continue
    img = cv2.imread(os.path.join('/work/orig', file), cv2.IMREAD_UNCHANGED)
    if img is None:
        continue
    if img.ndim == 2:
        img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
    alpha = None
    if img.shape[2] == 4:
        alpha = img[:, :, 3]
        img = img[:, :, :3].copy()
        mask = (alpha == 0).astype(np.uint8)
        if mask.any():
            img = cv2.inpaint(img, mask, 3, cv2.INPAINT_TELEA)  # The hidden colour must not bleed into the edges.
    out = upscale_tiled(model, img)
    if alpha is not None:
        a = upscale_tiled(model, cv2.cvtColor(alpha, cv2.COLOR_GRAY2BGR))[:, :, 0]
        a = np.where(a >= 128, 255, 0).astype(np.uint8)
        out = cv2.merge([out[:, :, 0], out[:, :, 1], out[:, :, 2], a])
    cv2.imwrite(target, out)
    print(name, file, img.shape[1], 'x', img.shape[0], '->', out.shape[1], 'x', out.shape[0], flush=True)
