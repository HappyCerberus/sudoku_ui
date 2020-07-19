/* (c) 2020 RNDr. Simon Toth (happy.cerberus@gmail.com) */

#include "asm-dom.hpp"
#include <emscripten/val.h>
#include <functional>
#include <string>
#include <sstream>
#include <libc/search.h>

#include "solver/Sudoku.h"
#include "solver/SmartSolver.h"

using asmdom::VNode;
using asmdom::h;
using asmdom::Attrs;
using asmdom::Data;
using asmdom::Children;
using asmdom::Callbacks;

struct RenderedSudokuSquare {
    VNode *container;
    VNode *text;
    VNode *background;
    int x;
    int y;
    unsigned value = 0;
};

std::vector<RenderedSudokuSquare> squares;

VNode *render_square_background(std::string id, int x, int y, std::string css_class);

VNode *render_square_text(std::string id, int x, int y, std::string css_class, std::string value);

std::string debugString(emscripten::val e) {
    if (e.isString()) {
        return e.as<std::string>();
    } else if (e.isArray()) {
        return "is an array";
    } else if (e.isNull()) {
        return "is null";
    } else if (e.isNumber()) {
        return std::to_string(e.as<double>());
    } else if (e.isUndefined()) {
        return "is undefined";
    } else if (e.isTrue()) {
        return "is true";
    } else if (e.isFalse()) {
        return "is false";
    } else {
        return "something else";
    }
}


void debugLog(emscripten::val e) {
    emscripten::val::global("console").call<void>("log", e);
}

void debugLog(std::string s) {
    debugLog(emscripten::val(s));
}


bool onSquareClick(emscripten::val e) {
    // find any previously highlighted squares and un-highlight
    // highlight the current square
    if (e["target"]["dataset"]["type"].as<std::string>() == "square" ||
        e["target"]["dataset"]["type"].as<std::string>() == "text") {
        emscripten::val res = emscripten::val::global("document").call<emscripten::val>("getElementsByClassName",
                                                                                        emscripten::val(
                                                                                                "square-highlighted"));
        unsigned len = res["length"].as<unsigned>();
        for (unsigned i = 0; i < len; i++) {
            std::string squareId = res[i]["dataset"]["cellId"].as<std::string>();
            int id = std::stoi(squareId);
            squares[id].background = patch(squares[id].background,
                                           render_square_background(squareId, squares[id].x, squares[id].y, "square"));
        }

        std::string squareId = e["target"]["dataset"]["cellId"].as<std::string>();
        int id = std::stoi(squareId);
        squares[id].background = patch(squares[id].background,
                                       render_square_background(squareId, squares[id].x, squares[id].y,
                                                                "square-highlighted"));
        return true;
    }
    return false;
}

bool onKeyDown(emscripten::val e) {
    if (e["key"].as<std::string>()[0] > '0' && e["key"].as<std::string>()[0] <= '9') {
        emscripten::val res = emscripten::val::global("document").call<emscripten::val>("getElementsByClassName",
                                                                                        emscripten::val(
                                                                                                "square-highlighted"));
        unsigned len = res["length"].as<unsigned>();
        for (unsigned i = 0; i < len; i++) {
            std::string squareId = res[i]["dataset"]["cellId"].as<std::string>();
            int id = std::stoi(squareId);
            squares[id].text = patch(squares[id].text,
                                     render_square_text(squareId, squares[id].x + 50, squares[id].y + 75, "digit",
                                                        e["key"].as<std::string>()));
            squares[id].value = e["key"].as<std::string>()[0] - '0';
        }
    } else if (e["key"].as<std::string>() == "Enter") {
        sudoku::Sudoku s;
        for (unsigned i = 0; i < 9; i++) {
            for (unsigned j = 0; j < 9; j++) {
                if (squares[i * 9 + j].value == 0) {
                    s[i][j].Reset();
                } else {
                    s[i][j] = squares[i * 9 + j].value;
                }
            }
        }
        SolveStats stats;
        if (SmartSolver::Solve(s, stats)) {
            for (unsigned i = 0; i < 9; i++) {
                for (unsigned j = 0; j < 9; j++) {
                    int id = i * 9 + j;
                    std::string squareId = std::to_string(id);
                    std::string css_class = "digit-solved";
                    if (squares[id].value != 0) {
                        css_class = "digit";
                    }
                    squares[id].text = patch(squares[id].text,
                                             render_square_text(squareId, squares[id].x + 50, squares[id].y + 75,
                                                                css_class, std::to_string(s[i][j].Value())));
                }
            }
        } else {
            debugLog("unable to solve this puzzle");
        }
    } else if (e["key"].as<std::string>() == "Delete") {
        emscripten::val res = emscripten::val::global("document").call<emscripten::val>("getElementsByClassName",
                                                                                        emscripten::val(
                                                                                                "square-highlighted"));
        unsigned len = res["length"].as<unsigned>();
        for (unsigned i = 0; i < len; i++) {
            std::string squareId = res[i]["dataset"]["cellId"].as<std::string>();
            int id = std::stoi(squareId);
            squares[id].text = patch(squares[id].text,
                                     render_square_text(squareId, squares[id].x + 50, squares[id].y + 75, "digit",
                                                        std::string("")));
            squares[id].value = 0;
        }
    }
    return true;
}

