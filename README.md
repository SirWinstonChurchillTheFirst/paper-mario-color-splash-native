# Paper Mario Color Splash – native (recompiled) build scripts

Scripts and runtime patches that turn **your own** dump of *Paper Mario Color Splash* (Wii U, `main.rpx`) into a
native Linux/Windows program with a static recompiler. No game code, game data or Nintendo system files are in this
repository; you need your own dump (and the country/language list title 0005001B-1005C000 of your own Wii U, given
with `CAFE_SYS_DIR`).

Base: [Smash4PC](https://github.com/josiahg2502-glitch/Smash4PC) (MIT; built on Petexy's cafe-recomp). The
`port/patch_*.py` scripts are the local changes one by one (each script's header says what it fixes).

## What this title needed

- **Heap headers** (`port/os_extra.cpp`, `port/patch_heap_headers.py`): the game finds the heap that owns a pointer
  from the MEMHeapHeader data range itself. Unit and block heaps lacked it, so `free()` silently leaked, object
  handles stayed valid after `delete` and the game crashed after "A" on the title.
- **H.264 decoder** (`port/os_extra.cpp`, `port/patch_h264.py`): H264DEC* as the console has it. Pictures come from
  the system's FFmpeg (libavcodec 58-61, loaded at run time); without it frames are black. `CAFE_H264=0` = old stubs.
- More: `patch_hfio_sys.py`, `patch_heap_more.py`, `patch_fs_cwd.py`, a hand-written jump table as extra entries
  (`port/colorsplash-extra-entries.txt`, `env $(cat ...) cafe-recomp ... --all`).

## State

Title, opening movie, Port Prisma, dialogue and walking run. Console (Batocera, Ryzen 3 4350GE / Vega 6, 1280x720):
steady 30 fps = the game's own rate (with `TTT2_GUEST_DMABUF=1`, see `console/colorsplash.game.env`). Battles and
saving were not tested yet.
