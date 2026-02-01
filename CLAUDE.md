# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Home Assistant ePaper Remote is an ESP32-S3 embedded firmware (C++/Arduino) that creates an e-ink touchscreen remote for Home Assistant. It connects via WiFi using the WebSocket API (no HA plugins required).

**Hardware:** Lilygo T5 E-Paper S3 Pro, M5Stack M5Paper S3 (both 540x960 displays)

## Build Commands

Requires [PlatformIO](https://platformio.org/).

```bash
# Build for Lilygo T5 Pro
platformio run -e lilygo-t5-s3

# Build for M5Stack M5Paper S3
platformio run -e m5-papers3

# Build and upload
platformio run -e lilygo-t5-s3 --target upload

# Monitor serial output (115200 baud)
platformio run -e lilygo-t5-s3 --target monitor
```

## Code Formatting

Uses `.clang-format` (LLVM-based, C++11, 4-space indent, 135 column limit).

```bash
clang-format -i src/**/*.cpp src/**/*.h
```

## Architecture

**FreeRTOS Multi-Task Design** (see `src/main.cpp`):
- **UI Task** (2KB stack): E-paper rendering, widget display
- **Home Assistant Task** (8KB stack): WebSocket connection, JSON parsing, command sending
- **Touch Task** (4KB stack): Capacitive touch polling, gesture detection
- **WiFi Task**: Connection management

**Thread Safety:** `EntityStore::mutex` protects entity state; `SharedUIState::mutex` protects UI mode; `EventGroupHandle_t` signals WiFi status.

**Key Data Flow:**
1. HASS task receives state updates → updates EntityStore → signals UI task
2. Touch task detects input → enqueues command in EntityStore → HASS task sends to HA
3. UI task reads EntityStore → renders widgets

**Widget System** (`src/widgets/`):
- Abstract `Widget` base class with `fullDraw()`, `partialDraw()`, `isTouching()`, `getValueFromTouch()`
- `Slider`: Brightness/percentage control (variable dimensions)
- `OnOffButton`: Binary toggle (100x100px)

**Command Types** (`src/store.h`):
- `SetLightBrightnessPercentage`, `SetFanSpeedPercentage`, `SwitchOnOff`, `AutomationOnOff`

## Configuration

1. Generate icons: `pip install Pillow && python generate-icons.py`
2. Copy an example config to `src/config_remote.cpp`:
   - `src/config_remote_single.cpp.example` - Single page, no navigation
   - `src/config_remote_multi.cpp.example` - Multiple pages with carousel navigation (arrows + dots)
   - `src/config_remote_tabs.cpp.example` - Multiple pages with tab navigation (labeled buttons)
3. Edit WiFi credentials, HA WebSocket URL, and long-lived access token
4. Define entities and add widgets to pages

## Multi-Page Support

The UI supports up to 4 pages (`MAX_PAGES`). Two navigation modes are available:

**Carousel mode** (default): Arrows and dots at bottom, tap left/right thirds to navigate.

**Tab mode**: Labeled text buttons at bottom, tap to jump directly to page. Active tab shown with bold + underline.

```cpp
// Enable tab navigation
screen_manager_set_nav_mode(screens, NavigationMode::Tabs);

// Add pages with labels (used as tab text)
Screen* page1 = screen_manager_add_page(screens, "Lights");
Screen* page2 = screen_manager_add_page(screens, "Climate");
```

**Key structs:**
- `ScreenManager`: holds all pages, tracks current page and nav mode
- `Screen`: holds widgets for a single page, plus optional label for tabs

## Debugging

Enable verbose logging by uncommenting in `platformio.ini`:
```ini
-DCORE_DEBUG_LEVEL=5
```

Log tags: `home_assistant`, `ui`, `wifi`, `touch`

## Key Constants

Defined in `src/constants.h`:
- `MAX_ENTITIES = 8`, `MAX_WIDGETS_PER_SCREEN = 8`, `MAX_PAGES = 4`
- `NAV_BAR_HEIGHT = 60` (navigation bar at bottom when multiple pages)
- `HASS_RECONNECT_DELAY_MS = 10000`, `HASS_TASK_SEND_DELAY_MS = 500` (prevents Zigbee flooding)

## Dependencies

- [FastEPD](https://github.com/bitbank2/FastEPD) - E-paper rendering
- [bb_captouch](https://github.com/bitbank2/bb_captouch) - Touch controller driver (GT911)
