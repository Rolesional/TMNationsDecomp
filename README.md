# TMNF Android native port

This repository is a source-recovery and native Android port of the locally supplied
TrackMania Nations Forever 2.11.11 Windows build. The immutable reference install is
kept in `TmNationsForever/` and deliberately excluded from version control.

The port targets Android 15 (`compileSdk`/`targetSdk` 35) and uses a C++ engine hosted
by Android GameActivity. The first playable native slice now runs on-device: the app
imports user-selected game data into private storage, mounts it through the portable VFS,
follows the original 65-map
Nations campaign order, builds the selected track scene and collision mesh, and drives
the recovered Stadium car low LOD from the real start line with the source-recovered
Race3 loose-follow camera.
This is not yet a compatible release of the full game; exact contact dynamics, full
vehicle audio, replays, broader UI, and source-proven unlock rules remain recovery work.

## Current recovery state

- Reproducible Ghidra analysis of the exact 2.11.11 executable, including 67,840
  functions, 412 imports, and 26,769 defined strings.
- `reverse/reconstruction-manifest.csv` counts only functions with a raw export,
  buildable C/C++ ownership, and runnable evidence. `check-decomp-coverage.ps1` validates
  that ledger against the complete inventory and fails closed under `-RequireComplete`.
- 761 engine class registrations recovered into a portable ID/name table.
- Binary GBX v6 preamble, user-data chunks, external-reference tables, body metadata,
  recovered class-parent relationships, and LZO1X/raw body decoding; headers and bodies
  are validated against all 157 supplied GBX files (141 compressed, 16 raw).
- `CGameCtnChallenge` placement streams parse across all 72 supplied maps: 50,576
  blocks and 6,187 skin references. A01 is independently pinned at 397 blocks and 32
  unique block names.
- Challenge user-data exposes the source medal targets, cost, author score, lap flag,
  and UID without decoding a full scene. The `CGameCtnCampaign` reader preserves the
  White/Green/Blue/Red/Black order, resolves 65 of 66 serialized UIDs to the official
  loose maps, and deliberately omits the container's one stale Red UID.
- `packlist.dat` v1 and `NadeoPak` v3 parsing, MD5 key derivation, little-endian
  Blowfish/CBC decoding, parser-driven cipher reseeding, the original `0x100`-byte
  encrypted-input/`0x400`-byte inflated-output buffering, folder/file indexes, zlib
  payload inflation, and individual file extraction.
- Streaming archive-backed GBX probes validate real root-node reseeds, and a bounded
  zlib-oracle recovery path reproduces a three-seed nested-node correction in
  `Sound.Solid.Gbx` byte-for-byte.
- All three supplied archive indexes parse: 2,572 files total across `Game.pak`,
  `Resource.pak`, and `Stadium.pak`.
- A portable, case-insensitive multi-archive VFS now owns pack data and keys; both the
  host extraction tools and Android content path use the same implementation. It also
  reproduces Nadeo's logical-path hash lookup for flattened archive entries.
- The packed `CPlugSolid`/tree/visual reader follows parser-timed cipher reseeds and
  recovers indexed vertices, packed normals, UV sets, transforms, materials, LODs, and
  collision surfaces. High-detail `CPlugTreeVisualMip` branches are selected while
  lower-detail duplicates are suppressed. The visual and collision buffers in
  `TubeRoad_BaseGround` match an independent GBX.NET parse byte-for-byte.
- Construction-block graphs resolve all 32 A01 block types and choose their ground,
  air, and topology variants. The portable `TrackScene` contains 373 visible
  placements, 33 cached models, 33 textures, 262 enabled visual meshes, 264,735
  instanced vertices, and 635,640 instanced render indices.
- The same host path resolves every block graph and assembles render/collision/trigger
  scenes for all 65 Nations campaign tracks: 41,260 visible placements and 315 waypoint
  trigger instances in total. Animated Bink sign materials are deliberately left
  unbound until video decoding exists; they no longer discard otherwise valid geometry.
- Packed `CPlugSurfaceGeom` data builds an indexed world collision scene with 98,189
  instanced vertices, 528,984 indices (176,328 triangles), and 359 occupied grid cells.
  Per-surface local material tables are kept distinct, external `CPlugMaterial`
  references resolve to their numeric surface IDs, and the 136 triangles authored as
  `NotCollidable` (surface 28) are omitted before grid insertion.
  Portable ground and wall-contact queries feed deterministic, substepped vehicle
  motion, steering, gravity/landing, respawn, and validated saved state. Separate
  two-sided trigger meshes drive ordered checkpoints, finishes, lap counts, race time,
  and `NoRespawn` behavior without entering physical collision.
- The legacy Stadium vehicle-tuning graph parses all 30 records and selects original
  tuning `20juin2006`. Its force curves, speed limits, steering radius, gravity, drag,
  braking, and slowdown values drive the deterministic portable simulation.
