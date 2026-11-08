# AGENTS.md — PS1 PsyQ Project Context
**Maintenance rule:** Any agent working in this repo MUST update this file when project structure, build config, memory layout, available libraries, asset pipelines, or conventions change. Keep it accurate and current. If you add a new source file, library, asset type, or change the build system, reflect that here before finishing.

Target: MIPS R3000, little-endian, 32-bit. SDK: PsyQ 4.7. Emulator: pcsx-redux.
Compiler: `mipsel-none-elf-gcc`. No stdlib. Freestanding.

## Build

```bash
cd project-example
make fix-headers   # ONE-TIME: copies patched headers into nugget/psyq/include
make               # → projectname.elf → projectname.ps-exe
make image         # → also .bin + .cue (CD ISO)
make play          # → launches pcsx-redux with the ISO
make run           # → launches pcsx-redux with the .ps-exe directly (no ISO)
make clean         # removes generated files
```

Add source: drop a `.c` file in `project-example/src/` — the Makefile auto-globs `src/*.c`.
Add PsyQ lib: append `-l<name>` to `LDFLAGS` in the Makefile.

Linker script: `nugget/ps-exe.ld` (PS-EXE) or `nugget/cpe.ld` (CPE).

**mkpsxiso:** `make image` requires mkpsxiso **v2.30+** on PATH (older
versions, e.g. 2.03 from PSn00bSDK, can't decode FLAC audio). Download from
[Lameguy64/mkpsxiso releases](https://github.com/Lameguy64/mkpsxiso/releases).
The helper must be the current upstream
[OrthoMatrix/mkpsxiso-helper](https://codeberg.org/OrthoMatrix/mkpsxiso-helper) —
v2.30 rejects a `trackid` on the `<track type="audio">` element (it belongs only
on the DA `<file>` entry). Audio files in `project-example/AUDIO/` are packed as
CD-DA tracks (WAV, FLAC, PCM, MP3).

## File Map (only what matters)

```
project-example/Makefile          # Build config. SRCS, LDFLAGS, TYPE
project-example/src/main.c        # Entry, main loop, CD load
project-example/src/display.h     # Display types (DoubleBuffer), globals, init/update decls
project-example/src/display.c     # Display init/update (double buffering), debug font
project-example/src/input.h       # init_input/update_input/button_pressed decls (uses libetc.h PAD* defines)
project-example/src/input.c       # Pad init + per-frame PadRead, button_pressed()
project-example/src/enemy.h       # Enemy struct, globals, init/draw decls
project-example/src/player.h      # Player struct (Object3D cube), globals, init/update/draw decls
project-example/src/3d.h          # 3D module: Shape/Object3D/ColorPalette types, init_3d/draw_3d_object/set_3d_scale/set_3d_palette/flash_3d_object/update_3d_flash decls, 7 predefined shapes
project-example/src/3d.c          # 3D module: GTE transform + projection, shape geometry (cube, octahedron, icosahedron, pyramid, cone, torus, diamond)
project-example/src/palette.h     # Named ColorPalette constants (diamond, emerald, ruby, sapphire, gold, neon, ice, steel)
project-example/src/enemy.c        # Enemy logic (red tile draw), circle collision vs player bullets (threshold 50)
project-example/src/player.c      # Player: 3D rotating icosahedron (via 3d.h), friction fixed-point (16.16) movement, bullet firing (split-bullet powerup via split_timer)
project-example/src/bullet.h      # Bullet struct, player/enemy bullet arrays, init/update/draw/fire_*/*_bullet_collides (bool, kills on hit) decls
project-example/src/bullet.c      # Dual bullet pools (player=yellow square→right, enemy=red→left), spawn/update/cull/draw, kill-on-hit circle collision; fire_player_bullet_dir for angled (split) shots
project-example/src/powerup.h     # Powerup struct (neon torus), pool, init/clear/update/draw/spawn_powerup decls
project-example/src/powerup.c     # Powerup pool: neon torus drifts left, spins, scale-pulses, collides with player → grants timed split-bullet powerup + plays SFX
project-example/src/sfx.h         # SFX module: SFX_ID enum, init_sfx/play_sfx/play_sfx_pitch/stop_sfx decls
project-example/src/sfx.c         # SFX module: loads VAG (4-bit ADPCM) from CD → SPU RAM, voice setup, key on/off
project-example/src/cdaudio.h     # CD-DA track playback (play_track)
project-example/src/cdaudio.c     # CdPlay wrapper for background music tracks
project-example/src/screens.h      # Game flow state machine: ScreenState enum (START/PLAYING/GAME_OVER), init_screens/update_screens/draw_screens decls
project-example/src/screens.c      # Start screen (FntPrint), game-over screen, START button to begin/restart; gates game updates in main loop
project-example/demos/            # Standalone example programs (own main(), NOT built by the Makefile)
project-example/DATA/             # CD assets (TIM files go here)
project-example/assets/           # Source art (pre-conversion)
project-example/tools/            # Dev tools (bin/psxavenc.exe — WAV→VAG ADPCM encoder, prebuilt Windows binary)
nugget/common.mk                  # Shared build rules (don't edit)
nugget/psyq/include/              # SDK headers (libgpu.h, libgs.h, libgte.h, etc.)
nugget/psyq/lib/                  # Prebuilt .a libs
nugget/common/crt0/crt0.s         # Startup (already in SRCS)
nugget/ps-exe.ld                  # Linker script
mkpsxiso-helper/mkpsxiso-helper.py # ISO generator (called by make image)
```

## Memory Layout

```
0x80000000  RAM start (2 MB total)
0x80010000  Program link address
            ...text/data...
HEAP_START  = STACK_END - 1MB     (heap, 1 MB)
STACK_END   = 0x801FFF00 - 32KB   (stack, 32 KB)
0x801FFFFF  RAM end
```

VRAM: 1 MB, 1024×512 16-bit pixels. Separate from RAM.
Scratchpad: 8 KB. SPU RAM: 512 KB.

## Display Init (already in main.c)

```c
ResetGraph(0);
SetDefDispEnv(&db[0].disp, 0, 0, 320, 240);
SetDefDispEnv(&db[1].disp, 0, 240, 320, 240);
SetDefDrawEnv(&db[0].draw, 0, 240, 320, 240);
SetDefDrawEnv(&db[1].draw, 0, 0, 320, 240);
setRGB0(&db[n].draw, r, g, b);  // bg clear color
db[n].draw.isbg = 1;            // auto-clear
PutDispEnv(&db[0].disp);
PutDrawEnv(&db[0].draw);
FntOpen(16, 16, 288, 64, 0, 512);
FntLoad(960, 256);
```

## Per-Frame Loop

```c
DrawSync(0);
VSync(0);
db_idx = !db_idx;
PutDispEnv(&db[db_idx].disp);
PutDrawEnv(&db[db_idx].draw);
// ... fill OT with prims ...
DrawOTag(&db[db_idx].ot[OTSIZE - 1]); // OTag at last slot; ClearOTagR chains all slots
SetDispMask(1);
```

OT: `u_long ot[1024]`. Depth 0=front, 1023=back. Insert: `addPrim(ot+depth, &prim)`.
`ClearOTagR(ot, N)` chains slots N→N-1→…→0 via `next` pointers, so a single
`DrawOTag(&ot[N-1])` draws the whole OT far-to-near. (Plain `ClearOTag` just zeroes slots.)

## 2D Primitives (libgpu.h)

### Solid shapes (no texture)

```c
TILE tile;
setTile(&tile);           // setlen(3), code 0x60
setXY0(&tile, x, y);
setWH(&tile, w, h);
setRGB0(&tile, r, g, b);
addPrim(ot + depth, &tile);
```

Also: `TILE_16` (16×16), `TILE_8` (8×8), `TILE_1` (1×1).
Lines: `LINE_F2` (2pt flat), `LINE_G2` (2pt gouraud), `LINE_F3`, `LINE_F4`.
Polys: `POLY_F3`, `POLY_F4` (flat), `POLY_G3`, `POLY_G4` (gouraud).

### Sprites (textured rect)

```c
SPRT sprt;
setSprt(&sprt);           // setlen(4), code 0x64
setXY0(&sprt, x, y);
setWH(&sprt, w, h);
setUV0(&sprt, u, v);      // offset into texture page
setClut(&sprt, getClut(clut_x, clut_y));  // 4/8-bit only
// SPRT has NO tpage field — must emit DR_TPAGE before it in OT:
DR_TPAGE tpage_prim;
setDrawTPage(&tpage_prim, 0, 0, getTPage(depth_bits, 0, vram_x, vram_y));
addPrim(ot + depth, &tpage_prim);
addPrim(ot + depth, &sprt);
```

Also: `SPRT_16` (16×16, no w/h), `SPRT_8` (8×8, no w/h).

### Textured polys (arbitrary quads)

```c
POLY_FT4 poly;
setPolyFT4(&poly);
setXY4(&poly, x0,y0, x1,y1, x2,y2, x3,y3);
setUV4(&poly, u0,v0, u1,v1, u2,v2, u3,v3);
setTPage(&poly, getTPage(depth, abr, vram_x, vram_y), 0, 0);
setClut(&poly, getClut(x, y));
setRGB0(&poly, r, g, b);
addPrim(ot + depth, &poly);
```

### Higher-level (libgs.h) — rotation/scale/flip without manual math

```c
GsSPRITE sp;
sp.x = x; sp.y = y; sp.w = w; sp.h = h;
sp.tpage = getTPage(depth, 0, vram_x, vram_y);
sp.u = u; sp.v = v;
sp.rotate = 0;            // fixed-point, 0 = no rotation
sp.scalex = 0x10000;     // 1.0
sp.scaley = 0x10000;
sp.r = 255; sp.g = 255; sp.b = 255;
GsSortSprite(&sp, ot, depth);
```

`GsSortBoxFill(GsBOXF*)`, `GsSortLine(GsLINE*)`, `GsSortGLine(GsGLINE*)` for rects/lines.
`GsSortBg(GsBG*, GsMAP*)` for scrolling tilemaps.

## Texture System

| Depth | Max size | Width align | CLUT |
|-------|----------|-------------|------|
| 4-bit | 256×256 | mult of 4 | 16 colors (getClut) |
| 8-bit | 256×256 | mult of 2 | 256 colors (getClut) |
| 16-bit | 256×256 | any | none |

`getTPage(tp, abr, x, y)`: tp=0(4bit)/1(8bit)/2(16bit), abr=blend(0-3), x/y=VRAM coords.
`getClut(x, y)`: x must be mult of 16.

### Loading a TIM into VRAM

```c
u_long size = load_file("MYIMAGE.TIM");  // reads CD → load_buffer
if (size) {
    LoadTImg((GsIMAGE *)load_buffer);    // uploads to VRAM at TIM's x,y coords
}
```

TIM files go in `project-example/DATA/`. Convert with `img2tim` or `timedit`.

## 3D (src/3d.h) — 3D rendering module

The `3d.h`/`3d.c` module wraps all GTE transform + projection code.
Provides 7 predefined shapes (cube, octahedron, icosahedron, pyramid, cone, torus, diamond)
with a simple object-based API. Each object carries a `ColorPalette` (one color per face);
named palettes live in `palette.h`.

```c
#include "palette.h"

// Init once at startup (sets GTE geometry: origin offset, screen depth)
init_3d(SCREEN_W, SCREEN_H, SCREEN_Z);

// Create an object (z defaults to SCREEN_Z = 512 → 1:1 screen mapping)
Object3D obj;
init_3d_object(&obj, SHAPE_DIAMOND, 160, 120);

// Per-face colors (optional; init defaults to flat white)
set_3d_palette(&obj, &palette_diamond);

// Per-frame: update transform, then draw
rotate_3d_object(&obj, 0, 16, 16);
move_3d_object(&obj, player.x, player.y);
draw_3d_object(&obj, db[db_idx].ot, OTSIZE);

// Scale (fixed-point 16.16: 0x10000 = 1.0x, 0x08000 = 0.5x)
set_3d_scale(&obj, 0x10000);

// Flash: sweep the faces (rainbow or solid white) for N frames, then
// restore the previous palette. Start it, then tick it once per frame.
flash_3d_object(&obj, FLASH_RAINBOW, 30);  // or FLASH_WHITE
update_3d_flash(&obj);   // per frame
```

**Positioning:** All shape vertices are centered at origin (0,0,0).
The `translation` field places the shape's **center** at that point.
Use `z == SCREEN_Z` (512) for 1:1 pixel mapping (perspective factor = 1.0).
Larger z values shrink the projected size and pull positions toward origin.

**Available shapes** (`ShapeType` enum):
| Shape | Vertices | Faces | Type |
|-------|----------|-------|------|
| `SHAPE_CUBE` | 8 | 6 | quads |
| `SHAPE_OCTAHEDRON` | 6 | 8 | tris |
| `SHAPE_ICOSAHEDRON` | 12 | 20 | tris |
| `SHAPE_PYRAMID` | 5 | 5 | 4 tri + 1 quad |
| `SHAPE_CONE` | 10 | 16 | tris (8 side + 8 base) |
| `SHAPE_TORUS` | 63 | 48 | quads (6×8 grid) |
| `SHAPE_DIAMOND` | 6 | 8 | tris (brilliant-cut gem) |

**Key GTE rule:** `RotMatrix`/`TransMatrix` compute into a C struct.
You MUST call `SetRotMatrix`/`SetTransMatrix` to load into GTE hardware
before `RotAverageNclip3/4` will use the transform. (The 3d module handles this.)

**Raw GTE** (for custom code):
```c
SVECTOR in, out;
MATRIX m;
SetRotMatrix(&m);
SetTransMatrix(&m);
RotTransMatrix(&m, &in, &out);
PersDiv(&out);
long otz = GetOTZ(&out);
MakePolyF4(ot, &poly, &out0, &out1, &out2, &out3);
```

## CD-ROM Access

```c
u_long load_file(char *filename) {
    CdlFILE file;
    CdSearchFile(&file, filename);
    CdControl(CdlSetloc, (u_char*)&file.pos, buf);
    CdRead(len_to_sectors(file.size), load_buffer, CdlModeSpeed);
    CdReadSync(0, 0);
    return file.size;
}
```

`load_buffer` is 64 KB global in main.c. Cast to needed type.

## Input (src/input.c)

Pad input is wrapped in `input.c`/`input.h`. Call `init_input()` once at startup
(done in `init_everything()`), and `update_input()` once per frame before any
button queries (done in `main.c`'s loop).

```c
// input.h — pass the SDK's PAD* defines (libetc.h)
bool button_held(u_long b);        // true while held this frame
bool button_just_pressed(u_long b); // true only on the frame it goes down
u_long buttons_held();             // raw bstat from last update_input()
// button_pressed(b) is a #define for button_held(b) (compat)
```


Button defines (`libetc.h`): `PADo` Circle, `PADm` Cross, `PADn` Square, `PADl` Triangle,
`PADR1`/`PADR2`/`PADR3` (L1/L2/L3), `PADL1`/`PADL2`/`PADL3` (R1/R2/R3),
`PADstart`, `PADselect`, `PADLup`, `PADLright`, `PADLdown`, `PADLleft`.
Raw SDK (`libetc.h`): `PadInit(0)`, `u_long buttons = PadRead(0)`.

## SFX (src/sfx.c) — Sound effects

VAG (4-bit ADPCM) sound effects loaded from CD into SPU RAM and played via `libspu`.
The SPU **only decodes 4-bit ADPCM** — it cannot play raw 16-bit PCM.
Each SFX gets a fixed slot in SPU RAM and a dedicated voice (voices 2–23;
0–1 are reserved for CD-DA).

```c
#include "sfx.h"

// Called once in init_everything() (after spu_init)
init_sfx();

// Play with a small random pitch spread (±SFX_PITCH_JITTER, default ±10%)
play_sfx(SFX_BOOM);

// Play at custom pitch (0x1000 = 44.1 kHz, 0x0800 = 22.05 kHz, 0x2000 = 88.2 kHz)
play_sfx_pitch(SFX_BOOM, 0x0800);

// Stop one or all
stop_sfx(SFX_BOOM);
stop_all_sfx();
```

**Pitch formula:** `pitch = (sample_rate << 12) / 44100`

**Adding a new SFX:**
1. Convert: `tools/bin/psxavenc.exe -t vag -f 44100 assets/NAME.wav DATA/NAME.VAG`
   (psxavenc handles any input format via FFmpeg; output is a 48-byte-header mono
   VAG. Pad the file to a 64-byte multiple with trailing zeros so the whole-file
   SPU DMA stays aligned — the SPU stops at the ADPCM EOF flag, so the pad is
   never decoded.)
2. Add an entry to the `SFX_ID` enum in `sfx.h`
3. Add a row to the `sfx_samples[]` array in `sfx.c` (voice, pitch, CD name);
   the SPU address is allocated at runtime with `SpuMalloc`, no manual bumping

**SPU RAM budget:** 512 KB total. ADPCM is 4× smaller than 16-bit PCM
(~22 KB/s at 44.1 kHz). First 4 KB reserved for SPU capture buffers.

**VAG file format:** 48-byte header (magic `VAGp`, big-endian fields) +
16-byte ADPCM blocks (28 samples/block). `sfx.c` DMA's the whole file into SPU
RAM and points the voice's waveform start at `spu_addr + 48` (`VAG_HEADER_SIZE`),
so the header bytes are skipped during decoding.

**Converter:** `tools/bin/psxavenc.exe` (WonderfulToolchain/psxavenc v0.3.1
prebuilt Windows binary). The old `tools/wav2vag.py` produced inaudible output
and has been superseded.

## Debug / Text

```c
printf("...\n");      // → emulator console
FntPrint("...\n");    // → on-screen (bottom-left by default)
FntFlush(-1);         // MUST call before update_display()
```

## Common Pitfalls

- **SPRT has no tpage field.** Must emit `DR_TPAGE` prim before it in the same OT slot.
- **4-bit texture width must be mult of 4, 8-bit mult of 2.** Artifacts otherwise.
- **U,V are 8-bit (0-255).** Can't draw >256px in one sprite. Tile multiple prims.
- **OT depth: higher = further back.** Draw far things at high depth, near at low.
- **`DrawOTag` draws ONE slot** (following that slot's prim `next` chain), not a range. To draw the full OT, use `ClearOTagR` (chains all slots) + `DrawOTag(&ot[OTSIZE-1])`. Plain `ClearOTag` zeros slots with no chaining — `DrawOTag` on it draws only that one slot.
- **PsyQ headers don't include what they use.** Run `make fix-headers` first. After that, include order doesn't matter.
- **`load_buffer` is shared.** Don't hold a pointer to it across another `load_file` call.
- **crt0.s is already in SRCS.** Don't add it twice.
- **No malloc by default.** `InitHeap3` is called in main.c. Use `malloc`/`free` after that.
- **All prims must be `u_long` aligned.** Structs in libgpu.h are already aligned.

## Headers Quick Reference

| Header | Key types/functions |
|--------|-------------------|
| `libgpu.h` | POLY_*, SPRT, TILE, LINE_*, DRAWENV, DISPENV, RECT, setXY*, setRGB*, setTPage, setClut, addPrim, AddPrim |
| `libgs.h` | GsSPRITE, GsBOXF, GsLINE, GsBG, GsMAP, GsSort*, GsOT, TMD_* |
| `libgte.h` | SVECTOR, VECTOR, MATRIX, CVECTOR, RotMatrix, TransMatrix, SetTransMatrix, PersDiv, GetOTZ, MakePoly*, InitGeom, SetGeomOffset, SetGeomScreen, RotAverageNclip4 |
| `libetc.h` | ResetGraph, SetDefDispEnv, SetDefDrawEnv, LoadImage, LoadTImg, MoveImage, DrawOTag, DrawSync, VSync |
| `libcd.h` | CdlFILE, CdSearchFile, CdRead, CdControl, CdReadSync |
| `libpad.h` | PadInit, PadStart, PadRead, PadStat |
| `libspu.h` | SpuInit, SpuOpenChannel, SpuWriteChannel, SpuSetVolume |
| `libsnd.h` | SoundOpen, SoundStart, SoundStop (VAB/ADPCM) |
| `kernel.h` | TAlloc, TCreateThrd, TStartThrd, SysTick |
| `libapi.h` | API init |

## Adding a New 2D Feature (checklist)

1. Create/convert asset → `.tim` in `project-example/DATA/`
2. `load_file("ASSET.TIM")` + `LoadTImg((GsIMAGE*)load_buffer)` at init
3. Note the TIM's VRAM x,y coords (from the TIM header or converter output)
4. Compute `tpage = getTPage(depth, 0, tim_x, tim_y)` and `clut = getClut(clut_x, clut_y)`
5. Fill prim struct, `addPrim(ot + depth, &prim)`
6. `make image && make play` to test

## Adding a New Source File

1. Create `project-example/src/foo.c` (auto-picked-up by the Makefile's `src/*.c` glob — no SRCS edit needed)
2. `#include` what you need (headers are in `nugget/psyq/include/`, already on `-I` path)
3. Build: `make`

Standalone example programs (with their own `main()`) go in `project-example/demos/` — they are NOT built by the Makefile.
