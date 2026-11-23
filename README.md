# Paper Mario Color Splash – native (recompiled) build scripts

Scripts and runtime patches that turn **your own** dump of *Paper Mario Color Splash* (Wii U, `main.rpx`) into a
native Linux/Windows program with a static recompiler. No game code, game data or Nintendo system files are in this
repository; you need your own dump (and the country/language list title 0005001B-1005C000 of your own Wii U, given
with `CAFE_SYS_DIR`).

Base: [Smash4PC](https://github.com/josiahg2502-glitch/Smash4PC) (MIT; built on Petexy's cafe-recomp). The
`port/patch_*.py` scripts are the local changes one by one (each script's header says what it fixes).

## Selbst bauen / Build it yourself

One command builds the native program from **your own dump** of Paper Mario: Color Splash. Nothing from the game is in this repository
or downloaded by the scripts: the game's code (main.rpx) is translated to C++ and compiled on your computer.
*Ein Befehl baut das native Programm aus deinem eigenen Dump; nichts vom Spiel ist hier enthalten oder wird geladen.*

**You need / Du brauchst**
- your own dump of the game, **unpacked**: a folder with `code/`, `content/`, `meta/` (as Cemu or a Wii U dumper
  such as Dumpling gives it; `.wua`/`.wud` images must be unpacked first). Tested with Europe
  (000500001F600B00); other versions are translated from your files the same way and may work.
- Windows 10/11 64-bit or Linux x86-64; a CPU with AVX2/FMA (Intel Haswell, AMD Zen or newer); a GPU with a Vulkan driver.
- internet for the build tools and the pinned sources (Smash4PC `f145e7f`, ZArchive, addrlib, Vulkan-Headers from GitHub).
- about **2 GB** free in this folder (plus about 3 GB for MSYS2 on Windows) and 8 GB of RAM or more.
- time: about **4-10 minutes** for the first build on an 8-core PC; a second run only redoes what changed.

**System files.** This game needs the country/language list title (`0005001B-1005C000`) of **your own Wii U** (not part of the game, not included, never downloaded): give a Cemu-style `mlc01/sys` folder holding `title/0005001b/1005c000/` with `-SysDir` (Windows) or `--sys` (Linux). *Deutsch:* Gebraucht wird die Laender-/Sprachliste deiner eigenen Wii U (`-SysDir` / `--sys`).

**Windows** (PowerShell):

```powershell
git clone https://github.com/SirWinstonChurchillTheFirst/paper-mario-color-splash-native
cd paper-mario-color-splash-native
powershell -ExecutionPolicy Bypass -File .\setup-windows.ps1 -GameDir "D:\Games\Paper Mario: Color Splash" -SysDir "D:\Cemu\mlc01\sys"
.\play\Start.bat
```

It installs [MSYS2](https://www.msys2.org) with winget if it is missing and the build packages into its CLANG64
environment with pacman (clang, lld, cmake, ninja, Python, git, SDL3, Vulkan loader + headers, shaderc, zlib, zstd,
xz, DejaVu fonts). Without `-GameDir` it asks for the folder. `-Jobs N` limits the parallel compile jobs; the build
runs at low priority so the PC stays usable.

**Linux** (Debian/Ubuntu, Arch, Fedora; others: install the same packages yourself):

```bash
git clone https://github.com/SirWinstonChurchillTheFirst/paper-mario-color-splash-native
cd paper-mario-color-splash-native
./setup-linux.sh "/path/to/Paper Mario: Color Splash" --sys ~/.local/share/Cemu/mlc01/sys
./play/start.sh
```

Missing packages are installed with sudo (apt: clang cmake ninja-build git python3 pkg-config zlib1g-dev libzstd-dev
libssl-dev libvulkan-dev libshaderc-dev fonts-dejavu-core, and libsdl3-dev where the distribution has it; without it
SDL 3.2.26 is built from source, e.g. on Ubuntu 24.04). `--jobs N` limits the compile jobs.

**What the setup does** (`tools/native-build.sh`, the same on both systems): checks your dump; clones
[Smash4PC](https://github.com/josiahg2502-glitch/Smash4PC) at `f145e7f` and the three pinned dependencies; applies
`port/upstream-local.patch` (all runtime/recompiler changes, made by the `port/patch_*.py` scripts in order); builds the
recompiler `cafe-recomp`; translates your `code/`; compiles runtime + translated game; and puts `cafegame`, a stand-in
font and the starter into `play/`. Everything it makes stays in `build/` and `play/` (both ignored by git): they hold
code translated from the game — **keep them for yourself, do not share them.**

The starter sets `CAFE_TOUCH_UI=1` for this game (see the comments in `game.conf`); any of them can be overridden by setting it before starting.

The Wii U system font is not part of a game dump: DejaVu Sans stands in (`play/fonts/CafeStd.ttf`); put your console's
own `CafeStd.ttf` there to replace it. Saves go to `play/save/`. The system language follows your PC
(`CAFE_LANGUAGE=de|en|fr|...` overrides it).

**Getestet / tested (23 Nov 2026):** For this game the Windows setup's translation step was run on the tested dump: it gives the same code as the build that runs on the Batocera console (only the newer recompiler's `dcbf` lines differ); the complete build of this game through the scripts was not run yet. The same scripts were run end to end from an empty folder on 23 November 2026: Lost Levels and Mario Kart 8 on Windows 11, Lost Levels on Ubuntu 26.04 and on a fresh Ubuntu 24.04 (SDL3 built from source there), Mario Party 10 on Windows 11.

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
