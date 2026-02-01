#include "managers/touch.h"
#include "boards.h"
#include "constants.h"

static const char* TAG = "touch";

// Check if touch is in navigation area (bottom of screen)
static bool is_nav_touch(uint16_t y) {
    return y >= (DISPLAY_HEIGHT - NAV_BAR_HEIGHT);
}

// Handle carousel navigation touch - returns true if page changed
static bool handle_carousel_touch(ScreenManager* screens, uint16_t x, EntityStore* store) {
    uint8_t old_page = screens->current_page;

    // Left third = previous, right third = next
    if (x < DISPLAY_WIDTH / 3) {
        screen_manager_prev_page(screens);
    } else if (x > DISPLAY_WIDTH * 2 / 3) {
        screen_manager_next_page(screens);
    }

    if (screens->current_page != old_page) {
        ESP_LOGI(TAG, "Page changed from %d to %d", old_page, screens->current_page);
        if (store->ui_task) {
            xTaskNotifyGive(store->ui_task);
        }
        return true;
    }
    return false;
}

// Handle tab navigation touch - returns true if page changed
static bool handle_tabs_touch(ScreenManager* screens, uint16_t x, EntityStore* store) {
    uint8_t old_page = screens->current_page;

    // Calculate tab positions (same logic as ui_draw_tabs_nav)
    // We estimate tab widths based on a rough average since we don't have font metrics here
    uint16_t avg_tab_width = DISPLAY_WIDTH / screens->page_count;
    uint16_t start_x = (DISPLAY_WIDTH - (avg_tab_width * screens->page_count)) / 2;

    // Determine which tab was touched
    if (x >= start_x) {
        uint8_t touched_tab = (x - start_x) / avg_tab_width;
        if (touched_tab < screens->page_count) {
            screen_manager_set_page(screens, touched_tab);
        }
    }

    if (screens->current_page != old_page) {
        ESP_LOGI(TAG, "Tab changed from %d to %d", old_page, screens->current_page);
        if (store->ui_task) {
            xTaskNotifyGive(store->ui_task);
        }
        return true;
    }
    return false;
}

// Handle navigation touch - returns true if page changed
static bool handle_nav_touch(ScreenManager* screens, uint16_t x, EntityStore* store) {
    if (screens->nav_mode == NavigationMode::Tabs) {
        return handle_tabs_touch(screens, x, store);
    } else {
        return handle_carousel_touch(screens, x, store);
    }
}

void touch_task(void* arg) {
    TouchTaskArgs* ctx = static_cast<TouchTaskArgs*>(arg);
    BBCapTouch* bbct = ctx->bbct;
    EntityStore* store = ctx->store;
    ScreenManager* screens = ctx->screens;

    // UI State values
    uint32_t ui_state_version = 0;
    auto* ui_state = new UIState{};

    // Touch infos
    TOUCHINFO ti;
    TouchEvent touch_event = TouchEvent{};
    bool touching = false;
    int active_widget = -1;
    uint32_t last_touch_ms = 0;
    uint8_t widget_original_value = 0;
    uint8_t widget_current_value = 0;
    bool nav_touch_handled = false;

    // Initialize touch
    ESP_LOGI(TAG, "Initializing touchscreen...");
    int rc = bbct->init(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_INT);
    ESP_LOGI(TAG, "init() rc = %d", rc);
    int type = bbct->sensorType();
    ESP_LOGI(TAG, "Sensor type = %d", type);

    while (true) {
        if (bbct->getSamples(&ti)) {
            last_touch_ms = millis();
            ui_state_copy(ctx->state, &ui_state_version, ui_state);

            // Only process touches when on main screen
            if (ui_state->mode != UiMode::MainScreen) {
                continue;
            }

            Screen* screen = screen_manager_get_current(screens);

            // We're already targeting a widget
            if (active_widget != -1) {
                if (touch_event.x == ti.x[0] && touch_event.y == ti.y[0]) {
                    // Finger did not move, ignore
                } else {
                    touch_event.x = ti.x[0];
                    touch_event.y = ti.y[0];
                    ESP_LOGI(TAG, "Widget %d, Coordinates: %d %d", active_widget, touch_event.x, touch_event.y);

                    // Get the new value
                    widget_current_value = screen->widgets[active_widget]->getValueFromTouch(&touch_event, widget_original_value);

                    store_send_command(store, screen->entity_ids[active_widget], widget_current_value);
                }
            } else if (touching == false) {
                touch_event.x = ti.x[0];
                touch_event.y = ti.y[0];
                touching = true;
                nav_touch_handled = false;

                // Check for navigation touch first (only if multiple pages)
                if (screens->page_count > 1 && is_nav_touch(touch_event.y)) {
                    nav_touch_handled = handle_nav_touch(screens, touch_event.x, store);
                } else {
                    // Check widgets on current page
                    for (size_t widget_idx = 0; widget_idx < screen->widget_count; widget_idx++) {
                        if (screen->widgets[widget_idx]->isTouching(&touch_event)) {
                            ESP_LOGI(TAG, "Starting touch on widget %d", widget_idx);
                            active_widget = widget_idx;

                            // Get the new value
                            widget_original_value = ui_state->widget_values[widget_idx];
                            widget_current_value = screen->widgets[widget_idx]->getValueFromTouch(&touch_event, widget_original_value);

                            store_send_command(store, screen->entity_ids[active_widget], widget_current_value);

                            break;
                        }
                    }
                }
            }
        } else {
            if (touching) {
                if (millis() - last_touch_ms > TOUCH_RELEASE_TIMEOUT_MS) {
                    ESP_LOGI(TAG, "End of touch");
                    touching = false;
                    active_widget = -1;
                    nav_touch_handled = false;
                }
                vTaskDelay(pdMS_TO_TICKS(25));
            } else {
                vTaskDelay(pdMS_TO_TICKS(200));
            }
        }
    }
}
