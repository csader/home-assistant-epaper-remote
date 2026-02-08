#include "ui.h"
#include "assets/icons.h"
#include "assets/Montserrat_Regular_26.h"
#include "boards.h"
#include "constants.h"
#include "draw.h"
#include "store.h"
#include "widgets/Widget.h"
#include "widgets/Thermostat.h"
#include <algorithm>

static const char* TAG = "ui";
static const char* const TEXT_BOOT[] = {"Home Assistant", "e-paper remote", nullptr};
static const char* const TEXT_WIFI_DISCONNECTED[] = {"Not connected", "to Wifi", nullptr};
static const char* const TEXT_HASS_DISCONNECTED[] = {"Not connected", "to Home Assistant", nullptr};
static const char* const TEXT_HASS_INVALID_KEY[] = {"Cannot connect", "to Home Assistant:", "invalid token", nullptr};
static const char* const TEXT_GENERIC_ERROR[] = {"Unknown error", nullptr};

void accumulate_damage(Rect& acc, const Rect& r) {
    if (r.w <= 0 || r.h <= 0) {
        return;
    }

    if (acc.w <= 0 || acc.h <= 0) {
        acc = r;
        return;
    }

    const int16_t x1 = std::min(acc.x, r.x);
    const int16_t y1 = std::min(acc.y, r.y);
    const int16_t x2 = std::max(acc.x + acc.w, r.x + r.w);
    const int16_t y2 = std::max(acc.y + acc.h, r.y + r.h);

    acc.x = x1;
    acc.y = y1;
    acc.w = x2 - x1;
    acc.h = y2 - y1;
}

void ui_draw_carousel_nav(FASTEPD* epaper, uint8_t current_page, uint8_t page_count, BitDepth depth) {
    uint8_t fill_color = (depth == BitDepth::BD_4BPP) ? 0xf : BBEP_WHITE;

    // Calculate total width of indicator dots
    uint16_t total_width = (page_count - 1) * PAGE_DOT_SPACING;
    uint16_t start_x = DISPLAY_WIDTH / 2 - total_width / 2;
    uint16_t y = DISPLAY_HEIGHT - NAV_BAR_HEIGHT / 2;

    // Draw navigation arrows using chevrons (lines)
    uint16_t arrow_y = y;
    uint8_t arrow_size = 12;

    // Left arrow (previous) - only if not on first page
    if (current_page > 0) {
        uint16_t arrow_x = 40;
        // Draw < chevron
        epaper->drawLine(arrow_x + arrow_size, arrow_y - arrow_size, arrow_x, arrow_y, BBEP_BLACK);
        epaper->drawLine(arrow_x, arrow_y, arrow_x + arrow_size, arrow_y + arrow_size, BBEP_BLACK);
        // Thicken
        epaper->drawLine(arrow_x + arrow_size, arrow_y - arrow_size + 1, arrow_x + 1, arrow_y, BBEP_BLACK);
        epaper->drawLine(arrow_x + 1, arrow_y, arrow_x + arrow_size, arrow_y + arrow_size - 1, BBEP_BLACK);
    }

    // Right arrow (next) - only if not on last page
    if (current_page < page_count - 1) {
        uint16_t arrow_x = DISPLAY_WIDTH - 40;
        // Draw > chevron
        epaper->drawLine(arrow_x - arrow_size, arrow_y - arrow_size, arrow_x, arrow_y, BBEP_BLACK);
        epaper->drawLine(arrow_x, arrow_y, arrow_x - arrow_size, arrow_y + arrow_size, BBEP_BLACK);
        // Thicken
        epaper->drawLine(arrow_x - arrow_size, arrow_y - arrow_size + 1, arrow_x - 1, arrow_y, BBEP_BLACK);
        epaper->drawLine(arrow_x - 1, arrow_y, arrow_x - arrow_size, arrow_y + arrow_size - 1, BBEP_BLACK);
    }

    // Draw page dots
    for (uint8_t i = 0; i < page_count; i++) {
        uint16_t x = start_x + i * PAGE_DOT_SPACING;
        if (i == current_page) {
            // Filled dot for current page
            epaper->fillCircle(x, y, PAGE_DOT_RADIUS, BBEP_BLACK);
        } else {
            // Outlined dot for other pages
            epaper->fillCircle(x, y, PAGE_DOT_RADIUS, BBEP_BLACK);
            epaper->fillCircle(x, y, PAGE_DOT_RADIUS - 2, fill_color);
        }
    }
}

