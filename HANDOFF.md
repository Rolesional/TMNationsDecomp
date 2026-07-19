# TMNF Reconstruction and Android Port Handoff

Last verified: 2026-07-18 15:59 CDT

Workspace: `D:\Stuff\DevFolder\TMNationsDecomp`

This is the authoritative restart document for reconstruction of the locally supplied
TrackMania Nations Forever 2.11.11 executable and its native Android 15 port. Read this
file before claiming source coverage, changing physics, or launching an Android target.

## Active goal and coverage checkpoint

- The active `/goal` is **100% executable code-path coverage in reconstructed C/C++**.
  It remains active and is not complete.
- The authoritative Ghidra inventory contains 67,840 unique internal function entry
  addresses. Completion requires all 67,840 to have a raw decompiler export and a
  verified row in `reverse/reconstruction-manifest.csv` pointing to buildable C/C++ and
  runnable evidence.
- Current exact coverage is 12,544/67,840 raw exports (18.4906%) and 43/67,840 verified
  reconstructions (0.0634%). Verified low-level clusters now include localization,
  archive RAII wrappers, `CGbxGame` dispatch and ordered shutdown, its unnamed vslot
  `+0x94` predicate, one-parameter `CGbxApp` fast callbacks, intrusive references,
  nullable fast-string construction, narrow/UTF-16 fast-string storage and assignment,
  the permissive one-to-six-byte UTF-8 decoder and BOM-sensitive UTF-16 conversion,
  typed array access, and vector-pair initialization. The next raw address is
  `0x006004E0`; the next verified-reconstruction address is `0x00401E80`.
- `scripts/check-decomp-coverage.ps1` validates inventory/export/manifest identity.
  `-RequireComplete` deliberately fails until both counts equal 67,840. Ghidra pseudocode,
  an address in a comment, or a compatible behavior without source/test ownership does
  not count as reconstructed.
- `scripts/decompile.ps1 -Missing -Limit 500` resumes from the next missing raw export.
  Raw exports and the Ghidra project remain ignored local analysis artifacts; the compact
  manifest, reconstructed source, tests, and selected disassemblies are tracked evidence.
- Android P1 remains **A01 fully playable end to end**, but it is no longer the completion
  criterion for the broader reconstruction goal.
- The user can finish A01 and explicitly said not to have an agent play the map.
  Do not script-drive, replay-drive, or manually drive A01 as a correctness oracle.
  Recover behavior from the executable/data, implement it, and prove it with focused
  host tests and bounded Android startup checks.
- The current source-first collision/camera/Go-launch checkpoint is green: the host build passes,
  CTest is 17/17, the complete local reference-data gate passes, and both Android
  ABIs build.
- A hidden API 35 cold-start check loaded A01 and initialized the native renderer,
  simulation, HUD, and audio graph without an Android fatal. The car was not driven.
- This checkpoint launched no emulator. End cleanup proof is
  `EMULATOR_DEVICES=0`, `EMULATOR_PROCESSES=0`, and `ADB_PROCESSES=0`.
- Recovery heartbeat `tmnf-android-port-recovery` is active. Continue using it; do
  not create a duplicate.

Latest debug APK:

- Path: `app\build\outputs\apk\debug\app-debug.apk`
- Size: 9,467,030 bytes
- SHA-256: `3769E614E11F7F52FD475D38FF3C8C3E96D5DF285114D4EDBB7193EA6D959F9D`

## Repository safety

- Current branch/checkpoint: `main` at
  `67332465b5c549e51e94d775d030b12a44c11c29`.
- The worktree is heavily dirty (112 modified/untracked entries at this checkpoint).
  All existing changes are live user/project work. Preserve them.
- Never use `git clean`, `git reset --hard`, `git checkout --`, or bulk deletion.
  Do not stage or commit unless the user explicitly asks.
- `TmNationsForever` is the user's immutable reference installation and is ignored.
  Never edit it.
- Do not add the original executable, archives, tracks, replays, or copyrighted
  assets to the repository or APK.

Reference executable:

- `TmNationsForever\TmForever.exe`
- Version: 2.11.11
- SHA-256: `808BF7FED8D74E54950129BB47C6F0A65A25C05415D49E280760DB68527FC26F`

## Product/data boundary

The APK is not self-contained. On an Android device the user should copy the
original `TmNationsForever` folder to an ordinary readable location such as
`Internal storage/Download/TmNationsForever`, install the APK, launch it, and select
that folder in the Android folder picker. `LauncherActivity` validates it and imports
the required `Packs` and `GameData` trees into app-private storage, then creates
`.tmnf-assets-ready` only after a complete import.

