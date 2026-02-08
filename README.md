# Home Assistant ePaper remote

e-Ink remote for Home Assistant built with [FastEPD](https://github.com/bitbank2/FastEPD).

![Preview](./preview.jpg)

It uses the websocket API of Home Assistant, no plugin is required on the server.
As it stays permanently connected to the Wifi to get updates, the remote only lasts a few hours on battery.

## Hardware supported

- [Lilygo T5 E-Paper S3 Pro](https://lilygo.cc/products/t5-e-paper-s3-pro)
- [M5Stack M5Paper S3](https://docs.m5stack.com/en/core/PaperS3)

## Setup

You will need to install [PlatformIO](https://platformio.org/) to compile the project.

### Option 1: Visual Configurator (Recommended)

Open `tools/configurator.html` in a web browser for a visual configuration tool that:
- Lets you configure pages, widgets, and entity bindings with a live preview
- Fetches icons directly from Material Design Icons CDN (no manual downloads)
- Auto-generates `icons.h` with only the icons you use
- Exports directly to your project folder via File System Access API
- Creates backups before overwriting existing files
- Can import existing `config_remote.cpp` configurations

### Option 2: Manual Configuration

#### Generate icons

Find the icons for your buttons at [Pictogrammers](https://pictogrammers.com/library/mdi/).
Use "Download PNG (256x256)" and place your icons in the `icons-buttons` folder.
Make sure you have an icon for the "on" state and one for the "off" state of each of your buttons.

Then run the python script `generate-icons.py` to generate the file `src/assets/icons.h`.
You will need to install the library [Pillow](https://pillow.readthedocs.io/en/stable/installation/basic-installation.html#basic-installation) to run this script.

#### Get a home assistant token

In Home Assistant:

- Click on your username in the bottom left
- Go to "security"
- Click on "Create Token" in the "Long-lived access tokens" section
- Note the token generated

#### Update configuration

Choose an example configuration and copy it to `src/config_remote.cpp`:

#### Single-page layout

```bash
cp src/config_remote_single.cpp.example src/config_remote.cpp
```

All widgets on one screen, no navigation bar.

#### Multi-page with carousel navigation

```bash
cp src/config_remote_multi.cpp.example src/config_remote.cpp
```

Widgets organized across multiple pages (up to 4). Arrows and dots appear at the bottom - tap left third to go back, right third to go forward.

#### Multi-page with tab navigation

```bash
cp src/config_remote_tabs.cpp.example src/config_remote.cpp
```

Widgets organized across multiple pages with labeled tabs at the bottom. Tap a tab to jump directly to that page. Active tab is shown with bold text and underline.

---

Then edit `src/config_remote.cpp` with your WiFi credentials, Home Assistant URL, token, and entity IDs.

## Widget Types

### Buttons
Tap to toggle between on/off states. Shows different icons for on and off states.

### Sliders
Drag along the slider to adjust values (0-100%). Tap the icon on the left to quickly toggle between off (0) and on (100).

### Thermostats
Control climate devices with mode button and +/- temperature buttons. The mode button cycles through off/heat/cool/auto modes. Tap +/- buttons to adjust the target temperature within a configurable range. Displays both current and target temperatures (e.g., "72/75°F").

## Supported Entity Types

### Lights
- **SetLightBrightnessPercentage**: Control light brightness with a slider (0-100%)
- **SwitchOnOff**: Simple on/off toggle for lights or switches

### Covers
- **SetCoverPosition**: Control blinds/shades position with a slider (0-100%)

### Fans
- **SetFanSpeedPercentage**: Control fan speed with a slider (0-100%)

### Scenes & Scripts
- **ActivateScene**: One-tap button to activate a scene (no state feedback)
- **RunScript**: One-tap button to run an automation script (no state feedback)

### Locks
- **LockUnlock**: Lock/unlock doors with state feedback (button shows locked/unlocked state)

### Media Players
- **SetMediaPlayerVolume**: Control volume with a slider (0-100%)
- **MediaPlayerPlayPause**: Toggle play/pause state with a button

### Automations
- **AutomationOnOff**: Enable/disable automations with a button

### Input Helpers
- **SetInputNumber**: Control input_number helpers with a slider
- **InputBooleanToggle**: Toggle input_boolean helpers with a button

### Vacuum
- **VacuumCommand**: Control vacuum (0=stop, 1=start, 2+=dock)

### Climate (Thermostats)
- **SetClimateMode**: Control HVAC mode (0=off, 1=heat, 2=cool, 3=auto)
- **SetClimateTemperature**: Set target temperature (configurable range, e.g., 60-85°F or 16-30°C)

Note: Thermostat widgets use two entity slots (one for mode, one for temperature) and display both current and target temperatures.

### Build and upload

```bash
# Build for Lilygo T5 E-Paper S3 Pro
pio run -e lilygo-t5-s3

# Build for M5Stack M5Paper S3
pio run -e m5-papers3

# Build and upload (connect board via USB)
pio run -e lilygo-t5-s3 --target upload
```

## Notes

### Getting more logs

To get some logs from the serial port, uncomment the following line from `platformio.ini`:

```
    # -DCORE_DEBUG_LEVEL=5
```

### Updating the font

The font used is Montserrat Regular in size 26, it was converted using [fontconvert from FastEPD](https://github.com/bitbank2/FastEPD/tree/main/fontconvert):

```
./fontconvert Montserrat-Regular.ttf `src/assets/Montserrat_Regular_26.h` 26 32 126
```

## License

[This project is released under Apache License 2.0.](./LICENSE)

This repository contains resources from:

- https://github.com/Templarian/MaterialDesign (SIL OPEN FONT LICENSE Version 1.1)
- https://github.com/JulietaUla/Montserrat (Apache License 2.0)



