# Port architecture

The Windows executable cannot be translated mechanically into a maintainable Android
binary: it is 32-bit x86/MSVC code tied to Win32, D3D9/D3DX, DirectInput, WinInet,
Winsock, Bink, and OpenAL. The port therefore recovers engine behavior and data layouts
while replacing platform boundaries behind portable C++ interfaces.

## Layers

1. **Recovered core** — deterministic simulation, GameBox object model, GBX/PAK I/O,
   scene/campaign state, vehicle dynamics, replay logic, and UI state machines.
2. **Portable services** — virtual file system, jobs, clocks, networking contracts,
   renderer command stream, audio mixer, and input actions.
3. **Android platform** — GameActivity lifecycle/input, EGL/OpenGL ES initially,
   AAudio output, scoped app storage, networking, and asset delivery.
4. **Host tools/tests** — format probes and deterministic fixtures built from the same
   C++ library as the APK.

The initial renderer is OpenGL ES 3 because it is the smallest verifiable replacement
for D3D9. The command/data boundary must remain backend-neutral so Vulkan can be added
without contaminating recovered gameplay code.

## Milestone order and status

1. **Complete bootstrap:** GameActivity lifecycle, EGL/OpenGL ES 3 rendering,
   touch/keyboard/gamepad input, AAudio output, scoped content storage, API 35
   packaging, and 16 KiB-compatible ARM64 ELF/APK layout run on an Android 15 emulator.
2. **Core container path complete:** GBX headers, user-data chunks, reference tables,
   common body metadata, the 761-class registry, direct parent relationships, and
   LZO1X/raw bodies parse across the complete loose corpus. Per-class readers continue
   to be added as gameplay requires them.
3. **Core path complete:** NadeoPak v3 key discovery, index parsing, decryption,
   original zlib buffering, parser-driven dummy writes, inflation, and extraction.
   Real root and nested reseeds have regressions. A case-insensitive, mountable
   multi-archive VFS now provides the shared host/Android content boundary.
4. **First full track path complete:** portable TGA/DDS decoding, packed solid/tree/
   visual/material/MIP/collision parsing, block-variant selection, texture binding,
   full A01 scene assembly, cardinal transforms, Direct3D-to-GLES V orientation, and
   indexed GLES upload run on Android. The scene caches 33 models and 33 textures for
   373 placements. The host path also assembles all 65 Nations campaign scenes. The APK
   now joins the original campaign container to loose maps by UID and can load any of the
   65 source-ordered scenes from its native selector.
5. **Initial gameplay path complete:** portable action state, Android adapters, an
   instanced 176,328-triangle collision scene with source material IDs and
   `NotCollidable` filtering, ground and exact triangle/ellipsoid contact queries,
   deterministic vehicle stepping, ordered checkpoint/finish/lap triggers, race timing,
   road-safe respawn/saved state, the recovered Stadium car low LOD with steering front
   wheels, and the ordinary Race3 angle-aware loose-follow camera are live. Exact
   start/waypoint transforms, authored body ellipsoids, and the active source vehicle
   tuning are decoded; remaining wheel multi-contact reduction and broader vehicle state
   machines are still being recovered.
6. **In progress:** the original LED HUD, 65-map selector, author targets, atomic best-time
   persistence, executable-proven 3/2/1/Go start gate, and original countdown/checkpoint/
   finish audio, numeric speed, and four-button multi-touch driving overlay are
   operational. Exact unlock rules, broader menus, replays, engine/surface/skid audio,
   and recovered contact physics remain.
7. Editors and online systems after the offline 65-track campaign is compatible.