Do not tell users to populate `/sdcard/Android/data` manually. Android 15 ownership
and labeling can make such files unreadable to the app. The source assets remain
user-supplied and are deliberately absent from the APK.

## Current native runtime

The port is portable C++ hosted by Android GameActivity, targeting SDK 35 with GLES
3 rendering, AAudio, scoped-storage import, and touch/keyboard/gamepad input. The
official campaign parser resolves 65 playable Nations maps; the one stale campaign
UID remains deliberately omitted.

Player-facing work already present:

- Source countdown timing and input/timer release for 3, 2, 1, GO.
- Correct platform-boundary steering direction.
- Four simultaneous on-screen buttons: left/right on the lower left and
  forward/reverse on the lower right.
- Full vehicle quaternion rendering so the car follows road pitch, roll, and yaw.
- The ordinary Race3 loose-follow chase camera with source reset geometry, signed-speed
  response curve, spherical direction/up filters, distance spring, and vehicle-up basis.
- Checkpoint respawn restores a road-aligned prepared state rather than choosing the
  top of the checkpoint arch.
- Valid finish writes the PB, waits two seconds, and loads the next campaign track.
- Original StadiumCar low-LOD body and four wheel draw ranges. Front-wheel steering
  now follows the exact authored speed curve and smoothed physics steering;
  suspension articulation is not yet recovered.
- LED-atlas numeric speed HUD.
- Original six-layer Stadium engine sound graph plus race cues.

This is still a partial compatibility port. Do not call the vehicle physics fully
recovered merely because A01 can be finished.

## Track collision/material correctness checkpoint

The invisible-contact root fixed in this checkpoint was source data that the portable
parser had flattened or discarded, not bad A01 placement transforms:

- `CPlugSurface` stores a local material table for the geometry it references. A solid
  can contain several such tables. Each collision shape now records its own
  `[offset,count]` range; a triangle's local material index is never applied to another
  table.
- Referenced `CPlugMaterial` slots were previously stored as `-1`, causing every A01
  collision triangle to fall back to a shape-level surface 0. The VFS path now resolves
  the numeric surface ID from the material's `0x0907900E` chunk. All 53 unique A01
  material references resolve.
- Native surface 28 is `NotCollidable`. Those triangles are omitted before the portable
  collision grid is populated. Ordinary-looking source materials using it include
  `StadiumWarpSpotsGlow`, `StadiumWarpSpotsGlowBack`, and `StadiumStartSignGlow`.
- The correct A01 removal is exactly 136 triangles: Start Air 64, Finish Air 40, and
  Checkpoint RingVAir 32. A tempting 2,904-triangle result was rejected because 2,792
  Start triangles came from incorrectly flattening two local material tables.
- Current A01 collision totals are 98,189 vertices, 528,984 indices, 176,328 triangles,
  and 359 occupied cells. The start ground is Asphalt (surface 16). The exact 31-entry
  triangle histogram for IDs 0..30 is:

  ```text
  11796,0,528,684,65862,0,0,0,0,71232,276,0,0,0,0,0,25906,0,0,0,0,0,44,0,0,0,0,0,0,0,0
  ```

- A static parity audit found an enabled rendered counterpart for every retained A01
  collision triangle, with matching placement and transform bases. The recovered
  `FUN_008C1C70` one-sided narrowphase remains the authority; do not add guessed seam,
  winding, or vertical-wall filters.

The focused `track_collision_test` fixture locks mixed inline/reference material IDs,
per-shape local-table ranges, shape fallback behavior, and surface-28 removal. The full
reference-data gate locks the A01 totals, histogram, and Asphalt start surface.

## A01 Go-launch regression

The immediate upward launch was a solver timestep bug, not a bad start triangle or body
penetration. A bounded real-A01 trace found four shallow Asphalt wheel contacts
(about 1.5 mm each) and no body contact. A `0.6663 us` frame tail was nevertheless
accepted as its own physics step after the first 1/120-second tick. Dividing the pending
suspension correction by that tail produced wheel speeds below `-2,246 m/s`, about
`+8,980` net vertical force, a persisted rear load-transfer torque, and then a two-front-
wheel impulse that reversed vertical velocity and launched the chassis.

`VehicleSimulation::update` now accumulates released time and consumes only uniform
1/120-second physics ticks; reset/restore clears the fractional accumulator. This fixes
the numerical cause before suspension damping, rather than clamping its later impulse.
Reset also matches native `0x009D9590`/`0x009D6430`: each wheel starts at authored rest
length with zero pending correction/speed and no seeded support/contact.

