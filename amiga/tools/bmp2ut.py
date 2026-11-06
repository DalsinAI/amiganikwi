#!/usr/bin/env python3
"""bmp2ut.py: src/tools/bmp2ut's conversion (bmp2ut --halfbits -i FILE)
without the host's SDL 1.2: a 24-bit BMP to Nikwi's UT picture, "UT", a
flags byte (1: half bits), width and height (16-bit little-endian), then
the pixels as RGB565, little-endian. As bmp2ut reads SDL_LoadBMP's surface
from the top row, w*h pixels straight through (its rows padded to a 4-byte
pitch), so does this. makedata.sh's job, in amiga/Makefile.amigaos's dist.
The AmiCradle Team. zlib licence, like Nikwi's tools."""
import struct, sys

def convert(src, dst):
    d = open(src, 'rb').read()
    if d[:2] != b'BM':
        raise SystemExit(f'{src}: not a BMP')
    off = struct.unpack_from('<I', d, 10)[0]
    w, h = struct.unpack_from('<ii', d, 18)
    bpp = struct.unpack_from('<H', d, 28)[0]
    if bpp != 24:
        raise SystemExit(f'{src}: only 24-bit pixels are supported')
    top_down = h < 0
    h = abs(h)
    pitch = (w * 3 + 3) & ~3
    rows = [d[off + r * pitch: off + (r + 1) * pitch] for r in range(h)]
    if not top_down:
        rows.reverse()
    pix = b''.join(rows)            # SDL's surface: top row first, padded rows
    out = bytearray(b'UT' + b'\0' + bytes([1]) + struct.pack('<HH', w, h))
    for i in range(w * h):
        b, g, r = pix[3 * i], pix[3 * i + 1], pix[3 * i + 2]
        out += struct.pack('<H', (b >> 3) | ((g >> 2) << 5) | ((r >> 3) << 11))
    open(dst, 'wb').write(out)

for a in sys.argv[1:]:
    convert(a, a[:a.rindex('.')] + '.ut')
