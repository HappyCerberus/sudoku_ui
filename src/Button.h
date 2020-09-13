/* (c) 2020 RNDr. Simon Toth (happy.cerberus@gmail.com) */

#ifndef SUDOKU_UI_BUTTON_H
#define SUDOKU_UI_BUTTON_H

#include "asm-dom.hpp"
#include "common.h"

namespace UI {

class Button {
public:
    Button(unsigned x, unsigned y, unsigned width, std::string buttonID, std::string text, std::function<bool(emscripten::val e)> onClick) : x_(x), y_(y), w_(width), buttonID_(std::move(buttonID)), buttonText_(std::move(text)), callback_(std::move(onClick)) {}
    asmdom::VNode *Render(bool selected = false);
    void UpdateSelectState(bool selected);
    void UpdateInputMode(InputMode mode);
private:
    unsigned x_;
    unsigned y_;
    unsigned w_;
    std::string buttonID_;
    std::string buttonText_;
    asmdom::VNode *container_;
    asmdom::VNode *text_;
    asmdom::VNode *background_;
    std::function<bool(emscripten::val e)> callback_;

    asmdom::VNode *RenderBackground(bool selected) const;

    asmdom::VNode *RenderText(InputMode mode = InputMode::NormalInput) const;
};

}

#endif //SUDOKU_UI_BUTTON_H
