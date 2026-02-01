#pragma once

#include "constants.h"
#include "entity_ref.h"
#include "widgets/Widget.h"
#include <cstddef>

struct Screen {
    size_t widget_count;
    Widget* widgets[MAX_WIDGETS_PER_SCREEN];
    uint8_t entity_ids[MAX_WIDGETS_PER_SCREEN];
};

struct ScreenManager {
    size_t page_count;
    Screen pages[MAX_PAGES];
    uint8_t current_page;
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
Screen* screen_manager_add_page(ScreenManager* manager);
Screen* screen_manager_get_current(ScreenManager* manager);
void screen_manager_set_page(ScreenManager* manager, uint8_t page);
void screen_manager_next_page(ScreenManager* manager);
void screen_manager_prev_page(ScreenManager* manager);

void screen_add_slider(SliderConfig config, Screen* screen);
void screen_add_button(ButtonConfig config, Screen* screen);
