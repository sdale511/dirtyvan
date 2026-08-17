# Pinout

This controller (`src/main.cpp` + `src/DvMotor.*`) drives one DC motor (e.g.
a van bed-lift actuator) from a 3-button panel (forward / reverse / kill),
through one of three interchangeable motor driver boards. Only one driver is
compiled in at a time, selected in `src/DvMotor.cpp` via `#define
MOTOR_L298N` / `MOTOR_CYTRON` / `MOTOR_BTS7960` (currently **`MOTOR_BTS7960`**
is active).

## Target boards (`platformio.ini`)

Any of these Arduino-framework boards can run this firmware - pin numbers
below are the standard Arduino digital pin numbering, valid on all of them:

| Environment | Board |
|---|---|
| `uno` | Arduino Uno |
| `zero` / `zero-dbg` | Arduino Zero (default env) |
| `nanoatmega328` / `nanoatmega328new` | Arduino Nano (ATmega328) |

## Control panel (`src/main.cpp`)

Always wired the same way, regardless of which motor driver is selected.

| Signal | Arduino pin | Mode | Notes |
|---|---|---|---|
| `LED_FWD` | 9 | `OUTPUT` | Lit while driving forward (CW) |
| `LED_REV` | 10 | `OUTPUT` | Lit while driving reverse (CCW) |
| `SWC_FWD` | 11 | `INPUT_PULLUP` | Forward pushbutton - momentary, active-low (pressed = `LOW`) |
| `SWC_REV` | 12 | `INPUT_PULLUP` | Reverse pushbutton - momentary, active-low (pressed = `LOW`) |
| `SWC_KILL` | 2 | `INPUT_PULLUP` | Limit/kill switch - normally closed; opening it (`HIGH`) stops the motor if it's currently retracting (CW) and blocks a new CW command |

Pushing FWD or REV a second time while already running that direction stops
the motor (toggle behavior); pushing the opposite direction while running
switches direction. A motor run auto-stops after `gMotorTimeout` (35s
default - sized for a ~30s bed-post travel time), blinking both LEDs for the
last `gMotorBlinkTimeout` (3s) as a warning.

## Motor driver boards

### BTS7960 (`RobojaxBTS7960` library) - **currently active**

A dual half-bridge (43A-class) driver module, used here as a single H-bridge
(both halves ganged for one motor). Constructed in `DvMotor.cpp` as
`RobojaxBTS7960(R_EN, RPWM, R_IS, L_EN, LPWM, L_IS, debug)`.

| Module pin | Arduino pin | Mode | Purpose |
|---|---|---|---|
| `R_EN` | 4 | `OUTPUT` | Right-side enable - driven `HIGH` before every `rotate()` |
| `RPWM` | 5 | `OUTPUT` (PWM) | Right-side PWM speed input |
| `R_IS` | 3 | `INPUT` | Right-side current-sense feedback (read but not acted on by this library) |
| `L_EN` | 7 | `OUTPUT` | Left-side enable - driven `HIGH` before every `rotate()` |
| `LPWM` | 6 | `OUTPUT` (PWM) | Left-side PWM speed input |
| `L_IS` | 8 | `INPUT` | Left-side current-sense feedback (read but not acted on by this library) |

Direction is set by which side (`RPWM` vs `LPWM`) actually receives the PWM
signal (the other stays `LOW`); `R_EN`/`L_EN` are just enable lines, both
held high whenever the motor is commanded to move.

### L298N (`src/motors/L298N.h`, this project's own class)

A classic dual H-bridge module, used single-motor here (one `ENA` speed pin
plus two direction pins). Constructed in `DvMotor.cpp` as `L298N(PIN_PWM,
PIN_FWD, PIN_REV)`.

| Module pin | Arduino pin | Mode | Purpose |
|---|---|---|---|
| `ENA` | 6 | `OUTPUT` (PWM) | Motor speed (PWM) |
| `IN1` | 7 | `OUTPUT` | Direction control 1 |
| `IN2` | 8 | `OUTPUT` | Direction control 2 |

Note this reuses the same Arduino pins (6/7/8) as BTS7960's `LPWM`/`L_EN`/
`L_IS` above - fine, since only one driver is ever compiled in at a time,
but worth knowing if you're switching between them on the same wiring.

### Cytron MD10C / MD13S (`src/motors/CytronMotorDriver.h`)

A single-channel PWM+DIR driver. Constructed in `DvMotor.cpp` as
`CytronMD(PWM_DIR, PIN_CYTRON_PWM, PIN_CYTRON_DIR)`.

| Module pin | Arduino pin | Mode | Purpose |
|---|---|---|---|
| `PWM` | 3 | `OUTPUT` (PWM) | Motor speed + direction magnitude/sign (`setSpeed(-255..255)`) |
| `DIR` | 2 | `OUTPUT` | Direction |

**⚠ Pin conflict if this driver is ever enabled**: `PIN_CYTRON_DIR` (2) is
the same Arduino pin as the control panel's `SWC_KILL` switch. Re-route one
of them before switching `DvMotor.cpp` to `#define MOTOR_CYTRON`.

## Legacy root sketch (`dirtyvan.ino`)

The `dirtyvan.ino` file at the repo root is an older, standalone prototype
that predates the `DvMotor` abstraction above - it drives a single L298N
directly via the third-party `Robojax_L298N_DC_motor` library (from
`examples/`), **not** this project's own `src/motors/L298N.h`. It uses a
different pin set from the current controller and isn't part of the
PlatformIO build in `src/`:

| Signal | Arduino pin |
|---|---|
| `IN1` | 2 |
| `IN2` | 4 |
| `ENA` | 3 |
| FWD button | 12 |
| REV button | 11 |
| status LED | 10 (+ `LED_BUILTIN`) |

Treat this as a historical reference, not the current pinout - `src/main.cpp`
is the maintained controller.
