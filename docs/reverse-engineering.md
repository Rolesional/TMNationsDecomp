# Reverse-engineering baseline

Reference build: `TmForever.exe` from TrackMania Nations Forever 2.11.11.

- SHA-256: `808bf7fed8d74e54950129bb47c6f0a65a25c05415d49e280760db68527fc26f`
- PE: i386, MSVC linker 8.0, image base `0x00400000`
- Timestamp: 2008-04-13 23:03:28 UTC
- Entry RVA: `0x00003E7E`
- Sections: `.text`, `.rdata`, `.data`, `.rsrc`, `.reloc`
- No Authenticode signature and no obvious executable packer

The binary exposes a useful recovery spine even though it has no public symbols:

- MSVC RTTI metadata and many C++ type/construction strings are present.
- The embedded GameBox registry names hundreds of classes, including
  `CMwEngine`, `CGameCtnChallenge`, `CGameRace`, and `CGameCtnGhost`.
- Imported subsystem boundaries are explicit: D3D9/D3DX9, DirectInput 8, OpenAL,
  Bink, AVI, WinInet/Winsock, Win32 windowing/filesystem/registry, and COM.

`scripts/analyze.ps1` imports the executable into Ghidra 12.1.2 and writes diffable
function, import, and string inventories under `reverse/ghidra/exports/`. Address-based
names remain provisional until behavior or RTTI establishes a stable semantic name.

The completed baseline analysis contains:

- 67,840 functions
- 26,769 defined strings
- 1,874 class-like strings and 1,544 references to them
- 412 imported functions

## Recovered class registry

The static initialization sequence has a stable five-argument pattern that calls
`0x00429570`. That function constructs a `CMwClassInfo`-like record containing the
class ID, descriptor/parent pointer, class name, and constructor. The automated Ghidra
export recovered 761 unique registrations and no duplicate IDs.

Confirmed anchors include:

| Class ID | Name | Constructor |
| --- | --- | --- |
| `0x01001000` | `CMwNod` | `0x00425DF0` |
| `0x01003000` | `CMwEngine` | `0x00427630` |
| `0x03043000` | `CGameCtnChallenge` | `0x0056FFF0` |
| `0x03092000` | `CGameCtnGhost` | `0x005FA830` |
| `0x03093000` | `CGameCtnReplayRecord` | `0x00569100` |
| `0x09019000` | `CPlugFilePack` | `0x00900C40` |

`ExportClassRegistry.java` writes the evidence table, and
`tools/generate_class_registry.py` converts it into the sorted portable registry used
by host tools and Android. The export resolves each registration's descriptor pointer
back to another registration, so the portable table also contains the direct parent
class ID used by the encrypted GameBox reader.

## NadeoPak v3

`CPlugFilePack` parsing at `0x009020A0` reads eight bytes, compares them to
`NadeoPak`, reads a little-endian version, and accepts version 3. The constructor at
`0x00900910` initializes a 176-byte object containing a four-word cipher key at
offsets `+0x38..+0x44` and a loader object at `+0xA0`.

The recovered portable path now reproduces:

1. `packlist.dat` v1 entry decoding and its ten local archive base keys.
2. MD5 derivation of the per-archive key.
3. The little-endian Blowfish key schedule and block representation.
4. CBC-style streaming with an eight-byte IV and the engine's 256-byte reseed rule.
5. Version-3 archive metadata, folder hierarchy, and file table.
6. Per-file decryption plus zlib inflation.
7. Case-insensitive logical-path lookup, including the flattened filename scheme made
   from a lowercased path's length byte and MD5 digest with swapped output nibbles.

`CClassicBufferCrypted::vftable` is at `0x00CFC7B8`. Its read path at `0x0089C450`
decrypts 256-byte cycles, while `0x0089C540` is the mode-dependent write/dummy-write
path that rotates the 64-bit pending XOR by 13 bits per byte. The zlib wrapper is just
as important to observable cipher timing: `CClassicBufferZlib::vftable` is at
`0x00CFC774`, `0x0089AE00` configures inflate mode, and `0x0089AEA0` reads encrypted
input in `0x100`-byte chunks while filling a `0x400`-byte logical output buffer.

Real-data validation produces coherent indexes:

| Archive | Folders | Files | Data start |
| --- | ---: | ---: | ---: |
| `Game.pak` | 54 | 614 | 262,144 |
| `Resource.pak` | 4 | 27 | 262,144 |
| `Stadium.pak` | 56 | 1,931 | 262,144 |

