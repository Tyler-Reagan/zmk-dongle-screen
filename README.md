# ZMK Dongle Screen — TOTEM (Dongle Setup)

Personal ZMK firmware for the [TOTEM](https://github.com/GEIGEIGEIST/TOTEM) — a 38-key column-staggered wireless split keyboard using three [Seeed XIAO nRF52840](https://wiki.seeedstudio.com/XIAO-nRF52840/) microcontrollers: left half, right half, and a dedicated USB dongle with a ST7789V status display.

**This repo is for the dongle setup**: both keyboard halves connect wirelessly as BLE peripherals to the dongle, which plugs into your computer via USB and serves as the USB HID + ZMK Studio endpoint. The dongle displays live status (layer, modifiers, WPM, output, battery) on a small screen.

> **No dongle?** See [zmk-config-studio](https://github.com/Tyler-Reagan/zmk-config-studio) — a separate repo for the simpler two-piece setup (left half as USB central, ZMK Studio via left half directly).

---

## Hardware

Three XIAO nRF52840 boards:

| Piece      | Firmware artifact | Role                                             |
| ---------- | ----------------- | ------------------------------------------------ |
| Left half  | `totem_left`      | BLE peripheral — sends key events to dongle      |
| Right half | `totem_right`     | BLE peripheral — sends key events to dongle      |
| Dongle     | `totem_dongle`    | USB central — HID + ZMK Studio + ST7789V display |

The dongle display shows: output mode (USB/BLE), active layer, active modifiers, WPM, and battery levels for both halves.

---

## Repo structure

```
config/
  totem.keymap              ← keymap (edit this)
  totem_left.conf    ← left peripheral config (bootloader support)
  totem_right.conf          ← right peripheral config (bootloader support)
  totem_dongle.conf         ← dongle config (bootloader support)
  west.yml                  ← ZMK west manifest

boards/shields/
  totem/                    ← TOTEM keyboard shield (dongle variants)
    totem.dtsi              ← base matrix / kscan
    totem-layouts.dtsi      ← physical key positions (ZMK Studio)
    totem_left.overlay ← left half GPIO (peripheral mode)
    totem_right.overlay     ← right half GPIO (peripheral mode)
    totem_dongle.overlay    ← dongle mock kscan
    totem_dongle.conf       ← dongle screen preferences (orientation, brightness, timeout)
    Kconfig.shield          ← shield Kconfig symbols
    Kconfig.defconfig       ← dongle split role / BT connection defaults
    totem.zmk.yml           ← ZMK shield metadata

  dongle_screen/            ← dongle screen Zephyr module
    boards/
      xiao_ble_zmk.overlay  ← SPI/display/PWM hardware (XIAO BLE pinout)
    src/
      widgets/              ← layer, mod, WPM, battery, output widgets
      fonts/                ← NerdFonts (20px, 40px)
      custom_status_screen.c
      brightness.c
    dongle_screen.conf      ← display module defaults
    CMakeLists.txt

drivers/display/
  display_st7789v.c         ← ST7789V display driver
  display_st7789v.h

build.yaml                  ← GitHub Actions build matrix
Makefile                    ← workflow helper (requires gh CLI)
.github/workflows/
  build.yml                 ← GitHub Actions build workflow
zephyr/module.yml           ← Zephyr module declaration
CMakeLists.txt              ← module build entry point
Kconfig                     ← module Kconfig
```

---

## Makefile workflow

Requires the [GitHub CLI](https://cli.github.com/) (`gh`). ZMK firmware is built in GitHub Actions — the Makefile wraps `gh` commands for convenience.

```
make help            Show all targets and workflow summary
make build           Trigger a GitHub Actions build
make status          List recent build runs (latest 5)
make download        Download firmware artifacts → firmware/
make flash-left      Copy left UF2 to mounted XIAO bootloader drive
make flash-right     Copy right UF2 to mounted XIAO bootloader drive
make flash-dongle    Copy dongle UF2 to mounted XIAO bootloader drive
```

### Typical workflow

```sh
# 1. Edit your keymap
vim config/totem.keymap

# 2. Trigger a build
make build

# 3. Check when it finishes
make status

# 4. Download artifacts once the build succeeds
make download
# → firmware/totem_left/zmk.uf2
# → firmware/totem_right/zmk.uf2
# → firmware/totem_dongle/zmk.uf2

# 5. Flash each device (double-tap reset first to enter bootloader)
make flash-left
make flash-right
make flash-dongle
```

> The `BOOT_LEFT`, `BOOT_RIGHT`, and `BOOT_DONGLE` variables default to `/Volumes/XIAO-SENSE`. Override if needed:
>
> ```sh
> make flash-dongle BOOT_DONGLE=/Volumes/XIAO
> ```

---

## Flashing

### Enter bootloader mode

**Double-tap the reset button** on the XIAO — the drive mounts as `XIAO-SENSE` (or `XIAO`) automatically. Or hold the boot button while plugging in USB.

Flash one device at a time. The halves can be left wireless (off or on battery) while you flash the other — they don't need to be disconnected.

### First-time setup

Pairing order matters for the battery widget — the dongle assigns battery indicators in the order halves first connect. Always pair left before right.

1. Flash `settings_reset` to all three devices to clear any stale bond data.
2. Flash `totem_dongle` to the dongle.
3. Flash `totem_left` to the left half.
4. Flash `totem_right` to the right half.
5. Connect the dongle to USB. Power on the **left half** and wait until its battery indicator appears on the dongle screen.
6. Power on the **right half**.
7. The dongle advertises to your computer as **TOTEM** via USB HID — no Bluetooth pairing needed on the host side.

### Subsequent updates

Reflash whichever devices have changed. Bond data is preserved unless you flash `settings_reset`.

---

## ZMK Studio

[ZMK Studio](https://zmk.studio/) lets you remap keys live over USB without reflashing.

- Connect the **dongle** to your computer via USB.
- Open [zmk.studio](https://zmk.studio/) in a Chromium-based browser.
- **TOTEM** appears automatically.
- Changes are written to the dongle's flash instantly.

Studio locking is disabled — no unlock sequence required. Studio changes persist across power cycles but are overwritten on the next firmware flash.

> ZMK Studio connects to the **dongle** in this setup, not the keyboard halves.

---

## Dongle screen

The screen runs on the `dongle_screen` Zephyr module (bundled in this repo). It displays:

- **Output** — current output mode (USB or BLE). Arrow indicates active interface. USB is white when active, red when powered but not connected. BLE profiles are green (connected), blue (bonded), or white (free).
- **Layer** — currently active keymap layer.
- **Modifiers** — live modifier key state (Shift, Ctrl, Alt, GUI).
- **WPM** — real-time words-per-minute.
- **Battery** — battery level for each keyboard half.

### Screen configuration

User preferences live in [`boards/shields/totem/totem_dongle.conf`](boards/shields/totem/totem_dongle.conf):

```conf
CONFIG_DONGLE_SCREEN_HORIZONTAL=y    # Horizontal orientation (default)
CONFIG_DONGLE_SCREEN_IDLE_TIMEOUT_S=600  # Screen off after 10 min idle (0 = never)
CONFIG_DONGLE_SCREEN_MAX_BRIGHTNESS=80   # Max brightness (1–100)
```

Full configuration reference:

| Option                                             | Type | Default | Description                                |
| -------------------------------------------------- | ---- | ------- | ------------------------------------------ |
| `CONFIG_DONGLE_SCREEN_HORIZONTAL`                  | bool | y       | Horizontal screen orientation              |
| `CONFIG_DONGLE_SCREEN_FLIPPED`                     | bool | n       | Flip orientation 180°                      |
| `CONFIG_DONGLE_SCREEN_SYSTEM_ICON`                 | int  | 0       | GUI key icon: 0=macOS, 1=Linux, 2=Windows  |
| `CONFIG_DONGLE_SCREEN_AMBIENT_LIGHT`               | bool | n       | Auto-adjust brightness via APDS9960 sensor |
| `CONFIG_DONGLE_SCREEN_IDLE_TIMEOUT_S`              | int  | 600     | Seconds before screen turns off (0=never)  |
| `CONFIG_DONGLE_SCREEN_MAX_BRIGHTNESS`              | int  | 80      | Max brightness (1–100)                     |
| `CONFIG_DONGLE_SCREEN_MIN_BRIGHTNESS`              | int  | 1       | Min brightness (1–99)                      |
| `CONFIG_DONGLE_SCREEN_DEFAULT_BRIGHTNESS`          | int  | MAX     | Startup brightness                         |
| `CONFIG_DONGLE_SCREEN_BRIGHTNESS_MODIFIER`         | int  | 0       | Persistent brightness offset               |
| `CONFIG_DONGLE_SCREEN_BRIGHTNESS_STEP`             | int  | 10      | Per-keystroke brightness adjustment step   |
| `CONFIG_DONGLE_SCREEN_TOGGLE_KEYCODE`              | int  | 113     | Toggle screen on/off (default: F22)        |
| `CONFIG_DONGLE_SCREEN_BRIGHTNESS_KEYBOARD_CONTROL` | bool | y       | Enable F23/F24 brightness keys             |
| `CONFIG_DONGLE_SCREEN_BRIGHTNESS_UP_KEYCODE`       | int  | 115     | Brightness up keycode (default: F24)       |
| `CONFIG_DONGLE_SCREEN_BRIGHTNESS_DOWN_KEYCODE`     | int  | 114     | Brightness down keycode (default: F23)     |
| `CONFIG_DONGLE_SCREEN_WPM_ACTIVE`                  | bool | y       | Show WPM widget                            |
| `CONFIG_DONGLE_SCREEN_MODIFIER_ACTIVE`             | bool | y       | Show modifier widget                       |
| `CONFIG_DONGLE_SCREEN_LAYER_ACTIVE`                | bool | y       | Show layer widget                          |
| `CONFIG_DONGLE_SCREEN_OUTPUT_ACTIVE`               | bool | y       | Show output widget                         |
| `CONFIG_DONGLE_SCREEN_BATTERY_ACTIVE`              | bool | y       | Show battery widget                        |

### Custom background image

1. Export your image as PNG: **280×240** (horizontal) or **240×280** (vertical).
2. Convert at [lvgl.io/tools/imageconverter](https://lvgl.io/tools/imageconverter): Color format `RGB565`. Make sure you use v9, **not** v8.
3. Replace `boards/shields/dongle_screen/src/images/background.c` with the generated file, renaming the variable to `dongle_screen_background`.
4. Add `CONFIG_DONGLE_SCREEN_BACKGROUND_IMAGE=y` to `boards/shields/totem/totem_dongle.conf`.

5. Rebuild and flash the dongle.

---

## Keymap

Six layers. Source: [`config/totem.keymap`](config/totem.keymap).

| #   | Layer    | Hand  | Activation                          |
| --- | -------- | ----- | ----------------------------------- |
| 0   | **BASE** | Both  | Default                             |
| 1   | **DEV**  | Right | Hold `DEV/SPC` (right thumb inner)  |
| 2   | **SYS**  | Right | Hold `SYS/TAB` (right thumb middle) |
| 3   | **NUM**  | Left  | Hold `NUM/ENT` (left thumb middle)  |
| 4   | **FUN**  | Left  | Hold `FUN/DEL` (left thumb inner)   |
| 5   | **BOOT** | Both  | Assign via ZMK Studio or combo      |

**BASE** — QWERTY with home-row mods (`GUI/S` `CTRL/D` `SHIFT/F` left; `SHIFT/J` `CTRL/K` `GUI/L` right). Left outer pinky: `HYPER` (Ctrl+Shift+Alt+GUI). Right outer pinky: `'`.

**DEV** — Developer symbols on the right: `-{}` `` ` `` `=_[]'$&|*`. Left: modifiers. Right thumbs: `@()`. Right outer pinky: `Shift+Tab`.

**SYS** — Navigation and media: arrows, volume, prev/next/play/mute, screenshots, refresh, undo/cut/copy/paste. Left outer pinky: clear all BT bonds. Right outer pinky: lock screen.

**NUM** — Numpad on the left: `-789` / `=456` / `123`, `.0` on thumbs. Right: modifiers.

**FUN** — Function keys: `F12 F7–F9` / `F11 F4–F6` / `F10 F1–F3`. `SPC/TAB` on thumbs. Right: modifiers.

**BOOT** — `&sys_reset` on top-row outer keys, `&bootloader` on bottom-row outer keys. Both halves and the dongle respond. Activate via ZMK Studio.

---

## FAQ

**Double-tap reset isn't working on a half or the dongle.**
Double-tap is implemented in ZMK firmware — the XIAO bootloader alone may not support it. The firmware must be flashed at least once first. On a fresh XIAO with no firmware, hold the boot button while plugging in to enter bootloader manually.

**The battery widget is showing left and right swapped.**
The dongle assigns battery indicators in first-seen order. Flash `settings_reset` to the dongle, then reflash `totem_dongle` and re-pair: power on left first, wait for its indicator, then power on right.

**One half shows as connected on the dongle screen but keystrokes aren't registering.**
The half may have a stale BLE bond. Flash `settings_reset` to that half, then reflash normal firmware and re-pair.

**The dongle screen is blank / not turning on.**
Check that `CONFIG_DONGLE_SCREEN_IDLE_TIMEOUT_S` isn't 0 after a recent edit (that means never dim, which is correct, but 0 would mean instantly off in a broken build). If idle timeout is fine, the display brightness might be at 0 — press the brightness-up key (F24 by default).

**How do I toggle the screen off/on?**
Assign F22 to a key in your keymap via ZMK Studio. That keycode toggles the display. F23/F24 adjust brightness.

**ZMK Studio shows "Keyboard Locked".**
Shouldn't happen — Studio locking is disabled (`CONFIG_ZMK_STUDIO_LOCKING=n`). If it does, reflash `totem_dongle`.

**The dongle connects but the keyboard halves won't pair.**
Make sure you're not using `settings_reset` firmware accidentally. Confirm the correct `totem_left`/`totem_right` artifacts are flashed. Both halves need to be running peripheral firmware (not central).

**How do I switch to the no-dongle setup?**
Flash from [zmk-config-studio](https://github.com/Tyler-Reagan/zmk-config-studio) instead. Run `settings_reset` on both halves first to clear old dongle bonds, then reflash and re-pair normally.

**GitHub Actions is failing.**
Check the Actions tab. Common causes: keymap syntax error in `totem.keymap`, or a ZMK API change on `main`. Check the [ZMK changelog](https://zmk.dev/docs/changelog) for breaking changes.

**Where do I find the built firmware without `make download`?**
Go to the **Actions** tab in your GitHub repo → select the latest successful run → scroll to **Artifacts** at the bottom of the run summary.

---

## Credits

The `dongle_screen` module is based on the original [zmk-dongle-screen](https://github.com/janpfischer/zmk-dongle-screen) by janpfischer, itself inspired by [prospector-zmk-module](https://github.com/carrefinho/prospector-zmk-module) and [zmk-dongle-display](https://github.com/englmaxi/zmk-dongle-display).

## License

MIT