The exact pre-fix diagnostic pulse was throttle from `0` through `8.334 ms`, then idle.
Before the fix A01 reached `y=91.0445`, `vy=+1.51601` at 0.2 seconds. After the fix it
remained grounded, reached `y=90.0243`, `vy=-0.709629`, settled to `y=90.0158`, stayed
finite/deterministic, and never entered the upward-launch sequence. This is a bounded
first-step diagnostic, not an A01 completion drive.

## Source-exact ordinary Race3 camera

The Android renderer no longer owns a provisional independent XYZ spring. Portable
`Race3Camera` implements the ordinary source path owned by
`CGameControlCameraTrackManiaRace3`:

- Constructor `0x006F1E10`, reset `0x006F3880`, update `0x006F41E0`.
- Reset/neutral eye `P + 2*U - 4*F`; target `P + U`; view-up is the smoothed vehicle up.
- Direction fraction `speedGain * clamp(5*dt/(2*theta+1),0,1)`.
- Up fraction `speedGain * clamp(4*dt/(2*phi+1),0,1)`.
- Signed-speed curve X `[-100,-50,0,250,500,700]` km/h and
  Y `[1.5,1.2,1,1.5,2,3]`.
- `dt` caps at 0.2 seconds and a backwards clock yields zero.
- Neutral distance is `sqrt(20)` with scalar spring
  `v += dt*((sqrt(20)-x)*10-v*5)`, then `x += dt*v`.
- Ordinary vertical FOV is 90 degrees (`this+0x2C` plus zero ordinary delta). Turbo
  eventually adds a cosine-ramped 10.5-degree delta, with 1000 ms rise and 1400 ms fall;
  that event state is not implemented yet.
- Vehicle position `P` is read directly each frame. No camera-owned world XYZ spring or
  base-class post-filter exists; looseness belongs only to relative direction/up/distance.
- The slower 3/1 angular gains belong only to a special internal mode. There is no
  invented distance-based camera reset; attach, map load, spawn, and respawn reset it.

`race3_camera_test` locks reset geometry, the 90-degree FOV, ordinary 5/4 filtering, the
signed-speed gain, backwards-clock behavior, and explicit reset. Android consumes the
shared camera frame and full vehicle quaternion; it does not retune the source equations
in GLES code.

## Source-correct vehicle checkpoint

### Parsed tuning and solid data

- `CPlugVehiclePhyTunings` legacy data is parsed natively. All 30 records load and
  active index 29, `20juin2006`, is selected.
- Previously skipped active fields are now retained and reference-locked:
  `TurboBoost=5`, `Turbo2Boost=20`, `TurboDuration=250 ms`,
  `Turbo2Duration=100 ms`, and visual-steer curve X `[0,100]` km/h,
  Y `[30,10]` degrees.
- StadiumCar collision data is supplied to `VehicleSimulation` from the parsed
  vehicle solid. Four authored body ellipsoids are live:

  | Shape | Half axes |
  | --- | --- |
  | `BodySurf` | `(0.439118, 0.362, 1.90153)` |
  | `BodySurf01` | `(0.968297, 0.362741, 1.68228)` |
  | `BodySurf02` | `(1.02092, 0.515218, 1.03801)` |
  | `BodySurf04` | `(0.384841, 0.905323, 0.283418)` |

  Their authored rotations/translations are parsed and locked by
  `scripts/test-reference-data.ps1`; do not replace them with a bounding sphere.
- Wheel order and force points remain `FLSurf`, `FRSurf`, `RRSurf`, `RLSurf`.
  Wheel ellipsoid half axes are `(0.182, 0.364, 0.364)`.
- Active body coefficients are concrete friction `0.3`, metal friction `0.2`, and
  stored concrete/metal rest values `-0.5`. The contact callback negates the stored
  value to obtain restitution `0.5`.

### Wheel suspension and tire/yaw forces

- The recovered wheel collision chain is
  `00699B90 -> 008BDF00 -> 008C1C70`. It uses one-sided triangle/ellipsoid
  narrowphase, not a height-only road sample.
- Suspension preserves the source's one-step contact latency: consume prior contact
  correction, apply the spring at each wheel point, integrate, then gather the next
  four contacts.
- Model-5 partial compliance absorbs positive local-Y correction into remaining
  suspension travel and leaves residual body correction. It uses the active wheel
  center lever for the impulse path.
- Tire side-force ownership is per wheel and uses the previous slip state for the
  sliding/brake multipliers. Lateral slope adherence applies to the side-force cap;
  the later axle/yaw pass intentionally does not reuse it.
