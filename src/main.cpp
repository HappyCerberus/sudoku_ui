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
    VNode *candidates;
    VNode *background;
    int x;
    int y;
    unsigned value = 0;
};

struct Button {
    VNode *text;
    VNode *button;
};

VNode *render_square_background(std::string id, int x, int y, std::string css_class);
VNode *render_square_text(std::string id, int x, int y, std::string css_class, std::string value);
VNode *render_candidates(std::string id, int x, int y, std::string css_class, const std::vector<std::string>& values);
VNode *render_normal_mode_button(std::string button_class);
VNode *render_candidate_mode_button(std::string button_class);

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

struct ButtonUI {
    Button normal_mode;
    Button candidate_mode;
};

ButtonUI buttons;

struct SudokuUI {
    SudokuUI() : puzzle_(), candidate_mode_(false) {}

    std::vector<RenderedSudokuSquare> squares_;
    sudoku::Sudoku puzzle_;
    bool candidate_mode_;

    void ReRenderSquare(std::string squareId) {
        int id = std::stoi(squareId);
        std::string value = std::to_string(puzzle_[id/9][id%9].Value());
        std::vector<std::string> cand;
        for (int i = 1; i <= 9; i++) {
            if (puzzle_[id/9][id%9].IsPossible(i)) {
                cand.push_back(std::to_string(i));
            } else {
                cand.push_back("");
            }
        }

        std::string big_text_class;
        std::string small_text_class;
        if (puzzle_[id/9][id%9].IsSet()) {
            big_text_class = "digit";
            small_text_class = "small-digit-hidden";
        } else {
            big_text_class = "digit-hidden";
            small_text_class = "small-digit";
        }

        squares_[id].text = patch(squares_[id].text,
                                  render_square_text(squareId, squares_[id].x + 50, squares_[id].y + 75, big_text_class, value));
        squares_[id].candidates = patch(squares_[id].candidates, render_candidates(squareId, squares_[id].x, squares_[id].y, small_text_class, cand));
        squares_[id].value = puzzle_[id/9][id%9].Value();
    }

    void UpdateSquareValue(std::string squareId, std::string key) {
        int id = std::stoi(squareId);
        int value = key[0] - '0';
        if (candidate_mode_) {
            if (puzzle_[id/9][id%9].IsPossible(value)) {
                puzzle_[id/9][id%9] -= value;
            } else {
                puzzle_[id/9][id%9] += value;
            }
        } else {
            if (puzzle_[id/9][id%9].Value() == value) {
                puzzle_[id/9][id%9].Reset();
            } else {
                puzzle_[id/9][id%9] = value;
            }
        }

        ReRenderSquare(squareId);
        PushHistory();
    }

