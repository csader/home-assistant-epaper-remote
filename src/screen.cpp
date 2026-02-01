#include "screen.h"
#include "esp_system.h"
#include "widgets/OnOffButton.h"
#include "widgets/Slider.h"

void screen_manager_init(ScreenManager* manager) {
    manager->page_count = 0;
    manager->current_page = 0;
    for (size_t i = 0; i < MAX_PAGES; i++) {
        manager->pages[i].widget_count = 0;
    }
}

Screen* screen_manager_add_page(ScreenManager* manager) {
    if (manager->page_count >= MAX_PAGES) {
        esp_system_abort("too many pages configured");
    }
    return &manager->pages[manager->page_count++];
}

Screen* screen_manager_get_current(ScreenManager* manager) {
    return &manager->pages[manager->current_page];
}

void screen_manager_set_page(ScreenManager* manager, uint8_t page) {
    if (page < manager->page_count) {
        manager->current_page = page;
    }
}

void screen_manager_next_page(ScreenManager* manager) {
    if (manager->current_page < manager->page_count - 1) {
        manager->current_page++;
    }
}

void screen_manager_prev_page(ScreenManager* manager) {
    if (manager->current_page > 0) {
        manager->current_page--;
    }
}

void screen_add_slider(SliderConfig config, Screen* screen) {
    if (screen->widget_count >= MAX_WIDGETS_PER_SCREEN) {
        esp_system_abort("too many widgets configured");
    }

    Rect rect{
        .x = (int16_t)config.pos_x,
        .y = (int16_t)config.pos_y,
        .w = (int16_t)config.width,
        .h = (int16_t)config.height,
    };

    Slider* widget = new (std::nothrow) Slider(config.label, config.icon_on, config.icon_off, rect);
    if (!widget) {
        esp_system_abort("out of memory");
    }

    const uint16_t widget_idx = screen->widget_count++;
    screen->widgets[widget_idx] = widget;
    screen->entity_ids[widget_idx] = config.entity_ref.index;
}

void screen_add_button(ButtonConfig config, Screen* screen) {
    if (screen->widget_count >= MAX_WIDGETS_PER_SCREEN) {
        esp_system_abort("too many widgets configured");
    }

    Rect rect{
        .x = (int16_t)config.pos_x,
        .y = (int16_t)config.pos_y,
        .w = 0, // ignored
        .h = 0, // ignored
    };

    OnOffButton* widget = new (std::nothrow) OnOffButton(config.label, config.icon_on, config.icon_off, rect);
    if (!widget) {
        esp_system_abort("out of memory");
    }

    const uint16_t widget_idx = screen->widget_count++;
    screen->widgets[widget_idx] = widget;
    screen->entity_ids[widget_idx] = config.entity_ref.index;
}
