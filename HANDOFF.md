# TMNF Android Port Handoff

Last verified: 2026-07-17 14:31 CDT

Workspace: D:\Stuff\DevFolder\TMNationsDecomp

This is the restart document for the native Android 15 port of the locally supplied
TrackMania Nations Forever 2.11.11 build. Read README.md, docs/architecture.md, and
docs/reverse-engineering.md for the durable implementation and format details. This
file records the current checkpoint, runtime proof, unresolved research, and safest
next slice.

## Stop state

- The goal is paused, not complete.
- Recovery heartbeat tmnf-android-port-recovery is PAUSED.
- All three research subagents stopped at read-only checkpoints.
- No Android device is connected.
- adb devices was empty and the emulator/QEMU process count was zero at shutdown.
- No emulator is meant to be running in the background.
- The latest debug APK is app/build/outputs/apk/debug/app-debug.apk.
- APK size: 8,710,166 bytes.
- APK SHA-256: 29D0FFE1945181F9B47275EE6A8A08A684258B4DA3C430B7AAF0AE0C6859D492.

## Repository safety

- This Git repository has no commit yet. The current branch is main and the entire
  implementation appears as untracked files. There is no commit SHA to return to.
- Preserve everything. Do not use git clean, reset, checkout, or any bulk deletion.
- TmNationsForever contains the user's immutable reference installation and is ignored
  by .gitignore. Never edit it.
- Do not copy the original executable, archives, maps, replays, or copyrighted assets
  into tracked source or into the APK.
- The APK intentionally requires separately deployed user-owned reference data.
- Prefer a bounded source-backed change and one runnable regression over speculative
  infrastructure.

Reference executable:

- TmNationsForever\TmForever.exe
- Version: 2.11.11
- SHA-256: 808BF7FED8D74E54950129BB47C6F0A65A25C05415D49E280760DB68527FC26F

## Mission and honest compatibility boundary

The objective is a buildable native Android application targeting Android 15 that
recovers the Windows game's data formats and offline Nations campaign behavior. The
implementation is portable C++ hosted by Android GameActivity, with GLES 3 rendering,
AAudio, scoped storage, and touch/keyboard/gamepad input.

This is not a complete compatible port yet. It is a source-backed playable framework
with all 65 Nations maps loadable. Exact vehicle dynamics, the original countdown and
input gate, vehicle/game audio, replay/ghost playback, and broader menu flow remain.
Do not describe the current vehicle simulation as recovered TMNF physics.

## Current proof

| Area | Verified state |
| --- | --- |
| Android target | compileSdk/targetSdk 35, minSdk 26, NDK r29, C++20 |
| ABIs | arm64-v8a and x86_64; 16 KiB-compatible ELF/APK layout |
| Host tests | 14/14 CTest tests passed |
| Loose GBX corpus | 157/157 headers and bodies; 141 LZO, 16 raw |
| Challenge corpus | 72/72; 50,576 blocks, 6,187 skins, 247 checkpoints |
| Nations campaign | 65 resolved maps in White/Green/Blue/Red/Black order |
| All-track scene proof | Host-built all 65 scenes: 41,260 visible placements and 315 waypoint triggers |
| Android runtime proof | API 35 emulator: A01 loaded, selector moved to A02, A02 loaded, selection survived force-stop/relaunch |
| Vehicle visual | Original Stadium car low LOD: 7 visuals, 3,506 vertices, 10,875 indices, 3 textures |
| Gameplay currently live | Spawn, chase camera, deterministic stand-in motion, collision, ordered checkpoints, finish/laps, NoRespawn, respawn, timer |
| HUD and records | Original LED atlas, map selector, author target, personal best, atomic persistence |
| Audio currently live | Original RaceCheckPoint.wav and RaceFinish.wav only |

The all-65 scene claim is host-side proof. Device runtime was directly proved only on
A01 and A02. A full 65-track Android traversal remains required.

Latest runtime evidence:

- build\tmnf-selector.png: upright A01 selector showing MAP 01/65 and A 00:24.540.
- build\tmnf-a02.png: upright A02 after selector activation.
- Android log: Official Nations campaign ready: 65 ordered maps.
- A01: 397 blocks, 373 visible placements, 176,464 collision triangles,
  132 trigger triangles, 2 checkpoints.
- A02: 299 blocks, 299 placements, 21 models, 33 textures, 172,852 collision
  triangles, 56 trigger triangles, 1 checkpoint.
- Relaunch log: Loaded A02-Race (299 blocks).
- Scoped progress file:
  /sdcard/Android/data/com.tmnf.port/files/nations-progress.txt

The image-orientation regression was fixed with the Direct3D-to-GLES V-coordinate
conversion in app/src/main/cpp/platform/android/egl_renderer.cpp. The current
screenshots are upright. Do not add a horizontal/global texture flip to address a
single asset.