    void ResetSquare(std::string squareId) {
        int id = std::stoi(squareId);
        puzzle_[id/9][id%9].Reset();
        ReRenderSquare(squareId);
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
                ReRenderSquare(squareId);
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

bool onMouseDown(emscripten::val e) {
    if (e["target"]["dataset"]["buttonValue"].as<std::string>() == "Delete") {
        emscripten::val res = emscripten::val::global("document").call<emscripten::val>("getElementsByClassName",
                                                                                        emscripten::val(
                                                                                                "square-highlighted"));
        unsigned len = res["length"].as<unsigned>();
        for (unsigned i = 0; i < len; i++) {
            std::string squareId = res[i]["dataset"]["cellId"].as<std::string>();
            ui.ResetSquare(squareId);
        }
    } else if (e["target"]["dataset"]["buttonValue"].as<std::string>() == "Candidate") {
        ui.candidate_mode_ = true;
        buttons.normal_mode.button = patch(buttons.normal_mode.button, render_normal_mode_button("button"));
        buttons.candidate_mode.button = patch(buttons.candidate_mode.button, render_candidate_mode_button("button-selected"));
    } else if (e["target"]["dataset"]["buttonValue"].as<std::string>() == "Normal") {
        ui.candidate_mode_ = false;
        buttons.normal_mode.button = patch(buttons.normal_mode.button, render_normal_mode_button("button-selected"));
        buttons.candidate_mode.button = patch(buttons.candidate_mode.button, render_candidate_mode_button("button"));
    } else if (e["target"]["dataset"]["buttonValue"].as<std::string>() == "Solve") {
        SolveStats stats;
        if (!SmartSolver::Solve(ui.puzzle_, stats)) {
            debugLog("unable to solve this puzzle");
        }
        ui.UpdateAllSquares();
    } else if (e["target"]["dataset"]["buttonValue"].as<std::string>()[0] > '0' && e["target"]["dataset"]["buttonValue"].as<std::string>()[0] <= '9') {
        emscripten::val res = emscripten::val::global("document").call<emscripten::val>("getElementsByClassName",
                                                                                        emscripten::val(
                                                                                                "square-highlighted"));
        unsigned len = res["length"].as<unsigned>();
        for (unsigned i = 0; i < len; i++) {
            std::string squareId = res[i]["dataset"]["cellId"].as<std::string>();
            ui.UpdateSquareValue(squareId, e["target"]["dataset"]["buttonValue"].as<std::string>());
        }
    }
    return true;
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
        if (!SmartSolver::Solve(ui.puzzle_, stats)) {
            debugLog("unable to solve this puzzle");
        }
        ui.UpdateAllSquares();
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

VNode *render_normal_mode_button(std::string button_class) {
    return h("rect",
             Data(
                     Attrs{
                             {"class",             button_class},
                             {"data-button-value", "Normal"},
                             {"x",                 std::to_string(160)},
                             {"y",                 std::to_string(380)},
                             {"width",             "80px"},
                             {"height",            "80px"},
                             {"rx",                "10px"},
                             {"ry",                "10px"}
                     },
                     Callbacks{
                             {"onmousedown", onMouseDown}
                     }));
}

VNode *render_candidate_mode_button(std::string button_class) {
    return h("rect",
      Data(
              Attrs{
                      {"class",             button_class},
                      {"data-button-value", "Candidate"},
                      {"x",                 std::to_string(260)},
                      {"y",                 std::to_string(380)},
                      {"width",             "80px"},
                      {"height",            "80px"},
                      {"rx",                "10px"},
                      {"ry",                "10px"}
              },
              Callbacks{
                      {"onmousedown", onMouseDown}
              }));
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

VNode *render_candidates(std::string id, int x, int y, std::string css_class, const std::vector<std::string>& values) {
    Children candidates;
    for (int i = 0; i < 9; i++) {
        std::string subId = std::to_string(i+1);
        candidates.push_back(
                h("text", Data(Attrs{
                  {"id",           std::string("square_candidate_text_") + id + "_" + subId},
                  {"ns",           "http://www.w3.org/2000/svg"},
                  {"data-cell-id", id},
                  {"data-cell-candidate-id", subId},
                  {"data-type",    std::string{"text"}},
                  {"class",        css_class},
                  {"x",            std::to_string(x+20+(i%3)*30)},
                  {"y",            std::to_string(y+30+(i/3)*30)},
                  {"text-anchor",  std::string("middle")}}),
          values[i]));
    }
    return h("g", Data(Attrs{{"id", std::string("square_candidates_")+id}}),
            candidates);
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
            ui.squares_.back().candidates = render_candidates(squareId, x, y, "small-digit", std::vector<std::string>(9,""));
            ui.squares_.back().text = render_square_text(squareId, x + 50, y + 75, "digit");
            ui.squares_.back().background = render_square_background(squareId, x, y, "square");
            ui.squares_.back().container = h("g", Data(
                    Attrs{{"data-cell-id", squareId}},
                    Callbacks{{"onclick", onSquareClick}}),
                                         Children{ui.squares_.back().background, ui.squares_.back().text, ui.squares_.back().candidates});
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

VNode *render_buttons() {
    Children grid;
    for (int i = 1; i <= 9; i++) {
        std::string buttonId = std::to_string(i);
        int x = 50+((i-1)%3)*100;
        int y = 50+((i-1)/3)*100;
        grid.push_back(h("g", Children{
                h("text", Data(Attrs{
                          {"id",           std::string("button_text_") + buttonId},
                          {"ns",           "http://www.w3.org/2000/svg"},
                          {"data-button-value", buttonId},
                          {"data-type",    std::string{"text"}},
                          {"class",        "button-text"},
                          {"x",            std::to_string(x+50)},
                          {"y",            std::to_string(y+75)},
                          {"text-anchor",  std::string("middle")}}),
                  buttonId),
            h("rect",
                    Data(
                            Attrs{
                                {"class", "button"},
                                {"data-button-value", buttonId},
                                {"x",            std::to_string(x+10)},
                                {"y",            std::to_string(y+10)},
                                {"width",        "80px"},
                                {"height",       "80px"},
                                {"rx", "10px"},
                                {"ry", "10px"}
                                },
                    Callbacks{
                            {"onmousedown", onMouseDown}
                    }))}));
    }

    // Normal input mode
    buttons.normal_mode.text = h("text", Data(Attrs{
              {"id",           std::string("button_text_normal")},
              {"ns",           "http://www.w3.org/2000/svg"},
              {"data-button-value", "normal"},
              {"data-type",    std::string{"normal"}},
              {"class",        "button-text"},
              {"x",            std::to_string(200)},
              {"y",            std::to_string(445)},
              {"text-anchor",  std::string("middle")}}),
      "✒");
    buttons.normal_mode.button = render_normal_mode_button("button-selected");
    // Candidate input mode
    buttons.candidate_mode.text = h("text", Data(Attrs{
                      {"id",           std::string("button_text_candidate")},
                      {"ns",           "http://www.w3.org/2000/svg"},
                      {"data-button-value", "candidate"},
                      {"data-type",    std::string{"candidate"}},
                      {"class",        "button-text"},
                      {"x",            std::to_string(300)},
                      {"y",            std::to_string(445)},
                      {"text-anchor",  std::string("middle")}}),
              "✏");
    buttons.candidate_mode.button = render_candidate_mode_button("button");

    // Delete
    grid.push_back(h("g", Children{
            h("text", Data(Attrs{
                      {"id",           std::string("button_text_delete")},
                      {"ns",           "http://www.w3.org/2000/svg"},
                      {"data-button-value", "delete"},
                      {"data-type",    std::string{"delete"}},
                      {"class",        "button-text"},
                      {"x",            std::to_string(100)},
                      {"y",            std::to_string(445)},
                      {"text-anchor",  std::string("middle")}}),
              "🗑"),
            h("rect",
              Data(
                      Attrs{
                              {"class", "button"},
                              {"data-button-value", "Delete"},
                              {"x",            std::to_string(60)},
                              {"y",            std::to_string(380)},
                              {"width",        "80px"},
                              {"height",       "80px"},
                              {"rx", "10px"},
                              {"ry", "10px"}
                      },
                      Callbacks{
                              {"onmousedown", onMouseDown}
                      })),
                      buttons.normal_mode.text,
                      buttons.normal_mode.button,
                buttons.candidate_mode.text,
                buttons.candidate_mode.button,
    }));
    grid.push_back(h("g", Children{
            h("text", Data(Attrs{
                      {"id",           std::string("button_text_solve")},
                      {"ns",           "http://www.w3.org/2000/svg"},
                      {"data-button-value", "solve"},
                      {"data-type",    std::string{"solve"}},
                      {"class",        "button-text"},
                      {"x",            std::to_string(160)},
                      {"y",            std::to_string(545)},
                      {"text-anchor",  std::string("middle")}}),
              "Solve"),
            h("rect",
              Data(
                      Attrs{
                              {"class", "button"},
                              {"data-button-value", "Solve"},
                              {"x",            std::to_string(60)},
                              {"y",            std::to_string(480)},
                              {"width",        "280px"},
                              {"height",       "80px"},
                              {"rx", "10px"},
                              {"ry", "10px"}
                      },
                      Callbacks{
                              {"onmousedown", onMouseDown}
                      }))}));
    return h("g", Data(Attrs{{"id", "button_grid"}}), grid);
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
                        }),
                      h("svg",
                              Data(Attrs{{"id", "buttons"},
                                         {"xmlns",   "http://www.w3.org/2000/svg"},
                                         {"ns",      "http://www.w3.org/2000/svg"},
                                         {"viewBox", "0 0 400 800"}}),
                                         Children({
                                             render_buttons(),
                                         })
                                )});
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