- Concrete and asphalt Stadium vehicle-material multipliers are both exactly `1`.
- The yaw pass uses all four wheels, the source cap/blend, and the exact half-wheelbase
  expression `(frontZ - rearZ) * 0.5`.
- Low-speed steering torque is zero below `0.7 m/s`, then follows the recovered sine
  ramp to `steerLowSpeed`. Front-wheel direction comes from the persistent reverse
  latch below, not the instantaneous sign of velocity.
- Exact steering slowdown data is parsed: curve X
  `[-50, 0, 50, 100, 200, 300]`, Y `[1, 0, 0, 0, 0, 0]`, coefficient `0.5`.
- `FUN_009DC910` rejects Water (surface 13) and GolfBall (surface 23) before the
  wheel callback. Portable wheel sampling now does the same: those contacts provide
  no wheel support, correction, or impulse. This is separate from the already-live
  body-contact suppression for the same two IDs.
- One wheel-contact mismatch remains and must not be hidden: native
  `009DC910 -> 009DA440` invokes the callback for every accepted triangle in order,
  mutating wheel state and body velocity after each callback. The portable gather still
  selects one maximum suspension-axis contact. Native support is not ORed: the last
  callback overwrites the final support flag; support count/normal sum aggregate all
  supporting contacts; material is last-support; point is last non-suppressed contact;
  positive local-Y absorption is max-reduced per wheel while each residual correction
  remains in the ordered outer reduction.
- The outer trace is also complete: `FUN_006847C0` appends every callback-mutated body
  and wheel correction to one list owned by the physical rigid body; `FUN_006907F0`
  calls ordered reducer `FUN_0068FF90` once after full dispatch and applies one pose
  translation. The current portable shape-by-shape API does not yet prove native
  collision-pair interleaving. Do not replace the one-best query with an unordered loop,
  guessed average, or separate per-wheel reducer. First expose/lock encounter order in a
  focused multi-shape fixture, then switch the whole car to one correction list.

### Source-exact front-wheel visual steering

`FUN_009DCE00` owns the visual steering angle. It obtains local forward velocity,
samples `visualSteerAngleFromSpeed` at `abs(localForwardMps) * 3.6`, converts the
result from degrees to radians, and multiplies it by the smoothed tire steering.
The portable coordinate-sign conversion reduces to:

```text
frontWheelRadians = smoothedSteering
                  * curve(abs(localForwardMps) * 3.6)
                  * pi / 180
```

`VehicleSimulation::frontWheelSteeringRadians()` owns that calculation. Android now
passes the final angle to the renderer, which applies it directly around each parsed
front-wheel tree pivot. The old provisional `control.steering * 0.45` renderer
shortcut is gone. The focused regression locks 30 degrees at rest, 10 degrees at
100 km/h, reverse-speed absolute value, and steering sign.

### Body collision callback

The former bounded sphere response has been removed. Current behavior follows the
recovered body-contact callback boundary:

1. Compose each authored ellipsoid into the car's post-integration world pose.
2. Enumerate every accepted `008C1C70` contact in deterministic authored-shape and
   collision-grid order.
3. For surface IDs 13 and 23, zero correction and skip response/collision pulse.
4. For every other contact, recompute point velocity after earlier contact impulses.
5. Split point velocity into normal and tangent components, clamp tangent magnitude
   to `friction * abs(normalVelocity)`, and use the normalized negative sum as the
   impulse direction.
6. Apply the exact world-space effective-mass linear/angular impulse. Surface 4 uses
   metal coefficients; other surfaces use concrete coefficients.
7. Reduce all correction vectors in source order, apply the final 1 cm slop, and
   translate the body once per substep.

The ordered correction reducer is important. Starting from the first correction
`acc`, each later vector `v` removes only its component already covered by `acc` when
`dot(acc,v) > 0`, with the projection capped at `|acc|^2`, then adds the remainder.
After all contacts, lengths at or below 1 cm become zero; longer results are scaled by
`1 - 0.01/length`.

The callback's `+0x3C` flag gates the later generic impulse, not correction queuing.
Surface 13/23 suppression works because their correction fields are explicitly
zeroed. Do not reintroduce a global collision sphere or a deepest-contact-only body
reducer.

### Persistent reverse-direction latch

`VehicleState::reverseDirectionLatched` implements source field `+0x5C4`. It updates
only on a supported/grounded step, in this exact order:

1. If brake `> 0.1`, local Z `< 10`, and `abs(local X) < 2`, set the latch.
2. If throttle `> 0.1` and either local Z `> 0` or `abs(local X) > 2`, clear it.
3. If throttle `< 0.1` and brake `< 0.1`, overwrite it with `local Z <= -2`.

