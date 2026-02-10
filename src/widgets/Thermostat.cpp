#include "widgets/Thermostat.h"
#include "assets/Montserrat_Regular_26.h"
#include "constants.h"
#include <FastEPD.h>
#include <cstdio>

Thermostat::Thermostat(const char* label, const uint8_t* icon_off, const uint8_t* icon_heat,
                       const uint8_t* icon_cool, const uint8_t* icon_auto, Rect rect,
                       uint8_t min_temp, uint8_t max_temp, char temp_unit)
    : label_(label)
    , rect_(rect)
    , min_temp_(min_temp)
    , max_temp_(max_temp)
    , temp_unit_(temp_unit)
    , current_temp_(70)
    , mode_(0)
    , target_temp_(70) {
    
    uint8_t icon_pos = (BUTTON_SIZE - BUTTON_ICON_SIZE) / 2;
    const uint8_t* icons[] = {icon_off, icon_heat, icon_cool, icon_auto};

    // Initialize mode button sprites (4BPP)
    for (int i = 0; i < 4; i++) {
        mode_sprites_4bpp[i].initSprite(BUTTON_SIZE, BUTTON_SIZE);
        mode_sprites_4bpp[i].setMode(BB_MODE_4BPP);
        mode_sprites_4bpp[i].fillScreen(0xf);
        mode_sprites_4bpp[i].fillCircle(BUTTON_SIZE / 2, BUTTON_SIZE / 2, BUTTON_SIZE / 2, BBEP_BLACK);
        if (i == 0) {
            mode_sprites_4bpp[i].fillCircle(BUTTON_SIZE / 2, BUTTON_SIZE / 2, BUTTON_SIZE / 2 - BUTTON_BORDER_SIZE, 0xf);
            mode_sprites_4bpp[i].loadBMP(icons[i], icon_pos, icon_pos, 0xf, BBEP_BLACK);
        } else {
            mode_sprites_4bpp[i].loadBMP(icons[i], icon_pos, icon_pos, BBEP_BLACK, 0xf);
        }
    }

    // Initialize mode button sprites (1BPP)
    for (int i = 0; i < 4; i++) {
        mode_sprites_1bpp[i].initSprite(BUTTON_SIZE, BUTTON_SIZE);
        mode_sprites_1bpp[i].setMode(BB_MODE_1BPP);
        mode_sprites_1bpp[i].fillScreen(BBEP_WHITE);
        mode_sprites_1bpp[i].fillCircle(BUTTON_SIZE / 2, BUTTON_SIZE / 2, BUTTON_SIZE / 2, BBEP_BLACK);
        if (i == 0) {
            mode_sprites_1bpp[i].fillCircle(BUTTON_SIZE / 2, BUTTON_SIZE / 2, BUTTON_SIZE / 2 - BUTTON_BORDER_SIZE, BBEP_WHITE);
            mode_sprites_1bpp[i].loadBMP(icons[i], icon_pos, icon_pos, BBEP_WHITE, BBEP_BLACK);
        } else {
            mode_sprites_1bpp[i].loadBMP(icons[i], icon_pos, icon_pos, BBEP_BLACK, BBEP_WHITE);
        }
    }

    // Compute hit boxes
    const int mode_x_min = static_cast<int>(rect_.x) - TOUCH_AREA_MARGIN;
    const int mode_y_min = static_cast<int>(rect_.y) - TOUCH_AREA_MARGIN;
    mode_hit_rect_ = Rect{
        static_cast<uint16_t>(mode_x_min < 0 ? 0 : mode_x_min),
        static_cast<uint16_t>(mode_y_min < 0 ? 0 : mode_y_min),
        static_cast<uint16_t>(BUTTON_SIZE + 2 * TOUCH_AREA_MARGIN),
        static_cast<uint16_t>(BUTTON_SIZE + 2 * TOUCH_AREA_MARGIN)
    };

    // Temperature +/- button hit boxes (40px buttons at y=50)
    uint16_t temp_y = rect_.y + 50;
    uint16_t temp_x = rect_.x + BUTTON_SIZE + 30;
    uint16_t btn_size = 40;
    
    // Minus button on left
    const int minus_x_min = static_cast<int>(temp_x) - TOUCH_AREA_MARGIN;
    const int minus_y_min = static_cast<int>(temp_y) - TOUCH_AREA_MARGIN;
    slider_hit_rect_ = Rect{
        static_cast<uint16_t>(minus_x_min < 0 ? 0 : minus_x_min),
        static_cast<uint16_t>(minus_y_min < 0 ? 0 : minus_y_min),
        static_cast<uint16_t>(btn_size + 2 * TOUCH_AREA_MARGIN),
        static_cast<uint16_t>(btn_size + 2 * TOUCH_AREA_MARGIN)
    };
    
    // Plus button on right (reuse slider_hit_rect_ for minus, we'll check both in touch handler)
}

