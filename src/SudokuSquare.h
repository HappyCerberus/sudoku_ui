// (c) 2020 RNDr. Simon Toth (happy.cerberus@gmail.com)

#ifndef SUDOKU_UI_SUDOKUSQUARE_H
#define SUDOKU_UI_SUDOKUSQUARE_H

#include <string>
#include <functional>
#include <emscripten/val.h>
#include "asm-dom.hpp"
#include "common.h"

namespace UI {

class SudokuSquare {
public:
    //asmdom::VNode *Container() { return container_; }
    asmdom::VNode *Render(std::string squareId, int x, int y, std::vector<std::function<bool(emscripten::val)>> callbacks);
    void UpdateBackground(bool highlighted);
    void FlipBackground();
    void UpdateText(bool hidden, std::string value);
    void UpdateCandidates(bool hidden, const std::vector<std::string>& cand);

private:
    void UpdateCandidates(std::string css_class, const std::vector<std::string>& cand);
    void UpdateText(std::string css_class, std::string value);
    void UpdateBackground(std::string css_class);
    static asmdom::VNode *render_candidates(std::string id, int x, int y, std::string css_class, const std::vector<std::string>& values);
    static asmdom::VNode *render_square_text(std::string id, int x, int y, std::string css_class, std::string value = "");
    static asmdom::VNode *render_square_background(std::string id, int x, int y, std::string css_class);

    asmdom::VNode *container_;
    asmdom::VNode *text_;
    asmdom::VNode *candidates_;
    asmdom::VNode *background_;
    bool highlighted_;
    std::string square_id_;
    int x_;
    int y_;
};

}

#endif //SUDOKU_UI_SUDOKUSQUARE_H