At exactly `0.1`, none of the threshold rules fires. Reset/checkpoint restart clears
the latch; ordinary saved-state restoration preserves it.

Two executable-owned overrides are intentionally not guessed yet:

- `car+0x69C` is the Burnout/Donut/AfterBurnout enum (0..3); nonzero clears the latch.
- `car+0x600` is active Turbo/Turbo2/TurboRoulette state; active turbo with local
  Z `> 0` clears the latch.

### Recent-contact airborne angular control

The branch order and active constants from `009D83A0` are live:

- Duration: 30,000 ms.
- Maximum Y angular speed for opposite-yaw hold: `3.14`.
- Z multiplier curve: constant `1` for the active tuning.
- Angular drag magnitude: `0.4*speed + 0.2*speed^2`.

Ground contact records the current local angular velocity and contact time. A body
collision pulse refreshes the cached velocity but deliberately not the timestamp and
bypasses the hold/boost curve for that step. During the 30-second recent-contact
window, opposite steering holds cached yaw below the `3.14` threshold; above it the
drag is tripled and the cache advances. Same-direction steering/cached yaw also uses
the tripled drag. Brake clamps a positive cached local-X pitch to zero. The adjusted
local angular velocity is written before the drag torque is integrated.

Portable state owns `cachedLocalAngularVelocity`, `lastWheelContactSeconds`, and
`bodyCollisionPulse`. Body collision discovery occurs after integration, so its pulse
is consumed on the next substep, matching the recovered engine chronology.

## Recovered stateful physics boundaries not yet implemented

### Turbo, Turbo2, TurboRoulette, and FreeWheeling

The trace is complete enough to implement without guessing:

- `FUN_009D5F50` owns activation, `FUN_009D5420` owns expiry/progress, and
  `FUN_009DFFD0` dispatches contacted materials after the model-5 force step.
- `car+0x600` is mode `0=None`, `1=Fixed`, `2=Roulette`. Separate field
  `car+0x60C` is the persistent FreeWheeling latch.
- The first supporting wheel matching each requested surface is used. Dispatch order
  is Turbo 7, Turbo2 26, TurboRoulette 30, then FreeWheeling 29; a later boost type
  wins when different pads are touched simultaneously.
- Fixed pads use strengths/durations `5/250 ms` and `20/100 ms`. Roulette uses the
  base strength 5 and a synchronized 1000 ms period, choosing multiplier `1`, `1.5`,
  or `2` in phase buckets `[0,4/7)`, `[4/7,6/7)`, and `[6/7,1)`.
- Repeated roulette contact with the same source extends expiry without rerolling.
  A different source rerolls. Android therefore needs a stable contacted-object or
  trigger index, not a transient pointer.
- Model 5 computes `turboTerm = selectedAccelerationCurve(localZ*3.6) * strength`
  and adds it to ordinary longitudinal force. FreeWheeling replaces all ordinary
  propulsion with that turbo term (or zero), while later brake and speed-limit logic
  still runs. Active turbo with positive local Z clears the reverse latch.
- Material activation happens after the current model-5 force calculation, so a new
  pad affects the following physics tick. Preserve this latency.
- Checkpoint/reset clears mode, progress, strength, and FreeWheeling. Lifecycle save
  must retain active mode/strength/times, roulette clock/phase/source, and
  FreeWheeling in the same race-clock domain.

After wheel multi-contact parity, this is the best next bounded stateful slice. Add
explicit portable state and save validation first, then contact material/source
plumbing, then force ownership and one-tick-latency fixtures.

### Burnout, Donut, and AfterBurnout

`FUN_009DD380` exclusively owns the active model-5 state machine at `car+0x69C`:
`0=None`, `1=Burnout`, `2=Donut`, `3=AfterBurnout`. Dispatcher
`FUN_009DFFD0` calls it only for model 5. Do not collapse the enum into the distinct
`car+0x6A0` latch; the latch survives AfterBurnout until its timer expires and also
owns burnout wheel spin/visual activity.

The trace includes all entry/exit order, tuning offsets, tire/friction envelopes,
donut pivot/normal/radius/sign state, force/torque formulas, RPM mode, reset/save
requirements, and two confirmed native quirks:

- Active-Donut radius acceptance uses the odd source predicate
  `(entryBaseRadius < currentRadius) || (currentRadius < radiusMax)`.
- The active AfterBurnout impulse uses unsigned integer `elapsed/duration`; because
  the state exits at duration, the source impulse is constant each active frame.

