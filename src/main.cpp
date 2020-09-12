/* (c) 2020 RNDr. Simon Toth (happy.cerberus@gmail.com) */

#include "SudokuSquare.h"
#include "SudokuGrid.h"

#include "asm-dom.hpp"
#include <emscripten/val.h>
#include <functional>
#include <string>
#include <sstream>
#include <libc/search.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <functional>

#include "solver/Sudoku.h"
#include "solver/SmartSolver.h"


using asmdom::VNode;
using asmdom::h;
using asmdom::Attrs;
using asmdom::Data;
using asmdom::Children;
using asmdom::Callbacks;




struct Button {
    VNode *text;
    VNode *button;
};

VNode *render_normal_mode_button(std::string button_class);
VNode *render_candidate_mode_button(std::string button_class);

struct ButtonUI {
    Button normal_mode;
    Button candidate_mode;
};

bool candidate_mode_ = false;
ButtonUI buttons;

bool onSquareClick(emscripten::val e);
bool onSquareMouseDown(emscripten::val e);
bool onSquareMouseUp(emscripten::val e);
bool onSquareMouseOver(emscripten::val e);
UI::SudokuGrid ui{std::vector<std::function<bool(emscripten::val)>>{onSquareClick, onSquareMouseDown, onSquareMouseUp, onSquareMouseOver}};

bool onSquareClick(emscripten::val e) {
    static std::chrono::steady_clock::time_point last;
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    if (e["target"]["dataset"]["type"].isUndefined())
        return true;

    if (e["target"]["dataset"]["type"].as<std::string>() == "square" ||
        e["target"]["dataset"]["type"].as<std::string>() == "text") {
        std::string squareId = e["target"]["dataset"]["cellId"].as<std::string>();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
        if (elapsed > 200) {
            ui.FlipSquareHighlight(squareId);
            last = now;
        }
        return true;
    }
    return true;
}

bool mouse_select_dragging_ = false;

bool onSquareMouseDown(emscripten::val e) {
    mouse_select_dragging_ = true;
    return onSquareClick(e);
}

bool onSquareMouseUp(emscripten::val e) {
    mouse_select_dragging_ = false;
    return true;
}

bool onSquareMouseOver(emscripten::val e) {
    if (!mouse_select_dragging_) return true;

    for (unsigned i = 0; i < e["target"]["childElementCount"].as<unsigned>(); i++) {
        if (e["target"]["children"][i]["dataset"]["type"].isUndefined())
            continue;
        if (e["target"]["children"][i]["dataset"]["type"].as<std::string>() != "square")
            continue;

        std::string squareId = e["target"]["children"][i]["dataset"]["cellId"].as<std::string>();
        ui.FlipSquareHighlight(squareId);
    }

    return true;
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
        candidate_mode_ = true;
        buttons.normal_mode.button = patch(buttons.normal_mode.button, render_normal_mode_button("button"));
        buttons.candidate_mode.button = patch(buttons.candidate_mode.button, render_candidate_mode_button("button-selected"));
    } else if (e["target"]["dataset"]["buttonValue"].as<std::string>() == "Normal") {
        candidate_mode_ = false;
        buttons.normal_mode.button = patch(buttons.normal_mode.button, render_normal_mode_button("button-selected"));
        buttons.candidate_mode.button = patch(buttons.candidate_mode.button, render_candidate_mode_button("button"));
    } else if (e["target"]["dataset"]["buttonValue"].as<std::string>() == "Solve") {
        ui.SolveAndUpdateSquares();
    } else if (e["target"]["dataset"]["buttonValue"].as<std::string>()[0] > '0' && e["target"]["dataset"]["buttonValue"].as<std::string>()[0] <= '9') {
        emscripten::val res = emscripten::val::global("document").call<emscripten::val>("getElementsByClassName",
                                                                                        emscripten::val(
                                                                                                "square-highlighted"));
        unsigned len = res["length"].as<unsigned>();
        for (unsigned i = 0; i < len; i++) {
            std::string squareId = res[i]["dataset"]["cellId"].as<std::string>();
            ui.UpdateSquareValue(squareId, e["target"]["dataset"]["buttonValue"].as<std::string>(), candidate_mode_);
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
            ui.UpdateSquareValue(squareId, e["key"].as<std::string>(), candidate_mode_);
        }
    } else if (e["key"].as<std::string>() == "Enter") {
        ui.SolveAndUpdateSquares();
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
                      ui.Render(),
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

bool onpopstate(emscripten::val e) {
    debugLog(e);
    return true;
};

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

    EM_ASM(window['onpopstate'] = Module['onpopstate'];);


    return 0;
};

EMSCRIPTEN_BINDINGS(app) {
        emscripten::function("onpopstate", &onpopstate);
};

// vim: filetype=cpp