## Campaign invariants

- CGameCtnCampaign is class 0x03090000.
- The source container serializes group counts 15/15/15/16/5, or 66 UIDs.
- Exactly 65 UIDs resolve to the loose official maps.
- The stale Red UID is Nk8jftxeWaIICIOEGV8M_cV3gba and is deliberately omitted.
- Join campaign entries to loose challenges by stable map UID only.
- Do not require campaign and loose challenge authors to match. They can differ
  intentionally; UID syko1DvAxleoVlqMsc1sJzqbt66 is campaign author alinoa but loose
  A06 author Nadeo.
- All 65 loose headers have complete medal targets and NeedUnlock=false.
- No source unlock threshold has been recovered. All 65 maps remain selectable; do
  not invent gating rules.

## Main code map

| Path | Ownership |
| --- | --- |
| app/src/main/cpp/native_main.cpp | Android lifecycle, selector, progress, race events, simulation loop |
| app/src/main/cpp/content/reference_probe.cpp | Android campaign/data mount and scene/car/audio loading |
| app/src/main/cpp/platform/android/egl_renderer.cpp | GLES scene, Stadium car, camera, LED HUD, selector |
| app/src/main/cpp/platform/android/audio_output.cpp | AAudio output and one-shot playback |
| app/src/main/cpp/platform/android/input_adapter.cpp | Touch, keyboard, and gamepad mapping |
| core/src/gameplay/vehicle.cpp | Deterministic temporary vehicle simulation and race state |
| core/src/content/track_scene.cpp | Challenge blocks to packed render/collision/trigger scene |
| core/src/content/track_collision.cpp | Collision grid, ground/wall queries, trigger geometry |
| core/src/gbx/campaign.cpp | Original campaign group/order parser |
| core/src/gbx/challenge.cpp | Challenge header/body, blocks, medals, waypoints |
| core/src/gbx/solid.cpp | Packed solid/tree/visual/surface parsing |
| core/src/pak | Pack list, NadeoPak, cipher recovery, VFS |
| scripts/test-reference-data.ps1 | Full local reference-corpus regression |
| scripts/deploy-reference-data.ps1 | Exact user-owned runtime data deployment |

## Build and checks

Prerequisites:

- Android SDK with API 35
- NDK 29.0.14206865
- CMake 3.22.1 and Ninja from the Android SDK
- JDK 21; scripts prefer .tools\jdk-21.0.11+10 when present
- C:\msys64\mingw64\lib\libz.a for the current Windows host build

From the workspace root:

~~~powershell
Set-Location D:\Stuff\DevFolder\TMNationsDecomp
.\scripts\build-host-tools.ps1
.\scripts\test-reference-data.ps1
.\scripts\build-android.ps1
~~~

Expected APK:

~~~text
app\build\outputs\apk\debug\app-debug.apk
~~~

Run the host build after core/parser/gameplay changes, the reference regression after
format/content changes, and the Android build after any app or shared-core change.

## Emulator rule: mandatory

The user must never see the emulator window. Every emulator launch requires both:

1. Emulator argument -no-window.
2. Start-Process -WindowStyle Hidden.

Do not launch an emulator for host-only work. When device validation is actually
needed, the known AVD is TMNF_API_35:

~~~powershell
. .\scripts\toolchain.ps1
$emulator = Join-Path $env:ANDROID_HOME 'emulator\emulator.exe'
$emulatorArgs = @(
    '-avd', 'TMNF_API_35',
    '-no-window',
    '-no-boot-anim',
    '-no-snapshot-save',
    '-gpu', 'swiftshader_indirect',
    '-no-audio'
)
$emulatorProcess = Start-Process -FilePath $emulator -ArgumentList $emulatorArgs -WindowStyle Hidden -PassThru
~~~

Confirm the live command line contains -no-window before continuing:

~~~powershell
Get-CimInstance Win32_Process |
    Where-Object ProcessId -EQ $emulatorProcess.Id |
    Select-Object ProcessId, Name, CommandLine
~~~

Install, deploy, and launch:

~~~powershell
$adb = Join-Path $env:ANDROID_HOME 'platform-tools\adb.exe'
$serial = 'emulator-5554'
& $adb -s $serial wait-for-device
for ($attempt = 0; $attempt -lt 120; ++$attempt) {
    if ((& $adb -s $serial shell getprop sys.boot_completed).Trim() -eq '1') {
        break
    }
    Start-Sleep -Seconds 2
}
if ((& $adb -s $serial shell getprop sys.boot_completed).Trim() -ne '1') {
    throw 'Android emulator did not finish booting'
}
& $adb -s $serial install -r '.\app\build\outputs\apk\debug\app-debug.apk'
if ($LASTEXITCODE -ne 0) { throw 'APK install failed' }
.\scripts\deploy-reference-data.ps1 -Serial $serial
& $adb -s $serial logcat -c
& $adb -s $serial shell am start -n 'com.tmnf.port/.TmnfActivity'
~~~

