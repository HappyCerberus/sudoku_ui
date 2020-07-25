/* (c) 2020 RNDr. Simon Toth (happy.cerberus@gmail.com) */

#include "asm-dom.hpp"
#include <emscripten/val.h>
#include <functional>
#include <string>
#include <sstream>
#include <libc/search.h>
#include <emscripten.h>

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

struct SudokuUI {
    SudokuUI() : puzzle_() {}

    std::vector<RenderedSudokuSquare> squares_;
    sudoku::Sudoku puzzle_;
    void UpdateSquareValue(std::string squareId, std::string key) {
        int id = std::stoi(squareId);
        squares_[id].text = patch(squares_[id].text,
                render_square_text(squareId, squares_[id].x + 50, squares_[id].y + 75, "digit", key));
        squares_[id].value = key[0] - '0';
        puzzle_[id/9][id%9] = squares_[id].value;
        PushHistory();
    }

    void UpdateSquareFromPuzzle(std::string squareId) {
        int id = std::stoi(squareId);
        std::string css_class = "digit-solved";
        if (squares_[id].value != 0) {
            css_class = "digit";
        }
        squares_[id].text = patch(squares_[id].text,
                                  render_square_text(squareId, squares_[id].x + 50, squares_[id].y + 75, css_class,
                                          std::to_string(puzzle_[id/9][id%9].Value())));
        squares_[id].value = puzzle_[id/9][id%9].Value();
        PushHistory();
    }

    void ResetSquare(std::string squareId) {
        int id = std::stoi(squareId);
        squares_[id].text = patch(squares_[id].text,
                                 render_square_text(squareId, squares_[id].x + 50, squares_[id].y + 75, "digit",
                                                    std::string("")));
        squares_[id].value = 0;
        puzzle_[id/9][id%9].Reset();
        PushHistory();
    }

    void PushHistory() {
        std::string data = puzzle_.Serialize();
        auto history = emscripten::val::global("history");
        std::string url = std::string("?puzzle=")+data+std::string("");
        history.call<void>("pushState", emscripten::val(data), emscripten::val(""), emscripten::val(url));
    }

    void RestoreState(const std::string& state) {
        try {
            puzzle_.Deserialize(state);
        } catch (std::exception& e) {
            debugLog(e.what());
        }
        UpdateAllSquares();
    }

    void UpdateAllSquares() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                int id = i*9+j;
                std::string squareId = std::to_string(id);
                std::string content = "";
                if (puzzle_[i][j].IsSet())
                        content = std::to_string(puzzle_[i][j].Value());
                squares_[id].text = patch(squares_[id].text,
                                          render_square_text(squareId, squares_[id].x + 50, squares_[id].y + 75, "digit",
                                                  content));
                squares_[id].value = puzzle_[i][j].Value();
            }
        }
    }
};

SudokuUI ui;


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
            ui.squares_[id].background = patch(ui.squares_[id].background,
                                           render_square_background(squareId, ui.squares_[id].x, ui.squares_[id].y, "square"));
        }

        std::string squareId = e["target"]["dataset"]["cellId"].as<std::string>();
        int id = std::stoi(squareId);
        ui.squares_[id].background = patch(ui.squares_[id].background,
                                       render_square_background(squareId, ui.squares_[id].x, ui.squares_[id].y,
                                                                "square-highlighted"));
        return true;
    }
    return false;
}

extern "C" {
void onPopState() {
    debugLog(emscripten::val(std::string("printing out something")));
}
}

bool onKeyDown(emscripten::val e) {
    if (e["key"].as<std::string>()[0] > '0' && e["key"].as<std::string>()[0] <= '9') {
        emscripten::val res = emscripten::val::global("document").call<emscripten::val>("getElementsByClassName",
                                                                                        emscripten::val(
                                                                                                "square-highlighted"));
        unsigned len = res["length"].as<unsigned>();
        for (unsigned i = 0; i < len; i++) {
            std::string squareId = res[i]["dataset"]["cellId"].as<std::string>();
            ui.UpdateSquareValue(squareId, e["key"].as<std::string>());
        }
    } else if (e["key"].as<std::string>() == "Enter") {
        SolveStats stats;
        if (SmartSolver::Solve(ui.puzzle_, stats)) {
            if (ui.puzzle_.HasConflict()) {
                debugLog("Solved puzzle, but there is a conflict.");
            }
            for (unsigned i = 0; i < 9; i++) {
                for (unsigned j = 0; j < 9; j++) {
                    int id = i * 9 + j;
                    std::string squareId = std::to_string(id);
                    ui.UpdateSquareFromPuzzle(squareId);
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
            ui.ResetSquare(squareId);
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

            ui.squares_.push_back(RenderedSudokuSquare{});
            ui.squares_.back().x = x;
            ui.squares_.back().y = y;
            ui.squares_.back().text = render_square_text(squareId, x + 50, y + 75, "digit");
            ui.squares_.back().background = render_square_background(squareId, x, y, "square");
            ui.squares_.back().container = h("g", Data(
                    Attrs{{"data-cell-id", squareId}},
                    Callbacks{{"onclick", onSquareClick}}),
                                         Children{ui.squares_.back().background, ui.squares_.back().text});
            result.push_back(ui.squares_.back().container);
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
    emscripten::val window = emscripten::val::global("window");
    std::string data = window["location"]["search"].as<std::string>();
    size_t pos = data.find("?puzzle=");
    size_t len = strlen("?puzzle=");
    if (pos != std::string::npos && data.length() > len) {
        ui.RestoreState(data.substr(pos+len,data.length()-len-pos));
    }
    (void) root_view;

    return 0;
};

// vim: filetype=cpp