Implement this only as its own checkpoint with state/save migration and direct
transition/force fixtures. A partial control-condition approximation would be less
correct than leaving it explicitly absent.

## Honest remaining source gaps

Prioritize these from executable/data evidence, not by retuning A01:

1. Replace the one-best wheel collision sample with the exact ordered multi-contact
   callback semantics described above. Preserve immediate per-contact impulses, support
   count/normal sum, last-contact ownership, max suspension absorption, and ordered
   one-list-per-rigid-body residual reduction. The missing proof is collision-pair
   interleaving, not callback field semantics. This is the active collision priority.
2. Add the native countdown's collision-only frozen pass before claiming general start
   parity. A01's shallow flat overlap has zero residual pose correction and is cleared at
   Go, so the current P1 result is equivalent; deeper or nonvertical campaign starts may
   retain a native chassis correction that the portable all-frozen countdown omits.
3. Implement the fully traced Turbo/Turbo2/TurboRoulette and FreeWheeling state,
   material dispatch, one-frame contact latency, drivetrain override, reset, and
   lifecycle persistence described above.
4. Implement the fully traced but substantially larger
   Burnout/Donut/AfterBurnout state machine as a separate checkpoint.
5. Continue the remaining model-5 tire paths: material grip lookup, longitudinal
   slip/skid ownership, and the large Dirt-specific branch. Water/GolfBall wheel and
   body rejection are now closed.
6. Recover the remaining crash/rollover state transitions and impact outputs around
   `FUN_009D98E0`/`FUN_009DFFD0`. The authored body narrowphase and callback impulse
   are now present, but the entire original vehicle solver is not.
7. Recover vertical wheel/suspension articulation. Visual steering angle and parsed
   wheel pivots are now source-backed.
8. Confirm the original speedometer rounding rule and prove audible output on real
   Android hardware. The hidden emulator used `-no-audio`.

Replay/ghost playback, broader result UI, and full campaign compatibility remain
post-P1 Android work. The broader reconstruction goal proceeds by verified inventory
entry or coherent source-owned clusters; do not confuse Android priority with total
executable coverage.

## Validation evidence at this checkpoint

Host/core:

- `scripts/build-host-tools.ps1`: configure/build succeeded.
- SDK CTest: 17/17 passed, 0 failed, including `race3_camera_test` and the recovered
  legacy wrapper/storage primitives.
- `scripts/check-decomp-coverage.ps1`: inventory 67,840; raw 12,544 (18.4906%);
  verified reconstructed 43 (0.0634%). `-RequireComplete` fails as required.
- Focused vehicle tests cover authored body reach/orientation, surface 13 suppression,
  centered restitution, off-center effective mass/angular impulse, reverse-latch
  thresholds and persistence, airborne yaw hold/boost, collision-pulse precedence,
  brake pitch clamp, grounded history seeding, Water/GolfBall wheel rejection, and
  exact speed-dependent visual-wheel steering. The A01-height fixture also locks that a
  full tick plus a microsecond tail is equivalent to uniform accumulated ticks, so the
  suspension derivative cannot consume the tail directly.
- The bounded real-A01 one-tick throttle diagnostic stayed grounded and deterministic,
  settled to `y=90.0158`, and had `firstAirSeconds=-1`.
- `git diff --check` on the relevant source/test/parser/handoff files was clean after
  this handoff rewrite.

Reference corpus:

- WAV: 12/12.
- Loose GBX headers/bodies: 157/157; 141 LZO and 16 raw bodies.
- Challenges: 72/72, with 50,576 blocks, 6,187 skinned blocks, and 247 checkpoints.
- Campaign: 65/66 resolved, with one known stale UID.
- Pak/archive/index gates: 3/3 packs, 1/1 pack list, 3/3 indexes.
- A01 portable scene: 373 placements, 33 models, 33 textures,
  635,640 render indices, 528,984 collision indices, 176,328 collision triangles,
  and 359 occupied collision cells.
- All 53 referenced A01 `CPlugMaterial` surface IDs resolved; the exact surface
  histogram and Asphalt start surface matched the gate.
- StadiumCar: low LOD 7/87, four wheels, 32/32 construction graphs.
- Engine graph: 6/6 components and 8/8 root curves.
- Active vehicle tuning, exact turbo/duration/visual-steer fields, and all four
  `BodySurf*` transforms matched the reference.
- C11 correctly retained its expected Bink fallback.

Android/API 35:

- `scripts/build-android.ps1` / Gradle `:app:assembleDebug`: successful ARM64-v8a and
  x86_64 native compilation and APK packaging with the collision/material/camera and
  fixed-timestep/reset changes.