void Thermostat::fullDraw(FASTEPD* display, BitDepth depth, uint8_t value) {
    // Draw mode button
    if (depth == BitDepth::BD_4BPP) {
        display->drawSprite(&mode_sprites_4bpp[mode_ % 4], rect_.x, rect_.y);
    } else {
        display->drawSprite(&mode_sprites_1bpp[mode_ % 4], rect_.x, rect_.y);
    }

    // Draw label
    display->setFont(Montserrat_Regular_26);
    display->setTextColor(BBEP_BLACK);
    display->setCursor(rect_.x + BUTTON_SIZE + 30, rect_.y + 10);
    display->write(label_);

    // Draw +/- buttons
    uint16_t temp_y = rect_.y + 50;
    uint16_t temp_x = rect_.x + BUTTON_SIZE + 30;
    uint16_t btn_size = 40;
    uint8_t fill_color = (depth == BitDepth::BD_4BPP) ? 0xf : BBEP_WHITE;
    
    // Draw - button (left)
    display->fillCircle(temp_x + btn_size/2, temp_y + btn_size/2, btn_size/2, BBEP_BLACK);
    display->fillCircle(temp_x + btn_size/2, temp_y + btn_size/2, btn_size/2 - 3, fill_color);
    display->fillRect(temp_x + btn_size/2 - 10, temp_y + btn_size/2 - 2, 20, 4, BBEP_BLACK);
    
    // Draw + button (right)
    uint16_t plus_x = temp_x + rect_.w - BUTTON_SIZE - 30 - btn_size;
    display->fillCircle(plus_x + btn_size/2, temp_y + btn_size/2, btn_size/2, BBEP_BLACK);
    display->fillCircle(plus_x + btn_size/2, temp_y + btn_size/2, btn_size/2 - 3, fill_color);
    display->fillRect(plus_x + btn_size/2 - 10, temp_y + btn_size/2 - 2, 20, 4, BBEP_BLACK);
    display->fillRect(plus_x + btn_size/2 - 2, temp_y + btn_size/2 - 10, 4, 20, BBEP_BLACK);
    
    // Draw current/target temperature centered between buttons
    char temp_str[32];
    snprintf(temp_str, sizeof(temp_str), "%d/%d%c", current_temp_, target_temp_, temp_unit_);
    BB_RECT text_rect;
    display->getStringBox(temp_str, &text_rect);
    uint16_t text_x = temp_x + btn_size + (plus_x - temp_x - btn_size) / 2 - text_rect.w / 2;
    display->setCursor(text_x, temp_y + btn_size/2 + text_rect.h/2 - 5);
    display->write(temp_str);
}

Rect Thermostat::partialDraw(FASTEPD* display, BitDepth depth, uint8_t from, uint8_t to) {
    // For temperature changes, clear the text area first to prevent ghosting
    uint16_t temp_y = rect_.y + 50;
    uint16_t temp_x = rect_.x + BUTTON_SIZE + 30;
    uint16_t btn_size = 40;
    uint16_t plus_x = temp_x + rect_.w - BUTTON_SIZE - 30 - btn_size;
    
    // Calculate text area between buttons
    uint16_t text_x = temp_x + btn_size;
    uint16_t text_width = plus_x - text_x;
    uint16_t text_height = btn_size;
    
    // Clear the text area with white
    display->fillRect(text_x, temp_y, text_width, text_height, BBEP_WHITE);
    
    // Now draw the full widget
    fullDraw(display, depth, to);
    return rect_;
}

bool Thermostat::isTouching(const TouchEvent* touch_event) const {
    // Check mode button
    if (touch_event->x >= mode_hit_rect_.x && touch_event->x < mode_hit_rect_.x + mode_hit_rect_.w &&
        touch_event->y >= mode_hit_rect_.y && touch_event->y < mode_hit_rect_.y + mode_hit_rect_.h) {
        return true;
    }
    
    // Check +/- buttons
    uint16_t temp_y = rect_.y + 50;
    uint16_t temp_x = rect_.x + BUTTON_SIZE + 30;
    uint16_t btn_size = 40;
    uint16_t plus_x = temp_x + rect_.w - BUTTON_SIZE - 30 - btn_size;
    
    // Minus button
    if (touch_event->x >= temp_x && touch_event->x < temp_x + btn_size &&
        touch_event->y >= temp_y && touch_event->y < temp_y + btn_size) {
        return true;
    }
    
    // Plus button
    if (touch_event->x >= plus_x && touch_event->x < plus_x + btn_size &&
        touch_event->y >= temp_y && touch_event->y < temp_y + btn_size) {
        return true;
    }
    
    return false;
}

bool Thermostat::isModeTouch(const TouchEvent* touch_event) const {
    return touch_event->x >= mode_hit_rect_.x && touch_event->x < mode_hit_rect_.x + mode_hit_rect_.w &&
           touch_event->y >= mode_hit_rect_.y && touch_event->y < mode_hit_rect_.y + mode_hit_rect_.h;
}

uint8_t Thermostat::getValueFromTouch(const TouchEvent* touch_event, uint8_t original_value) const {
    if (!isTouching(touch_event)) {
        return original_value;
    }

    // Check if touching mode button
    if (isModeTouch(touch_event)) {
        return (original_value + 1) % 4;
    }

    // Check +/- buttons for temperature
    uint16_t temp_y = rect_.y + 50;
    uint16_t temp_x = rect_.x + BUTTON_SIZE + 30;
    uint16_t btn_size = 40;
    uint16_t plus_x = temp_x + rect_.w - BUTTON_SIZE - 30 - btn_size;
    
    // Minus button - decrease temperature
    if (touch_event->x >= temp_x && touch_event->x < temp_x + btn_size &&
        touch_event->y >= temp_y && touch_event->y < temp_y + btn_size) {
        if (original_value > min_temp_) {
            return original_value - 1;
        }
        return original_value;
    }
    
    // Plus button - increase temperature
    if (touch_event->x >= plus_x && touch_event->x < plus_x + btn_size &&
        touch_event->y >= temp_y && touch_event->y < temp_y + btn_size) {
        if (original_value < max_temp_) {
            return original_value + 1;
        }
        return original_value;
    }
    
    return original_value;
}