Always clean up in the same work cycle, even after a failed test:

~~~powershell
& $adb -s $serial emu kill
Start-Sleep -Seconds 3
Get-CimInstance Win32_Process |
    Where-Object Name -Match '^(emulator|qemu-system.*)\.exe$' |
    ForEach-Object { Stop-Process -Id $_.ProcessId -Force }

$emulatorDevices = @(& $adb devices) -match '^emulator-'
$emulatorProcesses = @(
    Get-CimInstance Win32_Process |
        Where-Object Name -Match '^(emulator|qemu-system.*)\.exe$'
)
if ($emulatorDevices -or $emulatorProcesses.Count -ne 0) {
    throw 'Emulator cleanup failed'
}
& $adb devices
Write-Output "EMULATOR_PROCESSES=$($emulatorProcesses.Count)"
~~~

The required final result is no emulator device and EMULATOR_PROCESSES=0.

## Best next bounded slice: source-proven race countdown

Do not implement a guessed generic three-second timer yet. The read-only executable
trace established:

- CGameInterface class: 0x0302D000.
- Constructor: 0x005EB6B0; object size 0x1A4.
- Field registration: 0x005EC1C0.
- Chunk/field reader: 0x005EB720.
- Chunk 0x0302D01B, object +0x14C: RaceGo.
- Chunk 0x0302D01C, object +0x150: RaceCountdown3.
- Chunk 0x0302D01D, object +0x154: RaceCountdown2.
- Chunk 0x0302D01E, object +0x158: RaceCountdown1.
- HUD control TimeCountDown is bound from object +0x160 by function 0x0075EF90;
  binding instruction 0x0075F10A.

Likely loose sounds:

- GameData\Interface\Media\Audio\Sound\Wav\Race1.wav
- GameData\Interface\Media\Audio\Sound\Wav\Race2.wav
- GameData\Interface\Media\Audio\Sound\Wav\Race3.wav
- GameData\Interface\Media\Audio\Sound\Wav\RaceGo.wav

Race1/2/3 are PCM16 stereo 44.1 kHz and 1.111519 seconds each. RaceGo is the same
format and 1.410612 seconds. The names strongly support the mapping, but exact phase
boundaries, timer zero, and control release are not yet runtime-proved.

Next action: trace executable callsites that dereference CGameInterface +0x14C through
+0x158, then follow their time comparisons and vehicle-control gate. Implement the
smallest shared race-start state only after the 3, 2, 1, Go timestamps and input/timer
release relationship are established. Add one portable regression and deploy the four
WAVs only when used.

Temporary decompilation from the stopped research thread was under
%TEMP%\tmnf-countdown-decomp and is disposable.

## Replay/ghost checkpoint

- There are exactly 65 campaign Replay.gbx files under
  TmNationsForever\GameData\Tracks\Campaigns\Nations.
- TmNationsForever\GameData\Tracks\Replays\Bench.Replay.Gbx is separate.
- Replay root class: CGameCtnReplayRecord, 0x03093000.
- Relevant chunks: 0x03093002 embedded challenge, 0x03093007, 0x03093014 ghost array,
  and 0x03093015 clip.
- A01 contains four CGameCtnGhost medal ghosts.
- Ghost vehicle state is zlib data in chunk 0x0303F005.
- Version 9 uses fixed 100 ms sampling and uniform 61-byte records:
  12-byte float position, 6-byte compressed quaternion, 4-byte velocity,
  4-byte angular velocity, and 35 bytes of vehicle state.
- A01 Author Medal is 24,540 ms with 276 samples covering 0 through 27,500 ms.
- Playback can initially ignore velocity and the 35-byte vehicle state.

Minimal next replay boundary: parse replay chunk 0x03093014, select the first/Author
ghost, inflate 0x0303F005, decode position and quaternion, and linearly interpolate at
100 ms. Before implementation, hash the compressed/decompressed A01 buffer and prove
the six-byte quaternion plus TMF-to-renderer axis mapping against the start block.
Then run the schema/count check over all 65 replay files.

No replay parser was added at this checkpoint.

## Vehicle physics recovery checkpoint

core/src/gameplay/vehicle.cpp is deliberately a deterministic stand-in. Avoid tuning
its constants by feel and calling that recovered physics.

Recovered asset chain:

- Vehicles\TrackManiaVehicle\StadiumCar.ConstructionVehicle.Gbx
- Vehicles\Mobil\Vehicle\StadiumCar.Car.Vehicle.Gbx, old class 0x0A02B000
- Vehicles\VehicleTunings\StadiumCar.VehicleTunings.Gbx
- Vehicles\VehicleStructs\StadiumCar.VehicleStruct.Gbx

