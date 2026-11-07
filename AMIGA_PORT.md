Nikwi Deluxe for AmigaOS 3.x
============================

Nikwi Deluxe (Kostas Michalopoulos, Slashstone), the 2D platform game:
guide Nikwi through 30 levels of snow, sand and sweets, collecting the
candies and keeping clear of the creatures, with passwords to go back to a
level. For AmigaOS 3.x on a 68040 or 68060 with an FPU and an RTG screen,
on OpenGPU's SDL 1.2 (sdl12-compat).

Base: nikwi_0.0.20120213.orig.tar.xz from Debian (sha256
8f08ebd487f03d0d0bc30de0eb7bdf27a52034b200054c8f12706165af42d327), with
Debian's patches (0.0.20120213-8; among them the big-endian fixes, which the
68k needs) as the second commit. Branch team2/nikwi-amiga.

Status: plays in the lab (Instance-45 on GFOpenUp 0.8.19, 6 November
2026): the menu over its moving demo level, New Game, level 1 (Nikwi, the
platforms, the candies and creatures), Escape back to the menu, and Quit
back to the Shell. Screenshots from that run are in amiga/screenshots
(nikwi-menu.png, nikwi-level1.png). The lab is muted, so the music and
sounds were not heard there.


What you need
-------------

- A 68040 or 68060 with an FPU, AmigaOS 3.x, an RTG card (or the
  AmigaChrome lab).
- GFOpenUp 0.8.18 or later (LIBS:OpenGPU/SDL2.module). Everything else is
  in the program.


How to play
-----------

Open the Nikwi icon, or run Nikwi from a Shell. It opens a 640x480 window
(Nikwi --fullscreen for the full screen, --nosound for none). The cursor
keys and Return choose in the menu; in the game the cursor keys run and
Up jumps, Escape goes back to the menu. Password on the menu takes you
back to a level you reached. ReadMe-upstream in the drawer is upstream's
readme, manual its manual (HTML). The whole game is the drawer's
justdata.up.


Building
--------

    make -f amiga/Makefile.amigaos -j2 dist

with GFOpenAmigaGCC 16.2.0-open4, the OpenNDK headers, the OpenGPU kit, a
host C compiler and Python 3 (see the Makefile's head). The drawer is
dist/Nikwi (and dist/Nikwi.info).


What was changed
----------------

- src/nikwi/nikio.cpp: the game's pack is PROGDIR:justdata.up (upstream
  joins its directory and the name with "/", and "PROGDIR:/" would be the
  parent drawer to AmigaDOS).
- amiga/Makefile.amigaos builds src/Makefile.debian's nikwi and its four
  libraries (slashfx, badcfg, slashtdp, us), and makes justdata.up as
  Debian's makedata.sh and makepack.sh make it: the pictures converted to
  Nikwi's UT format by amiga/tools/bmp2ut.py (src/tools/bmp2ut's conversion
  without the host's SDL), then src/tools/upack built for the host and run.
- amiga/amiga_prefix.hpp: the C++ streams first in every file.
- amiga/stack.c: 256 KB of stack.


Licences
--------

Nikwi's program (src/): zlib licence. Its pictures, levels, sounds, music
and everything else: WTFPL (COPYING.txt, upstream's license.txt). The
Amiga files (amiga/): the AmiCradle Team, zlib licence; the icon is
upstream's nikwi.ico.