void ui_draw_tabs_nav(FASTEPD* epaper, ScreenManager* screens, uint8_t current_page, BitDepth depth) {
    uint8_t fill_color = (depth == BitDepth::BD_4BPP) ? 0xf : BBEP_WHITE;

    epaper->setFont(Montserrat_Regular_26);
    epaper->setTextColor(BBEP_BLACK);

    // Get consistent font height using reference string (same pattern as widgets)
    BB_RECT font_rect;
    epaper->getStringBox("pI", &font_rect);
    uint16_t font_height = font_rect.h;

    // Calculate tab widths and total width
    BB_RECT text_rects[MAX_PAGES];
    uint16_t total_width = 0;
    for (uint8_t i = 0; i < screens->page_count; i++) {
        const char* label = screens->pages[i].label ? screens->pages[i].label : "Page";
        epaper->getStringBox(label, &text_rects[i]);
        total_width += text_rects[i].w + TAB_PADDING * 2;
    }

    // Calculate starting x position to center all tabs
    uint16_t x = (DISPLAY_WIDTH - total_width) / 2;
    uint16_t nav_center_y = DISPLAY_HEIGHT - NAV_BAR_HEIGHT / 2;

    // Consistent text baseline for all tabs
    uint16_t text_y = nav_center_y + font_height / 2 - 4;

    // Draw each tab
    for (uint8_t i = 0; i < screens->page_count; i++) {
        const char* label = screens->pages[i].label ? screens->pages[i].label : "Page";
        uint16_t tab_width = text_rects[i].w + TAB_PADDING * 2;
        uint16_t text_x = x + TAB_PADDING;

        // Active tab: draw simple underline
        if (i == current_page) {
            uint16_t underline_y = text_y + 6;
            epaper->fillRect(text_x - 4, underline_y, text_rects[i].w + 8, 3, BBEP_BLACK);
        }

        // Draw text (same position for all tabs)
        epaper->setCursor(text_x, text_y);
        epaper->write(label);

        x += tab_width;
    }
}

void ui_draw_navigation(FASTEPD* epaper, ScreenManager* screens, uint8_t current_page, BitDepth depth) {
    if (screens->page_count <= 1) {
        return;
    }

    if (screens->nav_mode == NavigationMode::Tabs) {
        ui_draw_tabs_nav(epaper, screens, current_page, depth);
    } else {
        ui_draw_carousel_nav(epaper, current_page, screens->page_count, depth);
    }
}

void ui_main_screen_full_draw(UIState* state, BitDepth depth, Screen* screen, ScreenManager* screens, FASTEPD* epaper, EntityStore* store) {
    for (uint8_t widget_idx = 0; widget_idx < screen->widget_count; widget_idx++) {
        // Check if this is a thermostat widget
        if (screen->widgets[widget_idx]->getType() == WidgetType::Thermostat) {
            Thermostat* thermostat = static_cast<Thermostat*>(screen->widgets[widget_idx]);
            // Update thermostat with mode, target temp, and current temp
            thermostat->setMode(state->widget_values[widget_idx]);
            if (widget_idx + 1 < screen->widget_count) {
                thermostat->setTargetTemp(state->widget_values[widget_idx + 1]);
                // Get current temperature from entity store
                uint8_t temp_entity_idx = screen->entity_ids[widget_idx + 1];
                thermostat->setCurrentTemp(store->entities[temp_entity_idx].current_temperature);
            }
            thermostat->fullDraw(epaper, depth, state->widget_values[widget_idx]);
            // Skip the next widget index since thermostat uses two slots
            widget_idx++;
        } else {
            screen->widgets[widget_idx]->fullDraw(epaper, depth, state->widget_values[widget_idx]);
        }
    }

    // Draw navigation
    ui_draw_navigation(epaper, screens, state->current_page, depth);
}

void ui_show_message(UiMode mode, FASTEPD* epaper) {
    const uint8_t* icon = alert_circle;
    const char* const* text_lines = TEXT_GENERIC_ERROR;

    switch (mode) {
    case UiMode::Boot:
        icon = home_assistant;
        text_lines = TEXT_BOOT;
        break;
    case UiMode::WifiDisconnected:
        icon = wifi_off;
        text_lines = TEXT_WIFI_DISCONNECTED;
        break;
    case UiMode::HassDisconnected:
        icon = server_network_off;
        text_lines = TEXT_HASS_DISCONNECTED;
        break;
    case UiMode::HassInvalidKey:
        icon = lock_alert_outline;
        text_lines = TEXT_HASS_INVALID_KEY;
        break;
    }

    drawCenteredIconWithText(epaper, icon, text_lines, 30, 100);
}

