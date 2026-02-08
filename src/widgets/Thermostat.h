#pragma once

#include "Widget.h"
#include <FastEPD.h>

class Thermostat : public Widget {
public:
    Thermostat(const char* label, const uint8_t* icon_off, const uint8_t* icon_heat, 
               const uint8_t* icon_cool, const uint8_t* icon_auto, Rect rect,
               uint8_t min_temp, uint8_t max_temp, char temp_unit);

    void fullDraw(FASTEPD* display, BitDepth depth, uint8_t value) override;
    Rect partialDraw(FASTEPD* display, BitDepth depth, uint8_t from, uint8_t to) override;
    bool isTouching(const TouchEvent* touch_event) const override;
    uint8_t getValueFromTouch(const TouchEvent* touch_event, uint8_t original_value) const override;

    void setCurrentTemp(uint8_t temp) { current_temp_ = temp; }
    void setMode(uint8_t mode) { mode_ = mode; }
    void setTargetTemp(uint8_t temp) { target_temp_ = temp; }
    bool isModeTouch(const TouchEvent* touch_event) const;
    WidgetType getType() const override { return WidgetType::Thermostat; }

private:
    const char* label_;
    Rect rect_;
    Rect mode_hit_rect_;
    Rect slider_hit_rect_;
    uint8_t min_temp_;
    uint8_t max_temp_;
    char temp_unit_;
    uint8_t current_temp_;
    uint8_t mode_;
    uint8_t target_temp_;

    FASTEPD mode_sprites_4bpp[4];
    FASTEPD mode_sprites_1bpp[4];
};
