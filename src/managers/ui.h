#pragma once
#include "screen.h"
#include "store.h"
#include "ui_state.h"
#include <FastEPD.h>

struct UITaskArgs {
    EntityStore* store;
    ScreenManager* screens;
    FASTEPD* epaper;
    SharedUIState* shared_state;
};

void ui_task(void* arg);