void ui_task(void* arg) {
    UITaskArgs* ctx = static_cast<UITaskArgs*>(arg);
    UIState current_state = {};
    UIState displayed_state = {};
    bool display_is_dirty = false;

    xTaskNotifyGive(xTaskGetCurrentTaskHandle()); // First refresh needs a notification

    while (1) {
        TickType_t notify_timeout = portMAX_DELAY;
        if (display_is_dirty) {
            notify_timeout = pdMS_TO_TICKS(DISPLAY_FULL_REDRAW_TIMEOUT_MS);
        }

        if (ulTaskNotifyTake(pdTRUE, notify_timeout)) {
            // Get current page from screen manager
            current_state.current_page = ctx->screens->current_page;
            current_state.page_count = ctx->screens->page_count;

            Screen* current_screen = screen_manager_get_current(ctx->screens);
            store_update_ui_state(ctx->store, current_screen, &current_state);

            // Handle screen change or page change
            size_t widget_idx;
            bool page_changed = (current_state.current_page != displayed_state.current_page);

            if (current_state.mode != displayed_state.mode || page_changed) {
                ctx->epaper->setMode(BB_MODE_4BPP);
                ctx->epaper->fillScreen(0xf);

                if (current_state.mode == UiMode::MainScreen) {
                    // Display the main screen in its full glory
                    ui_main_screen_full_draw(&current_state, BitDepth::BD_4BPP, current_screen, ctx->screens, ctx->epaper, ctx->store);
                    ctx->epaper->fullUpdate(CLEAR_SLOW, true);

                    // Preload the 1BPP version for fast updates
                    ctx->epaper->setMode(BB_MODE_1BPP);
                    ctx->epaper->fillScreen(BBEP_WHITE);
                    ui_main_screen_full_draw(&current_state, BitDepth::BD_1BPP, current_screen, ctx->screens, ctx->epaper, ctx->store);
                    ctx->epaper->backupPlane();
                } else {
                    ui_show_message(current_state.mode, ctx->epaper);
                    ctx->epaper->fullUpdate(CLEAR_SLOW, true);
                }
                display_is_dirty = false;
            } else if (current_state.mode == UiMode::MainScreen) {
                Rect damage_accum = {};

                for (widget_idx = 0; widget_idx < current_screen->widget_count; widget_idx++) {
                    uint8_t displayed_value = displayed_state.widget_values[widget_idx];
                    uint8_t current_value = current_state.widget_values[widget_idx];

                    // Check if this is a thermostat widget
                    if (current_screen->widgets[widget_idx]->getType() == WidgetType::Thermostat) {
                        Thermostat* thermostat = static_cast<Thermostat*>(current_screen->widgets[widget_idx]);
                        // Update thermostat with mode, target temp, and current temp
                        thermostat->setMode(current_state.widget_values[widget_idx]);
                        if (widget_idx + 1 < current_screen->widget_count) {
                            thermostat->setTargetTemp(current_state.widget_values[widget_idx + 1]);
                            // Get current temperature from entity store
                            uint8_t temp_entity_idx = current_screen->entity_ids[widget_idx + 1];
                            thermostat->setCurrentTemp(ctx->store->entities[temp_entity_idx].current_temperature);
                        }
                        
                        // Check if either mode or temp changed
                        bool changed = (displayed_value != current_value);
                        if (widget_idx + 1 < current_screen->widget_count) {
                            changed = changed || (displayed_state.widget_values[widget_idx + 1] != current_state.widget_values[widget_idx + 1]);
                        }
                        
                        if (changed) {
                            ESP_LOGI(TAG, "updating thermostat widget %d", widget_idx);
                            Rect damage = thermostat->partialDraw(ctx->epaper, BitDepth::BD_1BPP, displayed_value, current_value);
                            accumulate_damage(damage_accum, damage);
                        }
                        
                        // Skip next widget index
                        widget_idx++;
                    } else if (displayed_value != current_value) {
                        ESP_LOGI(TAG, "updating widget %d from %d to %d", widget_idx, displayed_value, current_value);
                        Rect damage = current_screen->widgets[widget_idx]->partialDraw(ctx->epaper, BitDepth::BD_1BPP, displayed_value,
                                                                                    current_value);
                        accumulate_damage(damage_accum, damage);
                    }
                }
                if (damage_accum.w > 0 || damage_accum.h > 0) {
                    ESP_LOGI(TAG, "Launching partial update rows %d to %d", damage_accum.x, damage_accum.x + damage_accum.w);
                    ctx->epaper->partialUpdate(true,
                                               DISPLAY_WIDTH - (damage_accum.x + damage_accum.w), // row start (reversed)
                                               DISPLAY_WIDTH - damage_accum.x                     // row end (reversed)
                    );
                    display_is_dirty = true;
                }
            }

            // Save new state
            displayed_state = current_state;
            ui_state_set(ctx->shared_state, &displayed_state);
        } else if (display_is_dirty) {
            ESP_LOGI(TAG, "Forcing a full refresh of the display");

            Screen* current_screen = screen_manager_get_current(ctx->screens);

            // Cleanup the display
            ctx->epaper->setMode(BB_MODE_4BPP);
            ctx->epaper->fillScreen(0xf);
            ui_main_screen_full_draw(&displayed_state, BitDepth::BD_4BPP, current_screen, ctx->screens, ctx->epaper, ctx->store);
            ctx->epaper->fullUpdate(CLEAR_FAST, true);

            // Preload the 1bpp version for fast updates
            ctx->epaper->setMode(BB_MODE_1BPP);
            ctx->epaper->fillScreen(BBEP_WHITE);
            ui_main_screen_full_draw(&displayed_state, BitDepth::BD_1BPP, current_screen, ctx->screens, ctx->epaper, ctx->store);
            ctx->epaper->backupPlane();

            display_is_dirty = false;
        }
    }
}