Flattened archive entries:

- Tunings: Vehicles\C2A255EF80618DC714C2B1F7CA68608C60
- Struct: Vehicles\B2BF3F79BBB3C147236CAE6FD05B733B19

Class wrapping confirmed in .tools\reference\gbx-net\Resources\Wrap.txt:

- old 0x0A030000 -> modern 0x090EC000 CPlugVehiclePhyTunings
- old 0x0A029000 -> modern 0x090ED000 CPlugVehicleCarPhyTuning
- old 0x0A039000 -> modern 0x090E8000 CPlugVehicleVisModelShared

Useful schemas:

- .tools\reference\gbx-net\Src\GBX.NET\Engines\Plug\CPlugVehiclePhyTunings.chunkl
- .tools\reference\gbx-net\Src\GBX.NET\Engines\Plug\CPlugVehicleCarPhyTuning.chunkl
- .tools\reference\gbx-net\Src\GBX.NET\Engines\Plug\CPlugVehicleGearBox.chunkl

archive_gbx_prefix recovered the tuning header and a prefix:

- Root class 0x0A030000.
- Body starts at logical offset 25 with chunk 0x0A030000.
- Version-like value 10, tuning count 30.
- First tuning node class 0x0A029000 and ID 26Nov2002.

Full VFS extraction still fails on nested cipher dummy writes. archive_recover's
bounded oracle, currently limited to three seed writes per cycle, did not recover the
whole tunings or struct files. Do not blindly widen it into an unbounded brute-force
search. Prefer a parser-driven dummy-write sequence or another source-proven bounded
recovery.

## Vehicle audio checkpoint

Do not add a speed-only engine loop. The original graph requires drivetrain state that
VehicleState does not yet expose.

- StadiumCarEngine.SoundEngine.Gbx is packed as
  Vehicles\Media\C2D92488B94D629BE48DD411986EF06362.
- Root class: CPlugSoundEngine, 0x0908E000.
- It uses six PCM16 stereo 44.1 kHz clips:
  SportCarEngineIdle.wav, SportCarEngineLow.wav, SportCarEngineMid.wav,
  SportCarEngineFast.wav, SportCarEngineRev.wav, and SportCarEngineRev2.wav.
- The graph has six engine components driven by signed RPM and acceleration, with
  component RPM fade ranges, RPM-to-pitch mapping, and volume mapping.
- Reverse components use negative RPM.
- The WAVs have no smpl loop; looping belongs to the GBX/runtime graph.
- Surface sound consumes normalized speed and surface ID.
- Skid sound additionally requires skid intensity.
- Useful skid sources are SkidAsphalt.wav, SkidMud.wav, and SkidSand.wav under
  GameData\Vehicles\Media\Audio\Sound\WavData.

Smallest source-faithful order:

1. Recover signed RPM, gear, and drivetrain output from the vehicle tuning path.
2. Implement the six-component engine mixer.
3. Persist contact surface and derive source-backed wheel slip/skid intensity.
4. Add surface/skid mixing.

Until then, the honest audio boundary is checkpoint/finish one-shots.

## Current runtime controls

- WASD or D-pad: steer, accelerate, brake.
- R or gamepad Y: respawn.
- P, Menu, gamepad Start, or upper-left M touch target: toggle map selector.
- Selector left/right: previous/next map.
- Enter, gamepad A, or center touch: load selected map.
- Touch while driving: horizontal steering; upper/lower screen throttle/brake.

## Remaining work, in priority order

1. Prove and implement original countdown, timer zero, and input release.
2. Recover exact Stadium drivetrain/vehicle tuning and replace the stand-in physics.
3. Implement source-backed engine, surface, and skid audio from recovered state.
4. Add minimal Author ghost playback, then validate all 65 replay files.
5. Expand offline menu/results/restart flow only where source behavior requires it.
6. Traverse all 65 maps on Android 15 and record load/render/crash evidence.
7. Re-run host, reference, Android build, 16 KiB, install, persistence, and lifecycle
   checks for the final package.

Animated Bink sign textures are intentionally unbound; geometry remains visible.
Video decoding, editors, and online systems are not the next critical path for the
offline campaign.

## Resume checklist

1. Confirm the goal is intentionally resumed and reactivate the existing heartbeat;
   do not create a duplicate automation.
2. Read this file plus README.md and the two docs files.
3. Run git status --short and preserve the untracked workspace.
4. Run the host build/tests before editing.
5. Take the countdown callsite trace as the next bounded task.
6. Do not launch an emulator until device-only proof is needed.
7. If launched, use both -no-window and -WindowStyle Hidden, then prove cleanup.