The Android probe has rendered both `Media\\Texture\\Image\\WhiteFlare1.tga` and the
BC1 `Media\\Texture\\Image\\MeshDefault.dds` directly through this path. The streaming
GBX reader applies root-node dummy writes and validates them on real Stadium entries.
A bounded deflate-oracle fallback also recovers the nested three-seed XOR
`0x1FF0FE07F86E4317` at cipher cycle 2 of `Media\\Solid\\Sound.Solid.Gbx`; the recovered
2,065-byte logical file is pinned by SHA-256.

The parser-driven nested graph is operational for the packed
`CPlugSolid`/`CPlugTree`/`CPlugTreeVisualMip`/`CPlugVisual` hierarchy. Opening
`Media\\Solid\\Tube\\Road\\BaseGround.Solid.Gbx` resolves the stored flattened entry
`Stadium\\Media\\Solid\\E11CFB84BECB8590C7832182A5B1718D70`, then produces 25 nodes,
four external nodes, seven visuals, 2,697 vertices, and 4,254 indices. Position and
index hashes for every visual match GBX.NET byte-for-byte. MIP references are resolved
after graph parsing so only level-zero visual leaves render; a corpus sweep of A01's 77
candidate solids keeps 616 high-detail visuals and suppresses 344 lower-detail copies.

`CPlugSurfaceGeom` collision is decoded rather than skipped. A01's candidate solids
contain 79 version-3 mesh shapes with 42,815 unique vertices and 232,116 unique indices.
For `TubeRoad_BaseGround`, its 104 collision positions and 558 indices match GBX.NET
byte-for-byte (`CC9A7BBC6B8D9136EB6D385F49FCA86B` and
`D2AEE7F055F44CCC39F5934A70B1079A`). `CPlugSurface` owns a local material table for
each referenced geometry, so the parser records that table's range instead of flattening
unrelated tables. External `CPlugMaterial` nodes expose their numeric surface in chunk
`0x0907900E`; surface 28 is the native `NotCollidable` boundary and is discarded before
the portable collision grid is built. On A01 that removes exactly 136 authored triangles
(64 Start, 40 Finish, and 32 Checkpoint), leaving 176,328 world triangles and 359 occupied
grid cells. The start road resolves to Asphalt (surface 16).

## Challenge and track assembly

LZO1X body decoding now validates all 141 compressed GBX files; the other 16 bodies are
raw. The 72 `CGameCtnChallenge` streams contain 50,576 placements and 6,187 skin
references. `CGameCtnBlock` flags use bits 0–5 for Variant, bits 6–11 for SubVariant,
and bit 12 for IsGround. Those fields select recovered ground/air assets and the
`Base`, `Deadend`, `Corner`, `Straight`, `TShaped`, and `Cross` topology families.

The challenge header's `0x03043002` version-11 record supplies bronze, silver, gold,
and author times plus cost/lap/score flags; `0x03043003` version 5 supplies MapInfo UID
and name. The Nations `CGameCtnCampaign` body contains five ordered
`CGameCtnChallengeGroup` nodes with 66 MapInfo identities. Exact UID joins resolve
15/15/15/15/5 loose maps in White/Green/Blue/Red/Black order. The unmatched serialized
Red UID `Nk8jftxeWaIICIOEGV8M_cV3gba` is stale and is omitted. The campaign declares
`ByRow`, but the supplied files do not prove row thresholds, so the port does not invent
an unlock gate.

For A01, all 32 construction-block graphs resolve without an unresolved reference.
Twenty-four `GrassClip` placements are intentionally nonvisual; the remaining 373
placements select 33 solids and 33 decoded textures. The assembled scene has 262 unique
enabled meshes, 64,516 unique vertices, 161,715 unique indices, and 635,640 instanced
render indices. The start block is `StadiumRoadMainStartLine` at grid coordinate
`{5,11,21}`, direction 3. Visual and collision bounds agree there, and collision
continuity establishes its forward course direction as world +X.

TMF's cardinal basis is North `+Z`, East `-X`, South `-Z`, and West `+X`; using that
basis consistently keeps visual placements, collision, and the start transform aligned.
The recovered DDS/mesh path also reflects the Direct3D-authored V coordinate at the
GLES upload boundary, so asymmetric decals and lettering render upright on Android.

The vehicle remains only partially recovered, but its current contact path is no longer a
height-query stand-in. It initializes from the decoded start transform, uses the active
legacy Stadium tuning, samples the source one-sided triangle/ellipsoid narrowphase for
four wheel ellipsoids, and enumerates all contacts for the four authored body ellipsoids.
The body callback applies source-ordered correction reduction plus effective-mass linear/
angular impulses. The remaining known wheel mismatch is that portable suspension sampling
selects one maximum-axis contact while the native callback path retains every accepted
contact. `FUN_009DA440` makes wheel state deliberately order-sensitive: the last callback
owns final support and point, all supporting callbacks contribute to the raw normal sum,
the last supporting callback owns material, and maximum positive suspension absorption is
tracked while each residual remains on its contact. `FUN_006847C0` then appends body and
wheel corrections to one list per rigid body; `FUN_006907F0 -> FUN_0068FF90` reduces that
single list after dispatch. The portable broadphase does not yet prove native
collision-pair interleaving, so this is documented rather than replaced with a guessed
loop.
Ordered two-sided trigger meshes drive checkpoints, finishes, lap counts, race timing, and
`NoRespawn` behavior. The GLES path renders the packed Stadium car's seven-mesh low LOD
with its original three texture images at the simulated pose.