- This checkpoint did not launch an emulator; no device-only invariant required it.
- Previous bounded startup evidence, not rerun here: hidden AVD `TMNF_API_35`, target
  SDK 35. Its 176,464-triangle log predates the source-28 fix and is retained only as
  startup evidence, not as the current collision oracle.
- Launch transitioned from `LauncherActivity` to `TmnfActivity` using an existing
  validated `.tmnf-assets-ready` import.
- Native logs reported the 65-map campaign, A01 with 397 blocks, 373 visible
  placements, 33 models/textures, 176,464 pre-fix collision triangles, 132 trigger triangles,
  two checkpoints, six race sounds, six engine layers, AAudio at 48 kHz, HUD ready,
  and GLES ready at 2400x1080 on SwiftShader.
- Scene upload reported 264,735 vertices, 635,640 indices, and 36 textures;
  StadiumCar reported 3,506 vertices, 10,875 indices, and seven draws.
- No `FATAL EXCEPTION`/AndroidRuntime fatal was found.
- A01 was not driven, by explicit user instruction.
- Final cleanup: `EMULATOR_DEVICES=0`, `EMULATOR_PROCESSES=0`, and
  `ADB_PROCESSES=0`.

Optional texture warnings for `SpecularCubicPC1.dds` and
`DefaultEnvmapPC0.dds` remain nonfatal fallback behavior.

## Main code map

| Path | Ownership |
| --- | --- |
| `core/include/tmnf/legacy/*.hpp` | Verified low-level engine primitives and scoped wrappers |
| `reverse/reconstruction-manifest.csv` | Verified executable-address to source/test ownership |
| `scripts/check-decomp-coverage.ps1` | Fail-closed raw and reconstructed coverage accounting |
| `core/src/gameplay/vehicle.cpp` | Shared vehicle forces, contacts, state, and race physics |
| `core/src/gameplay/race3_camera.cpp` | Source Race3 reset and loose-follow equations |
| `core/src/content/track_collision.cpp` | Grid queries and exact triangle/ellipsoid contacts |
| `core/src/content/materials.cpp` | Visual textures and referenced collision-surface IDs |
| `core/src/gbx/vehicle_tunings.cpp` | Legacy 30-entry vehicle-tuning parser |
| `core/src/gbx/solid.cpp` | Packed solid/tree/visual/surface parsing |
| `tests/vehicle_simulation_test.cpp` | Focused source-semantics regression |
| `tests/track_collision_test.cpp` | `008C1C70` contact/reducer fixtures |
| `tests/race3_camera_test.cpp` | Race3 geometry/filter/speed/reset fixture |
| `tools/archive_track_scene.cpp` | Reference scene/tuning/solid inspection oracle |
| `scripts/test-reference-data.ps1` | Full local reference-corpus gate |
| `app/src/main/cpp/native_main.cpp` | Android lifecycle, scene/simulation/race loop |
| `app/src/main/cpp/platform/android/egl_renderer.cpp` | GLES track/car/camera/HUD rendering |
| `app/src/main/cpp/platform/android/audio_output.cpp` | AAudio race and engine mixing |
| `app/src/main/java/com/tmnf/port/LauncherActivity.java` | Folder validation/private import |
| `app/src/main/java/com/tmnf/port/TmnfActivity.java` | GameActivity host and four-button overlay |

Useful recovered source:

- `reverse/ghidra/exports/decompiled/008c1c70_FUN_008c1c70.c`
- `reverse/ghidra/exports/disassembled/009d83a0_FUN_009d83a0.asm`
- `reverse/ghidra/exports/disassembled/009dce00_FUN_009dce00.asm`
- `reverse/ghidra/exports/disassembled/009dffd0_FUN_009dffd0.asm`
- `reverse/ghidra/exports/disassembled/009dd380_FUN_009dd380.asm`
- `reverse/ghidra/exports/disassembled/009da440_FUN_009da440.asm`
- `reverse/ghidra/exports/decompiled/009d5f50_FUN_009d5f50.c`
- `reverse/ghidra/exports/decompiled/009d5420_FUN_009d5420.c`
- `reverse/ghidra/exports/defined-strings.csv`

Recovered asset chain:

- `Vehicles\TrackManiaVehicle\StadiumCar.ConstructionVehicle.Gbx`
- `Vehicles\Mobil\Vehicle\StadiumCar.Car.Vehicle.Gbx`
- `Vehicles\VehicleTunings\StadiumCar.VehicleTunings.Gbx`
- `Vehicles\VehicleStructs\StadiumCar.VehicleStruct.Gbx`

## Build and host checks

From the workspace root:

