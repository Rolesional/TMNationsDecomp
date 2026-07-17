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
`D2AEE7F055F44CCC39F5934A70B1079A`). Instancing those shapes with the same block
transforms used by rendering produces 176,464 world triangles indexed into 364 grid
cells for portable height queries.

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

The current vehicle dynamics are a portable deterministic stand-in, not recovered TMNF
physics. The simulation initializes from the decoded start ground/air transform, follows
decoded ground triangles, applies substepped acceleration/braking/steering/gravity, and
supports horizontal contact against decoded non-ground triangles, respawn, and lifecycle
state validation. Ordered two-sided trigger meshes now drive checkpoints, finishes, lap
counts, race timing, and `NoRespawn` behavior. The GLES path renders the packed Stadium
car's seven-mesh low LOD with its original three texture images at the simulated pose;
exact vehicle/contact dynamics remain recovery work.

The cryptographic behavior was also cross-checked against the MIT-licensed GBX.NET
implementation identified in `THIRD_PARTY_NOTICES.md`; executable addresses and the
supplied corpus remain the authority for this reference build.

## Recovery discipline

- Preserve original addresses in comments and recovery notes.
- Separate observations (disassembly/data) from hypotheses and confirmed behavior.
- Test parsers against both tiny fixtures and the supplied original files.
- Keep platform replacements out of recovered algorithms.
- Do not ship or commit the original executable or assets.