## Race3 chase camera

The ordinary chase view is owned by `CGameControlCameraTrackManiaRace3`: constructor
`0x006F1E10`, reset `0x006F3880`, and update `0x006F41E0`. Its neutral reset eye is
`P + 2*U - 4*F`; the look target is `P + U`, and the view-up vector is the smoothed
vehicle up rather than world Y. Direction and up are spherical low-pass filters whose
ordinary-mode fractions are:

```text
direction t = speedGain * clamp(5*dt / (2*theta + 1), 0, 1)
up        t = speedGain * clamp(4*dt / (2*phi   + 1), 0, 1)
```

`dt` is capped at 0.2 seconds and a backwards clock yields zero. `speedGain` is the
piecewise-linear curve X `[-100,-50,0,250,500,700]` km/h and
Y `[1.5,1.2,1,1.5,2,3]`. The neutral distance is `sqrt(20)` and uses
`v += dt*((sqrt(20)-x)*10-v*5)`, followed by `x += dt*v`. The slower 3/1 angular gains
belong to a special internal mode and are not used for ordinary Race3 driving. Vehicle
position is added directly each frame; there is no eye/target world-position spring.
Ordinary vertical FOV is 90 degrees. Turbo adds a cosine-ramped 10.5-degree delta with
1000 ms rise and 1400 ms fall, which remains deferred with the Turbo state machine.

## Race start clock

The single-player countdown timing is recovered from the executable rather than chosen
by feel. `CTrackManiaRace1P::vftable` slot `+0x110` points to `0x00715310`, which returns
`0x9C4` (2,500 ms). `0x00715320` derives the countdown end and start timestamps from that
virtual duration. `0x00715460` divides the open interval between the start field at
`+0x28C` and end field at `+0x294` into three integer phases; for 2,500 ms the transitions
are 834 and 1,667 ms. It plays cue IDs `0x1E`, `0x1D`, and `0x1C` for 3, 2, and 1, then
keeps phase 3/Go visible while race state `+0x2FC` is 1 and the clock is less than end plus
500 ms. The central race update at `0x0071890B` transitions to state 1 when
`current >= end`, and `0x0074EB40` returns zero before that state then derives active
race time as `current - end`. Player controls are sampled earlier in the update at
`0x00718833`; the start state constrains motion rather than discarding input.

The constraint is now traced exactly. `0x0071E030` calls vehicle slot `+0x140(1)`, which
sets the freeze bit through `0x009D5350`. While frozen, `0x009E004F` bypasses propulsion,
suspension, tire, and brake force solving and overwrites linear/angular velocity, net
force, and torque with zero. Outer collision dispatch remains active. At Go,
`0x00718960` clears the freeze and `0x0071896C` immediately calls reset `0x009D9590`;
per-wheel reset `0x009D6430` restores the authored suspension rest length and clears
speed, pending correction, support, and contact. A01's shallow flat wheel overlap is
fully suspension-absorbed during the countdown, produces no chassis translation, and is
then cleared by that reset.

The portable `RaceStartClock` keeps consuming held input while holding portable vehicle
and race time at zero through the 2.5-second interval, releases only the post-boundary
portion of a crossing frame, and uses the original four loose Race3/Race2/Race1/RaceGo
WAVs. `VehicleSimulation` consumes released time through a 120 Hz accumulator so an
arbitrary microsecond crossing-frame tail cannot become a suspension derivative step.
Portable countdown collision-only dispatch remains a known non-A01 parity gap for a
start with deeper or nonvertical contact. The 2,500 ms duration is specific to the
base/single-player modes used here; NetLaps and NetRounds override the same virtual slot
with 3,000 ms.

The cryptographic behavior was also cross-checked against the MIT-licensed GBX.NET
implementation identified in `THIRD_PARTY_NOTICES.md`; executable addresses and the
supplied corpus remain the authority for this reference build.

## Recovery discipline

- Preserve original addresses in comments and recovery notes.
- Separate observations (disassembly/data) from hypotheses and confirmed behavior.
- Test parsers against both tiny fixtures and the supplied original files.
- Keep platform replacements out of recovered algorithms.
- Do not ship or commit the original executable or assets.