```powershell
Set-Location D:\Stuff\DevFolder\TMNationsDecomp
.\scripts\build-host-tools.ps1
.\scripts\test-reference-data.ps1
.\scripts\build-android.ps1
```

`build-host-tools.ps1` already selects the SDK CMake/CTest. For a direct run use:

```powershell
C:\Users\Tyler\AppData\Local\Android\Sdk\cmake\3.22.1\bin\ctest.exe `
    --test-dir .\build\host --output-on-failure
```

Do not use the bare `ctest` on this machine; it may resolve to devkitPro/MSYS and
turn valid Windows test paths into `BAD_COMMAND`.

Prerequisites currently used:

- Android SDK/API 35 and NDK 29.0.14206865.
- SDK CMake 3.22.1 and Ninja.
- JDK 21; scripts prefer `.tools\jdk-21.0.11+10`.
- `C:\msys64\mingw64\lib\libz.a` for the host build.

## Emulator rule: mandatory hidden launch and cleanup

Do not launch an emulator for host/source work. When device-only validation is
necessary, the user must not see its window. Every launch requires both the
`-no-window` emulator argument and `Start-Process -WindowStyle Hidden`.

Minimal known launch:

```powershell
. .\scripts\toolchain.ps1
$emulator = Join-Path $env:ANDROID_HOME 'emulator\emulator.exe'
$args = @(
    '-avd', 'TMNF_API_35',
    '-port', '5554',
    '-no-window',
    '-no-boot-anim',
    '-no-snapshot-load',
    '-no-snapshot-save',
    '-gpu', 'swiftshader_indirect',
    '-no-audio'
)
$emulatorProcess = Start-Process -FilePath $emulator -ArgumentList $args `
    -WindowStyle Hidden -PassThru

Get-CimInstance Win32_Process |
    Where-Object ProcessId -EQ $emulatorProcess.Id |
    Select-Object ProcessId, Name, CommandLine
```

Do not proceed unless the live command line proves `-no-window`. Install through
`com.tmnf.port/.LauncherActivity`; do not launch non-exported `TmnfActivity` directly
and do not push reference files into `Android/data`.

Always clean up in the same work cycle, including failure paths:

```powershell
$adb = Join-Path $env:ANDROID_HOME 'platform-tools\adb.exe'
$serial = 'emulator-5554'
& $adb -s $serial emu kill 2>$null
Start-Sleep -Seconds 3

Get-CimInstance Win32_Process |
    Where-Object Name -Match '^(emulator|qemu-system.*)\.exe$' |
    ForEach-Object { Stop-Process -Id $_.ProcessId -Force }

$emulatorDevices = @(& $adb devices) -match '^emulator-'
$emulatorProcesses = @(
    Get-CimInstance Win32_Process |
        Where-Object Name -Match '^(emulator|qemu-system.*)\.exe$'
)
Write-Output "EMULATOR_DEVICES=$($emulatorDevices.Count)"
Write-Output "EMULATOR_PROCESSES=$($emulatorProcesses.Count)"
if ($emulatorDevices.Count -ne 0 -or $emulatorProcesses.Count -ne 0) {
    throw 'Emulator cleanup failed'
}
& $adb kill-server
```

The required stop state is zero devices, zero emulator/QEMU processes, and no adb
server left running.

## Continuation sequence

1. Read this file, `README.md`, `docs/architecture.md`, and
   `docs/reverse-engineering.md`.
2. Inspect `git status` and preserve every existing change.
3. Run `scripts/check-decomp-coverage.ps1`; never infer a percentage from source size or
   Ghidra output count alone.
4. Resume raw exports with `scripts/decompile.ps1 -Missing -Limit 500` as useful. The
   next raw address at this checkpoint is `0x006004E0`.
5. Reconstruct the next unverified entry (`0x00401E80`), the `CGbxGame` constructor,
   from raw C, exact disassembly, base-constructor ownership, initialized field offsets,
   and unwind behavior. Prefer the standard library where it exactly replaces a
   recovered primitive.
6. Add a manifest row only after buildable C/C++ and a runnable check prove every branch
   of that function. Run the focused test, full 17/17-or-later host suite, coverage
   checker, and reference gate when runtime behavior changes.
7. Build the APK only after shared runtime/app changes. Do not play A01. If Android is
   needed, launch the emulator hidden and prove cleanup before pausing.

At the next pause, update this document with exact inventory/raw/verified counts, source
ownership, tests, and—when Android changed—APK hash and zero-emulator proof. Keep both
goals honest: runtime compatibility does not prove total source coverage, and raw
decompiler output does not prove reconstructed C/C++.
