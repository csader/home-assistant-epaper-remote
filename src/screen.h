#pragma once

#include "constants.h"
#include "entity_ref.h"
#include "widgets/Widget.h"
#include <cstddef>

enum class NavigationMode : uint8_t {
    Carousel, // Arrows + dots, tap left/right to navigate
    Tabs,     // Text buttons at bottom, tap to jump to page
};

struct Screen {
    size_t widget_count;
    Widget* widgets[MAX_WIDGETS_PER_SCREEN];
    uint8_t entity_ids[MAX_WIDGETS_PER_SCREEN];
    const char* label; // Used for tab navigation
};

struct ScreenManager {
    size_t page_count;
    Screen pages[MAX_PAGES];
    uint8_t current_page;
    NavigationMode nav_mode;
};

struct SliderConfig {
    EntityRef entity_ref;
    const char* label;
    const uint8_t* icon_on;
    const uint8_t* icon_off;
    uint16_t pos_x;
    uint16_t pos_y;
    uint16_t width;
    uint16_t height;
};

struct ButtonConfig {
    EntityRef entity_ref;
    const char* label;
    const uint8_t* icon_on;
    const uint8_t* icon_off;
    uint16_t pos_x;
    uint16_t pos_y;
};

void screen_manager_init(ScreenManager* manager);
Screen* screen_manager_add_page(ScreenManager* manager, const char* label = nullptr);
Screen* screen_manager_get_current(ScreenManager* manager);
void screen_manager_set_page(ScreenManager* manager, uint8_t page);
void screen_manager_next_page(ScreenManager* manager);
void screen_manager_prev_page(ScreenManager* manager);
void screen_manager_set_nav_mode(ScreenManager* manager, NavigationMode mode);

void screen_add_slider(SliderConfig config, Screen* screen);
void screen_add_button(ButtonConfig config, Screen* screen);