- The seven-mesh Stadium car low LOD is parsed from the original packed solid at runtime
  (3,506 vertices and 10,875 indices), textured from its three user-deployed images, and
  follows the simulated vehicle pose and road normal in the GLES scene. The two recovered
  front-wheel meshes visibly steer around their source pivots.
- The ordinary `CGameControlCameraTrackManiaRace3` path now owns the Android chase view:
  source reset geometry, signed-speed response curve, angle-aware spherical direction/up
  follow, distance spring, and vehicle-up view basis are shared portable code rather than
  renderer-tuned constants.
- The original LED bitmap-font atlas renders the live race timer, ordered checkpoint
  and lap counts, numeric km/h speed, plus a native `MAP 01/65` selector with
  personal-best and author times.
  Selection and best records persist atomically in scoped app storage. No unlock
  thresholds are guessed: all 65 source-ordered maps remain selectable.
- Portable image decoding covers TGA (raw/RLE) and DDS top mips in BC1/DXT1,
  BC2/DXT3, BC3/DXT5, RGB(A), luminance, and alpha layouts. Decoded reference-image
  hashes match an independent decoder byte-for-byte.
- Portable input state and an Android GameActivity adapter cover touch, keyboard, and
  gamepad actions. Four visible multi-touch buttons put left/right on the left and
  forward/reverse on the right. Focus/lifecycle transitions release held input and
  preserve the current vehicle state.
- A lock-free stereo frame queue feeds low-latency AAudio output. Pause/resume closes
  and reopens the stream without terminating the process. Portable RIFF PCM16 decode,
  mono expansion, and linear resampling play the original 3/2/1/Go, checkpoint, and
  finish WAVs. The executable-proven 2.5-second start clock locks simulation and race
  time until Go, partitions the three countdown phases at 834/1,667 ms, and keeps Go
  visible for 500 ms.
- Native Android builds for `arm64-v8a` and `x86_64`; the ARM64 ELF uses 16 KiB load
  alignment (as does x86_64), the APK passes 16 KiB zip alignment checks, and both have been
  smoke-tested on an Android 15/API 35 emulator.
- Seventeen portable unit tests cover formats, crypto, VFS, images, input/audio,
  challenge/block data, recovered legacy string views, race start, track collision,
  and vehicle simulation.

## Build

Prerequisites are an Android SDK containing API 35, NDK r29, CMake 3.22.1, and JDK 21.
The workspace bootstrap scripts prefer the project-local JDK in `.tools/` when present.

```powershell
./scripts/build-android.ps1
```

The debug APK is emitted under `app/build/outputs/apk/debug/`.

## Host-side GBX probe

```powershell
./scripts/build-host-tools.ps1
./build/host/gbx_inspect.exe ./TmNationsForever/GameData/Tracks/Campaigns/Nations/White/A01-Race.Challenge.Gbx
./scripts/test-reference-data.ps1
```

The reference test covers all GBX headers and bodies, every challenge placement stream,
all archive preambles and indexes, five real extraction/texture probes, parser-driven
cipher reseeds, material/bitmap chains, high-detail MIP selection, packed visual and
collision hashes, A01/C11 scene totals, and the packed aliases used by the campaign.

## Install on Android

The APK deliberately contains no original game assets. Copy the original
`TmNationsForever` folder to a normal user-visible folder on the device, for example
`Download/TmNationsForever`. Install and launch the APK, tap **Select game folder**, and
choose that folder. The app imports `Packs` and `GameData` into its private storage and
then starts the native game. Do not copy files into `Android/data`; Android 15 can leave
such manually copied files inaccessible to the app.

For a connected development target, this stages the minimum current runtime set in
`Download/TmNationsForever`:

```powershell
./scripts/build-android.ps1
./scripts/deploy-reference-data.ps1 -Serial emulator-5554
```

The script stages the required user-owned packs, campaign container, all 65 challenges,
and the exact 73-file campaign/car/HUD/audio runtime set, including six race-event WAVs.
Select the staged folder on first launch. The native app mounts the archives, joins the
campaign by UID, builds the selected scene/collision, and spawns at its decoded start transform. Use
the visible left/right buttons on the left and forward/reverse buttons on the right,
or use WASD/the D-pad to drive; `R`/gamepad Y respawns. Press `P`/Menu/gamepad Start, or
tap the `M` in the upper-left corner, to open the map selector; left/right changes maps
and Enter/gamepad A or a center-screen tap loads one. A completed race advances to the
next campaign map after a short result pause.

## Reverse engineering

```powershell
./scripts/analyze.ps1
./scripts/decompile.ps1 -Missing -Limit 500
./scripts/check-decomp-coverage.ps1
```

This imports the exact reference executable into a local Ghidra project and exports
machine-readable function, string, and class-registration inventories. The decompiler
command resumes from the next missing raw function; the coverage command reports raw and
verified reconstructed totals separately. See `docs/reverse-engineering.md` and
`docs/architecture.md` for scope and sequencing.

No original executable, archive, or copyrighted game asset is copied into the source
tree or APK by the current build.
