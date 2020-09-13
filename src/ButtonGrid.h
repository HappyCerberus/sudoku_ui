/* (c) 2020 RNDr. Simon Toth (happy.cerberus@gmail.com) */

#ifndef SUDOKU_UI_BUTTONGRID_H
#define SUDOKU_UI_BUTTONGRID_H

#include <functional>
#include <emscripten/val.h>

#include "asm-dom.hpp"
#include "common.h"
#include "Button.h"
#include <unordered_map>

namespace UI {

class ButtonGrid {
public:
    ButtonGrid(std::function<bool(emscripten::val e)> onClick) : callback_(std::move(onClick)), mode_(NormalInput) {}
    asmdom::VNode *Render();
    void SwitchToInputMode(InputMode target);
    InputMode GetInputMode() { return mode_; }

private:
    asmdom::VNode *render_buttons();
    asmdom::VNode *render_candidate_mode_button(std::string button_class);
    asmdom::VNode *render_normal_mode_button(std::string button_class);

    std::function<bool(emscripten::val e)> callback_;
    asmdom::VNode *container_;
    InputMode mode_;
    std::unordered_map<std::string, Button> buttons_;
};

}


#endif //SUDOKU_UI_BUTTONGRID_H