VNode *render_square_background(std::string id, int x, int y, std::string css_class) {
    return h("rect", Data(Attrs{
            {"id",           std::string("square_rect_") + id},
            {"ns",           "http://www.w3.org/2000/svg"},
            {"class",        css_class},
            {"data-cell-id", id},
            {"data-type",    std::string{"square"}},
            {"width",        "100px"},
            {"height",       "100px"},
            {"x",            std::to_string(x)},
            {"y",            std::to_string(y)},
            {"fill",         "transparent"}}));
}

VNode *render_square_text(std::string id, int x, int y, std::string css_class, std::string value = "") {
    return h("text", Data(Attrs{
                     {"id",           std::string("square_text_") + id},
                     {"ns",           "http://www.w3.org/2000/svg"},
                     {"data-cell-id", id},
                     {"data-type",    std::string{"text"}},
                     {"class",        css_class},
                     {"x",            std::to_string(x)},
                     {"y",            std::to_string(y)},
                     {"text-anchor",  std::string("middle")}}),
             value);
}

VNode *render_squares() {
    Children result;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            int x = 50 + j * 100;
            int y = 50 + i * 100;
            std::string squareId = std::to_string(i * 9 + j);

            squares.push_back(RenderedSudokuSquare{});
            squares.back().x = x;
            squares.back().y = y;
            squares.back().text = render_square_text(squareId, x + 50, y + 75, "digit");
            squares.back().background = render_square_background(squareId, x, y, "square");
            squares.back().container = h("g", Data(
                    Attrs{{"data-cell-id", squareId}},
                    Callbacks{{"onclick", onSquareClick}}),
                                         Children{squares.back().background, squares.back().text});
            result.push_back(squares.back().container);
        }
    }
    return h("g", Data(Attrs{{"id", "squares"}}), result);
}

VNode *render_blocks() {
    Children result;
    for (int i = 0; i < 9; i++) {
        int x = 50 + (i % 3) * 300;
        int y = 50 + (i / 3) * 300;
        std::stringstream path;
        path << "M " << std::to_string(x) << " " << std::to_string(y) << " h 300 v 300 h -300 v -300";
        result.push_back(h("path",
                           Data(Attrs{{"class", "line-thick"},
                                      {"ns",    "http://www.w3.org/2000/svg"},
                                      {"d",     path.str()},
                                      {"fill",  "none"}})));
    }
    return h("g", Data(Attrs{{"id", "blocks"}}), result);
}

VNode *render_background() {
    // TODO: generate path, so it works for all sizes of Sudoku
    Children grid{
            h("path", Data(Attrs{{"class", "line"},
                                 {"d",     "M 50 50 h 900 M 50 150 h 900 M 50 250 h 900 M 50 350 h 900 M 50 450 h 900 M 50 550 h 900 M 50 650 h 900 M 50 750 h 900 M 50 850 h 900 M 50 950 h 900"}})),
            h("path", Data(Attrs{{"class", "line"},
                                 {"d",     "M 50 50 v 900 M 150 50 v 900 M 250 50 v 900 M 350 50 v 900 M 450 50 v 900 M 550 50 v 900 M 650 50 v 900 M 750 50 v 900 M 850 50 v 900 M 950 50 v 900"}})),
            h("path", Data(Attrs{{"class", "line-thick"},
                                 {"d",     "M 50 50 h 900 v 900 h -900 v -900"},
                                 {"fill",  "none"}}))
    };
    return h("g", Data(Attrs{{"id", "background"}}), grid);
}

VNode *render_puzzle() {
    VNode *puzzle =
            h("body",
              Callbacks{{"onkeydown", onKeyDown}},
              Children{
                      h("svg",
                        Data(Attrs{{"id",      "canvas"},
                                   {"xmlns",   "http://www.w3.org/2000/svg"},
                                   {"viewBox", "0 0 1000 1000"},
                                   {"ns",      "http://www.w3.org/2000/svg"}}),
                        Children{
                                render_squares(),
                                render_background(),
                                render_blocks(),
                        })});
    return puzzle;
}

int main() {
    // Initialize asm-dom.
    asmdom::Config config = asmdom::Config();
    config.unsafePatch = true;
    asmdom::init(config);

    emscripten::val document = emscripten::val::global("document");
    VNode *root_view = patch(document.call<emscripten::val>("getElementById", emscripten::val("root")),
                             render_puzzle());
    (void) root_view;

    return 0;
};

// vim: filetype=cpp
