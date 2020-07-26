// (c) 2020 RNDr. Simon Toth (happy.cerberus@gmail.com)

#ifndef SUDOKU_UI_SUDOKUSQUARE_H
#define SUDOKU_UI_SUDOKUSQUARE_H

#include <string>
#include <functional>
#include <emscripten/val.h>
#include "asm-dom.hpp"

namespace UI {

    class SudokuSquare {
    public:
        asmdom::VNode *Container() { return container_; }
        void Render(std::string squareId, int x, int y, std::function<bool(emscripten::val)> callback);
        void UpdateBackground(std::string css_class);
        void UpdateText(std::string css_class, std::string value);
        void UpdateCandidates(std::string css_class, const std::vector<std::string>& cand);

    private:
        static asmdom::VNode *render_candidates(std::string id, int x, int y, std::string css_class, const std::vector<std::string>& values);
        static asmdom::VNode *render_square_text(std::string id, int x, int y, std::string css_class, std::string value = "");
        static asmdom::VNode *render_square_background(std::string id, int x, int y, std::string css_class);

        asmdom::VNode *container_;
        asmdom::VNode *text_;
        asmdom::VNode *candidates_;
        asmdom::VNode *background_;
        std::string square_id_;
        int x_;
        int y_;
    };

}

#endif //SUDOKU_UI_SUDOKUSQUARE_H
